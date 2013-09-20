#include "StdAfx.h"
#include "DocHistoryMgr.h"
#include "MainFrm.h"
#include "MainView.h"

using namespace my;

StaticMeshTreeNode::~StaticMeshTreeNode(void)
{
}

void StaticMeshTreeNode::SetMesh(my::OgreMeshPtr mesh)
{
	ASSERT(mesh);

	m_mesh = mesh;
}

void StaticMeshTreeNode::DrawStaticMeshTreeNode(
	StaticMeshTreeNode * node,
	IDirect3DDevice9 * pd3dDevice,
	float fElapsedTime,
	DWORD RenderMode,
	my::Vector4 & Color)
{
	my::Effect * pSimpleSample = CMainView::getSingleton().m_SimpleSample.get();

	DWORD OldState;
	pd3dDevice->GetRenderState(D3DRS_FILLMODE, &OldState);
	if(CMainView::RenderModeWire == RenderMode)
	{
		pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}

	pSimpleSample->SetVector("g_MaterialAmbientColor", CMainView::RenderModeWire == RenderMode ? Color : Vector4(0,0,0,1));
	pSimpleSample->SetVector("g_MaterialDiffuseColor", CMainView::RenderModeWire == RenderMode ? Vector4(0,0,0,1) : Color);
	pSimpleSample->SetTexture("g_MeshTexture", CMainView::getSingleton().m_WhiteTex);
	UINT cPasses = pSimpleSample->Begin();
	for(UINT p = 0; p < cPasses; p++)
	{
		pSimpleSample->BeginPass(p);
		for(DWORD i = 0; i < node->m_mesh->GetMaterialNum(); i++)
		{
			node->m_mesh->DrawSubset(i);
		}
		pSimpleSample->EndPass();
	}
	pSimpleSample->End();

	if(CMainView::RenderModeWire == RenderMode)
	{
		pd3dDevice->SetRenderState(D3DRS_FILLMODE, OldState);
	}
}

void StaticMeshTreeNode::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, DWORD RenderMode, bool IsSelected)
{
	if(m_mesh)
	{
		switch(RenderMode)
		{
		case CMainView::RenderModeDefault:
			{
				DrawStaticMeshTreeNode(this, pd3dDevice, fElapsedTime, RenderMode, my::Vector4(1,1,1,1));
			}
			if(IsSelected)
			{
				DrawStaticMeshTreeNode(this, pd3dDevice, fElapsedTime, CMainView::RenderModeWire, my::Vector4(67,255,163,255)/my::Vector4(255,255,255,255));
			}
			break;

		case CMainView::RenderModeWire:
			if(IsSelected)
			{
				DrawStaticMeshTreeNode(this, pd3dDevice, fElapsedTime, CMainView::RenderModeWire, my::Vector4(67,255,163,255)/my::Vector4(255,255,255,255));
			}
			else
			{
				DrawStaticMeshTreeNode(this, pd3dDevice, fElapsedTime, RenderMode, my::Vector4(0,4,96,255)/my::Vector4(255,255,255,255));
			}
			break;
		}
	}
}

void CDocHistory::Do(void)
{
	const_iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		hist_iter->first->Do();
	}
}

void CDocHistory::Undo(void)
{
	const_reverse_iterator hist_iter = rbegin();
	for(; hist_iter != rend(); hist_iter++)
	{
		hist_iter->second->Do();
	}
}

void CAddTreeNodeStep::Do(void)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	ASSERT(!m_strItem.empty() && pOutliner->m_ItemMap.end() == pOutliner->m_ItemMap.find(m_strItem));
	ASSERT(m_strParent.empty() || pOutliner->m_ItemMap.end() == pOutliner->m_ItemMap.find(m_strParent));
	ASSERT(m_strBefore.empty() || pOutliner->m_ItemMap.end() == pOutliner->m_ItemMap.find(m_strBefore));

	HTREEITEM hParent = m_strParent.empty() ? TVI_ROOT : pOutliner->m_ItemMap[m_strParent];
	HTREEITEM hBefore = m_strBefore.empty() ? TVI_LAST : pOutliner->m_ItemMap[m_strBefore];
	pOutliner->InsertItem(m_strItem, m_node, hParent, hBefore);
}

void CDeleteTreeNodeStep::Do(void)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	ASSERT(!m_strItem.empty() && pOutliner->m_ItemMap.end() != pOutliner->m_ItemMap.find(m_strItem));
	HTREEITEM hItem = pOutliner->m_ItemMap[m_strItem];

	m_node = boost::dynamic_pointer_cast<StaticMeshTreeNode>(pOutliner->GetItemNode(hItem));
	ASSERT(m_node);

	pOutliner->m_TreeCtrl.DeleteItem(hItem);
}

void CDocHistoryMgr::Do(void)
{
	ASSERT(m_nStep < (int)size() - 1);

	operator[](++m_nStep)->Do();
}

void CDocHistoryMgr::Undo(void)
{
	ASSERT(m_nStep >= 0);

	operator[](m_nStep--)->Undo();
}

void CDocHistoryMgr::ClearAllHistory(void)
{
	clear();

	m_nStep = -1;
}

void CDocHistoryMgr::AddHistory(CDocHistoryPtr hist)
{
	if(!empty())
		erase(begin() + (m_nStep + 1), end());

	push_back(hist);
}

void CDocHistoryMgr::AddTreeNode(LPCTSTR lpszItem, TreeNodeBasePtr node)
{
	CDocHistoryPtr hist(new CDocHistory());
	hist->push_back(std::make_pair(
		CDocStepBasePtr(new CAddTreeNodeStep(lpszItem, node)),
		CDocStepBasePtr(new CDeleteTreeNodeStep(lpszItem))));

	AddHistory(hist);

	Do();
}

void CDocHistoryMgr::DeleteTreeNode(HTREEITEM hItem)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	CString strItem = pOutliner->m_TreeCtrl.GetItemText(hItem);
	boost::shared_ptr<CDeleteTreeNodeStep> del_step(new CDeleteTreeNodeStep(strItem));
	del_step->Do();

	CDocHistoryPtr hist(new CDocHistory());
	hist->push_back(std::make_pair(
		del_step,
		CDocStepBasePtr(new CAddTreeNodeStep(strItem, del_step->m_node))));

	AddHistory(hist);

	m_nStep++;
}

void CDocHistoryMgr::AddStaticMeshTreeNode(LPCTSTR lpszItem, my::OgreMeshPtr mesh)
{
	StaticMeshTreeNodePtr node(new StaticMeshTreeNode);
	node->SetMesh(mesh);

	AddTreeNode(lpszItem, node);
}

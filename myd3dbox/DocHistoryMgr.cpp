#include "StdAfx.h"
#include "DocHistoryMgr.h"
#include "MainFrm.h"
#include "MainView.h"

using namespace my;

StaticMeshTreeNode::~StaticMeshTreeNode(void)
{
	//CMainView * pView = CMainView::getSingletonPtr();
	//ASSERT(pView);

	//pView->m_dynamicsWorld->removeRigidBody(m_rigidBody.get());
}

void StaticMeshTreeNode::SetMesh(my::OgreMeshPtr mesh)
{
	ASSERT(mesh);

	m_mesh = mesh;

	VOID * pIndices = m_mesh->LockIndexBuffer();

	VOID * pVertices = m_mesh->LockVertexBuffer();

	m_indexVertexArray.reset(new btTriangleIndexVertexArray(
		m_mesh->GetNumFaces(),
		(int *)pIndices,
		sizeof(DWORD) * 3,
		m_mesh->GetNumVertices(),
		(btScalar *)pVertices,
		m_mesh->m_VertexElemSet.CalculateVertexStride()));

	m_meshShape.reset(new btBvhTriangleMeshShape(
		m_indexVertexArray.get(), true, btVector3(-1000,-1000,-1000), btVector3(1000,1000,1000)));

	m_mesh->UnlockVertexBuffer();

	m_mesh->UnlockIndexBuffer();

	m_motionState.reset(new btDefaultMotionState(btTransform::getIdentity()));

	m_rigidBody.reset(new btRigidBody(0, m_motionState.get(), m_meshShape.get(), btVector3(0,0,0)));

	m_rigidBody->setContactProcessingThreshold(1e18f);

	CMainView * pView = CMainView::getSingletonPtr();
	ASSERT(pView);

	pView->m_dynamicsWorld->addRigidBody(m_rigidBody.get());
}

void StaticMeshTreeNode::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_mesh)
	{
		CMainFrame::getSingleton().m_SimpleSample->SetVector("g_MaterialAmbientColor", Vector4(0,0,0,1));
		CMainFrame::getSingleton().m_SimpleSample->SetVector("g_MaterialDiffuseColor", Vector4(1,1,1,1));
		CMainFrame::getSingleton().m_SimpleSample->SetTexture("g_MeshTexture", CMainFrame::getSingleton().m_WhiteTex->m_ptr);
		UINT cPasses = CMainFrame::getSingleton().m_SimpleSample->Begin();
		for(UINT p = 0; p < cPasses; p++)
		{
			CMainFrame::getSingleton().m_SimpleSample->BeginPass(p);
			for(DWORD i = 0; i < m_mesh->GetMaterialNum(); i++)
			{
				m_mesh->DrawSubset(i);
			}
			CMainFrame::getSingleton().m_SimpleSample->EndPass();
		}
		CMainFrame::getSingleton().m_SimpleSample->End();
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

void CAddStaticMeshTreeNodeStep::Do(void)
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

void CDeleteStaticMeshTreeNodeStep::Do(void)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	ASSERT(!m_strItem.empty() && pOutliner->m_ItemMap.end() != pOutliner->m_ItemMap.find(m_strItem));
	HTREEITEM hItem = pOutliner->m_ItemMap[m_strItem];

	m_node = boost::dynamic_pointer_cast<StaticMeshTreeNode>(pOutliner->GetItemNode(hItem));
	ASSERT(m_node);

	pOutliner->m_TreeCtrl.DeleteItem(hItem);
}

void CDocHistoryMgr::AddTreeStaticMeshNode(LPCTSTR lpszItem, my::OgreMeshPtr mesh)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	StaticMeshTreeNodePtr node(new StaticMeshTreeNode);
	node->SetMesh(mesh);

	CDocHistoryPtr hist(new CDocHistory());
	hist->push_back(std::make_pair(
		CDocStepBasePtr(new CAddStaticMeshTreeNodeStep(lpszItem, node)),
		CDocStepBasePtr(new CDeleteStaticMeshTreeNodeStep(lpszItem))));

	push_back(hist);

	operator[](++m_nStep)->Do();
}

#include "StdAfx.h"
#include "DocHistoryMgr.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "MainView.h"

using namespace my;

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

void CDocHistoryMgr::AddStaticMeshTreeNode(LPCTSTR lpszMesh)
{
	StaticMeshTreeNodePtr node(new StaticMeshTreeNode);
	node->m_Mesh = theApp.LoadMesh(ts2ms(lpszMesh));
	if(node->m_Mesh)
	{
		std::vector<std::string>::const_iterator mat_name_iter = node->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != node->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			MaterialPtr mat = theApp.LoadMaterial(str_printf("material/%s.txt", mat_name_iter->c_str()));
			mat = mat ? mat : theApp.m_DefaultMat;
			node->m_Materials.push_back(StaticMeshTreeNode::MaterialPair(mat, theApp.LoadEffect("shader/SimpleSample.fx", EffectMacroPairList())));
		}

		static unsigned int i = 0;
		CString strItem;
		strItem.Format(_T("mesh_%03d"), i++);
		AddTreeNode(strItem, node);
	}
}

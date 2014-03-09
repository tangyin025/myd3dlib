#include "StdAfx.h"
#include "HistoryMgr.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "MainView.h"

using namespace my;

void CHistory::Do(void)
{
	const_iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		hist_iter->first->Do();
	}
}

void CHistory::Undo(void)
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

	ASSERT(m_itemID && pOutliner->m_ItemMap.end() == pOutliner->m_ItemMap.find(m_itemID));
	ASSERT(!m_parentID || pOutliner->m_ItemMap.end() != pOutliner->m_ItemMap.find(m_parentID));
	ASSERT(!m_beforeID || pOutliner->m_ItemMap.end() != pOutliner->m_ItemMap.find(m_beforeID));

	HTREEITEM hParent = m_parentID ? pOutliner->m_ItemMap[m_parentID] : TVI_ROOT;
	HTREEITEM hBefore = m_beforeID ? pOutliner->m_ItemMap[m_beforeID] : TVI_FIRST;
	pOutliner->InsertItem(m_itemID, m_strItem, m_node, hParent, hBefore);
}

void CDeleteTreeNodeStep::Do(void)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	ASSERT(m_itemID && pOutliner->m_ItemMap.end() != pOutliner->m_ItemMap.find(m_itemID));

	HTREEITEM hItem = pOutliner->m_ItemMap[m_itemID];
	m_strItem = pOutliner->m_TreeCtrl.GetItemText(hItem);
	m_node = boost::dynamic_pointer_cast<MeshTreeNode>(pOutliner->GetItemNode(hItem));
	ASSERT(m_node);

	HTREEITEM hParent = pOutliner->m_TreeCtrl.GetParentItem(hItem);
	m_parentID = hParent ? pOutliner->GetItemId(hParent) : 0;

	HTREEITEM hBefore = pOutliner->m_TreeCtrl.GetPrevSiblingItem(hItem);
	m_beforeID = hBefore ? pOutliner->GetItemId(hBefore) : 0;

	pOutliner->m_TreeCtrl.DeleteItem(hItem);
}

void CHistoryMgr::Do(void)
{
	ASSERT(m_nStep < (int)size() - 1);

	operator[](++m_nStep)->Do();
}

void CHistoryMgr::Undo(void)
{
	ASSERT(m_nStep >= 0);

	operator[](m_nStep--)->Undo();
}

void CHistoryMgr::ClearAllHistory(void)
{
	clear();

	m_nStep = -1;
}

void CHistoryMgr::AddHistory(CHistoryPtr hist)
{
	if(!empty())
		erase(begin() + (m_nStep + 1), end());

	push_back(hist);
}

void CHistoryMgr::AddTreeNode(LPCTSTR lpszItem, TreeNodeBasePtr node)
{
	m_guid++;

	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	HTREEITEM hBefore = pOutliner->m_TreeCtrl.CalcLastChildItem(TVI_ROOT);
	UINT beforeID = hBefore ? pOutliner->GetItemId(hBefore) : 0;

	CHistoryPtr hist(new CHistory());
	hist->push_back(std::make_pair(
		CStepBasePtr(new CAddTreeNodeStep(m_guid, lpszItem, node, 0, beforeID)),
		CStepBasePtr(new CDeleteTreeNodeStep(m_guid))));

	AddHistory(hist);

	Do();
}

void CHistoryMgr::DeleteTreeNode(HTREEITEM hItem)
{
	COutlinerView * pOutliner = COutlinerView::getSingletonPtr();
	ASSERT(pOutliner);

	UINT itemID = pOutliner->GetItemId(hItem);
	CString szItem = pOutliner->m_TreeCtrl.GetItemText(hItem);
	boost::shared_ptr<CDeleteTreeNodeStep> del_step(new CDeleteTreeNodeStep(itemID));
	del_step->Do();

	CHistoryPtr hist(new CHistory());
	hist->push_back(std::make_pair(
		del_step,
		CStepBasePtr(new CAddTreeNodeStep(itemID, szItem, del_step->m_node, del_step->m_parentID, del_step->m_beforeID))));

	AddHistory(hist);

	m_nStep++;
}

void CHistoryMgr::AddMeshTreeNode(LPCTSTR lpszMesh)
{
	MeshTreeNodePtr node(new MeshTreeNode);
	if(node->LoadFromMesh(lpszMesh))
	{
		AddTreeNode(PathFindFileName(lpszMesh), node);
	}
}

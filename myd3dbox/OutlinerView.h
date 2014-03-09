#pragma once

#include "TreeNode.h"

class COutlinerTreeCtrl : public CTreeCtrl
{
	DECLARE_MESSAGE_MAP()

	BOOL m_bDrag;

	HTREEITEM m_hDragItem;

	CPoint m_LastDragPos;

	enum DropType
	{
		DropTypeNone = 0,
		DropTypeFront,
		DropTypeChild,
		DropTypeBack,
	};

	DropType m_DragDropType;

public:
	COutlinerTreeCtrl(void)
		: m_bDrag(FALSE)
		, m_hDragItem(NULL)
		, m_DragDropType(DropTypeNone)
	{
	}

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	int CalcChildCount(HTREEITEM hItem);

	HTREEITEM CalcLastChildItem(HTREEITEM hItem);

	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class COutlinerView
	: public CDockablePane
	, public my::SingleInstance<COutlinerView>
{
public:
	COutlinerTreeCtrl m_TreeCtrl;

	CMFCToolBar m_wndToolBar;

	typedef boost::unordered_map<UINT, HTREEITEM> TreeItemMap;

	TreeItemMap m_ItemMap;

	typedef std::pair<UINT, TreeNodeBasePtr> ItemDataType;

public:
	COutlinerView(void)
	{
	}

	virtual ~COutlinerView(void)
	{
		ASSERT(m_ItemMap.empty());
	}

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void AdjustLayout(void);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnUserDeleting(NMHDR *pNMHDR, LRESULT *pResult);

	HTREEITEM InsertItem(UINT id, const std::basic_string<TCHAR> & strItem, TreeNodeBasePtr node, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

	UINT GetItemId(HTREEITEM hItem);

	TreeNodeBasePtr GetItemNode(HTREEITEM hItem);

	TreeNodeBasePtr GetSelectedNode(void);

	void DrawItemNode(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, HTREEITEM hItem, const my::Matrix4 & World);

	bool RayTestItemNode(const std::pair<my::Vector3, my::Vector3> & ray, HTREEITEM hItem, const my::Matrix4 & World);

	void SerializeSubItemRecursively(CArchive & ar, HTREEITEM hParent);

	void Serialize(CArchive & ar);

	DECLARE_MESSAGE_MAP()
};

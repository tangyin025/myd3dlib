#pragma once

class TreeNodeBase
{
public:
	TreeNodeBase(void)
	{
	}

	virtual ~TreeNodeBase(void)
	{
	}
};

typedef boost::shared_ptr<TreeNodeBase> TreeNodeBasePtr;

class COutlinerTreeCtrl
	: public CTreeCtrl
{
public:
	COutlinerTreeCtrl(void)
		: m_bDrag(FALSE)
		, m_hDragItem(NULL)
		, m_DragDropType(DropTypeNone)
	{
	}

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

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class COutlinerView
	: public CDockablePane
	, public my::SingleInstance<COutlinerView>
{
public:
	COutlinerView(void)
	{
	}

	virtual ~COutlinerView(void)
	{
	}

	DECLARE_MESSAGE_MAP()

	COutlinerTreeCtrl m_TreeCtrl;

	CMFCToolBar m_wndToolBar;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void AdjustLayout(void);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnUserDeleting(NMHDR *pNMHDR, LRESULT *pResult);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnOutlinerCreatemesh();
};

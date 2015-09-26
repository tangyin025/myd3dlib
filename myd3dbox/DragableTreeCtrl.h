
#pragma once

class CDragableTreeCtrl : public CTreeCtrl
{
	DECLARE_MESSAGE_MAP()
public:
	static const int TVN_DRAGCHANGED = (TVN_LAST + 1);

	static const int TVN_USERDELETING = (TVN_LAST + 2);

	struct NMTREEVIEWDRAG
	{
		NMHDR hdr;
		HTREEITEM hDragItem;
		HTREEITEM hDragTagParent;
		HTREEITEM hDragTagFront;
	};

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
	CDragableTreeCtrl(void)
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

	BOOL FindTreeChildItem(HTREEITEM hParent, HTREEITEM hChild);

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

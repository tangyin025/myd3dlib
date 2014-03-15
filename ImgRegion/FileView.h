
#pragma once

class CImgRegionDoc;

class CImgRegionTreeCtrl : public CTreeCtrl
{
protected:
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

	enum TimerEvent
	{
		TimerEventUnknown = 0,
		TimerEventScrollUp,
		TimerEventScrollDown,
	};

	DECLARE_DYNAMIC(CImgRegionTreeCtrl)

public:
	CImgRegionTreeCtrl(void);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

public:
	BOOL FindTreeChildItem(HTREEITEM hParent, HTREEITEM hChild);

	BOOL CanItemMove(HTREEITEM hParent, HTREEITEM hInsertAfter, HTREEITEM hOtherItem);

	int CalcChildCount(HTREEITEM hItem);

	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

class CFileView : public CDockablePane
{
public:
	CImgRegionDoc * m_pDoc;

	BOOL m_bIsLayoutInvalid;

public:
	CFileView(void);

	void AdjustLayout(void);

	void OnChangeVisualStyle(void);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTvnSelchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnTvnDragchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	void InvalidLayout(void)
	{
		m_bIsLayoutInvalid = TRUE;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


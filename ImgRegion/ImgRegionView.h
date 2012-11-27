#pragma once

#include "ImageView.h"
#include "ImgRegionDoc.h"

class CImgRegionView : public CImageView
{
public:
	DECLARE_DYNCREATE(CImgRegionView)

	enum CursorType
	{
		CursorTypeArrow = 0,
		CursorTypeCross,
		CursorTypeCount
	};

	HCURSOR m_hCursor[CursorTypeCount];

	DWORD m_dwCurrCursor;

	BOOL m_bPrepareDrag;

	BOOL m_bDrag;

	CPoint m_bDragStartPos;

	CPoint m_bDragStartScrollPos;

	DWORD m_dwZoomIdx;

public:
	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	virtual void OnDraw(CDC * pDC);

	void DrawRegionNode(CDC * pDC, const CRegionNode * node, const CPoint & ptOff = CPoint(0,0));

	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void OnInitialUpdate();

	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

	afx_msg void OnZoomIn(void);

	afx_msg void OnUpdateZoomIn(CCmdUI *pCmdUI);

	afx_msg void OnZoomOut(void);

	afx_msg void OnUpdateZoomOut(CCmdUI *pCmdUI);

	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

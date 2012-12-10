#pragma once

#include "ImageView.h"
#include "ImgRegionDoc.h"
#include <set>

static const float ZoomTable[] = {
	32, 16, 12, 8, 7, 6, 5, 4, 3, 2, 1, 2.0f/3, 1.0f/2, 1.0f/3, 1.0f/4, 1.0f/6, 1.0f/8, 1.0f/12, 1.0f/16, 1.0f/20, 1.0f/25, 3.0f/100, 2.0f/100, 1.5f/100, 1.0f/100, 0.7f/100 };

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

	int m_nCurrCursor;

	CSize m_ImageSizeTable[_countof(ZoomTable)];

	int m_nCurrImageSize;

	enum DragState
	{
		DragStateNone = 0,
		DragStateScroll,
		DragStateControl,
	};

	DragState m_DragState;

	CPoint m_DragPos;

	CPoint m_DragRegLocal;

	CSize m_DragRegSize;

	CPoint m_DragScrollPos;

	enum HandleType
	{
		HandleTypeNone = 0,
		HandleTypeLeftTop,
		HandleTypeCenterTop,
		HandleTypeRightTop,
		HandleTypeLeftMiddle,
		HandleTypeRightMiddle,
		HandleTypeLeftBottom,
		HandleTypeCenterBottom,
		HandleTypeRightBottom,
		HandleTypeCount
	};

	int m_nSelectedHandle;

public:
	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	virtual void OnDraw(CDC * pDC);

	void DrawRegionNode(Gdiplus::Graphics & grap, HTREEITEM hItem, const CPoint & ptOff = CPoint(0,0));

	void DrawRegionImage(Gdiplus::Graphics & grap, Gdiplus::Image * img, const CRect & dstRect, const Vector4i & border, const Gdiplus::Color & color);

	void DrawRectHandle(Gdiplus::Graphics & grap, const CRect & rectHandle);

	void DrawSmallHandle(Gdiplus::Graphics & grap, const CPoint & ptHandle, BOOL bSelected);

	BOOL CheckSmallHandle(const CPoint & ptHandle, const CPoint & ptMouse);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void OnInitialUpdate();

	void UpdateImageSizeTable(const CSize & sizeRoot);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnZoomIn();

	afx_msg void OnUpdateZoomIn(CCmdUI *pCmdUI);

	afx_msg void OnZoomOut();

	afx_msg void OnUpdateZoomOut(CCmdUI *pCmdUI);

	void ZoomImage(int ImageSizeIdx, const CPoint & ptLook, BOOL bRedraw = TRUE);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
};

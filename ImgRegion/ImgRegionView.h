#pragma once

#include "ImageView.h"
#include "ImgRegionDoc.h"
#include <set>

class CImgRegionView : public CImageView
{
public:
	DECLARE_DYNCREATE(CImgRegionView)

	enum CursorType
	{
		CursorTypeArrow = 0,
		CursorTypeMove,
		CursorTypeCount
	};

	HCURSOR m_hCursor[CursorTypeCount];

	int m_nCurrCursor;

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

	CPoint m_DragRegTextOff;

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
		HandleTypeLeftTopText,
		HandleTypeCount
	};

	int m_nSelectedHandle;

	CMenu m_ContextMenu;

public:
	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	afx_msg void OnPaint();

	virtual void OnDraw(CDC * pDC);

	void Draw(Gdiplus::Graphics & grap);

	static void DrawRegionDoc(Gdiplus::Graphics & grap, CImgRegionDoc * pDoc);

	static void DrawRegionDocNode(Gdiplus::Graphics & grap, CTreeCtrl * pTreeCtrl, HTREEITEM hItem, const CPoint & ptOff = CPoint(0,0));

	static void DrawRegionDocImage(Gdiplus::Graphics & grap, Gdiplus::Image * img, const CRect & dstRect, const Vector4i & border, const Gdiplus::Color & color);

	static void DrawControlHandle(Gdiplus::Graphics & grap, const CPoint & ptHandle, const Gdiplus::Color & clrHandle, BOOL bSelected);

	static void DrawTextHandle(Gdiplus::Graphics & grap, const CPoint & ptHandle, const Gdiplus::Color & clrHandle, BOOL bSelected);

	BOOL CheckControlHandle(const CPoint & ptHandle, const CPoint & ptMouse);

	BOOL CheckTextHandle(const CPoint & ptHandle, const CPoint & ptMouse);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void OnInitialUpdate();

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

	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	void OnMenuCommand(UINT nPos, CMenu* pMenu);

	static void InsertPointedRegionNodeToMenuItem(CMenu * pMenu, CTreeCtrl * pTreeCtrl, HTREEITEM hItem, const CPoint & ptLocal);
};

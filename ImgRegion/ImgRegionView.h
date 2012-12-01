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

public:
	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	virtual void OnDraw(CDC * pDC);

	void DrawRegionNode(CDC * pDC, const CRegionNode * node, const CPoint & ptOff = CPoint(0,0));

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
};

#pragma once

#include <myMath.h>

class CImageView : public CView
{
public:
	DECLARE_DYNAMIC(CImageView)

	CImageView(void);

	DECLARE_MESSAGE_MAP()

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	void SetScrollSizes(const CSize & sizeTotal, BOOL bRedraw = TRUE, const CPoint & scrollPos = CPoint(0,0));

	static void PrepareDC(CDC * pDC, const CRect & rectImageLog, const CRect & rectImageDev);

	static my::Vector2 MapPoint(const my::Vector2 & point, const CRect & rectImageSrc, const CRect & rectImageDst);
};

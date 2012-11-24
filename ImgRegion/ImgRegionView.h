#pragma once

#include "ImgRegionDoc.h"

class CImgRegionView : public CScrollView
{
public:
	DECLARE_DYNCREATE(CImgRegionView)

	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	virtual void OnDraw(CDC * pDC);

	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#pragma once

#include "ImgRegionDoc.h"

class CImageView : public CView
{
public:
	CImageView(void);

	DECLARE_MESSAGE_MAP()

	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
};

class CImgRegionView : public CImageView
{
public:
	DECLARE_DYNCREATE(CImgRegionView)

	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	virtual void OnDraw(CDC * pDC);

	DECLARE_MESSAGE_MAP()

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void OnInitialUpdate();

	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
};

#pragma once

#include "ImageView.h"
#include "ImgRegionDoc.h"

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

	afx_msg void OnZoomIn(void);

	afx_msg void OnUpdateZoomIn(CCmdUI *pCmdUI);

	afx_msg void OnZoomOut(void);

	afx_msg void OnUpdateZoomOut(CCmdUI *pCmdUI);
};

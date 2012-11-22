#pragma once

#include <afxwin.h>
#include "ImgRegionDoc.h"

class CImgRegionView : public CView
{
public:
	DECLARE_DYNCREATE(CImgRegionView)

	CImgRegionView(void);

	CImgRegionDoc * GetDocument() const;

	virtual void OnDraw(CDC * pDC);

	DECLARE_MESSAGE_MAP()
};

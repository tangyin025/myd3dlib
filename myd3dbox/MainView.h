#pragma once

#include "MainDoc.h"

class CMainView : public CView
{
public:
	CMainView(void);

	DECLARE_DYNCREATE(CMainView)

	CMainDoc * GetDocument(void) const;

	virtual void OnDraw(CDC* pDC) {}
};

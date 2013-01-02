#pragma once

#include "MainDoc.h"

class CMainView : public CView
{
public:
	CMainView(void);

	DECLARE_DYNCREATE(CMainView)

	CMainDoc * GetDocument(void) const;

	virtual void OnDraw(CDC* pDC) {}

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;

	my::Surface m_DepthStencil;

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();

	afx_msg void OnPaint();

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

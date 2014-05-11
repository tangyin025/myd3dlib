
// ChildView.h : interface of the CChildView class
//


#pragma once


class CChildView : public CView
{
protected: // create from serialization only
	CChildView();
	DECLARE_DYNCREATE(CChildView)

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;
	D3DSURFACE_DESC m_SwapChainBufferDesc;
	my::Surface m_DepthStencil;

	BOOL ResetD3DSwapChain(void);
	void OnDeviceLost(void);
	void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

// Attributes
public:
	CMainDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CChildView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in ChildView.cpp
inline CMainDoc* CChildView::GetDocument() const
   { return reinterpret_cast<CMainDoc*>(m_pDocument); }
#endif


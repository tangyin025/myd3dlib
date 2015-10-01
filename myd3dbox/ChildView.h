
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "../demo2_3/Component/RenderPipeline.h"
#include "PivotController.h"

class CMainDoc;

class CChildView
	: public CView
	, public my::DialogMgr
	, public RenderPipeline::IRenderContext
	, public my::DrawHelper
{
protected: // create from serialization only
	CChildView();
	DECLARE_DYNCREATE(CChildView)

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

	enum CameraType
	{
		CameraTypeUnknown,
		CameraTypePerspective,
		CameraTypeFront,
		CameraTypeSide,
		CameraTypeTop
	};

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;
	my::SurfacePtr m_SwapChainBuffer;
	D3DSURFACE_DESC m_SwapChainBufferDesc;
	my::SurfacePtr m_DepthStencil;
	my::Texture2DPtr m_NormalRT;
	my::Texture2DPtr m_PositionRT;
	my::Texture2DPtr m_LightRT;
	my::Texture2DPtr m_OpaqueRT;
	my::Texture2DPtr m_DownFilterRT[2];
	typedef std::map<int, boost::array<wchar_t, 256> > ScrInfoType;
	ScrInfoType m_ScrInfos;
	PivotController m_Pivot;
	typedef std::multimap<float, Component *> SelCmpMap;
	SelCmpMap m_SelCmpMap;
	float m_CameraDiagonal;
	CameraType m_CameraType;
	LARGE_INTEGER m_qwTime[2];

	BOOL ResetD3DSwapChain(void);
	BOOL ResetRenderTargets(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);
	virtual IDirect3DSurface9 * GetScreenSurface(void);
	virtual IDirect3DSurface9 * GetScreenDepthStencilSurface(void);
	virtual IDirect3DSurface9 * GetNormalSurface(void);
	virtual my::Texture2D * GetNormalTexture(void);
	virtual IDirect3DSurface9 * GetPositionSurface(void);
	virtual my::Texture2D * GetPositionTexture(void);
	virtual IDirect3DSurface9 * GetLightSurface(void);
	virtual my::Texture2D * GetLightTexture(void);
	virtual IDirect3DSurface9 * GetOpaqueSurface(void);
	virtual my::Texture2D * GetOpaqueTexture(void);
	virtual IDirect3DSurface9 * GetDownFilterSurface(unsigned int i);
	virtual my::Texture2D * GetDownFilterTexture(unsigned int i);
	virtual void OnQueryComponent(const my::Frustum & frustum, unsigned int PassMask);
	bool OnRayTest(const my::Ray & ray);
	bool OnFrustumTest(const my::Frustum & ftm);
	void RenderSelectedObject(IDirect3DDevice9 * pd3dDevice);
	void StartPerformanceCount(void);
	double EndPerformanceCount(void);

	void OnOutlinerSelectChanged(void);

// Implementation
public:
	virtual ~CChildView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HRESULT hr;

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCameratypePerspective();
	afx_msg void OnUpdateCameratypePerspective(CCmdUI *pCmdUI);
	afx_msg void OnCameratypeFront();
	afx_msg void OnUpdateCameratypeFront(CCmdUI *pCmdUI);
	afx_msg void OnCameratypeSide();
	afx_msg void OnUpdateCameratypeSide(CCmdUI *pCmdUI);
	afx_msg void OnCameratypeTop();
	afx_msg void OnUpdateCameratypeTop(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in ChildView.cpp
inline CMainDoc* CChildView::GetDocument() const
   { return reinterpret_cast<CMainDoc*>(m_pDocument); }
#endif


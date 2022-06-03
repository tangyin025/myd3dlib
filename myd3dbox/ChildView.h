
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "RenderPipeline.h"
#include "Actor.h"
#include "Terrain.h"
#include "StaticEmitter.h"

class CMainDoc;

class CChildView
	: public CView
	, public RenderPipeline::IRenderContext
	, public my::DrawHelper
{
protected: // create from serialization only
	CChildView();
	DECLARE_DYNCREATE(CChildView)

// Attributes
public:
	//CMainDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	float m_PivotScale;
	float m_CameraDiagonal;
	float m_ViewedDist;
	BOOL m_bShowGrid;
	BOOL m_bShowCmpHandle;
	BOOL m_bShowNavigation;
	BOOL m_bCopyActors;
	boost::shared_ptr<TerrainStream> m_PaintTerrainCaptured;
	boost::shared_ptr<StaticEmitterStream> m_PaintEmitterCaptured;
protected:

	my::PerspectiveCamera m_UICamera;
	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;
	my::SurfacePtr m_SwapChainBuffer;
	D3DSURFACE_DESC m_SwapChainBufferDesc;
	my::SurfacePtr m_DepthStencil;
	typedef std::map<int, boost::array<wchar_t, 256> > ScrInfoMap;
	ScrInfoMap m_ScrInfo;
	LARGE_INTEGER m_qwTime[2];
	Component * m_raycmp;
	CPoint m_raychunkid;
	int m_rayinstid;

	BOOL ResetD3DSwapChain(void);
	BOOL ResetRenderTargets(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);
	virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
	void RenderSelectedActor(IDirect3DDevice9 * pd3dDevice, Actor * actor, D3DCOLOR color);
	void RenderSelectedComponent(IDirect3DDevice9 * pd3dDevice, Component * cmp, D3DCOLOR color);
	void RenderSelectedControl(IDirect3DDevice9 * pd3dDevice, my::Control * ctl, D3DCOLOR color, bool subhandle);
	void StartPerformanceCount(void);
	double EndPerformanceCount(void);
	static my::Matrix4 GetParticleTransform(DWORD EmitterFaceType, const my::Emitter::Particle & particle, const my::Matrix4 & World, const my::Matrix4 & View);
	bool OverlapTestFrustumAndActor(const my::Frustum & frustum, Actor * actor);
	bool OverlapTestFrustumAndComponent(const my::Frustum & frustum, const my::Frustum & local_ftm, Component * cmp);
	bool OverlapTestFrustumAndParticles(const my::Frustum & frustum, const my::Frustum & local_ftm, EmitterComponent * emitter, const my::Emitter::Particle * part_start, int part_num);
	my::RayResult OverlapTestRayAndActor(const my::Ray & ray, Actor * actor);
	my::RayResult OverlapTestRayAndComponent(const my::Ray & ray, const my::Ray & local_ray, Component * cmp, CPoint & raychunkid, int & rayinstid);
	my::RayResult OverlapTestRayAndParticles(const my::Ray & ray, const my::Ray & local_ray, EmitterComponent * emitter, const my::Emitter::Particle * part_start, int part_num, int & part_id);
	void OnSelectionChanged(my::EventArg * arg);
	void OnSelectionPlaying(my::EventArg * arg);
	void OnPivotModeChanged(my::EventArg * arg);
	void OnCmpAttriChanged(my::EventArg * arg);
	void OnCameraPropChanged(my::EventArg * arg);
	void DrawTerrainHeightFieldHandle(Terrain* terrain);

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
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnShowGrid();
	afx_msg void OnUpdateShowGrid(CCmdUI *pCmdUI);
	afx_msg void OnShowCmphandle();
	afx_msg void OnUpdateShowCmphandle(CCmdUI *pCmdUI);
	afx_msg void OnRendermodeWireframe();
	afx_msg void OnUpdateRendermodeWireframe(CCmdUI *pCmdUI);
	afx_msg void OnShowCollisiondebug();
	afx_msg void OnUpdateShowCollisiondebug(CCmdUI *pCmdUI);
	afx_msg void OnRendermodeDepthoffield();
	afx_msg void OnUpdateRendermodeDepthoffield(CCmdUI *pCmdUI);
	afx_msg void OnRendermodeFxaa();
	afx_msg void OnUpdateRendermodeFxaa(CCmdUI *pCmdUI);
	afx_msg void OnRendermodeSsao();
	afx_msg void OnUpdateRendermodeSsao(CCmdUI *pCmdUI);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg void OnShowNavigation();
	afx_msg void OnUpdateShowNavigation(CCmdUI *pCmdUI);
	void OnPaintTerrainHeightField(const my::Ray& ray, TerrainStream & tstr);
	void OnPaintTerrainColor(const my::Ray& ray, TerrainStream& tstr);
	void OnPaintEmitterInstance(const my::Ray& ray, TerrainStream& tstr, StaticEmitterStream& estr);
};

#ifndef _DEBUG  // debug version in ChildView.cpp
//inline CMainDoc* CChildView::GetDocument() const
//   { return reinterpret_cast<CMainDoc*>(m_pDocument); }
#endif


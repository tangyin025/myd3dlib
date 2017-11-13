
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "Component/RenderPipeline.h"
#include "Component/Actor.h"
#include "Component/Terrain.h"
#include "EventDefine.h"

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
	//CMainDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	enum CameraType
	{
		CameraTypeUnknown,
		CameraTypePerspective,
		CameraTypeFront,
		CameraTypeSide,
		CameraTypeTop
	};
	float m_PivotScale;
	float m_CameraDiagonal;
	CameraType m_CameraType;
	BOOL m_bShowGrid;
	BOOL m_bShowCmpHandle;
	BOOL m_bCopyActors;
protected:

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;
	my::SurfacePtr m_SwapChainBuffer;
	D3DSURFACE_DESC m_SwapChainBufferDesc;
	my::SurfacePtr m_DepthStencil;
	typedef std::map<int, boost::array<wchar_t, 256> > ScrInfoType;
	ScrInfoType m_ScrInfos;
	LARGE_INTEGER m_qwTime[2];

	typedef std::map<Actor *, my::Matrix4> ActorWorldMap;
	ActorWorldMap m_selactorwlds;

	BOOL ResetD3DSwapChain(void);
	BOOL ResetRenderTargets(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);
	virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
	void RenderSelectedActor(IDirect3DDevice9 * pd3dDevice, Actor * actor);
	void RenderSelectedComponent(IDirect3DDevice9 * pd3dDevice, Component * cmp);
	void StartPerformanceCount(void);
	double EndPerformanceCount(void);
	bool OverlapTestFrustumAndActor(const my::Frustum & frustum, Actor * actor);
	bool OverlapTestFrustumAndComponent(const my::Frustum & frustum, Component * cmp);
	bool OverlapTestFrustumAndMesh(
		const my::Frustum & frustum,
		void * pVertices,
		DWORD NumVerts,
		DWORD VertexStride,
		void * pIndices,
		bool bIndices16,
		DWORD NumFaces,
		const my::D3DVertexElementSet & VertexElems);
	my::RayResult OverlapTestRayAndActor(const my::Ray & ray, Actor * actor);
	my::RayResult OverlapTestRayAndComponent(const my::Ray & ray, Component * cmp);
	my::RayResult OverlapTestRayAndMesh(
		const my::Ray & ray,
		void * pVertices,
		DWORD NumVerts,
		DWORD VertexStride,
		void * pIndices,
		bool bIndices16,
		DWORD NumFaces,
		const my::D3DVertexElementSet & VertexElems);
	my::RayResult OverlapTestRayAndTerrainChunk(
		const my::Ray & ray,
		Terrain * terrain,
		TerrainChunk * chunk,
		void * pVertices,
		DWORD NumVerts,
		DWORD VertexStride,
		void * pIndices,
		DWORD NumFaces);
	void OnSelectionChanged(EventArgs * arg);
	void OnSelectionPlaying(EventArgs * arg);
	void OnPivotModeChanged(EventArgs * arg);
	void OnCmpAttriChanged(EventArgs * arg);
	void PostCameraViewChanged(void);

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
};

#ifndef _DEBUG  // debug version in ChildView.cpp
//inline CMainDoc* CChildView::GetDocument() const
//   { return reinterpret_cast<CMainDoc*>(m_pDocument); }
#endif


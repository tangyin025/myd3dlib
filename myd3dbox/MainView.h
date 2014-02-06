#pragma once

#include "MainDoc.h"
#include "PivotController.h"

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	UINT m_Passes;

public:
	EffectUIRender(IDirect3DDevice9 * pd3dDevice, my::EffectPtr effect)
		: UIRender(pd3dDevice)
		, m_UIEffect(effect)
		, m_Passes(0)
	{
		_ASSERT(m_UIEffect);
	}

	virtual void Begin(void);

	virtual void End(void);

	virtual void SetWorld(const my::Matrix4 & World);

	virtual void SetViewProj(const my::Matrix4 & ViewProj);

	virtual void SetTexture(const my::BaseTexturePtr & Texture);

	virtual void DrawVertexList(void);
};

class CMainView
	: public CView
	, public my::SingleInstance<CMainView>
	, public my::DialogMgr
{
public:
	enum DragMode
	{
		DragModeCameraNone = 0,
		DragModeCameraRotate,
		DragModeCameraTrack,
		DragModeCameraZoom,
		DragModePivotMove,
	};

	DragMode m_DragMode;

	CRectTracker m_Tracker;

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;

	D3DSURFACE_DESC m_SwapChainBufferDesc;

	my::Surface m_DepthStencil;

	my::UIRenderPtr m_UIRender;

	my::BaseTexturePtr m_WhiteTex;

	my::FontPtr m_Font;

	my::ModelViewerCamera m_Camera;

	PivotController m_PivotController;

public:
	CMainView(void)
		: m_DragMode(DragModeCameraNone)
		, m_Camera()
	{
	}

	DECLARE_DYNCREATE(CMainView)

	DECLARE_MESSAGE_MAP()

	void DrawTextAtWorld(const my::Vector3 & pos, LPCWSTR lpszText, D3DCOLOR Color, my::Font::Align align = my::Font::AlignCenterMiddle);

	BOOL ResetD3DSwapChain(void);

	void OnDeviceLost(void);

	void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	CMainDoc * GetDocument(void) const;

	virtual void OnDraw(CDC* pDC) {}

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();

	afx_msg void OnPaint();

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnTransformMove();

	afx_msg void OnUpdateTransformMove(CCmdUI *pCmdUI);

	afx_msg void OnTransformRotate();

	afx_msg void OnUpdateTransformRotate(CCmdUI *pCmdUI);
};

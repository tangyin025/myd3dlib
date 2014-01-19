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
	CMainView(void)
		: m_bAltDown(FALSE)
		, m_bEatAltUp(FALSE)
		, m_DragCameraMode(DragCameraNone)
		, m_Camera()
		, m_RenderMode(RenderModeDefault)
	{
	}

	DECLARE_DYNCREATE(CMainView)

	void DrawTextAtWorld(const my::Vector3 & pos, LPCWSTR lpszText, D3DCOLOR Color, my::Font::Align align = my::Font::AlignCenterMiddle);

	CMainDoc * GetDocument(void) const;

	virtual void OnDraw(CDC* pDC) {}

	int m_DebugDrawModes;

	BOOL m_bAltDown;

	BOOL m_bEatAltUp;

	enum DragCameraMode
	{
		DragCameraNone = 0,
		DragCameraRotate,
		DragCameraTrack,
		DragCameraZoom,
	};

	DragCameraMode m_DragCameraMode;

	CRectTracker m_Tracker;

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;

	my::Surface m_DepthStencil;

	my::UIRenderPtr m_UIRender;

	my::BaseTexturePtr m_WhiteTex;

	my::FontPtr m_Font;

	my::ModelViewerCamera m_Camera;

	enum RenderMode
	{
		RenderModeDefault = 0,
		RenderModeWire,
		RenderModePhysics,
	};

	RenderMode m_RenderMode;

	PivotController m_PivotController;

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();

	afx_msg void OnPaint();

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	BOOL ResetD3DSwapChain(void);

	void OnDeviceLost(void);

	void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

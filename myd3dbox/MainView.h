#pragma once

#include "MainDoc.h"

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

	virtual void SetTexture(IDirect3DBaseTexture9 * pTexture);

	virtual void SetTransform(const my::Matrix4 & World, const my::Matrix4 & View, const my::Matrix4 & Proj);

	virtual void DrawVertexList(void);
};

class CMainView
	: public CView
	, public my::SingleInstance<CMainView>
	, public my::DrawHelper
{
public:
	CMainView(void);

	DECLARE_DYNCREATE(CMainView)

	CMainDoc * GetDocument(void) const;

	virtual void OnDraw(CDC* pDC) {}

	CComPtr<IDirect3DSwapChain9> m_d3dSwapChain;

	my::Surface m_DepthStencil;

	my::FontPtr m_Font;

	my::TexturePtr m_WhiteTex;

	my::UIRenderPtr m_UIRender;

	my::ModelViewerCamera m_Camera;

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

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();

	afx_msg void OnPaint();

	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	HRESULT OnDeviceReset(void);

	void OnDeviceLost(void);

	void OnFrameMove(
		double fTime,
		float fElapsedTime);

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
};

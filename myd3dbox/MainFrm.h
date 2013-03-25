#pragma once

#include "OutlinerView.h"
#include "PropertiesWnd.h"

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

class CMainFrame
	: public CFrameWndEx
	, public my::SingleInstance<CMainFrame>
	, public my::ResourceMgr
{
public:
	CMainFrame(void);

	DECLARE_DYNCREATE(CMainFrame)

	UINT  m_nAppLook;

	CMFCMenuBar m_wndMenuBar;

	CMFCToolBar m_wndToolBar;

	CMFCStatusBar m_wndStatusBar;

	COutlinerView m_wndOutliner;

	CPropertiesWnd m_wndProperties;

	D3DPRESENT_PARAMETERS m_d3dpp;

	CComPtr<IDirect3DDevice9> m_d3dDevice;

	my::UIRenderPtr m_UIRender;

	my::TexturePtr m_WhiteTex;

	my::FontPtr m_Font;

	my::EffectPtr m_SimpleSample;

	bool m_DeviceObjectsCreated;

	bool m_DeviceObjectsReset;

	DECLARE_MESSAGE_MAP()

	IDirect3DDevice9 * GetD3D9Device(void)
	{
		return m_d3dDevice;
	}

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnViewCustomize();

	afx_msg void OnApplicationLook(UINT id);

	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	afx_msg void OnDestroy();
public:
	HRESULT ResetD3DDevice(void);

	HRESULT OnDeviceReset(void);

	void OnDeviceLost(void);
};

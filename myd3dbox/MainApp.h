
// myd3dbox.h : main header file for the myd3dbox application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "Component/ActorResourceMgr.h"


// CMainApp:
// See myd3dbox.cpp for the implementation of this class
//

class CMainApp : public CWinAppEx
	, public my::D3DContext
	, public my::Clock
	, public ActorResourceMgr
	, public RenderPipeline
{
public:
	CMainApp();

	HRESULT hr;

	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	my::EffectPtr m_SimpleSample;

	typedef boost::tuple<RenderPipeline::MeshType, RenderPipeline::DrawStage, bool, const Material *> ShaderCacheKey;

	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	BOOL CreateD3DDevice(HWND hWnd);

	BOOL ResetD3DDevice(void);

	void DestroyD3DDevice(void);

	void OnShaderLoaded(my::DeviceRelatedObjectBasePtr res, ShaderCacheKey key);

	my::Effect * QueryShader(RenderPipeline::MeshType mesh_type, RenderPipeline::DrawStage draw_stage, bool bInstance, const Material * material);

// Overrides
public:
	virtual BOOL InitInstance();

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
};

extern CMainApp theApp;

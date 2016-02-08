
// myd3dbox.h : main header file for the myd3dbox application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "Component/RenderPipeline.h"
#include "Component/LuaExtension.h"
#include "Component/PhysXContext.h"


// CMainApp:
// See myd3dbox.cpp for the implementation of this class
//

class CMainApp : public CWinAppEx
	, public my::D3DContext
	, public my::ResourceMgr
	, public LuaContext
	, public RenderPipeline
	, public PhysXContext
{
public:
	CMainApp();

	HRESULT hr;
	typedef boost::tuple<RenderPipeline::MeshType, bool, std::string> ShaderCacheKey;
	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;
	ShaderCacheMap m_ShaderCache;
	my::UIRenderPtr m_UIRender;
	my::FontPtr m_Font;

	BOOL CreateD3DDevice(HWND hWnd);
	BOOL ResetD3DDevice(void);
	void DestroyD3DDevice(void);

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
	virtual void OnResourceFailed(const std::string & error_str);
	virtual my::Effect * QueryShader(RenderPipeline::MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID);
	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);

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

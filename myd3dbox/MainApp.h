
// myd3dbox.h : main header file for the myd3dbox application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "RenderPipeline.h"
#include "PhysxContext.h"

// CMainApp:
// See myd3dbox.cpp for the implementation of this class
//

class CMainApp : public CWinAppEx
	, public my::D3DContext
	, public my::FontLibrary
	, public my::ResourceMgr
	, public RenderPipeline
	, public PhysxSdk
{
public:
	CMainApp();
	~CMainApp();

	HRESULT hr;
	my::UIRenderPtr m_UIRender;
	my::FontPtr m_Font;

	// cfg
	float default_fov;
	bool default_load_shader_cache;
	std::string default_font;
	int default_font_height;
	int default_font_face_index;
	std::string default_texture;
	std::string default_normal_texture;
	std::string default_specular_texture;
	std::string default_shader;
	std::string default_water_shader;
	std::string default_sky_texture[6];
	int max_editable_particle_count;
	D3DXHANDLE technique_RenderSceneColor;
	D3DXHANDLE handle_MeshColor;
	BOOL m_bNeedDraw;

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
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
};

extern CMainApp theApp;

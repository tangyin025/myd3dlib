
// myd3dbox.h : main header file for the myd3dbox application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "../demo2_3/Component/ComponentResMgr.h"


// CMainApp:
// See myd3dbox.cpp for the implementation of this class
//

class CMainApp : public CWinAppEx
	, public my::D3DContext
	, public ComponentResMgr
	, public my::Clock
{
public:
	CMainApp();

	my::UIRenderPtr m_UIRender;

	my::FontPtr m_Font;

	BOOL CreateD3DDevice(HWND hWnd);
	BOOL ResetD3DDevice(void);
	void DestroyD3DDevice(void);

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMainApp theApp;

#pragma once

#include "resource.h"

class CMainApp
	: public CWinAppEx
	, public my::D3DContext
	, public my::Clock
	, public my::ResourceMgr
{
public:
	CMainApp(void)
	{
	}

	virtual BOOL InitInstance(void);

	DECLARE_MESSAGE_MAP()

public:
	my::MaterialPtr m_DefaultMat;

	my::EffectPtr m_SimpleSample;

	virtual BOOL OnIdle(LONG lCount);

	BOOL CreateD3DDevice(HWND hWnd);

	BOOL ResetD3DDevice(void);

	void DestroyD3DDevice(void);

	virtual void OnResourceFailed(const std::basic_string<TCHAR> & error_str);
};

extern CMainApp theApp;

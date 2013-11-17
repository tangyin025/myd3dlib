#pragma once

#include "resource.h"

class CMainApp
	: public CWinAppEx
	, public my::D3DContext
	, public my::Clock
	, public my::AsynchronousResourceMgr
{
public:
	CMainApp(void)
	{
	}

	virtual BOOL InitInstance(void);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnIdle(LONG lCount);

	BOOL CreateD3DDevice(HWND hWnd);

	BOOL ResetD3DDevice(void);

	void DestroyD3DDevice(void);
};

extern CMainApp theApp;

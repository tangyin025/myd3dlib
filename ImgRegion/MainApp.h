#pragma once

#include "resource.h"

class CMainApp : public CWinAppEx
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);

	UINT  m_nAppLook;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	ULONG_PTR gdiplusToken;

	CArray<Gdiplus::FontFamily> fontFamilies;

	DECLARE_MESSAGE_MAP()

	virtual int ExitInstance();

	virtual BOOL OnIdle(LONG lCount);
};

extern CMainApp theApp;

#pragma once

#include <afxwinappex.h>

class CMainApp : public CWinAppEx
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);
};

extern CMainApp theApp;

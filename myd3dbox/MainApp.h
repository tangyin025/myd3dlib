#pragma once

#include "resource.h"

class CMainApp : public CWinAppEx
{
public:
	CMainApp(void);

	virtual BOOL InitInstance(void);

	DECLARE_MESSAGE_MAP()
};

extern CMainApp theApp;

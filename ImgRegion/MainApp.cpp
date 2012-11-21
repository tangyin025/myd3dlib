#include "MainApp.h"
#include "MainFrm.h"
#include "resource.h"

CMainApp theApp;

CMainApp::CMainApp(void)
{
}

BOOL CMainApp::InitInstance(void)
{
	CWinAppEx::InitInstance();

	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CMainFrame * pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

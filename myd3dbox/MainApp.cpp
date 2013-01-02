#include "StdAfx.h"
#include "MainApp.h"
#include "MainDoc.h"
#include "MainFrm.h"
#include "MainView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainApp theApp;

CMainApp::CMainApp(void)
{
}

BOOL CMainApp::InitInstance(void)
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	LoadStdProfileSettings(4);

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	LPDIRECT3D9 pd3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(NULL == pd3d9)
	{
		return FALSE;
	}
	m_d3d9.Attach(pd3d9);

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMainDoc),
		RUNTIME_CLASS(CMainFrame),
		RUNTIME_CLASS(CMainView));

	if (!pDocTemplate)
		return FALSE;

	AddDocTemplate(pDocTemplate);

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

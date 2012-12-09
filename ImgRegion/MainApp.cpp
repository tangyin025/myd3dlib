#include "stdafx.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "resource.h"
#include "ImgRegionDoc.h"
#include "ChildFrm.h"
#include "ImgRegionView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

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

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	CMultiDocTemplate * pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_ImgRegionTYPE,
		RUNTIME_CLASS(CImgRegionDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CImgRegionView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	CMainFrame * pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	CCommandLineInfo cmdInfo;
	//cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CMainApp::ExitInstance()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);

	return CWinAppEx::ExitInstance();
}

BOOL CMainApp::OnIdle(LONG lCount)
{
	((CMainFrame *)m_pMainWnd)->m_wndFileView.OnIdleUpdate();

	((CMainFrame *)m_pMainWnd)->m_wndProperties.OnIdleUpdate();

	return CWinAppEx::OnIdle(lCount);
}

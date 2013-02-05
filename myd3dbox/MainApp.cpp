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
	: m_dwFrames(0)
	, m_fLastTime(0)
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

	m_d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if(!m_d3d9)
	{
		return FALSE;
	}

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

BOOL CMainApp::OnIdle(LONG lCount)
{
	CWinAppEx::OnIdle(lCount);

	my::Clock::Update();

	m_dwFrames++;

	if(m_fAbsoluteTime - m_fLastTime > 1.0f)
	{
		m_fFPS = (float)(m_dwFrames / (m_fAbsoluteTime - m_fLastTime));
		m_fLastTime = m_fAbsoluteTime;
		m_dwFrames = 0;
	}

	CMainFrame::getSingleton().OnFrameMove(m_fAbsoluteTime, m_fElapsedTime);

	CMainFrame::getSingleton().OnFrameRender(m_fAbsoluteTime, m_fElapsedTime);

	return TRUE;
}

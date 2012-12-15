#include "stdafx.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "resource.h"
#include "ImgRegionDoc.h"
#include "ChildFrm.h"
#include "ImgRegionView.h"
//
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

class CAboutDlg : public CDialog
{
public:
	CAboutDlg()
		: CDialog(CAboutDlg::IDD)
	{
	}

	enum { IDD = IDD_ABOUTBOX };

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
	ON_COMMAND(ID_APP_ABOUT, &CMainApp::OnAppAbout)
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

	Gdiplus::InstalledFontCollection installedFontCollection;
	const int count = installedFontCollection.GetFamilyCount();
	fontFamilies.SetSize(count);
	int found;
	installedFontCollection.GetFamilies(count, fontFamilies.GetData(), &found);
	fontFamilies.SetSize(found);

	m_ImageMIME[L"jpg"] = _T("jpeg");
	m_ImageMIME[L"jpeg"] = _T("jpeg");
	m_ImageMIME[L"jpe"] = _T("jpeg");
	m_ImageMIME[L"bmp"] = _T("bmp");
	m_ImageMIME[L"rle"] = _T("bmp");
	m_ImageMIME[L"dib"] = _T("bmp");
	m_ImageMIME[L"png"] = _T("png");

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
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

const CString & CMainApp::GetMIME(const CString & strExt)
{
	return m_ImageMIME[std::wstring(strExt)];
}

ImagePtr CMainApp::GetImage(const CString & strImg)
{
	if(!strImg.IsEmpty())
	{
		ASSERT(!PathIsRelative(strImg));

		std::wstring key(strImg);
		ImagePtrMap::iterator img_iter = m_ImageMap.find(key);
		if(img_iter == m_ImageMap.end())
		{
			m_ImageMap[key] = ImagePtr(new Gdiplus::Image(strImg));
		}

		return m_ImageMap[key];
	}

	return ImagePtr();
}

FontPtr2 CMainApp::GetFont(const CString & strFamily, float fSize)
{
	if(!strFamily.IsEmpty())
	{
		CString strFont;
		strFont.Format(_T("%s, %f"), strFamily, fSize);

		std::wstring key(strFont);
		FontPtr2Map::iterator fnt_iter = m_FontMap.find(key);
		if(fnt_iter == m_FontMap.end())
		{
			m_FontMap[key] = FontPtr2(new Gdiplus::Font(strFamily, fSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint));
		}

		return m_FontMap[key];
	}

	return FontPtr2();
}

int CMainApp::ExitInstance()
{
	fontFamilies.RemoveAll();

	m_ImageMap.clear();

	m_FontMap.clear();

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return CWinAppEx::ExitInstance();
}

BOOL CMainApp::OnIdle(LONG lCount)
{
	((CMainFrame *)m_pMainWnd)->m_wndFileView.OnIdleUpdate();

	((CMainFrame *)m_pMainWnd)->m_wndProperties.OnIdleUpdate();

	return CWinAppEx::OnIdle(lCount);
}

void CMainApp::OnAppAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

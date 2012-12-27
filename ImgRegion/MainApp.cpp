#include "stdafx.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "resource.h"
#include "ImgRegionDoc.h"
#include "ChildFrm.h"
#include "ImgRegionView.h"
#include "Tga.h"
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

	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);

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
		ImagePtr ret;
		std::wstring key(strImg);
		ImagePtrMap::iterator img_iter = m_ImageMap.find(key);
		if(img_iter != m_ImageMap.end())
		{
			ret = img_iter->second.lock();
			if(ret)
				return ret;
		}

		LPCTSTR szExt = PathFindExtension(strImg);
		m_ImageMap[key] =
			ret = (0 == _tcsicmp(szExt, _T(".tga")) ? OpenTgaImage(strImg) : ImagePtr(new Gdiplus::Image(strImg)));
		return ret;
	}

	return ImagePtr();
}

ImagePtr CMainApp::OpenTgaImage(const CString & strImg)
{
	Texture texTga;
	if(!LoadTGA(&texTga, strImg))
		return ImagePtr();

	boost::shared_ptr<Gdiplus::Bitmap> bmp;
	Gdiplus::BitmapData bd;
	switch(texTga.bpp)
	{
	case 32:
		bmp.reset(new Gdiplus::Bitmap(texTga.width, texTga.height, PixelFormat32bppARGB));
		if(Gdiplus::Ok == bmp->LockBits(NULL, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bd))
		{
			for(UINT y = 0; y < texTga.height; y++)
			{
				for(UINT x = 0; x < texTga.width; x++)
				{
					BYTE * pSrc = texTga.imageData + ((texTga.height - y - 1) * texTga.width + x) * 4;
					BYTE * pDst = (BYTE *)bd.Scan0 + (y * bd.Stride + x * 4);
					pDst[0] = pSrc[2];
					pDst[1] = pSrc[1];
					pDst[2] = pSrc[0];
					pDst[3] = pSrc[3];
				}
			}
			bmp->UnlockBits(&bd);
		}
		break;

	case 24:
		bmp.reset(new Gdiplus::Bitmap(texTga.width, texTga.height, PixelFormat24bppRGB));
		if(Gdiplus::Ok == bmp->LockBits(NULL, Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, &bd))
		{
			for(UINT y = 0; y < texTga.height; y++)
			{
				for(UINT x = 0; x < texTga.width; x++)
				{
					BYTE * pSrc = texTga.imageData + ((texTga.height - y - 1) * texTga.width + x) * 3;
					BYTE * pDst = (BYTE *)bd.Scan0 + (y * bd.Stride + x * 3);
					pDst[0] = pSrc[2];
					pDst[1] = pSrc[1];
					pDst[2] = pSrc[0];
				}
			}
			bmp->UnlockBits(&bd);
		}
		break;

	case 16:
		ASSERT(false);
		break;
	}
	free(texTga.imageData);
	return bmp;
}

FontPtr2 CMainApp::GetFont(const CString & strFamily, float fSize)
{
	if(!strFamily.IsEmpty())
	{
		CString strFont;
		strFont.Format(_T("%s, %f"), strFamily, fSize);

		FontPtr2 ret;
		std::wstring key(strFont);
		FontPtr2Map::iterator fnt_iter = m_FontMap.find(key);
		if(fnt_iter != m_FontMap.end())
		{
			ret = fnt_iter->second.lock();
			if(ret)
				return ret;
		}

		m_FontMap[key] =
			ret = FontPtr2(new Gdiplus::Font(strFamily, fSize, Gdiplus::FontStyleRegular, Gdiplus::UnitWorld));
		return ret;
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

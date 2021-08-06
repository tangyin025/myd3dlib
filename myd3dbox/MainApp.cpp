
// myd3dbox.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"
#include <boost/program_options.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainApp

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMainApp::OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CMainApp construction

CMainApp::CMainApp()
{
	default_font_height = 13;
	default_font_face_index = 0;
	max_editable_particle_count = 10;
	technique_RenderSceneColor = NULL;
	handle_MeshColor = NULL;
	m_bNeedDraw = FALSE;
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MFCApplication1.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_UIRender.reset(new my::UIRender());
}

CMainApp::~CMainApp()
{
}

// The one and only CMainApp object

CMainApp theApp;

BOOL CMainApp::CreateD3DDevice(HWND hWnd)
{
	ZeroMemory(&m_DeviceSettings, sizeof(m_DeviceSettings));
	m_DeviceSettings.AdapterOrdinal = D3DADAPTER_DEFAULT;
	m_DeviceSettings.DeviceType = D3DDEVTYPE_HAL;
	m_DeviceSettings.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	m_DeviceSettings.pp.Windowed = TRUE;
	m_DeviceSettings.pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_DeviceSettings.pp.BackBufferFormat = D3DFMT_UNKNOWN;
	m_DeviceSettings.pp.hDeviceWindow = hWnd;
	m_DeviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr;
	if(FAILED(hr = m_d3d9->CreateDevice(
		m_DeviceSettings.AdapterOrdinal,
		m_DeviceSettings.DeviceType,
		hWnd,
		m_DeviceSettings.BehaviorFlags,
		&m_DeviceSettings.pp,
		&m_d3dDevice)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_DeviceObjectsCreated = true;

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if (FAILED(hr = OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_DeviceObjectsReset = true;

	if (FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	return TRUE;
}

BOOL CMainApp::ResetD3DDevice(void)
{
	if(m_DeviceObjectsReset)
	{
		OnLostDevice();
		m_DeviceObjectsReset = false;
	}

	if(FAILED(hr = m_d3dDevice->Reset(&m_DeviceSettings.pp)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_DeviceObjectsReset = true;

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if (FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		OnLostDevice();
		m_DeviceObjectsReset = false;
		return FALSE;
	}

	return TRUE;
}

void CMainApp::DestroyD3DDevice(void)
{
	if (m_DeviceObjectsReset)
	{
		OnLostDevice();
		m_DeviceObjectsReset = false;
	}

	if(m_DeviceObjectsCreated)
	{
		OnDestroyDevice();

		UINT references = m_d3dDevice.Detach()->Release();
		if(references > 0)
		{
			CString msg;
			msg.Format(_T("no zero reference count: %u"), references);
			AfxMessageBox(msg);
		}
		m_DeviceObjectsCreated = false;
	}
}

// CMainApp initialization

BOOL CMainApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	char buff[MAX_PATH];
	GetModuleFileNameA(NULL, buff, _countof(buff));
	std::string cfg_file(PathFindFileNameA(buff), PathFindExtensionA(buff));

	boost::program_options::options_description desc("Options");
	std::vector<std::string> path_list;
	desc.add_options()
		("path", boost::program_options::value(&path_list), "Path")
		("default_fov", boost::program_options::value(&default_fov)->default_value(75.0f), "Default fov")
		("default_load_shader_cache", boost::program_options::value(&default_load_shader_cache)->default_value(true), "Default load shader cache")
		("default_font", boost::program_options::value(&default_font)->default_value("font/wqy-microhei.ttc"), "Default font")
		("default_font_height", boost::program_options::value(&default_font_height)->default_value(13), "Default font height")
		("default_font_face_index", boost::program_options::value(&default_font_face_index)->default_value(0), "Default font face index")
		("default_texture", boost::program_options::value(&default_texture)->default_value("texture/Checker.bmp"), "Default texture")
		("default_normal_texture", boost::program_options::value(&default_normal_texture)->default_value("texture/Normal.dds"), "Default normal texture")
		("default_specular_texture", boost::program_options::value(&default_specular_texture)->default_value("texture/White.dds"), "Default specular texture")
		("default_shader", boost::program_options::value(&default_shader)->default_value("shader/mtl_BlinnPhong.fx"), "Default shader")
		("default_terrain_shader", boost::program_options::value(&default_terrain_shader)->default_value("shader/mtl_Splatmap.fx"), "Default terrain shader")
		("default_dlg_img", boost::program_options::value(&default_dlg_img)->default_value("texture/CommonUI.png"), "Default dialog image")
		("default_dlg_img_rect_x", boost::program_options::value(&default_dlg_img_rect.x)->default_value(154.0f), "Default dialog image rect x")
		("default_dlg_img_rect_y", boost::program_options::value(&default_dlg_img_rect.y)->default_value(43.0f), "Default dialog image rect y")
		("default_dlg_img_rect_w", boost::program_options::value(&default_dlg_img_rect.z)->default_value(2.0f), "Default dialog image rect width")
		("default_dlg_img_rect_h", boost::program_options::value(&default_dlg_img_rect.w)->default_value(2.0f), "Default dialog image rect height")
		("default_dlg_img_border_x", boost::program_options::value(&default_dlg_img_border.x)->default_value(0), "Default dialog image border x")
		("default_dlg_img_border_y", boost::program_options::value(&default_dlg_img_border.y)->default_value(0), "Default dialog image border y")
		("default_dlg_img_border_z", boost::program_options::value(&default_dlg_img_border.z)->default_value(0), "Default dialog image border z")
		("default_dlg_img_border_w", boost::program_options::value(&default_dlg_img_border.w)->default_value(0), "Default dialog image border w")
		("max_editable_particle_count", boost::program_options::value(&max_editable_particle_count)->default_value(10), "Max editable particle count")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_config_file<char>((cfg_file + ".cfg").c_str(), desc, true), vm);
	boost::program_options::notify(vm);
	if (path_list.empty())
	{
		path_list.push_back("..\\demo2_3\\Media");
		path_list.push_back("Media");
	}
	std::vector<std::string>::const_iterator path_iter = path_list.begin();
	for (; path_iter != path_list.end(); path_iter++)
	{
		ResourceMgr::RegisterFileDir(*path_iter);
		ResourceMgr::RegisterZipDir(*path_iter + ".zip");
	}

	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	if (!PhysxSdk::Init())
	{
		return FALSE;
	}

	m_d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if(!m_d3d9)
	{
		return FALSE;
	}

	//// Register the application's document templates.  Document templates
	////  serve as the connection between documents, frame windows and views
	//CSingleDocTemplate* pDocTemplate;
	//pDocTemplate = new CSingleDocTemplate(
	//	IDR_MAINFRAME,
	//	RUNTIME_CLASS(CMainDoc),
	//	RUNTIME_CLASS(CMainFrame),       // main SDI frame window
	//	RUNTIME_CLASS(CChildView));
	//if (!pDocTemplate)
	//	return FALSE;
	//AddDocTemplate(pDocTemplate);

	CMainFrame * pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	//// Parse command line for standard shell commands, DDE, file open
	//CCommandLineInfo cmdInfo;
	//ParseCommandLine(cmdInfo);

	//// Dispatch commands specified on the command line.  Will return FALSE if
	//// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	//if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;
	pFrame->OnCmdMsg(ID_FILE_NEW, 0, NULL, NULL);

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}


HRESULT CMainApp::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	D3DContext::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

	if(FAILED(hr = my::ResourceMgr::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	my::ResourceMgr::StartIORequestProc(4);

	if (FAILED(hr = RenderPipeline::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (default_load_shader_cache)
	{
		TCHAR szDir[MAX_PATH];
		GetCurrentDirectory(_countof(szDir), szDir);
		RenderPipeline::LoadShaderCache(szDir);
	}

	BOOST_VERIFY(technique_RenderSceneColor = m_SimpleSample->GetTechniqueByName("RenderSceneColor"));
	BOOST_VERIFY(handle_MeshColor = m_SimpleSample->GetParameterByName(NULL, "g_MeshColor"));

	if (FAILED(hr = m_UIRender->OnCreateDevice(pd3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (!(m_Font = LoadFont(default_font.c_str(), default_font_height, default_font_face_index)))
	{
		TRACE("LoadFont failed");
		return S_FALSE;
	}
	return S_OK;
}

HRESULT CMainApp::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	D3DContext::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

	if(FAILED(hr = ResourceMgr::OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (FAILED(hr = m_UIRender->OnResetDevice(pd3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	return S_OK;
}

void CMainApp::OnLostDevice(void)
{
	m_UIRender->OnLostDevice();

	ResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();

	D3DContext::OnLostDevice();
}

void CMainApp::OnDestroyDevice(void)
{
	m_UIRender->OnDestroyDevice();

	ResourceMgr::OnDestroyDevice();

	RenderPipeline::OnDestroyDevice();

	D3DContext::OnDestroyDevice();

	m_UIRender.reset();
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CMainApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMainApp customization load/save methods

void CMainApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	GetContextMenuManager()->AddMenu(_T(""), IDR_POPUP_VIEW);
}

void CMainApp::LoadCustomState()
{
}

void CMainApp::SaveCustomState()
{
}

// CMainApp message handlers




BOOL CMainApp::OnIdle(LONG lCount)
{
	// TODO: Add your specialized code here and/or call the base class
	Clock::UpdateClock();

	LeaveDeviceSection();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);
	if (pFrame->m_State)
	{
		pFrame->dogcstep(1);
	}

	if (!m_IORequestList.empty())
	{
		m_bNeedDraw = TRUE;
	}

	BOOL bContinue = FALSE;
	if (my::ResourceMgr::CheckIORequests(0))
	{
		bContinue = TRUE;
	}

	if (CWinAppEx::OnIdle(lCount))
	{
		bContinue = TRUE;
	}

	EnterDeviceSection();

	if (!pFrame->m_selactors.empty())
	{
		pFrame->OnFrameTick(m_fElapsedTime);

		my::EventArg arg;
		pFrame->m_EventSelectionPlaying(&arg);

		bContinue = TRUE;
	}
	else if (!bContinue && m_bNeedDraw)
	{
		my::EventArg arg;
		pFrame->m_EventSelectionPlaying(&arg);

		m_bNeedDraw = FALSE;
	}

	return bContinue;
}

int CMainApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	PhysxSdk::Shutdown();

	return CWinAppEx::ExitInstance();
}

extern BOOL g_bRemoveFromMRU;

CDocument* CMainApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: Add your specialized code here and/or call the base class
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_strPathName = lpszFileName;
	//theApp.AddToRecentFileList(lpszFileName);
	CWaitCursor wait;
	pFrame->ClearFileContext();
	pFrame->InitFileContext();
	if (pFrame->OpenFileContext(lpszFileName))
	{
		g_bRemoveFromMRU = FALSE;

		pFrame->OnSelChanged();

		CChildView * pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
		ASSERT_VALID(pView);
		CEnvironmentWnd::CameraPropEventArgs arg(pView);
		pFrame->m_EventCameraPropChanged(&arg);
	}
	return NULL;
}

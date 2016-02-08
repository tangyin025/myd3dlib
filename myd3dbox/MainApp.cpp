
// myd3dbox.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"

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

	m_bHiColorIcons = TRUE;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
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

	char CurrDir[MAX_PATH];
	GetCurrentDirectoryA(_countof(CurrDir), CurrDir);

	char BuffDir[MAX_PATH];
	RegisterFileDir(PathCombineA(BuffDir, CurrDir, "Media"));
	RegisterZipDir(PathCombineA(BuffDir, CurrDir, "Media.zip"));
	RegisterFileDir(PathCombineA(BuffDir, CurrDir, "..\\demo2_3\\Media"));
	RegisterZipDir(PathCombineA(BuffDir, CurrDir, "..\\demo2_3\\Media.zip"));

	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	LuaContext::Init();

	if (!PhysXContext::Init())
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
	if(FAILED(hr = my::ResourceMgr::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	m_UIRender.reset(new my::UIRender(m_d3dDevice));

	if (!(m_Font = LoadFont("font/wqy-microhei.ttc", 13)))
	{
		TRACE("LoadFont failed");
		return S_FALSE;
	}

	if (!(m_SimpleSample = LoadEffect("shader/SimpleSample.fx", "")))
	{
		TRACE("LoadEffect failed");
		return S_FALSE;
	}
	return S_OK;
}

HRESULT CMainApp::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	D3DContext::m_EventDeviceReset();

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

	return S_OK;
}

void CMainApp::OnLostDevice(void)
{
	D3DContext::m_EventDeviceLost();

	ResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();
}

void CMainApp::OnDestroyDevice(void)
{
	D3DContext::m_EventDeviceDestroy();

	ResourceMgr::OnDestroyDevice();

	RenderPipeline::OnDestroyDevice();

	m_UIRender.reset();

	m_ShaderCache.clear();
}

void CMainApp::OnResourceFailed(const std::string & error_str)
{
	TRACE(ms2ts(error_str).c_str());
}

static size_t hash_value(const CMainApp::ShaderCacheKey & key)
{
	size_t seed = 0;
	boost::hash_combine(seed, key.get<0>());
	boost::hash_combine(seed, key.get<1>());
	boost::hash_combine(seed, key.get<2>());
	return seed;
}

my::Effect * CMainApp::QueryShader(RenderPipeline::MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID)
{
	_ASSERT(material && !material->m_Shader.empty());

	ShaderCacheKey key(mesh_type, bInstance, material->m_Shader);
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(key);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	struct Header
	{
		static const char * vs_header(unsigned int mesh_type)
		{
			switch (mesh_type)
			{
			case RenderPipeline::MeshTypeAnimation:
				return "MeshSkeleton.fx";
			case RenderPipeline::MeshTypeParticle:
				return "MeshParticle.fx";
			}
			return "MeshStatic.fx";
		}
	};

	std::ostringstream oss;
	oss << "#define SHADOW_MAP_SIZE " << SHADOW_MAP_SIZE << std::endl;
	oss << "#define SHADOW_EPSILON " << SHADOW_EPSILON << std::endl;
	oss << "#define INSTANCE " << (unsigned int)bInstance << std::endl;
	oss << "#include \"CommonHeader.fx\"" << std::endl;
	oss << "#include \"" << Header::vs_header(mesh_type) << "\"" << std::endl;
	oss << "#include \"" << material->m_Shader << "\"" << std::endl;
	std::string source = oss.str();

	CComPtr<ID3DXBuffer> buff;
	if (SUCCEEDED(D3DXPreprocessShader(source.c_str(), source.length(), NULL, this, &buff, NULL)))
	{
		my::OStreamPtr ostr = my::FileOStream::Open(str_printf(_T("%S_%u_%u.fx"), material->m_Shader.c_str(), mesh_type, bInstance).c_str());
		ostr->write(buff->GetBufferPointer(), buff->GetBufferSize()-1);
	}

	my::EffectPtr shader(new my::Effect());
	try
	{
		shader->CreateEffect(m_d3dDevice, source.c_str(), source.size(), NULL, this, 0, m_EffectPool);
	}
	catch (const my::Exception & e)
	{
		OnResourceFailed(e.what());
		shader.reset();
	}
	m_ShaderCache.insert(std::make_pair(key, shader));
	return shader.get();
}

void CMainApp::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	TRACE(message);
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
	if (my::ResourceMgr::CheckIORequests())
	{
		return TRUE;
	}

	return CWinAppEx::OnIdle(lCount);
}

int CMainApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	PhysXContext::Shutdown();

	return CWinAppEx::ExitInstance();
}

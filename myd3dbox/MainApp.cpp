
// myd3dbox.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "MainApp.h"
#include "MainFrm.h"

#include "MainDoc.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainApp

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMainApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
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
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if(FAILED(hr = ResourceMgr::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}
	m_DeviceObjectsCreated = true;

	if(FAILED(hr = ResourceMgr::OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}
	m_DeviceObjectsReset = true;

	return TRUE;
}

BOOL CMainApp::ResetD3DDevice(void)
{
	if(m_DeviceObjectsReset)
	{
		ResourceMgr::OnLostDevice();
	}
	m_DeviceObjectsReset = false;

	if(FAILED(hr = m_d3dDevice->Reset(&m_DeviceSettings.pp)))
	{
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	// ! 不会通知除my::ResourceMgr以外其他对象DeviceReset，要注意
	if(FAILED(hr = ResourceMgr::OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}
	m_DeviceObjectsReset = true;

	return TRUE;
}

void CMainApp::DestroyD3DDevice(void)
{
	if(m_DeviceObjectsCreated)
	{
		ResourceMgr::OnDestroyDevice();

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

	m_d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if(!m_d3d9)
	{
		return FALSE;
	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMainDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CChildView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);



	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
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
}

void CMainApp::LoadCustomState()
{
}

void CMainApp::SaveCustomState()
{
}

// CMainApp message handlers




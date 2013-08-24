#include "StdAfx.h"
#include "MainApp.h"
#include "MainDoc.h"
#include "MainFrm.h"
#include "MainView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainApp theApp;

BOOL CMainApp::InitInstance(void)
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings(4);

	RegisterFileDir("Media");
	RegisterZipArchive("Media.zip");
	RegisterFileDir("..\\demo2_3\\Media");
	RegisterZipArchive("..\\demo2_3\\Media.zip");

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
	BOOL bRet = CWinAppEx::OnIdle(lCount);

	if(!m_DeviceObjectsReset)
	{
		if(D3DERR_DEVICELOST == ResetD3DDevice())
		{
			return bRet;
		}
	}
	return bRet;
}

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
	if(FAILED(hr = theApp.m_d3d9->CreateDevice(
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

		m_DeviceObjectsReset = false;
	}

	if(FAILED(hr = m_d3dDevice->Reset(&m_DeviceSettings.pp)))
	{
		TRACE(my::D3DException::Translate(hr).c_str());
		return FALSE;
	}

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	// ! 不会通知除my::ResourceMgr意外其他对象DeviceReset，要注意
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


#include "stdafx.h"
#include "myd3dlib.h"

using namespace my;

bool CALLBACK DxutAppBase::IsD3D9DeviceAcceptable_s(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed,
	void * pUserContext)
{
	IDirect3D9 * pD3D = DXUTGetD3D9Object();
	if(FAILED((pD3D->CheckDeviceFormat(
		pCaps->AdapterOrdinal,
		pCaps->DeviceType,
		AdapterFormat,
		D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE,
		BackBufferFormat))))
	{
		return false;
	}

	return reinterpret_cast<DxutAppBase *>(pUserContext)->IsD3D9DeviceAcceptable(
		pCaps, AdapterFormat, BackBufferFormat, bWindowed);
}

bool CALLBACK DxutAppBase::ModifyDeviceSettings_s(
	DXUTDeviceSettings * pDeviceSettings,
	void * pUserContext)
{
	return reinterpret_cast<DxutAppBase *>(pUserContext)->ModifyDeviceSettings(
		pDeviceSettings);
}

HRESULT CALLBACK DxutAppBase::OnD3D9CreateDevice_s(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	void * pUserContext)
{
	return reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9CreateDevice(
		pd3dDevice, pBackBufferSurfaceDesc);
}

HRESULT CALLBACK DxutAppBase::OnD3D9ResetDevice_s(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	void * pUserContext)
{
	return reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9ResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc);
}

void CALLBACK DxutAppBase::OnD3D9LostDevice_s(
	void * pUserContext)
{
	reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9LostDevice();
}

void CALLBACK DxutAppBase::OnD3D9DestroyDevice_s(
	void * pUserContext)
{
	reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9DestroyDevice();
}

void CALLBACK DxutAppBase::OnFrameMove_s(
	double fTime,
	float fElapsedTime,
	void * pUserContext)
{
	reinterpret_cast<DxutAppBase *>(pUserContext)->OnFrameMove(
		fTime, fElapsedTime);
}

void CALLBACK DxutAppBase::OnD3D9FrameRender_s(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime,
	void * pUserContext)
{
	reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9FrameRender(
		pd3dDevice, fTime, fElapsedTime);
}

LRESULT CALLBACK DxutAppBase::MsgProc_s(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing,
	void * pUserContext)
{
	return reinterpret_cast<DxutAppBase *>(pUserContext)->MsgProc(
		hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
}

void CALLBACK DxutAppBase::OnKeyboard_s(
	UINT nChar,
	bool bKeyDown,
	bool bAltDown,
	void * pUserContext)
{
	reinterpret_cast<DxutAppBase *>(pUserContext)->OnKeyboard(
		nChar, bKeyDown, bAltDown);
}

DxutAppBase::DxutAppBase(void)
{
	DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable_s, this);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings_s, this);
	DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice_s, this);
	DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice_s, this);
	DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice_s, this);
	DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice_s, this);
	DXUTSetCallbackFrameMove(OnFrameMove_s, this);
	DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender_s, this);
	DXUTSetCallbackMsgProc(MsgProc_s, this);
	DXUTSetCallbackKeyboard(OnKeyboard_s, this);
}

DxutAppBase::~DxutAppBase(void)
{
	//DXUTDestroyState();
	// cannot call DXUTDestroyState() at base class whose drived class'es interface have been destroyed
}

int DxutAppBase::Run(
	bool bWindowed,
	int nSuggestedWidth,
	int nSuggestedHeight)
{
	DXUTInit(true, true, NULL);
	DXUTSetCursorSettings(true, true);
	WCHAR szPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
	DXUTCreateWindow(szPath);
	DXUTCreateDevice(bWindowed, nSuggestedWidth, nSuggestedHeight);
	DXUTMainLoop();
	return DXUTGetExitCode();
}

SingleInstance<DxutApp> * SingleInstance<DxutApp>::s_ptr = NULL;

bool DxutApp::IsD3D9DeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	return true;
}

bool DxutApp::ModifyDeviceSettings(
	DXUTDeviceSettings * pDeviceSettings)
{
	if(DXUT_D3D9_DEVICE == pDeviceSettings->ver
		&& D3DDEVTYPE_REF == pDeviceSettings->d3d9.DeviceType)
	{
		DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	}
	return true;
}

HRESULT DxutApp::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

HRESULT DxutApp::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	ResourceMgr::getSingleton().OnResetDevice();
	return S_OK;
}

void DxutApp::OnD3D9LostDevice(void)
{
	ResourceMgr::getSingleton().OnLostDevice();
}

void DxutApp::OnD3D9DestroyDevice(void)
{
	ResourceMgr::getSingleton().OnDestroyDevice();
}

void DxutApp::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
}

void DxutApp::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
}

LRESULT DxutApp::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

void DxutApp::OnKeyboard(
	UINT nChar,
	bool bKeyDown,
	bool bAltDown)
{
}

void DxutApp::OnInit(void)
{
}

int DxutApp::Run(
	bool bWindowed,
	int nSuggestedWidth,
	int nSuggestedHeight)
{
	try
	{
		OnInit();
		int nExitCode = DxutAppBase::Run(bWindowed, nSuggestedWidth, nSuggestedHeight);
		DXUTDestroyState();
		return nExitCode;
	}
	catch(const my::Exception & e)
	{
		MessageBoxA(GetDesktopWindow(), e.GetFullDescription().c_str(), "Exception", MB_OK);
		DXUTDestroyState();
		return 0;
	}
}

bool DxutSample::IsD3D9DeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	// Skip backbuffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	// No fallback defined by this app, so reject any device that 
	// doesn't support at least ps2.0
	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}

bool DxutSample::ModifyDeviceSettings(
	DXUTDeviceSettings * pDeviceSettings)
{
	assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

	HRESULT hr;
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	D3DCAPS9 caps;

	V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps ) );

	//// Turn vsync off
	//pDeviceSettings->d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	//m_settingsDlg.GetDialogControl()->GetComboBox( DXUTSETTINGSDLG_PRESENT_INTERVAL )->SetEnabled( false );

	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// If the hardware cannot do vertex blending, use software vertex processing.
	if( caps.MaxVertexBlendMatrices < 2 )
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//// If using hardware vertex processing, change to mixed vertex processing
	//// so there is a fallback.
	//if( pDeviceSettings->d3d9.BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
	//    pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
//
//		// Debugging vertex shaders requires either REF or software vertex processing 
//		// and debugging pixel shaders requires REF.  
//#ifdef DEBUG_VS
//		if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
//		{
//			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
//			pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
//			pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
//		}
//#endif
//#ifdef DEBUG_PS
//		pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
//#endif
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}

void DxutSample::OnInit(void)
{
	DxutApp::OnInit();

	m_settingsDlg.Init(&m_dlgResourceMgr);
	m_hudDlg.Init(&m_dlgResourceMgr);
	m_hudDlg.SetCallback(OnGUIEvent_s, this);

	//// Supports all types of vertex processing, including mixed.
	//DXUTGetD3D9Enumeration()->SetPossibleVertexProcessingList( true, true, true, true );

	int nY = 10;
	m_hudDlg.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, nY, 125, 22);
	m_hudDlg.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, nY += 24, 125, 22, VK_F3);
	m_hudDlg.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 35, nY += 24, 125, 22, VK_F2);
}

HRESULT DxutSample::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9CreateDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	V(m_dlgResourceMgr.OnD3D9CreateDevice(pd3dDevice));

	V(m_settingsDlg.OnD3D9CreateDevice(pd3dDevice));

	if(FAILED(hres = D3DXCreateFont(
		pd3dDevice, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_font)))
	{
		return hres;
	}

	V(D3DXCreateSprite(pd3dDevice, &m_sprite));

	my::ImeEditBox::Initialize(DXUTGetHWND());

	my::ImeEditBox::EnableImeSystem(false);

	return S_OK;
}

HRESULT DxutSample::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	V(m_dlgResourceMgr.OnD3D9ResetDevice());

	V(m_settingsDlg.OnD3D9ResetDevice());

	m_hudDlg.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);

	m_hudDlg.SetSize(170, 170);

	V(m_font->OnResetDevice());

	V(m_sprite->OnResetDevice());

	return S_OK;
}

void DxutSample::OnD3D9LostDevice(void)
{
	DxutApp::OnD3D9LostDevice();

	m_dlgResourceMgr.OnD3D9LostDevice();

	m_settingsDlg.OnD3D9LostDevice();

	m_font->OnLostDevice();

	m_sprite->OnLostDevice();
}

void DxutSample::OnD3D9DestroyDevice(void)
{
	DxutApp::OnD3D9DestroyDevice();

	m_dlgResourceMgr.OnD3D9DestroyDevice();

	m_settingsDlg.OnD3D9DestroyDevice();

	m_font.Release();

	m_sprite.Release();

	my::ImeEditBox::Uninitialize();
}

void DxutSample::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	DxutApp::OnFrameMove(fTime, fElapsedTime);
}

void DxutSample::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	if(m_settingsDlg.IsActive())
	{
		m_settingsDlg.OnRender(fElapsedTime);
		return;
	}

	OnRender(pd3dDevice, fTime, fElapsedTime);

	HRESULT hr;
	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		CDXUTTextHelper txtHelper(m_font, m_sprite, 15);
		txtHelper.Begin();
		txtHelper.SetInsertionPos(5, 5);
		txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		txtHelper.DrawTextLine(DXUTGetFrameStats( DXUTIsVsyncEnabled()));
		txtHelper.DrawTextLine(DXUTGetDeviceStats());
		txtHelper.End();

		V(m_hudDlg.OnRender(fElapsedTime));

		V(pd3dDevice->EndScene());
	}
}

void DxutSample::OnRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	HRESULT hr;
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));
}

void CALLBACK DxutSample::OnGUIEvent_s(
	UINT nEvent,
	int nControlID,
	CDXUTControl * pControl,
	void * pUserContext)
{
	reinterpret_cast<DxutSample *>(pUserContext)->OnGUIEvent(
		nEvent, nControlID, pControl);
}

void DxutSample::OnGUIEvent(
	UINT nEvent,
	int nControlID,
	CDXUTControl * pControl)
{
	switch(nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen();
		break;

	case IDC_TOGGLEREF:
		DXUTToggleREF();
		break;

	case IDC_CHANGEDEVICE:
		m_settingsDlg.SetActive(!m_settingsDlg.IsActive());
		break;
	}
}

LRESULT DxutSample::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	LRESULT hres;
	if(FAILED(hres = DxutApp::MsgProc(
		hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing)
	{
		return hres;
	}

	if((*pbNoFurtherProcessing = my::ImeEditBox::StaticMsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if((*pbNoFurtherProcessing = m_dlgResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if(m_settingsDlg.IsActive())
	{
		m_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		//*pbNoFurtherProcessing = true;
		return 0;
	}

	if((*pbNoFurtherProcessing = m_hudDlg.MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	return 0;
}

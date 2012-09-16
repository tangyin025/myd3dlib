#include "stdafx.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "libc.h"

using namespace my;

BOOL DxutWindow::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
{
	switch(dwMsgMapID)
	{
	case 0:
		switch(uMsg)
		{
		case WM_CREATE:
			ATLASSERT(m_hWnd);
			lResult = 0;
			return TRUE;

		case WM_DESTROY:
			lResult = 0;
			return TRUE;

		case WM_SIZE:
			switch(wParam)
			{
			case SIZE_MINIMIZED:
				m_Minimized = true;
				break;

			case SIZE_MAXIMIZED:
				DxutApplication::getSingleton().CheckForWindowSizeChange();
				m_Maximized = true;
				break;

			case SIZE_RESTORED:
				if(!m_InSizeMove)
				{
					DxutApplication::getSingleton().CheckForWindowSizeChange();
					m_Minimized = false;
					m_Maximized = false;
				}
				break;
			}
			break;

		case WM_GETMINMAXINFO:
			((MINMAXINFO *)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO *)lParam)->ptMinTrackSize.y = 200;
			break;

		case WM_ENTERSIZEMOVE:
			m_InSizeMove = true;
			break;

		case WM_EXITSIZEMOVE:
			DxutApplication::getSingleton().CheckForWindowSizeChange();
			m_InSizeMove = false;
			break;

		case WM_ACTIVATEAPP:
			break;

		case WM_MENUCHAR:
			lResult = MAKELRESULT(0, MNC_CLOSE);
			return TRUE;

		case WM_SYSKEYDOWN:
			switch(wParam)
			{
			case VK_RETURN:
				if(GetKeyState(VK_MENU))
				{
					DxutApplication::getSingleton().ToggleFullScreen();
					lResult = 0;
					return TRUE;
				}
				break;
			}
			break;

		default:
			bool bNoFurtherProcessing = false;
			lResult = DxutApplication::getSingleton().MsgProc(hWnd, uMsg, wParam, lParam, &bNoFurtherProcessing);
			if(bNoFurtherProcessing)
			{
				return TRUE;
			}
		}
		break;

	default:
		ATLTRACE(ATL::atlTraceWindowing, 0, _T("Invalid message map ID (%i)\n"), dwMsgMapID);
		ATLASSERT(FALSE);
		break;
	}
	return FALSE;
}

DxutApplication::SingleInstance * SingleInstance<DxutApplication>::s_ptr = NULL;

DxutApplication::DxutApplication(void)
	: m_DeviceObjectsCreated(false)
	, m_DeviceObjectsReset(false)
	, m_FullScreenBackBufferWidthAtModeChange(0)
	, m_FullScreenBackBufferHeightAtModeChange(0)
	, m_WindowBackBufferWidthAtModeChange(800)
	, m_WindowBackBufferHeightAtModeChange(600)
	, m_IgnoreSizeChange(false)
	, m_DeviceLost(false)
	, m_fAbsoluteTime(0)
	, m_fLastTime(0)
	, m_dwFrames(0)
	, m_llQPFTicksPerSec(0)
	, m_llLastElapsedTime(0)
{
	FT_Error err = FT_Init_FreeType(&m_Library);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Init_FreeType failed");
	}

	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency(&qwTicksPerSec);
	m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
}

DxutApplication::~DxutApplication(void)
{
	FT_Error err = FT_Done_FreeType(m_Library);
}

int DxutApplication::Run(void)
{
	m_wnd = DxutWindowPtr(new DxutWindow());
	CRect desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	CRect clientRect(0, 0, m_WindowBackBufferWidthAtModeChange, m_WindowBackBufferHeightAtModeChange);
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
	clientRect.MoveToXY((desktopRect.Width() - clientRect.Width()) / 2, (desktopRect.Height() - clientRect.Height()) / 2);
	m_wnd->Create(NULL, clientRect, GetModuleFileName().c_str());

	MSG msg = {0};

	try
	{
		LPDIRECT3D9 pd3d9 = Direct3DCreate9(D3D_SDK_VERSION);
		if(NULL == pd3d9)
		{
			THROW_CUSEXCEPTION("cannnot create direct3d9");
		}
		m_d3d9.Attach(pd3d9);

		CRect clientRect;
		m_wnd->GetClientRect(&clientRect);

		CreateDevice(true, clientRect.Width(), clientRect.Height());

		m_wnd->ShowWindow(SW_SHOW);
		m_wnd->UpdateWindow();

		msg.message = WM_NULL;
		while(WM_QUIT != msg.message)
		{
			if(::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
			else
			{
				Render3DEnvironment();
			}
		}

		Cleanup3DEnvironment();
	}
	catch(const my::Exception & e)
	{
		m_wnd->MessageBox(ms2ts(e.GetFullDescription().c_str()).c_str(), _T("Exception"));
		m_wnd->DestroyWindow();
	}

	return (int)msg.wParam;
}

void DxutApplication::CreateDevice(bool bWindowed, int nSuggestedWidth, int nSuggestedHeight)
{
	DXUTMatchOptions matchOptions;
	matchOptions.eAPIVersion = DXUTMT_IGNORE_INPUT;
	matchOptions.eAdapterOrdinal = DXUTMT_IGNORE_INPUT;
	matchOptions.eDeviceType = DXUTMT_IGNORE_INPUT;
	matchOptions.eOutput = DXUTMT_IGNORE_INPUT;
	matchOptions.eWindowed = DXUTMT_PRESERVE_INPUT;
	matchOptions.eAdapterFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eVertexProcessing = DXUTMT_IGNORE_INPUT;
	matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eBackBufferFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eBackBufferCount = DXUTMT_IGNORE_INPUT;
	matchOptions.eMultiSample = DXUTMT_IGNORE_INPUT;
	matchOptions.eSwapEffect = DXUTMT_IGNORE_INPUT;
	matchOptions.eDepthFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eStencilFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.ePresentFlags = DXUTMT_IGNORE_INPUT;
	matchOptions.eRefreshRate = DXUTMT_IGNORE_INPUT;
	matchOptions.ePresentInterval = DXUTMT_PRESERVE_INPUT;

	DXUTD3D9DeviceSettings deviceSettings;
	ZeroMemory(&deviceSettings, sizeof(deviceSettings));
	deviceSettings.pp.Windowed = true;
	deviceSettings.pp.BackBufferWidth = nSuggestedWidth;
	deviceSettings.pp.BackBufferHeight = nSuggestedHeight;
	deviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if(FAILED(hr = DXUTFindValidDeviceSettings(&deviceSettings, &deviceSettings, &matchOptions)))
	{
		THROW_CUSEXCEPTION("no valid devices were found");
	}

	ChangeDevice(deviceSettings);
}

void DxutApplication::CheckForWindowSizeChange(void)
{
	_ASSERT(m_d3dDevice);

	if(!m_IgnoreSizeChange)
	{
		CRect clientRect;
		m_wnd->GetClientRect(&clientRect);

		if(m_BackBufferSurfaceDesc.Width != clientRect.Width() || m_BackBufferSurfaceDesc.Height != clientRect.Height())
		{
			DXUTD3D9DeviceSettings deviceSettings = m_DeviceSettings;
			deviceSettings.pp.BackBufferWidth = clientRect.Width();
			deviceSettings.pp.BackBufferHeight = clientRect.Height();
			ChangeDevice(deviceSettings);
		}
	}
}

bool DxutApplication::CanDeviceBeReset(const DXUTD3D9DeviceSettings & oldDeviceSettings, const DXUTD3D9DeviceSettings & newDeviceSettings)
{
	if(oldDeviceSettings.AdapterOrdinal == newDeviceSettings.AdapterOrdinal
		&& oldDeviceSettings.DeviceType == newDeviceSettings.DeviceType
		&& oldDeviceSettings.BehaviorFlags == newDeviceSettings.BehaviorFlags)
	{
		return true;
	}
	return false;
}

void DxutApplication::ChangeDevice(DXUTD3D9DeviceSettings & deviceSettings)
{
	if(!ModifyDeviceSettings(&deviceSettings))
	{
		return;
	}

	m_IgnoreSizeChange = true;

	if(deviceSettings.pp.Windowed)
	{
		if(m_d3dDevice && !m_DeviceSettings.pp.Windowed)
		{
			m_FullScreenBackBufferWidthAtModeChange = m_BackBufferSurfaceDesc.Width;
			m_FullScreenBackBufferHeightAtModeChange = m_BackBufferSurfaceDesc.Height;
			m_wnd->SetWindowLong(GWL_STYLE, m_WindowedStyleAtModeChange);
			m_wnd->SetWindowPlacement(&m_WindowedPlacement);
			HWND hWndInsertAfter = m_TopmostWhileWindowed ? HWND_TOPMOST : HWND_NOTOPMOST;
			CRect clientRect(0, 0, m_WindowBackBufferWidthAtModeChange, m_WindowBackBufferHeightAtModeChange);
			AdjustWindowRect(&clientRect, m_wnd->GetWindowLong(GWL_STYLE), NULL);
			m_wnd->SetWindowPos(hWndInsertAfter, 0, 0, clientRect.Width(), clientRect.Height(), SWP_NOMOVE | SWP_NOREDRAW);
		}
	}
	else
	{
		if(m_d3dDevice && m_DeviceSettings.pp.Windowed)
		{
			m_WindowBackBufferWidthAtModeChange = m_BackBufferSurfaceDesc.Width;
			m_WindowBackBufferHeightAtModeChange = m_BackBufferSurfaceDesc.Height;
			ZeroMemory(&m_WindowedPlacement, sizeof(m_WindowedPlacement));
			m_WindowedPlacement.length = sizeof(m_WindowedPlacement);
			m_wnd->GetWindowPlacement(&m_WindowedPlacement);
			m_WindowedStyleAtModeChange = m_wnd->GetWindowLong(GWL_STYLE);
			m_TopmostWhileWindowed = ((m_WindowedStyleAtModeChange & WS_EX_TOPMOST) != 0);
		}

		m_wnd->ShowWindow(SW_HIDE);
		m_wnd->SetWindowLong(GWL_STYLE, WS_POPUP | WS_SYSMENU);
		WINDOWPLACEMENT wpFullscreen;
		ZeroMemory(&wpFullscreen, sizeof(WINDOWPLACEMENT));
		wpFullscreen.length = sizeof(WINDOWPLACEMENT);
		m_wnd->GetWindowPlacement(&wpFullscreen);
		if((wpFullscreen.flags & WPF_RESTORETOMAXIMIZED) != 0)
		{
			wpFullscreen.flags &= ~WPF_RESTORETOMAXIMIZED;
			wpFullscreen.showCmd = SW_RESTORE;
			m_wnd->SetWindowPlacement(&wpFullscreen);
		}
	}

	if(m_d3dDevice && CanDeviceBeReset(m_DeviceSettings, deviceSettings))
	{
		if(FAILED(hr = Reset3DEnvironment(deviceSettings)))
		{
			THROW_D3DEXCEPTION(hr);
		}
	}
	else
	{
		if(m_d3dDevice)
		{
			Cleanup3DEnvironment();
		}

		if(FAILED(hr = Create3DEnvironment(deviceSettings)))
		{
			THROW_D3DEXCEPTION(hr);
		}
	}

	m_IgnoreSizeChange = false;
}

void DxutApplication::ToggleFullScreen(void)
{
	DXUTMatchOptions matchOptions;
	matchOptions.eAPIVersion = DXUTMT_PRESERVE_INPUT;
	matchOptions.eAdapterOrdinal = DXUTMT_PRESERVE_INPUT;
	matchOptions.eDeviceType = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eOutput = DXUTMT_IGNORE_INPUT;
	matchOptions.eWindowed = DXUTMT_PRESERVE_INPUT;
	matchOptions.eAdapterFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eVertexProcessing = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eBackBufferFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eBackBufferCount = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eMultiSample = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eSwapEffect = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eDepthFormat = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eStencilFormat = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.ePresentFlags = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eRefreshRate = DXUTMT_IGNORE_INPUT;
	matchOptions.ePresentInterval = DXUTMT_CLOSEST_TO_INPUT;

	DXUTD3D9DeviceSettings deviceSettings = m_DeviceSettings;
	deviceSettings.pp.Windowed = !deviceSettings.pp.Windowed;
	if(deviceSettings.pp.Windowed)
	{
		deviceSettings.pp.BackBufferWidth = m_WindowBackBufferWidthAtModeChange;
		deviceSettings.pp.BackBufferHeight = m_WindowBackBufferHeightAtModeChange;
	}
	else
	{
		deviceSettings.pp.BackBufferWidth = m_FullScreenBackBufferWidthAtModeChange;
		deviceSettings.pp.BackBufferHeight = m_FullScreenBackBufferHeightAtModeChange;
	}

	if(deviceSettings.pp.BackBufferWidth > 0 && deviceSettings.pp.BackBufferHeight > 0)
	{
		matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
	}
	else
	{
		matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
	}

	if(FAILED(hr = DXUTFindValidDeviceSettings(&deviceSettings, &deviceSettings, &matchOptions)))
	{
		THROW_CUSEXCEPTION("no valid devices were found");
	}

	ChangeDevice(deviceSettings);
}

HRESULT DxutApplication::Create3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings)
{
	_ASSERT(!m_DeviceObjectsCreated);

	if(FAILED(hr = m_d3d9->CreateDevice(
		deviceSettings.AdapterOrdinal,
		deviceSettings.DeviceType,
		GetHWND(),
		deviceSettings.BehaviorFlags,
		const_cast<D3DPRESENT_PARAMETERS *>(&deviceSettings.pp),
		&m_d3dDevice)))
	{
		return hr;
	}

	m_DeviceSettings = deviceSettings;

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if(FAILED(hr = OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_DeviceObjectsCreated = true;

	_ASSERT(!m_DeviceObjectsReset);

	if(FAILED(hr = m_d3dDevice->CreateStateBlock(D3DSBT_ALL, &m_StateBlock))
		|| FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_DeviceObjectsReset = true;

	return S_OK;
}

HRESULT DxutApplication::Reset3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings)
{
	if(m_DeviceObjectsReset)
	{
		m_StateBlock.Release();
		OnLostDevice();
		m_DeviceObjectsReset = false;
	}

	_ASSERT(!m_DeviceObjectsReset);

	if(FAILED(hr = m_d3dDevice->Reset(const_cast<D3DPRESENT_PARAMETERS *>(&deviceSettings.pp))))
	{
		return hr;
	}

	m_DeviceSettings = deviceSettings;

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if(FAILED(hr = m_d3dDevice->CreateStateBlock(D3DSBT_ALL, &m_StateBlock))
		|| FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		m_StateBlock.Release();
		OnLostDevice();
		return hr;
	}

	m_DeviceObjectsReset = true;

	return S_OK;
}

void DxutApplication::Render3DEnvironment(void)
{
	if(m_DeviceLost)
	{
		if(FAILED(hr = m_d3dDevice->TestCooperativeLevel()))
		{
			if(D3DERR_DEVICELOST == hr)
			{
				WaitMessage();
				return;
			}

			if(FAILED(hr = Reset3DEnvironment(m_DeviceSettings)))
			{
				if(D3DERR_DEVICELOST == hr)
				{
					WaitMessage();
					return;
				}

				THROW_D3DEXCEPTION(hr);
			}
		}

		m_DeviceLost = false;
	}

	LARGE_INTEGER qwTime;
	QueryPerformanceCounter(&qwTime);
	double fTime = qwTime.QuadPart / (double)m_llQPFTicksPerSec;

	float fElapsedTime = (float)((qwTime.QuadPart - m_llLastElapsedTime) / (double)m_llQPFTicksPerSec);

	m_llLastElapsedTime = qwTime.QuadPart;

	m_fAbsoluteTime = fTime;

	m_dwFrames++;

	if(m_fAbsoluteTime - m_fLastTime > 1.0f)
	{
		float fFPS = (float)(m_dwFrames / (m_fAbsoluteTime - m_fLastTime));
		swprintf_s(m_strFPS, _countof(m_strFPS), L"%0.2f fps", fFPS);
		m_fLastTime = m_fAbsoluteTime;
		m_dwFrames = 0;
	}

	OnFrameMove(fTime, fElapsedTime);

	OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

	if(FAILED(hr = m_d3dDevice->Present(NULL, NULL, NULL, NULL)))
	{
		if(D3DERR_DEVICELOST == hr || D3DERR_DRIVERINTERNALERROR == hr)
		{
			m_DeviceLost = true;
		}
	}
}

void DxutApplication::Cleanup3DEnvironment(void)
{
	if(m_DeviceObjectsReset)
	{
		m_StateBlock.Release();
		OnLostDevice();
		m_DeviceObjectsReset = false;
	}

	OnDestroyDevice();

	_ASSERT(m_d3dDevice && m_DeviceObjectsCreated);

	UINT references = m_d3dDevice.Detach()->Release();
	if(references > 0)
	{
		wchar_t msg[256];
		swprintf_s(msg, _countof(msg), L"no zero reference count: %u", references);
		m_wnd->MessageBox(msg);
	}

	m_DeviceObjectsCreated = false;
}

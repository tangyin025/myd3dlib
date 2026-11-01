// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myDxutApp11.h"
#include "myException.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using namespace my;

//--------------------------------------------------------------------------------------
CD3D11EnumAdapterInfo::~CD3D11EnumAdapterInfo(void)
{
	for (int i = 0; i < outputInfoList.GetSize(); i++)
	{
		CD3D11EnumOutputInfo* pOutputInfo = outputInfoList.GetAt(i);
		delete pOutputInfo;
	}
	outputInfoList.RemoveAll();

	for (int i = 0; i < deviceInfoList.GetSize(); ++i)
	{
		CD3D11EnumDeviceInfo* pDeviceInfo = deviceInfoList.GetAt(i);
		delete pDeviceInfo;
	}
	deviceInfoList.RemoveAll();

	for (int i = 0; i < deviceSettingsComboList.GetSize(); ++i)
	{
		CD3D11EnumDeviceSettingsCombo* pDeviceCombo = deviceSettingsComboList.GetAt(i);
		delete pDeviceCombo;
	}
	deviceSettingsComboList.RemoveAll();

	SAFE_RELEASE(m_pAdapter);
}

//--------------------------------------------------------------------------------------
CD3D11EnumOutputInfo::~CD3D11EnumOutputInfo(void)
{
	SAFE_RELEASE(m_pOutput);
	displayModeList.RemoveAll();
}

//--------------------------------------------------------------------------------------
CD3D11EnumDeviceInfo::~CD3D11EnumDeviceInfo()
{
}

BOOL DxutWindow11::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
{
	switch (dwMsgMapID)
	{
	case 0:
		switch (uMsg)
		{
		case WM_CREATE:
			ATLASSERT(m_hWnd);
			lResult = 0;
			return TRUE;

		case WM_DESTROY:
			lResult = 0;
			return TRUE;

		case WM_SIZE:
			switch (wParam)
			{
			case SIZE_MINIMIZED:
				m_Minimized = true;
				break;

			case SIZE_MAXIMIZED:
				//DxutApp::getSingleton().CheckForWindowSizeChange();
				m_Maximized = true;
				break;

			case SIZE_RESTORED:
				if (!m_InSizeMove)
				{
					//DxutApp::getSingleton().CheckForWindowSizeChange();
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
			//DxutApp::getSingleton().CheckForWindowSizeChange();
			m_InSizeMove = false;
			break;

		case WM_ACTIVATEAPP:
			m_ActivateEvent(wParam != 0);
			break;

		case WM_MENUCHAR:
			lResult = MAKELRESULT(0, MNC_CLOSE);
			return TRUE;

		case WM_SYSKEYDOWN:
			switch (wParam)
			{
			case VK_RETURN:
				if (GetKeyState(VK_MENU))
				{
					//DxutApp::getSingleton().ToggleFullScreen();
					lResult = 0;
					return TRUE;
				}
				break;
			}
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			// goto default

		default:
			bool bNoFurtherProcessing = false;
			//lResult = DxutApp::getSingleton().MsgProc(hWnd, uMsg, wParam, lParam, &bNoFurtherProcessing);
			if (bNoFurtherProcessing)
			{
				switch (uMsg)
				{
				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
					SetCapture();
					break;
				}
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

int DxutApp11::Run(void)
{
	m_wnd.reset(new DxutWindow11());
	CRect desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	CRect clientRect(0, 0, 800, 600);
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
	clientRect.MoveToXY((desktopRect.Width() - clientRect.Width()) / 2, (desktopRect.Height() - clientRect.Height()) / 2);
	m_wnd->Create(NULL, clientRect, GetModuleFileName().c_str());

	try
	{
		if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_DXGIFactory)))
		{
			THROW_CUSEXCEPTION("CreateDXGIFactory failed!");
		}

		CreateDevice(D3D_FEATURE_LEVEL_10_0, true, 800, 600);

		m_wnd->ShowWindow(SW_SHOW);
		m_wnd->UpdateWindow();

		MSG msg = { 0 };
		msg.message = WM_NULL;
		while (WM_QUIT != msg.message)
		{
			if (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
			else
			{
				//Render3DEnvironment();
			}
		}
	}
	catch (const my::Exception& e)
	{
		::MessageBox(m_wnd->m_hWnd, _T("aaa"), NULL, MB_OK);
	}
	return 0;
}

void DxutApp11::CreateDevice(D3D_FEATURE_LEVEL reqFL, bool bWindowed, int nSuggestedWidth, int nSuggestedHeight)
{
	DXUTD3D11DeviceSettings deviceSettings = { 0 };
	deviceSettings.AdapterOrdinal = 0;
	deviceSettings.AutoCreateDepthStencil = true;
	deviceSettings.AutoDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//#if defined(DEBUG) || defined(_DEBUG)
//	deviceSettings.CreateFlags |= D3D10_CREATE_DEVICE_DEBUG;
//#else
	deviceSettings.CreateFlags = 0;
//#endif
	deviceSettings.DriverType = D3D_DRIVER_TYPE_HARDWARE;
	deviceSettings.Output = 0;
	deviceSettings.PresentFlags = 0;
	deviceSettings.sd.BufferCount = 2;
	deviceSettings.sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	deviceSettings.sd.BufferDesc.Height = nSuggestedHeight;
	deviceSettings.sd.BufferDesc.RefreshRate.Numerator = 60;
	deviceSettings.sd.BufferDesc.RefreshRate.Denominator = 1;
	deviceSettings.sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	deviceSettings.sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	deviceSettings.sd.BufferDesc.Width = nSuggestedWidth;
	deviceSettings.sd.BufferUsage = 32;
	deviceSettings.sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	deviceSettings.sd.OutputWindow = m_wnd->m_hWnd;
	deviceSettings.sd.SampleDesc.Count = 1;
	deviceSettings.sd.SampleDesc.Quality = 0;
	deviceSettings.sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	deviceSettings.sd.Windowed = bWindowed;
	deviceSettings.SyncInterval = 0;

	ChangeDevice(&deviceSettings, false, true);
}

void DxutApp11::ChangeDevice(DXUTD3D11DeviceSettings* pNewDeviceSettings, bool bForceRecreate, bool bClipWindowToSingleAdapter)
{
	//DXUTSnapDeviceSettingsToEnumDevice(pNewDeviceSettings, false, (D3D_FEATURE_LEVEL)0);
	pNewDeviceSettings->DeviceFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	if (pNewDeviceSettings->sd.Windowed)
	{
		// Going to windowed mode
		if (!m_DeviceSettings.sd.Windowed)
		{
			//// Going from fullscreen -> windowed
			//GetDXUTState().SetFullScreenBackBufferWidthAtModeChange(DXUTGetBackBufferWidthFromDS(
			//	pOldDeviceSettings));
			//GetDXUTState().SetFullScreenBackBufferHeightAtModeChange(DXUTGetBackBufferHeightFromDS(
			//	pOldDeviceSettings));
			////DXGI should handle SetWindowLong, but in the case where switching from d3d9 full screen to windowed d3d11 it does not.
			//m_wnd->SetWindowLong(GWL_STYLE, m_WindowedStyleAtModeChange);
		}
	}
	else
	{
		// Going to fullscreen mode
		if (m_DeviceSettings.sd.Windowed)
		{
			//// Transistioning to full screen mode from a standard window so 
			//GetDXUTState().SetWindowBackBufferWidthAtModeChange(DXUTGetBackBufferWidthFromDS(
			//	pOldDeviceSettings));
			//GetDXUTState().SetWindowBackBufferHeightAtModeChange(DXUTGetBackBufferHeightFromDS(
			//	pOldDeviceSettings));
		}
	}

	if (true)
	{
		// ! dxut11每次全屏/窗口切换都是一次destroy/create过程
		Cleanup3DEnvironment11(false);
	}

	HRESULT hr = Create3DEnvironment(pNewDeviceSettings);
	if (FAILED(hr))
	{
		m_wnd->DestroyWindow();
		::MessageBox(NULL, _T("aaa"), NULL, MB_OK);
	}
}

void DxutApp11::Cleanup3DEnvironment11(bool bReleaseSettings)
{

}

HRESULT DxutApp11::Create3DEnvironment(DXUTD3D11DeviceSettings* pNewDeviceSettings)
{
	IDXGISwapChain* pSwapChain = NULL;
	HRESULT hr = m_DXGIFactory->MakeWindowAssociation(m_wnd->m_hWnd, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D_DRIVER_TYPE ddt = pNewDeviceSettings->DriverType;
	if (pNewDeviceSettings->DriverType == D3D_DRIVER_TYPE_HARDWARE)
	{
		m_Adapter.Release();
		hr = m_DXGIFactory->EnumAdapters(pNewDeviceSettings->AdapterOrdinal, &m_Adapter);
		if (FAILED(hr))
		{
			return hr;
		}
		ddt = D3D_DRIVER_TYPE_UNKNOWN;
	}
	else if (pNewDeviceSettings->DriverType == D3D_DRIVER_TYPE_WARP)
	{
		ddt = D3D_DRIVER_TYPE_WARP;
		m_Adapter.Release();
	}
	else if (pNewDeviceSettings->DriverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		ddt = D3D_DRIVER_TYPE_REFERENCE;
		m_Adapter.Release();
	}

	if (SUCCEEDED(hr))
	{
		hr = D3D11CreateDevice(m_Adapter,
			ddt,
			(HMODULE)0,
			pNewDeviceSettings->CreateFlags,
			&pNewDeviceSettings->DeviceFeatureLevel,
			1,
			D3D11_SDK_VERSION,
			&m_d3d11Device,
			&m_FeatureLevel,
			&m_d3dImmediateContext);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	return S_OK;
}
//
//HRESULT DxutApp11::DXUTSnapDeviceSettingsToEnumDevice(DXUTD3D11DeviceSettings* pDeviceSettings, bool forceEnum, D3D_FEATURE_LEVEL forceFL)
//{
//	int bestModeIndex = 0;
//	int bestMSAAIndex = 0;
//	CD3D11Enumeration* pEnum = NULL;
//
//	pEnum = DXUTGetD3D11Enumeration(forceEnum, false, forceFL);
//
//	CD3D11EnumAdapterInfo* pAdapterInfo = NULL;
//	CGrowableArray2 <CD3D11EnumAdapterInfo*>* pAdapterList = pEnum->GetAdapterInfoList();
//	CD3D11EnumAdapterInfo* tempAdapterInfo = pAdapterList->GetAt(0);
//	for (int iAdapter = 0; iAdapter < pAdapterList->GetSize(); iAdapter++)
//	{
//		tempAdapterInfo = pAdapterList->GetAt(iAdapter);
//		if (tempAdapterInfo->AdapterOrdinal == pDeviceSettings->AdapterOrdinal) pAdapterInfo = tempAdapterInfo;
//	}
//	if (pAdapterInfo == NULL) return E_FAIL; // no adapters found.
//	CD3D11EnumDeviceSettingsCombo* pDeviceSettingsCombo = NULL;
//	float biggestScore = 0;
//
//	int combo = 0;
//	for (int iDeviceCombo = 0; iDeviceCombo < pAdapterInfo->deviceSettingsComboList.GetSize(); iDeviceCombo++)
//	{
//		CD3D11EnumDeviceSettingsCombo* tempDeviceSettingsCombo = pAdapterInfo->deviceSettingsComboList.GetAt(iDeviceCombo);
//
//		DXGI_MODE_DESC adapterDisplayMode;
//		DXUTGetD3D11AdapterDisplayMode(pAdapterInfo->AdapterOrdinal, 0, &adapterDisplayMode);
//
//		int bestMode;
//		int bestMSAA;
//		float score = DXUTRankD3D11DeviceCombo(tempDeviceSettingsCombo, pDeviceSettings, &adapterDisplayMode, bestMode, bestMSAA);
//		if (score > biggestScore) {
//			combo = iDeviceCombo;
//			biggestScore = score;
//			pDeviceSettingsCombo = tempDeviceSettingsCombo;
//			bestModeIndex = bestMode;
//			bestMSAAIndex = bestMSAA;
//		}
//
//	}
//	if (NULL == pDeviceSettingsCombo) {
//		return E_FAIL; // no settigns found.
//	}
//
//	pDeviceSettings->AdapterOrdinal = pDeviceSettingsCombo->AdapterOrdinal;
//	pDeviceSettings->DriverType = pDeviceSettingsCombo->DeviceType;
//	pDeviceSettings->Output = pDeviceSettingsCombo->Output;
//
//	pDeviceSettings->sd.Windowed = pDeviceSettingsCombo->Windowed;
//	if (GetSystemMetrics(0x1000) != 0) {// SM_REMOTESESSION
//		pDeviceSettings->sd.Windowed = 1;
//	}
//	if (pDeviceSettingsCombo->pOutputInfo != NULL) {
//		DXGI_MODE_DESC bestDisplayMode;
//		bestDisplayMode = pDeviceSettingsCombo->pOutputInfo->displayModeList.GetAt(bestModeIndex);
//		if (!pDeviceSettingsCombo->Windowed) {
//
//			pDeviceSettings->sd.BufferDesc.Height = bestDisplayMode.Height;
//			pDeviceSettings->sd.BufferDesc.Width = bestDisplayMode.Width;
//			pDeviceSettings->sd.BufferDesc.RefreshRate.Numerator = bestDisplayMode.RefreshRate.Numerator;
//			pDeviceSettings->sd.BufferDesc.RefreshRate.Denominator = bestDisplayMode.RefreshRate.Denominator;
//			pDeviceSettings->sd.BufferDesc.Scaling = bestDisplayMode.Scaling;
//			pDeviceSettings->sd.BufferDesc.ScanlineOrdering = bestDisplayMode.ScanlineOrdering;
//		}
//	}
//	if (pDeviceSettings->DeviceFeatureLevel == 0)
//		pDeviceSettings->DeviceFeatureLevel = pDeviceSettingsCombo->pDeviceInfo->SelectedLevel;
//
//
//
//
//	pDeviceSettings->sd.SampleDesc.Count = pDeviceSettingsCombo->multiSampleCountList.GetAt(bestMSAAIndex);
//	if (pDeviceSettings->sd.SampleDesc.Quality > pDeviceSettingsCombo->multiSampleQualityList.GetAt(bestMSAAIndex) - 1)
//		pDeviceSettings->sd.SampleDesc.Quality = pDeviceSettingsCombo->multiSampleQualityList.GetAt(bestMSAAIndex) - 1;
//
//	pDeviceSettings->sd.BufferDesc.Format = pDeviceSettingsCombo->BackBufferFormat;
//
//	return S_OK;
//}

// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myDxutApp.h"
#include "myResource.h"
#include "mySound.h"
#include "libc.h"
extern "C" {
#include <lua.h>
}
#include <luabind/error.hpp>

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
			DxutApp::getSingleton().Cleanup3DEnvironment();
			lResult = 0;
			return TRUE;

		case WM_SIZE:
			switch(wParam)
			{
			case SIZE_MINIMIZED:
				m_Minimized = true;
				break;

			case SIZE_MAXIMIZED:
				DxutApp::getSingleton().CheckForWindowSizeChange();
				m_Maximized = true;
				break;

			case SIZE_RESTORED:
				if(!m_InSizeMove)
				{
					DxutApp::getSingleton().CheckForWindowSizeChange();
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
			DxutApp::getSingleton().CheckForWindowSizeChange();
			m_InSizeMove = false;
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
					DxutApp::getSingleton().ToggleFullScreen();
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
			lResult = DxutApp::getSingleton().MsgProc(hWnd, uMsg, wParam, lParam, &bNoFurtherProcessing);
			if(bNoFurtherProcessing)
			{
				switch(uMsg)
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

Clock::Clock(void)
	: m_llQPFTicksPerSec(0)
	, m_llLastElapsedTime(0)
	, m_fAbsoluteTime(0)
	, m_fTimeScale(1.0f)
	, m_MaxAllowedTimestep(0.1f)
	, m_fElapsedTime(0)
	, m_fTotalTime(0)
{
	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency(&qwTicksPerSec);
	m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;

	LARGE_INTEGER qwTime;
	QueryPerformanceCounter(&qwTime);
	m_llLastElapsedTime = qwTime.QuadPart;

	UpdateClock();
}

void Clock::UpdateClock(void)
{
	LARGE_INTEGER qwTime;
	QueryPerformanceCounter(&qwTime);

	m_fAbsoluteTime = qwTime.QuadPart / (double)m_llQPFTicksPerSec;

	m_fUnscaledElapsedTime = Min(m_MaxAllowedTimestep, (float)((qwTime.QuadPart - m_llLastElapsedTime) / (double)m_llQPFTicksPerSec));

	m_fElapsedTime = m_fUnscaledElapsedTime * m_fTimeScale;

	m_fTotalTime += m_fElapsedTime;

	m_llLastElapsedTime = qwTime.QuadPart;
}

HRESULT D3DContext::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	CriticalSectionLock lock(m_DeviceObjectsSec);

	m_DeviceObjectsCreated = true;

	return S_OK;
}

HRESULT D3DContext::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	CriticalSectionLock lock(m_DeviceObjectsSec);

	m_DeviceObjectsReset = true;

	DeviceResourceBaseSet::iterator device_iter = m_DeviceObjects.begin();
	for (; device_iter != m_DeviceObjects.end(); device_iter++)
	{
		(*device_iter)->OnResetDevice();
	}
	return S_OK;
}

void D3DContext::OnLostDevice(void)
{
	m_BackBuffer.Release();

	m_DepthStencil.Release();

	CriticalSectionLock lock(m_DeviceObjectsSec);

	m_DeviceObjectsReset = false;

	DeviceResourceBaseSet::iterator device_iter = m_DeviceObjects.begin();
	for (; device_iter != m_DeviceObjects.end(); device_iter++)
	{
		(*device_iter)->OnLostDevice();
	}
}

void D3DContext::OnDestroyDevice(void)
{
	CriticalSectionLock lock(m_DeviceObjectsSec);

	m_DeviceObjectsCreated = false;

	DeviceResourceBaseSet::iterator device_iter = m_DeviceObjects.begin();
	for (; device_iter != m_DeviceObjects.end(); device_iter++)
	{
		(*device_iter)->OnDestroyDevice();
	}
}

bool D3DContext::RegisterNamedObject(const char * Name, NamedObject * Object)
{
	CriticalSectionLock lock(m_NamedObjectsSec);

	std::pair<NamedObjectMap::iterator, bool> result = m_NamedObjects.insert(NamedObjectMap::value_type(Name, Object));
	if (!result.second)
	{
		return false;
	}

	_ASSERT(!Object->m_Name);

	Object->m_Name = result.first->first.c_str();

	return true;
}

bool D3DContext::UnregisterNamedObject(const char * Name, NamedObject * Object)
{
	CriticalSectionLock lock(m_NamedObjectsSec);

	NamedObjectMap::iterator obj_iter = m_NamedObjects.find(Name);
	if (obj_iter != m_NamedObjects.end())
	{
		_ASSERT(Object == obj_iter->second && Object->m_Name == obj_iter->first.c_str());

		m_NamedObjects.erase(obj_iter);

		Object->m_Name = NULL;

		return true;
	}

	return false;
}

NamedObject * D3DContext::GetNamedObject(const std::string & Name)
{
	CriticalSectionLock lock(m_NamedObjectsSec);

	NamedObjectMap::const_iterator obj_iter = m_NamedObjects.find(Name);
	if (obj_iter != m_NamedObjects.end())
	{
		return obj_iter->second;
	}

	return NULL;
}

bool DxutApp::IsDeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	return true;
}

bool DxutApp::ModifyDeviceSettings(
	DXUTD3D9DeviceSettings * pDeviceSettings)
{
	return true;
}

void DxutApp::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	V(m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0));

	Present(0, 0, 0, 0);
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

int DxutApp::Run(void)
{
	m_wnd = DxutWindowPtr(new DxutWindow());
	CRect desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	CRect clientRect(0, 0, m_WindowBackBufferWidthAtModeChange, m_WindowBackBufferHeightAtModeChange);
	AdjustWindowRect(&clientRect, DxutWindow::GetWndStyle(0), FALSE);
	clientRect.MoveToXY((desktopRect.Width() - clientRect.Width()) / 2, (desktopRect.Height() - clientRect.Height()) / 2);
	m_wnd->Create(NULL, clientRect, GetModuleFileName().c_str());

	MSG msg = {0};

	try
	{
		_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

		m_d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
		if(!m_d3d9)
		{
			THROW_CUSEXCEPTION("cannnot create direct3d9");
		}

		CreateDevice(m_WindowedModeAtFirstCreate, m_WindowBackBufferWidthAtModeChange, m_WindowBackBufferHeightAtModeChange, m_RefreshRateAtFirstCreate, m_PresentIntervalAtFirstCreate);

		m_wnd->ShowWindow(SW_SHOW);
		m_wnd->UpdateWindow();

		msg.message = WM_NULL;
		while(WM_QUIT != msg.message)
		{
			if(::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_USER + 1024)
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
			else
			{
				Render3DEnvironment();
			}
		}

		_ASSERT(!m_DeviceObjectsCreated);
	}
	catch(const my::Exception & e)
	{
		::MessageBox(m_wnd->m_hWnd, ms2ts(e.what().c_str()).c_str(), NULL, MB_OK);
	}
	catch(const luabind::error & e)
	{
		::MessageBox(m_wnd->m_hWnd, ms2ts(lua_tostring(e.state(), -1)).c_str(), NULL, MB_OK);
	}
	catch (const luabind::cast_failed & e)
	{
		std::string msg(e.what());
		msg.append(": ");
		msg.append(e.info().name());
		::MessageBox(m_wnd->m_hWnd, ms2ts(msg.c_str()).c_str(), NULL, MB_OK);
	}
	catch(const std::exception & e)
	{
		::MessageBox(m_wnd->m_hWnd, ms2ts(e.what()).c_str(), NULL, MB_OK);
	}

	return (int)msg.wParam;
}

HRESULT DxutApp::DXUTFindValidD3D9DeviceSettings(
	DXUTD3D9DeviceSettings * pOut,
	DXUTD3D9DeviceSettings * pIn,
	DXUTMatchOptions * pMatchOptions,
	DXUTD3D9DeviceSettings * pOptimal)
{
    // Find the best combination of:
    //      Adapter Ordinal
    //      Device Type
    //      Adapter Format
    //      Back Buffer Format
    //      Windowed
    // given what's available on the system and the match options combined with the device settings input.
    // This combination of settings is encapsulated by the CD3D9EnumDeviceSettingsCombo class.
    float fBestRanking = -1.0f;
    CD3D9EnumDeviceSettingsCombo* pBestDeviceSettingsCombo = NULL;
    D3DDISPLAYMODE adapterDesktopDisplayMode;

	if( !HasEnumerated() )
		Enumerate( m_d3d9, m_wnd->m_hWnd );

    CGrowableArray <CD3D9EnumAdapterInfo*>* pAdapterList = GetAdapterInfoList();
    for( int iAdapter = 0; iAdapter < pAdapterList->GetSize(); iAdapter++ )
    {
        CD3D9EnumAdapterInfo* pAdapterInfo = pAdapterList->GetAt( iAdapter );

        // Get the desktop display mode of adapter 
        m_d3d9->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );

        // Enum all the device types supported by this adapter to find the best device settings
        for( int iDeviceInfo = 0; iDeviceInfo < pAdapterInfo->deviceInfoList.GetSize(); iDeviceInfo++ )
        {
            CD3D9EnumDeviceInfo* pDeviceInfo = pAdapterInfo->deviceInfoList.GetAt( iDeviceInfo );

            // Enum all the device settings combinations.  A device settings combination is 
            // a unique set of an adapter format, back buffer format, and IsWindowed.
            for( int iDeviceCombo = 0; iDeviceCombo < pDeviceInfo->deviceSettingsComboList.GetSize(); iDeviceCombo++ )
            {
                CD3D9EnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList.GetAt(
                    iDeviceCombo );

                // If windowed mode the adapter format has to be the same as the desktop 
                // display mode format so skip any that don't match
                if( pDeviceSettingsCombo->Windowed &&
                    ( pDeviceSettingsCombo->AdapterFormat != adapterDesktopDisplayMode.Format ) )
                    continue;

                // Skip any combo that doesn't meet the preserve match options
                if( false == DXUTDoesD3D9DeviceComboMatchPreserveOptions( pDeviceSettingsCombo, pIn, pMatchOptions ) )
                    continue;

                // Get a ranking number that describes how closely this device combo matches the optimal combo
                float fCurRanking = DXUTRankD3D9DeviceCombo( pDeviceSettingsCombo,
                                                             pOptimal, &adapterDesktopDisplayMode );

                // If this combo better matches the input device settings then save it
                if( fCurRanking > fBestRanking )
                {
                    pBestDeviceSettingsCombo = pDeviceSettingsCombo;
                    fBestRanking = fCurRanking;
                }
            }
        }
    }

    // If no best device combination was found then fail
    if( pBestDeviceSettingsCombo == NULL )
        return DXUTERR_NOCOMPATIBLEDEVICES;

    // Using the best device settings combo found, build valid device settings taking heed of 
    // the match options and the input device settings
    DXUTD3D9DeviceSettings validDeviceSettings;
    DXUTBuildValidD3D9DeviceSettings( &validDeviceSettings, pBestDeviceSettingsCombo, pIn, pMatchOptions );
    *pOut = validDeviceSettings;

    return S_OK;
}

void DxutApp::DXUTBuildOptimalD3D9DeviceSettings(
	DXUTD3D9DeviceSettings * pOptimalDeviceSettings,
	DXUTD3D9DeviceSettings * pDeviceSettingsIn,
	DXUTMatchOptions * pMatchOptions)
{
    D3DDISPLAYMODE adapterDesktopDisplayMode;

    ZeroMemory( pOptimalDeviceSettings, sizeof( DXUTD3D9DeviceSettings ) );

    //---------------------
    // Adapter ordinal
    //---------------------    
    if( pMatchOptions->eAdapterOrdinal == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->AdapterOrdinal = D3DADAPTER_DEFAULT;
    else
        pOptimalDeviceSettings->AdapterOrdinal = pDeviceSettingsIn->AdapterOrdinal;

    //---------------------
    // Device type
    //---------------------
    if( pMatchOptions->eDeviceType == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->DeviceType = D3DDEVTYPE_HAL;
    else
        pOptimalDeviceSettings->DeviceType = pDeviceSettingsIn->DeviceType;

    //---------------------
    // Windowed
    //---------------------
    if( pMatchOptions->eWindowed == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.Windowed = TRUE;
    else
        pOptimalDeviceSettings->pp.Windowed = pDeviceSettingsIn->pp.Windowed;

    //---------------------
    // Adapter format
    //---------------------
    if( pMatchOptions->eAdapterFormat == DXUTMT_IGNORE_INPUT )
    {
        // If windowed, default to the desktop display mode
        // If fullscreen, default to the desktop display mode for quick mode change or 
        // default to D3DFMT_X8R8G8B8 if the desktop display mode is < 32bit
        m_d3d9->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
        if( pOptimalDeviceSettings->pp.Windowed || DXUTGetD3D9ColorChannelBits( adapterDesktopDisplayMode.Format ) >=
            8 )
            pOptimalDeviceSettings->AdapterFormat = adapterDesktopDisplayMode.Format;
        else
            pOptimalDeviceSettings->AdapterFormat = D3DFMT_X8R8G8B8;
    }
    else
    {
        pOptimalDeviceSettings->AdapterFormat = pDeviceSettingsIn->AdapterFormat;
    }

    //---------------------
    // Vertex processing
    //---------------------
    if( pMatchOptions->eVertexProcessing == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        pOptimalDeviceSettings->BehaviorFlags = pDeviceSettingsIn->BehaviorFlags;

    //---------------------
    // Resolution
    //---------------------
    if( pMatchOptions->eResolution == DXUTMT_IGNORE_INPUT )
    {
        // If windowed, default to 640x480
        // If fullscreen, default to the desktop res for quick mode change
        if( pOptimalDeviceSettings->pp.Windowed )
        {
            pOptimalDeviceSettings->pp.BackBufferWidth = 640;
            pOptimalDeviceSettings->pp.BackBufferHeight = 480;
        }
        else
        {
            m_d3d9->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
            pOptimalDeviceSettings->pp.BackBufferWidth = adapterDesktopDisplayMode.Width;
            pOptimalDeviceSettings->pp.BackBufferHeight = adapterDesktopDisplayMode.Height;
        }
    }
    else
    {
        pOptimalDeviceSettings->pp.BackBufferWidth = pDeviceSettingsIn->pp.BackBufferWidth;
        pOptimalDeviceSettings->pp.BackBufferHeight = pDeviceSettingsIn->pp.BackBufferHeight;
    }

    //---------------------
    // Back buffer format
    //---------------------
    if( pMatchOptions->eBackBufferFormat == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.BackBufferFormat = pOptimalDeviceSettings->AdapterFormat; // Default to match the adapter format
    else
        pOptimalDeviceSettings->pp.BackBufferFormat = pDeviceSettingsIn->pp.BackBufferFormat;

    //---------------------
    // Back buffer count
    //---------------------
    if( pMatchOptions->eBackBufferCount == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.BackBufferCount = 1; // Default to double buffering.  Causes less latency than triple buffering
    else
        pOptimalDeviceSettings->pp.BackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;

    //---------------------
    // Multisample
    //---------------------
    if( pMatchOptions->eMultiSample == DXUTMT_IGNORE_INPUT )
    {
        // Default to no multisampling 
        pOptimalDeviceSettings->pp.MultiSampleType = D3DMULTISAMPLE_NONE;
        pOptimalDeviceSettings->pp.MultiSampleQuality = 0;
    }
    else
    {
        pOptimalDeviceSettings->pp.MultiSampleType = pDeviceSettingsIn->pp.MultiSampleType;
        pOptimalDeviceSettings->pp.MultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
    }

    //---------------------
    // Swap effect
    //---------------------
    if( pMatchOptions->eSwapEffect == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    else
        pOptimalDeviceSettings->pp.SwapEffect = pDeviceSettingsIn->pp.SwapEffect;

    //---------------------
    // Depth stencil 
    //---------------------
    if( pMatchOptions->eDepthFormat == DXUTMT_IGNORE_INPUT &&
        pMatchOptions->eStencilFormat == DXUTMT_IGNORE_INPUT )
    {
        UINT nBackBufferBits = DXUTGetD3D9ColorChannelBits( pOptimalDeviceSettings->pp.BackBufferFormat );
        if( nBackBufferBits >= 8 )
            pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D32;
        else
            pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D16;
    }
    else
    {
        pOptimalDeviceSettings->pp.AutoDepthStencilFormat = pDeviceSettingsIn->pp.AutoDepthStencilFormat;
    }

    //---------------------
    // Present flags
    //---------------------
    if( pMatchOptions->ePresentFlags == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    else
        pOptimalDeviceSettings->pp.Flags = pDeviceSettingsIn->pp.Flags;

    //---------------------
    // Refresh rate
    //---------------------
    if( pMatchOptions->eRefreshRate == DXUTMT_IGNORE_INPUT )
        pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = 0;
    else
        pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;

    //---------------------
    // Present interval
    //---------------------
    if( pMatchOptions->ePresentInterval == DXUTMT_IGNORE_INPUT )
    {
        // For windowed and fullscreen, default to D3DPRESENT_INTERVAL_DEFAULT
        // which will wait for the vertical retrace period to prevent tearing.
        // For benchmarking, use D3DPRESENT_INTERVAL_IMMEDIATE which will
        // will wait not for the vertical retrace period but may introduce tearing.
        pOptimalDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }
    else
    {
        pOptimalDeviceSettings->pp.PresentationInterval = pDeviceSettingsIn->pp.PresentationInterval;
    }
}

bool DxutApp::DXUTDoesD3D9DeviceComboMatchPreserveOptions(
	CD3D9EnumDeviceSettingsCombo * pDeviceSettingsCombo,
	DXUTD3D9DeviceSettings * pDeviceSettingsIn,
	DXUTMatchOptions * pMatchOptions)
{
    //---------------------
    // Adapter ordinal
    //---------------------
    if( pMatchOptions->eAdapterOrdinal == DXUTMT_PRESERVE_INPUT &&
        ( pDeviceSettingsCombo->AdapterOrdinal != pDeviceSettingsIn->AdapterOrdinal ) )
        return false;

    //---------------------
    // Device type
    //---------------------
    if( pMatchOptions->eDeviceType == DXUTMT_PRESERVE_INPUT &&
        ( pDeviceSettingsCombo->DeviceType != pDeviceSettingsIn->DeviceType ) )
        return false;

    //---------------------
    // Windowed
    //---------------------
    if( pMatchOptions->eWindowed == DXUTMT_PRESERVE_INPUT &&
        ( pDeviceSettingsCombo->Windowed != pDeviceSettingsIn->pp.Windowed ) )
        return false;

    //---------------------
    // Adapter format
    //---------------------
    if( pMatchOptions->eAdapterFormat == DXUTMT_PRESERVE_INPUT &&
        ( pDeviceSettingsCombo->AdapterFormat != pDeviceSettingsIn->AdapterFormat ) )
        return false;

    //---------------------
    // Vertex processing
    //---------------------
    // If keep VP and input has HWVP, then skip if this combo doesn't have HWTL 
    if( pMatchOptions->eVertexProcessing == DXUTMT_PRESERVE_INPUT &&
        ( ( pDeviceSettingsIn->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) != 0 ) &&
        ( ( pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ) )
        return false;

    //---------------------
    // Resolution
    //---------------------
    // If keep resolution then check that width and height supported by this combo
    if( pMatchOptions->eResolution == DXUTMT_PRESERVE_INPUT )
    {
        bool bFound = false;
        for( int i = 0; i < pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); i++ )
        {
            D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( i );
            if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
                continue; // Skip this display mode if it doesn't match the combo's adapter format

            if( displayMode.Width == pDeviceSettingsIn->pp.BackBufferWidth &&
                displayMode.Height == pDeviceSettingsIn->pp.BackBufferHeight )
            {
                bFound = true;
                break;
            }
        }

        // If the width and height are not supported by this combo, return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Back buffer format
    //---------------------
    if( pMatchOptions->eBackBufferFormat == DXUTMT_PRESERVE_INPUT &&
        pDeviceSettingsCombo->BackBufferFormat != pDeviceSettingsIn->pp.BackBufferFormat )
        return false;

    //---------------------
    // Back buffer count
    //---------------------
    // No caps for the back buffer count

    //---------------------
    // Multisample
    //---------------------
    if( pMatchOptions->eMultiSample == DXUTMT_PRESERVE_INPUT )
    {
        bool bFound = false;
        for( int i = 0; i < pDeviceSettingsCombo->multiSampleTypeList.GetSize(); i++ )
        {
            D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList.GetAt( i );
            DWORD msQuality = pDeviceSettingsCombo->multiSampleQualityList.GetAt( i );

            if( msType == pDeviceSettingsIn->pp.MultiSampleType &&
                msQuality > pDeviceSettingsIn->pp.MultiSampleQuality )
            {
                bFound = true;
                break;
            }
        }

        // If multisample type/quality not supported by this combo, then return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Swap effect
    //---------------------
    // No caps for swap effects

    //---------------------
    // Depth stencil 
    //---------------------
    // If keep depth stencil format then check that the depth stencil format is supported by this combo
    if( pMatchOptions->eDepthFormat == DXUTMT_PRESERVE_INPUT &&
        pMatchOptions->eStencilFormat == DXUTMT_PRESERVE_INPUT )
    {
        if( pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN &&
            !pDeviceSettingsCombo->depthStencilFormatList.Contains( pDeviceSettingsIn->pp.AutoDepthStencilFormat ) )
            return false;
    }

    // If keep depth format then check that the depth format is supported by this combo
    if( pMatchOptions->eDepthFormat == DXUTMT_PRESERVE_INPUT &&
        pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
    {
        bool bFound = false;
        UINT dwDepthBits = DXUTGetDepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
        for( int i = 0; i < pDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
        {
            D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList.GetAt( i );
            UINT dwCurDepthBits = DXUTGetDepthBits( depthStencilFmt );
            if( dwCurDepthBits - dwDepthBits == 0 )
                bFound = true;
        }

        if( !bFound )
            return false;
    }

    // If keep depth format then check that the depth format is supported by this combo
    if( pMatchOptions->eStencilFormat == DXUTMT_PRESERVE_INPUT &&
        pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
    {
        bool bFound = false;
        UINT dwStencilBits = DXUTGetStencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
        for( int i = 0; i < pDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
        {
            D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList.GetAt( i );
            UINT dwCurStencilBits = DXUTGetStencilBits( depthStencilFmt );
            if( dwCurStencilBits - dwStencilBits == 0 )
                bFound = true;
        }

        if( !bFound )
            return false;
    }

    //---------------------
    // Present flags
    //---------------------
    // No caps for the present flags

    //---------------------
    // Refresh rate
    //---------------------
    // If keep refresh rate then check that the resolution is supported by this combo
    if( pMatchOptions->eRefreshRate == DXUTMT_PRESERVE_INPUT )
    {
        bool bFound = false;
        for( int i = 0; i < pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); i++ )
        {
            D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( i );
            if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
                continue;
            if( displayMode.RefreshRate == pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz )
            {
                bFound = true;
                break;
            }
        }

        // If refresh rate not supported by this combo, then return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Present interval
    //---------------------
    // If keep present interval then check that the present interval is supported by this combo
    if( pMatchOptions->ePresentInterval == DXUTMT_PRESERVE_INPUT &&
        !pDeviceSettingsCombo->presentIntervalList.Contains( pDeviceSettingsIn->pp.PresentationInterval ) )
        return false;

    return true;
}

float DxutApp::DXUTRankD3D9DeviceCombo(
	CD3D9EnumDeviceSettingsCombo * pDeviceSettingsCombo,
	DXUTD3D9DeviceSettings * pOptimalDeviceSettings,
	D3DDISPLAYMODE * pAdapterDesktopDisplayMode)
{
    float fCurRanking = 0.0f;

    // Arbitrary weights.  Gives preference to the ordinal, device type, and windowed
    const float fAdapterOrdinalWeight = 1000.0f;
    const float fDeviceTypeWeight = 100.0f;
    const float fWindowWeight = 10.0f;
    const float fAdapterFormatWeight = 1.0f;
    const float fVertexProcessingWeight = 1.0f;
    const float fResolutionWeight = 1.0f;
    const float fBackBufferFormatWeight = 1.0f;
    const float fMultiSampleWeight = 1.0f;
    const float fDepthStencilWeight = 1.0f;
    const float fRefreshRateWeight = 1.0f;
    const float fPresentIntervalWeight = 1.0f;

    //---------------------
    // Adapter ordinal
    //---------------------
    if( pDeviceSettingsCombo->AdapterOrdinal == pOptimalDeviceSettings->AdapterOrdinal )
        fCurRanking += fAdapterOrdinalWeight;

    //---------------------
    // Device type
    //---------------------
    if( pDeviceSettingsCombo->DeviceType == pOptimalDeviceSettings->DeviceType )
        fCurRanking += fDeviceTypeWeight;
    // Slightly prefer HAL 
    if( pDeviceSettingsCombo->DeviceType == D3DDEVTYPE_HAL )
        fCurRanking += 0.1f;

    //---------------------
    // Windowed
    //---------------------
    if( pDeviceSettingsCombo->Windowed == pOptimalDeviceSettings->pp.Windowed )
        fCurRanking += fWindowWeight;

    //---------------------
    // Adapter format
    //---------------------
    if( pDeviceSettingsCombo->AdapterFormat == pOptimalDeviceSettings->AdapterFormat )
    {
        fCurRanking += fAdapterFormatWeight;
    }
    else
    {
        int nBitDepthDelta = abs( ( long )DXUTGetD3D9ColorChannelBits( pDeviceSettingsCombo->AdapterFormat ) -
                                  ( long )DXUTGetD3D9ColorChannelBits( pOptimalDeviceSettings->AdapterFormat ) );
        float fScale = __max( 0.9f - ( float )nBitDepthDelta * 0.2f, 0.0f );
        fCurRanking += fScale * fAdapterFormatWeight;
    }

    if( !pDeviceSettingsCombo->Windowed )
    {
        // Slightly prefer when it matches the desktop format or is D3DFMT_X8R8G8B8
        bool bAdapterOptimalMatch;
        if( DXUTGetD3D9ColorChannelBits( pAdapterDesktopDisplayMode->Format ) >= 8 )
            bAdapterOptimalMatch = ( pDeviceSettingsCombo->AdapterFormat == pAdapterDesktopDisplayMode->Format );
        else
            bAdapterOptimalMatch = ( pDeviceSettingsCombo->AdapterFormat == D3DFMT_X8R8G8B8 );

        if( bAdapterOptimalMatch )
            fCurRanking += 0.1f;
    }

    //---------------------
    // Vertex processing
    //---------------------
    if( ( pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) != 0 ||
        ( pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING ) != 0 )
    {
        if( ( pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0 )
            fCurRanking += fVertexProcessingWeight;
    }
    // Slightly prefer HW T&L
    if( ( pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0 )
        fCurRanking += 0.1f;

    //---------------------
    // Resolution
    //---------------------
    bool bResolutionFound = false;
    for( int idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); idm++ )
    {
        D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( idm );
        if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
            continue;
        if( displayMode.Width == pOptimalDeviceSettings->pp.BackBufferWidth &&
            displayMode.Height == pOptimalDeviceSettings->pp.BackBufferHeight )
            bResolutionFound = true;
    }
    if( bResolutionFound )
        fCurRanking += fResolutionWeight;

    //---------------------
    // Back buffer format
    //---------------------
    if( pDeviceSettingsCombo->BackBufferFormat == pOptimalDeviceSettings->pp.BackBufferFormat )
    {
        fCurRanking += fBackBufferFormatWeight;
    }
    else
    {
        int nBitDepthDelta = abs( ( long )DXUTGetD3D9ColorChannelBits( pDeviceSettingsCombo->BackBufferFormat ) -
                                  ( long )DXUTGetD3D9ColorChannelBits( pOptimalDeviceSettings->pp.BackBufferFormat ) );
        float fScale = __max( 0.9f - ( float )nBitDepthDelta * 0.2f, 0.0f );
        fCurRanking += fScale * fBackBufferFormatWeight;
    }

    // Check if this back buffer format is the same as 
    // the adapter format since this is preferred.
    bool bAdapterMatchesBB = ( pDeviceSettingsCombo->BackBufferFormat == pDeviceSettingsCombo->AdapterFormat );
    if( bAdapterMatchesBB )
        fCurRanking += 0.1f;

    //---------------------
    // Back buffer count
    //---------------------
    // No caps for the back buffer count

    //---------------------
    // Multisample
    //---------------------
    bool bMultiSampleFound = false;
    for( int i = 0; i < pDeviceSettingsCombo->multiSampleTypeList.GetSize(); i++ )
    {
        D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList.GetAt( i );
        DWORD msQuality = pDeviceSettingsCombo->multiSampleQualityList.GetAt( i );

        if( msType == pOptimalDeviceSettings->pp.MultiSampleType &&
            msQuality > pOptimalDeviceSettings->pp.MultiSampleQuality )
        {
            bMultiSampleFound = true;
            break;
        }
    }
    if( bMultiSampleFound )
        fCurRanking += fMultiSampleWeight;

    //---------------------
    // Swap effect
    //---------------------
    // No caps for swap effects

    //---------------------
    // Depth stencil 
    //---------------------
    if( pDeviceSettingsCombo->depthStencilFormatList.Contains( pOptimalDeviceSettings->pp.AutoDepthStencilFormat ) )
        fCurRanking += fDepthStencilWeight;

    //---------------------
    // Present flags
    //---------------------
    // No caps for the present flags

    //---------------------
    // Refresh rate
    //---------------------
    bool bRefreshFound = false;
    for( int idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetSize(); idm++ )
    {
        D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList.GetAt( idm );
        if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
            continue;
        if( displayMode.RefreshRate == pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz )
            bRefreshFound = true;
    }
    if( bRefreshFound )
        fCurRanking += fRefreshRateWeight;

    //---------------------
    // Present interval
    //---------------------
    // If keep present interval then check that the present interval is supported by this combo
    if( pDeviceSettingsCombo->presentIntervalList.Contains( pOptimalDeviceSettings->pp.PresentationInterval ) )
        fCurRanking += fPresentIntervalWeight;

    return fCurRanking;
}

void DxutApp::DXUTBuildValidD3D9DeviceSettings(
	DXUTD3D9DeviceSettings * pValidDeviceSettings,
	CD3D9EnumDeviceSettingsCombo * pBestDeviceSettingsCombo,
	DXUTD3D9DeviceSettings * pDeviceSettingsIn,
	DXUTMatchOptions * pMatchOptions)
{
	D3DDISPLAYMODE adapterDesktopDisplayMode;
	m_d3d9->GetAdapterDisplayMode( pBestDeviceSettingsCombo->AdapterOrdinal, &adapterDesktopDisplayMode );

	// For each setting pick the best, taking into account the match options and 
	// what's supported by the device

	//---------------------
	// Adapter Ordinal
	//---------------------
	// Just using pBestDeviceSettingsCombo->AdapterOrdinal

	//---------------------
	// Device Type
	//---------------------
	// Just using pBestDeviceSettingsCombo->DeviceType

	//---------------------
	// Windowed 
	//---------------------
	// Just using pBestDeviceSettingsCombo->Windowed

	//---------------------
	// Adapter Format
	//---------------------
	// Just using pBestDeviceSettingsCombo->AdapterFormat

	//---------------------
	// Vertex processing
	//---------------------
	DWORD dwBestBehaviorFlags = 0;
	if( pMatchOptions->eVertexProcessing == DXUTMT_PRESERVE_INPUT )
	{
		dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
	}
	else if( pMatchOptions->eVertexProcessing == DXUTMT_IGNORE_INPUT )
	{
		// The framework defaults to HWVP if available otherwise use SWVP
		if( ( pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0 )
			dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else // if( pMatchOptions->eVertexProcessing == DXUTMT_CLOSEST_TO_INPUT )    
	{
		// Default to input, and fallback to SWVP if HWVP not available 
		dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
		if( ( pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 &&
			( ( dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) != 0 ||
			( dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING ) != 0 ) )
		{
			dwBestBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			dwBestBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
			dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// One of these must be selected
		if( ( dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) == 0 &&
			( dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING ) == 0 &&
			( dwBestBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING ) == 0 )
		{
			if( ( pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0 )
				dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
			else
				dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}

	//---------------------
	// Resolution
	//---------------------
	D3DDISPLAYMODE bestDisplayMode;
	if( pMatchOptions->eResolution == DXUTMT_PRESERVE_INPUT )
	{
		bestDisplayMode.Width = pDeviceSettingsIn->pp.BackBufferWidth;
		bestDisplayMode.Height = pDeviceSettingsIn->pp.BackBufferHeight;
	}
	else
	{
		D3DDISPLAYMODE displayModeIn;
		if( pMatchOptions->eResolution == DXUTMT_CLOSEST_TO_INPUT &&
			pDeviceSettingsIn )
		{
			displayModeIn.Width = pDeviceSettingsIn->pp.BackBufferWidth;
			displayModeIn.Height = pDeviceSettingsIn->pp.BackBufferHeight;
		}
		else // if( pMatchOptions->eResolution == DXUTMT_IGNORE_INPUT )   
		{
			if( pBestDeviceSettingsCombo->Windowed )
			{
				// The framework defaults to 640x480 for windowed
				displayModeIn.Width = 640;
				displayModeIn.Height = 480;
			}
			else
			{
				// The framework defaults to desktop resolution for fullscreen to try to avoid slow mode change
				displayModeIn.Width = adapterDesktopDisplayMode.Width;
				displayModeIn.Height = adapterDesktopDisplayMode.Height;
			}
		}

		// Call a helper function to find the closest valid display mode to the optimal 
		DXUTFindValidD3D9Resolution( pBestDeviceSettingsCombo, displayModeIn, &bestDisplayMode );
	}

	//---------------------
	// Back Buffer Format
	//---------------------
	// Just using pBestDeviceSettingsCombo->BackBufferFormat

	//---------------------
	// Back buffer count
	//---------------------
	UINT bestBackBufferCount;
	if( pMatchOptions->eBackBufferCount == DXUTMT_PRESERVE_INPUT )
	{
		bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
	}
	else if( pMatchOptions->eBackBufferCount == DXUTMT_IGNORE_INPUT )
	{
		// Default to double buffering.  Causes less latency than triple buffering
		bestBackBufferCount = 1;
	}
	else // if( pMatchOptions->eBackBufferCount == DXUTMT_CLOSEST_TO_INPUT )   
	{
		bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
		if( bestBackBufferCount > 3 )
			bestBackBufferCount = 3;
		if( bestBackBufferCount < 1 )
			bestBackBufferCount = 1;
	}

	//---------------------
	// Multisample
	//---------------------
	D3DMULTISAMPLE_TYPE bestMultiSampleType;
	DWORD bestMultiSampleQuality;
	if( pDeviceSettingsIn && pDeviceSettingsIn->pp.SwapEffect != D3DSWAPEFFECT_DISCARD )
	{
		// Swap effect is not set to discard so multisampling has to off
		bestMultiSampleType = D3DMULTISAMPLE_NONE;
		bestMultiSampleQuality = 0;
	}
	else
	{
		if( pMatchOptions->eMultiSample == DXUTMT_PRESERVE_INPUT )
		{
			bestMultiSampleType = pDeviceSettingsIn->pp.MultiSampleType;
			bestMultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
		}
		else if( pMatchOptions->eMultiSample == DXUTMT_IGNORE_INPUT )
		{
			// Default to no multisampling (always supported)
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;
		}
		else if( pMatchOptions->eMultiSample == DXUTMT_CLOSEST_TO_INPUT )
		{
			// Default to no multisampling (always supported)
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;

			for( int i = 0; i < pBestDeviceSettingsCombo->multiSampleTypeList.GetSize(); i++ )
			{
				D3DMULTISAMPLE_TYPE type = pBestDeviceSettingsCombo->multiSampleTypeList.GetAt( i );
				DWORD qualityLevels = pBestDeviceSettingsCombo->multiSampleQualityList.GetAt( i );

				// Check whether supported type is closer to the input than our current best
				if( abs( type - pDeviceSettingsIn->pp.MultiSampleType ) < abs( bestMultiSampleType -
					pDeviceSettingsIn->pp.MultiSampleType )
					)
				{
					bestMultiSampleType = type;
					bestMultiSampleQuality = __min( qualityLevels - 1, pDeviceSettingsIn->pp.MultiSampleQuality );
				}
			}
		}
		else
		{
			// Error case
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;
		}
	}

	//---------------------
	// Swap effect
	//---------------------
	D3DSWAPEFFECT bestSwapEffect;
	if( pMatchOptions->eSwapEffect == DXUTMT_PRESERVE_INPUT )
	{
		bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;
	}
	else if( pMatchOptions->eSwapEffect == DXUTMT_IGNORE_INPUT )
	{
		bestSwapEffect = D3DSWAPEFFECT_DISCARD;
	}
	else // if( pMatchOptions->eSwapEffect == DXUTMT_CLOSEST_TO_INPUT )   
	{
		bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;

		// Swap effect has to be one of these 3
		if( bestSwapEffect != D3DSWAPEFFECT_DISCARD &&
			bestSwapEffect != D3DSWAPEFFECT_FLIP &&
			bestSwapEffect != D3DSWAPEFFECT_COPY )
		{
			bestSwapEffect = D3DSWAPEFFECT_DISCARD;
		}
	}

	//---------------------
	// Depth stencil 
	//---------------------
	D3DFORMAT bestDepthStencilFormat;
	bool bestEnableAutoDepthStencil;

	CGrowableArray <int> depthStencilRanking;
	depthStencilRanking.SetSize( pBestDeviceSettingsCombo->depthStencilFormatList.GetSize() );

	UINT dwBackBufferBitDepth = DXUTGetD3D9ColorChannelBits( pBestDeviceSettingsCombo->BackBufferFormat );
	UINT dwInputDepthBitDepth = 0;
	if( pDeviceSettingsIn )
		dwInputDepthBitDepth = DXUTGetDepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

	for( int i = 0; i < pBestDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
	{
		D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList.GetAt( i );
		DWORD dwCurDepthBitDepth = DXUTGetDepthBits( curDepthStencilFmt );
		int nRanking;

		if( pMatchOptions->eDepthFormat == DXUTMT_PRESERVE_INPUT )
		{
			// Need to match bit depth of input
			if( dwCurDepthBitDepth == dwInputDepthBitDepth )
				nRanking = 0;
			else
				nRanking = 10000;
		}
		else if( pMatchOptions->eDepthFormat == DXUTMT_IGNORE_INPUT )
		{
			// Prefer match of backbuffer bit depth
			nRanking = abs( ( int )dwCurDepthBitDepth - ( int )dwBackBufferBitDepth * 4 );
		}
		else // if( pMatchOptions->eDepthFormat == DXUTMT_CLOSEST_TO_INPUT )
		{
			// Prefer match of input depth format bit depth
			nRanking = abs( ( int )dwCurDepthBitDepth - ( int )dwInputDepthBitDepth );
		}

		depthStencilRanking.Add( nRanking );
	}

	UINT dwInputStencilBitDepth = 0;
	if( pDeviceSettingsIn )
		dwInputStencilBitDepth = DXUTGetStencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

	for( int i = 0; i < pBestDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
	{
		D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList.GetAt( i );
		int nRanking = depthStencilRanking.GetAt( i );
		DWORD dwCurStencilBitDepth = DXUTGetStencilBits( curDepthStencilFmt );

		if( pMatchOptions->eStencilFormat == DXUTMT_PRESERVE_INPUT )
		{
			// Need to match bit depth of input
			if( dwCurStencilBitDepth == dwInputStencilBitDepth )
				nRanking += 0;
			else
				nRanking += 10000;
		}
		else if( pMatchOptions->eStencilFormat == DXUTMT_IGNORE_INPUT )
		{
			// Prefer 0 stencil bit depth
			nRanking += dwCurStencilBitDepth;
		}
		else // if( pMatchOptions->eStencilFormat == DXUTMT_CLOSEST_TO_INPUT )
		{
			// Prefer match of input stencil format bit depth
			nRanking += abs( ( int )dwCurStencilBitDepth - ( int )dwInputStencilBitDepth );
		}

		depthStencilRanking.SetAt( i, nRanking );
	}

	int nBestRanking = 100000;
	int nBestIndex = -1;
	for( int i = 0; i < pBestDeviceSettingsCombo->depthStencilFormatList.GetSize(); i++ )
	{
		int nRanking = depthStencilRanking.GetAt( i );
		if( nRanking < nBestRanking )
		{
			nBestRanking = nRanking;
			nBestIndex = i;
		}
	}

	if( nBestIndex >= 0 )
	{
		bestDepthStencilFormat = pBestDeviceSettingsCombo->depthStencilFormatList.GetAt( nBestIndex );
		bestEnableAutoDepthStencil = true;
	}
	else
	{
		bestDepthStencilFormat = D3DFMT_UNKNOWN;
		bestEnableAutoDepthStencil = false;
	}


	//---------------------
	// Present flags
	//---------------------
	DWORD dwBestFlags;
	if( pMatchOptions->ePresentFlags == DXUTMT_PRESERVE_INPUT )
	{
		dwBestFlags = pDeviceSettingsIn->pp.Flags;
	}
	else if( pMatchOptions->ePresentFlags == DXUTMT_IGNORE_INPUT )
	{
		dwBestFlags = 0;
		if( bestEnableAutoDepthStencil )
			dwBestFlags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	}
	else // if( pMatchOptions->ePresentFlags == DXUTMT_CLOSEST_TO_INPUT )   
	{
		dwBestFlags = pDeviceSettingsIn->pp.Flags;
		if( bestEnableAutoDepthStencil )
			dwBestFlags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	}

	//---------------------
	// Refresh rate
	//---------------------
	if( pBestDeviceSettingsCombo->Windowed )
	{
		// Must be 0 for windowed
		bestDisplayMode.RefreshRate = 0;
	}
	else
	{
		if( pMatchOptions->eRefreshRate == DXUTMT_PRESERVE_INPUT )
		{
			bestDisplayMode.RefreshRate = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
		}
		else
		{
			UINT refreshRateMatch;
			if( pMatchOptions->eRefreshRate == DXUTMT_CLOSEST_TO_INPUT )
			{
				refreshRateMatch = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
			}
			else // if( pMatchOptions->eRefreshRate == DXUTMT_IGNORE_INPUT )   
			{
				refreshRateMatch = adapterDesktopDisplayMode.RefreshRate;
			}

			bestDisplayMode.RefreshRate = 0;

			if( refreshRateMatch != 0 )
			{
				int nBestRefreshRanking = 100000;
				CGrowableArray <D3DDISPLAYMODE>* pDisplayModeList =
					&pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
				for( int iDisplayMode = 0; iDisplayMode < pDisplayModeList->GetSize(); iDisplayMode++ )
				{
					D3DDISPLAYMODE displayMode = pDisplayModeList->GetAt( iDisplayMode );
					if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat ||
						displayMode.Height != bestDisplayMode.Height ||
						displayMode.Width != bestDisplayMode.Width )
						continue; // Skip display modes that don't match 

					// Find the delta between the current refresh rate and the optimal refresh rate 
					int nCurRanking = abs( ( int )displayMode.RefreshRate - ( int )refreshRateMatch );

					if( nCurRanking < nBestRefreshRanking )
					{
						bestDisplayMode.RefreshRate = displayMode.RefreshRate;
						nBestRefreshRanking = nCurRanking;

						// Stop if perfect match found
						if( nBestRefreshRanking == 0 )
							break;
					}
				}
			}
		}
	}

	//---------------------
	// Present interval
	//---------------------
	UINT bestPresentInterval;
	if( pMatchOptions->ePresentInterval == DXUTMT_PRESERVE_INPUT )
	{
		bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
	}
	else if( pMatchOptions->ePresentInterval == DXUTMT_IGNORE_INPUT )
	{
		// For windowed and fullscreen, default to D3DPRESENT_INTERVAL_DEFAULT
		// which will wait for the vertical retrace period to prevent tearing.
		// For benchmarking, use D3DPRESENT_INTERVAL_DEFAULT  which will
		// will wait not for the vertical retrace period but may introduce tearing.
		bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}
	else // if( pMatchOptions->ePresentInterval == DXUTMT_CLOSEST_TO_INPUT )   
	{
		if( pBestDeviceSettingsCombo->presentIntervalList.Contains( pDeviceSettingsIn->pp.PresentationInterval ) )
		{
			bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
		}
		else
		{
			bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
	}

	// Fill the device settings struct
	ZeroMemory( pValidDeviceSettings, sizeof( DXUTD3D9DeviceSettings ) );
	pValidDeviceSettings->AdapterOrdinal = pBestDeviceSettingsCombo->AdapterOrdinal;
	pValidDeviceSettings->DeviceType = pBestDeviceSettingsCombo->DeviceType;
	pValidDeviceSettings->AdapterFormat = pBestDeviceSettingsCombo->AdapterFormat;
	pValidDeviceSettings->BehaviorFlags = dwBestBehaviorFlags;
	pValidDeviceSettings->pp.BackBufferWidth = bestDisplayMode.Width;
	pValidDeviceSettings->pp.BackBufferHeight = bestDisplayMode.Height;
	pValidDeviceSettings->pp.BackBufferFormat = pBestDeviceSettingsCombo->BackBufferFormat;
	pValidDeviceSettings->pp.BackBufferCount = bestBackBufferCount;
	pValidDeviceSettings->pp.MultiSampleType = bestMultiSampleType;
	pValidDeviceSettings->pp.MultiSampleQuality = bestMultiSampleQuality;
	pValidDeviceSettings->pp.SwapEffect = bestSwapEffect;
	pValidDeviceSettings->pp.hDeviceWindow = m_wnd->m_hWnd;
	pValidDeviceSettings->pp.Windowed = pBestDeviceSettingsCombo->Windowed;
	pValidDeviceSettings->pp.EnableAutoDepthStencil = bestEnableAutoDepthStencil;
	pValidDeviceSettings->pp.AutoDepthStencilFormat = bestDepthStencilFormat;
	pValidDeviceSettings->pp.Flags = dwBestFlags;
	pValidDeviceSettings->pp.FullScreen_RefreshRateInHz = bestDisplayMode.RefreshRate;
	pValidDeviceSettings->pp.PresentationInterval = bestPresentInterval;
}

HRESULT DxutApp::DXUTFindValidD3D9Resolution(
	CD3D9EnumDeviceSettingsCombo * pBestDeviceSettingsCombo,
	D3DDISPLAYMODE displayModeIn,
	D3DDISPLAYMODE * pBestDisplayMode)
{
	D3DDISPLAYMODE bestDisplayMode;
	ZeroMemory( &bestDisplayMode, sizeof( D3DDISPLAYMODE ) );

	if( pBestDeviceSettingsCombo->Windowed )
	{
		// In windowed mode, all resolutions are valid but restritions still apply 
		// on the size of the window.  See DXUTChangeD3D9Device() for details
		*pBestDisplayMode = displayModeIn;
	}
	else
	{
		int nBestRanking = 100000;
		int nCurRanking;
		CGrowableArray <D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
		for( int iDisplayMode = 0; iDisplayMode < pDisplayModeList->GetSize(); iDisplayMode++ )
		{
			D3DDISPLAYMODE displayMode = pDisplayModeList->GetAt( iDisplayMode );

			// Skip display modes that don't match the combo's adapter format
			if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat )
				continue;

			// Find the delta between the current width/height and the optimal width/height
			nCurRanking = abs( ( int )displayMode.Width - ( int )displayModeIn.Width ) +
				abs( ( int )displayMode.Height - ( int )displayModeIn.Height );

			if( nCurRanking < nBestRanking )
			{
				bestDisplayMode = displayMode;
				nBestRanking = nCurRanking;

				// Stop if perfect match found
				if( nBestRanking == 0 )
					break;
			}
		}

		if( bestDisplayMode.Width == 0 )
		{
			*pBestDisplayMode = displayModeIn;
			return E_FAIL; // No valid display modes found
		}

		*pBestDisplayMode = bestDisplayMode;
	}

	return S_OK;
}

const char * DxutApp::DXUTD3DDeviceTypeToString(D3DDEVTYPE devType)
{
	switch( devType )
	{
	case D3DDEVTYPE_HAL:
		return "D3DDEVTYPE_HAL";
	case D3DDEVTYPE_SW:
		return "D3DDEVTYPE_SW";
	case D3DDEVTYPE_REF:
		return "D3DDEVTYPE_REF";
	default:
		return "Unknown devType";
	}
}

const char * DxutApp::DXUTD3DFormatToString(D3DFORMAT format)
{
	switch( format )
	{
	case D3DFMT_UNKNOWN:
		return "D3DFMT_UNKNOWN";
	case D3DFMT_R8G8B8:
		return "D3DFMT_R8G8B8";
	case D3DFMT_A8R8G8B8:
		return "D3DFMT_A8R8G8B8";
	case D3DFMT_X8R8G8B8:
		return "D3DFMT_X8R8G8B8";
	case D3DFMT_R5G6B5:
		return "D3DFMT_R5G6B5";
	case D3DFMT_X1R5G5B5:
		return "D3DFMT_X1R5G5B5";
	case D3DFMT_A1R5G5B5:
		return "D3DFMT_A1R5G5B5";
	case D3DFMT_A4R4G4B4:
		return "D3DFMT_A4R4G4B4";
	case D3DFMT_R3G3B2:
		return "D3DFMT_R3G3B2";
	case D3DFMT_A8:
		return "D3DFMT_A8";
	case D3DFMT_A8R3G3B2:
		return "D3DFMT_A8R3G3B2";
	case D3DFMT_X4R4G4B4:
		return "D3DFMT_X4R4G4B4";
	case D3DFMT_A2B10G10R10:
		return "D3DFMT_A2B10G10R10";
	case D3DFMT_A8B8G8R8:
		return "D3DFMT_A8B8G8R8";
	case D3DFMT_X8B8G8R8:
		return "D3DFMT_X8B8G8R8";
	case D3DFMT_G16R16:
		return "D3DFMT_G16R16";
	case D3DFMT_A2R10G10B10:
		return "D3DFMT_A2R10G10B10";
	case D3DFMT_A16B16G16R16:
		return "D3DFMT_A16B16G16R16";
	case D3DFMT_A8P8:
		return "D3DFMT_A8P8";
	case D3DFMT_P8:
		return "D3DFMT_P8";
	case D3DFMT_L8:
		return "D3DFMT_L8";
	case D3DFMT_A8L8:
		return "D3DFMT_A8L8";
	case D3DFMT_A4L4:
		return "D3DFMT_A4L4";
	case D3DFMT_V8U8:
		return "D3DFMT_V8U8";
	case D3DFMT_L6V5U5:
		return "D3DFMT_L6V5U5";
	case D3DFMT_X8L8V8U8:
		return "D3DFMT_X8L8V8U8";
	case D3DFMT_Q8W8V8U8:
		return "D3DFMT_Q8W8V8U8";
	case D3DFMT_V16U16:
		return "D3DFMT_V16U16";
	case D3DFMT_A2W10V10U10:
		return "D3DFMT_A2W10V10U10";
	case D3DFMT_UYVY:
		return "D3DFMT_UYVY";
	case D3DFMT_YUY2:
		return "D3DFMT_YUY2";
	case D3DFMT_DXT1:
		return "D3DFMT_DXT1";
	case D3DFMT_DXT2:
		return "D3DFMT_DXT2";
	case D3DFMT_DXT3:
		return "D3DFMT_DXT3";
	case D3DFMT_DXT4:
		return "D3DFMT_DXT4";
	case D3DFMT_DXT5:
		return "D3DFMT_DXT5";
	case D3DFMT_D16_LOCKABLE:
		return "D3DFMT_D16_LOCKABLE";
	case D3DFMT_D32:
		return "D3DFMT_D32";
	case D3DFMT_D15S1:
		return "D3DFMT_D15S1";
	case D3DFMT_D24S8:
		return "D3DFMT_D24S8";
	case D3DFMT_D24X8:
		return "D3DFMT_D24X8";
	case D3DFMT_D24X4S4:
		return "D3DFMT_D24X4S4";
	case D3DFMT_D16:
		return "D3DFMT_D16";
	case D3DFMT_L16:
		return "D3DFMT_L16";
	case D3DFMT_VERTEXDATA:
		return "D3DFMT_VERTEXDATA";
	case D3DFMT_INDEX16:
		return "D3DFMT_INDEX16";
	case D3DFMT_INDEX32:
		return "D3DFMT_INDEX32";
	case D3DFMT_Q16W16V16U16:
		return "D3DFMT_Q16W16V16U16";
	case D3DFMT_MULTI2_ARGB8:
		return "D3DFMT_MULTI2_ARGB8";
	case D3DFMT_R16F:
		return "D3DFMT_R16F";
	case D3DFMT_G16R16F:
		return "D3DFMT_G16R16F";
	case D3DFMT_A16B16G16R16F:
		return "D3DFMT_A16B16G16R16F";
	case D3DFMT_R32F:
		return "D3DFMT_R32F";
	case D3DFMT_G32R32F:
		return "D3DFMT_G32R32F";
	case D3DFMT_A32B32G32R32F:
		return "D3DFMT_A32B32G32R32F";
	case D3DFMT_CxV8U8:
		return "D3DFMT_CxV8U8";
	default:
		return "Unknown format";
	}
}

const char * DxutApp::DXUTMultisampleTypeToString(D3DMULTISAMPLE_TYPE MultiSampleType)
{
	switch( MultiSampleType )
	{
	case D3DMULTISAMPLE_NONE:
		return "D3DMULTISAMPLE_NONE";
	case D3DMULTISAMPLE_NONMASKABLE:
		return "D3DMULTISAMPLE_NONMASKABLE";
	case D3DMULTISAMPLE_2_SAMPLES:
		return "D3DMULTISAMPLE_2_SAMPLES";
	case D3DMULTISAMPLE_3_SAMPLES:
		return "D3DMULTISAMPLE_3_SAMPLES";
	case D3DMULTISAMPLE_4_SAMPLES:
		return "D3DMULTISAMPLE_4_SAMPLES";
	case D3DMULTISAMPLE_5_SAMPLES:
		return "D3DMULTISAMPLE_5_SAMPLES";
	case D3DMULTISAMPLE_6_SAMPLES:
		return "D3DMULTISAMPLE_6_SAMPLES";
	case D3DMULTISAMPLE_7_SAMPLES:
		return "D3DMULTISAMPLE_7_SAMPLES";
	case D3DMULTISAMPLE_8_SAMPLES:
		return "D3DMULTISAMPLE_8_SAMPLES";
	case D3DMULTISAMPLE_9_SAMPLES:
		return "D3DMULTISAMPLE_9_SAMPLES";
	case D3DMULTISAMPLE_10_SAMPLES:
		return "D3DMULTISAMPLE_10_SAMPLES";
	case D3DMULTISAMPLE_11_SAMPLES:
		return "D3DMULTISAMPLE_11_SAMPLES";
	case D3DMULTISAMPLE_12_SAMPLES:
		return "D3DMULTISAMPLE_12_SAMPLES";
	case D3DMULTISAMPLE_13_SAMPLES:
		return "D3DMULTISAMPLE_13_SAMPLES";
	case D3DMULTISAMPLE_14_SAMPLES:
		return "D3DMULTISAMPLE_14_SAMPLES";
	case D3DMULTISAMPLE_15_SAMPLES:
		return "D3DMULTISAMPLE_15_SAMPLES";
	case D3DMULTISAMPLE_16_SAMPLES:
		return "D3DMULTISAMPLE_16_SAMPLES";
	default:
		return "Unknown Multisample Type";
	}
}

const char * DxutApp::DXUTVertexProcessingTypeToString(DWORD vpt)
{
	switch( vpt )
	{
	case D3DCREATE_SOFTWARE_VERTEXPROCESSING:
		return "Software vertex processing";
	case D3DCREATE_MIXED_VERTEXPROCESSING:
		return "Mixed vertex processing";
	case D3DCREATE_HARDWARE_VERTEXPROCESSING:
		return "Hardware vertex processing";
	case D3DCREATE_PUREDEVICE:
		return "Pure hardware vertex processing";
	default:
		return "Unknown vertex processing type";
	}
}

DXUTD3D9DeviceSettings DxutApp::FindValidDeviceSettings(const DXUTD3D9DeviceSettings & deviceSettings, const DXUTMatchOptions & matchOptions)
{
	DXUTD3D9DeviceSettings validDeviceSettings;
	CopyMemory( &validDeviceSettings, &deviceSettings, sizeof( validDeviceSettings ) );
	DXUTD3D9DeviceSettings optimalDeviceSettings;
	DXUTBuildOptimalD3D9DeviceSettings( &optimalDeviceSettings, const_cast<DXUTD3D9DeviceSettings *>(&deviceSettings), const_cast<DXUTMatchOptions *>(&matchOptions) );
    hr = DXUTFindValidD3D9DeviceSettings( &validDeviceSettings, const_cast<DXUTD3D9DeviceSettings *>(&deviceSettings),
                                          const_cast<DXUTMatchOptions *>(&matchOptions), &optimalDeviceSettings );
    if( FAILED( hr ) )
	{
		THROW_CUSEXCEPTION("no valid devices were found");
	}
	return validDeviceSettings;
}

void DxutApp::CreateDevice(bool bWindowed, int nSuggestedWidth, int nSuggestedHeight, UINT RefreshRate, UINT nPresentInterval)
{
	DXUTMatchOptions matchOptions;
	matchOptions.eAdapterOrdinal = DXUTMT_IGNORE_INPUT;
	matchOptions.eDeviceType = DXUTMT_IGNORE_INPUT;
	matchOptions.eWindowed = DXUTMT_PRESERVE_INPUT;
	matchOptions.eAdapterFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eVertexProcessing = DXUTMT_IGNORE_INPUT;
	if (bWindowed || (nSuggestedWidth != 0 && nSuggestedHeight != 0))
		matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
	else
		matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
	matchOptions.eBackBufferFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eBackBufferCount = DXUTMT_IGNORE_INPUT;
	matchOptions.eMultiSample = DXUTMT_IGNORE_INPUT;
	matchOptions.eSwapEffect = DXUTMT_IGNORE_INPUT;
	matchOptions.eDepthFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eStencilFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.ePresentFlags = DXUTMT_IGNORE_INPUT;
	if (RefreshRate != 0)
		matchOptions.eRefreshRate = DXUTMT_CLOSEST_TO_INPUT;
	else
		matchOptions.eRefreshRate = DXUTMT_IGNORE_INPUT;
	matchOptions.ePresentInterval = DXUTMT_PRESERVE_INPUT;

	DXUTD3D9DeviceSettings deviceSettings;
	ZeroMemory(&deviceSettings, sizeof(deviceSettings));
	deviceSettings.pp.Windowed = bWindowed;
	deviceSettings.pp.BackBufferWidth = nSuggestedWidth;
	deviceSettings.pp.BackBufferHeight = nSuggestedHeight;
	deviceSettings.pp.FullScreen_RefreshRateInHz = RefreshRate;
	deviceSettings.pp.PresentationInterval = nPresentInterval;

	ChangeDevice(FindValidDeviceSettings(deviceSettings, matchOptions));
}

void DxutApp::CheckForWindowSizeChange(void)
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

void DxutApp::ToggleFullScreen(void)
{
	DXUTMatchOptions matchOptions;
	matchOptions.eAdapterOrdinal = DXUTMT_PRESERVE_INPUT;
	matchOptions.eDeviceType = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eWindowed = DXUTMT_PRESERVE_INPUT;
	matchOptions.eAdapterFormat = DXUTMT_IGNORE_INPUT;
	matchOptions.eVertexProcessing = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
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

	ChangeDevice(FindValidDeviceSettings(deviceSettings, matchOptions));
}

void DxutApp::ToggleREF(void)
{
	DXUTMatchOptions matchOptions;
	matchOptions.eAdapterOrdinal = DXUTMT_PRESERVE_INPUT;
	matchOptions.eDeviceType = DXUTMT_PRESERVE_INPUT;
	matchOptions.eWindowed = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eAdapterFormat = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eVertexProcessing = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eBackBufferFormat = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eBackBufferCount = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eMultiSample = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eSwapEffect = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eDepthFormat = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eStencilFormat = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.ePresentFlags = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.eRefreshRate = DXUTMT_CLOSEST_TO_INPUT;
	matchOptions.ePresentInterval = DXUTMT_CLOSEST_TO_INPUT;

	DXUTD3D9DeviceSettings deviceSettings = m_DeviceSettings;
	if(deviceSettings.DeviceType == D3DDEVTYPE_HAL)
	{
		deviceSettings.DeviceType = D3DDEVTYPE_REF;
	}
	else if(deviceSettings.DeviceType == D3DDEVTYPE_REF)
	{
		deviceSettings.DeviceType = D3DDEVTYPE_HAL;
	}

	ChangeDevice(FindValidDeviceSettings(deviceSettings, matchOptions));
}

bool DxutApp::CanDeviceBeReset(const DXUTD3D9DeviceSettings & oldDeviceSettings, const DXUTD3D9DeviceSettings & newDeviceSettings)
{
	if(oldDeviceSettings.AdapterOrdinal == newDeviceSettings.AdapterOrdinal
		&& oldDeviceSettings.DeviceType == newDeviceSettings.DeviceType
		&& oldDeviceSettings.BehaviorFlags == newDeviceSettings.BehaviorFlags)
	{
		return true;
	}
	return false;
}

void DxutApp::ChangeDevice(DXUTD3D9DeviceSettings & deviceSettings)
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
		}

		HWND hWndInsertAfter = ((m_wnd->GetWindowLong(GWL_STYLE) & WS_EX_TOPMOST) != 0) ? HWND_TOPMOST : HWND_NOTOPMOST;
		CRect clientRect(0, 0, deviceSettings.pp.BackBufferWidth, deviceSettings.pp.BackBufferHeight);
		AdjustWindowRect(&clientRect, m_wnd->GetWindowLong(GWL_STYLE), NULL);
		m_wnd->SetWindowPos(hWndInsertAfter, 0, 0, clientRect.Width(), clientRect.Height(), SWP_NOMOVE | SWP_NOREDRAW);
	}
	else
	{
		if(!m_d3dDevice || m_DeviceSettings.pp.Windowed)
		{
			m_WindowBackBufferWidthAtModeChange = m_BackBufferSurfaceDesc.Width;
			m_WindowBackBufferHeightAtModeChange = m_BackBufferSurfaceDesc.Height;
			ZeroMemory(&m_WindowedPlacement, sizeof(m_WindowedPlacement));
			m_WindowedPlacement.length = sizeof(m_WindowedPlacement);
			m_wnd->GetWindowPlacement(&m_WindowedPlacement);
			m_WindowedStyleAtModeChange = m_wnd->GetWindowLong(GWL_STYLE);
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
			if(D3DERR_DEVICELOST == hr)
			{
				m_DeviceLost = true;
			}
			else
			{
				THROW_D3DEXCEPTION(hr);
			}
		}
	}
	else
	{
		if(FAILED(hr = Create3DEnvironment(deviceSettings)))
		{
			m_wnd->DestroyWindow();
			::MessageBox(NULL, ms2ts(my::D3DException::Translate(hr)).c_str(), NULL, MB_OK);
		}
	}

	if (!m_wnd->IsWindowVisible())
	{
		m_wnd->ShowWindow(SW_SHOW);
	}

	m_DeviceSettings = deviceSettings;

	m_IgnoreSizeChange = false;
}

HRESULT DxutApp::Create3DEnvironment(DXUTD3D9DeviceSettings & deviceSettings)
{
	if(m_DeviceObjectsCreated)
	{
		Cleanup3DEnvironment();
	}

	if(FAILED(hr = m_d3d9->CreateDevice(
		deviceSettings.AdapterOrdinal,
		deviceSettings.DeviceType,
		m_wnd->m_hWnd,
		deviceSettings.BehaviorFlags,
		&deviceSettings.pp,
		&m_d3dDevice)))
	{
		return hr;
	}

	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BackBuffer));
	V(m_d3dDevice->GetDepthStencilSurface(&m_DepthStencil));
	V(m_BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if(FAILED(hr = OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		return hr;
	}

	return S_OK;
}

HRESULT DxutApp::Reset3DEnvironment(DXUTD3D9DeviceSettings & deviceSettings)
{
	if(m_DeviceObjectsReset)
	{
		OnLostDevice();
	}

	if(FAILED(hr = m_d3dDevice->Reset(&deviceSettings.pp)))
	{
		return hr;
	}

	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_BackBuffer));
	V(m_d3dDevice->GetDepthStencilSurface(&m_DepthStencil));
	V(m_BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if(FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		OnLostDevice();
		return hr;
	}

	return S_OK;
}

void DxutApp::Render3DEnvironment(void)
{
	if (m_DeviceLost)
	{
		if (FAILED(hr = m_d3dDevice->TestCooperativeLevel()))
		{
			if (D3DERR_DEVICELOST == hr)
			{
				WaitMessage();
				return;
			}

			if (FAILED(hr = Reset3DEnvironment(m_DeviceSettings)))
			{
				if (D3DERR_DEVICELOST == hr)
				{
					WaitMessage();
					return;
				}

				THROW_D3DEXCEPTION(hr);
			}
		}

		m_DeviceLost = false;
	}

	UpdateClock();

	m_dwFrames++;

	if (m_fAbsoluteTime - m_fLastTime > 1.0f)
	{
		m_fFps = (float)(m_dwFrames / (m_fAbsoluteTime - m_fLastTime));
		m_fLastTime = m_fAbsoluteTime;
		m_dwFrames = 0;
	}

	OnFrameTick(m_fAbsoluteTime, m_fElapsedTime);
}

void DxutApp::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	if(FAILED(hr = m_d3dDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion)))
	{
		if(D3DERR_DEVICELOST == hr || D3DERR_DRIVERINTERNALERROR == hr)
		{
			m_DeviceLost = true;
		}
	}
}

void DxutApp::Cleanup3DEnvironment(void)
{
	if(m_DeviceObjectsReset)
	{
		OnLostDevice();
	}

	if(m_DeviceObjectsCreated)
	{
		OnDestroyDevice();
		UINT references = m_d3dDevice.Detach()->Release();
#ifdef _DEBUG
		if(references > 0)
		{
			TCHAR msg[256];
			_stprintf_s(msg, _countof(msg), _T("no zero reference count: %u"), references);
			::MessageBox(NULL, msg, NULL, MB_OK);
		}
#endif
	}
}

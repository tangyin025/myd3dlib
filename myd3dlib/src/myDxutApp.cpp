#include "stdafx.h"
#include "myDxutApp.h"
#include "myResource.h"

using namespace my;

BOOL DxutWindow::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
{
	switch(dwMsgMapID)
	{
	case 0:
		switch(uMsg)
		{
		case WM_CREATE:
			{
				ATLASSERT(m_hWnd);
				lResult = 0;
				return TRUE;
			}

		case WM_DESTROY:
			{
				lResult = 0;
				return TRUE;
			}

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
{
	FT_Error err = FT_Init_FreeType(&m_Library);
	if(err)
	{
		THROW_CUSEXCEPTION("FT_Init_FreeType failed");
	}
}

DxutApplication::~DxutApplication(void)
{
	FT_Error err = FT_Done_FreeType(m_Library);

	_ASSERT(m_deviceRelatedObjs.empty());
}

int DxutApplication::Run(void)
{
	LPDIRECT3D9 pd3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(NULL == pd3d9)
		return 0;
	m_d3d9.Attach(pd3d9);

	m_wnd = DxutWindowPtr(new DxutWindow());
	m_wnd->Create(NULL, Window::rcDefault, GetModuleFileName().c_str());

	CRect clientRect;
	m_wnd->GetClientRect(&clientRect);

    DXUTMatchOptions matchOptions;
    matchOptions.eAPIVersion = DXUTMT_IGNORE_INPUT;
    matchOptions.eAdapterOrdinal = DXUTMT_IGNORE_INPUT;
    matchOptions.eDeviceType = DXUTMT_IGNORE_INPUT;
    matchOptions.eOutput = DXUTMT_IGNORE_INPUT;
    matchOptions.eWindowed = DXUTMT_PRESERVE_INPUT;
    matchOptions.eAdapterFormat = DXUTMT_IGNORE_INPUT;
    matchOptions.eVertexProcessing = DXUTMT_IGNORE_INPUT;
    //if( bWindowed || ( nSuggestedWidth != 0 && nSuggestedHeight != 0 ) )
        matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
    //else
    //    matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
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
	ZeroMemory( &deviceSettings, sizeof( deviceSettings ) );
    deviceSettings.pp.Windowed = true;
    deviceSettings.pp.BackBufferWidth = clientRect.Width();
    deviceSettings.pp.BackBufferHeight = clientRect.Height();
	deviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    hr = DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( FAILED( hr ) ) // the call will fail if no valid devices were found
    {
        //DXUTDisplayErrorMessage( hr );
        //return DXUT_ERR( L"DXUTFindValidDeviceSettings", hr );
		return 0;
    }

	if(FAILED(m_d3d9->CreateDevice(
		deviceSettings.AdapterOrdinal,
		deviceSettings.DeviceType,
		GetHWND(),
		deviceSettings.BehaviorFlags | D3DCREATE_FPU_PRESERVE,
		&deviceSettings.pp,
		&m_d3dDevice)))
	{
		return 0;
	}

	IDirect3DSurface9 * pBackBuffer;
	if(FAILED(hr = m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		return 0;
	}

	my::SurfacePtr surface(new my::Surface());
	surface->Create(pBackBuffer);
	m_BackBufferSurfaceDesc = surface->GetDesc();
	if(FAILED(OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		return 0;
	}

	if(FAILED(hr = m_d3dDevice->CreateStateBlock(D3DSBT_ALL, &m_StateBlock)))
	{
		return 0;
	}

	DeviceRelatedObjectBasePtrSet::iterator obj_iter = m_deviceRelatedObjs.begin();
	for(; obj_iter != m_deviceRelatedObjs.end(); obj_iter++)
	{
		(*obj_iter)->OnResetDevice();
	}

	if(FAILED(OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		return 0;
	}

	m_wnd->ShowWindow(SW_SHOW);
	m_wnd->UpdateWindow();

	MSG msg;
	msg.message = WM_NULL;

	double time = 0;
	while(WM_QUIT != msg.message)
	{
		if(::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else
		{
			float fElapsedTime = 1/30.0f;
			OnFrameMove(time, fElapsedTime);

			OnFrameRender(m_d3dDevice, time, fElapsedTime);

			if(FAILED(hr = m_d3dDevice->Present(NULL, NULL, NULL, NULL)))
			{
			}

			time += 0.01f;
		}
	}

	return (int)msg.wParam;
}

bool DxutApplication::IsDeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	return true;
}

bool DxutApplication::ModifyDeviceSettings(
	DXUTD3D9DeviceSettings * pDeviceSettings)
{
	return true;
}

HRESULT DxutApplication::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

HRESULT DxutApplication::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void DxutApplication::OnLostDevice(void)
{
}

void DxutApplication::OnDestroyDevice(void)
{
}

void DxutApplication::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
}

void DxutApplication::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
}

LRESULT DxutApplication::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

void DxutApplication::OnKeyboard(
	UINT nChar,
	bool bKeyDown,
	bool bAltDown)
{
}

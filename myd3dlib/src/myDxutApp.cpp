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

	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency(&qwTicksPerSec);
	m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;

	m_llLastElapsedTime = 0;
}

DxutApplication::~DxutApplication(void)
{
	FT_Error err = FT_Done_FreeType(m_Library);

	_ASSERT(m_deviceRelatedObjs.empty());
}

int DxutApplication::Run(void)
{
	m_wnd = DxutWindowPtr(new DxutWindow());
	m_wnd->Create(NULL, Window::rcDefault, GetModuleFileName().c_str());

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

		IDirect3DSurface9 * pBackBuffer;
		if(FAILED(hr = m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
		{
			THROW_D3DEXCEPTION(hr);
		}

		my::SurfacePtr surface(new my::Surface());
		surface->Create(pBackBuffer);
		m_BackBufferSurfaceDesc = surface->GetDesc();
		if(FAILED(hr = OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
		{
			THROW_D3DEXCEPTION(hr);
		}

		if(FAILED(hr = m_d3dDevice->CreateStateBlock(D3DSBT_ALL, &m_StateBlock)))
		{
			THROW_D3DEXCEPTION(hr);
		}

		DeviceRelatedObjectBasePtrSet::iterator obj_iter = m_deviceRelatedObjs.begin();
		for(; obj_iter != m_deviceRelatedObjs.end(); obj_iter++)
		{
			(*obj_iter)->OnResetDevice();
		}

		if(FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
		{
			THROW_D3DEXCEPTION(hr);
		}

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
	ZeroMemory( &deviceSettings, sizeof( deviceSettings ) );
	deviceSettings.pp.Windowed = true;
	deviceSettings.pp.BackBufferWidth = nSuggestedWidth;
	deviceSettings.pp.BackBufferHeight = nSuggestedHeight;
	deviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	hr = DXUTFindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
	if( FAILED( hr ) ) // the call will fail if no valid devices were found
	{
		THROW_CUSEXCEPTION("no valid devices were found");
	}

	Create3DEnvironment(deviceSettings);
}

void DxutApplication::Create3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings)
{
	if(FAILED(hr = m_d3d9->CreateDevice(
		deviceSettings.AdapterOrdinal,
		deviceSettings.DeviceType,
		GetHWND(),
		deviceSettings.BehaviorFlags | D3DCREATE_FPU_PRESERVE,
		const_cast<D3DPRESENT_PARAMETERS *>(&deviceSettings.pp),
		&m_d3dDevice)))
	{
		THROW_D3DEXCEPTION(hr);
	}
}

void DxutApplication::Render3DEnvironment(void)
{
	LARGE_INTEGER qwTime;
	QueryPerformanceCounter(&qwTime);
	double fTime = qwTime.QuadPart / (double)m_llQPFTicksPerSec;

	float fElapsedTime = (float)((qwTime.QuadPart - m_llLastElapsedTime) / (double)m_llQPFTicksPerSec);

	m_llLastElapsedTime = qwTime.QuadPart;

	m_fAbsoluteTime = fTime;

	OnFrameMove(fTime, fElapsedTime);

	OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

	if(FAILED(hr = m_d3dDevice->Present(NULL, NULL, NULL, NULL)))
	{
	}
}

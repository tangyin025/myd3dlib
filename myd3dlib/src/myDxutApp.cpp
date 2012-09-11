#include "stdafx.h"
#include "myDxutApp.h"
#include <SDKmisc.h>
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

	D3DPRESENT_PARAMETERS d3dpp = {0};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;

	if(FAILED(m_d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		GetHWND(),
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
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
	if(FAILED(OnD3D9CreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
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

	if(FAILED(OnD3D9ResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
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

			OnD3D9FrameRender(m_d3dDevice, time, fElapsedTime);

			if(FAILED(hr = m_d3dDevice->Present(NULL, NULL, NULL, NULL)))
			{
			}

			time += 0.01f;
		}
	}

	return (int)msg.wParam;
}

bool DxutApplication::IsD3D9DeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	return true;
}

bool DxutApplication::ModifyDeviceSettings(
	DXUTDeviceSettings * pDeviceSettings)
{
	return true;
}

HRESULT DxutApplication::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

HRESULT DxutApplication::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void DxutApplication::OnD3D9LostDevice(void)
{
}

void DxutApplication::OnD3D9DestroyDevice(void)
{
}

void DxutApplication::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
}

void DxutApplication::OnD3D9FrameRender(
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

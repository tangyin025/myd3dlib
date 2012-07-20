#include "stdafx.h"
#include "Game.h"
#include "LuaExtension.h"
#include <luabind/luabind.hpp>
#include <SDKmisc.h>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

Game::Game(void)
{
	m_settingsDlg.Init(&m_dlgResourceMgr);

	m_lua = my::LuaContextPtr(new my::LuaContext());

	Export2Lua(m_lua->_state);
}

Game::~Game(void)
{
	my::ImeEditBox::Uninitialize();
}

bool Game::IsD3D9DeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}

bool Game::ModifyDeviceSettings(
	DXUTDeviceSettings * pDeviceSettings)
{
	assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

	HRESULT hr;
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	D3DCAPS9 caps;

	V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps ) );

	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if( caps.MaxVertexBlendMatrices < 2 )
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// ! 修正 lua print(0xffffffff)结果不正确的问题, 参见：http://www.lua.org/bugs.html
	pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_FPU_PRESERVE;

	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}

HRESULT Game::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9CreateDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	my::ImeEditBox::Initialize(DxutApp::getSingleton().GetHWND());

	my::ImeEditBox::EnableImeSystem(false);

	V(m_dlgResourceMgr.OnD3D9CreateDevice(pd3dDevice));

	V(m_settingsDlg.OnD3D9CreateDevice(pd3dDevice));

	// 必须保证这个脚本没有错误，否则将看不到控制台
	ExecuteCode("dofile(\"demo2_3.lua\")");

	if(!m_input)
	{
		m_input = my::Input::CreateInput(GetModuleHandle(NULL));
		m_keyboard = my::Keyboard::CreateKeyboard(m_input->m_ptr);
		m_keyboard->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		m_mouse = my::Mouse::CreateMouse(m_input->m_ptr);
		m_mouse->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}

	if(!m_sound)
	{
		m_sound = my::Sound::CreateSound();
		m_sound->SetCooperativeLevel(GetHWND(), DSSCL_PRIORITY);
	}

	//THROW_CUSEXCEPTION("aaa");

	return S_OK;
}

HRESULT Game::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	if(m_panel)
		m_panel->AddLine(L"Game::OnD3D9ResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	V(m_dlgResourceMgr.OnD3D9ResetDevice());

	V(m_settingsDlg.OnD3D9ResetDevice());

	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		UpdateDlgViewProj(*dlg_iter);
	}

	return S_OK;
}

void Game::OnD3D9LostDevice(void)
{
	if(m_panel)
		m_panel->AddLine(L"Game::OnD3D9LostDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_dlgResourceMgr.OnD3D9LostDevice();

	m_settingsDlg.OnD3D9LostDevice();

	DxutApp::OnD3D9LostDevice();
}

void Game::OnD3D9DestroyDevice(void)
{
	m_dlgResourceMgr.OnD3D9DestroyDevice();

	m_settingsDlg.OnD3D9DestroyDevice();

	m_dlgSet.clear();

	my::ImeEditBox::Uninitialize();

	DxutApp::OnD3D9DestroyDevice();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	DxutApp::OnFrameMove(fTime, fElapsedTime);

	m_keyboard->Capture();

	m_mouse->Capture();

	BaseScenePtrSet::iterator scene_iter = m_sceneSet.begin();
	for(; scene_iter != m_sceneSet.end(); scene_iter++)
	{
		(*scene_iter)->OnFrameMove(fTime, fElapsedTime);
	}
}

void Game::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	if(m_settingsDlg.IsActive())
	{
		m_settingsDlg.OnRender(fElapsedTime);
		return;
	}

	//V(pd3dDevice->Clear(
	//	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00484848, 1, 0));

	BaseScenePtrSet::iterator scene_iter = m_sceneSet.begin();
	for(; scene_iter != m_sceneSet.end(); scene_iter++)
	{
		(*scene_iter)->OnRender(pd3dDevice, fTime, fElapsedTime);
	}

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		my::UIRender::Begin(pd3dDevice);

		DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
		for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
		{
			(*dlg_iter)->Draw(pd3dDevice, fElapsedTime);
		}

		if(m_font)
		{
			my::Matrix4 View, Proj;
			D3DVIEWPORT9 vp;
			pd3dDevice->GetViewport(&vp);
			my::UIRender::BuildPerspectiveMatrices(
				D3DXToRadian(75.0f), (float)vp.Width, (float)vp.Height, View, Proj);
			V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&my::Matrix4::identity));
			V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&View));
			V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Proj));
			m_font->DrawString(DXUTGetFrameStats(DXUTIsVsyncEnabled()),
				my::Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0), my::Font::AlignLeftTop);
			m_font->DrawString(DXUTGetDeviceStats(),
				my::Rectangle::LeftTop(5,5 + (float)m_font->m_LineHeight,500,10), D3DCOLOR_ARGB(255,255,255,0), my::Font::AlignLeftTop);
		}

		my::UIRender::End(pd3dDevice);

		V(pd3dDevice->EndScene());
	}
}

LRESULT Game::MsgProc(
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

	if((*pbNoFurtherProcessing = m_dlgResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if(m_settingsDlg.IsActive())
	{
		m_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	if(EventToggleConsole && uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		EventToggleConsole(my::EventArgsPtr(new my::EventArgs()));
		*pbNoFurtherProcessing = true;
		return 0;
	}

	DialogPtrSet::reverse_iterator dlg_iter = m_dlgSet.rbegin();
	for(; dlg_iter != m_dlgSet.rend(); dlg_iter++)
	{
		if((*pbNoFurtherProcessing = (*dlg_iter)->MsgProc(hWnd, uMsg, wParam, lParam)))
			return 0;
	}

	return 0;
}

void Game::ToggleFullScreen(void)
{
	DXUTToggleFullScreen();
}

void Game::ToggleRef(void)
{
	DXUTToggleREF();
}

void Game::ChangeDevice(void)
{
	m_settingsDlg.SetActive(!m_settingsDlg.IsActive());
}

void Game::ExecuteCode(const char * code)
{
	try
	{
		m_lua->executeCode(code);
	}
	catch(const std::runtime_error & e)
	{
		if(!m_panel)
			THROW_CUSEXCEPTION(e.what());

		m_panel->AddLine(ms2ws(e.what()));
	}
}

void Game::UpdateDlgViewProj(my::DialogPtr dlg)
{
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();

	float aspect = pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;

	float height = (float)pBackBufferSurfaceDesc->Height;

	my::Vector2 vp(height * aspect, height);

	my::UIRender::BuildPerspectiveMatrices(D3DXToRadian(75.0f), vp.x, vp.y, dlg->m_View, dlg->m_Proj);

	if(dlg->EventAlign)
		dlg->EventAlign(my::EventArgsPtr(new AlignEventArgs(vp)));
}

void Game::MustThrowException(void)
{
	THROW_CUSEXCEPTION("Game::MustThrowException");
}

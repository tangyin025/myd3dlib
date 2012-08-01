#include "stdafx.h"
#include "Game.h"
#include "GameState.h"
#include "LuaExtension.h"
#include <luabind/luabind.hpp>
#include <SDKmisc.h>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

Game::Game(void)
{
	m_settingsDlg.Init(&m_dlgResourceMgr);

	m_lua.reset(new LuaContext());

	Export2Lua(m_lua->_state);
}

Game::~Game(void)
{
	ImeEditBox::Uninitialize();
}

TexturePtr Game::LoadTexture(const char * path)
{
	LPDIRECT3DDEVICE9 pDevice = Game::getSingleton().GetD3D9Device();
	std::string full_path = ResourceMgr::getSingleton().GetFullPath(path);
	if(!full_path.empty())
		return Texture::CreateTextureFromFile(pDevice, full_path.c_str());

	CachePtr cache = ResourceMgr::getSingleton().OpenArchiveStream(path)->GetWholeCache();
	return Texture::CreateTextureFromFileInMemory(pDevice, &(*cache)[0], cache->size());
}

FontPtr Game::LoadFont(const char * path, int height)
{
	LPDIRECT3DDEVICE9 pDevice = Game::getSingleton().GetD3D9Device();
	std::string full_path = ResourceMgr::getSingleton().GetFullPath(path);
	if(!full_path.empty())
		return Font::CreateFontFromFile(pDevice, full_path.c_str(), height, 1);

	CachePtr cache = ResourceMgr::getSingleton().OpenArchiveStream(path)->GetWholeCache();
	return Font::CreateFontFromFileInCache(pDevice, cache, height, 1);
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

	// ! Fix lua print(0xffffffff) issue, ref: http://www.lua.org/bugs.html#5.1-3
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

	ImeEditBox::Initialize(DxutApp::getSingleton().GetHWND());

	ImeEditBox::EnableImeSystem(false);

	V(m_dlgResourceMgr.OnD3D9CreateDevice(pd3dDevice));

	V(m_settingsDlg.OnD3D9CreateDevice(pd3dDevice));

	ExecuteCode("dofile \"Font.lua\"");

	ExecuteCode("dofile \"Console.lua\"");

	if(!m_font || !m_console || !m_panel)
	{
		THROW_CUSEXCEPTION("m_font, m_console, m_panel must be created");
	}

	UpdateDlgViewProj(m_console);

	AddLine(L"Game::OnD3D9CreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(!m_input)
	{
		m_input = Input::CreateInput(GetModuleHandle(NULL));
		m_keyboard = Keyboard::CreateKeyboard(m_input->m_ptr);
		m_keyboard->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		m_mouse = Mouse::CreateMouse(m_input->m_ptr);
		m_mouse->SetCooperativeLevel(GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}

	if(!m_sound)
	{
		m_sound = Sound::CreateSound();
		m_sound->SetCooperativeLevel(GetHWND(), DSSCL_PRIORITY);
	}

	initiate();

	//THROW_CUSEXCEPTION("aaa");

	return S_OK;
}

HRESULT Game::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnD3D9ResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	HRESULT hres;
	if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	V(m_dlgResourceMgr.OnD3D9ResetDevice());

	V(m_settingsDlg.OnD3D9ResetDevice());

	UpdateDlgViewProj(m_console);

	DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
	for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
	{
		UpdateDlgViewProj(*dlg_iter);
	}

	return S_OK;
}

void Game::OnD3D9LostDevice(void)
{
	AddLine(L"Game::OnD3D9LostDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_dlgResourceMgr.OnD3D9LostDevice();

	m_settingsDlg.OnD3D9LostDevice();

	DxutApp::OnD3D9LostDevice();
}

void Game::OnD3D9DestroyDevice(void)
{
	AddLine(L"Game::OnD3D9DestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	terminate();

	m_dlgResourceMgr.OnD3D9DestroyDevice();

	m_settingsDlg.OnD3D9DestroyDevice();

	m_console.reset();

	m_dlgSet.clear();

	ImeEditBox::Uninitialize();

	DxutApp::OnD3D9DestroyDevice();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	DxutApp::OnFrameMove(fTime, fElapsedTime);

	m_keyboard->Capture();

	m_mouse->Capture();

	if(cs = CurrentState())
		cs->OnFrameMove(fTime, fElapsedTime);
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
	//	0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 161, 161, 161), 1, 0));

	// 当状态切换时发生异常会导致新状态没有被创建，所以有必要判断之
	if(cs = CurrentState())
		cs->OnD3D9FrameRender(pd3dDevice, fTime, fElapsedTime);

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		UIRender::Begin(pd3dDevice);

		DialogPtrSet::iterator dlg_iter = m_dlgSet.begin();
		for(; dlg_iter != m_dlgSet.end(); dlg_iter++)
		{
			(*dlg_iter)->Draw(pd3dDevice, fElapsedTime);
		}

		m_console->Draw(pd3dDevice, fElapsedTime);

		_ASSERT(m_font);

		Matrix4 View, Proj;
		D3DVIEWPORT9 vp;
		pd3dDevice->GetViewport(&vp);
		UIRender::BuildPerspectiveMatrices(
			D3DXToRadian(75.0f), (float)vp.Width, (float)vp.Height, View, Proj);
		V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity));
		V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&View));
		V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Proj));
		m_font->DrawString(DXUTGetFrameStats(DXUTIsVsyncEnabled()),
			Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0), Font::AlignLeftTop);
		m_font->DrawString(DXUTGetDeviceStats(),
			Rectangle::LeftTop(5,5 + (float)m_font->m_LineHeight,500,10), D3DCOLOR_ARGB(255,255,255,0), Font::AlignLeftTop);

		UIRender::End(pd3dDevice);

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

	if(m_console && uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_console->SetVisible(!m_console->GetVisible());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if(m_console && (*pbNoFurtherProcessing = m_console->MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	DialogPtrSet::reverse_iterator dlg_iter = m_dlgSet.rbegin();
	for(; dlg_iter != m_dlgSet.rend(); dlg_iter++)
	{
		if((*pbNoFurtherProcessing = (*dlg_iter)->MsgProc(hWnd, uMsg, wParam, lParam)))
			return 0;
	}

	if((cs = CurrentState()) &&
		(FAILED(hres = cs->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing))
	{
		return hres;
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

		AddLine(ms2ws(e.what()));
	}
}

void Game::UpdateDlgViewProj(DialogPtr dlg)
{
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc = DXUTGetD3D9BackBufferSurfaceDesc();

	float aspect = pBackBufferSurfaceDesc->Width / (float)pBackBufferSurfaceDesc->Height;

	float height = (float)pBackBufferSurfaceDesc->Height;

	Vector2 vp(height * aspect, height);

	UIRender::BuildPerspectiveMatrices(D3DXToRadian(75.0f), vp.x, vp.y, dlg->m_View, dlg->m_Proj);

	if(dlg->EventAlign)
		dlg->EventAlign(EventArgsPtr(new AlignEventArgs(vp)));
}

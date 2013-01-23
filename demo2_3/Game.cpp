#include "stdafx.h"
#include "Game.h"
#include "GameState.h"
#include "LuaExtension.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

void EffectUIRender::Begin(void)
{
	const D3DSURFACE_DESC & desc = my::DxutApp::getSingleton().GetD3D9BackBufferSurfaceDesc();
	if(m_UIEffect->m_ptr)
		m_UIEffect->SetVector("g_ScreenDim", Vector4((float)desc.Width, (float)desc.Height, 0, 0));

	if(m_UIEffect->m_ptr)
		m_Passes = m_UIEffect->Begin();
}

void EffectUIRender::End(void)
{
	if(m_UIEffect->m_ptr)
		m_UIEffect->End();
}

void EffectUIRender::SetTexture(IDirect3DBaseTexture9 * pTexture)
{
	_ASSERT(Game::getSingleton().m_WhiteTex);

	if(m_UIEffect->m_ptr)
		m_UIEffect->SetTexture("g_MeshTexture", pTexture ? pTexture : Game::getSingleton().m_WhiteTex->m_ptr);
}

void EffectUIRender::SetTransform(const Matrix4 & World, const Matrix4 & View, const Matrix4 & Proj)
{
	if(m_UIEffect->m_ptr)
		m_UIEffect->SetMatrix("g_mWorldViewProjection", World * View * Proj);
}

void EffectUIRender::DrawVertexList(void)
{
	if(m_UIEffect->m_ptr)
	{
		if(vertex_count > 0)
		{
			for(UINT p = 0; p < m_Passes; p++)
			{
				m_UIEffect->BeginPass(p);
				V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
				V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_count / 3, vertex_list, sizeof(vertex_list[0])));
				m_UIEffect->EndPass();
			}
		}
	}
}

Game::Game(void)
{
	m_lua.reset(new LuaContext());

	Export2Lua(m_lua->_state);

	m_CurrentStateIter = m_stateMap.end();

	RegisterFileDir("Media");
	RegisterZipArchive("Media.zip");
	RegisterFileDir("..\\demo2_3\\Media");
	RegisterZipArchive("..\\demo2_3\\Media.zip");
}

Game::~Game(void)
{
	RemoveAllDlg();

	RemoveAllTimer();

	ImeEditBox::Uninitialize();
}

bool Game::IsDeviceAcceptable(
	D3DCAPS9 * pCaps,
	D3DFORMAT AdapterFormat,
	D3DFORMAT BackBufferFormat,
	bool bWindowed)
{
	if( FAILED( m_d3d9->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}

bool Game::ModifyDeviceSettings(
	DXUTD3D9DeviceSettings * pDeviceSettings)
{
	D3DCAPS9 caps;
	V( m_d3d9->GetDeviceCaps( pDeviceSettings->AdapterOrdinal,
		pDeviceSettings->DeviceType,
		&caps ) );

	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if( caps.MaxVertexBlendMatrices < 2 )
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// ! Fix lua print(0xffffffff) issue, ref: http://www.lua.org/bugs.html#5.1-3
	pDeviceSettings->BehaviorFlags |= D3DCREATE_FPU_PRESERVE;

	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		//if( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
		//	DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}

HRESULT Game::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	if(FAILED(hr = D3DXCreateEffectPool(&m_EffectPool)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	ImeEditBox::Initialize(m_wnd->m_hWnd);

	ImeEditBox::EnableImeSystem(false);

	m_UIRender.reset(new EffectUIRender(pd3dDevice, LoadEffect("UIEffect.fx")));

	m_WhiteTex = LoadTexture("white.bmp");

	ExecuteCode("dofile \"Console.lua\"");

	ExecuteCode("dofile \"Hud.lua\"");

	if(!m_Font || !m_Console || !m_Panel)
	{
		THROW_CUSEXCEPTION("m_Font, m_Console, m_Panel must be created");
	}

	m_dlgSetMap[1].push_back(m_Console);

	if(m_Console->EventAlign)
		m_Console->EventAlign(EventArgsPtr(new EventArgs()));

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(!m_Input)
	{
		m_Input.reset(new Input());
		m_Input->CreateInput(GetModuleHandle(NULL));

		m_Keyboard.reset(new Keyboard());
		m_Keyboard->CreateKeyboard(m_Input->m_ptr);
		m_Keyboard->SetCooperativeLevel(m_wnd->m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

		m_Mouse.reset(new Mouse());
		m_Mouse->CreateMouse(m_Input->m_ptr);
		m_Mouse->SetCooperativeLevel(m_wnd->m_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}

	if(!m_Sound)
	{
		m_Sound.reset(new Sound());
		m_Sound->CreateSound();
		m_Sound->SetCooperativeLevel(m_wnd->m_hWnd, DSSCL_PRIORITY);
	}

	SetState("GameStateLoad", GameStateBasePtr(new GameStateLoad()));

	SetState("GameStateMain", GameStateBasePtr(new GameStateMain()));

	ChangeState("GameStateLoad");

	//THROW_CUSEXCEPTION("aaa");

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	HRESULT hres;
	if(FAILED(hres = LoaderMgr::OnResetDevice(
		pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hres;
	}

	Vector2 vp(600 * (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height, 600);

	DialogMgr::SetDlgViewport(vp);

	SafeResetState(GetCurrentState());

	return S_OK;
}

void Game::OnLostDevice(void)
{
	AddLine(L"Game::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	SafeLostState(GetCurrentState());

	LoaderMgr::OnLostDevice();
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	SafeDestroyState(GetCurrentState());

	RemoveAllState();

	m_EffectPool.Release();

	ExecuteCode("collectgarbage(\"collect\")");

	m_Console.reset();

	m_dlgSetMap[1].clear();

	RemoveAllDlg();

	m_UIRender.reset();

	ImeEditBox::Uninitialize();

	RemoveAllTimer();

	LoaderMgr::OnDestroyDevice();
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_Keyboard->Capture();

	m_Mouse->Capture();

	TimerMgr::OnFrameMove(fTime, fElapsedTime);

	if(m_stateMap.end() != m_CurrentStateIter)
		m_CurrentStateIter->second->OnFrameMove(fTime, fElapsedTime);
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	OnFrameMove(fTime, fElapsedTime);

	if(m_stateMap.end() != m_CurrentStateIter)
		m_CurrentStateIter->second->OnFrameRender(pd3dDevice, fTime, fElapsedTime);
	else
		V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		m_UIRender->Begin();

		DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime);

		_ASSERT(m_Font);

		m_UIRender->SetTransform(my::Matrix4::Identity(), DialogMgr::m_View, DialogMgr::m_Proj);

		m_Font->DrawString(m_UIRender.get(), m_strFPS, Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0));

		m_UIRender->End();

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
	if(m_Console && uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_Console->SetVisible(!m_Console->GetVisible());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if((*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if(m_stateMap.end() != m_CurrentStateIter)
	{
		LRESULT lr;
		if(lr = m_CurrentStateIter->second->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
			return lr;
	}

	return 0;
}

static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(Game::getSingleton().m_lua->_state, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}
//
//static void l_message (const char *pname, const char *msg) {
//  //if (pname) fprintf(stderr, "%s: ", pname);
//  //fprintf(stderr, "%s\n", msg);
//  //fflush(stderr);
//	Game::getSingleton().AddLine(L"");
//	Game::getSingleton().puts(ms2ws(msg).c_str());
//}
//
//static int report (lua_State *L, int status) {
//  if (status && !lua_isnil(L, -1)) {
//    const char *msg = lua_tostring(L, -1);
//    if (msg == NULL) msg = "(error object is not a string)";
//    l_message("aaa", msg);
//    lua_pop(L, 1);
//  }
//  return status;
//}

static int dostring (lua_State *L, const char *s, const char *name) {
  //int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  //return report(L, status);
  return luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
}

void Game::ExecuteCode(const char * code)
{
	if(dostring(m_lua->_state, code, "Game::ExecuteCode") && !lua_isnil(m_lua->_state, -1))
	{
		std::string msg = lua_tostring(m_lua->_state, -1);
		if(msg.empty())
			msg = "(error object is not a string)";
		lua_pop(m_lua->_state, 1);
		THROW_CUSEXCEPTION(msg);
	}
}

void Game::SetState(const std::string & key, GameStateBasePtr state)
{
	_ASSERT(!GetState(key));

	m_stateMap[key] = state;
}

GameStateBasePtr Game::GetState(const std::string & key) const
{
	GameStateBasePtrMap::const_iterator state_iter = m_stateMap.find(key);
	if(m_stateMap.end() != state_iter)
		return state_iter->second;

	return GameStateBasePtr();
}

GameStateBasePtr Game::GetCurrentState(void) const
{
	if(m_stateMap.end() != m_CurrentStateIter)
		return m_CurrentStateIter->second;

	return GameStateBasePtr();
}

std::string Game::GetCurrentStateKey(void) const
{
	if(m_stateMap.end() != m_CurrentStateIter)
		return m_CurrentStateIter->first;

	return "";
}

void Game::SafeCreateState(GameStateBasePtr state)
{
	if(state)
	{
		_ASSERT(!state->m_DeviceObjectsCreated);
		state->OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc);
		state->m_DeviceObjectsCreated = true;
	}
}

void Game::SafeResetState(GameStateBasePtr state)
{
	if(state && state->m_DeviceObjectsCreated && m_DeviceObjectsCreated)
	{
		_ASSERT(!state->m_DeviceObjectsReset);
		state->OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc);
		state->m_DeviceObjectsReset = true;
	}
}

void Game::SafeLostState(GameStateBasePtr state)
{
	if(state && state->m_DeviceObjectsReset)
	{
		state->OnLostDevice();
		state->m_DeviceObjectsReset = false;
	}
}

void Game::SafeDestroyState(GameStateBasePtr state)
{
	if(state && state->m_DeviceObjectsCreated)
	{
		_ASSERT(!state->m_DeviceObjectsReset);
		state->OnDestroyDevice();
		state->m_DeviceObjectsCreated = false;
	}
}

void Game::SafeChangeState(GameStateBasePtr old_state, GameStateBasePtrMap::const_iterator new_state_iter)
{
	SafeLostState(old_state);

	SafeDestroyState(old_state);

	m_CurrentStateIter = new_state_iter;

	if(m_stateMap.end() != new_state_iter)
	{
		try
		{
			SafeCreateState(new_state_iter->second);

			SafeResetState(new_state_iter->second);
		}
		catch(const my::Exception & e)
		{
			SafeLostState(new_state_iter->second);

			SafeDestroyState(new_state_iter->second);

			m_CurrentStateIter = m_stateMap.end();

			// ! 状态切换是可以容错的，只是当状态切换失败后，将没有CurrentState
			AddLine(ms2ws(e.GetDescription().c_str()));
		}
	}
}

void Game::ChangeState(const std::string & key)
{
	SafeChangeState(GetCurrentState(), m_stateMap.find(key));
}

void Game::RemoveAllState(void)
{
	m_stateMap.clear();

	m_CurrentStateIter = m_stateMap.end();
}

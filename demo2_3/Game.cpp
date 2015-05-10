#include "stdafx.h"
#include "Game.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fstream>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

extern void Export2Lua(lua_State * L);

my::ResourceMgr * FModContext::GetResourceMgr(void)
{
	return Game::getSingletonPtr();
}

void EffectUIRender::Begin(void)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->SetVector("g_ScreenDim", Vector4(
			(float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Width, (float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Height, 0, 0));
		m_Passes = m_UIEffect->Begin();
	}
}

void EffectUIRender::End(void)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->End();
		m_Passes = 0;
	}
}

void EffectUIRender::SetWorld(const Matrix4 & World)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->SetMatrix("g_World", World);
	}
}

void EffectUIRender::SetViewProj(const Matrix4 & ViewProj)
{
	if(m_UIEffect->m_ptr)
	{
		m_UIEffect->SetMatrix("g_ViewProj", ViewProj);
	}
}

void EffectUIRender::SetTexture(const BaseTexturePtr & Texture)
{
	if(m_UIEffect->m_ptr)
	{
		_ASSERT(Game::getSingleton().m_WhiteTex);
		m_UIEffect->SetTexture("g_MeshTexture", Texture ? Texture : Game::getSingleton().m_WhiteTex);
	}
}

void EffectUIRender::DrawVertexList(void)
{
	if(m_UIEffect->m_ptr && vertex_count > 0)
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

Game::Game(void)
	: SHADOW_MAP_SIZE(1024)
	, SHADOW_EPSILON(0.001f)
	, m_ShadowRT(new Texture2D())
	, m_ShadowDS(new Surface())
	, m_NormalRT(new Texture2D())
	, m_DiffuseRT(new Texture2D())
	, m_Camera(D3DXToRadian(75), 1.333333f, 0.1f, 3000.0f)
	, m_SkyLight(30,30,-100,100)
{
	Export2Lua(_state);
}

Game::~Game(void)
{
	//// ! Must manually call destructors at specific order
	//OnDestroyDevice();
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

	return true;
}

HRESULT Game::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	ImeEditBox::Initialize(m_wnd->m_hWnd);

	ImeEditBox::EnableImeSystem(false);

	InputMgr::Create(m_hinst, m_wnd->m_hWnd);

	ParallelTaskManager::StartParallelThread(3);

	if(FAILED(hr = ActorResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(!PhysXContext::OnInit())
	{
		THROW_CUSEXCEPTION("PhysXContext::OnInit failed");
	}

	if(!PhysXSceneContext::OnInit(m_sdk.get(), m_CpuDispatcher.get()))
	{
		THROW_CUSEXCEPTION("PhysXSceneContext::OnInit failed");
	}

	if (!FModContext::OnInit())
	{
		THROW_CUSEXCEPTION("FModContext::OnInit failed");
	}

	m_UIRender.reset(new EffectUIRender(pd3dDevice, LoadEffect("shader/UIEffect.fx", "")));

	if (!(m_SimpleSample = LoadEffect("shader/SimpleSample.fx", "")))
	{
		THROW_CUSEXCEPTION("create m_SimpleSample failed");
	}

	if (!(m_Font = LoadFont("font/wqy-microhei.ttc", 13)))
	{
		THROW_CUSEXCEPTION("create m_Font failed");
	}

	m_Console = ConsolePtr(new Console());

	m_Console->SetVisible(false);

	DialogMgr::InsertDlg(m_Console);

	if (!(m_WhiteTex = LoadTexture("texture/white.bmp")))
	{
		THROW_CUSEXCEPTION("create m_WhiteTex failed");
	}

	if (!(m_TexChecker = LoadTexture("texture/Checker.bmp")))
	{
		THROW_CUSEXCEPTION("create m_TexChecker failed");
	}

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(FAILED(hr = ActorResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_ShadowRT->CreateAdjustedTexture(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! ���е� render target����ʹ�þ�����ͬ multisample�� depth stencil
	m_ShadowDS->CreateDepthStencilSurface(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	m_NormalRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_DiffuseRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	Vector2 vp(600 * (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height, 600);

	DialogMgr::SetDlgViewport(vp, D3DXToRadian(75.0f));

	m_Font->SetScale(Vector2(pBackBufferSurfaceDesc->Width / vp.x, pBackBufferSurfaceDesc->Height / vp.y));

	if(m_Camera.EventAlign)
	{
		m_Camera.EventAlign(&EventArgs());
	}

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.begin();
	for (; shader_iter != m_ShaderCache.end(); shader_iter++)
	{
		if (shader_iter->second)
		{
			shader_iter->second->OnResetDevice();
		}
	}

	ActorPtrList::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->OnResetDevice();
	}

	return S_OK;
}

void Game::OnLostDevice(void)
{
	AddLine(L"Game::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	ActorResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();

	m_ShadowRT->OnDestroyDevice();

	m_ShadowDS->OnDestroyDevice();

	m_NormalRT->OnDestroyDevice();

	m_DiffuseRT->OnDestroyDevice();

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.begin();
	for (; shader_iter != m_ShaderCache.end(); shader_iter++)
	{
		if (shader_iter->second)
		{
			shader_iter->second->OnLostDevice();
		}
	}

	ActorPtrList::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->OnLostDevice();
	}
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	ParallelTaskManager::StopParallelThread();

	ActorPtrList::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->OnDestroyDevice();
	}

	ExecuteCode("collectgarbage(\"collect\")");

	m_Actors.clear();

	m_Console.reset();

	RemoveAllDlg();

	m_SimpleSample.reset();

	m_ShaderCache.clear();

	m_UIRender.reset();

	RemoveAllTimer();

	PhysXSceneContext::OnShutdown();

	PhysXContext::OnShutdown();

	FModContext::OnShutdown();

	RenderPipeline::OnDestroyDevice();

	ActorResourceMgr::OnDestroyDevice();

	InputMgr::Destroy();

	ImeEditBox::Uninitialize();
}

void Game::OnPxThreadSubstep(float dtime)
{
	ActorPtrList::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->OnPxThreadSubstep(dtime);
	}
}

void Game::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	InputMgr::Update(fTime, fElapsedTime);

	ActorResourceMgr::CheckRequests();

	TimerMgr::OnFrameMove(fTime, fElapsedTime);

	ActorPtrList::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->Update(fElapsedTime);
	}

	m_Camera.OnFrameMove(fTime, fElapsedTime);

	m_SkyLight.OnFrameMove(fTime, fElapsedTime);
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	// ! Ogre & Apexģ�Ͷ���˳ʱ�룬����ϵӦ������ʱ��
	V(m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
	m_SimpleSample->SetFloatArray("g_ScreenDim", (float *)&Vector2((float)m_BackBufferSurfaceDesc.Width, (float)m_BackBufferSurfaceDesc.Height), 2);

	CComPtr<IDirect3DSurface9> OldRT;
	V(pd3dDevice->GetRenderTarget(0, &OldRT));
	CComPtr<IDirect3DSurface9> OldDS = NULL;
	V(pd3dDevice->GetDepthStencilSurface(&OldDS));

	Frustum frustum = Frustum::ExtractMatrix(m_SkyLight.m_ViewProj);
	ActorPtrList::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->QueryComponent(frustum, this, Material::PassTypeToMask(Material::PassTypeShadow));
	}

	V(pd3dDevice->SetRenderTarget(0, m_ShadowRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowDS->m_ptr));
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	if(SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		m_SimpleSample->SetMatrix("g_View", m_SkyLight.m_View);
		m_SimpleSample->SetMatrix("g_ViewProj", m_SkyLight.m_ViewProj);
		RenderPipeline::RenderAllObjects(Material::PassTypeShadow, pd3dDevice, fTime, fElapsedTime);
		V(m_d3dDevice->EndScene());
	}

	frustum = Frustum::ExtractMatrix(m_Camera.m_ViewProj);
	actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*actor_iter)->QueryComponent(frustum, this,
			Material::PassTypeToMask(Material::PassTypeNormalDepth)
			| Material::PassTypeToMask(Material::PassTypeDiffuseSpec)
			| Material::PassTypeToMask(Material::PassTypeTextureColor)
			| Material::PassTypeToMask(Material::PassTypeTransparent));
	}

	V(pd3dDevice->SetRenderTarget(0, m_NormalRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(OldDS));
	V(m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));
	if(SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		m_SimpleSample->SetMatrix("g_View", m_Camera.m_View);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera.m_ViewProj);
		m_SimpleSample->SetMatrix("g_InvViewProj", m_Camera.m_InverseViewProj);
		m_SimpleSample->SetVector("g_SkyLightDir", -m_SkyLight.m_View.column<2>().xyz); // ! RH -z
		m_SimpleSample->SetMatrix("g_SkyLightViewProj", m_SkyLight.m_ViewProj);
		m_SimpleSample->SetTexture("g_ShadowRT", m_ShadowRT);
		RenderPipeline::RenderAllObjects(Material::PassTypeNormalDepth, pd3dDevice, fTime, fElapsedTime);
		V(m_d3dDevice->EndScene());
	}

	V(pd3dDevice->SetRenderTarget(0, m_DiffuseRT->GetSurfaceLevel(0)));
	V(m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0));
	if(SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		m_SimpleSample->SetTexture("g_NormalRT", m_NormalRT);
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
		V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR));
		V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		RenderPipeline::RenderAllObjects(Material::PassTypeDiffuseSpec, pd3dDevice, fTime, fElapsedTime);
		V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		V(m_d3dDevice->EndScene());
	}

	V(pd3dDevice->SetRenderTarget(0, OldRT));
	V(m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,45,50,170), 0, 0));
	if(SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		m_SimpleSample->SetTexture("g_DiffuseRT", m_DiffuseRT);
		RenderPipeline::RenderAllObjects(Material::PassTypeTextureColor, pd3dDevice, fTime, fElapsedTime);
		V(m_d3dDevice->EndScene());
	}

	if(SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		V(pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
		V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR));
		V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE));
		RenderPipeline::RenderAllObjects(Material::PassTypeTransparent, pd3dDevice, fTime, fElapsedTime);
		V(pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE));
		V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera.m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera.m_Proj);
		DrawHelper::EndLine(m_d3dDevice, Matrix4::identity);

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
		OnUIRender(m_UIRender.get(), fTime, fElapsedTime);
		m_UIRender->End();
		V(m_d3dDevice->EndScene());
	}

	RenderPipeline::ClearAllObjects();
}

void Game::OnUIRender(
	my::UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime);

	_ASSERT(m_Font);

	m_UIRender->SetWorld(Matrix4::identity);

	ScrInfoType::const_iterator info_iter = m_ScrInfos.begin();
	for (int y = 5; info_iter != m_ScrInfos.end(); info_iter++, y += m_Font->m_LineHeight)
	{
		m_Font->DrawString(m_UIRender.get(), info_iter->second.c_str(), Rectangle::LeftTop(5,(float)y,500,10), D3DCOLOR_ARGB(255,255,255,0));
	}
}

void Game::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	DrawHelper::BeginLine();

	PhysXSceneContext::PushRenderBuffer(this);

	OnFrameMove(fTime, fElapsedTime);

	PhysXSceneContext::OnTickPreRender(fElapsedTime);

	ParallelTaskManager::DoAllParallelTasks();

	OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

	m_FModSystem->update();

	Present(NULL,NULL,NULL,NULL);

	PhysXSceneContext::OnTickPostRender(fElapsedTime);
}

LRESULT Game::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	if(m_Console
		&& uMsg == WM_CHAR && (WCHAR)wParam == L'`')
	{
		m_Console->SetVisible(!m_Console->GetVisible());
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if((*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	if((*pbNoFurtherProcessing = InputMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
	{
		return 0;
	}

	LRESULT lr;
	if(lr = m_Camera.MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
	{
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
  lua_sethook(Game::getSingleton()._state, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
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
//	Game::getSingleton().puts(ms2ws(msg));
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

void Game::OnResourceFailed(const std::string & error_str)
{
	m_LastErrorStr = error_str;

	_ASSERT(m_Console && m_Console->m_Panel);

	AddLine(ms2ws(error_str), D3DCOLOR_ARGB(255,255,255,255));

	if(m_Console && !m_Console->GetVisible())
	{
		m_Console->SetVisible(true);
	}
}

void Game::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch(code)
	{
	case PxErrorCode::eDEBUG_INFO:
		AddLine(ms2ws(str_printf("%s (%d) : info: %s", file, line, message)));
		break;

	case PxErrorCode::eDEBUG_WARNING:
	case PxErrorCode::ePERF_WARNING:
		AddLine(ms2ws(str_printf("%s (%d) : warning: %s", file, line, message)), D3DCOLOR_ARGB(255,255,255,0));
		break;

	default:
		OutputDebugStringA(str_printf("%s (%d) : error: %s\n", file, line, message).c_str());
		AddLine(ms2ws(str_printf("%s, (%d) : error: %s", file, line, message)), D3DCOLOR_ARGB(255,255,0,0));
		break;
	}
}

void Game::AddLine(const std::wstring & str, D3DCOLOR Color)
{
	if (m_Console)
	{
		m_Console->m_Panel->AddLine(str, Color);
	}
}

void Game::puts(const std::wstring & str)
{
	if (m_Console)
	{
		m_Console->m_Panel->puts(str);
	}
}

bool Game::ExecuteCode(const char * code) throw()
{
	if(dostring(_state, code, "Game::ExecuteCode") && !lua_isnil(_state, -1))
	{
		std::string msg = lua_tostring(_state, -1);
		if(msg.empty())
			msg = "error object is not a string";
		lua_pop(_state, 1);

		OnResourceFailed(msg);

		return false;
	}
	return true;
}

static size_t hash_value(const Game::ShaderCacheKey & key)
{
	size_t seed = 0;
	boost::hash_combine(seed, key.get<0>());
	boost::hash_combine(seed, key.get<1>());
	boost::hash_combine(seed, key.get<2>());
	boost::hash_combine(seed, key.get<3>());
	return seed;
}

my::Effect * Game::QueryShader(Material::MeshType mesh_type, unsigned int PassID, bool bInstance, const Material * material)
{
	_ASSERT(material && 0 != material->m_TextureMask);

	ShaderCacheKey key(mesh_type, bInstance, PassID, material->m_TextureMask);
	ShaderCacheMap::iterator shader_iter = m_ShaderCache.find(key);
	if (shader_iter != m_ShaderCache.end())
	{
		return shader_iter->second.get();
	}

	struct Header
	{
		static const char * vs_header(unsigned int mesh_type)
		{
			switch (mesh_type)
			{
			case Material::MeshTypeAnimation:
				return "MeshSkeleton.fx";
			case Material::MeshTypeParticle:
				return "MeshParticle.fx";
			}
			return "MeshStatic.fx";
		}

		static const char * ps_header(unsigned int pass_type)
		{
			switch (pass_type)
			{
			case Material::PassTypeShadow:
				return "PassShadow.fx";
			case Material::PassTypeNormalDepth:
				return "PassNormalDepth.fx";
			case Material::PassTypeDiffuseSpec:
				return "PassDiffuseSpec.fx";
			case Material::PassTypeTransparent:
				return "PassTransparent.fx";
			}
			return "PassTextureColor.fx";
		}

		static const char * tx_macro(unsigned int texture_type)
		{
			switch (texture_type)
			{
			case Material::TextureTypeNormal:
				return "TEXTURE_TYPE_NORMAL";
			case Material::TextureTypeSpecular:
				return "TEXTURE_TYPE_SPECULAR";
			}
			return "TEXTURE_TYPE_DIFFUSE";
		}
	};

	std::ostringstream oss;
	oss << "#define SHADOW_MAP_SIZE " << SHADOW_MAP_SIZE << std::endl;
	oss << "#define SHADOW_EPSILON " << SHADOW_EPSILON << std::endl;
	oss << "#include \"CommonHeader.fx\"" << std::endl;
	oss << "#include \"" << Header::vs_header(mesh_type) << "\"" << std::endl;
	oss << "#include \"" << Header::ps_header(PassID) << "\"" << std::endl;
	oss << "technique RenderScene {\n"
		"	pass P0 {\n"
		"		VertexShader = compile vs_2_0 RenderSceneVS();\n"
		"		PixelShader  = compile ps_2_0 RenderScenePS();}}";
	std::string source = oss.str();

	std::vector<D3DXMACRO> macros;
	for (unsigned int texture_type = 0; texture_type < Material::TextureTypeNum; texture_type++)
	{
		if (material->m_TextureMask & Material::TextureTypeToMask(texture_type))
		{
			D3DXMACRO macro = {Header::tx_macro(texture_type), 0};
			macros.push_back(macro);
		}
	}
	D3DXMACRO end = {0};
	macros.push_back(end);

	CComPtr<ID3DXBuffer> buff;
	if (SUCCEEDED(D3DXPreprocessShader(source.c_str(), source.length(), &macros[0], this, &buff, NULL)))
	{
		OStreamPtr ostr = FileOStream::Open(str_printf(_T("shader_%u_%u_%u_%u.fx"), bInstance, mesh_type, PassID, material->m_TextureMask).c_str());
		ostr->write(buff->GetBufferPointer(), buff->GetBufferSize()-1);
	}

	EffectPtr shader(new Effect());
	try
	{
		shader->CreateEffect(m_d3dDevice, source.c_str(), source.length(), &macros[0], this, 0, m_EffectPool);
	}
	catch (const my::Exception & e)
	{
		AddLine(ms2ws(e.what()), D3DCOLOR_ARGB(255,255,0,0));
		shader.reset();
	}
	m_ShaderCache.insert(std::make_pair(key, shader));
	return shader.get();
}

void Game::ClearAllShaders(void)
{
	m_ShaderCache.clear();
}

void Game::AddActor(ActorPtr actor)
{
	_ASSERT(std::find(m_Actors.begin(), m_Actors.end(), actor) == m_Actors.end());
	m_Actors.push_back(actor);
}

void Game::RemoveActor(ActorPtr actor)
{
	ActorPtrList::iterator actor_iter = std::find(m_Actors.begin(), m_Actors.end(), actor);
	_ASSERT(actor_iter != m_Actors.end());
	m_Actors.erase(actor_iter);
}

void Game::RemoveAllActors(void)
{
	m_Actors.clear();
}

ClothComponentPtr Game::AddClothComponentFromFile(Actor * owner, const std::string & mesh_path, const std::string & skel_path, const std::string & root_name)
{
	return ActorResourceMgr::AddClothComponentFromFile(
		owner, boost::make_tuple(m_Cooking.get(), m_sdk.get(), m_PxScene.get()), mesh_path, skel_path, root_name, PxClothCollisionData());
}

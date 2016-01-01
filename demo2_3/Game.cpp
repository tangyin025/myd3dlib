#include "stdafx.h"
#include "Game.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

extern void Export2Lua(lua_State * L);

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

void EffectUIRender::Flush(void)
{
	if(m_UIEffect->m_ptr)
	{
		for(UINT p = 0; p < m_Passes; p++)
		{
			m_UIEffect->BeginPass(p);
			UIRender::Flush();
			m_UIEffect->EndPass();
		}
	}
}

Game::Game(void)
	: m_Root(Vector3(-1000), Vector3(1000), 1.0f)
{
	Export2Lua(_state);
	m_NormalRT.reset(new Texture2D());
	m_PositionRT.reset(new Texture2D());
	m_LightRT.reset(new Texture2D());
	m_OpaqueRT.reset(new Texture2D());
	for (unsigned int i = 0; i < _countof(m_DownFilterRT); i++)
	{
		m_DownFilterRT[i].reset(new Texture2D());
	}
	m_Camera.reset(new FirstPersonCamera(D3DXToRadian(75.0f),1.333333f,0.1f,3000.0f));
	m_SkyLightCam.reset(new my::OrthoCamera(sqrt(30*30*2.0f),1.0f,-100,100));
	m_Logic.reset(new Logic());
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

	if(FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (!FModContext::OnInit())
	{
		THROW_CUSEXCEPTION("FModContext::OnInit failed");
	}

	m_UIRender.reset(new EffectUIRender(pd3dDevice, LoadEffect("shader/UIEffect.fx", "")));

	if (!(m_UIRender->m_TexWhite = LoadTexture("texture/white.bmp")))
	{
		THROW_CUSEXCEPTION("create m_UIRender->m_TexWhite failed");
	}

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

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	if(FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.begin();
	for (; shader_iter != m_ShaderCache.end(); shader_iter++)
	{
		if (shader_iter->second)
		{
			shader_iter->second->OnResetDevice();
		}
	}

	m_NormalRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_PositionRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	m_OpaqueRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < _countof(m_DownFilterRT); i++)
	{
		m_DownFilterRT[i]->CreateTexture(
			pd3dDevice, pBackBufferSurfaceDesc->Width / 4, pBackBufferSurfaceDesc->Height / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}

	Vector2 Viewport(600 * (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height, 600);

	Vector2 Scale(pBackBufferSurfaceDesc->Width / Viewport.x, pBackBufferSurfaceDesc->Height / Viewport.y);

	DialogMgr::SetDlgViewport(Viewport, D3DXToRadian(75.0f));

	m_Font->SetScale(Scale);

	if(boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->EventAlign)
	{
		boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->EventAlign(&EventArgs());
	}

	return S_OK;
}

void Game::OnLostDevice(void)
{
	AddLine(L"Game::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	ResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();

	ShaderCacheMap::iterator shader_iter = m_ShaderCache.begin();
	for (; shader_iter != m_ShaderCache.end(); shader_iter++)
	{
		if (shader_iter->second)
		{
			shader_iter->second->OnLostDevice();
		}
	}

	m_NormalRT->OnDestroyDevice();
	m_PositionRT->OnDestroyDevice();
	m_LightRT->OnDestroyDevice();
	m_OpaqueRT->OnDestroyDevice();
	for (unsigned int i = 0; i < _countof(m_DownFilterRT); i++)
	{
		m_DownFilterRT[i]->OnDestroyDevice();
	}
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	ParallelTaskManager::StopParallelThread();

	ExecuteCode("collectgarbage(\"collect\")");

	m_Console.reset();

	RemoveAllDlg();

	m_SimpleSample.reset();

	m_UIRender.reset();

	RemoveAllTimer();

	RenderPipeline::OnDestroyDevice();

	m_ShaderCache.clear();

	ResourceMgr::OnDestroyDevice();

	InputMgr::Destroy();

	ImeEditBox::Uninitialize();

	FModContext::OnShutdown();
}

void Game::OnPxThreadSubstep(float dtime)
{
	// ! take care of multi thread
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	m_SimpleSample->SetFloat("g_Time", (float)m_fAbsoluteTime);
	m_SimpleSample->SetFloatArray("g_ScreenDim", (float *)&Vector2((float)m_BackBufferSurfaceDesc.Width, (float)m_BackBufferSurfaceDesc.Height), 2);

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		V(pd3dDevice->GetRenderTarget(0, &m_OldRT));
		V(pd3dDevice->GetDepthStencilSurface(&m_OldDS));
		m_BkColor = D3DCOLOR_ARGB(0,45,50,170);
		RenderPipeline::OnFrameRender(pd3dDevice, &m_BackBufferSurfaceDesc, this, fTime, fElapsedTime);
		m_OldRT.Release();
		m_OldDS.Release();

		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);
		DrawHelper::EndLine(pd3dDevice, Matrix4::identity);

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
		OnUIRender(m_UIRender.get(), fTime, fElapsedTime);
		m_UIRender->End();
		V(pd3dDevice->EndScene());
	}
}

void Game::OnUIRender(
	my::UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogMgr::Draw(ui_render, fTime, fElapsedTime);
	_ASSERT(m_Font);
	ScrInfoType::const_iterator info_iter = m_ScrInfos.begin();
	for (int y = 5; info_iter != m_ScrInfos.end(); info_iter++, y += m_Font->m_LineHeight)
	{
		m_Font->PushString(ui_render, &info_iter->second[0], Rectangle::LeftTop(5,(float)y,500,10), D3DCOLOR_ARGB(255,255,255,0));
	}
	ui_render->Flush();
}

void Game::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	DrawHelper::BeginLine();

	CheckIORequests();

	InputMgr::Update(fTime, fElapsedTime);

	TimerMgr::Update(fTime, fElapsedTime);

	FModContext::OnUpdate();

	boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->Update(fTime, fElapsedTime);

	boost::static_pointer_cast<my::OrthoCamera>(m_SkyLightCam)->Update(fTime, fElapsedTime);

	ParallelTaskManager::DoAllParallelTasks();

	OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

	Present(NULL,NULL,NULL,NULL);
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
	if(lr = boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
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

IDirect3DSurface9 * Game::GetScreenSurface(void)
{
	return m_OldRT;
}

IDirect3DSurface9 * Game::GetScreenDepthStencilSurface(void)
{
	return m_OldDS;
}

IDirect3DSurface9 * Game::GetNormalSurface(void)
{
	return m_NormalRT->GetSurfaceLevel(0);
}

my::Texture2D * Game::GetNormalTexture(void)
{
	return m_NormalRT.get();
}

IDirect3DSurface9 * Game::GetPositionSurface(void)
{
	return m_PositionRT->GetSurfaceLevel(0);
}

my::Texture2D * Game::GetPositionTexture(void)
{
	return m_PositionRT.get();
}

IDirect3DSurface9 * Game::GetLightSurface(void)
{
	return m_LightRT->GetSurfaceLevel(0);
}

my::Texture2D * Game::GetLightTexture(void)
{
	return m_LightRT.get();
}

IDirect3DSurface9 * Game::GetOpaqueSurface(void)
{
	return m_OpaqueRT->GetSurfaceLevel(0);
}

my::Texture2D * Game::GetOpaqueTexture(void)
{
	return m_OpaqueRT.get();
}

IDirect3DSurface9 * Game::GetDownFilterSurface(unsigned int i)
{
	_ASSERT(i < _countof(m_DownFilterRT));
	return m_DownFilterRT[i]->GetSurfaceLevel(0);
}

my::Texture2D * Game::GetDownFilterTexture(unsigned int i)
{
	_ASSERT(i < _countof(m_DownFilterRT));
	return m_DownFilterRT[i].get();
}

static size_t hash_value(const Game::ShaderCacheKey & key)
{
	size_t seed = 0;
	boost::hash_combine(seed, key.get<0>());
	boost::hash_combine(seed, key.get<1>());
	boost::hash_combine(seed, key.get<2>());
	return seed;
}

my::Effect * Game::QueryShader(RenderPipeline::MeshType mesh_type, bool bInstance, const Material * material, unsigned int PassID)
{
	_ASSERT(material && !material->m_Shader.empty());

	ShaderCacheKey key(mesh_type, bInstance, material->m_Shader);
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
			case RenderPipeline::MeshTypeAnimation:
				return "MeshSkeleton.fx";
			case RenderPipeline::MeshTypeParticle:
				return "MeshParticle.fx";
			}
			return "MeshStatic.fx";
		}
	};

	std::ostringstream oss;
	oss << "#define SHADOW_MAP_SIZE " << SHADOW_MAP_SIZE << std::endl;
	oss << "#define SHADOW_EPSILON " << SHADOW_EPSILON << std::endl;
	oss << "#define INSTANCE " << (unsigned int)bInstance << std::endl;
	oss << "#include \"CommonHeader.fx\"" << std::endl;
	oss << "#include \"" << Header::vs_header(mesh_type) << "\"" << std::endl;
	oss << "#include \"" << material->m_Shader << "\"" << std::endl;
	std::string source = oss.str();

	CComPtr<ID3DXBuffer> buff;
	if (SUCCEEDED(D3DXPreprocessShader(source.c_str(), source.length(), NULL, this, &buff, NULL)))
	{
		OStreamPtr ostr = FileOStream::Open(str_printf(_T("%S_%u_%u.fx"), material->m_Shader.c_str(), mesh_type, bInstance).c_str());
		ostr->write(buff->GetBufferPointer(), buff->GetBufferSize()-1);
	}

	EffectPtr shader(new Effect());
	try
	{
		shader->CreateEffect(m_d3dDevice, source.c_str(), source.size(), NULL, this, 0, m_EffectPool);
	}
	catch (const my::Exception & e)
	{
		OnResourceFailed(e.what());
		shader.reset();
	}
	m_ShaderCache.insert(std::make_pair(key, shader));
	return shader.get();
}

void Game::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public my::IQueryCallback
	{
		const my::Frustum & m_frustum;

		RenderPipeline * m_pipeline;

		unsigned int m_PassMask;

		CallBack(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
			: m_frustum(frustum)
			, m_pipeline(pipeline)
			, m_PassMask(PassMask)
		{
		}

		void operator() (OctComponent * oct_cmp, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Component *>(oct_cmp));
			Component * cmp = static_cast<Component *>(oct_cmp);
			cmp->AddToPipeline(m_pipeline, m_PassMask);
		}
	};

	m_Root.QueryComponent(frustum, &CallBack(frustum, pipeline, PassMask));
}

void Game::SaveMaterial(const std::string & path, MaterialPtr material)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Material", material);
}

void Game::SaveEmitter(const std::string & path, my::EmitterPtr emitter)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("Emitter", emitter);
}

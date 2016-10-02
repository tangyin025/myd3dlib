#include "stdafx.h"
#include "Game.h"
#include <sstream>
#include <fstream>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/iterator_policy.hpp>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	UINT m_Passes;

public:
	EffectUIRender(IDirect3DDevice9 * pd3dDevice, my::EffectPtr effect)
		: UIRender(pd3dDevice)
		, m_UIEffect(effect)
		, m_Passes(0)
	{
		_ASSERT(m_UIEffect);
	}

	void Begin(void)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetVector("g_ScreenDim", Vector4(
				(float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Width, (float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Height, 0, 0));
			m_Passes = m_UIEffect->Begin();
		}
	}

	void End(void)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->End();
			m_Passes = 0;
		}
	}

	void SetWorld(const Matrix4 & World)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetMatrix("g_World", World);
		}
	}

	void SetViewProj(const Matrix4 & ViewProj)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetMatrix("g_ViewProj", ViewProj);
		}
	}

	void Flush(void)
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
};

static int lua_print(lua_State * L)
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
			LUA_QL("print"));
		if (i>1)
			Game::getSingleton().puts(L"\t");
		else
			Game::getSingleton().AddLine(L"", D3DCOLOR_ARGB(255,255,255,255));
		Game::getSingleton().puts(u8tows(s));
		lua_pop(L, 1);  /* pop result */
	}
	return 0;
}

typedef struct LoadF {
	int extraline;
	//FILE *f;
	my::IStreamPtr stream;
	char buff[LUAL_BUFFERSIZE];
} LoadF;

static const char *getF (lua_State *L, void *ud, size_t *size) {
	LoadF *lf = (LoadF *)ud;
	(void)L;
	if (lf->extraline) {
		lf->extraline = 0;
		*size = 1;
		return "\n";
	}
	//if (feof(lf->f)) return NULL;
	*size = lf->stream->read(lf->buff, sizeof(lf->buff));
	return (*size > 0) ? lf->buff : NULL;
}
//
//static int errfile (lua_State *L, const char *what, int fnameindex) {
//	const char *serr = strerror(errno);
//	const char *filename = lua_tostring(L, fnameindex) + 1;
//	lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
//	lua_remove(L, fnameindex);
//	return LUA_ERRFILE;
//}

static int luaL_loadfile (lua_State *L, const char *filename)
{
	LoadF lf;
	//int status, readstatus;
	//int c;
	int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
	lf.extraline = 0;
	//if (filename == NULL) {
	//	lua_pushliteral(L, "=stdin");
	//	lf.f = stdin;
	//}
	//else {
		lua_pushfstring(L, "@%s", filename);
	//	lf.f = fopen(filename, "r");
	//	if (lf.f == NULL) return errfile(L, "open", fnameindex);
	//}
	//c = getc(lf.f);
	//if (c == '#') {  /* Unix exec. file? */
	//	lf.extraline = 1;
	//	while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
	//	if (c == '\n') c = getc(lf.f);
	//}
	//if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
	//	lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
	//	if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
	//	/* skip eventual `#!...' */
	//	while ((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
	//	lf.extraline = 0;
	//}
	//ungetc(c, lf.f);
	try
	{
		lf.stream = Game::getSingleton().OpenIStream(filename);
	}
	catch(const my::Exception & e)
	{
		lua_pushfstring(L, e.what().c_str());
		lua_remove(L, fnameindex);
		return LUA_ERRFILE;
	}
	int status = lua_load(L, getF, &lf, lua_tostring(L, -1));
	//readstatus = ferror(lf.f);
	//if (filename) fclose(lf.f);  /* close file (even in case of errors) */
	//if (readstatus) {
	//	lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
	//	return errfile(L, "read", fnameindex);
	//}
	lua_remove(L, fnameindex);
	return status;
}

static int load_aux (lua_State *L, int status) {
	if (status == 0)  /* OK? */
		return 1;
	else {
		lua_pushnil(L);
		lua_insert(L, -2);  /* put before error message */
		return 2;  /* return nil plus error message */
	}
}

static int luaB_loadfile (lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	return load_aux(L, luaL_loadfile(L, fname));
}

static int luaB_dofile (lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	int n = lua_gettop(L);
	if (luaL_loadfile(L, fname) != 0) lua_error(L);
	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L) - n;
}

static void loaderror (lua_State *L, const char *filename) {
  luaL_error(L, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s",
                lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

static int loader_Lua (lua_State *L) {
  //const char *filename;
  const char *name = luaL_checkstring(L, 1);
  //filename = findfile(L, name, "path");
  //if (filename == NULL) return 1;  /* library not found in this path */
  if (luaL_loadfile(L, name) != 0)
    loaderror(L, name);
  return 1;  /* library loaded successfully */
}

static int os_exit(lua_State * L)
{
	Game::getSingleton().m_wnd->SendMessage(WM_CLOSE);
	return 0;
}

Game::Game(void)
	: m_Root(Vector3(-3000), Vector3(3000), 1.0f)
{
	LuaContext::Init();
	lua_pushcfunction(m_State, lua_print);
	lua_setglobal(m_State, "print");
	lua_pushcfunction(m_State, luaB_loadfile);
	lua_setglobal(m_State, "loadfile");
	lua_pushcfunction(m_State, luaB_dofile);
	lua_setglobal(m_State, "dofile");
	lua_getglobal(m_State, "package");
	lua_getfield(m_State, -1, "loaders");
	lua_pushcfunction(m_State, loader_Lua);
	lua_rawseti(m_State, -2, 2);
	lua_getglobal(m_State, "os");
	lua_pushcclosure(m_State, os_exit, 0);
	lua_setfield(m_State, -2, "exit");
	lua_settop(m_State, 0);
	luabind::module(m_State)
	[
		luabind::class_<Game, luabind::bases<my::DxutApp, my::ResourceMgr, my::DialogMgr> >("Game")
			.def("AddTimer", &Game::AddTimer)
			.def("InsertTimer", &Game::InsertTimer)
			.def("RemoveTimer", &Game::RemoveTimer)
			.def("RemoveAllTimer", &Game::RemoveAllTimer)
			.def_readonly("Root", &Game::m_Root)
			.def_readonly("Console", &Game::m_Console)
			.def_readwrite("Camera", &Game::m_Camera)
			.def_readwrite("SkyLightCam", &Game::m_SkyLightCam)
			.def_readwrite("SkyLightDiffuse", &Game::m_SkyLightDiffuse)
			.def_readwrite("SkyLightAmbient", &Game::m_SkyLightAmbient)
			.def_readwrite("WireFrame", &Game::m_WireFrame)
			.def_readwrite("DofEnable", &Game::m_DofEnable)
			.def_readwrite("DofParams", &Game::m_DofParams)
			.def_readwrite("FxaaEnable", &Game::m_FxaaEnable)
			.def("PlaySound", &Game::PlaySound)
			.def("SaveDialog", &Game::SaveDialog)
			.def("LoadDialog", &Game::LoadDialog)
	];
	luabind::globals(m_State)["game"] = this;

	m_NormalRT.reset(new Texture2D());
	m_PositionRT.reset(new Texture2D());
	m_LightRT.reset(new Texture2D());
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i].reset(new Texture2D());
		m_DownFilterRT.m_RenderTarget[i].reset(new Texture2D());
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
	DxutApp::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

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

	if (!PhysXContext::Init())
	{
		THROW_CUSEXCEPTION("PhysXContext::Init failed");
	}

	if (!PhysXSceneContext::Init(m_sdk.get(), m_CpuDispatcher.get()))
	{
		THROW_CUSEXCEPTION("PhysXSceneContext::Init failed");
	}

	if (!FModContext::Init())
	{
		THROW_CUSEXCEPTION("FModContext::Init failed");
	}

	m_UIRender.reset(new EffectUIRender(pd3dDevice, LoadEffect("shader/UIEffect.fx", "")));

	if (!(m_Font = LoadFont("font/wqy-microhei.ttc", 13)))
	{
		THROW_CUSEXCEPTION("create m_Font failed");
	}

	m_Console = ConsolePtr(new Console());

	m_Console->SetVisible(false);

	DialogMgr::InsertDlg(m_Console);

	AddLine(L"Game::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_Logic->Create();

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	AddLine(L"Game::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	if (FAILED(hr = DxutApp::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_NormalRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_PositionRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->CreateTexture(
			pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

		m_DownFilterRT.m_RenderTarget[i]->CreateTexture(
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

	DxutApp::OnLostDevice();

	ResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();

	m_NormalRT->OnDestroyDevice();

	m_PositionRT->OnDestroyDevice();

	m_LightRT->OnDestroyDevice();

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->OnDestroyDevice();

		m_DownFilterRT.m_RenderTarget[i]->OnDestroyDevice();
	}
}

void Game::OnDestroyDevice(void)
{
	AddLine(L"Game::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_Logic->Destroy();

	DxutApp::OnDestroyDevice();

	ResourceMgr::OnDestroyDevice();

	RenderPipeline::OnDestroyDevice();

	ParallelTaskManager::StopParallelThread();

	ExecuteCode("collectgarbage(\"collect\")");

	m_Console.reset();

	m_Root.ClearAllComponents();

	m_ViewedCmps.clear();

	RemoveAllDlg();

	m_SimpleSample.reset();

	m_UIRender.reset();

	RemoveAllTimer();

	m_ShaderCache.clear();

	InputMgr::Destroy();

	ImeEditBox::Uninitialize();

	PhysXSceneContext::Shutdown();

	PhysXContext::Shutdown();

	FModContext::Shutdown();
}

void Game::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	m_SimpleSample->SetFloatArray("g_ScreenDim", (float *)&Vector2((float)m_BackBufferSurfaceDesc.Width, (float)m_BackBufferSurfaceDesc.Height), 2);

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		RenderPipeline::OnFrameRender(pd3dDevice, &m_BackBufferSurfaceDesc, this, fTime, fElapsedTime);

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

	PhysXSceneContext::PushRenderBuffer(this);

	FModContext::Update();

	m_Logic->Update(fElapsedTime);

	ComponentSet::iterator cmp_iter = m_ViewedCmps.begin();
	for (; cmp_iter != m_ViewedCmps.end(); cmp_iter++)
	{
		(*cmp_iter)->Update(fElapsedTime);
	}

	boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->Update(fTime, fElapsedTime);

	boost::static_pointer_cast<my::OrthoCamera>(m_SkyLightCam)->Update(fTime, fElapsedTime);

	ParallelTaskManager::DoAllParallelTasks();

	PhysXSceneContext::TickPreRender(fElapsedTime);

	OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

	Present(NULL,NULL,NULL,NULL);

	PhysXSceneContext::TickPostRender(fElapsedTime);
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

	//LRESULT lr;
	//if(lr = boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
	//{
	//	return lr;
	//}
	return 0;
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
	if(dostring(code, "Game::ExecuteCode") && !lua_isnil(m_State, -1))
	{
		std::string msg = lua_tostring(m_State, -1);
		if(msg.empty())
			msg = "error object is not a string";
		lua_pop(m_State, 1);

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
			case RenderPipeline::MeshTypeTerrain:
				return "MeshTerrain.fx";
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
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;

		CallBack(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
		{
		}

		void operator() (OctComponent * oct_cmp, IntersectionTests::IntersectionType)
		{
			RenderComponent * render_cmp = dynamic_cast<RenderComponent *>(oct_cmp);
			if (render_cmp)
			{
				render_cmp->AddToPipeline(frustum, pipeline, PassMask);
			}
		}
	};

	m_Root.QueryComponent(frustum, &CallBack(frustum, pipeline, PassMask));
}

void Game::ResetViewedCmps(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
	const Vector3 OutExtent(1050,1050,1050);
	AABB OutBox(TargetPos - OutExtent, TargetPos + OutExtent);
	ComponentSet::iterator cmp_iter = m_ViewedCmps.begin();
	for (; cmp_iter != m_ViewedCmps.end(); )
	{
		if (IntersectionTests::IntersectionTypeOutside
			== IntersectionTests::IntersectAABBAndAABB(OutBox, Component::GetComponentAABB(*cmp_iter)))
		{
			if ((*cmp_iter)->IsRequested())
			{
				(*cmp_iter)->ReleaseResource();
			}
			cmp_iter = m_ViewedCmps.erase(cmp_iter);
		}
		else
			cmp_iter++;
	}

	struct CallBack : public my::IQueryCallback
	{
		Game * game;
		const my::Vector3 & ViewedPos;
		const my::Vector3 & TargetPos;
		CallBack(Game * _game, const my::Vector3 & _ViewedPos, const my::Vector3 & _TargetPos)
			: game(_game)
			, ViewedPos(_ViewedPos)
			, TargetPos(_TargetPos)
		{
		}
		void operator() (OctComponent * oct_cmp, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Component *>(oct_cmp));
			Component * cmp = static_cast<Component *>(oct_cmp);
			ComponentSet::iterator cmp_iter = game->m_ViewedCmps.find(cmp);
			if (cmp_iter == game->m_ViewedCmps.end())
			{
				if (!cmp->IsRequested())
				{
					cmp->RequestResource();
				}
				game->m_ViewedCmps.insert(cmp);
			}
			cmp->UpdateLod(ViewedPos, TargetPos);
		}
	};

	const Vector3 InExtent(1000,1000,1000);
	AABB InBox(TargetPos - InExtent, TargetPos + InExtent);
	m_Root.QueryComponent(InBox, &CallBack(this, ViewedPos, TargetPos));
}

void Game::SaveDialog(my::DialogPtr dlg, const char * path)
{
	std::ofstream ostr(path);
	boost::archive::polymorphic_xml_oarchive oa(ostr);
	oa << BOOST_SERIALIZATION_NVP(dlg);
}

my::DialogPtr Game::LoadDialog(const char * path)
{
	DialogPtr dlg;
	IStreamBuff buff(OpenIStream(path));
	std::istream istr(&buff);
	boost::archive::polymorphic_xml_iarchive ia(istr);
	ia >> BOOST_SERIALIZATION_NVP(dlg);
	return dlg;
}

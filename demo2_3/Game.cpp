#include "stdafx.h"
#include "Game.h"
#include "Terrain.h"
#include "Material.h"
#include "resource.h"
#include <sstream>
#include <fstream>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/iterator_policy.hpp>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include "PlayerController.h"

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
	EffectUIRender(void)
		: m_Passes(0)
	{
	}

	HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if (FAILED(hr = UIRender::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_UIEffect = my::ResourceMgr::getSingleton().LoadEffect(Game::getSingleton().m_InitUIEffect.c_str(), "");

		return S_OK;
	}

	HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if (FAILED(hr = UIRender::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		return S_OK;
	}

	void OnLostDevice(void)
	{
		UIRender::OnLostDevice();
	}

	void OnDestroyDevice(void)
	{
		UIRender::OnDestroyDevice();

		m_Device.Release();

		m_UIEffect.reset();
	}

	void Begin(void)
	{
		if(m_UIEffect->m_ptr)
		{
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
			Game::getSingleton().puts(L"\n");
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
	: m_UIRender(new EffectUIRender())
	, m_Root(my::AABB(-1024, 1024))
{
	boost::program_options::options_description desc("Options");
	std::vector<std::string> path_list;
	desc.add_options()
		("path", boost::program_options::value<std::vector<std::string> >(&path_list), "Path")
		("width", boost::program_options::value(&m_WindowBackBufferWidthAtModeChange)->default_value(800), "Width")
		("height", boost::program_options::value(&m_WindowBackBufferHeightAtModeChange)->default_value(600), "Height")
		("font", boost::program_options::value(&m_InitFont)->default_value("font/wqy-microhei.ttc"), "Font")
		("fontheight", boost::program_options::value(&m_InitFontHeight)->default_value(13), "Font Height")
		("uieffect", boost::program_options::value(&m_InitUIEffect)->default_value("shader/UIEffect.fx"), "UI Effect")
		("sound", boost::program_options::value(&m_InitSound)->default_value("sound\\aaa.fev"), "Sound")
		("scene", boost::program_options::value(&m_InitScene)->default_value("scene01.xml"), "Scene")
		("script", boost::program_options::value(&m_InitScript)->default_value("dofile 'Main.lua'"), "Script")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(__argc, __targv, desc), vm);
	boost::program_options::notify(vm);
	if (path_list.empty())
	{
		path_list.push_back("Media");
		path_list.push_back("..\\demo2_3\\Media");
	}
	std::vector<std::string>::const_iterator path_iter = path_list.begin();
	for (; path_iter != path_list.end(); path_iter++)
	{
		ResourceMgr::RegisterFileDir(*path_iter);
		ResourceMgr::RegisterZipDir(*path_iter + ".zip");
	}

	m_NormalRT.reset(new Texture2D());
	m_PositionRT.reset(new Texture2D());
	m_LightRT.reset(new Texture2D());
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i].reset(new Texture2D());
		m_DownFilterRT.m_RenderTarget[i].reset(new Texture2D());
	}
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

	if (!(m_Font = LoadFont(m_InitFont.c_str(), m_InitFontHeight)))
	{
		THROW_CUSEXCEPTION("create m_Font failed");
	}

	if (FAILED(hr = m_UIRender->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_Console = ConsolePtr(new Console());

	m_Console->SetVisible(false);

	FModContext::LoadEventFile(m_InitSound.c_str());

	LoadScene(m_InitScene.c_str());

	m_Camera.reset(new PerspectiveCamera(D3DXToRadian(75.0f), 1.333333f, 0.1f, 3000.0f));

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
			.def_readonly("wnd", &Game::m_wnd)
			.def_readwrite("Camera", &Game::m_Camera)
			.def_readonly("SkyLightCam", &Game::m_SkyLightCam)
			.def_readwrite("SkyLightDiffuse", &Game::m_SkyLightDiffuse)
			.def_readwrite("SkyLightAmbient", &Game::m_SkyLightAmbient)
			.def_readwrite("WireFrame", &Game::m_WireFrame)
			.def_readwrite("DofEnable", &Game::m_DofEnable)
			.def_readwrite("DofParams", &Game::m_DofParams)
			.def_readwrite("FxaaEnable", &Game::m_FxaaEnable)
			.def_readwrite("SsaoEnable", &Game::m_SsaoEnable)
			.property("VisualizationParameter", &Game::GetVisualizationParameter, &Game::SetVisualizationParameter)
			.def_readonly("Font", &Game::m_Font)
			.def_readonly("Console", &Game::m_Console)
			.def_readonly("Root", &Game::m_Root)
			.def("PlaySound", &Game::PlaySound)
			.def("SaveDialog", &Game::SaveDialog)
			.def("LoadDialog", &Game::LoadDialog)
			.def("SaveMaterial", &Game::SaveMaterial)
			.def("LoadMaterial", &Game::LoadMaterial)
			.def("SaveComponent", &Game::SaveComponent)
			.def("LoadComponent", &Game::LoadComponent)
			.def("LoadScene", &Game::LoadScene)

		, luabind::class_<PlayerController, Controller, boost::shared_ptr<Controller> >("PlayerController")
			.def(luabind::constructor<Character *>())
	];
	luabind::globals(m_State)["game"] = this;

	ExecuteCode(m_InitScript.c_str());

	DialogMgr::InsertDlg(m_Console);

	m_EventLog("Game::OnCreateDevice");

	return S_OK;
}

HRESULT Game::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_EventLog("Game::OnResetDevice");

	DialogMgr::SetDlgViewport(Vector2(600 * (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height, 600), D3DXToRadian(75.0f));

	m_Font->SetScale(Vector2(pBackBufferSurfaceDesc->Height / 600.0f));

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

	if (FAILED(hr = m_UIRender->OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_NormalRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_PositionRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->CreateTexture(
			pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

		m_DownFilterRT.m_RenderTarget[i]->CreateTexture(
			pBackBufferSurfaceDesc->Width / 4, pBackBufferSurfaceDesc->Height / 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}

	if(m_Camera->EventAlign)
	{
		m_Camera->EventAlign(&ControlEventArgs(NULL));
	}

	return S_OK;
}

void Game::OnLostDevice(void)
{
	m_EventLog("Game::OnLostDevice");

	m_NormalRT->OnDestroyDevice();

	m_PositionRT->OnDestroyDevice();

	m_LightRT->OnDestroyDevice();

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->OnDestroyDevice();

		m_DownFilterRT.m_RenderTarget[i]->OnDestroyDevice();
	}

	m_UIRender->OnLostDevice();

	RenderPipeline::OnLostDevice();

	ResourceMgr::OnLostDevice();

	DxutApp::OnLostDevice();
}

void Game::OnDestroyDevice(void)
{
	m_EventLog("Game::OnDestroyDevice");

	m_Camera.reset();

	m_Root.ClearAllActor();

	m_Console.reset();

	RemoveAllDlg();

	LuaContext::Shutdown();

	m_SimpleSample.reset();

	m_UIRender->OnDestroyDevice();

	RemoveAllTimer();

	ImeEditBox::Uninitialize();

	FModContext::Shutdown();

	PhysXSceneContext::Shutdown();

	PhysXContext::Shutdown();

	RenderPipeline::OnDestroyDevice();

	ParallelTaskManager::StopParallelThread();

	ResourceMgr::OnDestroyDevice();

	InputMgr::Destroy();

	DxutApp::OnDestroyDevice();
}

void Game::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	DrawHelper::BeginLine();

	CheckIORequests(0);

	if (!InputMgr::Capture(fTime, fElapsedTime))
	{
		// TODO: lost user input
	}

	TimerMgr::Update(fTime, fElapsedTime);

	PhysXSceneContext::PushRenderBuffer(this);

	FModContext::Update();

	struct Callback : public my::OctNode::QueryCallback
	{
		float fElapsedTime;
		WeakActorMap & ViewedActors;
		typedef std::vector<WeakActorMap::value_type> WeakActorList;
		WeakActorList actor_list;
		Callback(float _fElapsedTime, WeakActorMap & _ViewedActors)
			: fElapsedTime(_fElapsedTime)
			, ViewedActors(_ViewedActors)
		{
		}
		void operator() (OctActor * oct_actor, const AABB & aabb, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			Game::WeakActorMap::const_iterator actor_iter = ViewedActors.find(actor);
			if (actor_iter != ViewedActors.end())
			{
				ViewedActors.erase(actor);
			}
			else
			{
				_ASSERT(!actor->IsRequested());
				actor->RequestResource();
				actor->OnEnterPxScene(Game::getSingletonPtr());
			}
			actor_list.push_back(std::make_pair(actor, boost::static_pointer_cast<Actor>(actor->shared_from_this())));
		}
	};
	Callback cb(fElapsedTime, m_ViewedActors);
	m_Root.QueryActor(my::AABB(-1000, 1000), &cb);
	WeakActorMap::iterator actor_iter = m_ViewedActors.begin();
	for (; actor_iter != m_ViewedActors.end(); actor_iter++)
	{
		ActorPtr actor = actor_iter->second.lock();
		if (actor)
		{
			_ASSERT(actor->IsRequested());
			actor->OnLeavePxScene(this);
			actor->ReleaseResource();
		}
	}
	m_ViewedActors.clear();
	Callback::WeakActorList::iterator weak_act_iter = cb.actor_list.begin();
	for (; weak_act_iter != cb.actor_list.end(); weak_act_iter++)
	{
		ActorPtr actor = weak_act_iter->second.lock();
		if (actor)
		{
			if (!actor->m_Base)
			{
				actor->Update(fElapsedTime); // ! will change other actors scope, event if octree node
			}
			m_ViewedActors.insert(std::make_pair(actor.get(), actor));
		}
	}

	m_SkyLightCam->UpdateViewProj();

	m_Camera->UpdateViewProj();

	ParallelTaskManager::DoAllParallelTasks();

	PhysXSceneContext::TickPreRender(fElapsedTime);

	m_SimpleSample->SetFloatArray("g_ScreenDim", (float *)&Vector2((float)m_BackBufferSurfaceDesc.Width, (float)m_BackBufferSurfaceDesc.Height), 2);

	if (SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		OnRender(m_d3dDevice, &m_BackBufferSurfaceDesc, this, fTime, fElapsedTime);

		m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);
		DrawHelper::EndLine(m_d3dDevice, Matrix4::identity);

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
		OnUIRender(m_UIRender.get(), fTime, fElapsedTime);
		m_UIRender->End();
		V(m_d3dDevice->EndScene());
	}

	Present(NULL, NULL, NULL, NULL);

	PhysXSceneContext::TickPostRender(fElapsedTime);

	physx::PxU32 nbActiveTransforms;
	const physx::PxActiveTransform* activeTransforms = m_PxScene->getActiveTransforms(nbActiveTransforms);
	for (physx::PxU32 i = 0; i < nbActiveTransforms; ++i)
	{
		Actor * actor = (Actor *)activeTransforms[i].userData;
		actor->OnUpdatePxTransform(activeTransforms[i].actor2World);
	}
}

void Game::OnRender(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	IRenderContext * pRC,
	double fTime,
	float fElapsedTime)
{
	RenderPipeline::OnRender(pd3dDevice, pBackBufferSurfaceDesc, pRC, fTime, fElapsedTime);
}

void Game::OnUIRender(
	my::UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogMgr::Draw(ui_render, fTime, fElapsedTime);
	_ASSERT(m_Font);
	ScrInfoMap::const_iterator info_iter = m_ScrInfo.begin();
	for (int y = 5; info_iter != m_ScrInfo.end(); info_iter++, y += m_Font->m_LineHeight)
	{
		m_Font->PushString(ui_render, &info_iter->second[0], Rectangle::LeftTop(5, (float)y, 500, 10), D3DCOLOR_ARGB(255, 255, 255, 0));
	}
	ui_render->Flush();
}

LRESULT Game::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	if (uMsg == ID_MAIN_TOGGLEREF)
	{
		ToggleREF();
		*pbNoFurtherProcessing = true;
		return 0;
	}

	if (m_Console
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

	//LRESULT lr;
	//if(lr = m_Camera->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
	//{
	//	return lr;
	//}
	return 0;
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

		m_EventLog(msg.c_str());

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

void Game::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const my::Vector3 & ViewPos;
		Callback(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask, const my::Vector3 & _ViewPos)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
			, ViewPos(_ViewPos)
		{
		}
		void operator() (my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos);
		}
	};
	m_Root.QueryActor(frustum, &Callback(frustum, pipeline, PassMask, m_Camera->m_Eye));
}

void Game::DrawStringAtWorld(const my::Vector3 & pos, LPCWSTR lpszText, D3DCOLOR Color, my::Font::Align align)
{
	const Vector3 ptProj = pos.transformCoord(m_Camera->m_ViewProj);
	if (ptProj.z > 0.0f && ptProj.z < 1.0f)
	{
		const Vector2 vp = DialogMgr::GetDlgViewport();
		const Vector2 ptVp(Lerp(0.0f, vp.x, (ptProj.x + 1) / 2), Lerp(0.0f, vp.y, (1 - ptProj.y) / 2));
		m_Font->PushString(m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);
	}
}

void Game::SaveDialog(my::DialogPtr dlg, const char * path)
{
	std::ofstream ostr(GetFullPath(path).c_str());
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

void Game::SaveMaterial(MaterialPtr mat, const char * path)
{
	std::ofstream ofs(GetFullPath(path).c_str());
	boost::archive::polymorphic_xml_oarchive oa(ofs);
	oa << BOOST_SERIALIZATION_NVP(mat);
}

MaterialPtr Game::LoadMaterial(const char * path)
{
	MaterialPtr mat;
	IStreamBuff buff(OpenIStream(path));
	std::istream istr(&buff);
	boost::archive::polymorphic_xml_iarchive ia(istr);
	ia >> BOOST_SERIALIZATION_NVP(mat);
	return mat;
}

void Game::SaveComponent(ComponentPtr cmp, const char * path)
{
	std::ofstream ostr(GetFullPath(path).c_str());
	boost::archive::polymorphic_xml_oarchive oa(ostr);
	oa << BOOST_SERIALIZATION_NVP(cmp);
}

ComponentPtr Game::LoadComponent(const char * path)
{
	ComponentPtr cmp;
	IStreamBuff buff(OpenIStream(path));
	std::istream istr(&buff);
	boost::archive::polymorphic_xml_iarchive ia(istr);
	ia >> BOOST_SERIALIZATION_NVP(cmp);
	return cmp;
}

void Game::LoadScene(const char * path)
{
	m_Root.ClearAllActor();
	PhysXSceneContext::ClearSerializedObjs();
	RenderPipeline::ReleaseResource();

	IStreamBuff buff(OpenIStream(path));
	std::istream istr(&buff);
	boost::archive::polymorphic_xml_iarchive ia(istr);
	ia >> boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)*this);
	ia >> boost::serialization::make_nvp("PhysXSceneContext", (PhysXSceneContext &)*this);
	ia >> boost::serialization::make_nvp("Root", m_Root);

	RenderPipeline::RequestResource();
}

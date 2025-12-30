// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "stdafx.h"
#include "Client.h"
#include "Material.h"
#include "resource.h"
#include "NavigationSerialization.h"
#include "Controller.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include <sstream>
#include <fstream>
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/tag_function.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/shared_container_iterator.hpp>
#include "DebugDraw.h"
#include "LuaExtension.inl"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	D3DXHANDLE handle_ScreenDim;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_ViewProj;

	D3DXHANDLE handle_MeshTexture;

public:
	EffectUIRender(void)
		: handle_ScreenDim(NULL)
		, handle_World(NULL)
		, handle_ViewProj(NULL)
		, handle_MeshTexture(NULL)
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

		m_UIEffect = my::ResourceMgr::getSingleton().LoadEffect(Client::getSingleton().m_InitUIEffect.c_str(), "");
		if (!m_UIEffect)
		{
			return S_FALSE;
		}

		BOOST_VERIFY(handle_ScreenDim = m_UIEffect->GetParameterByName(NULL, "g_ScreenDim"));
		BOOST_VERIFY(handle_World = m_UIEffect->GetParameterByName(NULL, "g_World"));
		BOOST_VERIFY(handle_ViewProj = m_UIEffect->GetParameterByName(NULL, "g_ViewProj"));
		BOOST_VERIFY(handle_MeshTexture = m_UIEffect->GetParameterByName(NULL, "g_MeshTexture"));

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
	}

	void Flush(void)
	{
		m_UIEffect->SetVector(handle_ScreenDim, Vector4(
			(float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Width, (float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Height, 0, 0));
		m_UIEffect->SetMatrix(handle_World, m_World);
		m_UIEffect->SetMatrix(handle_ViewProj, DialogMgr::getSingleton().m_ViewProj);
		V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));

		UILayerList::iterator layer_iter = m_Layer.begin();
		for (; layer_iter != m_Layer.end(); layer_iter++)
		{
			if (!layer_iter->second.empty())
			{
				_ASSERT(layer_iter->first && layer_iter->first->m_ptr);

				m_UIEffect->SetTexture(handle_MeshTexture, layer_iter->first);
				const UINT Passes = m_UIEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
				for (UINT p = 0; p < Passes; p++)
				{
					m_UIEffect->BeginPass(p);
					V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, layer_iter->second.size() / 3, &layer_iter->second[0], sizeof(CUSTOMVERTEX)));
					m_UIEffect->EndPass();
					m_LayerDrawCall++;
				}
				m_UIEffect->End();
			}
		}
		m_Layer.clear();
	}
};

template <typename ElementType>
class DelayRemover
	: public my::Singleton<DelayRemover<ElementType> >
	, protected std::deque<std::pair<boost::function<void(ElementType)>, std::list<ElementType> > >
{
public:
	DelayRemover(void)
	{
	}

	~DelayRemover(void)
	{
		_ASSERT(empty());
	}

	template <typename RemoveFuncType>
	bool IsDelay(const RemoveFuncType& func) const
	{
		return !empty() && back().first == func;
	}

	template <typename RemoveFuncType>
	void Enter(const RemoveFuncType& func)
	{
		_ASSERT(!IsDelay(func));

		deque::push_back(value_type(func, std::list<ElementType>()));
	}

	void push_back(ElementType elem)
	{
		_ASSERT(!empty());

		back().second.insert(back().second.end(), elem);
	}

	void Leave(void)
	{
		_ASSERT(!empty());

		value_type back_dummy = back();

		pop_back();

		std::list<ElementType>::iterator iter = back_dummy.second.begin();
		for (; iter != back_dummy.second.end(); iter++)
		{
			back_dummy.first(*iter);
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
			Client::getSingleton().puts(L"\t");
		else
			Client::getSingleton().puts(L"\n");
		Client::getSingleton().puts(u8tows(s));
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
		lf.stream = Client::getSingleton().OpenIStream(filename);
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
	Client::getSingleton().m_wnd->PostMessage(WM_CLOSE);
	return 0;
}

typedef std::vector<Actor*> ActorList;

typedef boost::shared_container_iterator<ActorList> shared_actor_list_iter;

static boost::iterator_range<shared_actor_list_iter> client_query_entity(Client* self, const my::AABB& aabb)
{
	struct Callback : public OctNode::QueryCallback
	{
		boost::shared_ptr<ActorList> acts;

		Callback(void)
			: acts(new ActorList())
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			acts->push_back(dynamic_cast<Actor*>(oct_entity));
			return true;
		}
	};

	Callback cb;
	self->QueryEntity(aabb, &cb);
	return boost::make_iterator_range(shared_actor_list_iter(cb.acts->begin(), cb.acts), shared_actor_list_iter(cb.acts->end(), cb.acts));
}

namespace boost
{
	namespace program_options
	{
		void validate(boost::any& v,
			const std::vector<std::string>& values,
			my::Vector3*, int)
		{
			static boost::regex r("([+-]?(\\d*[.])?\\d+),([+-]?(\\d*[.])?\\d+),([+-]?(\\d*[.])?\\d+)");

			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			boost::smatch match;
			if (boost::regex_match(s, match, r)) {
				v = boost::any(my::Vector3(
					boost::lexical_cast<float>(match[1]),
					boost::lexical_cast<float>(match[3]),
					boost::lexical_cast<float>(match[5])));
			}
			else {
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
		}
	}
}

Client::Client(int _DEFAULT_UI_RES_Y)
	: OctRoot(-4096, 4096)
	, InputMgr(32)
	, DEFAULT_UI_RES_Y(_DEFAULT_UI_RES_Y)
	, m_UIRender(new EffectUIRender())
	, m_ViewedCenter(0, 0, 0)
	, m_ShowCursorCount(0)
{
	char buff[MAX_PATH];
	GetModuleFileNameA(NULL, buff, _countof(buff));
	m_InitCfg.assign(PathFindFileNameA(buff), PathFindExtensionA(buff));

	boost::program_options::options_description desc("Options");
	std::vector<std::string> path_list;
	bool verticalsync;
	desc.add_options()
		("path", boost::program_options::value(&path_list), "Paths")
		("shaderinclude", boost::program_options::value(&m_SystemIncludes)->default_value(boost::assign::list_of("shader"), ""), "Shader Include")
		("loadshadercache", boost::program_options::value(&m_LoadShaderCache)->default_value(true), "Load Shader Cache")
		("width", boost::program_options::value(&m_WindowBackBufferWidthAtModeChange)->default_value(0), "Width")
		("height", boost::program_options::value(&m_WindowBackBufferHeightAtModeChange)->default_value(0), "Height")
		("windowed", boost::program_options::value(&m_WindowedModeAtFirstCreate)->default_value(false), "Windowed")
		("refreshrate", boost::program_options::value(&m_RefreshRateAtFirstCreate)->default_value(0), "Refresh Rate")
		("verticalsync", boost::program_options::value(&verticalsync)->default_value(false), "Vertical Sync")
		("bloom", boost::program_options::value(&m_BloomEnable)->default_value(false), "Bloom")
		("fxaa", boost::program_options::value(&m_FxaaEnable)->default_value(false), "FXAA")
		("ssao", boost::program_options::value(&m_SsaoEnable)->default_value(false), "SSAO")
		("joystickaxisdeadzone", boost::program_options::value(&m_JoystickAxisDeadZone)->default_value(1000), "Joystick Axis Dead Zone")
		("mousemoveaxiscoef", boost::program_options::value(&m_MouseMoveAxisCoef)->default_value(5000), "Mouse Move Axis Coef")
		("physxframeinterval", boost::program_options::value(&m_FrameInterval)->default_value(1/60.0f), "Physx Frame Interval")
		("maxlowedtimestep", boost::program_options::value(&m_MaxAllowedTimestep)->default_value(0.1f), "Max Allowed Timestep")
		("volume", boost::program_options::value(&m_Volume)->default_value(DSBVOLUME_MAX), "Volume")
		("fov", boost::program_options::value(&m_InitFov)->default_value(60.0f), "Fov")
		("physxsceneflags", boost::program_options::value(&m_InitPhysxSceneFlags)->default_value(physx::PxSceneFlag::eENABLE_PCM | physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS | physx::PxSceneFlag::eENABLE_CCD), "Physx Scene Flags")
		("physxscenegravity", boost::program_options::value(&m_InitPhysxSceneGravity)->default_value(Vector3(0, -19.6f, 0), ""), "Init Physx Scene Gravity")
		("iothreadnum", boost::program_options::value(&m_InitIOThreadNum)->default_value(3), "IO Thread Number")
		("font", boost::program_options::value(&m_InitFont)->default_value("font/SourceHanSansCN-Regular.otf"), "Font")
		("fontheight", boost::program_options::value(&m_InitFontHeight)->default_value(12), "Font Height")
		("fontfaceindex", boost::program_options::value(&m_InitFontFaceIndex)->default_value(0), "Font Face Index")
		("uieffect", boost::program_options::value(&m_InitUIEffect)->default_value("shader/UIEffect.fx"), "UI Effect")
		("vieweddist", boost::program_options::value(&m_ViewedDist)->default_value(1000.0f), "Viewed Distance")
		("actorcullingthreshold", boost::program_options::value(&m_ActorCullingThreshold)->default_value(10.0f), "Actor Culling Threshold")
		("showcmphandle", boost::program_options::value(&m_ShowCmpHandle)->default_value(false), "Show Component Handle")
		("shownavigation", boost::program_options::value(&m_ShowNavigation)->default_value(false), "Show Navigation")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_config_file<char>((m_InitCfg + ".cfg").c_str(), desc, true), vm);
	boost::program_options::store(boost::program_options::parse_command_line(__argc, __targv, desc), vm);
	boost::program_options::notify(vm);

	std::vector<std::string>::const_iterator path_iter = path_list.begin();
	for (; path_iter != path_list.end(); path_iter++)
	{
		if (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(path_iter->c_str()))
		{
			ResourceMgr::RegisterFileDir(*path_iter);
		}
		else
		{
			ResourceMgr::RegisterZipDir(*path_iter);
		}
	}

	m_PresentIntervalAtFirstCreate = verticalsync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;

	m_Camera.reset(new FirstPersonCamera(D3DXToRadian(m_InitFov), 1.333333f, 0.1f, 3000.0f));
	const float k = cos(D3DXToRadian(45));
	const float d = 20.0f;
	m_Camera->m_Eye = my::Vector3(d * k * k, d * k + 1, d * k * k);
	m_Camera->m_Euler = my::Vector3(D3DXToRadian(-45), D3DXToRadian(45), 0);

	m_NormalRT.reset(new Texture2D());
	m_SpecularRT.reset(new Texture2D());
	m_PositionRT.reset(new Texture2D());
	m_LightRT.reset(new Texture2D());
	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i].reset(new Texture2D());
		m_DownFilter4RT.m_RenderTarget[i].reset(new Texture2D());
		m_DownFilter8RT.m_RenderTarget[i].reset(new Texture2D());
	}
}

Client::~Client(void)
{
	//// ! Must manually call destructors at specific order
	//OnDestroyDevice();
}

bool Client::IsDeviceAcceptable(
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

bool Client::ModifyDeviceSettings(
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

HRESULT Client::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	DxutApp::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

	ImeEditBox::Initialize(m_wnd->m_hWnd);

	ImeEditBox::EnableImeSystem(false);

	InputMgr::Create(m_hinst, m_wnd->m_hWnd);

	if (!PhysxSdk::Init())
	{
		THROW_CUSEXCEPTION("PhysxSdk::Init failed");
	}

	if (!PhysxScene::Init(m_sdk.get(), m_CpuDispatcher.get(), physx::PxSceneFlags(m_InitPhysxSceneFlags), m_InitPhysxSceneGravity))
	{
		THROW_CUSEXCEPTION("PhysxScene::Init failed");
	}

	if (!SoundContext::Init(m_wnd->m_hWnd))
	{
		THROW_CUSEXCEPTION("SoundContext::Init failed");
	}

	ResourceMgr::StartIORequestProc(m_InitIOThreadNum);

	ParallelTaskManager::StartParallelThread(m_InitIOThreadNum);

	if(FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if (!(m_Font = LoadFont(m_InitFont.c_str(), m_InitFontHeight, m_InitFontFaceIndex)))
	{
		THROW_CUSEXCEPTION("create m_Font failed");
	}

	if (FAILED(hr = m_UIRender->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	m_Console = ConsolePtr(new Console());

	DialogMgr::InsertDlg(m_Console.get());

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
		luabind::class_<Console, my::Dialog, boost::shared_ptr<Console> >("Console")

		, luabind::class_<Client, luabind::bases<my::DxutApp, my::ResourceMgr, RenderPipeline, PhysxScene> >("Client")
			.def_readonly("wnd", &Client::m_wnd)
			.def_readwrite("DlgView", &Client::m_View)
			.def_readwrite("DlgProj", &Client::m_Proj)
			.def_readwrite("DlgViewProj", &Client::m_ViewProj)
			.def_readonly("LoadShaderCache", &Client::m_LoadShaderCache)
			.def_readwrite("Camera", &Client::m_Camera)
			.def_readwrite("WireFrame", &Client::m_WireFrame)
			.def_readwrite("BloomEnable", &Client::m_BloomEnable)
			.def_readwrite("FxaaEnable", &Client::m_FxaaEnable)
			.def_readwrite("SsaoEnable", &Client::m_SsaoEnable)
			.def_readwrite("RTType", &Client::m_RTType)
			.def_readonly("listener", &Client::m_listener)
			.def_readonly("FrameInterval", &Client::m_FrameInterval)
			.property("Volume", &Client::GetVolume, &Client::SetVolume)
			.def_readonly("DEFAULT_UI_RES_Y", &Client::DEFAULT_UI_RES_Y)
			.def_readonly("Font", &Client::m_Font)
			.def_readonly("Console", &Client::m_Console)
			.def_readwrite("ViewedCenter", &Client::m_ViewedCenter)
			.def_readwrite("ViewedDist", &Client::m_ViewedDist)
			.def_readwrite("ActorCullingThreshold", &Client::m_ActorCullingThreshold)
			.def_readwrite("ShowCmpHandle", &Client::m_ShowCmpHandle)
			.def_readwrite("ShowNavigation", &Client::m_ShowNavigation)
			.def_readonly("keyboard", &Client::m_keyboard)
			.def_readonly("mouse", &Client::m_mouse)
			.def_readonly("joystick", &Client::m_joystick)
			.def_readwrite("JoystickAxisDeadZone", &Client::m_JoystickAxisDeadZone)
			.def_readonly("InitCfg", &Client::m_InitCfg)
			.def("BindKey", &Client::BindKey)
			.def("UnbindKey", &Client::UnbindKey)
			.def("GetKeyAxisRaw", &Client::GetKeyAxisRaw)
			.def("IsKeyDown", &Client::IsKeyDown)
			.def("IsKeyPressRaw", &Client::IsKeyPressRaw)
			.def("IsKeyPress", &Client::IsKeyPress)
			.def("IsKeyRelease", &Client::IsKeyRelease)
			.property("DlgDimension", &Client::GetDlgDimension, &Client::SetDlgDimension)
			.def("InsertDlg", &Client::InsertDlg)
			.def("RemoveDlg", &Client::RemoveDlg)
			.def("RemoveAllDlg", &Client::RemoveAllDlg)
			.def("PushLineVertex", &Client::PushLineVertex)
			.def("PushLine", &Client::PushLine)
			.def("PushLineAABB", &Client::PushLineAABB)
			.def("PushLineBox", &Client::PushLineBox)
			.def("PushTriangleVertex", &Client::PushTriangleVertex)
			.def("PushTriangle", &Client::PushTriangle)
			.def("AddEntity", &Client::AddEntity)
			.def("AddEntity", luabind::tag_function<void(Client*, Actor*)>(
				boost::bind(boost::mem_fn(&Client::AddEntity), boost::placeholders::_1, boost::placeholders::_2, boost::bind(&AABB::transform, boost::bind(&Actor::m_aabb, boost::placeholders::_2), boost::bind(&Actor::m_World, boost::placeholders::_2)), Actor::MinBlock, Actor::Threshold)))
			.def("RemoveEntity", &Client::RemoveEntity)
			.def("ClearAllEntity", &Client::ClearAllEntity)
			.property("AllEntityNum", &Client::GetAllEntityNum)
			.property("AllEntityAABB", luabind::tag_function<AABB(Client*)>(
				boost::bind(&Client::GetAllEntityAABB, boost::placeholders::_1, AABB::Invalid())))
			.def("QueryEntity", &client_query_entity, luabind::return_stl_iterator)
			.def("RemoveViewedActor", &Client::RemoveViewedActor)
			//.def("OnControlSound", &Client::OnControlSound)
			.def("Play", (SoundEventPtr(SoundContext::*)(my::WavPtr, float, float, bool)) & Client::Play)
			.def("Play", (SoundEventPtr(SoundContext::*)(my::WavPtr, float, float, bool, const my::Vector3&, const my::Vector3&, float, float)) & Client::Play)
			.def("LoadSceneAsync", &Client::LoadSceneAsync<luabind::object>)
			.def("LoadScene", &Client::LoadScene)
			.def("GetLoadSceneProgress", &Client::GetLoadSceneProgress, luabind::pure_out_value(boost::placeholders::_4) + luabind::pure_out_value(boost::placeholders::_5) + luabind::pure_out_value(boost::placeholders::_6) + luabind::pure_out_value(boost::placeholders::_7))

		//, luabind::def("res2scene", (boost::shared_ptr<SceneContext>(*)(const boost::shared_ptr<my::DeviceResourceBase>&)) & boost::dynamic_pointer_cast<SceneContext, my::DeviceResourceBase>)
	];

	m_EventLog("Client::OnCreateDevice");

	return S_OK;
}

HRESULT Client::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_EventLog("Client::OnResetDevice");

	boost::dynamic_pointer_cast<my::PerspectiveCamera>(m_Camera)->m_Aspect = (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height;

	DialogMgr::SetDlgDimension(Vector2(DEFAULT_UI_RES_Y * boost::dynamic_pointer_cast<my::PerspectiveCamera>(m_Camera)->m_Aspect, DEFAULT_UI_RES_Y), D3DXToRadian(75.0f));

	FontLibrary::m_Scale = Vector2(pBackBufferSurfaceDesc->Height / DialogMgr::GetDlgDimension().y);

	FontLibrary::m_EventScaleChanged(FontLibrary::m_Scale);

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
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT);

	m_SpecularRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

	m_PositionRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_LightRT->CreateTexture(
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT);

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->CreateTexture(
			pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

		m_DownFilter4RT.m_RenderTarget[i]->CreateTexture(
			Max(pBackBufferSurfaceDesc->Width / 4, 1u), Max(pBackBufferSurfaceDesc->Height / 4, 1u), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);

		m_DownFilter8RT.m_RenderTarget[i]->CreateTexture(
			Max(pBackBufferSurfaceDesc->Width / 8, 1u), Max(pBackBufferSurfaceDesc->Height / 8, 1u), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT);
	}

	return S_OK;
}

void Client::OnLostDevice(void)
{
	m_EventLog("Client::OnLostDevice");

	m_NormalRT->OnDestroyDevice();

	m_SpecularRT->OnDestroyDevice();

	m_PositionRT->OnDestroyDevice();

	m_LightRT->OnDestroyDevice();

	for (unsigned int i = 0; i < RenderPipeline::RTChain::RTArray::static_size; i++)
	{
		m_OpaqueRT.m_RenderTarget[i]->OnDestroyDevice();

		m_DownFilter4RT.m_RenderTarget[i]->OnDestroyDevice();

		m_DownFilter8RT.m_RenderTarget[i]->OnDestroyDevice();
	}

	m_UIRender->OnLostDevice();

	RenderPipeline::OnLostDevice();

	ResourceMgr::OnLostDevice();

	DxutApp::OnLostDevice();
}

void Client::OnDestroyDevice(void)
{
	m_EventLog("Client::OnDestroyDevice");

	ParallelTaskManager::StopParallelThread();

	ClearAllEntity();

	RemoveAllDlg();

	m_SimpleSample.reset();

	m_UIRender->OnDestroyDevice();

	ResourceMgr::OnDestroyDevice();

	LuaContext::Shutdown();

	m_Console.reset();

	_ASSERT(m_NamedObjects.empty());

	ImeEditBox::Uninitialize();

	SoundContext::Shutdown();

	PhysxScene::Shutdown();

	PhysxSdk::Shutdown();

	RenderPipeline::OnDestroyDevice();

	InputMgr::Destroy();

	DxutApp::OnDestroyDevice();
}

void Client::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	m_d3dDeviceSec.Leave();

	ResourceMgr::CheckIORequests(0);

	PhysxScene::PushRenderBuffer(this);

	SoundContext::ReleaseIdleBuffer(fElapsedTime);

	m_d3dDeviceSec.Enter();

	if (InputMgr::Capture(fTime, fElapsedTime))
	{
		OnPostCapture(fTime, fElapsedTime);
	}

	OnPreUpdate(fTime, fElapsedTime);

	m_d3dDeviceSec.Leave();

	struct Callback : public OctNode::QueryCallback
	{
		Client* m_client;

		Vector3 m_ViewedCenter;

		Client::ViewedActorSet::iterator insert_actor_iter;

		Callback(Client* client, const Vector3 & ViewedCenter)
			: m_client(client)
			, m_ViewedCenter(ViewedCenter)
			, insert_actor_iter(m_client->m_ViewedActors.begin())
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);

			if (!actor->m_Base && (actor->m_OctAabb->Center() - m_ViewedCenter).magnitudeSq() < (actor->is_linked() ? actor->m_CullingDistSq + m_client->m_ActorCullingThreshold : actor->m_CullingDistSq))
			{
				InsertViewedActor(actor);
			}
			return true;
		}

		void InsertViewedActor(Actor* actor)
		{
			if (actor->is_linked())
			{
				ViewedActorSet::iterator actor_iter = m_client->m_ViewedActors.iterator_to(*actor);
				if (actor_iter != insert_actor_iter)
				{
					m_client->m_ViewedActors.splice(insert_actor_iter, m_client->m_ViewedActors, actor_iter);
				}
				else
				{
					_ASSERT(insert_actor_iter != m_client->m_ViewedActors.end());

					insert_actor_iter++;
				}
			}
			else
			{
				_ASSERT(!actor->IsRequested());

				m_client->m_d3dDeviceSec.Enter();

				actor->RequestResource();

				m_client->OnActorRequest(actor);

				m_client->m_d3dDeviceSec.Leave();

				m_client->m_ViewedActors.insert(insert_actor_iter, *actor);
			}

			Actor::AttachList::iterator attach_iter = actor->m_Attaches.begin();
			for (; attach_iter != actor->m_Attaches.end(); attach_iter++)
			{
				InsertViewedActor(*attach_iter);
			}
		}
	};

	// ! OnActorRequestResource, UpdateLod, Update may change other actor's life time, DoAllParallelTasks also dependent it
	DelayRemover<ActorPtr>::getSingleton().Enter(boost::bind(&Client::RemoveEntity, this, boost::bind(&boost::shared_ptr<Actor>::get, boost::placeholders::_1)));

	Callback cb(this, m_ViewedCenter);
	QueryEntity(AABB(m_ViewedCenter, m_ViewedDist), &cb);

	m_d3dDeviceSec.Enter();

	ViewedActorSet::iterator actor_iter = m_ViewedActors.begin();
	for (; actor_iter != cb.insert_actor_iter; actor_iter++)
	{
		_ASSERT(OctNode::HaveNode(actor_iter->m_Node));

		actor_iter->SetLod(my::Min(actor_iter->CalculateLod((actor_iter->m_OctAabb->Center() - m_ViewedCenter).magnitude()), Actor::MaxLod - 1));

		actor_iter->Update(fElapsedTime);
	}

	for (; actor_iter != m_ViewedActors.end(); )
	{
		_ASSERT(OctNode::HaveNode(actor_iter->m_Node));

		if (!actor_iter->m_Base || !actor_iter->m_Base->IsRequested())
		{
			actor_iter = RemoveViewedActorIter(actor_iter);
		}
		else
			actor_iter++;
	}

	m_Camera->UpdateViewProj();

	m_ControllerMgr->computeInteractions(fElapsedTime, &m_ControllerFilter);

	//LuaContext::dogc(LUA_GCCOLLECT, 1);

	m_d3dDeviceSec.Leave();

	ParallelTaskManager::DoAllParallelTasks();

	m_d3dDeviceSec.Enter();

	DelayRemover<ActorPtr>::getSingleton().Leave();

	//LuaContext::dogc(LUA_GCSTOP, 0); // ! avoid RemoveIORequestCallback called inside ScriptComponent::OnPxThreadSubstep

	PhysxScene::TickPreRender(fElapsedTime);

	if (SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		RenderPipeline::OnRender(m_d3dDevice, m_BackBuffer, m_DepthStencil, &m_BackBufferSurfaceDesc, this, fTime, fElapsedTime);

		V(m_d3dDevice->SetRenderTarget(0, m_BackBuffer));
		V(m_d3dDevice->SetVertexShader(NULL));
		V(m_d3dDevice->SetPixelShader(NULL));
		V(m_d3dDevice->SetTexture(0, NULL));
		V(m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		V(m_d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
		V(m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		V(m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
		V(m_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
		V(m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE));
		V(m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
		V(m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));
		V(m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW));
		V(m_d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View));
		V(m_d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj));
		V(m_d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&my::Matrix4::identity));
		DrawHelper::FlushLine(m_d3dDevice);

		OnUIRender(m_UIRender.get(), fTime, fElapsedTime);
		V(m_d3dDevice->EndScene());
	}

	Present(NULL, NULL, NULL, NULL);

	m_d3dDeviceSec.Leave();

	PhysxScene::TickPostRender(fElapsedTime);

	//LuaContext::dogc(LUA_GCRESTART, 0);

	//if (player && player->m_Node)
	//{
	//	m_EventSystem->set3DListenerAttributes(0,
	//		(FMOD_VECTOR *)&player->m_Position,
	//		(FMOD_VECTOR *)&player->m_Velocity,
	//		(FMOD_VECTOR *)&player->m_LookMatrix[2].xyz,
	//		(FMOD_VECTOR *)&player->m_LookMatrix[1].xyz);
	//}
	//else
	//{
	//	m_EventSystem->set3DListenerAttributes(0,
	//		(FMOD_VECTOR *)&m_Camera->m_Eye,
	//		NULL,
	//		(FMOD_VECTOR *)&m_Camera->m_View[2].xyz,
	//		(FMOD_VECTOR *)&m_Camera->m_View[1].xyz);
	//}

	//FModContext::Update();

	m_d3dDeviceSec.Enter();
}

void Client::OnUIRender(
	my::UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	ui_render->m_LayerDrawCall = 0;

	m_UIRender->m_World = Matrix4::identity;

	DialogMgr::Draw(ui_render, m_fAbsoluteTime, m_fUnscaledElapsedTime, DialogMgr::GetDlgDimension());

	OnPostUIRender(ui_render, m_fAbsoluteTime, m_fUnscaledElapsedTime);

	if (m_ShowCmpHandle)
	{
		_ASSERT(m_Font);
		m_UIRender->m_World = Matrix4::identity;
		ScrInfoMap::const_iterator info_iter = m_ScrInfo.begin();
		for (int y = 5; info_iter != m_ScrInfo.end(); info_iter++, y += m_Font->m_LineHeight)
		{
			ui_render->PushString(Rectangle::LeftTop(5, (float)y, 500, 10), &info_iter->second[0], D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignLeftTop, m_Font.get());
		}
	}
	ui_render->Flush();
}

LRESULT Client::MsgProc(
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
		if (!m_Console->GetVisible())
		{
			m_Console->SetVisible(true);
		}
		else if (!m_Console->GetFocused())
		{
			m_Console->SetFocused(true);
		}
		else
		{
			m_Console->SetVisible(false);
		}

		if (m_Console->GetVisible())
		{
			m_Console->MoveToFront();
		}
		*pbNoFurtherProcessing = true;
		return 0;
	}

	*pbNoFurtherProcessing = ImeEditBox::StaticMsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
	{
		return 0;
	}

	*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
	{
		return 0;
	}

	//Player * player = Player::getSingletonPtr();
	//if (!player || !player->m_Node)
	//{
	//	LRESULT lr = m_Camera->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
	//	if (lr || *pbNoFurtherProcessing)
	//	{
	//		return lr;
	//	}
	//}
	return 0;
}

void Client::puts(const std::wstring & str)
{
	if (m_Console)
	{
		m_Console->m_Panel->puts(str);
	}
}

bool Client::ExecuteCode(const char * code) throw()
{
	if(dostring(code, "Client::ExecuteCode") && !lua_isnil(m_State, -1))
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

#define DUCOLOR_TO_D3DCOLOR(col) ((col & 0xff00ff00) | (col & 0x00ff0000) >> 16 | (col & 0x000000ff) << 16)

void Client::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct Callback : public my::OctNode::QueryCallback
		, public duDebugDraw
	{
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const my::Vector3 & ViewPos;
		const my::Vector3 & TargetPos;
		Client * client;
		DWORD m_duDebugDrawPrimitives;

		Callback(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask, const my::Vector3 & _ViewPos, const my::Vector3 & _TargetPos, Client * _client)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
			, ViewPos(_ViewPos)
			, TargetPos(_TargetPos)
			, client(_client)
			, m_duDebugDrawPrimitives(DU_DRAW_QUADS + 1)
		{
		}

		void depthMask(bool state)
		{
		}

		void texture(bool state)
		{
		}

		void begin(duDebugDrawPrimitives prim, float size)
		{
			m_duDebugDrawPrimitives = prim;
		}

		void vertex(const float* pos, unsigned int color)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				client->PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				client->PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void vertex(const float x, const float y, const float z, unsigned int color)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				client->PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				client->PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void vertex(const float* pos, unsigned int color, const float* uv)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				client->PushLineVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				client->PushTriangleVertex(pos[0], pos[1], pos[2], DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
		{
			if (m_duDebugDrawPrimitives == DU_DRAW_LINES)
			{
				client->PushLineVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
			else if (m_duDebugDrawPrimitives == DU_DRAW_TRIS)
			{
				client->PushTriangleVertex(x, y, z, DUCOLOR_TO_D3DCOLOR(color));
			}
		}

		void end()
		{
			m_duDebugDrawPrimitives = DU_DRAW_QUADS + 1;
		}

		unsigned int areaToCol(unsigned int area)
		{
			return duDebugDraw::areaToCol(area);
		}

		virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_entity));

			Actor * actor = static_cast<Actor *>(oct_entity);

			if (actor->IsRequested())
			{
				actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
			}

			if (client->m_ShowNavigation && (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)))
			{
				Actor::ComponentPtrList::const_iterator cmp_iter = actor->m_Cmps.begin();
				for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
				{
					if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeNavigation)
					{
						Navigation* navi = dynamic_cast<Navigation*>(cmp_iter->get());
						navi->DebugDraw(this, frustum, ViewPos, TargetPos);
					}
				}
			}
			return true;
		}
	};

	my::Frustum dummy_ftm(frustum);
	dummy_ftm.Near.normalizeSelf();
	dummy_ftm.Near.d = Min(dummy_ftm.Near.d, m_ViewedDist - m_ViewedCenter.dot(dummy_ftm.Near.normal));
	QueryEntity(dummy_ftm, &Callback(frustum, pipeline, PassMask, m_Camera->m_Eye, m_ViewedCenter, this));
}

void Client::AddEntity(my::OctEntity * entity, const my::AABB & aabb, float minblock, float threshold)
{
	OctNode::AddEntity(entity, aabb, minblock, threshold);
}

void Client::RemoveEntity(my::OctEntity * entity)
{
	Actor * actor = dynamic_cast<Actor *>(entity);

	if (DelayRemover<ActorPtr>::getSingleton().IsDelay(boost::bind(&Client::RemoveEntity, this, boost::bind(&boost::shared_ptr<Actor>::get, boost::placeholders::_1))))
	{
		DelayRemover<ActorPtr>::getSingleton().push_back(actor->shared_from_this());
		return;
	}

	if (actor->IsRequested())
	{
		_ASSERT(actor->is_linked());

		RemoveViewedActor(actor);
	}

	actor->StopAllActionInst();

	actor->ClearAllAttach();

	if (actor->m_Base)
	{
		actor->m_Base->Detach(actor);
	}

	_ASSERT(HaveNode(entity->m_Node));

	OctNode::RemoveEntity(entity);
}

Client::ViewedActorSet::iterator Client::RemoveViewedActorIter(ViewedActorSet::iterator actor_iter)
{
	actor_iter->ReleaseResource();

	OnActorRelease(&*actor_iter);

	return m_ViewedActors.erase(actor_iter);
}

void Client::RemoveViewedActor(Actor* actor)
{
	ViewedActorSet::iterator actor_iter = m_ViewedActors.iterator_to(*actor);

	_ASSERT(actor_iter != m_ViewedActors.end());

	RemoveViewedActorIter(actor_iter);
}

void Client::OnControlSound(boost::shared_ptr<my::Wav> wav)
{
	SoundContext::Play(wav, 0, 60.0f, false);
}

void Client::OnPostCapture(double fTime, float fElapsedTime)
{

}

void Client::OnPreUpdate(double fTime, float fElapsedTime)
{

}

void Client::OnActorRequest(Actor* actor)
{

}

void Client::OnActorRelease(Actor* actor)
{

}

void Client::OnPostUIRender(my::UIRender* ui_render, double fTime, float fElapsedTime)
{

}

boost::shared_ptr<SceneContext> Client::LoadScene(const char * path, const char * prefix)
{
	std::string key = SceneContextRequest::BuildKey(path, prefix);
	SimpleResourceCallback cb;
	IORequestPtr request(new SceneContextRequest(path, prefix, INT_MAX));
	LoadIORequestAndWait(key, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<SceneContext>(cb.m_res);
}

void Client::GetLoadSceneProgress(const char* path, const char* prefix, int& ActorListSize, int& ActorProgress, int& DialogListSize, int& DialogProgress)
{
	_ASSERT(IsMainThread());

	std::string key = SceneContextRequest::BuildKey(path, prefix);

	MutexLock lock(m_IORequestListMutex);

	IORequestPtrPairList::iterator req_iter = m_IORequestList.find(key);
	if (req_iter != m_IORequestList.end())
	{
		SceneContextRequest* request = dynamic_cast<SceneContextRequest*>(req_iter->second.get());
		if (request)
		{
			ActorListSize = request->m_ActorListSize;
			ActorProgress = request->m_ActorProgress;
			DialogListSize = request->m_DialogListSize;
			DialogProgress = request->m_DialogProgress;
			return;
		}
	}

	lock.Unlock();

	DeviceResourceBasePtrSet::iterator res_iter = m_ResourceSet.find(key);
	if (res_iter != m_ResourceSet.end())
	{
		SceneContext* scene = dynamic_cast<SceneContext*>(res_iter->second.get());
		ActorListSize = scene->m_ActorList.size();
		ActorProgress = scene->m_ActorList.size();
		DialogListSize = scene->m_DialogList.size();
		DialogProgress = scene->m_DialogList.size();
		return;
	}

	ActorListSize = INT_MAX;
	ActorProgress = 0;
	DialogListSize = INT_MAX;
	DialogProgress = 0;
}

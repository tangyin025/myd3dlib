#include "stdafx.h"
#include "Client.h"
#include "Material.h"
#include "resource.h"
#include "NavigationSerialization.h"
#include "Controller.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "LargeImage.h"
#include <sstream>
#include <fstream>
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/tag_function.hpp>
#include "LuaExtension.inl"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/assign/list_of.hpp>
#include "DebugDraw.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_ViewProj;

	UINT m_Passes;

public:
	EffectUIRender(void)
		: m_Passes(0)
		, handle_World(NULL)
		, handle_ViewProj(NULL)
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

		BOOST_VERIFY(handle_World = m_UIEffect->GetParameterByName(NULL, "g_World"));
		BOOST_VERIFY(handle_ViewProj = m_UIEffect->GetParameterByName(NULL, "g_ViewProj"));

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
		_ASSERT(m_UIEffect->m_ptr);

		m_LayerDrawCall = 0;

		m_Passes = m_UIEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
	}

	void End(void)
	{
		_ASSERT(m_UIEffect->m_ptr);

		m_UIEffect->End();

		m_Passes = 0;
	}

	void SetWorld(const Matrix4 & World)
	{
		_ASSERT(m_UIEffect->m_ptr);

		m_UIEffect->SetMatrix(handle_World, World);
	}

	void SetViewProj(const Matrix4 & ViewProj)
	{
		_ASSERT(m_UIEffect->m_ptr);

		m_UIEffect->SetMatrix(handle_ViewProj, ViewProj);
	}

	void Flush(void)
	{
		_ASSERT(m_UIEffect->m_ptr);

		_ASSERT(m_Passes > 0); // must between Begin and End

		for (UINT p = 0; p < m_Passes; p++)
		{
			m_UIEffect->BeginPass(p);
			UIRender::Flush();
			m_UIEffect->EndPass();
		}
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

static void PlayerData_moveitems(PlayerData* self, int src, int count, int dst)
{
	_ASSERT(src + count <= _countof(PlayerData::items));
	_ASSERT(dst + count <= _countof(PlayerData::items));
	std::copy(&self->items[src], &self->items[src + count], &self->items[dst]);
	std::copy(&self->itemstatus[src], &self->itemstatus[src + count], &self->itemstatus[dst]);
}

static void client_query_entity(Client* self, const my::AABB& aabb, const luabind::object& callback)
{
	struct Callback : public OctNode::QueryCallback
	{
		const luabind::object& callback;

		Callback(const luabind::object& _callback)
			: callback(_callback)
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);
			luabind::call_function<void>(callback, actor);
			return true;
		}
	};

	Callback cb(callback);
	self->QueryEntity(aabb, &cb);
}

static void client_add_state_adopt(Client* self, StateBase* state)
{
	self->AddState(StateBasePtr(state));
}

static void client_add_state_adopt(Client* self, StateBase* state, StateBase* parent)
{
	self->AddState(StateBasePtr(state), parent);
}

static bool client_file_exists(Client* self, const char* u8path)
{
	return PathFileExists(u8tots(u8path).c_str());
}

PlayerData::PlayerData(void)
	: savetime(0)
	, sceneid(0)
	, pos(0, 0, 0)
	, angle(D3DXToRadian(0))
{
	std::fill_n(attrs, _countof(attrs), 0);
	std::fill_n(quests, _countof(quests), 0);
	std::fill_n(queststatus, _countof(queststatus), 0);
	std::fill_n(auras, _countof(auras), 0);
	std::fill_n(aurastatus, _countof(aurastatus), 0);
	std::fill_n(items, _countof(items), 0);
	std::fill_n(itemstatus, _countof(itemstatus), 0);
}

PlayerData::~PlayerData(void)
{
}

//BOOST_CLASS_VERSION(PlayerData, 1)

namespace boost {
	namespace serialization {
		template<> struct version<PlayerData>
		{
			static const int value = 1000;
		};
	}
}

template<class Archive>
void PlayerData::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(savetime);
	ar << BOOST_SERIALIZATION_NVP(sceneid);
	ar << BOOST_SERIALIZATION_NVP(pos);
	ar << BOOST_SERIALIZATION_NVP(angle);
	ar << BOOST_SERIALIZATION_NVP(attrs);
	ar << BOOST_SERIALIZATION_NVP(quests);
	ar << BOOST_SERIALIZATION_NVP(queststatus);
	ar << BOOST_SERIALIZATION_NVP(auras);
	ar << BOOST_SERIALIZATION_NVP(aurastatus);
	ar << BOOST_SERIALIZATION_NVP(items);
	ar << BOOST_SERIALIZATION_NVP(itemstatus);
}

template<class Archive>
void PlayerData::load(Archive& ar, const unsigned int version)
{
	if (version < boost::serialization::version<PlayerData>::value)
	{
		return;
	}

	_ASSERT(boost::serialization::version<PlayerData>::value == version);

	ar >> BOOST_SERIALIZATION_NVP(savetime);
	ar >> BOOST_SERIALIZATION_NVP(sceneid);
	ar >> BOOST_SERIALIZATION_NVP(pos);
	ar >> BOOST_SERIALIZATION_NVP(angle);
	ar >> BOOST_SERIALIZATION_NVP(attrs);
	ar >> BOOST_SERIALIZATION_NVP(quests);
	ar >> BOOST_SERIALIZATION_NVP(queststatus);
	ar >> BOOST_SERIALIZATION_NVP(auras);
	ar >> BOOST_SERIALIZATION_NVP(aurastatus);
	ar >> BOOST_SERIALIZATION_NVP(items);
	ar >> BOOST_SERIALIZATION_NVP(itemstatus);
}

PlayerDataRequest::PlayerDataRequest(const PlayerData* data, const char* path, int Priority)
	: IORequest(Priority)
	, m_path(path)
{
	m_res.reset(new PlayerData(*data));
}

void PlayerDataRequest::LoadResource(void)
{
	boost::shared_ptr<PlayerData> ret = boost::dynamic_pointer_cast<PlayerData>(m_res);
	_ASSERT(ret);
	ret->savetime = time(NULL);

	std::ofstream ofs(m_path, std::ios::out, _SH_DENYRW);
	boost::archive::text_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("PlayerData", ret);
}

void PlayerDataRequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
{
}

struct ScriptStateBase : StateBase, luabind::wrap_base
{
	ScriptStateBase(void)
	{
	}

	virtual ~ScriptStateBase(void)
	{
	}

	virtual void OnAdd(void)
	{
		try
		{
			luabind::wrap_base::call<void>("OnAdd");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnAdd(StateBase * ptr)
	{
		ptr->StateBase::OnAdd();
	}

	virtual void OnEnter(void)
	{
		try
		{
			luabind::wrap_base::call<void>("OnEnter");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnEnter(StateBase * ptr)
	{
		ptr->StateBase::OnEnter();
	}

	virtual void OnExit(void)
	{
		try
		{
			luabind::wrap_base::call<void>("OnExit");
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnExit(StateBase * ptr)
	{
		ptr->StateBase::OnExit();
	}

	virtual void OnUpdate(float fElapsedTime)
	{
		try
		{
			luabind::wrap_base::call<void>("OnUpdate", fElapsedTime);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnUpdate(StateBase * ptr, float fElapsedTime)
	{
		ptr->StateBase::OnUpdate(fElapsedTime);
	}

	virtual void OnActorRequestResource(Actor * actor)
	{
		try
		{
			luabind::wrap_base::call<void>("OnActorRequestResource", actor);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnActorRequestResource(StateBase * ptr, Actor * actor)
	{
		ptr->StateBase::OnActorRequestResource(actor);
	}

	virtual void OnActorReleaseResource(Actor * actor)
	{
		try
		{
			luabind::wrap_base::call<void>("OnActorReleaseResource", actor);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnActorReleaseResource(StateBase * ptr, Actor * actor)
	{
		ptr->StateBase::OnActorReleaseResource(actor);
	}

	virtual void OnGUI(my::UIRender * ui_render, float fElapsedTime, const my::Vector2 & Viewport)
	{
		my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		try
		{
			luabind::wrap_base::call<void>("OnGUI", ui_render, fElapsedTime, Viewport);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnGUI(StateBase * ptr, my::UIRender * ui_render, float fElapsedTime, const my::Vector2 & Viewport)
	{
		ptr->StateBase::OnGUI(ui_render, fElapsedTime, Viewport);
	}

	virtual void OnControlFocus(my::Control* control)
	{
		try
		{
			luabind::wrap_base::call<void>("OnControlFocus", control);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
	}

	static void default_OnControlFocus(StateBase* ptr, my::Control* control)
	{
		ptr->StateBase::OnControlFocus(control);
	}

	virtual bool OnControllerFilter(Controller * a, Controller * b)
	{
		//my::CriticalSectionLock lock(LuaContext::getSingleton().m_StateSec);

		try
		{
			return luabind::wrap_base::call<bool>("OnControllerFilter", a, b);
		}
		catch (const luabind::error& e)
		{
			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		}
		return false;
	}

	static bool default_OnControllerFilter(StateBase * ptr, Controller * a, Controller * b)
	{
		return ptr->StateBase::OnControllerFilter(a, b);
	}
};

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

		void validate(boost::any& v,
			const std::vector<std::string>& values,
			my::InputMgr::KeyPairList*, int)
		{
			//                      2                3                        4           5             6              7                      8             9                     10               11
			static boost::regex r("((KeyboardButton)|(KeyboardNegativeButton)|(MouseMove)|(MouseButton)|(JoystickAxis)|(JoystickNegativeAxis)|(JoystickPov)|(JoystickNegativePov)|(JoystickButton))(\\d+)");

			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			std::string::const_iterator s_iter = s.begin();
			boost::smatch match;
			my::InputMgr::KeyPairList res;
			for (; boost::regex_search(s_iter, s.end(), match, r, boost::match_default); s_iter = match[0].second)
			{
				_ASSERT(match[11].matched);
				if (match[2].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::KeyboardButton, boost::lexical_cast<int>(match[11])));
				}
				else if (match[3].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::KeyboardNegativeButton, boost::lexical_cast<int>(match[11])));
				}
				else if (match[4].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::MouseMove, boost::lexical_cast<int>(match[11])));
				}
				else if (match[5].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::MouseButton, boost::lexical_cast<int>(match[11])));
				}
				else if (match[6].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::JoystickAxis, boost::lexical_cast<int>(match[11])));
				}
				else if (match[7].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::JoystickNegativeAxis, boost::lexical_cast<int>(match[11])));
				}
				else if (match[8].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::JoystickPov, boost::lexical_cast<int>(match[11])));
				}
				else if (match[9].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::JoystickNegativePov, boost::lexical_cast<int>(match[11])));
				}
				else if (match[10].matched)
				{
					res.push_back(std::make_pair(my::InputMgr::JoystickButton, boost::lexical_cast<int>(match[11])));
				}
			}
			if (s_iter != s.end())
			{
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
			v = boost::any(res);
		}
	}
}

Client::Client(void)
	: OctRoot(-4096, 4096)
	, InputMgr(KeyCount)
	, m_UIRender(new EffectUIRender())
	, m_ViewedCenter(0, 0, 0)
	, m_Activated(false)
	, m_ShowCursorCount(0)
{
	char buff[MAX_PATH];
	GetModuleFileNameA(NULL, buff, _countof(buff));
	std::string cfg_file(PathFindFileNameA(buff), PathFindExtensionA(buff));

	boost::program_options::options_description desc("Options");
	std::vector<std::string> path_list;
	desc.add_options()
		("path", boost::program_options::value(&path_list)->default_value(boost::assign::list_of("Media")("..\\demo2_3\\Media"), ""), "Path")
		("width", boost::program_options::value(&m_WindowBackBufferWidthAtModeChange)->default_value(800), "Width")
		("height", boost::program_options::value(&m_WindowBackBufferHeightAtModeChange)->default_value(600), "Height")
		("windowed", boost::program_options::value(&m_WindowedModeAtFirstCreate)->default_value(true), "Windowed")
		("presentinterval", boost::program_options::value(&m_PresentIntervalAtFirstCreate)->default_value(D3DPRESENT_INTERVAL_IMMEDIATE), "Presentation Interval")
		("joystickaxisdeadzone", boost::program_options::value(&m_JoystickAxisDeadZone)->default_value(3276), "Joystick Axis Dead Zone")
		("mousemoveaxiscoef", boost::program_options::value(&m_MouseMoveAxisCoef)->default_value(5000), "Mouse Move Axis Coef")
		("physxframeinterval", boost::program_options::value(&PhysxScene::m_Timer.m_Interval)->default_value(1/60.0f), "Physx Frame Interval")
		("fov", boost::program_options::value(&m_InitFov)->default_value(60.0f), "Fov")
		("physxsceneflags", boost::program_options::value(&m_InitPhysxSceneFlags)->default_value(physx::PxSceneFlag::eENABLE_PCM | physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS | physx::PxSceneFlag::eENABLE_CCD), "Physx Scene Flags")
		("physxscenegravity", boost::program_options::value(&m_InitPhysxSceneGravity)->default_value(Vector3::Gravity, ""), "Init Physx Scene Gravity")
		("iothreadnum", boost::program_options::value(&m_InitIOThreadNum)->default_value(3), "IO Thread Number")
		("loadshadercache", boost::program_options::value(&m_InitLoadShaderCache)->default_value(true), "Load Shader Cache")
		("font", boost::program_options::value(&m_InitFont)->default_value("font/wqy-microhei.ttc"), "Font")
		("fontheight", boost::program_options::value(&m_InitFontHeight)->default_value(13), "Font Height")
		("fontfaceindex", boost::program_options::value(&m_InitFontFaceIndex)->default_value(0), "Font Face Index")
		("uieffect", boost::program_options::value(&m_InitUIEffect)->default_value("shader/UIEffect.fx"), "UI Effect")
		("script", boost::program_options::value(&m_InitScript)->default_value("dofile 'Main.lua'"), "Script")
		("languageid", boost::program_options::value(&m_InitLanguageId)->default_value("eng"), "Language Id")
		("keyuihorizontal", boost::program_options::value(&m_InitBindKeys[KeyUIHorizontal])->default_value(InputMgr::KeyPairList(), ""), "Key UI Horizontal")
		("keyuivertical", boost::program_options::value(&m_InitBindKeys[KeyUIVertical])->default_value(InputMgr::KeyPairList(), ""), "Key UI Vertical")
		("keyuiconfirm", boost::program_options::value(&m_InitBindKeys[KeyUIConfirm])->default_value(InputMgr::KeyPairList(), ""), "Key UI Confirm")
		("keyuicancel", boost::program_options::value(&m_InitBindKeys[KeyUICancel])->default_value(InputMgr::KeyPairList(), ""), "Key UI Cancel")
		("keyhorizontal", boost::program_options::value(&m_InitBindKeys[KeyHorizontal])->default_value(InputMgr::KeyPairList(), ""), "Key Horizontal")
		("keyvertical", boost::program_options::value(&m_InitBindKeys[KeyVertical])->default_value(InputMgr::KeyPairList(), ""), "Key Vertical")
		("keymousex", boost::program_options::value(&m_InitBindKeys[KeyMouseX])->default_value(InputMgr::KeyPairList(), ""), "Key Mouse X")
		("keymousey", boost::program_options::value(&m_InitBindKeys[KeyMouseY])->default_value(InputMgr::KeyPairList(), ""), "Key Mouse Y")
		("keyjump", boost::program_options::value(&m_InitBindKeys[KeyJump])->default_value(InputMgr::KeyPairList(), ""), "Key Jump")
		("keyfire", boost::program_options::value(&m_InitBindKeys[KeyFire])->default_value(InputMgr::KeyPairList(), ""), "Key Fire")
		("keycheck", boost::program_options::value(&m_InitBindKeys[KeyCheck])->default_value(InputMgr::KeyPairList(), ""), "Key Check")
		("keylock", boost::program_options::value(&m_InitBindKeys[KeyLock])->default_value(InputMgr::KeyPairList(), ""), "Key Lock")
		("keymenu", boost::program_options::value(&m_InitBindKeys[KeyMenu])->default_value(InputMgr::KeyPairList(), ""), "Key Menu")
		("keyweapon1", boost::program_options::value(&m_InitBindKeys[KeyWeapon1])->default_value(InputMgr::KeyPairList(), ""), "Key Weapon1")
		("keyweapon2", boost::program_options::value(&m_InitBindKeys[KeyWeapon2])->default_value(InputMgr::KeyPairList(), ""), "Key Weapon2")
		("keyweapon3", boost::program_options::value(&m_InitBindKeys[KeyWeapon3])->default_value(InputMgr::KeyPairList(), ""), "Key Weapon3")
		("keyweapon4", boost::program_options::value(&m_InitBindKeys[KeyWeapon4])->default_value(InputMgr::KeyPairList(), ""), "Key Weapon4")
		("vieweddist", boost::program_options::value(&m_ViewedDist)->default_value(1000.0f), "Viewed Distance")
		("actorcullingthreshold", boost::program_options::value(&m_ActorCullingThreshold)->default_value(10.0f), "Actor Culling Threshold")
		("shownavigation", boost::program_options::value(&m_ShowNavigation)->default_value(false), "Show Navigation")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_config_file<char>((cfg_file + ".cfg").c_str(), desc, true), vm);
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

	for (int Key = KeyUIHorizontal; Key < KeyCount; Key++)
	{
		InputMgr::KeyPairList::const_iterator value_iter = m_InitBindKeys[Key].begin();
		for (; value_iter != m_InitBindKeys[Key].end(); value_iter++)
		{
			InputMgr::BindKey(Key, value_iter->first, value_iter->second);
		}
	}

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
		m_DownFilterRT.m_RenderTarget[i].reset(new Texture2D());
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

	if (m_InitLoadShaderCache)
	{
		TCHAR szDir[MAX_PATH];
		GetCurrentDirectory(_countof(szDir), szDir);
		RenderPipeline::LoadShaderCache(szDir);
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

		, luabind::class_<PlayerData, my::DeviceResourceBase, boost::shared_ptr<my::DeviceResourceBase> >("PlayerData")
			.def(luabind::constructor<>())
			.enum_("ATTRIBUTE")
			[
				luabind::value("ATTR_EQUIP0", 0),
				luabind::value("ATTR_EQUIP1", 1),
				luabind::value("ATTR_EQUIP2", 2),
				luabind::value("ATTR_WEAPON0", 3),
				luabind::value("ATTR_WEAPON1", 4),
				luabind::value("ATTR_WEAPON2", 5),
				luabind::value("ATTR_WEAPON0_AMMO", 6),
				luabind::value("ATTR_WEAPON1_AMMO", 7),
				luabind::value("ATTR_WEAPON2_AMMO", 8),
				luabind::value("ATTR_HEALTH", 9),
				luabind::value("ATTR_MAX_HEALTH", 10),
				luabind::value("ATTR_ITEM_CAPACITY", 11),
				luabind::value("ATTR_COUNT", _countof(PlayerData::attrs)),
				luabind::value("QUEST_COUNT", _countof(PlayerData::quests)),
				luabind::value("AURA_COUNT", _countof(PlayerData::auras)),
				luabind::value("MAX_ITEM_NUM", _countof(PlayerData::items))
			]
			.def_readonly("savetime", &PlayerData::savetime)
			.def_readwrite("sceneid", &PlayerData::sceneid)
			.def_readwrite("pos", &PlayerData::pos)
			.def_readwrite("angle", &PlayerData::angle)
			.def("setattr", &PlayerData::setattr)
			.def("getattr", &PlayerData::getattr)
			.def("setquest", &PlayerData::setquest)
			.def("getquest", &PlayerData::getquest)
			.def("setqueststatus", &PlayerData::setqueststatus)
			.def("getqueststatus", &PlayerData::getqueststatus)
			.def("setaura", &PlayerData::setaura)
			.def("getaura", &PlayerData::getaura)
			.def("setaurastatus", &PlayerData::setaurastatus)
			.def("getaurastatus", &PlayerData::getaurastatus)
			.def("setitem", &PlayerData::setitem)
			.def("getitem", &PlayerData::getitem)
			.def("setitemstatus", &PlayerData::setitemstatus)
			.def("getitemstatus", &PlayerData::getitemstatus)
			.def("moveitems", &PlayerData_moveitems)

		, luabind::class_<StateBase, ScriptStateBase/*, boost::shared_ptr<StateBase>*/ >("StateBase")
			.def(luabind::constructor<>())
			.property("IsActivated", &StateBase::IsActivated)
			.def("OnAdd", &StateBase::OnAdd, &ScriptStateBase::default_OnAdd)
			.def("OnEnter", &StateBase::OnEnter, &ScriptStateBase::default_OnEnter)
			.def("OnExit", &StateBase::OnExit, &ScriptStateBase::default_OnExit)
			.def("OnUpdate", &StateBase::OnUpdate, &ScriptStateBase::default_OnUpdate)
			.def("OnControlFocus", &StateBase::OnControlFocus, &ScriptStateBase::default_OnControlFocus)
			.def("OnActorRequestResource", &StateBase::OnActorRequestResource, &ScriptStateBase::default_OnActorRequestResource)
			.def("OnActorReleaseResource", &StateBase::OnActorReleaseResource, &ScriptStateBase::default_OnActorReleaseResource)
			.def("OnGUI", &StateBase::OnGUI, &ScriptStateBase::default_OnGUI)
			.def("OnControllerFilter", &StateBase::OnControllerFilter, &ScriptStateBase::default_OnControllerFilter)

		, luabind::class_<Client, luabind::bases<my::DxutApp, my::ResourceMgr, RenderPipeline, PhysxScene> >("Client")
			.def_readonly("wnd", &Client::m_wnd)
			.def_readwrite("Camera", &Client::m_Camera)
			.def_readwrite("WireFrame", &Client::m_WireFrame)
			.def_readwrite("DofEnable", &Client::m_DofEnable)
			.def_readwrite("BloomEnable", &Client::m_BloomEnable)
			.def_readwrite("FxaaEnable", &Client::m_FxaaEnable)
			.def_readwrite("SsaoEnable", &Client::m_SsaoEnable)
			.def_readwrite("FogEnable", &Client::m_FogEnable)
			.def_readwrite("RTType", &Client::m_RTType)
			.def_readonly("listener", &Client::m_listener)
			.def_readonly("Font", &Client::m_Font)
			.def_readonly("Console", &Client::m_Console)
			.def_readonly("LanguageId", &Client::m_InitLanguageId)
			.def_readwrite("ViewedCenter", &Client::m_ViewedCenter)
			.def_readwrite("ViewedDist", &Client::m_ViewedDist)
			.def_readwrite("ActorCullingThreshold", &Client::m_ActorCullingThreshold)
			.def_readwrite("ShowNavigation", &Client::m_ShowNavigation)
			.enum_("Key")
			[
				luabind::value("KeyUIHorizontal", Client::KeyUIHorizontal),
				luabind::value("KeyUIVertical", Client::KeyUIVertical),
				luabind::value("KeyUIConfirm", Client::KeyUIConfirm),
				luabind::value("KeyUICancel", Client::KeyUICancel),
				luabind::value("KeyHorizontal", Client::KeyHorizontal),
				luabind::value("KeyVertical", Client::KeyVertical),
				luabind::value("KeyMouseX", Client::KeyMouseX),
				luabind::value("KeyMouseY", Client::KeyMouseY),
				luabind::value("KeyJump", Client::KeyJump),
				luabind::value("KeyFire", Client::KeyFire),
				luabind::value("KeyCheck", Client::KeyCheck),
				luabind::value("KeyLock", Client::KeyLock),
				luabind::value("KeyMenu", Client::KeyMenu),
				luabind::value("KeyWeapon1", Client::KeyWeapon1),
				luabind::value("KeyWeapon2", Client::KeyWeapon2),
				luabind::value("KeyWeapon3", Client::KeyWeapon3),
				luabind::value("KeyWeapon4", Client::KeyWeapon4),
				luabind::value("KeyCount", Client::KeyCount)
			]
			.def_readonly("keyboard", &Client::m_keyboard)
			.def_readonly("mouse", &Client::m_mouse)
			.def_readonly("joystick", &Client::m_joystick)
			.def("BindKey", &Client::BindKey)
			.def("UnbindKey", &Client::UnbindKey)
			.def("GetKeyAxisRaw", &Client::GetKeyAxisRaw)
			.def("IsKeyDown", &Client::IsKeyDown)
			.def("IsKeyPressRaw", &Client::IsKeyPressRaw)
			.def("IsKeyPress", &Client::IsKeyPress)
			.def("IsKeyRelease", &Client::IsKeyRelease)
			.property("DlgViewport", &Client::GetDlgViewport, &Client::SetDlgViewport)
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
			.def("QueryEntity", &client_query_entity)
			.def("AddStateAdopt", (void(*)(Client*, StateBase*)) & client_add_state_adopt, luabind::adopt(boost::placeholders::_2))
			.def("AddStateAdopt", (void(*)(Client*, StateBase*, StateBase*)) & client_add_state_adopt, luabind::adopt(boost::placeholders::_2)) // ! luabind::class_::def does not support default arguments (Release build.)
			.def("FileExists", &client_file_exists)
			.def("AddTransition", &Client::AddTransition)
			.def("ProcessEvent", &Client::ProcessEvent)
			.def("ClearAllState", &Client::ClearAllState)
			//.def("OnControlSound", &Client::OnControlSound)
			.def("GetTranslation", &Client::OnControlTranslate)
			.def("SetTranslation", &Client::SetTranslation)
			.def("Play", (SoundEventPtr(SoundContext::*)(my::WavPtr, bool)) & Client::Play)
			.def("Play", (SoundEventPtr(SoundContext::*)(my::WavPtr, bool, const my::Vector3&, const my::Vector3&, float, float)) & Client::Play)
			.def("LoadSceneAsync", &Client::LoadSceneAsync<luabind::object>)
			.def("LoadScene", &Client::LoadScene)
			.def("GetLoadSceneProgress", &Client::GetLoadSceneProgress, luabind::pure_out_value(boost::placeholders::_4) + luabind::pure_out_value(boost::placeholders::_5) + luabind::pure_out_value(boost::placeholders::_6) + luabind::pure_out_value(boost::placeholders::_7))
			.def("LoadPlayerData", &Client::LoadPlayerData)
			.def("SavePlayerDataAsync", &Client::SavePlayerDataAsync<luabind::object>)
			.def("SavePlayerData", &Client::SavePlayerData)

		//, luabind::def("res2scene", (boost::shared_ptr<SceneContext>(*)(const boost::shared_ptr<my::DeviceResourceBase>&)) & boost::dynamic_pointer_cast<SceneContext, my::DeviceResourceBase>)
	];
	luabind::globals(m_State)["client"] = this;

	m_Console->SetVisible(!ExecuteCode(m_InitScript.c_str()));

	DialogMgr::InsertDlg(m_Console.get());

	if (m_Console->GetVisible())
	{
		m_Console->SetFocused(true);
	}

	m_EventLog("Client::OnCreateDevice");

	return S_OK;
}

HRESULT Client::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_EventLog("Client::OnResetDevice");

	boost::dynamic_pointer_cast<my::PerspectiveCamera>(m_Camera)->m_Aspect = (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height;

	DialogMgr::SetDlgViewport(Vector2(600 * boost::dynamic_pointer_cast<my::PerspectiveCamera>(m_Camera)->m_Aspect, 600), D3DXToRadian(75.0f));

	FontLibrary::m_Scale = Vector2(pBackBufferSurfaceDesc->Height / DialogMgr::GetDlgViewport().y);

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
		pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_SpecularRT->CreateTexture(
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

		m_DownFilterRT.m_RenderTarget[i]->OnDestroyDevice();
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

	ClearAllState();

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

	::ClipCursor(NULL);

	DxutApp::OnDestroyDevice();
}

void Client::OnFrameTick(
	double fTime,
	float fElapsedTime)
{
	LeaveDeviceSection();

	ResourceMgr::CheckIORequests(0);

	PhysxScene::PushRenderBuffer(this);

	SoundContext::ReleaseIdleBuffer(fElapsedTime);

	EnterDeviceSection();

	if (InputMgr::Capture(fTime, fElapsedTime))
	{
		bool bNoFurtherProcessing = false;
		if (IsKeyPress(KeyUIHorizontal))
		{
			if (GetKeyAxisRaw(KeyUIHorizontal) < 32767)
			{
				bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYDOWN, VK_LEFT, 0);
			}
			else
			{
				bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYDOWN, VK_RIGHT, 0);
			}
		}

		if (IsKeyPress(KeyUIVertical))
		{
			if (GetKeyAxisRaw(KeyUIVertical) < 32767)
			{
				bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYDOWN, VK_UP, 0);
			}
			else
			{
				bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYDOWN, VK_DOWN, 0);
			}
		}

		if (IsKeyPress(KeyUIConfirm))
		{
			bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYDOWN, VK_RETURN, 0);
		}
		else if (IsKeyRelease(KeyUIConfirm))
		{
			bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYUP, VK_RETURN, 0);
		}
		
		if (IsKeyPress(KeyUICancel))
		{
			bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
		}
		else if (IsKeyRelease(KeyUICancel))
		{
			bNoFurtherProcessing = DialogMgr::MsgProc(m_wnd->m_hWnd, WM_KEYUP, VK_ESCAPE, 0);
		}

		if (bNoFurtherProcessing)
		{
			InputMgr::Capture(fTime, fElapsedTime);
		}
	}

	StateBase * curr_iter = m_Current;
	for (; curr_iter != NULL; curr_iter = curr_iter->m_Current)
	{
		curr_iter->OnUpdate(fElapsedTime);
	}

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

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);

			if (!actor->m_Base && (actor->m_OctAabb->Center() - m_ViewedCenter).magnitudeSq() < actor->m_CullingDistSq)
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

				actor->RequestResource();

				StateBase* curr_iter = m_client->m_Current;
				for (; curr_iter != NULL; curr_iter = curr_iter->m_Current)
				{
					curr_iter->OnActorRequestResource(actor);
				}

				m_client->m_ViewedActors.insert(insert_actor_iter, *actor);
			}

			Actor::ActorList::iterator attach_iter = actor->m_Attaches.begin();
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

	ViewedActorSet::iterator actor_iter = m_ViewedActors.begin();
	for (; actor_iter != cb.insert_actor_iter; actor_iter++)
	{
		_ASSERT(OctNode::HaveNode(actor_iter->m_Node));

		actor_iter->UpdateLod(m_Camera->m_Eye, m_ViewedCenter);

		actor_iter->Update(fElapsedTime);
	}

	for (; actor_iter != m_ViewedActors.end(); )
	{
		_ASSERT(OctNode::HaveNode(actor_iter->m_Node));

		if (actor_iter->m_Base ? !actor_iter->m_Base->IsRequested() : (actor_iter->m_OctAabb->Center() - m_ViewedCenter).magnitudeSq() > actor_iter->m_CullingDistSq + m_ActorCullingThreshold)
		{
			actor_iter->ReleaseResource();

			StateBase* curr_iter = m_Current;
			for (; curr_iter != NULL; curr_iter = curr_iter->m_Current)
			{
				curr_iter->OnActorReleaseResource(&*actor_iter);
			}

			actor_iter = m_ViewedActors.erase(actor_iter);
		}
		else
			actor_iter++;
	}

	m_SkyLightCam->UpdateViewProj();

	m_Camera->UpdateViewProj();

	m_ControllerMgr->computeInteractions(fElapsedTime, &m_ControllerFilter);

	LuaContext::dogc(LUA_GCCOLLECT, 1);

	ParallelTaskManager::DoAllParallelTasks();

	DelayRemover<ActorPtr>::getSingleton().Leave();

	//LuaContext::dogc(LUA_GCSTOP, 0); // ! avoid RemoveIORequestCallback called inside ScriptComponent::OnPxThreadSubstep

	PhysxScene::TickPreRender(fElapsedTime);

	if (SUCCEEDED(hr = m_d3dDevice->BeginScene()))
	{
		RenderPipeline::OnRender(m_d3dDevice, m_BackBuffer, m_DepthStencil, &m_BackBufferSurfaceDesc, this, fTime, fElapsedTime);

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

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
		OnUIRender(m_UIRender.get(), fTime, fElapsedTime);
		m_UIRender->End();
		V(m_d3dDevice->EndScene());
	}

	Present(NULL, NULL, NULL, NULL);

	LeaveDeviceSection();

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

	EnterDeviceSection();
}

void Client::OnUIRender(
	my::UIRender * ui_render,
	double fTime,
	float fElapsedTime)
{
	DialogMgr::Draw(ui_render, m_fAbsoluteTime, m_fAbsoluteElapsedTime, DialogMgr::GetDlgViewport());
	StateBase* curr_iter = m_Current;
	for (; curr_iter != NULL; curr_iter = curr_iter->m_Current)
	{
		curr_iter->OnGUI(ui_render, m_fAbsoluteElapsedTime, DialogMgr::GetDlgViewport());
	}
	_ASSERT(m_Font);
	ui_render->SetWorld(Matrix4::identity);
	ScrInfoMap::const_iterator info_iter = m_ScrInfo.begin();
	for (int y = 5; info_iter != m_ScrInfo.end(); info_iter++, y += m_Font->m_LineHeight)
	{
		ui_render->PushString(Rectangle::LeftTop(5, (float)y, 500, 10), &info_iter->second[0], D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignLeftTop, m_Font.get());
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

	if (uMsg == WM_ACTIVATE)
	{
		if (LOWORD(wParam) == WA_ACTIVE
			|| LOWORD(wParam) == WA_CLICKACTIVE)
		{
			m_Activated = true;

			if (m_DeviceObjectsCreated)
			{
				OnControlFocus(Control::s_FocusControl);
			}
		}
		else
		{
			m_Activated = false;
		}

		m_ActivateEvent(m_Activated);
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

		virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
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
		actor->ReleaseResource();

		StateBase* curr_iter = m_Current;
		for (; curr_iter != NULL; curr_iter = curr_iter->m_Current)
		{
			curr_iter->OnActorReleaseResource(actor);
		}
	}

	actor->StopAllAction();

	actor->ClearAllAttach();

	if (actor->m_Base)
	{
		actor->m_Base->Detach(actor);
	}

	if (actor->is_linked())
	{
		ViewedActorSet::iterator remove_actor_iter = m_ViewedActors.iterator_to(*actor);
		_ASSERT(remove_actor_iter != m_ViewedActors.end());
		m_ViewedActors.erase(remove_actor_iter);
	}

	_ASSERT(HaveNode(entity->m_Node));

	OctNode::RemoveEntity(entity);
}

void Client::OnControlSound(boost::shared_ptr<my::Wav> wav)
{
	SoundContext::Play(wav, false);
}

void Client::OnControlFocus(my::Control * control)
{
	StateBase* curr_iter = m_Current;
	for (; curr_iter != NULL; curr_iter = curr_iter->m_Current)
	{
		curr_iter->OnControlFocus(control);
	}
}

std::wstring Client::OnControlTranslate(const std::string& u8str)
{
	TranslationMap::iterator trans_iter = m_TranslationMap.find(u8str);
	if (trans_iter != m_TranslationMap.end())
	{
		return trans_iter->second;
	}
	return D3DContext::OnControlTranslate(u8str);
}

bool Client::OnControllerFilter(const physx::PxController& a, const physx::PxController& b)
{
	StateBase* active_leaf = GetActiveLeaf();
	if (active_leaf && a.getUserData() && b.getUserData())
	{
		Controller* controller0 = static_cast<Controller*>((Component*)a.getUserData());
		Controller* controller1 = static_cast<Controller*>((Component*)b.getUserData());
		return active_leaf->OnControllerFilter(controller0, controller1);
	}
	return PhysxScene::OnControllerFilter(a, b);
}

void Client::SetTranslation(const std::string& key, const std::wstring& text)
{
	m_TranslationMap[key] = text;
}

boost::shared_ptr<SceneContext> Client::LoadScene(const char * path, const char * prefix)
{
	std::string key = SceneContextRequest::BuildKey(path, prefix);
	SimpleResourceCallback cb;
	IORequestPtr request(new SceneContextRequest(path, prefix, INT_MAX));
	LoadIORequestAndWait(key, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
	return boost::dynamic_pointer_cast<SceneContext>(cb.m_res);
}

void Client::GetLoadSceneProgress(const char * path, const char * prefix, int & ActorListSize, int & ActorProgress, int & DialogListSize, int & DialogProgress)
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

boost::shared_ptr<PlayerData> Client::LoadPlayerData(const char * path)
{
	boost::shared_ptr<PlayerData> ret;
	my::IStreamBuff buff(my::FileIStream::Open(u8tots(path).c_str()));
	std::istream ifs(&buff);
	boost::archive::text_iarchive ia(ifs);
	ia >> boost::serialization::make_nvp("PlayerData", ret);
	return ret;
}

void Client::SavePlayerData(const PlayerData * data, const char * path)
{
	SimpleResourceCallback cb;
	IORequestPtr request(new PlayerDataRequest(data, path, INT_MAX));
	LoadIORequestAndWait(path, request, boost::bind(&SimpleResourceCallback::OnResourceReady, &cb, boost::placeholders::_1));
}

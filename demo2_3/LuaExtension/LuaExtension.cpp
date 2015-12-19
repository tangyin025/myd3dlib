#include "stdafx.h"
#include "LuaExtension.h"
#include "../Game.h"

namespace luabind
{
	int default_converter<std::wstring>::compute_score(lua_State* L, int index)
	{
		return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
	}

	std::wstring default_converter<std::wstring>::from(lua_State* L, int index)
	{
		return u8tows(lua_tostring(L, index));
	}

	void default_converter<std::wstring>::to(lua_State* L, std::wstring const& value)
	{
		std::string str = wstou8(value);
		lua_pushlstring(L, str.data(), str.size());
	}

	int default_converter<my::ControlEvent>::compute_score(lua_State * L, int index)
	{
		return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
	}

	my::ControlEvent default_converter<my::ControlEvent>::from(lua_State * L, int index)
	{
		struct InternalExceptionHandler
		{
			luabind::object obj;
			InternalExceptionHandler(const luabind::object & _obj)
				: obj(_obj)
			{
			}
			void operator()(my::EventArgs * args)
			{
				try
				{
					obj(args);
				}
				catch(const luabind::error & e)
				{
					// ! ControlEvent事件处理是容错的，当事件处理失败后，程序继续运行
					Game::getSingleton().AddLine(ms2ws(lua_tostring(e.state(), -1)));
				}
			}
		};
		return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
	}

	void default_converter<my::ControlEvent>::to(lua_State * L, my::ControlEvent const & e)
	{
		_ASSERT(false);
	}

	int default_converter<my::TimerEvent>::compute_score(lua_State * L, int index)
	{
		return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
	}

	my::TimerEvent default_converter<my::TimerEvent>::from(lua_State * L, int index)
	{
		struct InternalExceptionHandler
		{
			luabind::object obj;
			InternalExceptionHandler(const luabind::object & _obj)
				: obj(_obj)
			{
			}
			void operator()(float interval)
			{
				try
				{
					obj(interval);
				}
				catch(const luabind::error & e)
				{
					// ! TimerEvent事件处理是容错的，当事件处理失败后，程序继续运行
					Game::getSingleton().AddLine(ms2ws(lua_tostring(e.state(), -1)));
				}
			}
		};
		return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
	}

	void default_converter<my::TimerEvent>::to(lua_State * L, my::TimerEvent const & e)
	{
		_ASSERT(false);
	}
}

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

static int add_file_and_line(lua_State * L)
{
   lua_Debug d;
   lua_getstack(L, 1, &d);
   lua_getinfo(L, "Sln", &d);
   std::string err = lua_tostring(L, -1);
   lua_pop(L, 1);
   std::stringstream msg;
   msg << d.short_src << ":" << d.currentline;

   if (d.name != 0)
   {
      msg << "(" << d.namewhat << " " << d.name << ")";
   }
   msg << " " << err;
   lua_pushstring(L, msg.str().c_str());
   return 1;
}

static void translate_my_exception(lua_State* L, my::Exception const & e)
{
	std::string s = e.what();
	lua_pushlstring(L, s.c_str(), s.length());
}

using namespace luabind;

void Export2Lua(lua_State * L)
{
	lua_pushcfunction(L, lua_print);
	lua_setglobal(L, "print");

	lua_pushcfunction(L, luaB_loadfile);
	lua_setglobal(L, "loadfile");

	lua_pushcfunction(L, luaB_dofile);
	lua_setglobal(L, "dofile");

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaders");
	lua_pushcfunction(L, loader_Lua);
	lua_rawseti(L, -2, 2);

	lua_getglobal(L, "os");
	lua_pushcclosure(L, os_exit, 0);
	lua_setfield(L, -2, "exit");
	lua_settop(L, 0);

	open(L);

	// ! 会导致内存泄漏，但可以重写 handle_exception_aux，加入 my::Exception的支持
	register_exception_handler<my::Exception>(&translate_my_exception);

	//// ! 为什么不起作用
	//set_pcall_callback(add_file_and_line);

	ExportMath2Lua(L);

	ExportResource2Lua(L);

	ExportUI2Lua(L);

	ExportEmitter2Lua(L);

	ExportDevice2Lua(L);

	module(L)
	[

		//class_<MessagePanel, my::Control, boost::shared_ptr<my::Control> >("MessagePanel")
		//	.def_readonly("lbegin", &MessagePanel::m_lbegin)
		//	.def_readonly("lend", &MessagePanel::m_lend)
		//	.def_readonly("scrollbar", &MessagePanel::m_scrollbar)
		//	.def("AddLine", &MessagePanel::AddLine)
		//	.def("puts", &MessagePanel::puts)

		//, class_<ConsoleEditBox, my::ImeEditBox, boost::shared_ptr<my::Control> >("ConsoleEditBox")
		//	.def_readwrite("EventKeyUp", &ConsoleEditBox::EventKeyUp)
		//	.def_readwrite("EventKeyDown", &ConsoleEditBox::EventKeyDown)

		class_<Console, my::Dialog, boost::shared_ptr<Console> >("Console")
			//.def_readonly("Edit", &Console::m_Edit)
			//.def_readonly("Panel", &Console::m_Panel)

		//, class_<std::pair<std::string, my::BaseTexturePtr> >("MaterialTexturePair")
		//	.def(constructor<std::string, my::BaseTexturePtr>())
		//	.def_readwrite("first", &std::pair<std::string, my::BaseTexturePtr>::first)
		//	.def_readwrite("second", &std::pair<std::string, my::BaseTexturePtr>::second)

		, class_<Material::ParameterValue, boost::shared_ptr<Material::ParameterValue> >("ParameterValue")

		, class_<Material::ParameterValueTexture, Material::ParameterValue, boost::shared_ptr<Material::ParameterValue> >("ParameterValueTexture")
			.def(constructor<>())
			.def_readwrite("Path", &Material::ParameterValueTexture::m_Path)

		, class_<Material, boost::shared_ptr<Material> >("Material")
			.enum_("PassMask")
			[
				value("PassMaskNone", RenderPipeline::PassMaskNone),
				value("PassMaskLight", RenderPipeline::PassMaskLight),
				value("PassMaskOpaque", RenderPipeline::PassMaskOpaque),
				value("PassMaskTransparent", RenderPipeline::PassMaskTransparent)
			]
			.def(constructor<>())
			.def_readwrite("Shader", &Material::m_Shader)
			.def_readwrite("PassMask", &Material::m_PassMask)
			.def("AddParameter", &Material::AddParameter)

		, class_<Actor, boost::shared_ptr<Actor> >("Actor")
			.def(constructor<>())

		, class_<Component, boost::shared_ptr<Component> >("Component")
			.def_readwrite("World", &Component::m_World)

		, class_<RenderComponent, Component, boost::shared_ptr<RenderComponent> >("RenderComponent")

		, class_<MeshComponent, RenderComponent, boost::shared_ptr<MeshComponent> >("MeshComponent")
			.def_readwrite("Animator", &MeshComponent::m_Animator)

		, class_<IndexdPrimitiveUPComponent, RenderComponent, boost::shared_ptr<IndexdPrimitiveUPComponent> >("IndexdPrimitiveUPComponent")
			.def_readwrite("Animator", &IndexdPrimitiveUPComponent::m_Animator)

		, class_<EmitterComponent, RenderComponent, boost::shared_ptr<EmitterComponent> >("EmitterComponent")

		, class_<Animator, boost::shared_ptr<Animator> >("Animator")

		, class_<Game, bases<my::DxutApp, my::ResourceMgr> >("Game")
			.def("AddTimer", &Game::AddTimer)
			.def("InsertTimer", &Game::InsertTimer)
			.def("RemoveTimer", &Game::RemoveTimer)
			.def("RemoveAllTimer", &Game::RemoveAllTimer)
			.property("DlgViewport", &Game::GetDlgViewport, &Game::SetDlgViewport)
			.def("InsertDlg", &Game::InsertDlg)
			.def("RemoveDlg", &Game::RemoveDlg)
			.def("RemoveAllDlg", &Game::RemoveAllDlg)
			.def_readonly("Console", &Game::m_Console)
			.def_readwrite("Camera", &Game::m_Camera)
			.def_readwrite("SkyLightCam", &Game::m_SkyLightCam)
			.def_readwrite("SkyLightDiffuse", &Game::m_SkyLightDiffuse)
			.def_readwrite("SkyLightAmbient", &Game::m_SkyLightAmbient)
			.def_readwrite("WireFrame", &Game::m_WireFrame)
			.def_readwrite("DofEnable", &Game::m_DofEnable)
			.def_readwrite("DofParams", &Game::m_DofParams)
			.def("ExecuteCode", &Game::ExecuteCode)
			.def("PlaySound", &Game::PlaySound)

		, def("res2texture", &boost::dynamic_pointer_cast<my::BaseTexture, my::DeviceRelatedObjectBase>)
		, def("res2mesh", &boost::dynamic_pointer_cast<my::OgreMesh, my::DeviceRelatedObjectBase>)
		, def("res2mesh_set", &boost::dynamic_pointer_cast<my::OgreMeshSet, my::DeviceRelatedObjectBase>)
		, def("res2skeleton", &boost::dynamic_pointer_cast<my::OgreSkeletonAnimation, my::DeviceRelatedObjectBase>)
		, def("res2effect", &boost::dynamic_pointer_cast<my::Effect, my::DeviceRelatedObjectBase>)
		, def("res2font", &boost::dynamic_pointer_cast<my::Font, my::DeviceRelatedObjectBase>)
		, def("res2emitter", &boost::dynamic_pointer_cast<my::Emitter, my::DeviceRelatedObjectBase>)
		, def("res2material", &boost::dynamic_pointer_cast<Material, my::DeviceRelatedObjectBase>)
	];

	globals(L)["game"] = Game::getSingletonPtr();
}

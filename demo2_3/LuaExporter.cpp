#include "LuaExporter.h"
#include "Game.h"
#include <luabind/luabind.hpp>

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
		if (i>1) Game::getSingleton().m_console->puts(L"\t");
		Game::getSingleton().m_console->puts(ms2ws(s));
		lua_pop(L, 1);  /* pop result */
	}
	Game::getSingleton().m_console->puts(L"\n");
	return 0;
}

static int lua_exit(lua_State * L)
{
	HWND hwnd = my::DxutApp::getSingleton().GetHWND();
	_ASSERT(NULL != hwnd);
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	return 0;
}

void Export2Lua(lua_State * L)
{
	lua_pushcfunction(L, lua_print);
	lua_setglobal(L, "print");

	lua_getglobal(L, "os");
	lua_pushcclosure(L, lua_exit, 0);
	lua_setfield(L, -2, "exit");
	lua_pop(L, 1);

	luabind::open(L);

	luabind::module(L)
	[
		luabind::class_<my::ControlSkin, boost::shared_ptr<my::ControlSkin>>("ControlSkin")
			.def(luabind::constructor<>())

		, luabind::class_<my::Dialog, boost::shared_ptr<my::Dialog>>("Dialog")
			.def(luabind::constructor<>())
			.def_readwrite("m_Skin", &my::Dialog::m_Skin)

		, luabind::class_<Console, my::Dialog, boost::shared_ptr<my::Dialog>>("Console")
			.def(luabind::constructor<>())

		, luabind::class_<Game>("Game")
			.def_readonly("uiFont", &Game::m_uiFont)
			.def("InsertDlg", &Game::InsertDlg)
	];

	luabind::globals(L)["game"] = Game::getSingletonPtr();
}

#include "LuaExporter.h"
#include "Game.h"
#include <luabind/luabind.hpp>

void Export2Lua(lua_State * L)
{
	luabind::open(L);

	luabind::module(L)
	[
		luabind::class_<my::Dialog, boost::shared_ptr<my::Dialog>>("Dialog")
			.def(luabind::constructor<>())

		, luabind::class_<Console, my::Dialog, boost::shared_ptr<my::Dialog>>("Console")
			.def(luabind::constructor<>())

		, luabind::class_<Game>("Game")
			.def_readonly("uiFont", &Game::m_uiFont)
			.def("InsertDlg", &Game::InsertDlg)
	];

	luabind::globals(L)["game"] = Game::getSingletonPtr();
}

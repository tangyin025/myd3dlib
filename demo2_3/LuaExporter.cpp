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

namespace luabind
{
	template <>
	struct default_converter<std::wstring>
		: native_converter_base<std::wstring>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
		}

		std::wstring from(lua_State * L, int index)
		{
			return ms2ws(lua_tostring(L, index));
		}

		void to(lua_State * L, std::wstring const & s)
		{
			std::string t(ws2ms(s.c_str()));
			lua_pushlstring(L, t.c_str(), t.size());
		}
	};

	template <>
	struct default_converter<std::wstring const &>
		: default_converter<std::wstring>
	{
	};

	template <>
	struct default_converter<my::ControlEvent>
		: native_converter_base<my::ControlEvent>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::ControlEvent from(lua_State * L, int index)
		{
			return luabind::object(luabind::from_stack(L, index));
		}

		void to(lua_State * L, my::ControlEvent const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::ControlEvent const &>
		: default_converter<my::ControlEvent>
	{
	};
}

void Export2Lua(lua_State * L)
{
	lua_pushcfunction(L, lua_print);
	lua_setglobal(L, "print");

	lua_getglobal(L, "os");
	lua_pushcclosure(L, lua_exit, 0);
	lua_setfield(L, -2, "exit");
	lua_pop(L, 1);

	struct HelpFunc
	{
		static DWORD ARGB(int a, int r, int g, int b)
		{
			return D3DCOLOR_ARGB(a,r,g,b);
		}
	};

	luabind::open(L);

	luabind::module(L)
	[
		luabind::def("ARGB", &HelpFunc::ARGB)

		, luabind::class_<my::Vector2, boost::shared_ptr<my::Vector2> >("Vector2")
			.def(luabind::constructor<float, float>())
			.def_readwrite("x", &my::Vector2::x)
			.def_readwrite("y", &my::Vector2::y)
			
		, luabind::class_<RECT, boost::shared_ptr<RECT> >("RECT")
			.def_readwrite("left", &RECT::left)
			.def_readwrite("top", &RECT::top)
			.def_readwrite("right", &RECT::right)
			.def_readwrite("bottom", &RECT::bottom)

		, luabind::class_<CRect, RECT, boost::shared_ptr<RECT> >("CRect")
			.def(luabind::constructor<int, int, int, int>())

		, luabind::class_<my::Texture, boost::shared_ptr<my::Texture> >("Texture")

		, luabind::class_<my::Font, boost::shared_ptr<my::Font> >("Font")
			.enum_("constants")
			[
				luabind::value("AlignLeft",				my::Font::AlignLeft)
				, luabind::value("AlignCenter",			my::Font::AlignCenter)
				, luabind::value("AlignRight",			my::Font::AlignRight)
				, luabind::value("AlignTop",			my::Font::AlignTop)
				, luabind::value("AlignMiddle",			my::Font::AlignMiddle)
				, luabind::value("AlignBottom",			my::Font::AlignBottom)
				, luabind::value("AlignLeftTop",		my::Font::AlignLeftTop)
				, luabind::value("AlignCenterTop",		my::Font::AlignCenterTop)
				, luabind::value("AlignRightTop",		my::Font::AlignRightTop)
				, luabind::value("AlignLeftMiddle",		my::Font::AlignLeftMiddle)
				, luabind::value("AlignCenterMiddle",	my::Font::AlignCenterMiddle)
				, luabind::value("AlignRightMiddle",	my::Font::AlignRightMiddle)
				, luabind::value("AlignLeftBottom",		my::Font::AlignLeftBottom)
				, luabind::value("AlignCenterBottom",	my::Font::AlignCenterBottom)
				, luabind::value("AlignRightBottom",	my::Font::AlignRightBottom)
			]

		, luabind::class_<my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ControlSkin")
			.def(luabind::constructor<>())
			.def_readwrite("Texture", &my::ControlSkin::m_Texture)
			.def_readwrite("TextureRect", &my::ControlSkin::m_TextureRect)
			.def_readwrite("Font", &my::ControlSkin::m_Font)
			.def_readwrite("TextColor", &my::ControlSkin::m_TextColor)
			.def_readwrite("TextAlign", &my::ControlSkin::m_TextAlign)

		, luabind::class_<my::Control, boost::shared_ptr<my::Control> >("Control")
			.def(luabind::constructor<>())
			.property("Enabled", &my::Control::GetEnabled, &my::Control::SetEnabled)
			.property("Visible", &my::Control::GetVisible, &my::Control::SetVisible)
			.def_readwrite("Location", &my::Control::m_Location)
			.def_readwrite("Size", &my::Control::m_Size)
			.def_readwrite("Color", &my::Control::m_Color)
			.def_readwrite("Skin", &my::Control::m_Skin)

		, luabind::class_<my::Static, my::Control, boost::shared_ptr<my::Control> >("Static")
			.def(luabind::constructor<>())
			.def_readwrite("Text", &my::Static::m_Text)

		, luabind::class_<my::ButtonSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ButtonSkin")
			.def(luabind::constructor<>())
			.def_readwrite("DisabledTexRect", &my::ButtonSkin::m_DisabledTexRect)
			.def_readwrite("PressedTexRect", &my::ButtonSkin::m_PressedTexRect)
			.def_readwrite("MouseOverTexRect", &my::ButtonSkin::m_MouseOverTexRect)
			.def_readwrite("PressedOffset", &my::ButtonSkin::m_PressedOffset)

		, luabind::class_<my::ControlEvent>("ControlEvent")

		, luabind::class_<my::Button, my::Static, boost::shared_ptr<my::Control> >("Button")
			.def(luabind::constructor<>())
			.def_readwrite("EventClick", &my::Button::EventClick)

		, luabind::class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(luabind::constructor<>())
			.def("InsertControl", &my::Dialog::InsertControl)

		, luabind::class_<Console, my::Dialog, boost::shared_ptr<my::Dialog> >("Console")
			.def(luabind::constructor<>())

		, luabind::class_<Game>("Game")
			.def_readonly("uiFont", &Game::m_uiFont)
			.def("InsertDlg", &Game::InsertDlg)
	];

	luabind::globals(L)["game"] = Game::getSingletonPtr();
}

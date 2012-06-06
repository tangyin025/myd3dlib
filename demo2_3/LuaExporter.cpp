#include "LuaExporter.h"
#include "Game.h"
#include <luabind/luabind.hpp>

static int lua_print(lua_State * L)
{
	MessagePanelPtr panel = Game::getSingleton().m_panel;
	if(!panel)
		return luaL_error(L, "must have game.panel to output");

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
		if (i>1) panel->puts(L"\t");
		panel->puts(ms2ws(s));
		lua_pop(L, 1);  /* pop result */
	}
	panel->puts(L"\n");
	return 0;
}

typedef struct LoadF {
	int extraline;
	//FILE *f;
	my::ArchiveStreamPtr stream;
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

static int lua_loadfile (lua_State *L, const char *filename)
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
		lf.stream = my::ResourceMgr::getSingleton().OpenArchiveStream(filename);
	}
	catch(const my::Exception & e)
	{
		lua_pushfstring(L, e.GetDescription().c_str());
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

static int lua_dofile(lua_State * L)
{
	const char *fname = luaL_optstring(L, 1, NULL);
	int n = lua_gettop(L);
	if (lua_loadfile(L, fname) != 0) lua_error(L);
	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L) - n;
}

static int lua_exit(lua_State * L)
{
	HWND hwnd = my::DxutApp::getSingleton().GetHWND();
	_ASSERT(NULL != hwnd);
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	return 0;
}
//
//static int lua_error_pcall(lua_State * L)
//{
//   lua_Debug d;
//   lua_getstack(L, 1, &d);
//   lua_getinfo(L, "Sln", &d);
//   std::string err = lua_tostring(L, -1);
//   lua_pop(L, 1);
//   std::stringstream msg;
//   msg << d.short_src << ":" << d.currentline;
//
//   if (d.name != 0)
//   {
//      msg << "(" << d.namewhat << " " << d.name << ")";
//   }
//   msg << " " << err;
//   lua_pushstring(L, msg.str().c_str());
//   return 1;
//}

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
			struct InternalExceptionHandler
			{
				luabind::object obj;
				InternalExceptionHandler(const luabind::object & _obj)
					: obj(_obj)
				{
				}
				void operator()(my::ControlPtr control)
				{
					try
					{
						obj(control);
					}
					catch(const luabind::error & e)
					{
						Game::getSingleton().m_panel->AddLine(ms2ws(lua_tostring(e.state(), -1)));
					}
				}
			};
			return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
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

	lua_pushcfunction(L, lua_dofile);
	lua_setglobal(L, "dofile");

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

		static my::TexturePtr LoadTexture(const char * path)
		{
			try
			{
				LPDIRECT3DDEVICE9 pDevice = Game::getSingleton().GetD3D9Device();
				std::string full_path = my::ResourceMgr::getSingleton().GetFullPath(path);
				if(!full_path.empty())
					return my::Texture::CreateTextureFromFile(pDevice, full_path.c_str());

				my::CachePtr cache = my::ResourceMgr::getSingleton().OpenArchiveStream(path)->GetWholeCache();
				return my::Texture::CreateTextureFromFileInMemory(pDevice, &(*cache)[0], cache->size());
			}
			catch(const my::Exception & e)
			{
				throw my::LuaContext::ExecutionErrorException(e.GetDescription());
			}
		}

		static my::FontPtr LoadFont(const char * path, int height)
		{
			try
			{
				LPDIRECT3DDEVICE9 pDevice = Game::getSingleton().GetD3D9Device();
				std::string full_path = my::ResourceMgr::getSingleton().GetFullPath(path);
				if(!full_path.empty())
					return my::Font::CreateFontFromFile(pDevice, full_path.c_str(), height);

				my::CachePtr cache = my::ResourceMgr::getSingleton().OpenArchiveStream(path)->GetWholeCache();
				return my::Font::CreateFontFromFileInCache(pDevice, cache, height);
			}
			catch(const my::Exception & e)
			{
				throw my::LuaContext::ExecutionErrorException(e.GetDescription());
			}
		}
	};

	luabind::open(L);

	// 木有用？
	//luabind::set_pcall_callback(lua_error_pcall);

	luabind::module(L)
	[
		luabind::def("ARGB", &HelpFunc::ARGB)

		, luabind::def("LoadTexture", &HelpFunc::LoadTexture)

		, luabind::def("LoadFont", &HelpFunc::LoadFont)

		, luabind::class_<my::Vector2, boost::shared_ptr<my::Vector2> >("Vector2")
			.def(luabind::constructor<float, float>())
			.def_readwrite("x", &my::Vector2::x)
			.def_readwrite("y", &my::Vector2::y)

		, luabind::class_<my::Vector3, boost::shared_ptr<my::Vector3> >("Vector3")
			.def(luabind::constructor<float, float, float>())
			.def_readwrite("x", &my::Vector3::x)
			.def_readwrite("y", &my::Vector3::y)
			.def_readwrite("z", &my::Vector3::z)

		, luabind::class_<my::Vector4, boost::shared_ptr<my::Vector4> >("Vector4")
			.def(luabind::constructor<float, float, float, float>())
			.def_readwrite("x", &my::Vector4::x)
			.def_readwrite("y", &my::Vector4::y)
			.def_readwrite("z", &my::Vector4::z)
			.def_readwrite("w", &my::Vector4::w)
		
		, luabind::class_<CPoint, boost::shared_ptr<CPoint> >("CPoint")
			.def(luabind::constructor<int, int>())

		, luabind::class_<CSize, boost::shared_ptr<CSize> >("CSize")
			.def(luabind::constructor<int, int>())

		, luabind::class_<CRect, boost::shared_ptr<CRect> >("CRect")
			.def(luabind::constructor<int, int, int, int>())
			.def(luabind::constructor<CPoint, CSize>())

		, luabind::class_<my::Texture, boost::shared_ptr<my::Texture> >("Texture")

		, luabind::class_<my::Font, boost::shared_ptr<my::Font> >("Font")
			.enum_("constants")
			[
				luabind::value("AlignLeft", my::Font::AlignLeft)
				, luabind::value("AlignCenter", my::Font::AlignCenter)
				, luabind::value("AlignRight", my::Font::AlignRight)
				, luabind::value("AlignTop", my::Font::AlignTop)
				, luabind::value("AlignMiddle", my::Font::AlignMiddle)
				, luabind::value("AlignBottom", my::Font::AlignBottom)
				, luabind::value("AlignLeftTop", my::Font::AlignLeftTop)
				, luabind::value("AlignCenterTop", my::Font::AlignCenterTop)
				, luabind::value("AlignRightTop", my::Font::AlignRightTop)
				, luabind::value("AlignLeftMiddle", my::Font::AlignLeftMiddle)
				, luabind::value("AlignCenterMiddle", my::Font::AlignCenterMiddle)
				, luabind::value("AlignRightMiddle", my::Font::AlignRightMiddle)
				, luabind::value("AlignLeftBottom", my::Font::AlignLeftBottom)
				, luabind::value("AlignCenterBottom", my::Font::AlignCenterBottom)
				, luabind::value("AlignRightBottom", my::Font::AlignRightBottom)
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
			.def("SetHotkey", &my::Button::SetHotkey)

		, luabind::class_<my::EditBoxSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("EditBoxSkin")
			.def(luabind::constructor<>())
			.def_readwrite("DisabledTexRect", &my::EditBoxSkin::m_DisabledTexRect)
			.def_readwrite("FocusedTexRect", &my::EditBoxSkin::m_FocusedTexRect)
			.def_readwrite("SelTextColor", &my::EditBoxSkin::m_SelTextColor)
			.def_readwrite("SelBkColor", &my::EditBoxSkin::m_SelBkColor)
			.def_readwrite("CaretColor", &my::EditBoxSkin::m_CaretColor)

		, luabind::class_<my::EditBox, my::Static, boost::shared_ptr<my::Control> >("EditBox")
			.def(luabind::constructor<>())
			.def_readwrite("nCaret", &my::EditBox::m_nCaret) // ! should be removed
			.def_readwrite("nFirstVisible", &my::EditBox::m_nFirstVisible) // ! should be removed
			.def_readwrite("Border", &my::EditBox::m_Border)
			.def_readwrite("EventChange", &my::EditBox::EventChange)
			.def_readwrite("EventEnter", &my::EditBox::EventEnter)

		, luabind::class_<my::ImeEditBox, my::EditBox, boost::shared_ptr<my::Control> >("ImeEditBox")
			.def(luabind::constructor<>())

		, luabind::class_<my::ScrollBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ScrollBarSkin")
			.def(luabind::constructor<>())
			.def_readwrite("UpBtnNormalTexRect", &my::ScrollBarSkin::m_UpBtnNormalTexRect)
			.def_readwrite("UpBtnDisabledTexRect", &my::ScrollBarSkin::m_UpBtnDisabledTexRect)
			.def_readwrite("DownBtnNormalTexRect", &my::ScrollBarSkin::m_DownBtnNormalTexRect)
			.def_readwrite("DownBtnDisabledTexRect", &my::ScrollBarSkin::m_DownBtnDisabledTexRect)
			.def_readwrite("ThumbBtnNormalTexRect", &my::ScrollBarSkin::m_ThumbBtnNormalTexRect)
			.def_readwrite("ThumbBtnDisabledTexRect", &my::ScrollBarSkin::m_ThumbBtnDisabledTexRect)

		, luabind::class_<my::ScrollBar, my::Control, boost::shared_ptr<my::Control> >("ScrollBar")
			.def(luabind::constructor<>())
			.def_readwrite("nPosition", &my::ScrollBar::m_nPosition) // ! should be removed
			.def_readwrite("nPageSize", &my::ScrollBar::m_nPageSize) // ! should be removed
			.def_readwrite("nStart", &my::ScrollBar::m_nStart) // ! should be removed
			.def_readwrite("nEnd", &my::ScrollBar::m_nEnd) // ! should be removed

		, luabind::class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(luabind::constructor<>())
			.def("InsertControl", &my::Dialog::InsertControl)

		, luabind::class_<MessagePanel, my::Control, boost::shared_ptr<my::Control> >("MessagePanel")
			.def(luabind::constructor<>())
			.def_readwrite("lbegin", &MessagePanel::m_lbegin)
			.def_readwrite("lend", &MessagePanel::m_lend)
			.def_readwrite("scrollbar", &MessagePanel::m_scrollbar)
			.def("AddLine", &MessagePanel::AddLine)
			.def("puts", &MessagePanel::puts)

		, luabind::class_<Game>("Game")
			.def_readonly("uiFnt", &Game::m_uiFnt)
			.def("ToggleFullScreen", &Game::ToggleFullScreen)
			//.def("ToggleRef", &Game::ToggleRef)
			.def("ChangeDevice", &Game::ChangeDevice)
			.def("ExecuteCode", &Game::ExecuteCode)
			.def("InsertDlg", &Game::InsertDlg)
	];

	luabind::globals(L)["game"] = Game::getSingletonPtr();
}

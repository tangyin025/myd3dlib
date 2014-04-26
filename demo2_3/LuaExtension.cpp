#include "stdafx.h"
#include "LuaExtension.h"
#include "Game.h"

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
	my::IOStreamPtr stream;
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
		lf.stream = Game::getSingleton().OpenStream(filename);
	}
	catch(const my::Exception & e)
	{
		lua_pushfstring(L, ts2ms(e.what()).c_str());
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
	HWND hwnd = my::DxutWindow::getSingleton().m_hWnd;
	_ASSERT(NULL != hwnd);
	SendMessage(hwnd, WM_CLOSE, 0, 0);
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
	std::string s = ts2ms(e.what());
	lua_pushlstring(L, s.c_str(), s.length());
}

namespace luabind
{
	template <>
	struct default_converter<std::wstring>
		: native_converter_base<std::wstring>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
		}

		std::wstring from(lua_State* L, int index)
		{
			return u8tows(lua_tostring(L, index));
		}

		void to(lua_State* L, std::wstring const& value)
		{
			std::string str = wstou8(value);
			lua_pushlstring(L, str.data(), str.size());
		}
	};

	template <>
	struct default_converter<std::wstring const>
		: default_converter<std::wstring>
	{};

	template <>
	struct default_converter<std::wstring const&>
		: default_converter<std::wstring>
	{};

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
				void operator()(my::EventArgsPtr args)
				{
					try
					{
						obj(args);
					}
					catch(const luabind::error & e)
					{
						// ! ControlEvent�¼��������ݴ�ģ����¼�����ʧ�ܺ󣬳����������
						Game::getSingleton().AddLine(ms2ws(lua_tostring(e.state(), -1)));
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

	template <>
	struct default_converter<my::ResourceCallback>
		: native_converter_base<my::ResourceCallback>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::ResourceCallback from(lua_State * L, int index)
		{
			struct InternalExceptionHandler
			{
				luabind::object obj;
				InternalExceptionHandler(const luabind::object & _obj)
					: obj(_obj)
				{
				}
				void operator()(my::DeviceRelatedObjectBasePtr args)
				{
					try
					{
						obj(args);
					}
					catch(const luabind::error & e)
					{
						// ! ControlEvent�¼��������ݴ�ģ����¼�����ʧ�ܺ󣬳����������
						Game::getSingleton().AddLine(ms2ws(lua_tostring(e.state(), -1)));
					}
				}
			};
			return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
		}

		void to(lua_State * L, my::ResourceCallback const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::ResourceCallback const &>
		: default_converter<my::ResourceCallback>
	{
	};
}

struct HelpFunc
{
	static DWORD ARGB(int a, int r, int g, int b)
	{
		return D3DCOLOR_ARGB(a,r,g,b);
	}
};

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

	using namespace luabind;

	open(L);

	// ! �ᵼ���ڴ�й©����������д handle_exception_aux������ my::Exception��֧��
	register_exception_handler<my::Exception>(&translate_my_exception);

	//// ! Ϊʲô��������
	//set_pcall_callback(add_file_and_line);

	module(L)
	[
		def("ARGB", &HelpFunc::ARGB)

		, class_<my::Vector2, boost::shared_ptr<my::Vector2> >("Vector2")
			.def(constructor<float, float>())
			.def_readwrite("x", &my::Vector2::x)
			.def_readwrite("y", &my::Vector2::y)
			.def(self + other<const my::Vector2 &>())
			.def(self + float())
			.def(self - other<const my::Vector2 &>())
			.def(self - float())
			.def(self * other<const my::Vector2 &>())
			.def(self * float())
			.def(self / other<const my::Vector2 &>())
			.def(self / float())
			.def("cross", &my::Vector2::cross)
			.def("dot", &my::Vector2::dot)
			.def("magnitude", &my::Vector2::magnitude)
			.def("magnitudeSq", &my::Vector2::magnitudeSq)
			.def("lerp", &my::Vector2::lerp)
			.def("lerpSelf", &my::Vector2::lerpSelf)
			.def("normalize", &my::Vector2::normalize)
			.def("normalizeSelf", &my::Vector2::normalizeSelf)
			.def("transform", &my::Vector2::transform)
			.def("transformTranspose", &my::Vector2::transformTranspose)
			.def("transformCoord", &my::Vector2::transformCoord)
			.def("transformCoordTranspose", &my::Vector2::transformCoordTranspose)
			.def("transformNormal", &my::Vector2::transformNormal)
			.def("transformNormalTranspose", &my::Vector2::transformNormalTranspose)
			.def("transform", &my::Vector2::transform)

		, class_<my::Vector3, boost::shared_ptr<my::Vector3> >("Vector3")
			.def(constructor<float, float, float>())
			.def_readwrite("x", &my::Vector3::x)
			.def_readwrite("y", &my::Vector3::y)
			.def_readwrite("z", &my::Vector3::z)
			.def(self + other<const my::Vector3 &>())
			.def(self + float())
			.def(self - other<const my::Vector3 &>())
			.def(self - float())
			.def(self * other<const my::Vector3 &>())
			.def(self * float())
			.def(self / other<const my::Vector3 &>())
			.def(self / float())
			.def("cross", &my::Vector3::cross)
			.def("dot", &my::Vector3::dot)
			.def("magnitude", &my::Vector3::magnitude)
			.def("magnitudeSq", &my::Vector3::magnitudeSq)
			.def("lerp", &my::Vector3::lerp)
			.def("lerpSelf", &my::Vector3::lerpSelf)
			.def("normalize", &my::Vector3::normalize)
			.def("normalizeSelf", &my::Vector3::normalizeSelf)
			.def("transform", (my::Vector4 (my::Vector3::*)(const my::Matrix4 &) const)&my::Vector3::transform)
			.def("transformTranspose", &my::Vector3::transformTranspose)
			.def("transformCoord", &my::Vector3::transformCoord)
			.def("transformCoordTranspose", &my::Vector3::transformCoordTranspose)
			.def("transformNormal", &my::Vector3::transformNormal)
			.def("transformNormalTranspose", &my::Vector3::transformNormalTranspose)
			.def("transform", (my::Vector3 (my::Vector3::*)(const my::Quaternion &) const)&my::Vector3::transform)

		, class_<my::Vector4, boost::shared_ptr<my::Vector4> >("Vector4")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Vector4::x)
			.def_readwrite("y", &my::Vector4::y)
			.def_readwrite("z", &my::Vector4::z)
			.def_readwrite("w", &my::Vector4::w)
			.def(self + other<const my::Vector4 &>())
			.def(self + float())
			.def(self - other<const my::Vector4 &>())
			.def(self - float())
			.def(self * other<const my::Vector4 &>())
			.def(self * float())
			.def(self / other<const my::Vector4 &>())
			.def(self / float())
			.def("cross", &my::Vector4::cross)
			.def("dot", &my::Vector4::dot)
			.def("magnitude", &my::Vector4::magnitude)
			.def("magnitudeSq", &my::Vector4::magnitudeSq)
			.def("lerp", &my::Vector4::lerp)
			.def("lerpSelf", &my::Vector4::lerpSelf)
			.def("normalize", &my::Vector4::normalize)
			.def("normalizeSelf", &my::Vector4::normalizeSelf)
			.def("transform", &my::Vector4::transform)
			.def("transformTranspose", &my::Vector4::transformTranspose)

		, class_<my::Rectangle, boost::shared_ptr<my::Rectangle> >("Rectangle")
			.def(constructor<float, float, float, float>())
			.def_readwrite("l", &my::Rectangle::l)
			.def_readwrite("t", &my::Rectangle::t)
			.def_readwrite("r", &my::Rectangle::r)
			.def_readwrite("b", &my::Rectangle::b)
			.def("intersect", &my::Rectangle::intersect)
			.def("intersectSelf", &my::Rectangle::intersectSelf)
			.def("Union", &my::Rectangle::Union)
			.def("unionSelf", &my::Rectangle::unionSelf)
			.def("offset", (my::Rectangle (my::Rectangle::*)(float, float) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::offsetSelf)
			.def("offset", (my::Rectangle (my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::offset)
			.def("offsetSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::offsetSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(const my::Vector2 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector2 &))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(float, float, float, float) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(float, float, float, float))&my::Rectangle::shrinkSelf)
			.def("shrink", (my::Rectangle (my::Rectangle::*)(const my::Vector4 &) const)&my::Rectangle::shrink)
			.def("shrinkSelf", (my::Rectangle & (my::Rectangle::*)(const my::Vector4 &))&my::Rectangle::shrinkSelf)
			.def("LeftTop", (my::Vector2 (my::Rectangle::*)(void) const)&my::Rectangle::LeftTop)
			.def("RightBottom", (my::Vector2 (my::Rectangle::*)(void) const)&my::Rectangle::RightBottom)
			.def("Center", &my::Rectangle::Center)
			.def("Width", &my::Rectangle::Width)
			.def("Height", &my::Rectangle::Height)
			.def("Extent", &my::Rectangle::Extent)
			.def("PtInRect", &my::Rectangle::PtInRect)
			.scope
			[
				def("LeftTop", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::LeftTop),
				def("LeftTop", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftTop),
				def("LeftMiddle", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::LeftMiddle),
				def("LeftMiddle", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftMiddle),
				def("LeftBottom", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::LeftBottom),
				def("LeftBottom", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::LeftBottom),
				def("CenterTop", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::CenterTop),
				def("CenterTop", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterTop),
				def("CenterMiddle", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::CenterMiddle),
				def("CenterMiddle", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterMiddle),
				def("CenterBottom", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::CenterBottom),
				def("CenterBottom", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::CenterBottom),
				def("RightTop", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::RightTop),
				def("RightTop", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightTop),
				def("RightMiddle", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::RightMiddle),
				def("RightMiddle", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightMiddle),
				def("RightBottom", (my::Rectangle (*)(float, float, float, float))&my::Rectangle::RightBottom),
				def("RightBottom", (my::Rectangle (*)(const my::Vector2 &, const my::Vector2 &))&my::Rectangle::RightBottom)
			]

		, class_<my::Quaternion, boost::shared_ptr<my::Quaternion> >("Quaternion")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Quaternion::x)
			.def_readwrite("y", &my::Quaternion::y)
			.def_readwrite("z", &my::Quaternion::z)
			.def_readwrite("w", &my::Quaternion::w)
			.def(self + other<const my::Quaternion &>())
			.def(self + float())
			.def(self - other<const my::Quaternion &>())
			.def(self - float())
			.def(self * other<const my::Quaternion &>())
			.def(self * float())
			.def(self / other<const my::Quaternion &>())
			.def(self / float())
			.def("conjugate", &my::Quaternion::conjugate)
			.def("conjugateSelf", &my::Quaternion::conjugateSelf)
			.def("dot", &my::Quaternion::dot)
			.def("inverse", &my::Quaternion::inverse)
			.def("magnitude", &my::Quaternion::magnitude)
			.def("magnitudeSq", &my::Quaternion::magnitudeSq)
			.def("ln", &my::Quaternion::ln)
			.def("multiply", &my::Quaternion::multiply)
			.def("normalize", &my::Quaternion::normalize)
			.def("normalizeSelf", &my::Quaternion::normalizeSelf)
			.def("lerp", &my::Quaternion::lerp)
			.def("lerpSelf", &my::Quaternion::lerpSelf)
			.def("slerp", &my::Quaternion::slerp)
			.def("slerpSelf", &my::Quaternion::slerpSelf)
			.def("squad", &my::Quaternion::squad)
			.def("squadSelf", &my::Quaternion::squadSelf)
			.scope
			[
				def("Identity", &my::Quaternion::Identity),
				def("RotationAxis", &my::Quaternion::RotationAxis),
				def("RotationMatrix", &my::Quaternion::RotationMatrix),
				def("RotationYawPitchRoll", &my::Quaternion::RotationYawPitchRoll),
				def("RotationFromTo", &my::Quaternion::RotationFromTo)
			]

		, class_<my::Matrix4, boost::shared_ptr<my::Matrix4> >("Matrix4")
			.def(self + other<const my::Matrix4 &>())
			.def(self + float())
			.def(self - other<const my::Matrix4 &>())
			.def(self - float())
			.def(self * other<const my::Matrix4 &>())
			.def(self * float())
			.def(self / other<const my::Matrix4 &>())
			.def(self / float())
			.def("inverse", &my::Matrix4::inverse)
			.def("multiply", &my::Matrix4::multiply)
			.def("multiplyTranspose", &my::Matrix4::multiplyTranspose)
			.def("transpose", &my::Matrix4::transpose)
			.def("transformTranspose", &my::Matrix4::transformTranspose)
			.def("scale", (my::Matrix4 (my::Matrix4::*)(float, float, float) const)&my::Matrix4::scale)
			.def("scale", (my::Matrix4 (my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::scale)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::scaleSelf)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::scaleSelf)
			.def("rotateX", &my::Matrix4::rotateX)
			.def("rotateY", &my::Matrix4::rotateY)
			.def("rotateZ", &my::Matrix4::rotateZ)
			.def("rotate", &my::Matrix4::rotate)
			.def("translate", (my::Matrix4 (my::Matrix4::*)(float, float, float) const)&my::Matrix4::translate)
			.def("translate", (my::Matrix4 (my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::translate)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::translateSelf)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::translateSelf)
			.def("lerp", &my::Matrix4::lerp)
			.def("lerpSelf", &my::Matrix4::lerpSelf)
			.scope
			[
				def("Identity", &my::Matrix4::Identity),
				def("LookAtLH", &my::Matrix4::LookAtLH),
				def("LookAtRH", &my::Matrix4::LookAtRH),
				def("OrthoLH", &my::Matrix4::OrthoLH),
				def("OrthoRH", &my::Matrix4::OrthoRH),
				def("OrthoOffCenterLH", &my::Matrix4::OrthoOffCenterLH),
				def("OrthoOffCenterRH", &my::Matrix4::OrthoOffCenterRH),
				def("PerspectiveFovLH", &my::Matrix4::PerspectiveFovLH),
				def("PerspectiveFovRH", &my::Matrix4::PerspectiveFovRH),
				def("PerspectiveLH", &my::Matrix4::PerspectiveLH),
				def("PerspectiveRH", &my::Matrix4::PerspectiveRH),
				def("PerspectiveOffCenterLH", &my::Matrix4::PerspectiveOffCenterLH),
				def("PerspectiveOffCenterRH", &my::Matrix4::PerspectiveOffCenterRH),
				def("RotationAxis", &my::Matrix4::RotationAxis),
				def("RotationQuaternion", &my::Matrix4::RotationQuaternion),
				def("RotationX", &my::Matrix4::RotationX),
				def("RotationY", &my::Matrix4::RotationY),
				def("RotationZ", &my::Matrix4::RotationZ),
				def("RotationYawPitchRoll", &my::Matrix4::RotationYawPitchRoll),
				def("Scaling", (my::Matrix4 (*)(float, float, float))&my::Matrix4::Scaling),
				def("Scaling", (my::Matrix4 (*)(const my::Vector3 &))&my::Matrix4::Scaling),
				def("Transformation", &my::Matrix4::Transformation),
				def("Transformation2D", &my::Matrix4::Transformation2D),
				def("Translation", (my::Matrix4 (*)(float, float, float))&my::Matrix4::Translation),
				def("Translation", (my::Matrix4 (*)(const my::Vector3 &))&my::Matrix4::Translation)
			]

		, class_<my::Spline, boost::shared_ptr<my::Spline> >("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("Interpolate", (float (my::Spline::*)(float, float) const)&my::Spline::Interpolate)

		, class_<my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("BaseTexture")

		, class_<my::Texture2D, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("Texture2D")

		, class_<my::CubeTexture, my::BaseTexture, boost::shared_ptr<my::BaseTexture> >("CubeTexture")

		, class_<my::Mesh, boost::shared_ptr<my::Mesh> >("Mesh")

		, class_<my::OgreMesh, my::Mesh, boost::shared_ptr<my::OgreMesh> >("OgreMesh")
			.def("GetMaterialNum", &my::OgreMesh::GetMaterialNum)
			.def("GetMaterialName", &my::OgreMesh::GetMaterialName)

		, class_<my::OgreSkeletonAnimation, boost::shared_ptr<my::OgreSkeletonAnimation> >("OgreSkeletonAnimation")

		// ! many methods of my::BaseEffect, my::Effect cannot be use in lua
		, class_<my::BaseEffect, boost::shared_ptr<my::BaseEffect> >("BaseEffect")
			.def("GetAnnotation", &my::BaseEffect::GetAnnotation)
			.def("GetAnnotationByName", &my::BaseEffect::GetAnnotationByName)
			.def("GetBool", &my::BaseEffect::GetBool)
			.def("GetBoolArray", &my::BaseEffect::GetBoolArray)
			.def("GetDesc", &my::BaseEffect::GetDesc)
			.def("GetFloat", &my::BaseEffect::GetFloat)
			.def("GetFloatArray", &my::BaseEffect::GetFloatArray)
			.def("GetFunction", &my::BaseEffect::GetFunction)
			.def("GetFunctionByName", &my::BaseEffect::GetFunctionByName)
			.def("GetFunctionDesc", &my::BaseEffect::GetFunctionDesc)
			.def("GetInt", &my::BaseEffect::GetInt)
			.def("GetIntArray", &my::BaseEffect::GetIntArray)
			.def("GetMatrix", &my::BaseEffect::GetMatrix)
			.def("GetMatrixArray", &my::BaseEffect::GetMatrixArray)
			.def("GetMatrixPointerArray", &my::BaseEffect::GetMatrixPointerArray)
			.def("GetMatrixTranspose", &my::BaseEffect::GetMatrixTranspose)
			.def("GetMatrixTransposeArray", &my::BaseEffect::GetMatrixTransposeArray)
			.def("GetMatrixTransposePointerArray", &my::BaseEffect::GetMatrixTransposePointerArray)
			.def("GetParameter", &my::BaseEffect::GetParameter)
			.def("GetParameterByName", &my::BaseEffect::GetParameterByName)
			.def("GetParameterBySemantic", &my::BaseEffect::GetParameterBySemantic)
			.def("GetParameterDesc", &my::BaseEffect::GetParameterDesc)
			.def("GetParameterElement", &my::BaseEffect::GetParameterElement)
			.def("GetPass", &my::BaseEffect::GetPass)
			.def("GetPassByName", &my::BaseEffect::GetPassByName)
			.def("GetPassDesc", &my::BaseEffect::GetPassDesc)
			.def("GetPixelShader", &my::BaseEffect::GetPixelShader)
			.def("GetString", &my::BaseEffect::GetString)
			.def("GetTechnique", &my::BaseEffect::GetTechnique)
			.def("GetTechniqueByName", &my::BaseEffect::GetTechniqueByName)
			.def("GetTechniqueDesc", &my::BaseEffect::GetTechniqueDesc)
			.def("GetTexture", &my::BaseEffect::GetTexture)
			.def("GetValue", &my::BaseEffect::GetValue)
			.def("GetVector", &my::BaseEffect::GetVector)
			.def("GetVectorArray", &my::BaseEffect::GetVectorArray)
			.def("GetVertexShader", &my::BaseEffect::GetVertexShader)
			.def("SetArrayRange", &my::BaseEffect::SetArrayRange)
			.def("SetBool", &my::BaseEffect::SetBool)
			.def("SetBoolArray", &my::BaseEffect::SetBoolArray)
			.def("SetFloat", &my::BaseEffect::SetFloat)
			.def("SetFloatArray", &my::BaseEffect::SetFloatArray)
			.def("SetInt", &my::BaseEffect::SetInt)
			.def("SetIntArray", &my::BaseEffect::SetIntArray)
			.def("SetMatrix", &my::BaseEffect::SetMatrix)
			.def("SetMatrixArray", &my::BaseEffect::SetMatrixArray)
			.def("SetMatrixPointerArray", &my::BaseEffect::SetMatrixPointerArray)
			.def("SetMatrixTranspose", &my::BaseEffect::SetMatrixTranspose)
			.def("SetMatrixTransposeArray", &my::BaseEffect::SetMatrixTransposeArray)
			.def("SetMatrixTransposePointerArray", &my::BaseEffect::SetMatrixTransposePointerArray)
			.def("SetString", &my::BaseEffect::SetString)
			// ! luabind cannot convert boost::shared_ptr<Derived Class> to base ptr
			.def("SetTexture", &my::BaseEffect::SetTexture)
			.def("SetValue", &my::BaseEffect::SetValue)
			.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector4 &))&my::BaseEffect::SetVector)
			.def("SetVector", (void (my::BaseEffect::*)(D3DXHANDLE, const my::Vector3 &))&my::BaseEffect::SetVector)
			.def("SetVectorArray", &my::BaseEffect::SetVectorArray)

		, class_<my::Effect, my::BaseEffect, boost::shared_ptr<my::Effect> >("Effect")
			.def("ApplyParameterBlock", &my::Effect::ApplyParameterBlock)
			.def("Begin", &my::Effect::Begin)
			.def("BeginParameterBlock", &my::Effect::BeginParameterBlock)
			.def("BeginPass", &my::Effect::BeginPass)
			.def("CloneEffect", &my::Effect::CloneEffect)
			.def("CommitChanges", &my::Effect::CommitChanges)
			.def("DeleteParameterBlock", &my::Effect::DeleteParameterBlock)
			.def("End", &my::Effect::End)
			.def("EndParameterBlock", &my::Effect::EndParameterBlock)
			.def("EndPass", &my::Effect::EndPass)
			.def("FindNextValidTechnique", &my::Effect::FindNextValidTechnique)
			.def("GetCurrentTechnique", &my::Effect::GetCurrentTechnique)
			.def("GetDevice", &my::Effect::GetDevice)
			.def("GetPool", &my::Effect::GetPool)
			.def("GetStateManager", &my::Effect::GetStateManager)
			.def("IsParameterUsed", &my::Effect::IsParameterUsed)
			.def("SetRawValue", &my::Effect::SetRawValue)
			.def("SetStateManager", &my::Effect::SetStateManager)
			.def("SetTechnique", &my::Effect::SetTechnique)
			.def("ValidateTechnique", &my::Effect::ValidateTechnique)

		, class_<my::Font, boost::shared_ptr<my::Font> >("Font")
			.enum_("Align")
			[
				value("AlignLeft", my::Font::AlignLeft),
				value("AlignCenter", my::Font::AlignCenter),
				value("AlignRight", my::Font::AlignRight),
				value("AlignTop", my::Font::AlignTop),
				value("AlignMiddle", my::Font::AlignMiddle),
				value("AlignBottom", my::Font::AlignBottom),
				value("AlignLeftTop", my::Font::AlignLeftTop),
				value("AlignCenterTop", my::Font::AlignCenterTop),
				value("AlignRightTop", my::Font::AlignRightTop),
				value("AlignLeftMiddle", my::Font::AlignLeftMiddle),
				value("AlignCenterMiddle", my::Font::AlignCenterMiddle),
				value("AlignRightMiddle", my::Font::AlignRightMiddle),
				value("AlignLeftBottom", my::Font::AlignLeftBottom),
				value("AlignCenterBottom", my::Font::AlignCenterBottom),
				value("AlignRightBottom", my::Font::AlignRightBottom)
			]
			.def_readonly("Height", &my::Font::m_Height)
			.property("Scale", &my::Font::GetScale, &my::Font::SetScale)
			.def_readonly("LineHeight", &my::Font::m_LineHeight)

		, class_<my::EventArgs, boost::shared_ptr<my::EventArgs> >("EventArgs")

		, class_<my::ControlEvent>("ControlEvent")

		, class_<my::ControlImage, boost::shared_ptr<my::ControlImage> >("ControlImage")
			.def(constructor<my::BaseTexturePtr, const my::Vector4 &>())

		, class_<my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ControlSkin")
			.def(constructor<>())
			.def_readwrite("Image", &my::ControlSkin::m_Image)
			.def_readwrite("Font", &my::ControlSkin::m_Font)
			.def_readwrite("TextColor", &my::ControlSkin::m_TextColor)
			.def_readwrite("TextAlign", &my::ControlSkin::m_TextAlign)

		, class_<my::Control, boost::shared_ptr<my::Control> >("Control")
			.def(constructor<>())
			.property("Enabled", &my::Control::GetEnabled, &my::Control::SetEnabled)
			.property("Visible", &my::Control::GetVisible, &my::Control::SetVisible)
			.def_readwrite("Location", &my::Control::m_Location)
			.def_readwrite("Size", &my::Control::m_Size)
			.def_readwrite("Color", &my::Control::m_Color)
			.def_readwrite("Skin", &my::Control::m_Skin)

		, class_<my::Static, my::Control, boost::shared_ptr<my::Control> >("Static")
			.def(constructor<>())
			.def_readwrite("Text", &my::Static::m_Text)

		, class_<my::ButtonSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ButtonSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::ButtonSkin::m_DisabledImage)
			.def_readwrite("PressedImage", &my::ButtonSkin::m_PressedImage)
			.def_readwrite("MouseOverImage", &my::ButtonSkin::m_MouseOverImage)
			.def_readwrite("PressedOffset", &my::ButtonSkin::m_PressedOffset)

		, class_<my::Button, my::Static, boost::shared_ptr<my::Control> >("Button")
			.def(constructor<>())
			.def_readwrite("EventClick", &my::Button::EventClick)
			.def("SetHotkey", &my::Button::SetHotkey)

		, class_<my::EditBoxSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("EditBoxSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::EditBoxSkin::m_DisabledImage)
			.def_readwrite("FocusedImage", &my::EditBoxSkin::m_FocusedImage)
			.def_readwrite("SelBkColor", &my::EditBoxSkin::m_SelBkColor)
			.def_readwrite("CaretColor", &my::EditBoxSkin::m_CaretColor)

		, class_<my::EditBox, my::Static, boost::shared_ptr<my::Control> >("EditBox")
			.def(constructor<>())
			.property("Text", &my::EditBox::GetText, &my::EditBox::SetText)
			.def_readwrite("Border", &my::EditBox::m_Border)
			.def_readwrite("EventChange", &my::EditBox::EventChange)
			.def_readwrite("EventEnter", &my::EditBox::EventEnter)

		, class_<my::ImeEditBox, my::EditBox, boost::shared_ptr<my::Control> >("ImeEditBox")
			.def(constructor<>())

		, class_<my::ScrollBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ScrollBarSkin")
			.def(constructor<>())
			.def_readwrite("UpBtnNormalImage", &my::ScrollBarSkin::m_UpBtnNormalImage)
			.def_readwrite("UpBtnDisabledImage", &my::ScrollBarSkin::m_UpBtnDisabledImage)
			.def_readwrite("DownBtnNormalImage", &my::ScrollBarSkin::m_DownBtnNormalImage)
			.def_readwrite("DownBtnDisabledImage", &my::ScrollBarSkin::m_DownBtnDisabledImage)
			.def_readwrite("ThumbBtnNormalImage", &my::ScrollBarSkin::m_ThumbBtnNormalImage)

		, class_<my::ScrollBar, my::Control, boost::shared_ptr<my::Control> >("ScrollBar")
			.def(constructor<>())
			.def_readwrite("nPosition", &my::ScrollBar::m_nPosition) // ! should use property
			.def_readwrite("nPageSize", &my::ScrollBar::m_nPageSize) // ! should use property
			.def_readwrite("nStart", &my::ScrollBar::m_nStart) // ! should use property
			.def_readwrite("nEnd", &my::ScrollBar::m_nEnd) // ! should use property

		, class_<my::CheckBox, my::Button, boost::shared_ptr<my::Control> >("CheckBox")
			.def(constructor<>())
			.def_readwrite("Checked", &my::CheckBox::m_Checked)

		, class_<my::ComboBoxSkin, my::ButtonSkin, boost::shared_ptr<my::ControlSkin> >("ComboBoxSkin")
			.def(constructor<>())
			.def_readwrite("DropdownImage", &my::ComboBoxSkin::m_DropdownImage)
			.def_readwrite("DropdownItemMouseOverImage", &my::ComboBoxSkin::m_DropdownItemMouseOverImage)
			.def_readwrite("ScrollBarUpBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarUpBtnNormalImage)
			.def_readwrite("ScrollBarUpBtnDisabledImage", &my::ComboBoxSkin::m_ScrollBarUpBtnDisabledImage)
			.def_readwrite("ScrollBarDownBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarDownBtnNormalImage)
			.def_readwrite("ScrollBarDownBtnDisabledImage", &my::ComboBoxSkin::m_ScrollBarDownBtnDisabledImage)
			.def_readwrite("ScrollBarThumbBtnNormalImage", &my::ComboBoxSkin::m_ScrollBarThumbBtnNormalImage)
			.def_readwrite("ScrollBarImage", &my::ComboBoxSkin::m_ScrollBarImage)

		, class_<my::ComboBox, my::Button, boost::shared_ptr<my::Control> >("ComboBox")
			.def(constructor<>())
			.property("DropdownSize", &my::ComboBox::GetDropdownSize, &my::ComboBox::SetDropdownSize)
			.property("Border", &my::ComboBox::GetBorder, &my::ComboBox::SetBorder)
			.property("ItemHeight", &my::ComboBox::GetItemHeight, &my::ComboBox::SetItemHeight)
			.property("Selected", &my::ComboBox::GetSelected, &my::ComboBox::SetSelected)
			.def_readwrite("ScrollbarWidth", &my::ComboBox::m_ScrollbarWidth)
			.def("AddItem", &my::ComboBox::AddItem)
			.def("RemoveAllItems", &my::ComboBox::RemoveAllItems)
			.def("ContainsItem", &my::ComboBox::ContainsItem)
			.def("FindItem", &my::ComboBox::FindItem)
			.def("GetItemData", &my::ComboBox::GetItemDataUInt)
			.def("SetItemData", (void (my::ComboBox::*)(int, unsigned int))&my::ComboBox::SetItemData)
			.def("GetNumItems", &my::ComboBox::GetNumItems)
			.def_readwrite("EventSelectionChanged", &my::ComboBox::EventSelectionChanged)

		, class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(constructor<>())
			.def_readwrite("World", &my::Dialog::m_World)
			.def("Refresh", &my::Dialog::Refresh)
			.def("InsertControl", &my::Dialog::InsertControl)
			.def("RemoveControl", &my::Dialog::RemoveControl)
			.def("ClearAllControl", &my::Dialog::ClearAllControl)
			.def_readwrite("EventAlign", &my::Dialog::EventAlign)
			.def_readwrite("EventRefresh", &my::Dialog::EventRefresh)

		, class_<my::Emitter, boost::shared_ptr<my::Emitter> >("Emitter")
			.def(constructor<>())
			.enum_("WorldType")
			[
				value("WorldTypeWorld", my::Emitter::WorldTypeWorld),
				value("WorldTypeLocal", my::Emitter::WorldTypeLocal)
			]
			.def_readwrite("WorldType", &my::Emitter::m_WorldType)
			.enum_("DirectionType")
			[
				value("DirectionTypeCamera", my::Emitter::DirectionTypeCamera),
				value("DirectionTypeVertical", my::Emitter::DirectionTypeVertical),
				value("DirectionTypeHorizontal", my::Emitter::DirectionTypeHorizontal)
			]
			.def_readwrite("DirectionType", &my::Emitter::m_DirectionType)
			.def_readwrite("Position", &my::SphericalEmitter::m_Position)
			.def_readwrite("Orientation", &my::SphericalEmitter::m_Orientation)
			.def_readwrite("ParticleLifeTime", &my::Emitter::m_ParticleLifeTime)
			.def_readonly("ParticleColorA", &my::Emitter::m_ParticleColorA)
			.def_readonly("ParticleColorR", &my::Emitter::m_ParticleColorR)
			.def_readonly("ParticleColorG", &my::Emitter::m_ParticleColorG)
			.def_readonly("ParticleColorB", &my::Emitter::m_ParticleColorB)
			.def_readonly("ParticleSizeX", &my::Emitter::m_ParticleSizeX)
			.def_readonly("ParticleSizeY", &my::Emitter::m_ParticleSizeY)
			.def_readonly("ParticleAngle", &my::Emitter::m_ParticleAngle)
			.def_readwrite("ParticleAnimFPS", &my::Emitter::m_ParticleAnimFPS)
			.def_readwrite("ParticleAnimColumn", &my::Emitter::m_ParticleAnimColumn)
			.def_readwrite("ParticleAnimRow", &my::Emitter::m_ParticleAnimRow)
			.def_readwrite("Texture", &my::Emitter::m_Texture)

		, class_<my::SphericalEmitter, my::Emitter, boost::shared_ptr<my::Emitter> >("SphericalEmitter")
			.def(constructor<>())
			.def_readonly("Time", &my::SphericalEmitter::m_Time)
			.def_readwrite("SpawnInterval", &my::SphericalEmitter::m_SpawnInterval)
			.def_readwrite("RemainingSpawnTime", &my::SphericalEmitter::m_RemainingSpawnTime)
			.def_readwrite("HalfSpawnArea", &my::SphericalEmitter::m_HalfSpawnArea)
			.def_readwrite("SpawnSpeed", &my::SphericalEmitter::m_SpawnSpeed)
			.def_readwrite("SpawnInclination", &my::SphericalEmitter::m_SpawnInclination)
			.def_readwrite("SpawnAzimuth", &my::SphericalEmitter::m_SpawnAzimuth)
			.def_readwrite("SpawnLoopTime", &my::SphericalEmitter::m_SpawnLoopTime)

		, class_<my::ResourceCallback>("ResourceCallback")

		, class_<my::EffectMacroPair>("EffectMacroPair")
			.def(constructor<>())
			.def_readwrite("first", &my::EffectMacroPair::first)
			.def_readwrite("second", &my::EffectMacroPair::second)

		, class_<my::EffectMacroPairList>("EffectMacroPairList")
			.def(constructor<>())
			.def("push_back", &my::EffectMacroPairList::push_back)

		, class_<my::AsynchronousResourceMgr>("AsynchronousResourceMgr")
			.def("LoadTextureAsync", &my::AsynchronousResourceMgr::LoadTextureAsync)
			.def("LoadTexture", &my::AsynchronousResourceMgr::LoadTexture)
			.def("LoadMeshAsync", &my::AsynchronousResourceMgr::LoadMeshAsync)
			.def("LoadMesh", &my::AsynchronousResourceMgr::LoadMesh)
			.def("LoadSkeletonAsync", &my::AsynchronousResourceMgr::LoadSkeletonAsync)
			.def("LoadSkeleton", &my::AsynchronousResourceMgr::LoadSkeleton)
			.def("LoadEffectAsync", &my::AsynchronousResourceMgr::LoadEffectAsync)
			.def("LoadEffect", &my::AsynchronousResourceMgr::LoadEffect)
			.def("LoadFontAsync", &my::AsynchronousResourceMgr::LoadFontAsync)
			.def("LoadFont", &my::AsynchronousResourceMgr::LoadFont)

		, class_<D3DSURFACE_DESC>("D3DSURFACE_DESC")
			.def_readwrite("Format", &D3DSURFACE_DESC::Format)
			.def_readwrite("Type", &D3DSURFACE_DESC::Type)
			.def_readwrite("Usage", &D3DSURFACE_DESC::Usage)
			.def_readwrite("Pool", &D3DSURFACE_DESC::Pool)
			.def_readwrite("MultiSampleType", &D3DSURFACE_DESC::MultiSampleType)
			.def_readwrite("MultiSampleQuality", &D3DSURFACE_DESC::MultiSampleQuality)
			.def_readwrite("Width", &D3DSURFACE_DESC::Width)
			.def_readwrite("Height", &D3DSURFACE_DESC::Height)

		, class_<D3DPRESENT_PARAMETERS>("D3DPRESENT_PARAMETERS")
			.def_readwrite("BackBufferWidth", &D3DPRESENT_PARAMETERS::BackBufferWidth)
			.def_readwrite("BackBufferHeight", &D3DPRESENT_PARAMETERS::BackBufferHeight)
			.def_readwrite("BackBufferFormat", &D3DPRESENT_PARAMETERS::BackBufferFormat)
			.def_readwrite("BackBufferCount", &D3DPRESENT_PARAMETERS::BackBufferCount)
			.def_readwrite("MultiSampleType", &D3DPRESENT_PARAMETERS::MultiSampleType)
			.def_readwrite("MultiSampleQuality", &D3DPRESENT_PARAMETERS::MultiSampleQuality)
			.def_readwrite("SwapEffect", &D3DPRESENT_PARAMETERS::SwapEffect)
			.def_readwrite("hDeviceWindow", &D3DPRESENT_PARAMETERS::hDeviceWindow)
			.def_readwrite("Windowed", &D3DPRESENT_PARAMETERS::Windowed)
			.def_readwrite("EnableAutoDepthStencil", &D3DPRESENT_PARAMETERS::EnableAutoDepthStencil)
			.def_readwrite("AutoDepthStencilFormat", &D3DPRESENT_PARAMETERS::AutoDepthStencilFormat)
			.def_readwrite("Flags", &D3DPRESENT_PARAMETERS::Flags)
			.def_readwrite("FullScreen_RefreshRateInHz", &D3DPRESENT_PARAMETERS::FullScreen_RefreshRateInHz)
			.def_readwrite("PresentationInterval", &D3DPRESENT_PARAMETERS::PresentationInterval)

		, class_<D3DDISPLAYMODE>("D3DDISPLAYMODE")
			.def_readwrite("Width", &D3DDISPLAYMODE::Width)
			.def_readwrite("Height", &D3DDISPLAYMODE::Height)
			.def_readwrite("RefreshRate", &D3DDISPLAYMODE::RefreshRate)
			.def_readwrite("Format", &D3DDISPLAYMODE::Format)

		, class_<CGrowableArray<D3DDISPLAYMODE> >("CD3D9EnumAdapterInfoArray")
			.def("GetAt", &CGrowableArray<D3DDISPLAYMODE>::GetAt)
			.def("GetSize", &CGrowableArray<D3DDISPLAYMODE>::GetSize)

		, class_<DXUTD3D9DeviceSettings>("DXUTD3D9DeviceSettings")
			.enum_("VertexProcessingType")
			[
				value("D3DCREATE_SOFTWARE_VERTEXPROCESSING", D3DCREATE_SOFTWARE_VERTEXPROCESSING),
				value("D3DCREATE_MIXED_VERTEXPROCESSING", D3DCREATE_MIXED_VERTEXPROCESSING),
				value("D3DCREATE_HARDWARE_VERTEXPROCESSING", D3DCREATE_HARDWARE_VERTEXPROCESSING),
				value("D3DCREATE_PUREDEVICE", D3DCREATE_PUREDEVICE)
			]
			.enum_("PresentIntervalType")
			[
				value("D3DPRESENT_INTERVAL_DEFAULT", D3DPRESENT_INTERVAL_DEFAULT),
				value("D3DPRESENT_INTERVAL_IMMEDIATE", D3DPRESENT_INTERVAL_IMMEDIATE)
			]
			.def_readwrite("AdapterOrdinal", &DXUTD3D9DeviceSettings::AdapterOrdinal)
			.def_readwrite("DeviceType", &DXUTD3D9DeviceSettings::DeviceType)
			.def_readwrite("AdapterFormat", &DXUTD3D9DeviceSettings::AdapterFormat)
			.def_readwrite("BehaviorFlags", &DXUTD3D9DeviceSettings::BehaviorFlags)
			.def_readwrite("pp", &DXUTD3D9DeviceSettings::pp)

		, class_<CD3D9EnumAdapterInfo>("CD3D9EnumAdapterInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumAdapterInfo::AdapterOrdinal)
			.def_readonly("szUniqueDescription", &CD3D9EnumAdapterInfo::szUniqueDescription)
			.def_readonly("displayModeList", &CD3D9EnumAdapterInfo::displayModeList)
			.def_readonly("deviceInfoList", &CD3D9EnumAdapterInfo::deviceInfoList)

		, class_<CGrowableArray<CD3D9EnumAdapterInfo *> >("CD3D9EnumAdapterInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumAdapterInfo *>::GetSize)

		, class_<CD3D9EnumDeviceInfo>("CD3D9EnumDeviceInfo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceInfo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceInfo::DeviceType)
			.def_readonly("deviceSettingsComboList", &CD3D9EnumDeviceInfo::deviceSettingsComboList)

		, class_<CGrowableArray<CD3D9EnumDeviceInfo *> >("CD3D9EnumDeviceInfoArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumDeviceInfo *>::GetSize)

		, class_<CGrowableArray<D3DFORMAT> >("D3DFORMATArray")
			.def("GetAt", &CGrowableArray<D3DFORMAT>::GetAt)
			.def("GetSize", &CGrowableArray<D3DFORMAT>::GetSize)

		, class_<CGrowableArray<D3DMULTISAMPLE_TYPE> >("D3DMULTISAMPLE_TYPEArray")
			.def("GetAt", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetAt)
			.def("GetSize", &CGrowableArray<D3DMULTISAMPLE_TYPE>::GetSize)

		, class_<CGrowableArray<DWORD> >("DWORDArray")
			.def("GetAt", (const DWORD & (CGrowableArray<DWORD>::*)(int))&CGrowableArray<DWORD>::GetAt) // ! forced convert to const ref
			.def("GetSize", &CGrowableArray<DWORD>::GetSize)

		, class_<CD3D9EnumDeviceSettingsCombo>("CD3D9EnumDeviceSettingsCombo")
			.def_readonly("AdapterOrdinal", &CD3D9EnumDeviceSettingsCombo::AdapterOrdinal)
			.def_readonly("DeviceType", &CD3D9EnumDeviceSettingsCombo::DeviceType)
			.def_readonly("AdapterFormat", &CD3D9EnumDeviceSettingsCombo::AdapterFormat)
			.def_readonly("BackBufferFormat", &CD3D9EnumDeviceSettingsCombo::BackBufferFormat)
			.def_readonly("Windowed", &CD3D9EnumDeviceSettingsCombo::Windowed)
			.def_readonly("depthStencilFormatList", &CD3D9EnumDeviceSettingsCombo::depthStencilFormatList)
			.def_readonly("multiSampleTypeList", &CD3D9EnumDeviceSettingsCombo::multiSampleTypeList)
			.def_readonly("multiSampleQualityList", &CD3D9EnumDeviceSettingsCombo::multiSampleQualityList)
			.def("IsDepthStencilMultiSampleConflict", &CD3D9EnumDeviceSettingsCombo::IsDepthStencilMultiSampleConflict)

		, class_<CGrowableArray<CD3D9EnumDeviceSettingsCombo *> >("CD3D9EnumDeviceSettingsComboArray")
			.def("GetAt", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetAt)
			.def("GetSize", &CGrowableArray<CD3D9EnumDeviceSettingsCombo *>::GetSize)

		, class_<CD3D9Enumeration>("CD3D9Enumeration")
			.def("GetAdapterInfoList", &CD3D9Enumeration::GetAdapterInfoList)
			.def("GetAdapterInfo", &CD3D9Enumeration::GetAdapterInfo)
			.def("GetDeviceInfo", &CD3D9Enumeration::GetDeviceInfo)
			.def("GetDeviceSettingsCombo", (CD3D9EnumDeviceSettingsCombo *(CD3D9Enumeration::*)(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL))&CD3D9Enumeration::GetDeviceSettingsCombo)

		, class_<my::DxutApp, CD3D9Enumeration>("DxutApp")
			.scope
			[
				def("DXUTD3DDeviceTypeToString", &my::DxutApp::DXUTD3DDeviceTypeToString),
				def("DXUTD3DFormatToString", &my::DxutApp::DXUTD3DFormatToString),
				def("DXUTMultisampleTypeToString", &my::DxutApp::DXUTMultisampleTypeToString),
				def("DXUTVertexProcessingTypeToString", &my::DxutApp::DXUTVertexProcessingTypeToString)
			]
			.def("GetD3D9BackBufferSurfaceDesc", &my::DxutApp::GetD3D9BackBufferSurfaceDesc)
			.def("GetD3D9DeviceSettings", &my::DxutApp::GetD3D9DeviceSettings)
			.def("ToggleFullScreen", &my::DxutApp::ToggleFullScreen)
			.def("ToggleREF", &my::DxutApp::ToggleREF)
			.property("SoftwareVP", &my::DxutApp::GetSoftwareVP, &my::DxutApp::SetSoftwareVP)
			.property("HardwareVP", &my::DxutApp::GetHardwareVP, &my::DxutApp::SetHardwareVP)
			.property("PureHardwareVP", &my::DxutApp::GetPureHardwareVP, &my::DxutApp::SetPureHardwareVP)
			.property("MixedVP", &my::DxutApp::GetMixedVP, &my::DxutApp::SetMixedVP)
			.def("ChangeDevice", &my::DxutApp::ChangeDevice)

		, class_<my::Timer, boost::shared_ptr<my::Timer> >("Timer")
			.def(constructor<float, float>())
			.def_readonly("Interval", &my::Timer::m_Interval)
			.def_readonly("RemainingTime", &my::Timer::m_RemainingTime)
			.def_readwrite("EventTimer", &my::Timer::m_EventTimer)

		//, class_<my::EffectParameterMap>("EffectParameterMap")
		//	.def("SetBool", &my::EffectParameterMap::SetBool)
		//	.def("SetFloat", &my::EffectParameterMap::SetFloat)
		//	.def("SetInt", &my::EffectParameterMap::SetInt)
		//	.def("SetVector", &my::EffectParameterMap::SetVector)
		//	.def("SetMatrix", &my::EffectParameterMap::SetMatrix)
		//	.def("SetString", &my::EffectParameterMap::SetString)
		//	.def("SetTexture", &my::EffectParameterMap::SetTexture)

		, class_<my::Material, boost::shared_ptr<my::Material> >("Material")
			.def(constructor<>())
			.def_readwrite("DiffuseTexture", &my::Material::m_DiffuseTexture)
			.def_readwrite("NormalTexture", &my::Material::m_NormalTexture)
			.def_readwrite("SpecularTexture", &my::Material::m_SpecularTexture)

		, class_<my::ResourceMgr, my::AsynchronousResourceMgr>("ResourceMgr")
			.def("LoadMaterialAsync", &my::ResourceMgr::LoadMaterialAsync)
			.def("LoadMaterial", &my::ResourceMgr::LoadMaterial)
			.def("SaveMaterial", &my::ResourceMgr::SaveMaterial)
			.def("LoadEmitterAsync", &my::ResourceMgr::LoadEmitterAsync)
			.def("LoadEmitter", &my::ResourceMgr::LoadEmitter)
			.def("SaveEmitter", &my::ResourceMgr::SaveEmitter)

		, def("res2texture", (my::BaseTexturePtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::BaseTexture>)
		, def("res2mesh", (my::OgreMeshPtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::OgreMesh>)
		, def("res2skeleton", (my::OgreSkeletonAnimationPtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::OgreSkeletonAnimation>)
		, def("res2effect", (my::EffectPtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::Effect>)
		, def("res2font", (my::FontPtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::Font>)
		, def("res2material", (my::MaterialPtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::Material>)
		, def("res2emitter", (my::EmitterPtr (*)(boost::shared_ptr<my::DeviceRelatedObjectBase> const &))&boost::dynamic_pointer_cast<my::Emitter>)

		, class_<my::BaseCamera, boost::shared_ptr<my::BaseCamera> >("BaseCamera")
			.def_readwrite("Position", &my::BaseCamera::m_Position)
			.def_readwrite("Orientation", &my::BaseCamera::m_Orientation)
			.def_readwrite("Nz", &my::BaseCamera::m_Nz)
			.def_readwrite("Fz", &my::BaseCamera::m_Fz)
			.def_readonly("View", &my::BaseCamera::m_View)
			.def_readonly("Proj", &my::BaseCamera::m_Proj)
			.def_readonly("ViewProj", &my::BaseCamera::m_ViewProj)
			.def_readonly("InverseViewProj", &my::BaseCamera::m_InverseViewProj)

		, class_<my::OrthoCamera, my::BaseCamera, boost::shared_ptr<my::OrthoCamera> >("OrthoCamera")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, float, float, float, float>())
			.def_readwrite("Width", &my::OrthoCamera::m_Width)
			.def_readwrite("Height", &my::OrthoCamera::m_Height)

		, class_<my::Camera, my::BaseCamera, boost::shared_ptr<my::Camera> >("Camera")
			.def(constructor<const my::Vector3 &, const my::Quaternion &, float, float, float, float>())
			.def_readwrite("Fov", &my::Camera::m_Fov)
			.def_readwrite("Aspect", &my::Camera::m_Aspect)
			.def_readwrite("EventAlign", &my::Camera::EventAlign)

		, class_<my::ModelViewerCamera, my::Camera, boost::shared_ptr<my::Camera> >("ModelViewerCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("LookAt", &my::ModelViewerCamera::m_LookAt)
			.def_readwrite("Rotation", &my::ModelViewerCamera::m_Rotation)
			.def_readwrite("Distance", &my::ModelViewerCamera::m_Distance)

		, class_<my::FirstPersonCamera, my::Camera, boost::shared_ptr<my::Camera> >("FirstPersonCamera")
			.def(constructor<float, float, float, float>())
			.def_readwrite("Rotation", &my::FirstPersonCamera::m_Rotation)

		, class_<Game, bases<my::DxutApp, my::ResourceMgr> >("Game")
			.def("AddTimer", &my::TimerMgr::AddTimer)
			.def("InsertTimer", &my::TimerMgr::InsertTimer)
			.def("RemoveTimer", &my::TimerMgr::RemoveTimer)
			.def("RemoveAllTimer", &my::TimerMgr::RemoveAllTimer)
			.property("DlgViewport", &my::DialogMgr::GetDlgViewport, &my::DialogMgr::SetDlgViewport)
			.def("InsertDlg", &my::DialogMgr::InsertDlg)
			.def("RemoveDlg", &my::DialogMgr::RemoveDlg)
			.def("RemoveAllDlg", &my::DialogMgr::RemoveAllDlg)
			.def("InsertEmitter", &my::EmitterMgr::InsertEmitter)
			.def("RemoveEmitter", &my::EmitterMgr::RemoveEmitter)
			.def("RemoveAllEmitter", &my::EmitterMgr::RemoveAllEmitter)
			.def_readonly("Console", &Game::m_Console)
			.def_readwrite("Camera", &Game::m_Camera)
			.def("ExecuteCode", &Game::ExecuteCode)

		, class_<MessagePanel, my::Control, boost::shared_ptr<my::Control> >("MessagePanel")
			.def_readonly("lbegin", &MessagePanel::m_lbegin)
			.def_readonly("lend", &MessagePanel::m_lend)
			.def_readonly("scrollbar", &MessagePanel::m_scrollbar)
			.def("AddLine", &MessagePanel::AddLine)
			.def("puts", &MessagePanel::puts)

		, class_<ConsoleEditBox, my::ImeEditBox, boost::shared_ptr<my::Control> >("ConsoleEditBox")
			.def_readwrite("EventKeyUp", &ConsoleEditBox::EventKeyUp)
			.def_readwrite("EventKeyDown", &ConsoleEditBox::EventKeyDown)

		, class_<Console, my::Dialog, boost::shared_ptr<my::Control> >("Console")
			.def_readonly("Edit", &Console::m_Edit)
			.def_readonly("Panel", &Console::m_Panel)
	];

	globals(L)["game"] = Game::getSingletonPtr();
}

#pragma once

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>

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
	{
	};

	template <>
	struct default_converter<std::wstring const&>
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
						// ! ControlEvent事件处理是容错的，当事件处理失败后，程序继续运行
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

	template <>
	struct default_converter<my::TimerEvent>
		: native_converter_base<my::TimerEvent>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::TimerEvent from(lua_State * L, int index)
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

		void to(lua_State * L, my::TimerEvent const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::TimerEvent const &>
		: default_converter<my::TimerEvent>
	{
	};
}

void Export2Lua(lua_State * L);
void ExportMath2Lua(lua_State * L);
void ExportResource2Lua(lua_State * L);
void ExportUI2Lua(lua_State * L);
void ExportEmitter2Lua(lua_State * L);
void ExportDevice2Lua(lua_State * L);

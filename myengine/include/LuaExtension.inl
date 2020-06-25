#pragma once

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
			std::string str = wstou8(value.c_str());
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
	struct default_converter<my::EventFunction>
		: native_converter_base<my::EventFunction>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::EventFunction from(lua_State * L, int index)
		{
			struct InternalExceptionHandler
			{
				luabind::object obj;
				InternalExceptionHandler(lua_State * L, int index)
					: obj(luabind::from_stack(L, index))
				{
				}
				void operator()(my::EventArg * arg)
				{
					_ASSERT(GetCurrentThreadId() == my::D3DContext::getSingleton().m_d3dThreadId);
					try
					{
						obj(arg);
					}
					catch (const luabind::error & e)
					{
						const char * p = lua_tostring(e.state(), -1);
						my::D3DContext::getSingleton().m_EventLog(p);
					}
				}
			};
			return InternalExceptionHandler(L, index);
		}

		void to(lua_State * L, my::EventFunction const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::EventFunction const &>
		: default_converter<my::EventFunction>
	{
	};
}

#pragma once

namespace luabind
{
	template <>
	struct default_converter<std::wstring>
		: native_converter_base<std::wstring>
	{
		static int compute_score(lua_State* L, int index);

		std::wstring from(lua_State* L, int index);

		void to(lua_State* L, std::wstring const& value);
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
		static int compute_score(lua_State * L, int index);

		my::ControlEvent from(lua_State * L, int index);

		void to(lua_State * L, my::ControlEvent const & e);
	};

	template <>
	struct default_converter<my::ControlEvent const &>
		: default_converter<my::ControlEvent>
	{
	};

	template <>
	struct default_converter<my::TimerEvent>
		: native_converter_base<my::TimerEvent>
	{
		static int compute_score(lua_State * L, int index);

		my::TimerEvent from(lua_State * L, int index);

		void to(lua_State * L, my::TimerEvent const & e);
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

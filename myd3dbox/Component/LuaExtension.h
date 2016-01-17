#pragma once

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class LuaContext
{
public:
	lua_State * m_State;

public:
	LuaContext(void);

	virtual ~LuaContext(void);

	void Init(void);

	int docall(int narg, int clear);

	int dostring(const char *s, const char *name);
};

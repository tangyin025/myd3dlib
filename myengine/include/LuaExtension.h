#pragma once

#include "mySingleton.h"

struct lua_State;

class LuaContext
	: public my::SingleInstance<LuaContext>
{
public:
	lua_State * m_State;

public:
	LuaContext(void);

	virtual ~LuaContext(void);

	void Init(void);

	void Shutdown(void);

	int docall(int narg, int clear);

	int dostring(const char *s, const char *name);

	int dogcstep(int step);
};

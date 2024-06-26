// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "mySingleton.h"
#include "myThread.h"

struct lua_State;

class LuaContext
	: public my::SingletonInstance<LuaContext>
{
public:
	lua_State * m_State;

	my::CriticalSection m_StateSec;

public:
	LuaContext(void);

	virtual ~LuaContext(void);

	void Init(void);

	void Shutdown(void);

	int docall(int narg, int clear);

	int dostring(const char * s, const char * name);

	int dogc(int what, int data);
};

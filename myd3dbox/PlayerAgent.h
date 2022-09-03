#pragma once

#include "Component.h"

class PlayerAgent : public Component
{
public:
	PlayerAgent(const char* Name)
		: Component(Name)
	{
	}

	virtual ~PlayerAgent(void)
	{
		_ASSERT(!IsRequested());
	}

	virtual DWORD GetComponentType(void) const
	{
		return ComponentTypeScript;
	}

	virtual void RequestResource(void)
	{
		Component::RequestResource();
	}

	virtual void ReleaseResource(void)
	{
		Component::ReleaseResource();
	}

	virtual void Update(float fElapsedTime)
	{
		;
	}

	virtual void OnPxThreadSubstep(float dtime)
	{
		;
	}
};


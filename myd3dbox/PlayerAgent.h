#pragma once

#include "Component.h"

class PlayerAgent : public Component
{
protected:

public:
	PlayerAgent(const char* Name)
		: Component(Name)
	{
	}

	virtual ~PlayerAgent(void);

	virtual DWORD GetComponentType(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnPxThreadSubstep(float dtime);
};


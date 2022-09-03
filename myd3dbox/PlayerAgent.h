#pragma once

#include "Component.h"

class Controller;

class Steering;

class Animator;

class PlayerAgent : public Component
{
public:
	Controller* m_Controller;

	Steering* m_Steering;

	Animator* m_Animator;

	my::Vector3 m_MoveDir;

	float m_VerticalSpeed;

	bool m_Jumping;

	PlayerAgent(const char* Name)
		: Component(Name)
		, m_MoveDir(0, 0, 0)
		, m_VerticalSpeed(0)
		, m_Jumping(true)
	{
	}

	virtual ~PlayerAgent(void);

	virtual DWORD GetComponentType(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnPxThreadSubstep(float dtime);
};


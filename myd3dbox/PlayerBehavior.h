// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "Component.h"
#include "Animator.h"

class Action;

class ActionTbl : public my::Singleton<ActionTbl>
{
public:
	boost::shared_ptr<Action> Jump;

	boost::shared_ptr<Action> Climb;

	ActionTbl(void);
};

class NodeRunBlendList : public AnimationNodeBlendList
{
public:
	NodeRunBlendList(const char* Name);

	virtual void Tick(float fElapsedTime, float fTotalWeight);
};

class Controller;

class Steering;

class Animator;

class PlayerBehavior : public Component
{
public:
	Controller* m_Controller;

	Steering* m_Steering;

	Animator* m_Animator;

	my::Vector3 m_MoveDir;

	my::Vector3 m_JumpVel;

	my::Vector3 m_ClimbLerp;

	float m_VerticalSpeed;

	float m_Suspending;

	float m_Jumping;

	std::vector<MeshComponentPtr> m_Meshes;

	PlayerBehavior(const char* Name)
		: Component(Name)
		, m_MoveDir(0, 0, 0)
		, m_JumpVel(0, 0, 0)
		, m_ClimbLerp(0, 1, 0)
		, m_VerticalSpeed(0)
		, m_Suspending(0.0f)
		, m_Jumping(0.0f)
	{
	}

	virtual ~PlayerBehavior(void);

	enum { TypeID = ComponentTypeScript };

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnPxThreadSubstep(float dtime);

	virtual void OnPxThreadShapeHit(my::EventArg* arg);
};


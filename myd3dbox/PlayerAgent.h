#pragma once

#include "Component.h"

class Action;

class ActionTbl : public my::Singleton<ActionTbl>
{
public:
	boost::shared_ptr<Action> Jump;

	boost::shared_ptr<Action> Climb;

	ActionTbl(void);
};

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

	float m_Suspending;

	std::vector<MeshComponentPtr> m_Meshes;

	my::OgreSkeletonAnimationPtr m_Skel;

	unsigned int m_LastMoveFlags;

	float m_Submergence;

	PlayerAgent(const char* Name)
		: Component(Name)
		, m_MoveDir(0, 0, 0)
		, m_VerticalSpeed(0)
		, m_Suspending(0.0f)
		, m_LastMoveFlags(0)
		, m_Submergence(0)
	{
	}

	virtual ~PlayerAgent(void);

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


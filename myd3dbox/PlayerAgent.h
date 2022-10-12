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

	my::Vector3 m_ClimbDir;

	float m_VerticalSpeed;

	float m_Suspending;

	std::vector<MeshComponentPtr> m_Meshes;

	my::OgreSkeletonAnimationPtr m_Skel;

	Component* m_DownTouchedCmp;

	my::Vector3 m_DownTouchedPos;

	my::Vector3 m_DownTouchedNormal;

	Component* m_SideTouchedCmp;

	my::Vector3 m_SideTouchedPos;

	my::Vector3 m_SideTouchedNormal;

	Component* m_AnchoredCmp;

	PlayerAgent(const char* Name)
		: Component(Name)
		, m_MoveDir(0, 0, 0)
		, m_ClimbDir(0, 0, 0)
		, m_VerticalSpeed(0)
		, m_Suspending(0.0f)
		, m_DownTouchedCmp(NULL)
		, m_DownTouchedPos(0, 0, 0)
		, m_DownTouchedNormal(1, 0, 0)
		, m_SideTouchedCmp(NULL)
		, m_SideTouchedPos(0, 0, 0)
		, m_SideTouchedNormal(1, 0, 0)
		, m_AnchoredCmp(NULL)
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


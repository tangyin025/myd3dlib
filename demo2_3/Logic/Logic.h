#pragma once

#include "Character.h"
#include "../Component/MeshComponent.h"

class Logic
{
protected:
	enum LogicState
	{
		LogicStateMain,
	};

	LogicState m_State;

	physx_ptr<PxRigidActor> m_StaticSceneActor;

	CharacterPtr m_LocalPlayer;

public:
	Logic(void)
		: m_State(LogicStateMain)
		, m_LocalPlayer(new Character)
	{
	}

	virtual ~Logic(void)
	{
	}

	void Create(void);

	void Update(float fElapsedTime);

	void Destroy(void);

	void ShiftState(LogicState State);

	void OnEnterState(void);

	void OnLeaveState(void);

	void OnMouseMove(my::InputEventArg * arg);

	void OnKeyDown(my::InputEventArg * arg);
};

typedef boost::shared_ptr<Logic> LogicPtr;

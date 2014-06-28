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

	my::Timer m_FixedTickTimer;

	physx_ptr<PxRigidActor> m_StaticSceneActor;

	CharacterPtr m_LocalPlayer;

public:
	Logic(void);

	virtual ~Logic(void);

	void Create(void);

	void Update(float fElapsedTime);

	void OnFixedTick(float fElapsedTime);

	void Destroy(void);

	void ShiftState(LogicState State);

	void OnEnterState(void);

	void OnLeaveState(void);

	void OnMouseMove(my::InputEventArg * arg);

	void OnMouseBtnDown(my::InputEventArg * arg);

	void OnMouseBtnUp(my::InputEventArg * arg);

	void OnKeyDown(my::InputEventArg * arg);

	void OnKeyUp(my::InputEventArg * arg);
};

typedef boost::shared_ptr<Logic> LogicPtr;

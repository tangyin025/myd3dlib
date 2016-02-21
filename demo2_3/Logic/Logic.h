#pragma once

#include "Character.h"

class Logic
{
protected:
	my::Timer m_FixedTickTimer;

	PhysXPtr<PxRigidActor> m_StaticSceneActor;

	CharacterPtr m_LocalPlayer;

public:
	Logic(void);

	virtual ~Logic(void);

	void Create(void);

	void Update(float fElapsedTime);

	void OnFixedTick(float fElapsedTime);

	void Destroy(void);

	void OnMouseMove(my::InputEventArg * arg);

	void OnMouseBtnDown(my::InputEventArg * arg);

	void OnMouseBtnUp(my::InputEventArg * arg);

	void OnKeyDown(my::InputEventArg * arg);

	void OnKeyUp(my::InputEventArg * arg);

	void OnJoystickAxisMove(my::InputEventArg * arg);

	void OnJoystickPovMove(my::InputEventArg * arg);

	void OnJoystickBtnDown(my::InputEventArg * arg);

	void OnJoystickBtnUp(my::InputEventArg * arg);
};

typedef boost::shared_ptr<Logic> LogicPtr;

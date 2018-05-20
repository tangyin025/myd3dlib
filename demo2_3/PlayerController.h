#pragma once

#include "../myd3dbox/Component/Controller.h"

class PlayerController
	: public CharacterController
	, public my::SingleInstance<PlayerController>
{
public:
	my::Vector3 m_LookAngle;

	my::Vector2 m_MoveAxis;

protected:
	PlayerController(void);

public:
	PlayerController(Actor * actor);

	~PlayerController(void);

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CharacterController);
	}

	void Init(void);

	void Destroy(void);

	virtual void Update(float fElapsedTime);

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

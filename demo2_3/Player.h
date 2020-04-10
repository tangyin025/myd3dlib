#pragma once

#include "Character.h"

class Player
	: public Character
	, public my::SingleInstance<Player>
{
public:
	my::Vector3 m_LookAngle;

	float m_LookDist;

	CPoint m_MoveAxis;

protected:
	Player(void);

public:
	Player(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius, float ContactOffset);

	~Player(void);

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Character);
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

	void OnWindowActivate(bool bActivated);
};

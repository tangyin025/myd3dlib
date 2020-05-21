#pragma once

#include "Character.h"

class Player
	: public Character
	, public my::SingleInstance<Player>
{
public:
	my::Vector3 m_LookAngle;

	my::Matrix4 m_LookMatrix;

	float m_LookDist;

	CPoint m_MoveAxis;

	my::EventFunction m_EventMouseMove;

	my::EventFunction m_EventMouseBtnDown;

	my::EventFunction m_EventMouseBtnUp;

	my::EventFunction m_EventKeyDown;

	my::EventFunction m_EventKeyUp;

	my::EventFunction m_EventJoystickAxisMove;

	my::EventFunction m_EventJoystickPovMove;

	my::EventFunction m_EventJoystickBtnDown;

	my::EventFunction m_EventJoystickBtnUp;

protected:
	Player(void);

public:
	Player(const char * Name, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius, float ContactOffset, unsigned int filterWord0);

	~Player(void);

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Character);
	}

	void Init(void);

	void Destroy(void);

	virtual void Update(float fElapsedTime);

	void OnMouseMove(my::EventArg * arg);

	void OnMouseBtnDown(my::EventArg * arg);

	void OnMouseBtnUp(my::EventArg * arg);

	void OnKeyDown(my::EventArg * arg);

	void OnKeyUp(my::EventArg * arg);

	void OnJoystickAxisMove(my::EventArg * arg);

	void OnJoystickPovMove(my::EventArg * arg);

	void OnJoystickBtnDown(my::EventArg * arg);

	void OnJoystickBtnUp(my::EventArg * arg);

	void OnWindowActivate(bool bActivated);
};

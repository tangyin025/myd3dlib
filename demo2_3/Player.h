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

	virtual void Update(float fElapsedTime);
};

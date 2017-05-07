#pragma once

#include "Actor.h"

class Character : public Actor
{
public:
	Character(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: Actor(ComponentTypeCharacter, Position, Rotation, Scale, aabb)
	{
	}

	Character(void)
		: Actor(ComponentTypeCharacter, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1))
	{
	}

	virtual ~Character(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}
};

typedef boost::shared_ptr<Character> CharacterPtr;

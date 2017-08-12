#pragma once

#include "Actor.h"

class Character
	: public Actor
	, public physx::PxUserControllerHitReport
	, public physx::PxControllerBehaviorCallback
{
public:
	my::Vector3 m_Acceleration;

	my::Vector3 m_Velocity;

	PhysXPtr<physx::PxMaterial> m_PxMaterial;

	PhysXPtr<physx::PxController> m_PxController;

public:
	Character(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: Actor(ComponentTypeCharacter, Position, Rotation, Scale, aabb)
		, m_Acceleration(0,0,0)
		, m_Velocity(0,0,0)
	{
	}

	Character(void)
		: Actor(ComponentTypeCharacter, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1))
		, m_Acceleration(0,0,0)
		, m_Velocity(0,0,0)
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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(PhysXSceneContext * scene);

	virtual void OnLeavePxScene(PhysXSceneContext * scene);

	virtual void Update(float fElapsedTime);

	virtual void OnPxThreadSubstep(float dtime);

	virtual void onShapeHit(const physx::PxControllerShapeHit& hit);

	virtual void onControllerHit(const physx::PxControllersHit& hit);

	virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle);
};

typedef boost::shared_ptr<Character> CharacterPtr;

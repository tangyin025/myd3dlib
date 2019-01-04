#pragma once

#include "Actor.h"

class Character
	: public Actor
	, public physx::PxUserControllerHitReport
	, public physx::PxControllerBehaviorCallback
{
public:
	float m_Height;

	float m_Radius;

	PhysXPtr<physx::PxMaterial> m_PxMaterial;

	PhysXPtr<physx::PxController> m_PxController;

	my::Vector3 m_Velocity;

	float m_Orientation;

	float m_TargetSpeed;

	float m_TargetOrientation;

	float m_PotentialEnergy;

	float m_Resistance;

protected:
	Character(void)
		: m_Height(1.0f)
		, m_Radius(1.0f)
		, m_Velocity(0, 0, 0)
		, m_Orientation(0)
		, m_TargetSpeed(0)
		, m_TargetOrientation(0)
		, m_PotentialEnergy(100)
		, m_Resistance(50)
	{
	}

public:
	Character(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius)
		: Actor(Position, Rotation, Scale, aabb)
		, m_Height(Height)
		, m_Radius(Radius)
		, m_Velocity(0,0,0)
		, m_Orientation(0)
		, m_TargetSpeed(0)
		, m_TargetOrientation(0)
		, m_PotentialEnergy(100)
		, m_Resistance(50)
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

	virtual void OnPxTransformChanged(const physx::PxTransform & trans);

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

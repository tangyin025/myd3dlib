#pragma once

#include "Actor.h"
#include "mySpline.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

class Character;

struct ShapeHitEventArg : public ActorEventArg
{
public:
	my::Vector3 worldPos;		//!< Contact position in world space
	my::Vector3 worldNormal;	//!< Contact normal in world space
	my::Vector3 dir;			//!< Motion direction
	float length;				//!< Motion length
	Component* cmp;			//!< Touched shape
	Actor* other;				//!< Touched actor
	unsigned int triangleIndex;	//!< touched triangle index (only for meshes/heightfields)

	ShapeHitEventArg::ShapeHitEventArg(Character* _self);
};

class Character
	: public Actor
	, public physx::PxUserControllerHitReport
	, public physx::PxControllerBehaviorCallback
{
public:
	float m_Height;

	float m_Radius;

	float m_ContactOffset;

	unsigned int m_filterWord0;

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxController> m_PxController;

	my::EventFunction m_EventShapeHit;

protected:
	Character(void)
		: m_Height(1.0f)
		, m_Radius(1.0f)
		, m_ContactOffset(0.1f)
		, m_filterWord0(0)
	{
	}

public:
	Character(const char * Name, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius, float ContactOffset, unsigned int filterWord0)
		: Actor(Name, Position, Rotation, Scale, aabb)
		, m_Height(Height)
		, m_Radius(Radius)
		, m_ContactOffset(ContactOffset)
		, m_filterWord0(filterWord0)
	{
	}

	virtual ~Character(void);

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

	virtual void EnterPhysxScene(PhysxScene * scene);

	virtual void LeavePhysxScene(PhysxScene * scene);

	virtual void OnPxTransformChanged(const physx::PxTransform & trans);

	virtual void Update(float fElapsedTime);

	void SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	unsigned int Move(const my::Vector3 & disp, float minDist, float elapsedTime);

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void OnPxThreadSubstep(float dtime);

	virtual void onShapeHit(const physx::PxControllerShapeHit& hit);

	virtual void onControllerHit(const physx::PxControllersHit& hit);

	virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle);
};

typedef boost::shared_ptr<Character> CharacterPtr;

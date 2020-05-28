#pragma once

#include "Actor.h"
#include "mySpline.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

class PoseTrack : public boost::intrusive_ref_counter<PoseTrack>
{
public:
	float m_Length;

	my::Spline m_InterpolateX;

	my::Spline m_InterpolateY;

	my::Spline m_InterpolateZ;

	PoseTrack(float Length)
		: m_Length(Length)
	{
	}
};

class PoseTrackInst
{
public:
	boost::intrusive_ptr<PoseTrack> m_Template;

	my::Vector3 m_StartPos;

	float m_Time;

	PoseTrackInst(PoseTrack * Template, const my::Vector3 & StartPos)
		: m_Template(Template)
		, m_StartPos(StartPos)
		, m_Time(0)
	{
	}

	my::Vector3 GetPos(void) const;
};

typedef boost::shared_ptr<PoseTrackInst> PoseTrackInstPtr;

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

	my::Vector3 m_Velocity;

	float m_Orientation;

	float m_TargetSpeed;

	float m_TargetOrientation;

	float m_SteeringLinear;

	float m_SteeringAngular;

	float m_Resistance;

	typedef std::vector<PoseTrackInstPtr> PoseTrackInstPtrList;

	PoseTrackInstPtrList m_PoseTracks;

protected:
	Character(void)
		: m_Height(1.0f)
		, m_Radius(1.0f)
		, m_ContactOffset(0.1f)
		, m_filterWord0(0)
		, m_Velocity(0, 0, 0)
		, m_Orientation(0)
		, m_TargetSpeed(0)
		, m_TargetOrientation(0)
		, m_SteeringLinear(100)
		, m_SteeringAngular(D3DX_PI * 3)
		, m_Resistance(50)
	{
	}

public:
	Character(const char * Name, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius, float ContactOffset, unsigned int filterWord0)
		: Actor(Name, Position, Rotation, Scale, aabb)
		, m_Height(Height)
		, m_Radius(Radius)
		, m_ContactOffset(ContactOffset)
		, m_filterWord0(filterWord0)
		, m_Velocity(0,0,0)
		, m_Orientation(0)
		, m_TargetSpeed(0)
		, m_TargetOrientation(0)
		, m_SteeringLinear(100)
		, m_SteeringAngular(D3DX_PI * 3)
		, m_Resistance(50)
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

	virtual void EnterPhysxScene(PhysxSceneContext * scene);

	virtual void LeavePhysxScene(PhysxSceneContext * scene);

	virtual void OnPxTransformChanged(const physx::PxTransform & trans);

	virtual void Update(float fElapsedTime);

	void SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	void AddPoseTrack(PoseTrack * track);

	void OnPxThreadSubstep(float dtime);

	virtual void onShapeHit(const physx::PxControllerShapeHit& hit);

	virtual void onControllerHit(const physx::PxControllersHit& hit);

	virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle);
};

typedef boost::shared_ptr<Character> CharacterPtr;

#pragma once

#include "Component.h"

class Character
	: public Component
	, public physx::PxUserControllerHitReport
	, public physx::PxControllerBehaviorCallback
{
public:
	float m_Height;

	float m_Radius;

	float m_ContactOffset;

	unsigned int m_filterWord0;

	bool m_muted;

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxController> m_PxController;

protected:
	Character(void)
		: m_Height(1.0f)
		, m_Radius(1.0f)
		, m_ContactOffset(0.1f)
		, m_filterWord0(0)
		, m_muted(false)
	{
	}

public:
	Character(const char * Name, float Height, float Radius, float ContactOffset, unsigned int filterWord0)
		: Component(ComponentTypeCharacter, Name)
		, m_Height(Height)
		, m_Radius(Radius)
		, m_ContactOffset(ContactOffset)
		, m_filterWord0(filterWord0)
		, m_muted(false)
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

	virtual void OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam);

	virtual void OnSetPose(void);

	virtual void Update(float fElapsedTime);

	unsigned int Move(const my::Vector3 & disp, float minDist, float elapsedTime);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void OnPxThreadSubstep(float dtime);

	virtual void onShapeHit(const physx::PxControllerShapeHit& hit);

	virtual void onControllerHit(const physx::PxControllersHit& hit);

	virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle);
};

typedef boost::shared_ptr<Character> CharacterPtr;

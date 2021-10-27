#pragma once

#include "Component.h"

class Controller
	: public Component
	, public physx::PxUserControllerHitReport
	, public physx::PxControllerBehaviorCallback
{
public:
	float m_Height;

	float m_Radius;

	float m_ContactOffset;

	unsigned int m_filterWord0;

	bool m_PxControllerMoveMuted;

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxController> m_PxController;

protected:
	Controller(void)
		: m_Height(1.0f)
		, m_Radius(1.0f)
		, m_ContactOffset(0.1f)
		, m_filterWord0(0)
		, m_PxControllerMoveMuted(false)
	{
	}

public:
	Controller(const char * Name, float Height, float Radius, float ContactOffset, unsigned int filterWord0)
		: Component(Name)
		, m_Height(Height)
		, m_Radius(Radius)
		, m_ContactOffset(ContactOffset)
		, m_filterWord0(filterWord0)
		, m_PxControllerMoveMuted(false)
	{
	}

	virtual ~Controller(void);

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

	virtual ComponentType GetComponentType(void) const
	{
		return ComponentTypeController;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void EnterPhysxScene(PhysxScene * scene);

	virtual void LeavePhysxScene(PhysxScene * scene);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void SetPxPoseOrbyPxThread(const physx::PxTransform & pose);

	virtual void Update(float fElapsedTime);

	unsigned int Move(const my::Vector3 & disp, float minDist, float elapsedTime);

	virtual void onShapeHit(const physx::PxControllerShapeHit & hit);

	virtual void onControllerHit(const physx::PxControllersHit & hit);

	virtual void onObstacleHit(const physx::PxControllerObstacleHit & hit);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape & shape, const physx::PxActor & actor);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController & controller);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle & obstacle);
};

typedef boost::shared_ptr<Controller> ControllerPtr;

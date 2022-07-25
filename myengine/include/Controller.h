#pragma once

#include "Component.h"

class Controller
	: public Component
	, public physx::PxUserControllerHitReport
	, public physx::PxControllerBehaviorCallback
{
public:
	enum { TypeID = ComponentTypeController };

	physx::PxCapsuleControllerDesc m_desc;

	unsigned int m_DescSimulationFilterWord0;

	unsigned int m_DescQueryFilterWord0;

	bool m_PxControllerMoveMuted;

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxController> m_PxController;

protected:
	Controller(void)
		: m_DescSimulationFilterWord0(0)
		, m_DescQueryFilterWord0(0)
		, m_PxControllerMoveMuted(false)
	{
	}

public:
	Controller(const char * Name, float Height, float Radius, float ContactOffset, float StepOffset)
		: Component(Name)
		, m_DescSimulationFilterWord0(0)
		, m_DescQueryFilterWord0(0)
		, m_PxControllerMoveMuted(false)
	{
		m_desc.height = Height;
		m_desc.radius = Radius;
		m_desc.contactOffset = ContactOffset;
		m_desc.stepOffset = StepOffset;
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot);

	virtual void Update(float fElapsedTime);

	unsigned int Move(const my::Vector3 & disp, float minDist, float elapsedTime, unsigned int filterWord0);

	void SetHeight(float Height);

	float GetHeight(void) const;

	void SetRadius(float Radius);

	float GetRadius(void) const;

	void SetStepOffset(float StepOffset);

	float GetStepOffset(void) const;

	void SetContactOffset(float ContactOffset);

	float GetContactOffset(void) const;

	void SetSimulationFilterWord0(unsigned int filterWord0);

	unsigned int GetSimulationFilterWord0(void) const;

	void SetQueryFilterWord0(unsigned int filterWord0);

	unsigned int GetQueryFilterWord0(void) const;

	void SetUpDirection(const my::Vector3 & Up);

	my::Vector3 GetUpDirection(void) const;

	void SetPosition(const my::Vector3 & Pos);

	my::Vector3 GetPosition(void) const;

	my::Vector3 GetFootOffset(void) const;

	void SetFootPosition(const my::Vector3 & Pos);

	my::Vector3 GetFootPosition(void) const;

	const my::Vector3 & GetContactNormalDownPass(void) const;

	const my::Vector3 & GetContactNormalSidePass(void) const;

	Component* GetTouchedComponent(void) const;

	my::Vector3 & GetTouchedPosWorld(void) const;

	my::Vector3 & GetTouchedPosLocal(void) const;

	virtual void onShapeHit(const physx::PxControllerShapeHit & hit);

	virtual void onControllerHit(const physx::PxControllersHit & hit);

	virtual void onObstacleHit(const physx::PxControllerObstacleHit & hit);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape & shape, const physx::PxActor & actor);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController & controller);

	virtual physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle & obstacle);
};

typedef boost::shared_ptr<Controller> ControllerPtr;

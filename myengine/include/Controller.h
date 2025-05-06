// Copyright (c) 2011-2024 tangyin025
// License: MIT
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

	unsigned int m_DescShapeFlags;

#ifdef _DEBUG
	volatile bool m_PxControllerMoveMuted;
#endif

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxController> m_PxController;

protected:
	Controller(void)
		: m_DescSimulationFilterWord0(0)
		, m_DescQueryFilterWord0(0)
		, m_DescShapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE)
#ifdef _DEBUG
		, m_PxControllerMoveMuted(false)
#endif
	{
		m_desc.reportCallback = this;
		m_desc.behaviorCallback = this;
		m_desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
		m_desc.userData = this;
	}

public:
	Controller(const char * Name, float Height, float Radius, float ContactOffset, float StepOffset, float SlopeLimit)
		: Component(Name)
		, m_DescSimulationFilterWord0(0)
		, m_DescQueryFilterWord0(0)
		, m_DescShapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE)
#ifdef _DEBUG
		, m_PxControllerMoveMuted(false)
#endif
	{
		m_desc.height = Height;
		m_desc.radius = Radius;
		m_desc.slopeLimit = SlopeLimit;
		m_desc.contactOffset = ContactOffset;
		m_desc.stepOffset = StepOffset;
		m_desc.reportCallback = this;
		m_desc.behaviorCallback = this;
		m_desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
		m_desc.userData = this;
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

	virtual void SetSimulationFilterWord0(unsigned int filterWord0);

	virtual unsigned int GetSimulationFilterWord0(void) const;

	virtual void SetQueryFilterWord0(unsigned int filterWord0);

	virtual unsigned int GetQueryFilterWord0(void) const;

	virtual void SetShapeFlags(unsigned int Flags);

	virtual unsigned int GetShapeFlags(void) const;

	unsigned int Move(const my::Vector3 & disp, float minDist, float elapsedTime, unsigned int filterWord0);

	void SetHeight(float Height);

	float GetHeight(void) const;

	void SetRadius(float Radius);

	float GetRadius(void) const;

	void SetStepOffset(float StepOffset);

	float GetStepOffset(void) const;

	void SetContactOffset(float ContactOffset);

	float GetContactOffset(void) const;

	void SetSlopeLimit(float SlopeLimit);

	float GetSlopeLimit(void) const;

	void SetUpDirection(const my::Vector3 & Up);

	const my::Vector3 & GetUpDirection(void) const;

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

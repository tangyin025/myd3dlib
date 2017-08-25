#include "StdAfx.h"
#include "Character.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include "World.h"
#include "PhysXContext.h"

using namespace my;

BOOST_CLASS_EXPORT(Character)

template<>
void Character::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Actor);
}

template<>
void Character::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Actor);
}

void Character::RequestResource(void)
{
	Actor::RequestResource();
}

void Character::ReleaseResource(void)
{
	Actor::ReleaseResource();
}

void Character::OnEnterPxScene(PhysXSceneContext * scene)
{
	Actor::OnEnterPxScene(scene);

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f));

	Vector3 Offset = GetLevel()->GetOffset();

	physx::PxCapsuleControllerDesc desc;
	desc.height = 2.0f;
	desc.radius = 1.0f;
	desc.position = physx::PxExtendedVec3(m_Position.x + Offset.x, m_Position.y, m_Position.z + Offset.z);
	desc.material = m_PxMaterial.get();
	desc.reportCallback = this;
	desc.behaviorCallback = this;
	desc.userData = this;
	m_PxController.reset(scene->m_ControllerMgr->createController(desc));

	physx::PxActor * actor = m_PxController->getActor();
	actor->userData = this;

	scene->m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
}

void Character::OnLeavePxScene(PhysXSceneContext * scene)
{
	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, _1));

	m_PxController.reset();

	m_PxMaterial.reset();

	Actor::OnLeavePxScene(scene);
}

void Character::Update(float fElapsedTime)
{
	Actor::Update(fElapsedTime);
}

void Character::UpdateWorld(void)
{
	m_World = Matrix4::Compose(m_Scale, Quaternion::RotationYawPitchRoll(m_Orientation, 0, 0), GetWorldPosition());

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateWorld();
	}
}

void Character::OnPxThreadSubstep(float dtime)
{
	if (m_PxController)
	{
		m_Velocity += m_Acceleration * dtime;
		float magnitude = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
		if (magnitude > m_MaxVelocity)
		{
			m_Velocity.x = m_Velocity.x / magnitude * m_MaxVelocity;
			m_Velocity.z = m_Velocity.z / magnitude * m_MaxVelocity;
		}
		else if (magnitude > 0 && m_Acceleration.x == 0 && m_Acceleration.z == 0)
		{
			float step = m_Resistance * dtime;
			if (magnitude < step)
			{
				m_Velocity.x = 0;
				m_Velocity.z = 0;
			}
			else
			{
				m_Velocity.x = m_Velocity.x - m_Velocity.x / magnitude * step;
				m_Velocity.z = m_Velocity.z - m_Velocity.z / magnitude * step;
			}
		}
		physx::PxControllerCollisionFlags flags = m_PxController->move((physx::PxVec3&)m_Velocity * dtime, 0.001f, dtime, physx::PxControllerFilters());
	}
}

void Character::onShapeHit(const physx::PxControllerShapeHit& hit)
{

}

void Character::onControllerHit(const physx::PxControllersHit& hit)
{

}

void Character::onObstacleHit(const physx::PxControllerObstacleHit& hit)
{

}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
{
	return physx::PxControllerBehaviorFlags();
}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxController& controller)
{
	return physx::PxControllerBehaviorFlags();
}

physx::PxControllerBehaviorFlags Character::getBehaviorFlags(const physx::PxObstacle& obstacle)
{
	return physx::PxControllerBehaviorFlags();
}

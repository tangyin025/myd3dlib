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

	Octree * level = dynamic_cast<Octree *>(m_Node->GetTopNode());
	CPoint level_id = level->m_World->GetLevelId(level);
	Vector3 Offset(
		(level_id.x - level->m_World->m_LevelId.x) * WorldL::LEVEL_SIZE, 0,
		(level_id.y - level->m_World->m_LevelId.y) * WorldL::LEVEL_SIZE);

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f));

	physx::PxCapsuleControllerDesc desc;
	desc.height = 2.0f;
	desc.radius = 1.0f;
	desc.position = physx::PxExtendedVec3(m_Position.x + Offset.x, m_Position.y, m_Position.z + Offset.z);
	desc.material = m_PxMaterial.get();
	desc.reportCallback = this;
	desc.behaviorCallback = this;
	m_Controller.reset(scene->m_ControllerMgr->createController(desc));

	scene->m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
}

void Character::OnLeavePxScene(PhysXSceneContext * scene)
{
	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, _1));

	m_Controller.reset();

	m_PxMaterial.reset();

	Actor::OnLeavePxScene(scene);
}

void Character::Update(float fElapsedTime)
{
	Actor::Update(fElapsedTime);

	m_Velocity += m_Acceleration * fElapsedTime;

	m_Controller->move((physx::PxVec3&)m_Velocity * fElapsedTime, 0.001f, fElapsedTime, physx::PxControllerFilters());

	m_Position = (Vector3&)toVec3(m_Controller->getPosition());

	UpdateWorld(Matrix4::Identity());
}

void Character::OnPxThreadSubstep(float dtime)
{

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

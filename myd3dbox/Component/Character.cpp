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

	physx::PxCapsuleControllerDesc desc;
	desc.height = 2.0f;
	desc.position = physx::PxExtendedVec3(m_Position.x + Offset.x, m_Position.y, m_Position.z + Offset.z);
}

void Character::OnLeavePxScene(PhysXSceneContext * scene)
{
	Actor::OnLeavePxScene(scene);
}

void Character::Update(float fElapsedTime)
{
	Actor::Update(fElapsedTime);
}

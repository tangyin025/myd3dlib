#include "stdafx.h"
#include "Actor.h"
#include "Terrain.h"
#include "Animator.h"
#include "PhysXContext.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Actor)

template<>
void Actor::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Animator);
	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
	physx::PxActorType::Enum ActorType = m_PxActor ? m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT;
	ar << BOOST_SERIALIZATION_NVP(ActorType);

	if (m_PxActor)
	{
		PhysXPtr<physx::PxCollection> collection(PxCreateCollection());
		m_PxActor->getType();
		collection->add(*m_PxActor, physx::PxConcreteType::eRIGID_STATIC << 24 | 0);
		for (unsigned int i = 0; i < m_Cmps.size(); i++)
		{
			if (m_Cmps[i]->m_PxShape)
			{
				collection->add(*m_Cmps[i]->m_PxMaterial, physx::PxConcreteType::eMATERIAL << 24 | i);
				collection->add(*m_Cmps[i]->m_PxShape, physx::PxConcreteType::eSHAPE << 24 | i);
				if (m_Cmps[i]->m_Type == Component::ComponentTypeTerrain)
				{
					collection->add(*boost::dynamic_pointer_cast<Terrain>(m_Cmps[i])->m_PxHeightField, physx::PxConcreteType::eHEIGHTFIELD << 24 | i);
				}
			}
		}
		physx::PxSerialization::complete(*collection, *PhysXContext::getSingleton().m_Registry, PhysXContext::getSingleton().m_Collection.get());
		physx::PxDefaultMemoryOutputStream ostr;
		physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *PhysXContext::getSingleton().m_Registry, PhysXContext::getSingleton().m_Collection.get());
		unsigned int PxActorSize = ostr.getSize();
		ar << BOOST_SERIALIZATION_NVP(PxActorSize);
		ar << boost::serialization::make_nvp("m_PxActor", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	}
}

template<>
void Actor::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Animator);
	if (m_Animator)
	{
		m_Animator->m_Cmp = this;
	}
	ar >> BOOST_SERIALIZATION_NVP(m_Cmps);
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for(; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Actor = this;
	}
	physx::PxActorType::Enum ActorType;
	ar >> BOOST_SERIALIZATION_NVP(ActorType);

	if (ActorType == physx::PxActorType::eRIGID_STATIC || ActorType == physx::PxActorType::eRIGID_DYNAMIC)
	{
		unsigned int PxActorSize;
		ar >> BOOST_SERIALIZATION_NVP(PxActorSize);
		m_SerializeBuff.reset((unsigned char *)_aligned_malloc(PxActorSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
		ar >> boost::serialization::make_nvp("m_PxActor", boost::serialization::binary_object(m_SerializeBuff.get(), PxActorSize));
		PhysXPtr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *PhysXContext::getSingleton().m_Registry, PhysXContext::getSingleton().m_Collection.get()));
		const unsigned int numObjs = collection->getNbObjects();
		for (unsigned int i = 0; i < numObjs; i++)
		{
			physx::PxBase * obj = &collection->getObject(i);
			physx::PxSerialObjectId id = collection->getId(*obj);
			physx::PxConcreteType::Enum type = physx::PxConcreteType::Enum((id & 0xff000000) >> 24);
			_ASSERT(obj->getConcreteType() == type);
			unsigned int index = id & 0x00ffffff;
			switch (obj->getConcreteType())
			{
			case physx::PxConcreteType::eMATERIAL:
				m_Cmps[index]->m_PxMaterial.reset(obj->is<physx::PxMaterial>());
				break;
			case physx::PxConcreteType::eRIGID_STATIC:
				m_PxActor.reset(obj->is<physx::PxRigidStatic>());
				break;
			case physx::PxConcreteType::eSHAPE:
				m_Cmps[index]->m_PxShape.reset(obj->is<physx::PxShape>());
				break;
			case physx::PxConcreteType::eHEIGHTFIELD:
				_ASSERT(m_Cmps[index]->m_Type == Component::ComponentTypeTerrain);
				boost::dynamic_pointer_cast<Terrain>(m_Cmps[i])->m_PxHeightField.reset(obj->is<physx::PxHeightField>());
				break;
			default:
				_ASSERT(false);
				break;
			}
		}
	}
}

void Actor::RequestResource(void)
{
	_ASSERT(!m_Requested);

	Component::RequestResource();

	if (m_Animator)
	{
		m_Animator->RequestResource();
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->RequestResource();
	}
}

void Actor::ReleaseResource(void)
{
	_ASSERT(m_Requested);

	if (m_Animator)
	{
		m_Animator->ReleaseResource();
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ReleaseResource();
	}

	Component::ReleaseResource();
}

void Actor::OnEnterPxScene(PhysXSceneContext * scene)
{
	Component::OnEnterPxScene(scene);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnEnterPxScene(scene);
	}

	if (m_PxActor)
	{
		scene->m_PxScene->addActor(*m_PxActor);
	}
}

void Actor::OnLeavePxScene(PhysXSceneContext * scene)
{
	if (m_PxActor)
	{
		scene->m_PxScene->removeActor(*m_PxActor, false);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnLeavePxScene(scene);
	}

	Component::OnLeavePxScene(scene);
}

void Actor::Update(float fElapsedTime)
{
	Component::Update(fElapsedTime);

	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->Update(fElapsedTime);
	}
}

my::AABB Actor::CalculateAABB(void) const
{
	AABB ret = Component::CalculateAABB();
	ComponentPtrList::const_iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		ret.unionSelf((*cmp_iter)->CalculateAABB().transform((*cmp_iter)->CalculateLocal()));
	}
	return ret;
}

void Actor::UpdateAABB(void)
{
	m_aabb = CalculateAABB();
	if (!m_aabb.IsValid())
	{
		m_aabb.unionSelf(AABB(m_aabb.Center() - 1, m_aabb.Center() + 1));
	}
}

void Actor::UpdateWorld(const my::Matrix4 & World)
{
	Component::UpdateWorld(World);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateWorld(m_World);
	}
}

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	Component::AddToPipeline(frustum, pipeline, PassMask);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask);
	}
}

void Actor::UpdateLod(const my::Vector3 & ViewPos)
{
	Component::UpdateLod(ViewPos);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateLod(ViewPos);
	}
}

void Actor::CreateRigidActor(physx::PxActorType::Enum ActorType)
{
	if (m_PxActor && m_PxActor->getType() == ActorType)
	{
		return;
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ClearShape();
	}

	my::Vector3 pos, scale; my::Quaternion rot;
	m_World.Decompose(scale, rot, pos);
	switch (ActorType)
	{
	case physx::PxActorType::eRIGID_STATIC:
		m_PxActor.reset(PhysXContext::getSingleton().m_sdk->createRigidStatic(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot)));
		break;
	case physx::PxActorType::eRIGID_DYNAMIC:
		m_PxActor.reset(PhysXContext::getSingleton().m_sdk->createRigidDynamic(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot)));
		break;
	default:
		m_PxActor.reset();
		break;
	}
}

void Actor::AddComponent(ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Actor);
	m_Cmps.push_back(cmp);
	cmp->m_Actor = this;
}

void Actor::RemoveComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = std::find(m_Cmps.begin(), m_Cmps.end(), cmp);
	if (cmp_iter != m_Cmps.end())
	{
		_ASSERT((*cmp_iter)->m_Actor == this);
		m_Cmps.erase(cmp_iter);
		(*cmp_iter)->m_Actor = NULL;
	}
}

void Actor::ClearAllComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Actor = NULL;
	}
	m_Cmps.clear();
}

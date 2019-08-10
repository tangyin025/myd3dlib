#include "Actor.h"
#include "Terrain.h"
#include "Animator.h"
#include "Controller.h"
#include "PhysXContext.h"
#include "RenderPipeline.h"
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

Actor::~Actor(void)
{
	if (m_Base)
	{
		_ASSERT(m_Base->m_Suber == this);
		m_Base->m_Suber = NULL;
	}
}

template<>
void Actor::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Position);
	ar << BOOST_SERIALIZATION_NVP(m_Rotation);
	ar << BOOST_SERIALIZATION_NVP(m_Scale);
	ar << BOOST_SERIALIZATION_NVP(m_World);
	ar << BOOST_SERIALIZATION_NVP(m_LodRatio);
	ar << BOOST_SERIALIZATION_NVP(m_Animator);
	ar << BOOST_SERIALIZATION_NVP(m_Controller);
	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
	physx::PxActorType::Enum ActorType = m_PxActor ? m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT;
	ar << BOOST_SERIALIZATION_NVP(ActorType);

	if (m_PxActor)
	{
		boost::shared_ptr<physx::PxCollection> collection(PxCreateCollection(), PhysXDeleter<physx::PxCollection>());
		collection->add(*m_PxActor, m_PxActor->getConcreteType() << 24 | 0);
		for (unsigned int i = 0; i < m_Cmps.size(); i++)
		{
			if (m_Cmps[i]->m_PxShape)
			{
				collection->add(*m_Cmps[i]->m_PxMaterial, physx::PxConcreteType::eMATERIAL << 24 | i);
				collection->add(*m_Cmps[i]->m_PxShape, physx::PxConcreteType::eSHAPE << 24 | i);
				if (m_Cmps[i]->m_Type == Component::ComponentTypeTerrain)
				{
					Terrain * terrain = dynamic_cast<Terrain *>(m_Cmps[i].get());
					if (terrain->m_PxHeightField)
					{
						collection->add(*terrain->m_PxHeightField, physx::PxConcreteType::eHEIGHTFIELD << 24 | i);
					}
				}
			}
		}
		physx::PxSerialization::complete(*collection, *PhysXSceneContext::getSingleton().m_Registry, PhysXSceneContext::getSingleton().m_Collection.get());
		physx::PxDefaultMemoryOutputStream ostr;
		physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *PhysXSceneContext::getSingleton().m_Registry, PhysXSceneContext::getSingleton().m_Collection.get());
		unsigned int PxActorSize = ostr.getSize();
		ar << BOOST_SERIALIZATION_NVP(PxActorSize);
		ar << boost::serialization::make_nvp("m_PxActor", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	}
}

template<>
void Actor::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Position);
	ar >> BOOST_SERIALIZATION_NVP(m_Rotation);
	ar >> BOOST_SERIALIZATION_NVP(m_Scale);
	ar >> BOOST_SERIALIZATION_NVP(m_World);
	ar >> BOOST_SERIALIZATION_NVP(m_LodRatio);
	ar >> BOOST_SERIALIZATION_NVP(m_Animator);
	if (m_Animator)
	{
		m_Animator->m_Actor = this;
	}
	ar >> BOOST_SERIALIZATION_NVP(m_Controller);
	if (m_Controller)
	{
		m_Controller->m_Actor = this;
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
		boost::shared_ptr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *PhysXSceneContext::getSingleton().m_Registry, PhysXSceneContext::getSingleton().m_Collection.get()), PhysXDeleter<physx::PxCollection>());
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
				m_Cmps[index]->m_PxMaterial.reset(obj->is<physx::PxMaterial>(), PhysXDeleter<physx::PxMaterial>());
				break;
			case physx::PxConcreteType::eRIGID_STATIC:
				_ASSERT(!m_PxActor);
				m_PxActor.reset(obj->is<physx::PxRigidStatic>(), PhysXDeleter<physx::PxRigidActor>());
				m_PxActor->userData = this;
				break;
			case physx::PxConcreteType::eRIGID_DYNAMIC:
				_ASSERT(!m_PxActor);
				m_PxActor.reset(obj->is<physx::PxRigidDynamic>(), PhysXDeleter<physx::PxRigidActor>());
				m_PxActor->userData = this;
				break;
			case physx::PxConcreteType::eSHAPE:
				m_Cmps[index]->m_PxShape.reset(obj->is<physx::PxShape>(), PhysXDeleter<physx::PxShape>());
				break;
			case physx::PxConcreteType::eHEIGHTFIELD:
				_ASSERT(m_Cmps[index]->m_Type == Component::ComponentTypeTerrain);
				boost::dynamic_pointer_cast<Terrain>(m_Cmps[i])->m_PxHeightField.reset(obj->is<physx::PxHeightField>(), PhysXDeleter<physx::PxHeightField>());
				break;
			default:
				_ASSERT(false);
				break;
			}
		}
	}
}

void Actor::CopyFrom(const Actor & rhs)
{
	m_aabb = rhs.m_aabb;
	m_Position = rhs.m_Position;
	m_Rotation = rhs.m_Rotation;
	m_Scale = rhs.m_Scale;
	m_World = rhs.m_World;

	m_Cmps.resize(rhs.m_Cmps.size());
	for (unsigned int i = 0; i < rhs.m_Cmps.size(); i++)
	{
		m_Cmps[i] = rhs.m_Cmps[i]->Clone();
		m_Cmps[i]->m_Actor = this;
	}
}

ActorPtr Actor::Clone(void) const
{
	ActorPtr ret(new Actor());
	ret->CopyFrom(*this);
	return ret;
}

void Actor::RequestResource(void)
{
	m_Requested = true;

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
	m_Requested = false;

	if (m_Animator)
	{
		m_Animator->ReleaseResource();
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ReleaseResource();
	}
}

void Actor::OnEnterPxScene(PhysXSceneContext * scene)
{
	if (m_PxActor)
	{
		scene->m_PxScene->addActor(*m_PxActor);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnEnterPxScene(scene);
	}
}

void Actor::OnLeavePxScene(PhysXSceneContext * scene)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnLeavePxScene(scene);
	}

	if (m_PxActor)
	{
		scene->m_PxScene->removeActor(*m_PxActor, false);
	}
}

void Actor::OnPxTransformChanged(const physx::PxTransform & trans)
{
	m_Position = (my::Vector3 &)trans.p;
	m_Rotation = (my::Quaternion &)trans.q;
	UpdateWorld();
	UpdateOctNode();
}

void Actor::OnShaderChanged(void)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnShaderChanged();
	}
}

void Actor::Update(float fElapsedTime)
{
	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}

	if (m_Controller)
	{
		m_Controller->Update(fElapsedTime);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_LodMask & m_Lod)
		{
			(*cmp_iter)->Update(fElapsedTime);
		}
	}

	AttacherPtrList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter++)
	{
		(*att_iter)->Update(fElapsedTime);
	}
}

void Actor::UpdatePose(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	if (m_PxActor)
	{
		physx::PxRigidDynamic * rigidDynamic = m_PxActor->isRigidDynamic();
		if (rigidDynamic && rigidDynamic->getRigidDynamicFlags().isSet(physx::PxRigidDynamicFlag::eKINEMATIC))
		{
			rigidDynamic->setKinematicTarget(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
		}
		else
		{
			m_PxActor->setGlobalPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
		}
	}
	else
	{
		m_Position = Pos;
		m_Rotation = Rot;
		UpdateWorld();
		UpdateOctNode();
	}
}

my::AABB Actor::CalculateAABB(void) const
{
	if (m_Cmps.empty())
	{
		return AABB(-1, 1);
	}

	AABB ret = AABB::Invalid();
	ComponentPtrList::const_iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		ret.unionSelf((*cmp_iter)->CalculateAABB());
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

void Actor::UpdateWorld(void)
{
	m_World = Matrix4::Compose(m_Scale, m_Rotation, m_Position);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnWorldUpdated();
	}
}

void Actor::UpdateOctNode(void)
{
	if (m_Node)
	{
		ActorPtr actor_ptr = boost::dynamic_pointer_cast<Actor>(shared_from_this());
		my::OctNode * Root = m_Node->GetTopNode();
		Root->RemoveActor(actor_ptr);
		Root->AddActor(actor_ptr, m_aabb.transform(m_World));
	}
}
//
//void Actor::UpdatePxTransform(void)
//{
//	// ! conflict with OnPxTransformChanged
//	if (m_PxActor)
//	{
//		m_PxActor->setGlobalPose(physx::PxTransform((physx::PxVec3&)m_Position, (physx::PxQuat&)m_Rotation));
//
//		physx::PxRigidBody * body = m_PxActor->isRigidBody();
//		if (body)
//		{
//			body->setLinearVelocity((physx::PxVec3 &)Vector3(0, 0, 0), true);
//		}
//	}
//}

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if ((RenderPipeline::PassMaskLight | RenderPipeline::PassMaskOpaque | RenderPipeline::PassMaskTransparent) & PassMask)
	{
		// ! only scene pass update lod
		UpdateLod(ViewPos, TargetPos);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_LodMask & m_Lod)
		{
			(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
		}
	}
}

void Actor::UpdateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	float DistanceSq = (m_Position - ViewPos).magnitudeSq();
	if (DistanceSq < m_LodRatio * m_LodRatio)
	{
		m_Lod = Component::LOD0;
	}
	else if (DistanceSq < m_LodRatio * m_LodRatio * 4)
	{
		m_Lod = Component::LOD1;
	}
	else
	{
		m_Lod = Component::LOD2;
	}
}

void Actor::ClearRigidActor(void)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ClearShape();
	}

	m_PxActor.reset();
}

void Actor::CreateRigidActor(physx::PxActorType::Enum ActorType)
{
	_ASSERT(!m_PxActor);

	switch (ActorType)
	{
	case physx::PxActorType::eRIGID_STATIC:
		m_PxActor.reset(PhysXContext::getSingleton().m_sdk->createRigidStatic(
			physx::PxTransform((physx::PxVec3&)m_Position, (physx::PxQuat&)m_Rotation)), PhysXDeleter<physx::PxRigidActor>());
		break;
	case physx::PxActorType::eRIGID_DYNAMIC:
		m_PxActor.reset(PhysXContext::getSingleton().m_sdk->createRigidDynamic(
			physx::PxTransform((physx::PxVec3&)m_Position, (physx::PxQuat&)m_Rotation)), PhysXDeleter<physx::PxRigidActor>());
		break;
	}
	m_PxActor->userData = this;
}

void Actor::SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value)
{
	_ASSERT(m_PxActor);

	physx::PxRigidBody * body = m_PxActor->isRigidBody();
	if (body)
	{
		body->setRigidBodyFlag(Flag, Value);
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

void Actor::Attach(ActorPtr other, int BoneId)
{
	_ASSERT(other->m_Base == NULL);
	m_Attaches.insert(AttacherPtr(new BoneAttacher(this, other.get(), BoneId)));
}

void Actor::Dettach(ActorPtr other)
{
	_ASSERT(other->m_Base != NULL);
	AttacherPtrList::iterator att_iter = m_Attaches.find(other->m_Base->shared_from_this());
	_ASSERT(att_iter != m_Attaches.end());
	m_Attaches.erase(att_iter);
}

void BoneAttacher::Update(float fElapsedTime)
{
	_ASSERT(m_Owner);

	if (m_Suber)
	{
		if (m_Owner->m_Animator && !m_Owner->m_Animator->anim_pose_hier.empty())
		{
			const Bone & bone = m_Owner->m_Animator->anim_pose_hier[m_BoneId];
			m_Suber->UpdatePose(
				bone.m_position.transformCoord(m_Owner->m_World), bone.m_rotation.multiply(Quaternion::RotationMatrix(m_Owner->m_World)));
		}

		m_Suber->Update(fElapsedTime);
	}
}

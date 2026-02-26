// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "Actor.h"
#include "Terrain.h"
#include "Animator.h"
#include "PhysxContext.h"
#include "RenderPipeline.h"
#include "myResource.h"
#include "ActionTrack.h"
#include <PxPhysicsAPI.h>
#include <fstream>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/range/algorithm/find_if.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Actor)

ActorSerializationContext::ActorSerializationContext(void)
	: m_Registry(physx::PxSerialization::createSerializationRegistry(*PhysxSdk::getSingleton().m_sdk), PhysxDeleter<physx::PxSerializationRegistry>())
{
}

const int Actor::MaxLod = 3;

float Actor::MinBlock = 1.0f;

float Actor::Threshold = 0.1f;

Actor::~Actor(void)
{
	if (m_Node)
	{
		_ASSERT(false); m_Node->GetTopNode()->RemoveEntity(this);
	}

	_ASSERT(m_ActionInstList.empty());

	ClearAllComponent();

	_ASSERT(m_EventOnTrigger.empty());

	_ASSERT(m_EventOnContact.empty());

	_ASSERT(m_EventPxThreadShapeHit.empty());

	_ASSERT(m_EventPxThreadControllerHit.empty());

	_ASSERT(m_EventPxThreadObstacleHit.empty());

	_ASSERT(m_Attaches.empty());

	_ASSERT(m_Joints.empty());

	_ASSERT(!m_Aggregate || m_Aggregate->getNbActors() == 0);

	_ASSERT(!m_Base);

	_ASSERT(!IsRequested());

	_ASSERT(!m_PxActor || (!m_PxActor->getScene() && !m_PxActor->getAggregate()));
}

template<class Archive>
void Actor::save(Archive & ar, const unsigned int version) const
{
	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Position);
	ar << BOOST_SERIALIZATION_NVP(m_Rotation);
	ar << BOOST_SERIALIZATION_NVP(m_Scale);
	ar << BOOST_SERIALIZATION_NVP(m_LodDist);
	ar << BOOST_SERIALIZATION_NVP(m_LodFactor);
	ar << BOOST_SERIALIZATION_NVP(m_CullingDistSq);

	physx::PxActorType::Enum ActorType = GetRigidActorType();
	ar << BOOST_SERIALIZATION_NVP(ActorType);
	switch (ActorType)
	{
	case physx::PxActorType::eRIGID_DYNAMIC:
	{
		physx::PxRigidBody* body = m_PxActor->is<physx::PxRigidBody>();
		physx::PxRigidBodyFlags::InternalType RigidBodyFlags = (physx::PxRigidBodyFlags::InternalType)body->getRigidBodyFlags();
		ar << BOOST_SERIALIZATION_NVP(RigidBodyFlags);
		break;
	}
	case physx::PxActorType::eRIGID_STATIC:
	{
		break;
	}
	}

	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
}

template<class Archive>
void Actor::load(Archive & ar, const unsigned int version)
{
	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Position);
	ar >> BOOST_SERIALIZATION_NVP(m_Rotation);
	ar >> BOOST_SERIALIZATION_NVP(m_Scale);
	ar >> BOOST_SERIALIZATION_NVP(m_LodDist);
	ar >> BOOST_SERIALIZATION_NVP(m_LodFactor);
	ar >> BOOST_SERIALIZATION_NVP(m_CullingDistSq);

	physx::PxActorType::Enum ActorType;
	ar >> BOOST_SERIALIZATION_NVP(ActorType);
	switch (ActorType)
	{
	case physx::PxActorType::eRIGID_DYNAMIC:
	{
		CreateRigidActor(physx::PxActorType::eRIGID_DYNAMIC);
		physx::PxRigidBody* body = m_PxActor->is<physx::PxRigidBody>();
		physx::PxRigidBodyFlags::InternalType RigidBodyFlags;
		ar >> BOOST_SERIALIZATION_NVP(RigidBodyFlags);
		body->setRigidBodyFlags(physx::PxRigidBodyFlags(RigidBodyFlags));
		break;
	}
	case physx::PxActorType::eRIGID_STATIC:
	{
		CreateRigidActor(physx::PxActorType::eRIGID_STATIC);
		break;
	}
	}

	ComponentPtrList cmps;
	ar >> boost::serialization::make_nvp("m_Cmps", cmps);
	ComponentPtrList::iterator cmp_iter = cmps.begin();
	for(; cmp_iter != cmps.end(); cmp_iter++)
	{
		InsertComponent(*cmp_iter);
	}

	UpdateWorld();
}

template
void Actor::save<boost::archive::polymorphic_xml_oarchive>(boost::archive::polymorphic_xml_oarchive& ar, const unsigned int version) const;

template
void Actor::save<boost::archive::polymorphic_text_oarchive>(boost::archive::polymorphic_text_oarchive& ar, const unsigned int version) const;

template
void Actor::save<boost::archive::polymorphic_binary_oarchive>(boost::archive::polymorphic_binary_oarchive& ar, const unsigned int version) const;

template
void Actor::load<boost::archive::polymorphic_xml_iarchive>(boost::archive::polymorphic_xml_iarchive& ar, const unsigned int version);

template
void Actor::load<boost::archive::polymorphic_text_iarchive>(boost::archive::polymorphic_text_iarchive& ar, const unsigned int version);

template
void Actor::load<boost::archive::polymorphic_binary_iarchive>(boost::archive::polymorphic_binary_iarchive& ar, const unsigned int version);

boost::shared_ptr<boost::archive::polymorphic_iarchive> Actor::GetIArchive(std::istream& istr, const char* ext)
{
	if (_stricmp(ext, ".xml") == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::xml_iarchive>
			, public ActorSerializationContext
			, public my::NamedObjectSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags)
				: polymorphic_iarchive_route(is, flags)
			{
			}
		};
		return boost::shared_ptr<boost::archive::polymorphic_iarchive>(new Archive(istr, 0));
	}

	if (_stricmp(ext, ".txt") == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::text_iarchive>
			, public ActorSerializationContext
			, public my::NamedObjectSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags)
				: polymorphic_iarchive_route(is, flags)
			{
			}
		};
		return boost::shared_ptr<boost::archive::polymorphic_iarchive>(new Archive(istr, 0));
	}

	class Archive
		: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::binary_iarchive>
		, public ActorSerializationContext
		, public my::NamedObjectSerializationContext
	{
	public:
		Archive(std::istream& is, unsigned int flags)
			: polymorphic_iarchive_route(is, flags)
		{
		}
	};
	return boost::shared_ptr<boost::archive::polymorphic_iarchive>(new Archive(istr, 0));
}

boost::shared_ptr<boost::archive::polymorphic_oarchive> Actor::GetOArchive(std::ostream& ostr, const char* ext)
{
	if (_stricmp(ext, ".xml") == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_oarchive_route<boost::archive::xml_oarchive>
			, public ActorSerializationContext
		{
		public:
			Archive(std::ostream& os, unsigned int flags = 0)
				: polymorphic_oarchive_route(os, flags)
			{
			}
		};
		return boost::shared_ptr<boost::archive::polymorphic_oarchive>(new Archive(ostr));
	}
	else if (_stricmp(ext, ".txt") == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_oarchive_route<boost::archive::text_oarchive>
			, public ActorSerializationContext
		{
		public:
			Archive(std::ostream& os, unsigned int flags = 0)
				: polymorphic_oarchive_route(os, flags)
			{
			}
		};
		return boost::shared_ptr<boost::archive::polymorphic_oarchive>(new Archive(ostr));
	}

	class Archive
		: public boost::archive::detail::polymorphic_oarchive_route<boost::archive::binary_oarchive>
		, public ActorSerializationContext
	{
	public:
		Archive(std::ostream& os, unsigned int flags = 0)
			: polymorphic_oarchive_route(os, flags)
		{
		}
	};
	return boost::shared_ptr<boost::archive::polymorphic_oarchive>(new Archive(ostr));
}

ActorPtr Actor::Clone(void) const
{
	std::stringstream sstr;
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(sstr, ".txt");
	*oa << boost::serialization::make_nvp(__FUNCTION__, shared_from_this());

	ActorPtr ret;
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(sstr, ".txt");
	boost::dynamic_pointer_cast<NamedObjectSerializationContext>(ia)->make_unique = true;
	*ia >> boost::serialization::make_nvp(__FUNCTION__, ret);
	return ret;
}

void Actor::RequestResource(void)
{
	m_Requested = true;

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

	if (!m_Base && m_PxActor)
	{
		_ASSERT(!m_PxActor->getScene());

		scene->m_PxScene->addActor(*m_PxActor);
	}

	if (m_Aggregate)
	{
		scene->m_PxScene->addAggregate(*m_Aggregate);
	}
}

void Actor::ReleaseResource(void)
{
	// ! Component::ReleaseResource may change other cmp's life time
	ComponentPtrList dummy_cmps(m_Cmps.begin(), m_Cmps.end());
	ComponentPtrList::iterator cmp_iter = dummy_cmps.begin();
	for (; cmp_iter != dummy_cmps.end(); cmp_iter++)
	{
		if (this == (*cmp_iter)->m_Actor)
		{
			if ((*cmp_iter)->IsRequested())
			{
				(*cmp_iter)->ReleaseResource();
			}
		}
	}

	m_Requested = false;

	m_Lod = MaxLod;

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

	if (!m_Base && m_PxActor)
	{
		_ASSERT(m_PxActor->getScene() == scene->m_PxScene.get());

		scene->m_PxScene->removeActor(*m_PxActor, false);

		scene->removeRenderActorsFromPhysicsActor(m_PxActor.get());
	}

	if (m_Aggregate)
	{
		scene->m_PxScene->removeAggregate(*m_Aggregate, false);
	}
}

void Actor::Update(float fElapsedTime)
{
	if (m_Base && (!m_PxActor || GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC)))
	{
		Bone attach_pose = m_Base->GetAttachPose(m_BaseBoneId, m_Position, m_Rotation);

		m_World = Matrix4::Compose(m_Scale, attach_pose.m_rotation, attach_pose.m_position);

		UpdateOctNode();
	}

	if (!(m_SignatureFlags & SignatureFlagUpdate))
	{
		return;
	}

	// ! ScriptActionTrackInst::UpdateTime may invalidate action_inst_iter by self:Play, StopAction..
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		// ! ActionTrack length needs to exceed 1/60th of a second to prevent the loss of physics frames
		if ((*action_inst_iter)->m_LastTime < (*action_inst_iter)->m_Template->m_Length)
		{
			(*action_inst_iter)->Update(fElapsedTime);

			action_inst_iter++;
		}
		else
		{
			// ! make sure associated objs were not in parallel tasks, ActionTrackEmitterInst::Stop
			action_inst_iter = StopActionInstIter(action_inst_iter);
		}
	}

	// ! Component::Update may change other cmp's life time
	ComponentPtrList dummy_cmps(m_Cmps.begin(), m_Cmps.end());
	ComponentPtrList::iterator cmp_iter = dummy_cmps.begin();
	for (; cmp_iter != dummy_cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_LodMask & 1 << m_Lod && this == (*cmp_iter)->m_Actor)
		{
			(*cmp_iter)->Update(fElapsedTime);
		}
	}
}

void Actor::SetPose(const my::Vector3 & Pos)
{
	SetPose(Pos, m_Rotation);
}

void Actor::SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	m_Position = Pos;

	m_Rotation = Rot;

	if (!m_Base || (m_PxActor && !GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC))) // ! Actor::Update, m_Base->GetAttachPose
	{
		UpdateWorld();

		if (m_Node)
		{
			UpdateOctNode();
		}
	}
}

void Actor::SetPose(const my::Bone & Pose)
{
	SetPose(Pose.m_position, Pose.m_rotation);
}

void Actor::SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot, const Component * Exclusion)
{
	if (m_PxActor)
	{
		physx::PxRigidDynamic* rigidDynamic = m_PxActor->is<physx::PxRigidDynamic>();
		if (rigidDynamic && rigidDynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
		{
			rigidDynamic->setKinematicTarget(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
		}
		else
		{
			m_PxActor->setGlobalPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
		}
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if (cmp_iter->get() != Exclusion)
		{
			(*cmp_iter)->SetPxPoseOrbyPxThread(Pos, Rot);
		}
	}

	const Animator* animator = GetFirstComponent<Animator>();

	const Matrix4 World = Matrix4::Compose(m_Scale, Rot, Pos);

	AttachList::iterator attach_iter = m_Attaches.begin();
	for (; attach_iter != m_Attaches.end(); attach_iter++)
	{
		my::Bone pose((*attach_iter)->m_Position, (*attach_iter)->m_Rotation);

		if (animator && (*attach_iter)->m_BaseBoneId >= 0 && (*attach_iter)->m_BaseBoneId < (int)animator->anim_pose.size())
		{
			pose.TransformSelf(animator->anim_pose[(*attach_iter)->m_BaseBoneId]);
		}

		(*attach_iter)->SetPxPoseOrbyPxThread(pose.m_position.transformCoord(World), pose.m_rotation * Rot, Exclusion);
	}
}

my::AABB Actor::CalculateAABB(void) const
{
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
		m_aabb = AABB(-1, 1);
	}
}

void Actor::UpdateWorld(void)
{
	_ASSERT(!m_Base || (m_PxActor && !GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC)));

	_ASSERT(!m_Node || !PhysxSdk::getSingleton().m_RenderTickMuted);

	m_World = Matrix4::Compose(m_Scale, m_Rotation, m_Position);
}

void Actor::UpdateOctNode(void)
{
	_ASSERT(m_Node);

	_ASSERT(!m_Node || !PhysxSdk::getSingleton().m_RenderTickMuted);

	my::OctNode* Root = m_Node->GetTopNode();
	Root->OctNode::RemoveEntity(this);
	Root->OctNode::AddEntity(this, m_aabb.transform(m_World), MinBlock, Threshold);
}

int Actor::CalculateLod(float dist) const
{
	return dist > 0 ? Max((int)(logf(dist / m_LodDist) / logf(m_LodFactor)), 0) : 0;
}

void Actor::SetLod(int Lod)
{
	_ASSERT(IsRequested());

	if (m_Lod != Lod)
	{
		m_Lod = Lod;

		// ! Component::RequestResource may change other cmp's life time
		ComponentPtrList dummy_cmps(m_Cmps.begin(), m_Cmps.end());
		ComponentPtrList::iterator cmp_iter = dummy_cmps.begin();
		for (; cmp_iter != dummy_cmps.end(); cmp_iter++)
		{
			if (this == (*cmp_iter)->m_Actor)
			{
				if ((*cmp_iter)->m_LodMask >= 1 << Lod)
				{
					if (!(*cmp_iter)->IsRequested())
					{
						(*cmp_iter)->RequestResource();
					}
				}
				else
				{
					if ((*cmp_iter)->IsRequested())
					{
						(*cmp_iter)->ReleaseResource();
					}
				}
			}
		}
	}
}

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	for (int Lod = m_Lod; Lod < MaxLod; )
	{
		ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
		for (; cmp_iter != m_Cmps.end(); cmp_iter++)
		{
			if ((*cmp_iter)->m_LodMask & 1 << Lod)
			{
				if ((*cmp_iter)->IsRequested()
					&& ((*cmp_iter)->GetComponentType() != Component::ComponentTypeMesh || static_cast<MeshComponent*>(cmp_iter->get())->m_Mesh || static_cast<MeshComponent*>(cmp_iter->get())->m_MeshPath.empty()))
				{
					(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
				}
				else
				{
					goto continue_next_lod;
				}
			}
		}
		break;
	continue_next_lod:
		Lod++;
	}
}

void Actor::ClearRigidActor(void)
{
	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

	if (!m_Base && m_PxActor && IsRequested())
	{
		_ASSERT(m_PxActor->getScene() == scene->m_PxScene.get());

		scene->m_PxScene->removeActor(*m_PxActor, false);

		scene->removeRenderActorsFromPhysicsActor(m_PxActor.get());
	}

	if (m_Base && m_PxActor)
	{
		_ASSERT(m_Base->m_Aggregate);

		BOOST_VERIFY(m_Base->m_Aggregate->removeActor(*m_PxActor));

		scene->removeRenderActorsFromPhysicsActor(m_PxActor.get());
	}

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

	_ASSERT(m_Cmps.end() == boost::find_if(m_Cmps, boost::bind(&Component::m_PxShape, boost::bind(&ComponentPtr::get, boost::placeholders::_1))));

	switch (ActorType)
	{
	case physx::PxActorType::eRIGID_STATIC:
		m_PxActor.reset(PhysxSdk::getSingleton().m_sdk->createRigidStatic(
			physx::PxTransform((physx::PxVec3&)m_Position, (physx::PxQuat&)m_Rotation)), PhysxDeleter<physx::PxRigidActor>());
		m_PxActor->userData = this;
		break;
	case physx::PxActorType::eRIGID_DYNAMIC:
		m_PxActor.reset(PhysxSdk::getSingleton().m_sdk->createRigidDynamic(
			physx::PxTransform((physx::PxVec3&)m_Position, (physx::PxQuat&)m_Rotation)), PhysxDeleter<physx::PxRigidActor>());
		m_PxActor->userData = this;
		break;
	}

	if (!m_Base && m_PxActor && IsRequested())
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		_ASSERT(!m_PxActor->getScene());

		scene->m_PxScene->addActor(*m_PxActor);
	}

	if (m_Base && m_PxActor)
	{
		if (!m_Base->m_Aggregate)
		{
			m_Base->CreateAggregate(true);
		}

		BOOST_VERIFY(m_Base->m_Aggregate->addActor(*m_PxActor));
	}
}

physx::PxActorType::Enum Actor::GetRigidActorType(void) const
{
	return m_PxActor ? m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT;
}

void Actor::SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->setRigidBodyFlag(Flag, Value);
}

bool Actor::GetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag) const
{
	return m_PxActor ? m_PxActor->is<physx::PxRigidBody>()->getRigidBodyFlags() & Flag : false;
}

void Actor::SetMass(float mass)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->setMass(mass);
}

float Actor::GetMass(void) const
{
	return m_PxActor ? m_PxActor->is<physx::PxRigidBody>()->getMass() : 0.0f;
}

void Actor::SetCMassLocalPose(const my::Bone & pose)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->setCMassLocalPose((physx::PxTransform&)pose);
}

my::Bone Actor::GetCMassLocalPose(void) const
{
	return m_PxActor ? (Bone&)m_PxActor->is<physx::PxRigidBody>()->getCMassLocalPose() : Bone(Vector3(0, 0, 0));
}

void Actor::SetMassSpaceInertiaTensor(const my::Vector3 & m)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->setMassSpaceInertiaTensor((physx::PxVec3&)m);
}

my::Vector3 Actor::GetMassSpaceInertiaTensor(void)
{
	return m_PxActor ? (Vector3&)m_PxActor->is<physx::PxRigidBody>()->getCMassLocalPose() : Vector3(0, 0, 0);
}

void Actor::UpdateMassAndInertia(float density)
{
	_ASSERT(m_PxActor);
	physx::PxRigidBodyExt::updateMassAndInertia(*m_PxActor->is<physx::PxRigidBody>(), density);
}

void Actor::SetLinearVelocity(const my::Vector3& LinearVelocity)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->setLinearVelocity((physx::PxVec3&)LinearVelocity, true);
}

my::Vector3 Actor::GetLinearVelocity(void) const
{
	return m_PxActor ? (Vector3&)m_PxActor->is<physx::PxRigidBody>()->getLinearVelocity() : Vector3(0, 0, 0);
}

void Actor::SetAngularVelocity(const my::Vector3 & angVel)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->setAngularVelocity((physx::PxVec3&)angVel);
}

my::Vector3 Actor::GetAngularVelocity(void) const
{
	return m_PxActor ? (Vector3&)m_PxActor->is<physx::PxRigidBody>()->getAngularVelocity() : Vector3(0, 0, 0);
}

void Actor::AddForce(const my::Vector3& force, physx::PxForceMode::Enum mode, bool autowake)
{
	_ASSERT(m_PxActor);
	m_PxActor->is<physx::PxRigidBody>()->addForce((physx::PxVec3&)force, mode, true);
}

bool Actor::IsSleeping(void) const
{
	_ASSERT(GetRigidActorType() == physx::PxActorType::eRIGID_DYNAMIC);
	return m_PxActor->is<physx::PxRigidDynamic>()->isSleeping();
}

void Actor::WakeUp(void)
{
	_ASSERT(GetRigidActorType() == physx::PxActorType::eRIGID_DYNAMIC);
	m_PxActor->is<physx::PxRigidDynamic>()->wakeUp();
}

void Actor::InsertComponent(ComponentPtr cmp)
{
	InsertComponent(GetComponentNum(), cmp);
}

void Actor::InsertComponent(unsigned int i, ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Actor);

	_ASSERT(i <= m_Cmps.size());

	_ASSERT(!m_Node || !PhysxSdk::getSingleton().m_RenderTickMuted);

	// ! Component::RequestResource may change other cmp's life time
	m_Cmps.insert(m_Cmps.begin() + i, cmp);

	cmp->m_Actor = this;

	switch (cmp->GetComponentType())
	{
	//case Component::ComponentTypeComponent:
	//case Component::ComponentTypeController:
	//case Component::ComponentTypeMesh:
	case Component::ComponentTypeCloth:
	//case Component::ComponentTypeEmitter:
	//case Component::ComponentTypeStaticEmitter:
	//case Component::ComponentTypeCircularEmitter:
	case Component::ComponentTypeSphericalEmitter:
	//case Component::ComponentTypeTerrain:
	case Component::ComponentTypeAnimator:
	//case Component::ComponentTypeNavigation:
	//case Component::ComponentTypeSteering:
	case Component::ComponentTypeScript:
		m_SignatureFlags |= SignatureFlagUpdate;
		break;
	}

	if (cmp->m_PxShape && m_PxActor)
	{
		_ASSERT(!cmp->m_PxShape->getActor());

		m_PxActor->attachShape(*cmp->m_PxShape);
	}

	if (IsRequested())
	{
		_ASSERT(m_Node);

		if (cmp->m_LodMask & 1 << m_Lod)
		{
			cmp->RequestResource();
		}
	}
}

void Actor::RemoveComponent(unsigned int i)
{
	_ASSERT(i < m_Cmps.size());

	_ASSERT(!m_Node || !PhysxSdk::getSingleton().m_RenderTickMuted);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin() + i;

	// ! Component::ReleaseResource may change other cmp's life time
	ComponentPtr dummy_cmp = *cmp_iter;

	_ASSERT(dummy_cmp->m_Actor == this);

	m_Cmps.erase(cmp_iter);

	if (IsRequested())
	{
		_ASSERT(m_Node);

		if (dummy_cmp->IsRequested())
		{
			dummy_cmp->ReleaseResource();
		}
	}

	if (dummy_cmp->m_PxShape && m_PxActor)
	{
		_ASSERT(dummy_cmp->m_PxShape->getActor() == m_PxActor.get());

		m_PxActor->detachShape(*dummy_cmp->m_PxShape);
	}

	dummy_cmp->m_Actor = NULL;
}

unsigned int Actor::GetComponentNum(void) const
{
	return m_Cmps.size();
}

void Actor::ClearAllComponent(void)
{
	while (GetComponentNum() > 0)
	{
		RemoveComponent(GetComponentNum() - 1);
	}
}

void Actor::Attach(Actor * other, int BoneId)
{
	_ASSERT(other != this && other->m_Base == NULL);

	_ASSERT(m_Node && m_Node->GetTopNode()->HaveNode(other->m_Node));

	m_Attaches.push_back(other);

	other->m_Base = this;

	other->m_BaseBoneId = BoneId;

	if (other->m_PxActor)
	{
		if (!m_Aggregate)
		{
			CreateAggregate(true);
		}

		if (other->IsRequested())
		{
			PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

			scene->m_PxScene->removeActor(*other->m_PxActor);
		}

		_ASSERT(!other->m_PxActor->getScene());

		BOOST_VERIFY(m_Aggregate->addActor(*other->m_PxActor));
	}

	//const my::Bone pose = Bone(other->m_Position, other->m_Rotation).TransformTranspose(GetAttachPose(BoneId, Vector3(0, 0, 0), Quaternion::Identity()));

	//other->SetPose(pose);
}

void Actor::Detach(Actor * other)
{
	AttachList::iterator att_iter = std::find(m_Attaches.begin(), m_Attaches.end(), other);
	if (att_iter != m_Attaches.end())
	{
		_ASSERT((*att_iter)->m_Base == this);

		(*att_iter)->m_Base = NULL;

		m_Attaches.erase(att_iter);

		if (other->m_PxActor)
		{
			JointPtrList::iterator joint_iter = m_Joints.begin();
			for (; joint_iter != m_Joints.end(); )
			{
				physx::PxRigidActor* actor0, * actor1;
				(*joint_iter)->getActors(actor0, actor1);
				if (other->m_PxActor.get() == actor0 || other->m_PxActor.get() == actor1)
				{
					joint_iter = m_Joints.erase(joint_iter);
				}
				else
				{
					joint_iter++;
				}
			}

			_ASSERT(m_Aggregate);

			BOOST_VERIFY(m_Aggregate->removeActor(*other->m_PxActor)); // ! the actor is reinserted in that scene

			PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

			if (other->IsRequested())
			{
				if (IsRequested())
				{
					_ASSERT(other->m_PxActor->getScene() == scene->m_PxScene.get());
				}
				else
				{
					scene->m_PxScene->addActor(*other->m_PxActor);
				}
			}
			else
			{
				if (IsRequested())
				{
					_ASSERT(other->m_PxActor->getScene() == scene->m_PxScene.get());

					scene->m_PxScene->removeActor(*other->m_PxActor);
				}

				scene->removeRenderActorsFromPhysicsActor(other->m_PxActor.get());
			}
		}

		//const my::Bone pose = GetAttachPose(other->m_BaseBoneId, other->m_Position, other->m_Rotation);

		//other->SetPose(pose.m_position, pose.m_rotation);

		return;
	}
	_ASSERT(false);
}

unsigned int Actor::GetAttachNum(void) const
{
	return m_Attaches.size();
}

my::Bone Actor::GetAttachPose(int BoneId, const my::Vector3 & LocalPosition, const my::Quaternion & LocalRotation) const
{
	Quaternion RootRotation; Vector3 RootPosition, RootScale;

	m_World.Decompose(RootScale, RootRotation, RootPosition);

	my::Bone pose(LocalPosition, LocalRotation);

	const Animator* animator = GetFirstComponent<Animator>();

	if (animator && BoneId >= 0 && BoneId < (int)animator->anim_pose.size())
	{
		pose.TransformSelf(animator->anim_pose[BoneId]);
	}

	return my::Bone(pose.m_position.transformCoord(m_World), pose.m_rotation * RootRotation);
}

void Actor::ClearAllAttach(void)
{
	AttachList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter = m_Attaches.begin())
	{
		Detach(*att_iter);
	}
}

physx::PxRevoluteJoint * Actor::AddRevoluteJoint(Actor * actor0, const my::Bone & localFrame0, Actor * actor1, const my::Bone & localFrame1)
{
	physx::PxRevoluteJoint* joint = physx::PxRevoluteJointCreate(*PhysxSdk::getSingleton().m_sdk,
		actor0->m_PxActor.get(), (physx::PxTransform&)localFrame0, actor1->m_PxActor.get(), (physx::PxTransform&)localFrame1);
	m_Joints.push_back(boost::shared_ptr<physx::PxJoint>(joint, PhysxDeleter<physx::PxJoint>()));
	return joint;
}

physx::PxD6Joint * Actor::AddD6Joint(Actor * actor0, const my::Bone & localFrame0, Actor * actor1, const my::Bone & localFrame1)
{
	physx::PxD6Joint* joint = physx::PxD6JointCreate(*PhysxSdk::getSingleton().m_sdk,
		actor0->m_PxActor.get(), (physx::PxTransform&)localFrame0, actor1->m_PxActor.get(), (physx::PxTransform&)localFrame1);
	m_Joints.push_back(boost::shared_ptr<physx::PxJoint>(joint, PhysxDeleter<physx::PxJoint>()));
	return joint;
}

void Actor::CreateAggregate(bool enableSelfCollision)
{
	_ASSERT(!m_Aggregate);

	m_Aggregate.reset(PhysxSdk::getSingleton().m_sdk->createAggregate(64, enableSelfCollision), PhysxDeleter<physx::PxBase>());

	if (IsRequested())
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		scene->m_PxScene->addAggregate(*m_Aggregate);
	}
}

boost::shared_ptr<ActionInst> Actor::PlayAction(Action * action)
{
	ActionInstPtr action_inst = action->CreateInstance(this);
	m_ActionInstList.push_back(action_inst);
	return action_inst;
}

Actor::ActionInstPtrList::iterator Actor::StopActionInstIter(ActionInstPtrList::iterator action_inst_iter)
{
	_ASSERT(action_inst_iter != m_ActionInstList.end());

	(*action_inst_iter)->StopAllTrack();

	// ! inst may out of the actor's lifetime
	ActionInst::ActionTrackInstPtrList::iterator track_inst_iter = (*action_inst_iter)->m_TrackInstList.begin();
	for (; track_inst_iter != (*action_inst_iter)->m_TrackInstList.end(); track_inst_iter++)
	{
		(*track_inst_iter)->m_Actor = NULL;
	}

	return m_ActionInstList.erase(action_inst_iter);
}

void Actor::StopActionInst(boost::shared_ptr<ActionInst> action_inst)
{
	ActionInstPtrList::iterator action_inst_iter = std::find(m_ActionInstList.begin(), m_ActionInstList.end(), action_inst);
	if (action_inst_iter != m_ActionInstList.end())
	{
		StopActionInstIter(action_inst_iter);
	}
}

void Actor::StopAllActionInst(void)
{
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		action_inst_iter = StopActionInstIter(action_inst_iter);
	}
}

Component * Actor::GetFirstComponent(DWORD Type, unsigned int startid)
{
	for (unsigned int i = startid; i < m_Cmps.size(); i++)
	{
		if (m_Cmps[i]->GetComponentType() == Type)
		{
			return m_Cmps[i].get();
		}
	}
	return NULL;
}

const Component * Actor::GetFirstComponent(DWORD Type, unsigned int startid) const
{
	for (unsigned int i = startid; i < m_Cmps.size(); i++)
	{
		if (m_Cmps[i]->GetComponentType() == Type)
		{
			return m_Cmps[i].get();
		}
	}
	return NULL;
}

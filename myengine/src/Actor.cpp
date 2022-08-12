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

const float Actor::MinBlock = 1.0f;

const float Actor::Threshold = 0.1f;

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

	physx::PxActorType::Enum ActorType = m_PxActor ? m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT;
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

	ActorPtr ret(new Actor());
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

	m_Lod = Component::MAX_LOD;

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

		SetPxPoseOrbyPxThread(attach_pose.m_position, attach_pose.m_rotation, NULL);

		UpdateOctNode();
	}

	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		action_inst_iter->first->Update(fElapsedTime);

		if (action_inst_iter->first->m_LastTime < action_inst_iter->second)
		{
			action_inst_iter++;
		}
		else
		{
			// ! make sure action inst was not in parallel task list
			action_inst_iter = StopActionIter(action_inst_iter);
		}
	}

	// ! Component::Update may change other cmp's life time
	ComponentPtrList dummy_cmps(m_Cmps.begin(), m_Cmps.end());
	ComponentPtrList::iterator cmp_iter = dummy_cmps.begin();
	for (; cmp_iter != dummy_cmps.end(); cmp_iter++)
	{
		if (this == (*cmp_iter)->m_Actor)
		{
			if ((*cmp_iter)->m_LodMask & 1 << m_Lod)
			{
				(*cmp_iter)->Update(fElapsedTime);
			}
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

void Actor::SetPxPoseOrbyPxThread(const my::Vector3 & Pos)
{
	SetPxPoseOrbyPxThread(Pos, m_Rotation, NULL);
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
}

void Actor::SetPxPoseOrbyPxThread(const my::Bone & Pose)
{
	SetPxPoseOrbyPxThread(Pose.m_position, Pose.m_rotation, NULL);
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

void Actor::UpdateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(IsRequested());

	int Lod = Min(CalculateLod((m_OctAabb->Center() - ViewPos).magnitude()), (int)Component::MAX_LOD);
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
	for (int Lod = m_Lod; Lod < Component::MAX_LOD; Lod++)
	{
		bool lodRequested = false;
		ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
		for (; cmp_iter != m_Cmps.end(); cmp_iter++)
		{
			if (((*cmp_iter)->m_LodMask & 1 << Lod) && (*cmp_iter)->IsRequested())
			{
				(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos);
				lodRequested = true;
			}
		}

		if (lodRequested)
		{
			break;
		}
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

	_ASSERT(m_Cmps.end() == boost::find_if(m_Cmps, boost::bind(&Component::m_PxShape, boost::bind(&ComponentPtr::operator->, boost::placeholders::_1))));

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

void Actor::InsertComponent(ComponentPtr cmp)
{
	InsertComponent(GetComponentNum(), cmp);
}

void Actor::InsertComponent(unsigned int i, ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Actor);

	_ASSERT(i <= m_Cmps.size());

	// ! Component::RequestResource may change other cmp's life time
	m_Cmps.insert(m_Cmps.begin() + i, cmp);

	cmp->m_Actor = this;

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
	_ASSERT(other->m_Base == NULL);

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

	const my::Bone pose = Bone(other->m_Position, other->m_Rotation).TransformTranspose(GetAttachPose(BoneId, Vector3(0, 0, 0), Quaternion::Identity()));

	other->SetPose(pose);
}

void Actor::Detach(Actor * other)
{
	ActorList::iterator att_iter = std::find(m_Attaches.begin(), m_Attaches.end(), other);
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

		const my::Bone pose = GetAttachPose(other->m_BaseBoneId, other->m_Position, other->m_Rotation);

		other->SetPose(pose.m_position, pose.m_rotation);
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

	const Animator* animator = GetFirstComponent<Animator>();

	if (animator && BoneId >= 0 && BoneId < (int)animator->anim_pose_hier.size())
	{
		const my::Bone& bone = animator->anim_pose_hier[BoneId];

		my::Bone parent(bone.m_position.transformCoord(m_World), bone.m_rotation * RootRotation);

		return my::Bone(LocalPosition, LocalRotation).Transform(parent);
	}

	return my::Bone(LocalPosition, LocalRotation).Transform(RootPosition, RootRotation);
}

void Actor::ClearAllAttach(void)
{
	ActorList::iterator att_iter = m_Attaches.begin();
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

boost::shared_ptr<ActionInst> Actor::PlayAction(Action * action, float Length)
{
	ActionInstPtr action_inst = action->CreateInstance(this);
	m_ActionInstList.push_back(std::make_pair(action_inst, Length));
	return action_inst;
}

Actor::ActionInstPtrList::iterator Actor::StopActionIter(ActionInstPtrList::iterator action_inst_iter)
{
	_ASSERT(action_inst_iter != m_ActionInstList.end());

	action_inst_iter->first->StopAllTrack();

	// ! inst may out of the actor's lifetime
	ActionInst::ActionTrackInstPtrList::iterator track_inst_iter = action_inst_iter->first->m_TrackInstList.begin();
	for (; track_inst_iter != action_inst_iter->first->m_TrackInstList.end(); track_inst_iter++)
	{
		(*track_inst_iter)->m_Actor = NULL;
	}

	return m_ActionInstList.erase(action_inst_iter);
}

void Actor::StopAction(boost::shared_ptr<ActionInst> action_inst)
{
	ActionInstPtrList::iterator action_inst_iter = boost::find_if(m_ActionInstList,
		boost::bind(std::equal_to<const boost::shared_ptr<ActionInst>&>(), boost::bind(&std::pair< boost::shared_ptr<ActionInst>, float>::first, boost::placeholders::_1), action_inst));
	if (action_inst_iter != m_ActionInstList.end())
	{
		StopActionIter(action_inst_iter);
	}
}

void Actor::StopAllAction(void)
{
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		action_inst_iter = StopActionIter(action_inst_iter);
	}
}

bool Actor::TickActionAndGetDisplacement(float dtime, my::Vector3 & disp)
{
	_ASSERT(PhysxSdk::getSingleton().m_RenderTickMuted);

	bool ret = false;
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); action_inst_iter++)
	{
		const float LastTime = action_inst_iter->first->m_Time;

		action_inst_iter->first->m_Time += dtime;

		if (action_inst_iter->first->m_Time < action_inst_iter->second)
		{
			my::Vector3 local_disp;
			if (action_inst_iter->first->GetDisplacement(LastTime, dtime, local_disp))
			{
				if (ret)
				{
					disp += local_disp;
				}
				else
				{
					disp = local_disp;
					ret = true;
				}
			}
		}
	}
	return ret;
}

Component * Actor::GetFirstComponent(DWORD Type)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->GetComponentType() == Type)
		{
			return cmp_iter->get();
		}
	}
	return NULL;
}

const Component * Actor::GetFirstComponent(DWORD Type) const
{
	ComponentPtrList::const_iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->GetComponentType() == Type)
		{
			return cmp_iter->get();
		}
	}
	return NULL;
}

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
#include <boost/serialization/list.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

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

	_ASSERT(m_EventEnterTrigger.empty());

	_ASSERT(m_EventLeaveTrigger.empty());

	_ASSERT(m_EventPxThreadShapeHit.empty());

	_ASSERT(m_Attaches.empty());

	_ASSERT(!m_Base);

	_ASSERT(!IsRequested());

	_ASSERT(!m_PxActor || !m_PxActor->getScene());
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

	ar >> BOOST_SERIALIZATION_NVP(m_Cmps);
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for(; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Actor = this;
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

boost::shared_ptr<boost::archive::polymorphic_iarchive> Actor::GetIArchive(std::istream& istr, const char* ext, const char* prefix)
{
	if (_stricmp(ext, ".xml") == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::xml_iarchive>
			, public ActorSerializationContext
			, public my::NamedObjectSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags, const char* prefix)
				: polymorphic_iarchive_route(is, flags)
				, NamedObjectSerializationContext(prefix)
			{
			}
		};
		return boost::shared_ptr<boost::archive::polymorphic_iarchive>(new Archive(istr, 0, prefix));
	}

	if (_stricmp(ext, ".txt") == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::text_iarchive>
			, public ActorSerializationContext
			, public my::NamedObjectSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags, const char* prefix)
				: polymorphic_iarchive_route(is, flags)
				, NamedObjectSerializationContext(prefix)
			{
			}
		};
		return boost::shared_ptr<boost::archive::polymorphic_iarchive>(new Archive(istr, 0, prefix));
	}

	class Archive
		: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::binary_iarchive>
		, public ActorSerializationContext
		, public my::NamedObjectSerializationContext
	{
	public:
		Archive(std::istream& is, unsigned int flags, const char* prefix)
			: polymorphic_iarchive_route(is, flags)
			, NamedObjectSerializationContext(prefix)
		{
		}
	};
	return boost::shared_ptr<boost::archive::polymorphic_iarchive>(new Archive(istr, 0, prefix));
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
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(sstr, ".txt", "");
	*ia >> boost::serialization::make_nvp(__FUNCTION__, ret);
	return ret;
}

void Actor::RequestResource(void)
{
	m_Requested = true;

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

	if (m_PxActor)
	{
		_ASSERT(!m_PxActor->getScene());

		scene->m_PxScene->addActor(*m_PxActor);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->EnterPhysxScene(scene);
	}
}

void Actor::ReleaseResource(void)
{
	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

	// ! Component::ReleaseResource may change other cmp's life time
	DelayRemover<ComponentPtr>::getSingleton().Enter(boost::bind(&Actor::RemoveComponent, this, boost::placeholders::_1));
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_Actor)
		{
			if ((*cmp_iter)->IsRequested())
			{
				(*cmp_iter)->ReleaseResource();
			}

			(*cmp_iter)->LeavePhysxScene(scene);
		}
	}
	DelayRemover<ComponentPtr>::getSingleton().Leave();

	m_Requested = false;

	m_Lod = Component::LOD_INFINITE;

	if (m_PxActor)
	{
		_ASSERT(m_PxActor->getScene() == scene->m_PxScene.get());

		scene->m_PxScene->removeActor(*m_PxActor, false);

		scene->removeRenderActorsFromPhysicsActor(m_PxActor.get());
	}
}

void Actor::Update(float fElapsedTime)
{
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		if (action_inst_iter->first->m_Time < action_inst_iter->second)
		{
			action_inst_iter->first->Update(fElapsedTime);

			action_inst_iter++;
		}
		else
		{
			action_inst_iter->first->Stop();

			// ! make sure action inst was not in parallel task list
			action_inst_iter = m_ActionInstList.erase(action_inst_iter);
		}
	}

	// ! Component::Update may change other cmp's life time
	DelayRemover<ComponentPtr>::getSingleton().Enter(boost::bind(&Actor::RemoveComponent, this, boost::placeholders::_1));
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_Actor)
		{
			if ((*cmp_iter)->m_LodMask & 1 << m_Lod)
			{
				(*cmp_iter)->Update(fElapsedTime);
			}
		}
	}
	DelayRemover<ComponentPtr>::getSingleton().Leave();

	UpdateAttaches(fElapsedTime);
}

void Actor::UpdateAttaches(float fElapsedTime)
{
	Animator* animator = GetFirstComponent<Animator>();
	AttachPairList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter++)
	{
		if (animator && att_iter->second >= 0 && att_iter->second < (int)animator->anim_pose_hier.size())
		{
			const Bone & bone = animator->anim_pose_hier[att_iter->second];

			const Matrix4 World = Matrix4::Compose(
				Vector3(1, 1, 1), att_iter->first->m_Rotation * bone.m_rotation, bone.m_rotation * att_iter->first->m_Position + bone.m_position) * m_World;

			Vector3 Scale, Pos; Quaternion Rot;
			World.Decompose(Scale, Rot, Pos);

			att_iter->first->m_World = Matrix4::Compose(att_iter->first->m_Scale, Rot, Pos);

			att_iter->first->SetPxPoseOrbyPxThread(Pos, Rot);
		}
		else
		{
			const Matrix4 World = Matrix4::Compose(
				att_iter->first->m_Scale, att_iter->first->m_Rotation, att_iter->first->m_Position) * m_World;

			Vector3 Scale, Pos; Quaternion Rot;
			World.Decompose(Scale, Rot, Pos);

			att_iter->first->m_World = Matrix4::Compose(att_iter->first->m_Scale, Rot, Pos);

			att_iter->first->SetPxPoseOrbyPxThread(Pos, Rot);
		}

		att_iter->first->UpdateOctNode();

		att_iter->first->Update(fElapsedTime);
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

	UpdateWorld();

	UpdateOctNode();
}

void Actor::SetPxPoseOrbyPxThread(const my::Vector3 & Pos)
{
	SetPxPoseOrbyPxThread(Pos, m_Rotation);
}

void Actor::SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	if (m_PxActor)
	{
		physx::PxRigidDynamic* rigidDynamic = m_PxActor->is<physx::PxRigidDynamic>();
		if (rigidDynamic)
		{
			if (rigidDynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
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
			m_PxActor->setGlobalPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
		}
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->SetPxPoseOrbyPxThread(Pos, Rot);
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
	_ASSERT(!m_Base);

	m_World = Matrix4::Compose(m_Scale, m_Rotation, m_Position);
}

void Actor::UpdateOctNode(void)
{
	if (m_Node)
	{
		my::OctNode * Root = m_Node->GetTopNode();
		Root->OctNode::RemoveEntity(this);
		Root->OctNode::AddEntity(this, m_aabb.transform(m_World), MinBlock, Threshold);
	}
}

int Actor::CalculateLod(const my::AABB & Aabb, const my::Vector3 & ViewPos) const
{
	float DistanceSq = (Aabb.Center() - ViewPos).magnitudeSq();
	int Lod = (int)(logf(sqrt(DistanceSq) / m_LodDist) / logf(m_LodFactor));
	return Max(Lod, 0);
}

void Actor::UpdateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(IsRequested());

	int Lod = Min(CalculateLod(*m_OctAabb, ViewPos), Component::LOD_INFINITE - 1);
	if (m_Lod != Lod)
	{
		m_Lod = Lod;

		// ! Component::RequestResource may change other cmp's life time
		DelayRemover<ComponentPtr>::getSingleton().Enter(boost::bind(&Actor::RemoveComponent, this, boost::placeholders::_1));
		ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
		for (; cmp_iter != m_Cmps.end(); cmp_iter++)
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
		DelayRemover<ComponentPtr>::getSingleton().Leave();
	}
}

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	for (int Lod = m_Lod; Lod < Component::LOD_INFINITE; Lod++)
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
}

void Actor::SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value)
{
	_ASSERT(m_PxActor);

	physx::PxRigidBody * body = m_PxActor->is<physx::PxRigidBody>();
	if (body)
	{
		body->setRigidBodyFlag(Flag, Value);
	}
}

bool Actor::GetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag) const
{
	_ASSERT(m_PxActor);

	physx::PxRigidBody * body = m_PxActor->is<physx::PxRigidBody>();
	if (body)
	{
		return body->getRigidBodyFlags() & Flag;
	}
	return false;
}

void Actor::AddComponent(ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Actor);

	// ! Component::RequestResource may change other cmp's life time
	m_Cmps.push_back(cmp);

	cmp->m_Actor = this;

	if (IsRequested())
	{
		_ASSERT(m_Node);

		if (cmp->m_LodMask & 1 << m_Lod)
		{
			cmp->RequestResource();
		}

		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		cmp->EnterPhysxScene(scene);
	}
}

void Actor::RemoveComponent(ComponentPtr cmp)
{
	if (DelayRemover<ComponentPtr>::getSingleton().IsDelay(boost::bind(&Actor::RemoveComponent, this, boost::placeholders::_1)))
	{
		DelayRemover<ComponentPtr>::getSingleton().push_back(cmp);
		return;
	}

	ComponentPtrList::iterator cmp_iter = std::find(m_Cmps.begin(), m_Cmps.end(), cmp);
	if (cmp_iter != m_Cmps.end())
	{
		_ASSERT(cmp->m_Actor == this);

		// ! Component::ReleaseResource may change other cmp's life time
		m_Cmps.erase(cmp_iter);

		if (IsRequested())
		{
			_ASSERT(m_Node);

			if (cmp->IsRequested())
			{
				cmp->ReleaseResource();
			}

			PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

			cmp->LeavePhysxScene(scene);
		}

		cmp->m_Actor = NULL;
	}
	else
	{
		_ASSERT(false);
	}
}

void Actor::ClearAllComponent(void)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter = m_Cmps.begin())
	{
		RemoveComponent(*cmp_iter);
	}
	_ASSERT(m_Cmps.empty());
}

void Actor::Attach(Actor * other, int BoneId)
{
	_ASSERT(other->m_Base == NULL);

	_ASSERT(m_Node && m_Node->GetTopNode()->HaveNode(other->m_Node));

	m_Attaches.push_back(std::make_pair(other, BoneId));

	other->m_Base = this;
}

void Actor::Detach(Actor * other)
{
	AttachPairList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter++)
	{
		if (att_iter->first == other)
		{
			_ASSERT(att_iter->first->m_Base == this);
			att_iter->first->m_Base = NULL;
			m_Attaches.erase(att_iter);
			return;
		}
	}
	_ASSERT(false);
}

void Actor::ClearAllAttacher(void)
{
	AttachPairList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter = m_Attaches.begin())
	{
		Detach(att_iter->first);
	}
}

void Actor::PlayAction(Action * action, float Length)
{
	m_ActionInstList.push_back(std::make_pair(ActionInstPtr(action->CreateInstance(this)), Length));
}

void Actor::StopAllAction(void)
{
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); action_inst_iter++)
	{
		action_inst_iter->first->Stop();
	}
	m_ActionInstList.clear();
}

Component * Actor::GetFirstComponent(Component::ComponentType Type)
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

template <>
Animator * Actor::GetFirstComponent<Animator>(void)
{
	return dynamic_cast<Animator*>(GetFirstComponent(Component::ComponentTypeAnimator));
}

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

			(*cmp_iter)->LeavePhysxScene(scene);
		}
	}

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
	if (m_Base)
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

	if (!m_Base) // ! Actor::Update, m_Base->GetAttachPose
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
	_ASSERT(!m_Base);

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

int Actor::CalculateLod(float DistanceSq, float Scale) const
{
	return Max((int)(logf(sqrt(DistanceSq) / (m_LodDist * Scale)) / logf(m_LodFactor)), 0);
}

int Actor::CalculateLod(const my::Vector3 & Center, const my::Vector3 & ViewPos, float Scale) const
{
	return CalculateLod((Center - ViewPos).magnitudeSq(), Scale);
}

int Actor::CalculateLod2D(const my::Vector3 & Center, const my::Vector3 & ViewPos, float Scale) const
{
	return CalculateLod((Center - ViewPos).magnitudeSq2D(), Scale);
}

void Actor::UpdateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(IsRequested());

	int Lod = Min(CalculateLod(m_OctAabb->Center(), ViewPos, 1.0f), Component::LOD_INFINITE - 1);
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
	if (m_PxActor && IsRequested())
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		_ASSERT(m_PxActor->getScene() == scene->m_PxScene.get());

		scene->m_PxScene->removeActor(*m_PxActor, false);

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

	if (m_PxActor && IsRequested())
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		_ASSERT(!m_PxActor->getScene());

		scene->m_PxScene->addActor(*m_PxActor);
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

void Actor::InsertComponent(ComponentPtr cmp)
{
	InsertComponent(GetComponentNum(), cmp);
}

void Actor::InsertComponent(unsigned int i, ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Actor);

	_ASSERT(i <= m_Cmps.size());

	m_Cmps.insert(m_Cmps.begin() + i, cmp);

	cmp->m_Actor = this;

	// ! Component::RequestResource may change other cmp's life time
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

void Actor::RemoveComponent(unsigned int i)
{
	_ASSERT(i < m_Cmps.size());

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin() + i;

	_ASSERT((*cmp_iter)->m_Actor == this);

	// ! Component::ReleaseResource may change other cmp's life time
	if (IsRequested())
	{
		_ASSERT(m_Node);

		if ((*cmp_iter)->IsRequested())
		{
			(*cmp_iter)->ReleaseResource();
		}

		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		(*cmp_iter)->LeavePhysxScene(scene);
	}

	(*cmp_iter)->m_Actor = NULL;

	m_Cmps.erase(cmp_iter);
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

	m_Attaches.insert(other);

	other->m_Base = this;

	other->m_BaseBoneId = BoneId;

	const my::Bone pose = GetAttachPose(BoneId, Vector3(0, 0, 0), Quaternion::Identity());

	other->m_Position = pose.m_rotation.conjugate() * (other->m_Position - pose.m_position);

	other->m_Rotation = other->m_Rotation * pose.m_rotation.conjugate();
}

void Actor::Detach(Actor * other)
{
	ActorList::iterator att_iter = m_Attaches.find(other);
	if (att_iter != m_Attaches.end())
	{
		_ASSERT((*att_iter)->m_Base == this);

		(*att_iter)->m_Base = NULL;

		m_Attaches.erase(att_iter);

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

	return my::Bone(LocalPosition.transformCoord(m_World), LocalRotation * RootRotation);
}

void Actor::ClearAllAttach(void)
{
	ActorList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter = m_Attaches.begin())
	{
		Detach(*att_iter);
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

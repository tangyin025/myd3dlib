#include "Actor.h"
#include "Terrain.h"
#include "Animation.h"
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

using namespace my;

BOOST_CLASS_EXPORT(Actor)

const float Actor::MinBlock = 1.0f;

const float Actor::Threshold = 0.1f;

Actor::~Actor(void)
{
	if (m_Node)
	{
		m_Node->GetTopNode()->RemoveEntity(this);
	}

	_ASSERT(!IsViewNotified());

	_ASSERT(m_ActionInstList.empty());

	ClearAllComponent();

	_ASSERT(m_Attaches.empty());

	_ASSERT(!m_Base);

	_ASSERT(!IsRequested());

	_ASSERT(!m_PxActor || !m_PxActor->getScene());
}

template<class Archive>
void Actor::save(Archive & ar, const unsigned int version) const
{
	PhysxSerializationContext* pxar = dynamic_cast<PhysxSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Position);
	ar << BOOST_SERIALIZATION_NVP(m_Rotation);
	ar << BOOST_SERIALIZATION_NVP(m_Scale);
	ar << BOOST_SERIALIZATION_NVP(m_LodDist);
	ar << BOOST_SERIALIZATION_NVP(m_LodFactor);
	ar << BOOST_SERIALIZATION_NVP(m_Animation);
	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
	physx::PxActorType::Enum ActorType = m_PxActor ? m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT;
	ar << BOOST_SERIALIZATION_NVP(ActorType);

	if (m_PxActor)
	{
		boost::shared_ptr<physx::PxCollection> collection(PxCreateCollection(), PhysxDeleter<physx::PxCollection>());
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
		physx::PxSerialization::complete(*collection, *pxar->m_Registry);
		physx::PxDefaultMemoryOutputStream ostr;
		physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *pxar->m_Registry);
		unsigned int PxActorSize = ostr.getSize();
		ar << BOOST_SERIALIZATION_NVP(PxActorSize);
		ar << boost::serialization::make_nvp("m_PxActor", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	}
}

template<class Archive>
void Actor::load(Archive & ar, const unsigned int version)
{
	PhysxSerializationContext* pxar = dynamic_cast<PhysxSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Position);
	ar >> BOOST_SERIALIZATION_NVP(m_Rotation);
	ar >> BOOST_SERIALIZATION_NVP(m_Scale);
	ar >> BOOST_SERIALIZATION_NVP(m_LodDist);
	ar >> BOOST_SERIALIZATION_NVP(m_LodFactor);
	ar >> BOOST_SERIALIZATION_NVP(m_Animation);
	if (m_Animation)
	{
		m_Animation->m_Actor = this;
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
		boost::shared_ptr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *pxar->m_Registry), PhysxDeleter<physx::PxCollection>());
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
				m_Cmps[index]->m_PxMaterial.reset(obj->is<physx::PxMaterial>(), PhysxDeleter<physx::PxMaterial>());
				break;
			case physx::PxConcreteType::eRIGID_STATIC:
				_ASSERT(!m_PxActor);
				m_PxActor.reset(obj->is<physx::PxRigidStatic>(), PhysxDeleter<physx::PxRigidActor>());
				m_PxActor->userData = this;
				break;
			case physx::PxConcreteType::eRIGID_DYNAMIC:
				_ASSERT(!m_PxActor);
				m_PxActor.reset(obj->is<physx::PxRigidDynamic>(), PhysxDeleter<physx::PxRigidActor>());
				m_PxActor->userData = this;
				break;
			case physx::PxConcreteType::eSHAPE:
				m_Cmps[index]->m_PxShape.reset(obj->is<physx::PxShape>(), PhysxDeleter<physx::PxShape>());
				m_Cmps[index]->m_PxShape->userData = m_Cmps[index].get();
				break;
			case physx::PxConcreteType::eHEIGHTFIELD:
				_ASSERT(m_Cmps[index]->m_Type == Component::ComponentTypeTerrain);
				boost::dynamic_pointer_cast<Terrain>(m_Cmps[i])->m_PxHeightField.reset(obj->is<physx::PxHeightField>(), PhysxDeleter<physx::PxHeightField>());
				break;
			default:
				_ASSERT(false);
				break;
			}
		}
	}

	UpdateWorld();
}

void Actor::CopyFrom(const Actor & rhs)
{
	if (rhs.m_Name)
	{
		SetName(NamedObject::MakeUniqueName(rhs.m_Name).c_str());
	}

	m_aabb = rhs.m_aabb;
	m_Position = rhs.m_Position;
	m_Rotation = rhs.m_Rotation;
	m_Scale = rhs.m_Scale;
	m_World = rhs.m_World;
	m_LodDist = rhs.m_LodDist;
	m_LodFactor = rhs.m_LodFactor;
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

	if (m_Animation)
	{
		m_Animation->RequestResource();
	}

	_ASSERT(m_Lod == Component::LOD_INFINITE);

	SetLod(Component::LOD_INFINITE >> 1);
}

void Actor::ReleaseResource(void)
{
	m_Requested = false;

	if (m_Animation)
	{
		m_Animation->ReleaseResource();
	}

	SetLod(Component::LOD_INFINITE);

#ifdef _DEBUG
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		_ASSERT(!(*cmp_iter)->IsRequested());
	}
#endif
}

void Actor::EnterPhysxScene(PhysxScene * scene)
{
	if (m_PxActor)
	{
		scene->m_PxScene->addActor(*m_PxActor);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->EnterPhysxScene(scene);
	}
}

void Actor::LeavePhysxScene(PhysxScene * scene)
{
	_ASSERT(!m_PxActor || m_PxActor->getScene() == scene->m_PxScene.get());

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->LeavePhysxScene(scene);
	}

	if (m_PxActor)
	{
		scene->m_PxScene->removeActor(*m_PxActor, false);

		scene->removeRenderActorsFromPhysicsActor(m_PxActor.get());
	}
}

void Actor::NotifyEnterView(void)
{
	m_ViewNotified = true;

	if (m_EventEnterView)
	{
		ActorEventArg arg(this);
		m_EventEnterView(&arg);
	}
}

void Actor::NotifyLeaveView(void)
{
	m_ViewNotified = false;

	if (m_EventLeaveView)
	{
		ActorEventArg arg(this);
		m_EventLeaveView(&arg);
	}
}

void Actor::OnPxTransformChanged(const physx::PxTransform & trans)
{
	if (m_PxActor && !m_Base)
	{
		m_Position = (my::Vector3 &)trans.p;

		m_Rotation = (my::Quaternion &)trans.q;

		UpdateWorld();

		UpdateOctNode();
	}
}

void Actor::Update(float fElapsedTime)
{
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); )
	{
		if ((*action_inst_iter)->m_Time < (*action_inst_iter)->m_Template->m_Length)
		{
			(*action_inst_iter)->Update(fElapsedTime);

			action_inst_iter++;
		}
		else
		{
			(*action_inst_iter)->Stop();

			// ! make sure action inst was not in parallel task list
			action_inst_iter = m_ActionInstList.erase(action_inst_iter);
		}
	}

	if (m_Animation)
	{
		m_Animation->Update(fElapsedTime);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_LodMask & m_Lod)
		{
			(*cmp_iter)->Update(fElapsedTime);
		}
	}

	AttachPairList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter++)
	{
		if (m_Animation && att_iter->second >= 0 && att_iter->second < (int)m_Animation->anim_pose_hier.size())
		{
			const Bone & bone = m_Animation->anim_pose_hier[att_iter->second];
			att_iter->first->SetPose(
				bone.m_position.transformCoord(m_World), bone.m_rotation.multiply(Quaternion::RotationMatrix(m_World)));
		}
		else
		{
			att_iter->first->SetPose(m_Position, m_Rotation);
		}

		att_iter->first->Update(fElapsedTime);
	}
}

void Actor::SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	if (m_PxActor)
	{
		physx::PxRigidDynamic * rigidDynamic = m_PxActor->is<physx::PxRigidDynamic>();
		if (rigidDynamic)
		{
			if (rigidDynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
			{
				rigidDynamic->setKinematicTarget(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
				if (m_Base)
				{
					// ! attached Actor update render pose immediately
				}
				else
				{
					return;
				}
			}
			else
			{
				m_PxActor->setGlobalPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
				return;
			}
		}
		else
		{
			m_PxActor->setGlobalPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
		}
	}

	m_Position = Pos;

	m_Rotation = Rot;

	UpdateWorld();

	UpdateOctNode();
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

bool Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	bool ret = false;

	for (unsigned int lod = m_Lod; lod < Component::LOD_INFINITE && !ret; lod <<= 1)
	{
		ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
		for (; cmp_iter != m_Cmps.end(); cmp_iter++)
		{
			if ((*cmp_iter)->IsRequested() && ((*cmp_iter)->m_LodMask & lod))
			{
				if ((*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask, ViewPos, TargetPos))
				{
					ret = true;
				}
			}
		}
	}

	return ret;
}

Component::LODMask Actor::CalculateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_OctAabb);

	float DistanceSq = (m_OctAabb->Center() - ViewPos).magnitudeSq();

	if (DistanceSq < m_LodDist * m_LodDist)
	{
		return Component::LOD0;
	}
	else if (DistanceSq < powf(m_LodDist * powf(m_LodFactor, 2.0f), 2.0f))
	{
		return Component::LOD1;
	}
	return Component::LOD2;
}

void Actor::SetLod(unsigned int lod)
{
	if (m_Lod != lod)
	{
		m_Lod = lod;

		ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
		for (; cmp_iter != m_Cmps.end(); cmp_iter++)
		{
			if ((*cmp_iter)->m_LodMask >= lod)
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
	m_Cmps.push_back(cmp);
	cmp->m_Actor = this;

	if (IsRequested() && (cmp->m_LodMask & m_Lod))
	{
		cmp->RequestResource();
	}

	if (m_PxActor && m_PxActor->getScene())
	{
		cmp->EnterPhysxScene((PhysxScene *)m_PxActor->getScene()->userData);
	}
}

void Actor::RemoveComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = std::find(m_Cmps.begin(), m_Cmps.end(), cmp);
	if (cmp_iter != m_Cmps.end())
	{
		_ASSERT((*cmp_iter)->m_Actor == this);

		if (m_PxActor && m_PxActor->getScene())
		{
			cmp->LeavePhysxScene((PhysxScene*)m_PxActor->getScene()->userData);
		}

		if (IsRequested() && cmp->IsRequested())
		{
			cmp->ReleaseResource();
		}

		(*cmp_iter)->m_Actor = NULL;
		m_Cmps.erase(cmp_iter);
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

ActorPtr Actor::LoadFromFile(const char * path)
{
	IStreamBuff buff(my::ResourceMgr::getSingleton().OpenIStream(path));
	std::istream istr(&buff);
	LPCSTR Ext = PathFindExtensionA(path);
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia;
	if (_stricmp(Ext, ".xml") == 0)
	{
		ia.reset(new boost::archive::polymorphic_xml_iarchive(istr));
	}
	else if (_stricmp(Ext, ".txt") == 0)
	{
		ia.reset(new boost::archive::polymorphic_text_iarchive(istr));
	}
	else
	{
		ia.reset(new boost::archive::polymorphic_binary_iarchive(istr));
	}
	ActorPtr ret;
	*ia >> boost::serialization::make_nvp("Actor", ret);
	return ret;
}

void Actor::SaveToFile(const char * path) const
{
	std::ofstream ostr(my::ResourceMgr::getSingleton().GetFullPath(path), std::ios::binary);
	LPCSTR Ext = PathFindExtensionA(path);
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa;
	if (_stricmp(Ext, ".xml") == 0)
	{
		oa.reset(new boost::archive::polymorphic_xml_oarchive(ostr));
	}
	else if (_stricmp(Ext, ".txt") == 0)
	{
		oa.reset(new boost::archive::polymorphic_text_oarchive(ostr));
	}
	else
	{
		oa.reset(new boost::archive::polymorphic_binary_oarchive(ostr));
	}
	*oa << boost::serialization::make_nvp("Actor", shared_from_this());
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

void Actor::PlayAction(Action * action)
{
	ActionInstPtr act_inst(action->CreateInstance(this));
	m_ActionInstList.push_back(act_inst);
}

void Actor::StopAllAction(void)
{
	ActionInstPtrList::iterator action_inst_iter = m_ActionInstList.begin();
	for (; action_inst_iter != m_ActionInstList.end(); action_inst_iter++)
	{
		(*action_inst_iter)->Stop();
	}
	m_ActionInstList.clear();
}

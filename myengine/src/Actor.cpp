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
				collection->add(*m_Cmps[i]->m_PxShape, physx::PxConcreteType::eSHAPE << 24 | i);
				if (m_Cmps[i]->m_PxMaterial)
				{
					collection->add(*m_Cmps[i]->m_PxMaterial, physx::PxConcreteType::eMATERIAL << 24 | i);
				}
				if (m_Cmps[i]->GetComponentType() == Component::ComponentTypeMesh)
				{
					MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_Cmps[i].get());
					if (mesh_cmp->m_PxMesh)
					{
						_ASSERT(mesh_cmp->m_PxMesh->getConcreteType() == physx::PxConcreteType::eCONVEX_MESH
							|| mesh_cmp->m_PxMesh->getConcreteType() == physx::PxConcreteType::eTRIANGLE_MESH_BVH33);
						collection->add(*mesh_cmp->m_PxMesh, mesh_cmp->m_PxMesh->getConcreteType() << 24 | i);
					}
				}
				else if (m_Cmps[i]->GetComponentType() == Component::ComponentTypeTerrain)
				{
					Terrain * terrain = dynamic_cast<Terrain *>(m_Cmps[i].get());
					if (terrain->m_PxHeightField)
					{
						_ASSERT(terrain->m_PxHeightField->getConcreteType() == physx::PxConcreteType::eHEIGHTFIELD);
						collection->add(*terrain->m_PxHeightField, physx::PxConcreteType::eHEIGHTFIELD << 24 | i);
					}
				}
			}
		}
		physx::PxSerialization::complete(*collection, *pxar->m_Registry, pxar->m_Collection.get());
		physx::PxDefaultMemoryOutputStream ostr;
		physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *pxar->m_Registry, pxar->m_Collection.get());
		unsigned int PxActorSize = ostr.getSize();
		ar << BOOST_SERIALIZATION_NVP(PxActorSize);
		ar << boost::serialization::make_nvp("m_PxActor", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	}
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
		boost::shared_ptr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *pxar->m_Registry, pxar->m_Collection.get()), PhysxDeleter<physx::PxCollection>());
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
			case physx::PxConcreteType::eCONVEX_MESH:
				_ASSERT(m_Cmps[index]->GetComponentType() == Component::ComponentTypeMesh && !pxar->m_Collection->contains(*obj));
				boost::dynamic_pointer_cast<MeshComponent>(m_Cmps[index])->m_PxMesh.reset(obj->is<physx::PxConvexMesh>(), PhysxDeleter<physx::PxConvexMesh>());
				break;
			case physx::PxConcreteType::eTRIANGLE_MESH_BVH33:
				_ASSERT(m_Cmps[index]->GetComponentType() == Component::ComponentTypeMesh && !pxar->m_Collection->contains(*obj));
				boost::dynamic_pointer_cast<MeshComponent>(m_Cmps[index])->m_PxMesh.reset(obj->is<physx::PxTriangleMesh>(), PhysxDeleter<physx::PxTriangleMesh>());
				break;
			case physx::PxConcreteType::eMATERIAL:
				_ASSERT(!pxar->m_Collection->contains(*obj));
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
				_ASSERT(m_Cmps[index]->GetComponentType() == Component::ComponentTypeTerrain && !pxar->m_Collection->contains(*obj));
				boost::dynamic_pointer_cast<Terrain>(m_Cmps[index])->m_PxHeightField.reset(obj->is<physx::PxHeightField>(), PhysxDeleter<physx::PxHeightField>());
				break;
			default:
				_ASSERT(false);
				break;
			}
		}
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

	if (m_PxActor)
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

		_ASSERT(!m_PxActor->getScene());

		scene->m_PxScene->addActor(*m_PxActor);
	}
}

void Actor::ReleaseResource(void)
{
	m_Requested = false;

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->IsRequested())
		{
			(*cmp_iter)->ReleaseResource();
		}
	}

	m_Lod = Component::LOD_INFINITE;

	if (m_PxActor)
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Node->GetTopNode());

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

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_LodMask & 1 << m_Lod)
		{
			(*cmp_iter)->Update(fElapsedTime);
		}
	}
}

void Actor::UpdateAttaches(float fElapsedTime)
{
	Animator* animator = GetAnimator();
	AttachPairList::iterator att_iter = m_Attaches.begin();
	for (; att_iter != m_Attaches.end(); att_iter++)
	{
		if (animator && att_iter->second >= 0 && att_iter->second < (int)animator->anim_pose_hier.size())
		{
			const Bone & bone = animator->anim_pose_hier[att_iter->second];
			att_iter->first->SetPose(
				bone.m_position.transformCoord(m_World), bone.m_rotation * m_Rotation);
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
		physx::PxRigidDynamic* rigidDynamic = m_PxActor->is<physx::PxRigidDynamic>();
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

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->SetPxPoseOrbyPxThread(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
	}
}

void Actor::SetPxPoseOrbyPxThread(const physx::PxTransform& pose)
{
	if (m_PxActor)
	{
		physx::PxRigidDynamic* rigidDynamic = m_PxActor->is<physx::PxRigidDynamic>();
		if (rigidDynamic)
		{
			if (rigidDynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
			{
				rigidDynamic->setKinematicTarget(pose);
			}
			else
			{
				m_PxActor->setGlobalPose(pose);
			}
		}
		else
		{
			m_PxActor->setGlobalPose(pose);
		}
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->SetPxPoseOrbyPxThread(pose);
	}
}

void Actor::OnPxTransformChanged(const physx::PxTransform& trans)
{
	//_ASSERT(m_PxActor);

	if (!m_Base)
	{
		m_Position = (my::Vector3&)trans.p;

		m_Rotation = (my::Quaternion&)trans.q;

		UpdateWorld();

		UpdateOctNode();
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

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(IsRequested());

	if (PassMask | RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
	{
		int Lod = Min(CalculateLod(*m_OctAabb, ViewPos), Component::LOD_INFINITE - 1);
		if (m_Lod != Lod)
		{
			m_Lod = Lod;

			// ! Component::RequestResource may change other cmp's life time
			ComponentPtrList enable_reentrant_dummy(m_Cmps.begin(), m_Cmps.end());

			ComponentPtrList::iterator cmp_iter = enable_reentrant_dummy.begin();
			for (; cmp_iter != enable_reentrant_dummy.end(); cmp_iter++)
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

int Actor::CalculateLod(const my::AABB & Aabb, const my::Vector3 & ViewPos) const
{
	float DistanceSq = (Aabb.Center() - ViewPos).magnitudeSq();
	int Lod = (int)(logf(sqrt(DistanceSq) / m_LodDist) / logf(m_LodFactor));
	return Max(Lod, 0);
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

	if (IsRequested() && (cmp->m_LodMask & 1 << m_Lod))
	{
		_ASSERT(m_Node);

		cmp->RequestResource();
	}
}

void Actor::RemoveComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = std::find(m_Cmps.begin(), m_Cmps.end(), cmp);
	if (cmp_iter != m_Cmps.end())
	{
		_ASSERT((*cmp_iter)->m_Actor == this);

		if (IsRequested() && cmp->IsRequested())
		{
			_ASSERT(m_Node);

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

Animator* Actor::GetAnimator(void)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeAnimator)
		{
			return dynamic_cast<Animator*>(cmp_iter->get());
		}
	}
	return NULL;
}

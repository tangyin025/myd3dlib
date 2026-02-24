// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "Component.h"
#include "Actor.h"
#include "myDxutApp.h"
#include "myEffect.h"
#include "myResource.h"
#include "Animator.h"
#include "Material.h"
#include "PhysxContext.h"
#include "RenderPipeline.h"
#include "libc.h"
#include <boost/multi_array.hpp>
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

BOOST_CLASS_EXPORT(Component)

BOOST_CLASS_EXPORT(MeshComponent)

BOOST_CLASS_EXPORT(ClothComponent)

BOOST_CLASS_EXPORT(EmitterComponent)

BOOST_CLASS_EXPORT(CircularEmitter)

BOOST_CLASS_EXPORT(SphericalEmitter)

Component::~Component(void)
{
	_ASSERT(!IsRequested());

	if (m_Actor)
	{
		_ASSERT(false); //m_Actor->RemoveComponent(GetSiblingId());
	}
}

template<class Archive>
void Component::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar << BOOST_SERIALIZATION_NVP(m_LodMask);
	ar << BOOST_SERIALIZATION_NVP(m_Material);
	ar << BOOST_SERIALIZATION_NVP(m_PxGeometryType);
	switch (m_PxGeometryType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		_ASSERT(m_PxShape && m_PxGeometryType == m_PxShape->getGeometryType());
		my::Vector3 ShapePos = (my::Vector3&)m_PxShape->getLocalPose().p;
		ar << BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot = (my::Quaternion&)m_PxShape->getLocalPose().q;
		ar << BOOST_SERIALIZATION_NVP(ShapeRot);
		physx::PxSphereGeometry sphere;
		BOOST_VERIFY(m_PxShape->getSphereGeometry(sphere));
		float SphereRadius = sphere.radius;
		ar << BOOST_SERIALIZATION_NVP(SphereRadius);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::ePLANE:
	{
		_ASSERT(m_PxShape && m_PxGeometryType == m_PxShape->getGeometryType());
		my::Vector3 ShapePos = (my::Vector3&)m_PxShape->getLocalPose().p;
		ar << BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot = (my::Quaternion&)m_PxShape->getLocalPose().q;
		ar << BOOST_SERIALIZATION_NVP(ShapeRot);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eCAPSULE:
	{
		_ASSERT(m_PxShape && m_PxGeometryType == m_PxShape->getGeometryType());
		my::Vector3 ShapePos = (my::Vector3&)m_PxShape->getLocalPose().p;
		ar << BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot = (my::Quaternion&)m_PxShape->getLocalPose().q;
		ar << BOOST_SERIALIZATION_NVP(ShapeRot);
		physx::PxCapsuleGeometry capsule;
		BOOST_VERIFY(m_PxShape->getCapsuleGeometry(capsule));
		float CapsuleRadius = capsule.radius;
		ar << BOOST_SERIALIZATION_NVP(CapsuleRadius);
		float CapsuleHalfHeight = capsule.halfHeight;
		ar << BOOST_SERIALIZATION_NVP(CapsuleHalfHeight);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eBOX:
	{
		_ASSERT(m_PxShape && m_PxGeometryType == m_PxShape->getGeometryType());
		my::Vector3 ShapePos = (my::Vector3&)m_PxShape->getLocalPose().p;
		ar << BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot = (my::Quaternion&)m_PxShape->getLocalPose().q;
		ar << BOOST_SERIALIZATION_NVP(ShapeRot);
		physx::PxBoxGeometry box;
		BOOST_VERIFY(m_PxShape->getBoxGeometry(box));
		my::Vector3 BoxHalfExtents = (my::Vector3&)box.halfExtents;
		ar << BOOST_SERIALIZATION_NVP(BoxHalfExtents);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	}
}

template<class Archive>
void Component::load(Archive & ar, const unsigned int version)
{
	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar >> BOOST_SERIALIZATION_NVP(m_LodMask);
	MaterialPtr mtl;
	ar >> boost::serialization::make_nvp("m_Material", mtl);
	SetMaterial(mtl);
	ar >> BOOST_SERIALIZATION_NVP(m_PxGeometryType);
	switch (m_PxGeometryType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		my::Vector3 ShapePos;
		ar >> BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot;
		ar >> BOOST_SERIALIZATION_NVP(ShapeRot);
		float SphereRadius;
		ar >> BOOST_SERIALIZATION_NVP(SphereRadius);
		CreateSphereShape(ShapePos, ShapeRot, SphereRadius, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::ePLANE:
	{
		my::Vector3 ShapePos;
		ar >> BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot;
		ar >> BOOST_SERIALIZATION_NVP(ShapeRot);
		CreatePlaneShape(ShapePos, ShapeRot, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eCAPSULE:
	{
		my::Vector3 ShapePos;
		ar >> BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot;
		ar >> BOOST_SERIALIZATION_NVP(ShapeRot);
		float CapsuleRadius;
		ar >> BOOST_SERIALIZATION_NVP(CapsuleRadius);
		float CapsuleHalfHeight;
		ar >> BOOST_SERIALIZATION_NVP(CapsuleHalfHeight);
		CreateCapsuleShape(ShapePos, ShapeRot, CapsuleRadius, CapsuleHalfHeight, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eBOX:
	{
		my::Vector3 ShapePos;
		ar >> BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot;
		ar >> BOOST_SERIALIZATION_NVP(ShapeRot);
		my::Vector3 BoxHalfExtents;
		ar >> BOOST_SERIALIZATION_NVP(BoxHalfExtents);
		CreateBoxShape(ShapePos, ShapeRot, BoxHalfExtents.x, BoxHalfExtents.y, BoxHalfExtents.z, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	}
}

ComponentPtr Component::Clone(void) const
{
	std::stringstream sstr;
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(sstr, ".txt");
	*oa << boost::serialization::make_nvp(__FUNCTION__, shared_from_this());

	ComponentPtr ret;
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(sstr, ".txt");
	boost::dynamic_pointer_cast<NamedObjectSerializationContext>(ia)->make_unique = true;
	*ia >> boost::serialization::make_nvp(__FUNCTION__, ret);
	return ret;
}

void Component::RequestResource(void)
{
	m_Requested = true;

	if (m_Material)
	{
		m_Material->RequestResource();
	}
}

void Component::ReleaseResource(void)
{
	m_Requested = false;

	if (m_Material)
	{
		m_Material->ReleaseResource();
	}
}

void Component::SetMaterial(MaterialPtr material)
{
	if (m_Material)
	{
		if (IsRequested())
		{
			m_Material->ReleaseResource();
		}

		m_Material->m_Cmp = NULL;
	}

	m_Material = material;

	if (m_Material)
	{
		m_Material->m_Cmp = this;

		if (IsRequested())
		{
			m_Material->RequestResource();
		}
	}
}

physx::PxMaterial * Component::CreatePhysxMaterial(float staticFriction, float dynamicFriction, float restitution)
{
	// ! materialIndices[0] = Ps::to16((static_cast<NpMaterial*>(materials[0]))->getHandle());
	std::string Key = str_printf("physx material %f %f %f", staticFriction, dynamicFriction, restitution);
	CriticalSectionLock lock(PhysxSdk::getSingleton().m_CollectionObjsSec);
	std::pair<PhysxSdk::CollectionObjMap::iterator, bool> obj_res = PhysxSdk::getSingleton().m_CollectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
	if (obj_res.second)
	{
		obj_res.first->second.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(staticFriction, dynamicFriction, restitution), PhysxDeleter<physx::PxMaterial>());
	}
	lock.Unlock();

	return obj_res.first->second->is<physx::PxMaterial>();
}

void Component::CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, float staticFriction, float dynamicFriction, float restitution)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial * material = CreatePhysxMaterial(staticFriction, dynamicFriction, restitution);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxBoxGeometry(hx, hy, hz), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	SetShapeLocalPose(Bone(pos, rot));

	m_PxShape->userData = this;

	m_PxGeometryType = physx::PxGeometryType::eBOX;

	if (m_Actor && m_Actor->m_PxActor)
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, float staticFriction, float dynamicFriction, float restitution)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(staticFriction, dynamicFriction, restitution);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxCapsuleGeometry(radius, halfHeight), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	SetShapeLocalPose(Bone(pos, rot));

	m_PxShape->userData = this;

	m_PxGeometryType = physx::PxGeometryType::eCAPSULE;

	if (m_Actor && m_Actor->m_PxActor)
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, float staticFriction, float dynamicFriction, float restitution)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(staticFriction, dynamicFriction, restitution);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxPlaneGeometry(), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	SetShapeLocalPose(Bone(pos, rot));

	m_PxShape->userData = this;

	m_PxGeometryType = physx::PxGeometryType::ePLANE;

	if (m_Actor && m_Actor->m_PxActor)
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float staticFriction, float dynamicFriction, float restitution)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(staticFriction, dynamicFriction, restitution);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxSphereGeometry(radius), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	SetShapeLocalPose(Bone(pos, rot));

	m_PxShape->userData = this;

	m_PxGeometryType = physx::PxGeometryType::eSPHERE;

	if (m_Actor && m_Actor->m_PxActor)
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::SetSimulationFilterWord0(unsigned int filterWord0)
{
	_ASSERT(m_PxShape);
	physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
	m_PxShape->setSimulationFilterData(filter_data);
}

unsigned int Component::GetSimulationFilterWord0(void) const
{
	return m_PxShape ? m_PxShape->getSimulationFilterData().word0 : 0;
}

void Component::SetQueryFilterWord0(unsigned int filterWord0)
{
	_ASSERT(m_PxShape);
	physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
	m_PxShape->setQueryFilterData(filter_data);
}

unsigned int Component::GetQueryFilterWord0(void) const
{
	return m_PxShape ? m_PxShape->getQueryFilterData().word0 : 0;
}

void Component::SetShapeFlags(unsigned int Flags)
{
	_ASSERT(m_PxShape);
	m_PxShape->setFlags(physx::PxShapeFlags(Flags));
}

unsigned int Component::GetShapeFlags(void) const
{
	return m_PxShape ? (unsigned int)m_PxShape->getFlags() : 0;
}

physx::PxGeometryType::Enum Component::GetGeometryType(void) const
{
	return m_PxShape ? m_PxShape->getGeometryType() : m_PxGeometryType;
}

void Component::SetShapeLocalPose(const my::Bone & pose)
{
	_ASSERT(m_PxShape);
	m_PxShape->setLocalPose((physx::PxTransform&)pose);
}

my::Bone Component::GetShapeLocalPose(void) const
{
	_ASSERT(m_PxShape);
	return (my::Bone&)m_PxShape->getLocalPose();
}

void Component::ClearShape(void)
{
	if (m_PxShape)
	{
		if (m_Actor && m_Actor->m_PxActor)
		{
			_ASSERT(m_PxShape->getActor() == m_Actor->m_PxActor.get());

			m_Actor->m_PxActor->detachShape(*m_PxShape);
		}

		_ASSERT(!m_PxShape->getActor());

		m_PxShape.reset();
	}

	m_PxGeometryType = physx::PxGeometryType::eINVALID;
}

unsigned int Component::GetSiblingId(void) const
{
	if (m_Actor)
	{
		Actor::ComponentPtrList::iterator self_iter = boost::find_if(m_Actor->m_Cmps, boost::bind(std::equal_to<const Component*>(), this, boost::bind(&ComponentPtr::get, boost::placeholders::_1)));
		_ASSERT(self_iter != m_Actor->m_Cmps.end());
		return std::distance(m_Actor->m_Cmps.begin(), self_iter);
	}
	return 0;
}

void Component::SetSiblingId(unsigned int i)
{
	if (m_Actor && i < m_Actor->m_Cmps.size())
	{
		int sibling_id = GetSiblingId();
		if (i < sibling_id)
		{
			std::rotate(m_Actor->m_Cmps.rend() - sibling_id - 1, m_Actor->m_Cmps.rend() - sibling_id, m_Actor->m_Cmps.rend() - i);
		}
		else if (i > sibling_id)
		{
			std::rotate(m_Actor->m_Cmps.begin() + sibling_id, m_Actor->m_Cmps.begin() + sibling_id + 1, m_Actor->m_Cmps.begin() + i + 1);
		}
	}
}

const my::Vector3 MeshComponent::DefaultCollisionMaterial(0.5f, 0.5f, 0.5f);

MeshComponent::~MeshComponent(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

template<class Archive>
void MeshComponent::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_MeshPath);
	ar << BOOST_SERIALIZATION_NVP(m_MeshSubMeshId);
	ar << BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar << BOOST_SERIALIZATION_NVP(m_InstanceType);
	switch (m_PxGeometryType)
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		ar << BOOST_SERIALIZATION_NVP(m_PxMeshPath);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eCONVEXMESH:
	{
		ar << BOOST_SERIALIZATION_NVP(m_PxMeshPath);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	}
}

template<class Archive>
void MeshComponent::load(Archive & ar, const unsigned int version)
{
	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshPath);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshSubMeshId);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar >> BOOST_SERIALIZATION_NVP(m_InstanceType);
	switch (m_PxGeometryType)
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		std::string PxMeshPath;
		ar >> boost::serialization::make_nvp("m_PxMeshPath", PxMeshPath);
		CreateTriangleMeshShape(NULL, PxMeshPath.c_str());
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eCONVEXMESH:
	{
		std::string PxMeshPath;
		ar >> boost::serialization::make_nvp("m_PxMeshPath", PxMeshPath);
		CreateConvexMeshShape(NULL, PxMeshPath.c_str(), true);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	}
}

void MeshComponent::OnResetShader(void)
{
	handle_World = NULL;
	handle_dualquat = NULL;
}

void MeshComponent::OnMeshReady(my::DeviceResourceBasePtr res)
{
	OgreMeshPtr mesh = boost::dynamic_pointer_cast<my::OgreMesh>(res);

	if (m_MeshSubMeshId >= mesh->m_AttribTable.size())
	{
		THROW_CUSEXCEPTION(str_printf("invalid sub mesh id %d for %s", m_MeshSubMeshId, res->m_Key));
	}

	m_Mesh = mesh;
}

MeshComponent::PhysxBaseResource::~PhysxBaseResource(void)
{
	if (m_ptr)
	{
		switch (m_ptr->getConcreteType())
		{
		case physx::PxConcreteType::eTRIANGLE_MESH_BVH33:
		{
			_ASSERT(m_ptr->is<physx::PxTriangleMesh>()->getReferenceCount() == 1);
			break;
		}
		case physx::PxConcreteType::eCONVEX_MESH:
		{
			_ASSERT(m_ptr->is<physx::PxConvexMesh>()->getReferenceCount() == 1);
			break;
		}
		default:
			_ASSERT(false);
			break;
		}

		m_ptr->release();
	}
}

void MeshComponent::PhysxBaseResource::Create(physx::PxBase* ptr)
{
	_ASSERT(NULL == m_ptr);

	m_ptr = ptr;
}

class PhysxTriangleMeshIORequest : public my::IORequest
{
protected:
	std::string m_path;

public:
	PhysxTriangleMeshIORequest(const char * path, int Priority)
		: IORequest(Priority)
		, m_path(path)
	{
	}

	virtual void PhysxTriangleMeshIORequest::LoadResource(void)
	{
		if (my::ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
		{
			PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(m_path.c_str()));
			MeshComponent::PhysxBaseResourcePtr res(new MeshComponent::PhysxBaseResource());
			res->Create(PhysxSdk::getSingleton().m_sdk->createTriangleMesh(readBuffer));
			m_res = res;
		}
	}

	virtual void PhysxTriangleMeshIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if (!m_res)
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
	}
};

class PhysxConvexMeshIORequest : public my::IORequest
{
protected:
	std::string m_path;

public:
	PhysxConvexMeshIORequest(const char* path, int Priority)
		: IORequest(Priority)
		, m_path(path)
	{
	}

	virtual void PhysxConvexMeshIORequest::LoadResource(void)
	{
		if (my::ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
		{
			PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(m_path.c_str()));
			MeshComponent::PhysxBaseResourcePtr res(new MeshComponent::PhysxBaseResource());
			res->Create(PhysxSdk::getSingleton().m_sdk->createConvexMesh(readBuffer));
			m_res = res;
		}
	}

	virtual void PhysxConvexMeshIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if (!m_res)
		{
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
	}
};

void MeshComponent::OnPxMeshReady(my::DeviceResourceBasePtr res, physx::PxGeometryType::Enum type)
{
	_ASSERT(!m_PxMesh);

	m_PxMesh = boost::dynamic_pointer_cast<PhysxBaseResource>(res);

	_ASSERT(m_Actor && !m_PxShape && m_PxGeometryType == type);

	physx::PxMaterial* material = CreatePhysxMaterial(DefaultCollisionMaterial.x, DefaultCollisionMaterial.y, DefaultCollisionMaterial.z);

	switch (type)
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
		m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxTriangleMeshGeometry(m_PxMesh->m_ptr->is<physx::PxTriangleMesh>(), mesh_scaling, physx::PxMeshGeometryFlags()),
			*material, true, physx::PxShapeFlags(m_DescShapeFlags)), PhysxDeleter<physx::PxShape>());
		m_PxShape->userData = this;
		SetSimulationFilterWord0(m_DescSimulationFilterWord0);
		SetQueryFilterWord0(m_DescQueryFilterWord0);
		if (m_Actor && m_Actor->m_PxActor)
		{
			m_Actor->m_PxActor->attachShape(*m_PxShape);
		}
		break;
	}
	case physx::PxGeometryType::eCONVEXMESH:
	{
		physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
		m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxConvexMeshGeometry(m_PxMesh->m_ptr->is<physx::PxConvexMesh>(), mesh_scaling, physx::PxConvexMeshGeometryFlags()),
			*material, true, physx::PxShapeFlags(m_DescShapeFlags)), PhysxDeleter<physx::PxShape>());
		m_PxShape->userData = this;
		SetSimulationFilterWord0(m_DescSimulationFilterWord0);
		SetQueryFilterWord0(m_DescQueryFilterWord0);
		if (m_Actor && m_Actor->m_PxActor)
		{
			m_Actor->m_PxActor->attachShape(*m_PxShape);
		}
		break;
	}
	default:
		THROW_CUSEXCEPTION("error");
	}
}

void MeshComponent::RequestResource(void)
{
	Component::RequestResource();

	if (!m_MeshPath.empty())
	{
		_ASSERT(!m_Mesh);

		my::ResourceMgr::getSingleton().LoadMeshAsync(m_MeshPath.c_str(), boost::bind(&MeshComponent::OnMeshReady, this, boost::placeholders::_1), (m_LodMask & LOD0) ? ResPriorityLod0 : (m_LodMask & LOD1) ? ResPriorityLod1 : ResPriorityLod2);
	}

	_ASSERT(!PhysxSdk::getSingleton().m_RenderTickMuted);

	if (!m_PxMeshPath.empty())
	{
		_ASSERT(!m_PxMesh);

		_ASSERT(!m_PxShape);

		switch (m_PxGeometryType)
		{
		case physx::PxGeometryType::eTRIANGLEMESH:
		{
			IORequestPtr request(new PhysxTriangleMeshIORequest(m_PxMeshPath.c_str(), INT_MAX));
			my::ResourceMgr::getSingleton().LoadIORequestAsync(m_PxMeshPath, request, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, physx::PxGeometryType::eTRIANGLEMESH));
			break;
		}
		case physx::PxGeometryType::eCONVEXMESH:
		{
			IORequestPtr request(new PhysxConvexMeshIORequest(m_PxMeshPath.c_str(), INT_MAX));
			my::ResourceMgr::getSingleton().LoadIORequestAsync(m_PxMeshPath, request, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, physx::PxGeometryType::eCONVEXMESH));
			break;
		}
		default:
			THROW_CUSEXCEPTION("error");
		}
	}
}

void MeshComponent::ReleaseResource(void)
{
	Component::ReleaseResource();

	if (!m_MeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_MeshPath.c_str(), boost::bind(&MeshComponent::OnMeshReady, this, boost::placeholders::_1));

		m_Mesh.reset();
	}

	_ASSERT(!PhysxSdk::getSingleton().m_RenderTickMuted);

	if (!m_PxMeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_PxMeshPath, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, m_PxGeometryType));

		if (m_PxMesh)
		{
			_ASSERT(m_PxShape);

			if (m_Actor->m_PxActor)
			{
				_ASSERT(m_PxShape->getActor() == m_Actor->m_PxActor.get());

				m_Actor->m_PxActor->detachShape(*m_PxShape);
			}

			m_PxShape.reset();

			m_PxMesh.reset();
		}
	}
}

void MeshComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_InstanceType != InstanceTypeBatch ? m_Actor->m_World : Matrix4::identity);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	Animator* animator = m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4 ? m_Actor->GetFirstComponent<Animator>() : NULL;

	if (animator && !animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray(handle_dualquat, &animator->m_DualQuats[0], animator->m_DualQuats.size());
	}
}

void MeshComponent::Update(float fElapsedTime)
{
}

my::AABB MeshComponent::CalculateAABB(void) const
{
	if (!m_Mesh)
	{
		return Component::CalculateAABB();
	}
	return m_Mesh->CalculateAABB(m_MeshSubMeshId);
}

void MeshComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_Actor);

	if (m_Mesh)
	{
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
			Animator* animator = m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4 ? m_Actor->GetFirstComponent<Animator>() : NULL;

			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					D3DXMACRO macros[4] = { { "MESH_TYPE", "0" }, { 0 } };
					int j = 1;
					if (m_InstanceType == InstanceTypeInstance)
					{
						macros[j++].Name = "INSTANCE";
					}
					else if (animator && !animator->m_DualQuats.empty())
					{
						macros[j++].Name = "SKELETON";
					}
					my::Effect* shader = pipeline->QueryShader(m_Material->m_Shader.c_str(), macros, PassID);
					if (shader)
					{
						if (!handle_World)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
						}

						if (!handle_dualquat && animator && !animator->m_DualQuats.empty())
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
							BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));

							m_Material->OnResetShader();
						}

						switch (m_InstanceType)
						{
						case InstanceTypeInstance:
							pipeline->PushMeshInstance(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), 0);
							break;
						case InstanceTypeBatch:
							pipeline->PushMeshBatch(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), 0);
							break;
						default:
							pipeline->PushMesh(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), 0);
							break;
						}
					}
				}
			}
		}
	}
}

void MeshComponent::CreateTriangleMeshShape(my::OgreMesh * mesh, const char * TriangleMeshPath)
{
	_ASSERT(!m_PxShape);

	if (mesh)
	{
		D3DXATTRIBUTERANGE rang = { 0, 0, mesh->GetNumFaces(), 0, mesh->GetNumVertices() };
		physx::PxTriangleMeshDesc desc;
		desc.points.count = rang.VertexStart + rang.VertexCount;
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.points.data = &mesh->m_VertexElems.GetPosition(mesh->LockVertexBuffer());
		desc.triangles.count = rang.FaceCount;
		if (mesh->GetOptions() & D3DXMESH_32BIT)
		{
			desc.triangles.stride = 3 * sizeof(DWORD);
		}
		else
		{
			desc.triangles.stride = 3 * sizeof(WORD);
			desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		}
		desc.triangles.data = (unsigned char*)mesh->LockIndexBuffer() + rang.FaceStart * desc.triangles.stride;

		PhysxFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(TriangleMeshPath).c_str());
		bool status = PhysxSdk::getSingleton().m_Cooking->cookTriangleMesh(desc, writeBuffer);
		mesh->UnlockIndexBuffer();
		mesh->UnlockVertexBuffer();
		if (!status)
		{
			THROW_CUSEXCEPTION("cookTriangleMesh failed");
		}
	}

	_ASSERT(m_PxMeshPath.empty());

	m_PxMeshPath.assign(TriangleMeshPath);

	m_PxGeometryType = physx::PxGeometryType::eTRIANGLEMESH;

	if (IsRequested())
	{
		IORequestPtr request(new PhysxTriangleMeshIORequest(m_PxMeshPath.c_str(), INT_MAX));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(m_PxMeshPath, request, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, physx::PxGeometryType::eTRIANGLEMESH));
	}
}

void MeshComponent::CreateConvexMeshShape(my::OgreMesh * mesh, const char * ConvexMeshPath, bool bInflateConvex)
{
	_ASSERT(!m_PxShape);

	if (mesh)
	{
		D3DXATTRIBUTERANGE rang = { 0, 0, mesh->GetNumFaces(), 0, mesh->GetNumVertices() };
		physx::PxConvexMeshDesc desc;
		desc.points.count = rang.VertexCount;
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.points.data = &mesh->m_VertexElems.GetPosition((unsigned char*)mesh->LockVertexBuffer() + rang.VertexStart * desc.points.stride);
		desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		if (bInflateConvex)
		{
			desc.flags |= physx::PxConvexFlag::eINFLATE_CONVEX;
		}
		desc.vertexLimit = 256;

		PhysxFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(ConvexMeshPath).c_str());
		bool status = PhysxSdk::getSingleton().m_Cooking->cookConvexMesh(desc, writeBuffer);
		mesh->UnlockVertexBuffer();
		if (!status)
		{
			THROW_CUSEXCEPTION("cookConvexMesh failed");
		}
	}

	_ASSERT(m_PxMeshPath.empty());

	m_PxMeshPath.assign(ConvexMeshPath);

	m_PxGeometryType = physx::PxGeometryType::eCONVEXMESH;

	if (IsRequested())
	{
		IORequestPtr request(new PhysxConvexMeshIORequest(m_PxMeshPath.c_str(), INT_MAX));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(m_PxMeshPath, request, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, physx::PxGeometryType::eCONVEXMESH));
	}
}

void MeshComponent::SetSimulationFilterWord0(unsigned int filterWord0)
{
	m_DescSimulationFilterWord0 = filterWord0;

	if (m_PxShape)
	{
		physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
		m_PxShape->setSimulationFilterData(filter_data);
	}
}

unsigned int MeshComponent::GetSimulationFilterWord0(void) const
{
	if (m_PxShape)
	{
		return m_PxShape->getSimulationFilterData().word0;
	}

	return m_DescSimulationFilterWord0;
}

void MeshComponent::SetQueryFilterWord0(unsigned int filterWord0)
{
	m_DescQueryFilterWord0 = filterWord0;

	if (m_PxShape)
	{
		physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
		m_PxShape->setQueryFilterData(filter_data);
	}
}

unsigned int MeshComponent::GetQueryFilterWord0(void) const
{
	if (m_PxShape)
	{
		return m_PxShape->getQueryFilterData().word0;
	}

	return m_DescQueryFilterWord0;
}

void MeshComponent::SetShapeFlags(unsigned int Flags)
{
	m_DescShapeFlags = Flags;

	if (m_PxShape)
	{
		m_PxShape->setFlags(physx::PxShapeFlags(Flags));
	}
}

unsigned int MeshComponent::GetShapeFlags(void) const
{
	if (m_PxShape)
	{
		return m_PxShape->getFlags();
	}

	return m_DescShapeFlags;
}

void MeshComponent::ClearShape(void)
{
	Component::ClearShape();

	m_PxMesh.reset();

	m_PxMeshPath.clear();

	//m_PxMeshTmp = NULL; // ! dont release
}

ClothComponent::~ClothComponent(void)
{
	_ASSERT(!m_Cloth || !m_Cloth->getScene());
}

namespace boost { 
	namespace serialization {
		template<class Archive>
		inline void serialize(Archive & ar, physx::PxClothParticle & t, const unsigned int file_version)
		{
			ar & BOOST_SERIALIZATION_NVP(t.pos.x);
			ar & BOOST_SERIALIZATION_NVP(t.pos.y);
			ar & BOOST_SERIALIZATION_NVP(t.pos.z);
			ar & BOOST_SERIALIZATION_NVP(t.invWeight);
		}

		template<class Archive>
		inline void serialize(Archive & ar, ClothComponent::ClothCollisionSpherePair & t, const unsigned int file_version)
		{
			ar & BOOST_SERIALIZATION_NVP(t.first.pos.x);
			ar & BOOST_SERIALIZATION_NVP(t.first.pos.y);
			ar & BOOST_SERIALIZATION_NVP(t.first.pos.z);
			ar & BOOST_SERIALIZATION_NVP(t.first.radius);
			ar & BOOST_SERIALIZATION_NVP(t.second);
		}
	}
}

template<class Archive>
void ClothComponent::save(Archive & ar, const unsigned int version) const
{
	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	const_cast<ClothComponent*>(this)->UpdateVertexData(const_cast<unsigned char*>(m_VertexData.data()), m_particles.data(), m_particles.size(), NULL);

	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	unsigned int VertexSize = m_VertexData.size();
	ar << BOOST_SERIALIZATION_NVP(VertexSize);
	ar << boost::serialization::make_nvp("m_VertexData", boost::serialization::binary_object((void *)&m_VertexData[0], VertexSize));
	ar << BOOST_SERIALIZATION_NVP(m_VertexStride);
	unsigned int IndexSize = m_IndexData.size();
	ar << BOOST_SERIALIZATION_NVP(IndexSize);
	ar << boost::serialization::make_nvp("m_IndexData", boost::serialization::binary_object((void *)&m_IndexData[0], IndexSize * sizeof(m_IndexData[0])));
	ar << BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar << BOOST_SERIALIZATION_NVP(m_VertexElems);
	unsigned int ParticleSize = m_particles.size();
	ar << BOOST_SERIALIZATION_NVP(ParticleSize);
	ar << boost::serialization::make_nvp("m_particles", boost::serialization::binary_object((void*)&m_particles[0], ParticleSize * sizeof(m_particles[0])));
	ar << BOOST_SERIALIZATION_NVP(m_ClothFabricPath);
	unsigned int ClothFlags = GetClothFlags();
	ar << BOOST_SERIALIZATION_NVP(ClothFlags);
	float SolverFrequency = m_Cloth->getSolverFrequency();
	ar << BOOST_SERIALIZATION_NVP(SolverFrequency);
	float StiffnessFrequency = m_Cloth->getStiffnessFrequency();
	ar << BOOST_SERIALIZATION_NVP(StiffnessFrequency);
	Vector3 DampingCoefficient = (Vector3&)m_Cloth->getDampingCoefficient();
	ar << BOOST_SERIALIZATION_NVP(DampingCoefficient);
	Vector3 LinearDragCoefficient = (Vector3&)m_Cloth->getLinearDragCoefficient();
	ar << BOOST_SERIALIZATION_NVP(LinearDragCoefficient);
	Vector3 AngularDragCoefficient = (Vector3&)m_Cloth->getAngularDragCoefficient();
	ar << BOOST_SERIALIZATION_NVP(AngularDragCoefficient);
	Vector3 LinearInertiaScale = (Vector3&)m_Cloth->getLinearInertiaScale();
	ar << BOOST_SERIALIZATION_NVP(LinearInertiaScale);
	Vector3 AngularInertiaScale = (Vector3&)m_Cloth->getAngularInertiaScale();
	ar << BOOST_SERIALIZATION_NVP(AngularInertiaScale);
	Vector3 CentrifugalInertiaScale = (Vector3&)m_Cloth->getCentrifugalInertiaScale();
	ar << BOOST_SERIALIZATION_NVP(CentrifugalInertiaScale);
	physx::PxClothStretchConfig stretchConfig = m_Cloth->getStretchConfig(physx::PxClothFabricPhaseType::eVERTICAL);
	ar << BOOST_SERIALIZATION_NVP(stretchConfig.stiffness);
	ar << BOOST_SERIALIZATION_NVP(stretchConfig.stiffnessMultiplier);
	ar << BOOST_SERIALIZATION_NVP(stretchConfig.compressionLimit);
	ar << BOOST_SERIALIZATION_NVP(stretchConfig.stretchLimit);
	physx::PxClothTetherConfig tetherConfig = m_Cloth->getTetherConfig();
	ar << BOOST_SERIALIZATION_NVP(tetherConfig.stiffness);
	ar << BOOST_SERIALIZATION_NVP(tetherConfig.stretchLimit);
	std::vector<Vector3> VirtualParticleWeights(m_Cloth->getNbVirtualParticleWeights());
	m_Cloth->getVirtualParticleWeights((physx::PxVec3*)VirtualParticleWeights.data());
	ar << BOOST_SERIALIZATION_NVP(VirtualParticleWeights);
	std::vector<unsigned int> VirtualParticles(m_Cloth->getNbVirtualParticles() * 4);
	m_Cloth->getVirtualParticles(VirtualParticles.data());
	unsigned int VirtualParticleSize = VirtualParticles.size();
	ar << BOOST_SERIALIZATION_NVP(VirtualParticleSize);
	ar << boost::serialization::make_nvp("VirtualParticle", boost::serialization::binary_object((void*)&VirtualParticles[0], VirtualParticleSize * sizeof(VirtualParticles[0])));
	_ASSERT(m_ClothSphereBones.size() == m_Cloth->getNbCollisionSpheres());
	ar << BOOST_SERIALIZATION_NVP(m_ClothSphereBones);
	unsigned int NbCapsules = m_Cloth->getNbCollisionCapsules();
	std::vector<physx::PxU32> ClothCapsules(NbCapsules * 2);
	m_Cloth->getCollisionData(NULL, ClothCapsules.data(), NULL, NULL, NULL);
	ar << BOOST_SERIALIZATION_NVP(ClothCapsules);
}

template<class Archive>
void ClothComponent::load(Archive & ar, const unsigned int version)
{
	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	unsigned int VertexSize;
	ar >> BOOST_SERIALIZATION_NVP(VertexSize);
	m_VertexData.resize(VertexSize);
	ar >> boost::serialization::make_nvp("m_VertexData", boost::serialization::binary_object((void *)&m_VertexData[0], VertexSize));
	ar >> BOOST_SERIALIZATION_NVP(m_VertexStride);
	unsigned int IndexSize;
	ar >> BOOST_SERIALIZATION_NVP(IndexSize);
	m_IndexData.resize(IndexSize);
	ar >> boost::serialization::make_nvp("m_IndexData", boost::serialization::binary_object((void *)&m_IndexData[0], IndexSize * sizeof(m_IndexData[0])));
	ar >> BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar >> BOOST_SERIALIZATION_NVP(m_VertexElems);
	unsigned int ParticleSize;
	ar >> BOOST_SERIALIZATION_NVP(ParticleSize);
	m_particles.resize(ParticleSize);
	ar >> boost::serialization::make_nvp("m_particles", boost::serialization::binary_object((void*)&m_particles[0], ParticleSize * sizeof(m_particles[0])));

	std::string ClothFabricPath;
	ar >> boost::serialization::make_nvp("m_ClothFabricPath", ClothFabricPath);
	CreateClothFromMesh(ClothFabricPath.c_str(), my::OgreMeshPtr(), Vector3(0, 0, 0));

	unsigned int ClothFlags;
	ar >> BOOST_SERIALIZATION_NVP(ClothFlags);
	SetClothFlags(ClothFlags);

	float SolverFrequency;
	ar >> BOOST_SERIALIZATION_NVP(SolverFrequency);
	m_Cloth->setSolverFrequency(SolverFrequency);
	float StiffnessFrequency;
	ar >> BOOST_SERIALIZATION_NVP(StiffnessFrequency);
	m_Cloth->setStiffnessFrequency(StiffnessFrequency);
	Vector3 DampingCoefficient;
	ar >> BOOST_SERIALIZATION_NVP(DampingCoefficient);
	m_Cloth->setDampingCoefficient((physx::PxVec3&)DampingCoefficient);
	Vector3 LinearDragCoefficient;
	ar >> BOOST_SERIALIZATION_NVP(LinearDragCoefficient);
	m_Cloth->setLinearDragCoefficient((physx::PxVec3&)LinearDragCoefficient);
	Vector3 AngularDragCoefficient;
	ar >> BOOST_SERIALIZATION_NVP(AngularDragCoefficient);
	m_Cloth->setAngularDragCoefficient((physx::PxVec3&)AngularDragCoefficient);
	Vector3 LinearInertiaScale;
	ar >> BOOST_SERIALIZATION_NVP(LinearInertiaScale);
	m_Cloth->setLinearInertiaScale((physx::PxVec3&)LinearInertiaScale);
	Vector3 AngularInertiaScale;
	ar >> BOOST_SERIALIZATION_NVP(AngularInertiaScale);
	m_Cloth->setAngularInertiaScale((physx::PxVec3&)AngularInertiaScale);
	Vector3 CentrifugalInertiaScale;
	ar >> BOOST_SERIALIZATION_NVP(CentrifugalInertiaScale);
	m_Cloth->setCentrifugalInertiaScale((physx::PxVec3&)CentrifugalInertiaScale);

	physx::PxClothStretchConfig stretchConfig;
	ar >> BOOST_SERIALIZATION_NVP(stretchConfig.stiffness);
	ar >> BOOST_SERIALIZATION_NVP(stretchConfig.stiffnessMultiplier);
	ar >> BOOST_SERIALIZATION_NVP(stretchConfig.compressionLimit);
	ar >> BOOST_SERIALIZATION_NVP(stretchConfig.stretchLimit);
	m_Cloth->setStretchConfig(physx::PxClothFabricPhaseType::eVERTICAL, stretchConfig);
	physx::PxClothTetherConfig tetherConfig;
	ar >> BOOST_SERIALIZATION_NVP(tetherConfig.stiffness);
	ar >> BOOST_SERIALIZATION_NVP(tetherConfig.stretchLimit);
	m_Cloth->setTetherConfig(tetherConfig);

	std::vector<Vector3> VirtualParticleWeights;
	ar >> BOOST_SERIALIZATION_NVP(VirtualParticleWeights);
	unsigned int VirtualParticleSize;
	ar >> BOOST_SERIALIZATION_NVP(VirtualParticleSize);
	std::vector<unsigned int> VirtualParticles(VirtualParticleSize);
	ar >> boost::serialization::make_nvp("VirtualParticle", boost::serialization::binary_object((void*)&VirtualParticles[0], VirtualParticleSize * sizeof(VirtualParticles[0])));
	if (!VirtualParticles.empty())
	{
		m_Cloth->setVirtualParticles(VirtualParticles.size() / 4, VirtualParticles.data(), VirtualParticleWeights.size(), (physx::PxVec3*)VirtualParticleWeights.data());
	}

	ar >> BOOST_SERIALIZATION_NVP(m_ClothSphereBones);
	for (int i = 0; i < m_ClothSphereBones.size(); i++)
	{
		m_Cloth->addCollisionSphere(m_ClothSphereBones[i].first);
	}
	std::vector<physx::PxU32> ClothCapsules;
	ar >> BOOST_SERIALIZATION_NVP(ClothCapsules);
	for (int i = 0; i < ClothCapsules.size(); i += 2)
	{
		m_Cloth->addCollisionCapsule(ClothCapsules[i + 0], ClothCapsules[i + 1]);
	}
}

void ClothComponent::OnResetShader(void)
{
	handle_World = NULL;
}

void ClothComponent::CreateClothFromMesh(const char * ClothFabricPath, my::OgreMeshPtr mesh, const my::Vector3 & gravity)
{
	if (m_VertexData.empty())
	{
		_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

		D3DXATTRIBUTERANGE rang = { 0, 0, mesh->GetNumFaces(), 0, mesh->GetNumVertices() };
		m_VertexStride = mesh->GetNumBytesPerVertex();
		m_VertexData.resize(rang.VertexCount * m_VertexStride);
		memcpy(&m_VertexData[0], (unsigned char*)mesh->LockVertexBuffer() + rang.VertexStart * m_VertexStride, m_VertexData.size());
		mesh->UnlockVertexBuffer();

		m_IndexData.resize(rang.FaceCount * 3);
		if (mesh->GetNumVertices() > USHRT_MAX)
		{
			THROW_CUSEXCEPTION(str_printf("create deformation mesh with overflow index size %Iu", m_IndexData.size()));
		}
		VOID* pIndices = mesh->LockIndexBuffer();
		for (unsigned int face_i = 0; face_i < rang.FaceCount; face_i++)
		{
			// ! take care of rang.VertexStart
			if (mesh->GetOptions() & D3DXMESH_32BIT)
			{
				m_IndexData[face_i * 3 + 0] = (WORD) * ((DWORD*)pIndices + rang.FaceStart * 3 + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = (WORD) * ((DWORD*)pIndices + rang.FaceStart * 3 + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = (WORD) * ((DWORD*)pIndices + rang.FaceStart * 3 + face_i * 3 + 2);
			}
			else
			{
				m_IndexData[face_i * 3 + 0] = *((WORD*)pIndices + rang.FaceStart * 3 + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = *((WORD*)pIndices + rang.FaceStart * 3 + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = *((WORD*)pIndices + rang.FaceStart * 3 + face_i * 3 + 2);
			}
		}
		mesh->UnlockIndexBuffer();

		m_VertexElems = mesh->m_VertexElems;
		std::vector<D3DVERTEXELEMENT9> velist(MAX_FVF_DECL_SIZE);
		mesh->GetDeclaration(&velist[0]);
		HRESULT hr;
		if (FAILED(hr = mesh->GetDevice()->CreateVertexDeclaration(&velist[0], &m_Decl)))
		{
			THROW_D3DEXCEPTION(hr);
		}

		if (m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Type != D3DDECLTYPE_D3DCOLOR)
		{
			THROW_CUSEXCEPTION("mesh must have vertex color for cloth weight");
		}

		m_particles.resize(rang.VertexCount);
		unsigned char* pVertices = (unsigned char*)&m_VertexData[0];
		for (unsigned int i = 0; i < m_particles.size(); i++) {
			unsigned char* pVertex = pVertices + i * m_VertexStride;
			m_particles[i].pos = (physx::PxVec3&)m_VertexElems.GetPosition(pVertex);
			D3DXCOLOR Weight(m_VertexElems.GetColor(pVertex));
			if (Weight.r > EPSILON_E6)
			{
				m_particles[i].invWeight = 1 / Weight.r;
			}
			else
			{
				m_particles[i].invWeight = 0;
			}
		}
	}

	if (!my::ResourceMgr::getSingleton().CheckPath(ClothFabricPath))
	{
		_ASSERT(!m_VertexData.empty());

		physx::PxClothMeshDesc desc;
		desc.points.data = &m_particles[0].pos;
		desc.points.count = m_particles.size();
		desc.points.stride = sizeof(m_particles[0]);
		desc.invMasses.data = &m_particles[0].invWeight;
		desc.invMasses.count = m_particles.size();
		desc.invMasses.stride = sizeof(m_particles[0]);
		desc.triangles.data = &m_IndexData[0];
		desc.triangles.count = m_IndexData.size() / 3;
		desc.triangles.stride = 3 * sizeof(unsigned short);
		desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;

		physx::PxClothMeshQuadifier quadifier(desc);
		physx::PxClothMeshDesc desc2 = quadifier.getDescriptor();
		PhysxFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(ClothFabricPath).c_str());
		physx::PxClothFabricCooker cooker(desc2, (physx::PxVec3&)gravity, true);
		cooker.save(writeBuffer, true);
	}

	_ASSERT(m_ClothFabricPath.empty());

	m_ClothFabricPath.assign(ClothFabricPath);

	PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(ClothFabricPath));
	m_ClothFabric.reset(PhysxSdk::getSingleton().m_sdk->createClothFabric(readBuffer), PhysxDeleter<physx::PxClothFabric>());

	m_Cloth.reset(PhysxSdk::getSingleton().m_sdk->createCloth(
		physx::PxTransform(physx::PxIdentity), *m_ClothFabric, &m_particles[0], physx::PxClothFlags()), PhysxDeleter<physx::PxCloth>());
}

void ClothComponent::CreateVirtualParticles(int level)
{
	if (level < 1 || level > 5)
		return;

	physx::PxClothMeshDesc desc;
	desc.points.data = &m_particles[0].pos;
	desc.points.count = m_particles.size();
	desc.points.stride = sizeof(m_particles[0]);
	desc.invMasses.data = &m_particles[0].invWeight;
	desc.invMasses.count = m_particles.size();
	desc.invMasses.stride = sizeof(m_particles[0]);
	desc.triangles.data = &m_IndexData[0];
	desc.triangles.count = m_IndexData.size() / 3;
	desc.triangles.stride = 3 * sizeof(unsigned short);
	desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;

	physx::PxClothMeshQuadifier quadifier(desc);
	physx::PxClothMeshDesc desc2 = quadifier.getDescriptor();

	unsigned int edgeSampleCount[] = { 0, 0, 1, 1, 0, 1 };
	unsigned int triSampleCount[] = { 0, 1, 0, 1, 3, 3 };
	unsigned int quadSampleCount[] = { 0, 1, 0, 1, 4, 4 };

	unsigned int numEdgeSamples = edgeSampleCount[level];
	unsigned int numTriSamples = triSampleCount[level];
	unsigned int numQuadSamples = quadSampleCount[level];

	unsigned int numTriangles = desc2.triangles.count;
	unsigned char* triangles = (unsigned char*)desc2.triangles.data;

	unsigned int numQuads = desc2.quads.count;
	unsigned char* quads = (unsigned char*)desc2.quads.data;

	std::vector<unsigned int> indices;
	indices.reserve(numTriangles * (numTriSamples + 3 * numEdgeSamples)
		+ numQuads * (numQuadSamples + 4 * numEdgeSamples));

	typedef std::pair<unsigned int, unsigned int> Edge;
	std::set<Edge> edges;

	for (unsigned int i = 0; i < numTriangles; i++)
	{
		unsigned int v0, v1, v2;

		if (desc2.flags & physx::PxMeshFlag::e16_BIT_INDICES)
		{
			unsigned short* triangle = (unsigned short*)triangles;
			v0 = triangle[0];
			v1 = triangle[1];
			v2 = triangle[2];
		}
		else
		{
			unsigned int* triangle = (unsigned int*)triangles;
			v0 = triangle[0];
			v1 = triangle[1];
			v2 = triangle[2];
		}

		if (numTriSamples == 1)
		{
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(0);
		}

		if (numTriSamples == 3)
		{
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(1);

			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(v0);
			indices.push_back(1);

			indices.push_back(v2);
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(1);
		}

		if (numEdgeSamples == 1)
		{
			if (edges.insert(std::make_pair(v0, v1)).second)
			{
				indices.push_back(v0);
				indices.push_back(v1);
				indices.push_back(v2);
				indices.push_back(2);
			}

			if (edges.insert(std::make_pair(v1, v2)).second)
			{
				indices.push_back(v1);
				indices.push_back(v2);
				indices.push_back(v0);
				indices.push_back(2);
			}

			if (edges.insert(std::make_pair(v2, v0)).second)
			{
				indices.push_back(v2);
				indices.push_back(v0);
				indices.push_back(v1);
				indices.push_back(2);
			}
		}

		triangles += desc2.triangles.stride;
	}

	for (unsigned int i = 0; i < numQuads; i++)
	{
		unsigned int v0, v1, v2, v3;

		if (desc2.flags & physx::PxMeshFlag::e16_BIT_INDICES)
		{
			unsigned short* quad = (unsigned short*)quads;
			v0 = quad[0];
			v1 = quad[1];
			v2 = quad[2];
			v3 = quad[3];
		}
		else
		{
			unsigned int* quad = (unsigned int*)quads;
			v0 = quad[0];
			v1 = quad[1];
			v2 = quad[2];
			v3 = quad[3];
		}

		if (numQuadSamples == 1)
		{
			indices.push_back(v0);
			indices.push_back(v2);
			indices.push_back(v3);
			indices.push_back(2);
		}

		if (numQuadSamples == 4)
		{
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(1);

			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(v3);
			indices.push_back(1);

			indices.push_back(v2);
			indices.push_back(v3);
			indices.push_back(v0);
			indices.push_back(1);

			indices.push_back(v3);
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(1);
		}

		if (numEdgeSamples == 1)
		{
			if (edges.insert(std::make_pair(v0, v1)).second)
			{
				indices.push_back(v0);
				indices.push_back(v1);
				indices.push_back(v2);
				indices.push_back(2);
			}

			if (edges.insert(std::make_pair(v1, v2)).second)
			{
				indices.push_back(v1);
				indices.push_back(v2);
				indices.push_back(v3);
				indices.push_back(2);
			}

			if (edges.insert(std::make_pair(v2, v3)).second)
			{
				indices.push_back(v2);
				indices.push_back(v3);
				indices.push_back(v0);
				indices.push_back(2);
			}

			if (edges.insert(std::make_pair(v3, v0)).second)
			{
				indices.push_back(v3);
				indices.push_back(v0);
				indices.push_back(v1);
				indices.push_back(2);
			}
		}

		quads += desc2.quads.stride;
	}

	static const Vector3 gVirtualParticleWeights[] =
	{
		// center point
		Vector3(1.0f / 3, 1.0f / 3, 1.0f / 3),

		// off-center point
		Vector3(4.0f / 6, 1.0f / 6, 1.0f / 6),

		// edge point
		Vector3(1.0f / 2, 1.0f / 2, 0.0f),
	};

	m_Cloth->setVirtualParticles(indices.size() / 4,
		indices.data(), 3, (physx::PxVec3*)gVirtualParticleWeights);

	return;
}

void ClothComponent::RequestResource(void)
{
	Component::RequestResource();

	if (!m_Decl)
	{
		std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
		D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
		velist.push_back(ve_end);
		HRESULT hr;
		if (FAILED(hr = D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(&velist[0], &m_Decl)))
		{
			THROW_D3DEXCEPTION(hr);
		}
	}

	if (m_Cloth)
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

		_ASSERT(!m_Cloth->getScene());

		scene->m_PxScene->addActor(*m_Cloth);

		scene->m_EventPxThreadSubstep.connect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, boost::placeholders::_1));

		_ASSERT(m_Actor);

		Vector3 Pos, Scale; Quaternion Rot;
		m_Actor->m_World.Decompose(Scale, Rot, Pos);

		m_Cloth->setGlobalPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
	}
}

void ClothComponent::ReleaseResource(void)
{
	m_Decl.Release();

	if (m_Cloth)
	{
		PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

		_ASSERT(m_Cloth->getScene() == scene->m_PxScene.get());

		scene->m_PxScene->removeActor(*m_Cloth);

		scene->removeRenderActorsFromPhysicsActor(m_Cloth.get());

		scene->m_EventPxThreadSubstep.disconnect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, boost::placeholders::_1));
	}

	Component::ReleaseResource();
}

void ClothComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(!m_VertexData.empty());

	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);
}

void ClothComponent::SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot)
{
	if (m_Cloth)
	{
		m_Cloth->setTargetPose(physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
	}
}

my::AABB ClothComponent::CalculateAABB(void) const
{
	if (m_VertexData.empty())
	{
		return Component::CalculateAABB();
	}
	AABB ret = AABB::Invalid();
	unsigned char* pVertices = (unsigned char*)&m_VertexData[0];
	const unsigned int NumVertices = m_VertexData.size() / m_VertexStride;
	for (unsigned int i = 0; i < NumVertices; i++)
	{
		unsigned char* pVertex = pVertices + i * m_VertexStride;
		ret.unionSelf(m_VertexElems.GetPosition(pVertex));
	}
	return ret;
}

void ClothComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (!m_VertexData.empty())
	{
		_ASSERT(!m_VertexData.empty());
		_ASSERT(!m_IndexData.empty());
		_ASSERT(0 != m_VertexStride);
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					D3DXMACRO macros[2] = { { "MESH_TYPE", "0" }, { 0 } };
					my::Effect* shader = pipeline->QueryShader(m_Material->m_Shader.c_str(), macros, PassID);
					if (shader)
					{
						if (!handle_World)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
						}

						pipeline->PushIndexedPrimitiveUP(PassID, m_Decl, D3DPT_TRIANGLELIST,
							0, m_VertexData.size() / m_VertexStride, m_IndexData.size() / 3, &m_IndexData[0], D3DFMT_INDEX16, &m_VertexData[0], m_VertexStride, shader, this, m_Material.get(), 0);
					}
				}
			}
		}
	}
}

void ClothComponent::Update(float fElapsedTime)
{
	if (m_Cloth)
	{
		_ASSERT(m_particles.size() == m_VertexData.size() / m_VertexStride);
		physx::PxClothParticleData* readData = m_Cloth->lockParticleData(physx::PxDataAccessFlag::eREADABLE);
		if (readData)
		{
			Animator* animator = m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4 ? m_Actor->GetFirstComponent<Animator>() : NULL;

			UpdateVertexData(m_VertexData.data(), readData->particles, m_Cloth->getNbParticles(), animator);

			readData->unlock();
		}
	}
}

void ClothComponent::UpdateVertexData(unsigned char* pVertices, const physx::PxClothParticle* particles, unsigned int NbParticles, Animator* animator)
{
	if (animator && !animator->m_DualQuats.empty())
	{
		for (unsigned int i = 0; i < NbParticles; i++)
		{
			void* pVertex = pVertices + i * m_VertexStride;
			if (particles[i].invWeight == 0)
			{
				my::Vector3 pos = animator->m_DualQuats.TransformVertexWithDualQuaternionList(
					(my::Vector3&)m_particles[i].pos,
					m_VertexElems.GetBlendIndices(pVertex),
					m_VertexElems.GetBlendWeight(pVertex));
				m_VertexElems.SetPosition(pVertex, pos);
			}
			else
			{
				m_VertexElems.SetPosition(pVertex, (my::Vector3&)particles[i].pos);
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < NbParticles; i++)
		{
			void* pVertex = pVertices + i * m_VertexStride;
			m_VertexElems.SetPosition(pVertex, (my::Vector3&)particles[i].pos);
		}
	}

	my::OgreMesh::ComputeNormalFrame(
		pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);

	my::OgreMesh::ComputeTangentFrame(
		pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);
}

void ClothComponent::OnPxThreadSubstep(float dtime)
{
	_ASSERT(m_Actor);

	Animator* animator = m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4 ? m_Actor->GetFirstComponent<Animator>() : NULL;

	if (!m_ClothSphereBones.empty())
	{
		unsigned int NbSpheres = m_ClothSphereBones.size();
		boost::array<physx::PxClothCollisionSphere, 32> ClothSpheres;
		if (animator && !animator->m_DualQuats.empty())
		{
			for (unsigned int i = 0; i < NbSpheres; i++)
			{
				ClothSpheres[i].radius = m_ClothSphereBones[i].first.radius;
				if (m_ClothSphereBones[i].second >= 0)
				{
					Bone bone((Vector3&)m_ClothSphereBones[i].first.pos);
					bone.TransformSelf(animator->anim_pose[m_ClothSphereBones[i].second]);
					ClothSpheres[i].pos = (physx::PxVec3&)bone.m_position;
				}
				else
				{
					ClothSpheres[i].pos = m_ClothSphereBones[i].first.pos;
				}
			}
		}
		else
		{
			for (unsigned int i = 0; i < NbSpheres; i++)
			{
				ClothSpheres[i] = m_ClothSphereBones[i].first;
			}
		}

		m_Cloth->setCollisionSpheres(ClothSpheres.data(), NbSpheres);
	}

	physx::PxClothParticleData* readData = m_Cloth->lockParticleData(physx::PxDataAccessFlag::eWRITABLE);
	if (readData)
	{
		if (animator && !animator->m_DualQuats.empty())
		{
			physx::PxClothParticle* particles = readData->particles;
			unsigned int NbParticles = m_Cloth->getNbParticles();
			unsigned char* pVertices = &m_VertexData[0];
			for (unsigned int i = 0; i < NbParticles; i++)
			{
				void* pVertex = pVertices + i * m_VertexStride;
				if (particles[i].invWeight == 0)
				{
					my::Vector3 pos = animator->m_DualQuats.TransformVertexWithDualQuaternionList(
						(my::Vector3&)m_particles[i].pos,
						m_VertexElems.GetBlendIndices(pVertex),
						m_VertexElems.GetBlendWeight(pVertex));
					particles[i].pos = (physx::PxVec3&)pos;
				}
			}
		}
		readData->unlock();
	}
}

void ClothComponent::SetClothFlags(unsigned int Flags)
{
	m_Cloth->setClothFlags(physx::PxClothFlags(Flags));
}

unsigned int ClothComponent::GetClothFlags(void) const
{
	return (unsigned int)m_Cloth->getClothFlags();
}

void ClothComponent::SetExternalAcceleration(const my::Vector3& acceleration)
{
	m_Cloth->setExternalAcceleration((physx::PxVec3&)acceleration);
}

my::Vector3 ClothComponent::GetExternalAcceleration(void) const
{
	return (my::Vector3&)m_Cloth->getExternalAcceleration();
}

EmitterComponent::~EmitterComponent(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void EmitterComponent::OnMeshReady(my::DeviceResourceBasePtr res)
{
	OgreMeshPtr mesh = boost::dynamic_pointer_cast<my::OgreMesh>(res);

	if (m_MeshSubMeshId >= mesh->m_AttribTable.size())
	{
		THROW_CUSEXCEPTION(str_printf("invalid sub mesh id %d for %s", m_MeshSubMeshId, res->m_Key));
	}

	m_Mesh = mesh;
}

void EmitterComponent::RequestResource(void)
{
	Component::RequestResource();

	if (!m_MeshPath.empty())
	{
		_ASSERT(!m_Mesh);

		my::ResourceMgr::getSingleton().LoadMeshAsync(m_MeshPath.c_str(), boost::bind(&EmitterComponent::OnMeshReady, this, boost::placeholders::_1), (m_LodMask & LOD0) ? ResPriorityLod0 : (m_LodMask & LOD1) ? ResPriorityLod1 : ResPriorityLod2);
	}
}

void EmitterComponent::ReleaseResource(void)
{
	Component::ReleaseResource();

	if (!m_MeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_MeshPath.c_str(), boost::bind(&EmitterComponent::OnMeshReady, this, boost::placeholders::_1));

		m_Mesh.reset();

		m_Decl.Release();
	}
}

void EmitterComponent::OnResetShader(void)
{
	handle_World = NULL;

	handle_Tiles = NULL;
}

void EmitterComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	if (m_EmitterSpaceType == SpaceTypeWorld)
	{
		shader->SetMatrix(handle_World, my::Matrix4::identity);

		shader->SetFloatArray(handle_Scale, &my::Vector3::one.x, 3);
	}
	else
	{
		shader->SetMatrix(handle_World, m_Actor->m_World);

		shader->SetFloatArray(handle_Scale, &m_Actor->m_Scale.x, 3);
	}

	if (m_Tiles.x > 1 || m_Tiles.y > 1)
	{
		shader->SetIntArray(handle_Tiles, (int*)&m_Tiles.x, 2);
	}
}

void EmitterComponent::AddParticlePairToPipeline(
	RenderPipeline* pipeline,
	IDirect3DVertexBuffer9* pVB,
	IDirect3DIndexBuffer9* pIB,
	IDirect3DVertexDeclaration9* pDecl,
	UINT MinVertexIndex,
	UINT NumVertices,
	DWORD VertexStride,
	UINT StartIndex,
	UINT PrimitiveCount,
	unsigned int PassMask, my::Emitter::Particle* particles1, unsigned int particle_num1, my::Emitter::Particle* particles2, unsigned int particle_num2, LPARAM lparam)
{
	_ASSERT(m_Material && (m_Material->m_PassMask & PassMask));

	for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
	{
		if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
		{
			const char* num[] = { "0", "1", "2", "3", "4", "5", "6" };
			_ASSERT(_countof(num) > FaceTypeStretchedCamera);
			D3DXMACRO macros[5] = {
				{ "MESH_TYPE", "1" },
				{ "EMITTER_FACE_TYPE", num[m_EmitterFaceType] },
				{ "EMITTER_SPACE_TYPE", num[m_EmitterSpaceType]},
				{m_Tiles.x > 1 || m_Tiles.y > 1 ? "TILED" : NULL, NULL}, {0} };
			my::Effect* shader = pipeline->QueryShader(m_Material->m_Shader.c_str(), macros, PassID);
			if (shader)
			{
				if (!handle_World)
				{
					BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));

					BOOST_VERIFY(handle_Scale = shader->GetParameterByName(NULL, "g_Scale"));
				}

				if ((m_Tiles.x > 1 || m_Tiles.y > 1) && !handle_Tiles)
				{
					BOOST_VERIFY(handle_Tiles = shader->GetParameterByName(NULL, "g_Tiles"));
				}

				if (particle_num1 > 0)
				{
					pipeline->PushEmitter(PassID, pVB, pIB, pDecl, MinVertexIndex, NumVertices, VertexStride,
						StartIndex, PrimitiveCount, particles1, particle_num1, shader, this, m_Material.get(), lparam);
				}

				if (particle_num2 > 0)
				{
					pipeline->PushEmitter(PassID, pVB, pIB, pDecl, MinVertexIndex, NumVertices, VertexStride,
						StartIndex, PrimitiveCount, particles2, particle_num2, shader, this, m_Material.get(), lparam);
				}
			}
		}
	}
}

template<class Archive>
void CircularEmitter::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	Emitter::ParticleList::capacity_type Capacity;
	Capacity = m_ParticleList.capacity();
	ar << BOOST_SERIALIZATION_NVP(Capacity);
	boost::serialization::stl::save_collection<Archive, ParticleList>(ar, m_ParticleList);
}

template<class Archive>
void CircularEmitter::load(Archive& ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	Emitter::ParticleList::capacity_type Capacity;
	ar >> BOOST_SERIALIZATION_NVP(Capacity);
	m_ParticleList.set_capacity(Capacity);
	boost::serialization::item_version_type item_version(0);
	boost::serialization::collection_size_type count;
	ar >> BOOST_SERIALIZATION_NVP(count);
	ar >> BOOST_SERIALIZATION_NVP(item_version);
	m_ParticleList.resize(count);
	boost::serialization::stl::collection_load_impl<Archive, ParticleList>(ar, m_ParticleList, count, item_version);
}

void CircularEmitter::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		ParticleList::array_range array_one = m_ParticleList.array_one();

		ParticleList::array_range array_two = m_ParticleList.array_two();

		switch (m_ParticlePrimitiveType)
		{
		case PrimitiveTypeTri:
		case PrimitiveTypeQuad:
		{
			_ASSERT(m_ParticlePrimitiveType < RenderPipeline::ParticlePrimitiveTypeCount);
			AddParticlePairToPipeline(pipeline, pipeline->m_ParticleVb.m_ptr, pipeline->m_ParticleIb.m_ptr, pipeline->m_ParticleIEDecl,
				RenderPipeline::m_ParticlePrimitiveInfo[RenderPipeline::ParticlePrimitiveQuad][RenderPipeline::ParticlePrimitiveMinVertexIndex],
				RenderPipeline::m_ParticlePrimitiveInfo[RenderPipeline::ParticlePrimitiveQuad][RenderPipeline::ParticlePrimitiveNumVertices],
				pipeline->m_ParticleVertStride,
				RenderPipeline::m_ParticlePrimitiveInfo[RenderPipeline::ParticlePrimitiveQuad][RenderPipeline::ParticlePrimitiveStartIndex],
				RenderPipeline::m_ParticlePrimitiveInfo[RenderPipeline::ParticlePrimitiveQuad][RenderPipeline::ParticlePrimitivePrimitiveCount],
				PassMask, array_one.first, array_one.second, array_two.first, array_two.second, 0);
			break;
		}
		case PrimitiveTypeMesh:
			if (m_Mesh)
			{
				if (!m_Decl)
				{
					std::vector<D3DVERTEXELEMENT9> ve = m_Mesh->m_VertexElems.BuildVertexElementList(0);
					std::vector<D3DVERTEXELEMENT9> ie = pipeline->m_ParticleInstanceElems.BuildVertexElementList(1);
					ve.insert(ve.end(), ie.begin(), ie.end());
					D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
					ve.push_back(ve_end);

					HRESULT hr;
					if (FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(&ve[0], &m_Decl)))
					{
						THROW_D3DEXCEPTION(hr);
					}
				}

				AddParticlePairToPipeline(pipeline, m_Mesh->m_Vb.m_ptr, m_Mesh->m_Ib.m_ptr, m_Decl,
					m_Mesh->m_AttribTable[m_MeshSubMeshId].VertexStart,
					m_Mesh->m_AttribTable[m_MeshSubMeshId].VertexCount,
					m_Mesh->GetNumBytesPerVertex(),
					m_Mesh->m_AttribTable[m_MeshSubMeshId].FaceStart * 3,
					m_Mesh->m_AttribTable[m_MeshSubMeshId].FaceCount,
					PassMask, array_one.first, array_one.second, array_two.first, array_two.second, 0);
			}
			break;
		}
	}
}

template<class Archive>
void SphericalEmitter::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	Emitter::ParticleList::capacity_type Capacity;
	Capacity = m_ParticleList.capacity();
	ar << BOOST_SERIALIZATION_NVP(Capacity);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnInterval);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnCount);
	ar << BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnInclination);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnBoneId);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnLocalPose);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleGravity);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleDamping);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleColorR);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleColorG);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleColorB);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleColorA);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleSizeX);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleSizeY);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleAngle);
}

template<class Archive>
void SphericalEmitter::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	Emitter::ParticleList::capacity_type Capacity;
	ar >> BOOST_SERIALIZATION_NVP(Capacity);
	m_ParticleList.set_capacity(Capacity);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnInterval);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnCount);
	ar >> BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnInclination);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnBoneId);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnLocalPose);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleGravity);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleDamping);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleColorR);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleColorG);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleColorB);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleColorA);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleSizeX);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleSizeY);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleAngle);
}

void SphericalEmitter::RequestResource(void)
{
	CircularEmitter::RequestResource();
}

void SphericalEmitter::ReleaseResource(void)
{
	CircularEmitter::ReleaseResource();
}

void SphericalEmitter::Update(float fElapsedTime)
{
	ParticleList::iterator first_part_iter = std::upper_bound(m_ParticleList.begin(), m_ParticleList.end(),
		m_ParticleLifeTime, boost::bind(std::greater<float>(), boost::placeholders::_1, boost::bind(&Particle::m_Time, boost::placeholders::_2)));
	if (first_part_iter != m_ParticleList.begin())
	{
		m_ParticleList.rerase(m_ParticleList.begin(), first_part_iter);
	}

	if (m_DelayRemoveTime > 0)
	{
		m_DelayRemoveTime -= fElapsedTime;
		if (m_DelayRemoveTime <= 0)
		{
			m_Actor->RemoveComponent(GetSiblingId());
			return;
		}
	}

	if (m_SpawnInterval > 0)
	{
		m_SpawnTime += fElapsedTime;

		for (; m_SpawnTime >= 0; m_SpawnTime -= m_SpawnInterval)
		{
			const Bone pose = m_EmitterSpaceType == SpaceTypeWorld ?
				m_Actor->GetAttachPose(m_SpawnBoneId, m_SpawnLocalPose.m_position, m_SpawnLocalPose.m_rotation) : m_SpawnLocalPose;
			for (int i = 0; i < m_SpawnCount; i++)
			{
				Spawn(
					Vector4(pose.m_rotation * Vector3(
						Random(-m_HalfSpawnArea.x, m_HalfSpawnArea.x),
						Random(-m_HalfSpawnArea.y, m_HalfSpawnArea.y),
						Random(-m_HalfSpawnArea.z, m_HalfSpawnArea.z)) + pose.m_position, 1),
					Vector4(pose.m_rotation * Vector3::PolarToCartesian(
						m_SpawnSpeed,
						Random(m_SpawnInclination.x, m_SpawnInclination.y),
						Random(m_SpawnAzimuth.x, m_SpawnAzimuth.y)), 0),
					Vector4(1, 1, 1, 1), Vector2(1, 1), 0, 0);
			}
		}
	}

	ParallelTaskManager::getSingleton().PushTask(this);
}

my::AABB SphericalEmitter::CalculateAABB(void) const
{
	return Component::CalculateAABB();
}

void SphericalEmitter::DoTask(void)
{
	_ASSERT(m_Actor);

	// ! take care of thread safe
	Emitter::ParticleList::iterator particle_iter = m_ParticleList.begin();
	for (; particle_iter != m_ParticleList.end(); particle_iter++)
	{
		particle_iter->m_Velocity.xyz = (particle_iter->m_Velocity.xyz + m_ParticleGravity * D3DContext::getSingleton().m_fElapsedTime) * powf(m_ParticleDamping, D3DContext::getSingleton().m_fElapsedTime);
		particle_iter->m_Position.xyz += particle_iter->m_Velocity.xyz * D3DContext::getSingleton().m_fElapsedTime;
		const float ParticleTime = particle_iter->m_Time + D3DContext::getSingleton().m_fElapsedTime;
		particle_iter->m_Color.x = m_ParticleColorR.Interpolate(ParticleTime);
		particle_iter->m_Color.y = m_ParticleColorG.Interpolate(ParticleTime);
		particle_iter->m_Color.z = m_ParticleColorB.Interpolate(ParticleTime);
		particle_iter->m_Color.w = m_ParticleColorA.Interpolate(ParticleTime);
		particle_iter->m_Size.x = m_ParticleSizeX.Interpolate(ParticleTime);
		particle_iter->m_Size.y = m_ParticleSizeY.Interpolate(ParticleTime);
		particle_iter->m_Angle = m_ParticleAngle.Interpolate(ParticleTime);
		particle_iter->m_Time = ParticleTime;
	}
}

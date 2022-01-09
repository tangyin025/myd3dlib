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
#include "mesh\GuTriangleMesh.h"
#include "convex\GuConvexMesh.h"

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
	ar << BOOST_SERIALIZATION_NVP(m_PxShapeGeometryType);
	switch (m_PxShapeGeometryType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		_ASSERT(m_PxShape && m_PxShapeGeometryType == m_PxShape->getGeometryType());
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
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::ePLANE:
	{
		_ASSERT(m_PxShape && m_PxShapeGeometryType == m_PxShape->getGeometryType());
		my::Vector3 ShapePos = (my::Vector3&)m_PxShape->getLocalPose().p;
		ar << BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot = (my::Quaternion&)m_PxShape->getLocalPose().q;
		ar << BOOST_SERIALIZATION_NVP(ShapeRot);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eCAPSULE:
	{
		_ASSERT(m_PxShape && m_PxShapeGeometryType == m_PxShape->getGeometryType());
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
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	case physx::PxGeometryType::eBOX:
	{
		_ASSERT(m_PxShape && m_PxShapeGeometryType == m_PxShape->getGeometryType());
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
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
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
	ar >> BOOST_SERIALIZATION_NVP(m_Material);
	ar >> BOOST_SERIALIZATION_NVP(m_PxShapeGeometryType);
	switch (m_PxShapeGeometryType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		my::Vector3 ShapePos;
		ar >> BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot;
		ar >> BOOST_SERIALIZATION_NVP(ShapeRot);
		float SphereRadius;
		ar >> BOOST_SERIALIZATION_NVP(SphereRadius);
		CreateSphereShape(ShapePos, ShapeRot, SphereRadius, pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
		break;
	}
	case physx::PxGeometryType::ePLANE:
	{
		my::Vector3 ShapePos;
		ar >> BOOST_SERIALIZATION_NVP(ShapePos);
		my::Quaternion ShapeRot;
		ar >> BOOST_SERIALIZATION_NVP(ShapeRot);
		CreatePlaneShape(ShapePos, ShapeRot, pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
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
		CreateCapsuleShape(ShapePos, ShapeRot, CapsuleRadius, CapsuleHalfHeight, pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
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
		CreateBoxShape(ShapePos, ShapeRot, BoxHalfExtents.x, BoxHalfExtents.y, BoxHalfExtents.z, pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
		break;
	}
	}
}

ComponentPtr Component::Clone(void) const
{
	std::stringstream sstr;
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(sstr, ".txt");
	*oa << boost::serialization::make_nvp(__FUNCTION__, shared_from_this());

	ComponentPtr ret(new Component());
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(sstr, ".txt", "");
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

void Component::EnterPhysxScene(PhysxScene * scene)
{
	_ASSERT(m_Actor);

	_ASSERT(!PhysxSdk::getSingleton().m_RenderTickMuted);

	if (m_PxShape && m_Actor->m_PxActor)
	{
		_ASSERT(!m_PxShape->getActor());

		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::LeavePhysxScene(PhysxScene * scene)
{
	_ASSERT(m_Actor);

	_ASSERT(!PhysxSdk::getSingleton().m_RenderTickMuted);

	if (m_PxShape && m_Actor->m_PxActor)
	{
		_ASSERT(m_PxShape->getActor() == m_Actor->m_PxActor.get());

		m_Actor->m_PxActor->detachShape(*m_PxShape);
	}
}

void Component::SetMaterial(MaterialPtr material)
{
	if (IsRequested() && m_Material)
	{
		m_Material->ReleaseResource();
	}

	m_Material = material;

	if (IsRequested() && m_Material)
	{
		m_Material->RequestResource();
	}
}

physx::PxMaterial * Component::CreatePhysxMaterial(float staticFriction, float dynamicFriction, float restitution, CollectionObjMap & collectionObjs)
{
	// ! materialIndices[0] = Ps::to16((static_cast<NpMaterial*>(materials[0]))->getHandle());
	std::string Key = str_printf("physx material %f %f %f", staticFriction, dynamicFriction, restitution);
	std::pair<CollectionObjMap::iterator, bool> obj_res = collectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
	if (obj_res.second)
	{
		obj_res.first->second.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(staticFriction, dynamicFriction, restitution), PhysxDeleter<physx::PxMaterial>());
	}

	return obj_res.first->second->is<physx::PxMaterial>();
}

void Component::CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial * material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxBoxGeometry(hx, hy, hz), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->userData = this;

	m_PxShapeGeometryType = physx::PxGeometryType::eBOX;

	if (m_Actor && m_Actor->IsRequested())
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxCapsuleGeometry(radius, halfHeight), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->userData = this;

	m_PxShapeGeometryType = physx::PxGeometryType::eCAPSULE;

	if (m_Actor && m_Actor->IsRequested())
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxPlaneGeometry(), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->userData = this;

	m_PxShapeGeometryType = physx::PxGeometryType::ePLANE;

	if (m_Actor && m_Actor->IsRequested())
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Component::CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxSphereGeometry(radius), *material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->userData = this;

	m_PxShapeGeometryType = physx::PxGeometryType::eSPHERE;

	if (m_Actor && m_Actor->IsRequested())
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
	_ASSERT(m_PxShape);
	physx::PxFilterData filter_data = m_PxShape->getSimulationFilterData();
	return filter_data.word0;
}

void Component::SetQueryFilterWord0(unsigned int filterWord0)
{
	_ASSERT(m_PxShape);
	physx::PxFilterData filter_data(filterWord0, 0, 0, 0);
	m_PxShape->setQueryFilterData(filter_data);
}

void Component::SetShapeFlag(physx::PxShapeFlag::Enum Flag, bool Value)
{
	_ASSERT(m_PxShape);
	m_PxShape->setFlag(Flag, Value);
}

bool Component::GetShapeFlag(physx::PxShapeFlag::Enum Flag) const
{
	_ASSERT(m_PxShape);
	return m_PxShape->getFlags() & Flag;
}

unsigned int Component::GetQueryFilterWord0(void) const
{
	_ASSERT(m_PxShape);
	physx::PxFilterData filter_data = m_PxShape->getQueryFilterData();
	return filter_data.word0;
}

void Component::ClearShape(void)
{
	if (m_PxShape)
	{
		if (m_Actor && m_Actor->IsRequested())
		{
			_ASSERT(m_PxShape->getActor() == m_Actor->m_PxActor.get());

			m_Actor->m_PxActor->detachShape(*m_PxShape);
		}

		_ASSERT(!m_PxShape->getActor());

		m_PxShape.reset();
	}

	m_PxShapeGeometryType = physx::PxGeometryType::eINVALID;
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
	ar << BOOST_SERIALIZATION_NVP(m_MeshSubMeshName);
	ar << BOOST_SERIALIZATION_NVP(m_MeshSubMeshId);
	ar << BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar << BOOST_SERIALIZATION_NVP(m_bInstance);
	switch (m_PxShapeGeometryType)
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		ar << BOOST_SERIALIZATION_NVP(m_PxMeshPath);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
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
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
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
	ar >> BOOST_SERIALIZATION_NVP(m_MeshSubMeshName);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshSubMeshId);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar >> BOOST_SERIALIZATION_NVP(m_bInstance);
	switch (m_PxShapeGeometryType)
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		std::string PxMeshPath;
		ar >> boost::serialization::make_nvp("m_PxMeshPath", PxMeshPath);
		CreateTriangleMeshShape(PxMeshPath.c_str(), pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
		break;
	}
	case physx::PxGeometryType::eCONVEXMESH:
	{
		std::string PxMeshPath;
		ar >> boost::serialization::make_nvp("m_PxMeshPath", PxMeshPath);
		CreateConvexMeshShape(PxMeshPath.c_str(), true, pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
		break;
	}
	}
}

void MeshComponent::OnMeshReady(my::DeviceResourceBasePtr res)
{
	m_Mesh = boost::dynamic_pointer_cast<my::OgreMesh>(res);
}

class PhysxBaseResource : public my::DeviceResourceBase
{
public:
	physx::PxBase* m_ptr;

	PhysxBaseResource(void)
		: m_ptr(NULL)
	{
	}

	virtual ~PhysxBaseResource(void)
	{
		if (m_ptr)
		{
			switch (m_ptr->getConcreteType())
			{
			case physx::PxConcreteType::eTRIANGLE_MESH_BVH33:
			{
				_ASSERT(static_cast<physx::Gu::TriangleMesh*>(m_ptr)->getRefCount() == 1);
				break;
			}
			case physx::PxConcreteType::eCONVEX_MESH:
			{
				_ASSERT(static_cast<physx::Gu::ConvexMesh*>(m_ptr)->getRefCount() == 1);
				break;
			}
			default:
				_ASSERT(false);
				break;
			}

			m_ptr->release();
		}
	}

	void Create(physx::PxBase* ptr)
	{
		_ASSERT(NULL == m_ptr);

		m_ptr = ptr;
	}
};

typedef boost::shared_ptr<PhysxBaseResource> PhysxBaseResourcePtr;

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
			PhysxBaseResourcePtr res(new PhysxBaseResource());
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
			PhysxBaseResourcePtr res(new PhysxBaseResource());
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

	m_PxMesh = res;

	_ASSERT(m_Actor && m_Actor->m_PxActor && m_PxShape && !m_PxShape->getActor());

	switch (type)
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
		m_PxShape->setGeometry(physx::PxTriangleMeshGeometry(
			boost::dynamic_pointer_cast<PhysxBaseResource>(m_PxMesh)->m_ptr->is<physx::PxTriangleMesh>(), mesh_scaling, physx::PxMeshGeometryFlags()));
		m_Actor->m_PxActor->attachShape(*m_PxShape);
		break;
	}
	case physx::PxGeometryType::eCONVEXMESH:
	{
		physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
		m_PxShape->setGeometry(physx::PxConvexMeshGeometry(
			boost::dynamic_pointer_cast<PhysxBaseResource>(m_PxMesh)->m_ptr->is<physx::PxConvexMesh>(), mesh_scaling, physx::PxConvexMeshGeometryFlags()));
		m_Actor->m_PxActor->attachShape(*m_PxShape);
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

		my::ResourceMgr::getSingleton().LoadMeshAsync(m_MeshPath.c_str(), m_MeshSubMeshName.c_str(), boost::bind(&MeshComponent::OnMeshReady, this, boost::placeholders::_1));
	}
}

void MeshComponent::ReleaseResource(void)
{
	Component::ReleaseResource();

	if (!m_MeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(MeshIORequest::BuildKey(m_MeshPath.c_str(), m_MeshSubMeshName.c_str()), boost::bind(&MeshComponent::OnMeshReady, this, boost::placeholders::_1));

		m_Mesh.reset();
	}
}

void MeshComponent::EnterPhysxScene(PhysxScene * scene)
{
	_ASSERT(m_Actor);

	_ASSERT(!PhysxSdk::getSingleton().m_RenderTickMuted);

	if (!m_PxMeshPath.empty() && m_Actor->m_PxActor)
	{
		_ASSERT(!m_PxMesh);

		switch (m_PxShapeGeometryType)
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
	else
	{
		Component::EnterPhysxScene(scene);
	}
}

void MeshComponent::LeavePhysxScene(PhysxScene * scene)
{
	_ASSERT(m_Actor);

	_ASSERT(!PhysxSdk::getSingleton().m_RenderTickMuted);

	if (!m_PxMeshPath.empty() && m_Actor->m_PxActor)
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_PxMeshPath, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, m_PxShapeGeometryType));

		if (m_PxMesh)
		{
			_ASSERT(m_PxShape->getActor());

			m_Actor->m_PxActor->detachShape(*m_PxShape);

			switch (m_PxShapeGeometryType)
			{
			case physx::PxGeometryType::eTRIANGLEMESH:
			{
				_ASSERT(m_PxMeshTmp);
				physx::PxMeshScale mesh_scaling((physx::PxVec3&)my::Vector3(1, 1, 1), physx::PxQuat(physx::PxIdentity));
				m_PxShape->setGeometry(physx::PxTriangleMeshGeometry(
					m_PxMeshTmp->is<physx::PxTriangleMesh>(), mesh_scaling, physx::PxMeshGeometryFlags()));
				break;
			}
			case physx::PxGeometryType::eCONVEXMESH:
			{
				_ASSERT(m_PxMeshTmp);
				physx::PxMeshScale mesh_scaling((physx::PxVec3&)my::Vector3(1, 1, 1), physx::PxQuat(physx::PxIdentity));
				m_PxShape->setGeometry(physx::PxConvexMeshGeometry(
					m_PxMeshTmp->is<physx::PxConvexMesh>(), mesh_scaling, physx::PxConvexMeshGeometryFlags()));
				break;
			}
			}

			m_PxMesh.reset();
		}
	}
	else
	{
		Component::LeavePhysxScene(scene);
	}
}

void MeshComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	Animator* animator = m_Actor->GetFirstComponent<Animator>();

	if (animator && !animator->m_DualQuats.empty() && m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
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
			Animator* animator = m_Actor->GetFirstComponent<Animator>();
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					D3DXMACRO macro[3] = { {0} };
					int j = 0;
					if (m_bInstance)
					{
						macro[j++].Name = "INSTANCE";
					}
					if (animator && m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
					{
						macro[j++].Name = "SKELETON";
					}
					my::Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, macro, m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						if (!handle_World)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
						}

						if (!handle_dualquat && animator && m_Mesh->m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
							BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));
						}

						if (m_bInstance)
						{
							pipeline->PushMeshInstance(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), m_MeshSubMeshId);
						}
						else
						{
							pipeline->PushMesh(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), m_MeshSubMeshId);
						}
					}
				}
			}
		}
	}
}

void MeshComponent::CreateTriangleMeshShape(const char * TriangleMeshPath, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	if (!my::ResourceMgr::getSingleton().CheckPath(TriangleMeshPath))
	{
		_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

		OgreMeshPtr mesh = m_Mesh ? m_Mesh : my::ResourceMgr::getSingleton().LoadMesh(m_MeshPath.c_str(), m_MeshSubMeshName.c_str());
		const D3DXATTRIBUTERANGE& att = mesh->m_AttribTable[m_MeshSubMeshId];
		physx::PxTriangleMeshDesc desc;
		desc.points.count = mesh->GetNumVertices()/*att.VertexCount*/;
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.points.data = (unsigned char*)mesh->LockVertexBuffer()/* + att.VertexStart * desc.points.stride*/;
		desc.triangles.count = att.FaceCount;
		if (mesh->GetOptions() & D3DXMESH_32BIT)
		{
			desc.triangles.stride = 3 * sizeof(DWORD);
		}
		else
		{
			desc.triangles.stride = 3 * sizeof(WORD);
			desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		}
		desc.triangles.data = (unsigned char*)mesh->LockIndexBuffer() + att.FaceStart * desc.triangles.stride;

		physx::PxDefaultFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(TriangleMeshPath).c_str());
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

	std::string Key("base physx triangle mesh");
	std::pair<CollectionObjMap::iterator, bool> obj_res = collectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
	if (obj_res.second)
	{
		float v[3][3] = { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f} };
		short i[3] = { 0, 1, 2 };
		physx::PxTriangleMeshDesc desc;
		desc.points.count = 3;
		desc.points.stride = sizeof(v[0]);
		desc.points.data = (unsigned char*)v;
		desc.triangles.count = 1;
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.triangles.data = (unsigned char*)i;
		desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;

		obj_res.first->second.reset(
			PhysxSdk::getSingleton().m_Cooking->createTriangleMesh(desc, PhysxSdk::getSingleton().m_sdk->getPhysicsInsertionCallback()), PhysxDeleter<physx::PxTriangleMesh>());
	}

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)Vector3(1, 1, 1), physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxTriangleMeshGeometry(obj_res.first->second->is<physx::PxTriangleMesh>(), mesh_scaling, physx::PxMeshGeometryFlags()),
		*material, true, /*physx::PxShapeFlag::eVISUALIZATION |*/ physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->userData = this;

	m_PxMeshTmp = obj_res.first->second.get();

	m_PxShapeGeometryType = physx::PxGeometryType::eTRIANGLEMESH;

	if (m_Actor && m_Actor->IsRequested())
	{
		IORequestPtr request(new PhysxTriangleMeshIORequest(m_PxMeshPath.c_str(), INT_MAX));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(m_PxMeshPath, request, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, physx::PxGeometryType::eTRIANGLEMESH));
	}
}

void MeshComponent::CreateConvexMeshShape(const char * ConvexMeshPath, bool bInflateConvex, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	physx::PxMaterial* material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	if (!my::ResourceMgr::getSingleton().CheckPath(ConvexMeshPath))
	{
		_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

		OgreMeshPtr mesh = m_Mesh ? m_Mesh : my::ResourceMgr::getSingleton().LoadMesh(m_MeshPath.c_str(), m_MeshSubMeshName.c_str());
		const D3DXATTRIBUTERANGE& att = mesh->m_AttribTable[m_MeshSubMeshId];
		physx::PxConvexMeshDesc desc;
		desc.points.count = att.VertexCount;
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.points.data = (unsigned char*)mesh->LockVertexBuffer() + att.VertexStart * desc.points.stride;
		desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		if (bInflateConvex)
		{
			desc.flags |= physx::PxConvexFlag::eINFLATE_CONVEX;
		}
		desc.vertexLimit = 256;

		physx::PxDefaultFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(ConvexMeshPath).c_str());
		bool status = PhysxSdk::getSingleton().m_Cooking->cookConvexMesh(desc, writeBuffer);
		mesh->UnlockVertexBuffer();
		if (!status)
		{
			THROW_CUSEXCEPTION("cookConvexMesh failed");
		}
	}

	_ASSERT(m_PxMeshPath.empty());

	m_PxMeshPath.assign(ConvexMeshPath);

	std::string Key("base physx convex mesh");
	std::pair<CollectionObjMap::iterator, bool> obj_res = collectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
	if (obj_res.second)
	{
		float v[3][3] = { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f} };
		physx::PxConvexMeshDesc desc;
		desc.points.count = 3;
		desc.points.stride = sizeof(v[0]);
		desc.points.data = (unsigned char*)v;
		desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		desc.vertexLimit = 256;

		obj_res.first->second.reset(
			PhysxSdk::getSingleton().m_Cooking->createConvexMesh(desc, PhysxSdk::getSingleton().m_sdk->getPhysicsInsertionCallback()), PhysxDeleter<physx::PxConvexMesh>());
	}

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)Vector3(1, 1, 1), physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxConvexMeshGeometry(obj_res.first->second->is<physx::PxConvexMesh>(), mesh_scaling, physx::PxConvexMeshGeometryFlags()),
		*material, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->userData = this;

	m_PxMeshTmp = obj_res.first->second.get();

	m_PxShapeGeometryType = physx::PxGeometryType::eCONVEXMESH;

	if (m_Actor && m_Actor->IsRequested())
	{
		IORequestPtr request(new PhysxConvexMeshIORequest(m_PxMeshPath.c_str(), INT_MAX));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(m_PxMeshPath, request, boost::bind(&MeshComponent::OnPxMeshReady, this, boost::placeholders::_1, physx::PxGeometryType::eCONVEXMESH));
	}
}

void MeshComponent::ClearShape(void)
{
	Component::ClearShape();

	m_PxMesh.reset();

	m_PxMeshPath.clear();

	m_PxMeshTmp = NULL; // ! dont release
}

ClothComponent::~ClothComponent(void)
{
	_ASSERT(!m_Cloth || !m_Cloth->getScene());
}

namespace boost { 
	namespace serialization {
		template<class Archive>
		inline void serialize(Archive & ar, D3DXATTRIBUTERANGE & t, const unsigned int file_version)
		{
			ar & BOOST_SERIALIZATION_NVP(t.AttribId);
			ar & BOOST_SERIALIZATION_NVP(t.FaceStart);
			ar & BOOST_SERIALIZATION_NVP(t.FaceCount);
			ar & BOOST_SERIALIZATION_NVP(t.VertexStart);
			ar & BOOST_SERIALIZATION_NVP(t.VertexCount);
		}

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

	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	unsigned int VertexSize = m_VertexData.size();
	ar << BOOST_SERIALIZATION_NVP(VertexSize);
	ar << boost::serialization::make_nvp("m_VertexData", boost::serialization::binary_object((void *)&m_VertexData[0], VertexSize));
	ar << BOOST_SERIALIZATION_NVP(m_VertexStride);
	unsigned int IndexSize = m_IndexData.size() * sizeof(unsigned short);
	ar << BOOST_SERIALIZATION_NVP(IndexSize);
	ar << boost::serialization::make_nvp("m_IndexData", boost::serialization::binary_object((void *)&m_IndexData[0], IndexSize));
	ar << BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar << BOOST_SERIALIZATION_NVP(m_VertexElems);
	ar << BOOST_SERIALIZATION_NVP(m_particles);
	ar << BOOST_SERIALIZATION_NVP(m_ClothFabricPath);
	ar << BOOST_SERIALIZATION_NVP(m_ClothSpheres);
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
	m_IndexData.resize(IndexSize / sizeof(unsigned short));
	ar >> boost::serialization::make_nvp("m_IndexData", boost::serialization::binary_object((void *)&m_IndexData[0], IndexSize));
	ar >> BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar >> BOOST_SERIALIZATION_NVP(m_VertexElems);
	ar >> BOOST_SERIALIZATION_NVP(m_particles);

	std::string ClothFabricPath;
	ar >> boost::serialization::make_nvp("m_ClothFabricPath", ClothFabricPath);
	CreateClothFromMesh(ClothFabricPath.c_str(), my::OgreMeshPtr(), 0);

	ar >> BOOST_SERIALIZATION_NVP(m_ClothSpheres);
}

void ClothComponent::CreateClothFromMesh(const char * ClothFabricPath, my::OgreMeshPtr mesh, DWORD AttribId)
{
	if (m_VertexData.empty())
	{
		_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

		const D3DXATTRIBUTERANGE& att = mesh->m_AttribTable[AttribId];
		m_VertexStride = mesh->GetNumBytesPerVertex();
		m_VertexData.resize(att.VertexCount * m_VertexStride);
		memcpy(&m_VertexData[0], (unsigned char*)mesh->LockVertexBuffer() + att.VertexStart * m_VertexStride, m_VertexData.size());
		mesh->UnlockVertexBuffer();

		m_IndexData.resize(att.FaceCount * 3);
		if (mesh->GetNumVertices() > USHRT_MAX)
		{
			THROW_CUSEXCEPTION(str_printf("create deformation mesh with overflow index size %Iu", m_IndexData.size()));
		}
		VOID* pIndices = mesh->LockIndexBuffer();
		for (unsigned int face_i = 0; face_i < att.FaceCount; face_i++)
		{
			// ! take care of att.VertexStart
			if (mesh->GetOptions() & D3DXMESH_32BIT)
			{
				m_IndexData[face_i * 3 + 0] = (WORD) * ((DWORD*)pIndices + att.FaceStart * 3 + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = (WORD) * ((DWORD*)pIndices + att.FaceStart * 3 + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = (WORD) * ((DWORD*)pIndices + att.FaceStart * 3 + face_i * 3 + 2);
			}
			else
			{
				m_IndexData[face_i * 3 + 0] = *((WORD*)pIndices + att.FaceStart * 3 + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = *((WORD*)pIndices + att.FaceStart * 3 + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = *((WORD*)pIndices + att.FaceStart * 3 + face_i * 3 + 2);
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

		m_particles.resize(att.VertexCount);
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
		desc.points.data = &m_VertexData[0] + m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
		desc.points.count = m_VertexData.size() / m_VertexStride;
		desc.points.stride = m_VertexStride;
		desc.triangles.data = &m_IndexData[0];
		desc.triangles.count = m_IndexData.size() / 3;
		desc.triangles.stride = 3 * sizeof(unsigned short);
		desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;

		physx::PxDefaultFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(ClothFabricPath).c_str());
		physx::PxClothFabricCooker cooker(desc, (physx::PxVec3&)my::Vector3::Gravity, true);
		cooker.save(writeBuffer, true);
	}

	_ASSERT(m_ClothFabricPath.empty());

	m_ClothFabricPath.assign(ClothFabricPath);

	PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(ClothFabricPath));
	m_ClothFabric.reset(PhysxSdk::getSingleton().m_sdk->createClothFabric(readBuffer), PhysxDeleter<physx::PxClothFabric>());

	m_Cloth.reset(PhysxSdk::getSingleton().m_sdk->createCloth(
		physx::PxTransform(physx::PxIdentity), *m_ClothFabric, &m_particles[0], physx::PxClothFlags()), PhysxDeleter<physx::PxCloth>());
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

void ClothComponent::EnterPhysxScene(PhysxScene * scene)
{
	Component::EnterPhysxScene(scene);
}

void ClothComponent::LeavePhysxScene(PhysxScene * scene)
{
	Component::LeavePhysxScene(scene);
}

void ClothComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(!m_VertexData.empty());

	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	Animator* animator = m_Actor->GetFirstComponent<Animator>();

	if (animator && !animator->m_DualQuats.empty() && m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
	{
		shader->SetMatrixArray(handle_dualquat, &animator->m_DualQuats[0], animator->m_DualQuats.size());
	}
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
			Animator* animator = m_Actor->GetFirstComponent<Animator>();
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					D3DXMACRO macro[2] = { { 0 } };
					int j = 0;
					if (animator && m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
					{
						macro[j++].Name = "SKELETON";
					}
					my::Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, macro, m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						if (!handle_World)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
						}

						if (!handle_dualquat && animator && m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
							BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));
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
	UpdateCloth();
}

void ClothComponent::UpdateCloth(void)
{
	if (m_Cloth)
	{
		_ASSERT(m_particles.size() == m_VertexData.size() / m_VertexStride);
		physx::PxClothParticleData * readData = m_Cloth->lockParticleData(physx::PxDataAccessFlag::eWRITABLE);
		if (readData)
		{
			unsigned char * pVertices = &m_VertexData[0];
			const DWORD NbParticles = m_Cloth->getNbParticles();
			Animator* animator = m_Actor->GetFirstComponent<Animator>();
			if (animator && !animator->m_DualQuats.empty() && m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
			{
				for (unsigned int i = 0; i < NbParticles; i++)
				{
					void * pVertex = pVertices + i * m_VertexStride;
					if (readData->particles[i].invWeight == 0)
					{
						my::Vector3 pos = animator->m_DualQuats.TransformVertexWithDualQuaternionList(
							(my::Vector3 &)m_particles[i].pos,
							m_VertexElems.GetBlendIndices(pVertex),
							m_VertexElems.GetBlendWeight(pVertex));
						readData->particles[i].pos = (physx::PxVec3 &)pos;
						m_VertexElems.SetPosition(pVertex, pos);
					}
					else
					{
						m_VertexElems.SetPosition(pVertex, (my::Vector3 &)readData->particles[i].pos);
					}
				}
			}
			else
			{
				for (unsigned int i = 0; i < NbParticles; i++)
				{
					void * pVertex = pVertices + i * m_VertexStride;
					m_VertexElems.SetPosition(pVertex, (my::Vector3 &)readData->particles[i].pos);
				}
			}
			readData->unlock();

			my::OgreMesh::ComputeNormalFrame(
				pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);

			my::OgreMesh::ComputeTangentFrame(
				pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);
		}
	}
}

void ClothComponent::OnPxThreadSubstep(float dtime)
{
	_ASSERT(m_Actor);

	if (!m_ClothSpheres.empty())
	{
		m_ClothSpheresTmp.resize(m_ClothSpheres.size());
		Animator* animator = m_Actor->GetFirstComponent<Animator>();
		if (animator && !animator->m_DualQuats.empty() && m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
		{
			for (unsigned int i = 0; i < m_ClothSpheres.size(); i++)
			{
				m_ClothSpheresTmp[i].radius = m_ClothSpheres[i].first.radius;
				if (m_ClothSpheres[i].second >= 0)
				{
					Matrix4 & dual = animator->m_DualQuats[m_ClothSpheres[i].second];
					m_ClothSpheresTmp[i].pos = (physx::PxVec3 &)TransformList::TransformVertexWithDualQuaternion(
						(Vector3 &)m_ClothSpheres[i].first.pos, dual);
				}
				else
				{
					m_ClothSpheresTmp[i].pos = m_ClothSpheres[i].first.pos;
				}
			}
		}
		else
		{
			for (unsigned int i = 0; i < m_ClothSpheres.size(); i++)
			{
				m_ClothSpheresTmp[i].pos = m_ClothSpheres[i].first.pos;
			}
		}

		m_Cloth->setCollisionSpheres(&m_ClothSpheresTmp[0], m_ClothSpheresTmp.size());
	}
}

void EmitterComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	if (m_EmitterSpaceType == SpaceTypeWorld)
	{
		shader->SetMatrix(handle_World, my::Matrix4::identity);
	}
	else
	{
		shader->SetMatrix(handle_World, m_Actor->m_World);
	}
}

void EmitterComponent::AddParticlePairToPipeline(RenderPipeline* pipeline, unsigned int PassMask, my::Emitter::Particle* particles1, unsigned int particle_num1, my::Emitter::Particle* particles2, unsigned int particle_num2)
{
	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				D3DXMACRO macro[3] = { {0} };
				macro[0].Name = "EMITTER_FACE_TYPE";
				switch (m_EmitterFaceType)
				{
				default:
					macro[0].Definition = "0";
					break;
				case FaceTypeY:
					macro[0].Definition = "1";
					break;
				case FaceTypeZ:
					macro[0].Definition = "2";
					break;
				case FaceTypeCamera:
					macro[0].Definition = "3";
					break;
				case FaceTypeAngle:
					macro[0].Definition = "4";
					break;
				case FaceTypeAngleCamera:
					macro[0].Definition = "5";
					break;
				}
				macro[1].Name = "EMITTER_VEL_TYPE";
				switch (m_EmitterVelType)
				{
				default:
					macro[1].Definition = "0";
					break;
				case VelocityTypeVel:
					macro[1].Definition = "1";
					break;
				case VelocityTypeQuat:
					macro[1].Definition = "2";
					break;
				}
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeParticle, macro, m_Material->m_Shader.c_str(), PassID);
				if (shader)
				{
					if (!handle_World)
					{
						BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
					}

					if (particle_num1 > 0)
					{
						pipeline->PushEmitter(PassID, pipeline->m_ParticleVb.m_ptr, pipeline->m_ParticleIb.m_ptr,
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveMinVertexIndex],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveNumVertices],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount],
							particles1, particle_num1, shader, this, m_Material.get(), 0);
					}

					if (particle_num2 > 0)
					{
						pipeline->PushEmitter(PassID, pipeline->m_ParticleVb.m_ptr, pipeline->m_ParticleIb.m_ptr,
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveMinVertexIndex],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveNumVertices],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount],
							particles2, particle_num2, shader, this, m_Material.get(), 0);
					}
				}
			}
		}
	}
}

void CircularEmitter::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	ParticleList::array_range array_one = m_ParticleList.array_one();

	ParticleList::array_range array_two = m_ParticleList.array_two();

	AddParticlePairToPipeline(pipeline, PassMask, array_one.first, array_one.second, array_two.first, array_two.second);
}

template<class Archive>
void SphericalEmitter::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(CircularEmitter);
	Emitter::ParticleList::capacity_type Capacity;
	Capacity = m_ParticleList.capacity();
	ar << BOOST_SERIALIZATION_NVP(Capacity);
	ar << BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnInterval);
	ar << BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnInclination);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnColorR);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnColorG);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnColorB);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnColorA);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnSizeX);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnSizeY);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnAngle);
	ar << BOOST_SERIALIZATION_NVP(m_SpawnCycle);
}

template<class Archive>
void SphericalEmitter::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(CircularEmitter);
	Emitter::ParticleList::capacity_type Capacity;
	ar >> BOOST_SERIALIZATION_NVP(Capacity);
	m_ParticleList.set_capacity(Capacity);
	ar >> BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnInterval);
	ar >> BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnInclination);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnColorR);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnColorG);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnColorB);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnColorA);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnSizeX);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnSizeY);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnAngle);
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnCycle);
}

void SphericalEmitter::RequestResource(void)
{
	CircularEmitter::RequestResource();

	m_SpawnTime = D3DContext::getSingleton().m_fTotalTime;
}

void SphericalEmitter::ReleaseResource(void)
{
	CircularEmitter::ReleaseResource();
}

void SphericalEmitter::Update(float fElapsedTime)
{
	_ASSERT(m_SpawnInterval > 0);

	RemoveParticleBefore(D3DContext::getSingleton().m_fTotalTime - m_ParticleLifeTime);

	for (; m_SpawnTime < D3DContext::getSingleton().m_fTotalTime; m_SpawnTime += m_SpawnInterval)
	{
		const float SpawnTimeCycle = Wrap<float>(m_SpawnTime, 0, m_SpawnCycle);

		if (m_EmitterSpaceType == SpaceTypeWorld)
		{
			Spawn(
				Vector3(
					Random(-m_HalfSpawnArea.x, m_HalfSpawnArea.x),
					Random(-m_HalfSpawnArea.y, m_HalfSpawnArea.y),
					Random(-m_HalfSpawnArea.z, m_HalfSpawnArea.z)).transform(m_Actor->m_World),
				Vector4(Vector3::PolarToCartesian(
					m_SpawnSpeed,
					m_SpawnInclination.Interpolate(SpawnTimeCycle, 0),
					m_SpawnAzimuth.Interpolate(SpawnTimeCycle, 0)).transformNormal(m_Actor->m_World), 1),
				Vector4(
					m_SpawnColorR.Interpolate(SpawnTimeCycle, 1),
					m_SpawnColorG.Interpolate(SpawnTimeCycle, 1),
					m_SpawnColorB.Interpolate(SpawnTimeCycle, 1),
					m_SpawnColorA.Interpolate(SpawnTimeCycle, 1)),
				Vector2(
					m_SpawnSizeX.Interpolate(SpawnTimeCycle, 1),
					m_SpawnSizeY.Interpolate(SpawnTimeCycle, 1)),
				m_SpawnAngle.Interpolate(SpawnTimeCycle, 0), m_SpawnTime);
		}
		else
		{
			Spawn(
				Vector4(
					Random(-m_HalfSpawnArea.x, m_HalfSpawnArea.x),
					Random(-m_HalfSpawnArea.y, m_HalfSpawnArea.y),
					Random(-m_HalfSpawnArea.z, m_HalfSpawnArea.z), 1),
				Vector4(Vector3::PolarToCartesian(
					m_SpawnSpeed,
					m_SpawnInclination.Interpolate(SpawnTimeCycle, 0),
					m_SpawnAzimuth.Interpolate(SpawnTimeCycle, 0)), 1),
				Vector4(
					m_SpawnColorR.Interpolate(SpawnTimeCycle, 1),
					m_SpawnColorG.Interpolate(SpawnTimeCycle, 1),
					m_SpawnColorB.Interpolate(SpawnTimeCycle, 1),
					m_SpawnColorA.Interpolate(SpawnTimeCycle, 1)),
				Vector2(
					m_SpawnSizeX.Interpolate(SpawnTimeCycle, 1),
					m_SpawnSizeY.Interpolate(SpawnTimeCycle, 1)),
				m_SpawnAngle.Interpolate(SpawnTimeCycle, 0), m_SpawnTime);
		}
	}
}

my::AABB SphericalEmitter::CalculateAABB(void) const
{
	return Component::CalculateAABB();
}

#include "Component.h"
#include "Actor.h"
#include "myDxutApp.h"
#include "myEffect.h"
#include "myResource.h"
#include "Animation.h"
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

using namespace my;

BOOST_CLASS_EXPORT(Component)

BOOST_CLASS_EXPORT(MeshComponent)

BOOST_CLASS_EXPORT(ClothComponent)

BOOST_CLASS_EXPORT(EmitterComponent)

BOOST_CLASS_EXPORT(SphericalEmitter)

Component::~Component(void)
{
	_ASSERT(!IsRequested());

	if (m_Actor)
	{
		_ASSERT(false); //m_Actor->RemoveComponent(shared_from_this());
	}
}

template<class Archive>
void Component::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar << BOOST_SERIALIZATION_NVP(m_Type);
	ar << BOOST_SERIALIZATION_NVP(m_LodMask);
	ar << BOOST_SERIALIZATION_NVP(m_Material);
}

template<class Archive>
void Component::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(NamedObject);
	ar >> BOOST_SERIALIZATION_NVP(m_Type);
	ar >> BOOST_SERIALIZATION_NVP(m_LodMask);
	ar >> BOOST_SERIALIZATION_NVP(m_Material);
}

void Component::CopyFrom(const Component & rhs)
{
	if (rhs.m_Name)
	{
		SetName(NamedObject::MakeUniqueName(rhs.m_Name).c_str());
	}

	m_Type = rhs.m_Type;
	m_LodMask = rhs.m_LodMask;
	if (rhs.m_Material)
	{
		m_Material = rhs.m_Material->Clone();
	}
}

ComponentPtr Component::Clone(void) const
{
	ComponentPtr ret(new Component());
	ret->CopyFrom(*this);
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

MaterialPtr Component::GetMaterial(void) const
{
	return m_Material;
}

physx::PxMaterial * Component::CreatePhysxMaterial(float staticFriction, float dynamicFriction, float restitution, bool ShareSerializeCollection)
{
	// ! materialIndices[0] = Ps::to16((static_cast<NpMaterial*>(materials[0]))->getHandle());
	std::pair<PhysxSdk::CollectionObjMap::iterator, bool> obj_res;
	if (ShareSerializeCollection)
	{
		std::string Key = str_printf("PxMaterial %f %f %f", staticFriction, dynamicFriction, restitution);
		obj_res = PhysxSdk::getSingleton().m_CollectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
		if (!obj_res.second)
		{
			return obj_res.first->second->is<physx::PxMaterial>();
		}
	}

	physx::PxMaterial * material = PhysxSdk::getSingleton().m_sdk->createMaterial(staticFriction, dynamicFriction, restitution);
	if (ShareSerializeCollection)
	{
		obj_res.first->second.reset(material, PhysxDeleter<physx::PxMaterial>());
	}
	else
	{
		m_PxMaterial.reset(material, PhysxDeleter<physx::PxMaterial>());
	}
	return material;
}

void Component::CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, bool ShareSerializeCollection)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateBoxShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	physx::PxMaterial * matertial = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, ShareSerializeCollection);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxBoxGeometry(hx, hy, hz), *matertial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	m_PxShape->userData = this;
}

void Component::CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, bool ShareSerializeCollection)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateCapsuleShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	physx::PxMaterial* matertial = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, ShareSerializeCollection);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxCapsuleGeometry(radius, halfHeight), *matertial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	m_PxShape->userData = this;
}

void Component::CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, bool ShareSerializeCollection)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreatePlaneShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	if (m_Actor->m_PxActor->getType() == physx::PxActorType::eRIGID_DYNAMIC
		&& !m_Actor->m_PxActor->is<physx::PxRigidBody>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	physx::PxMaterial* matertial = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, ShareSerializeCollection);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxPlaneGeometry(), *matertial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	m_PxShape->userData = this;
}

void Component::CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, bool ShareSerializeCollection)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateSphereShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	physx::PxMaterial* matertial = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, ShareSerializeCollection);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxSphereGeometry(radius), *matertial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	m_PxShape->userData = this;
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
	if (!m_PxShape)
	{
		return;
	}

	if (m_Actor && m_Actor->m_PxActor)
	{
		m_Actor->m_PxActor->detachShape(*m_PxShape, true);
	}

	m_PxShape.reset();

	m_PxMaterial.reset();
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
}

template<class Archive>
void MeshComponent::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshPath);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshSubMeshName);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshSubMeshId);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshColor);
	ar >> BOOST_SERIALIZATION_NVP(m_bInstance);
}

void MeshComponent::CopyFrom(const MeshComponent & rhs)
{
	Component::CopyFrom(rhs);
	m_MeshPath = rhs.m_MeshPath;
	m_MeshSubMeshName = rhs.m_MeshSubMeshName;
	m_MeshSubMeshId = rhs.m_MeshSubMeshId;
	m_MeshColor = rhs.m_MeshColor;
	m_bInstance = rhs.m_bInstance;
}

ComponentPtr MeshComponent::Clone(void) const
{
	MeshComponentPtr ret(new MeshComponent());
	ret->CopyFrom(*this);
	return ret;
}

void MeshComponent::OnMeshReady(my::DeviceResourceBasePtr res)
{
	m_Mesh = boost::dynamic_pointer_cast<my::OgreMesh>(res);
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
	if (!m_MeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(MeshIORequest::BuildKey(m_MeshPath.c_str(), m_MeshSubMeshName.c_str()), boost::bind(&MeshComponent::OnMeshReady, this, boost::placeholders::_1));

		m_Mesh.reset();
	}

	Component::ReleaseResource();
}

void MeshComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	Animator* animator = m_Actor->GetAnimator();

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
	if (m_Mesh)
	{
		return m_Mesh->CalculateAABB(m_MeshSubMeshId);
	}
	return Component::CalculateAABB();
}

void MeshComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_Actor);

	if (m_Mesh)
	{
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
			Animator* animator = m_Actor->GetAnimator();
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

physx::PxTriangleMesh * MeshComponent::CreateTriangleMesh(bool ShareSerializeCollection)
{
	OgreMeshPtr mesh = m_Mesh ? m_Mesh : my::ResourceMgr::getSingleton().LoadMesh(m_MeshPath.c_str(), m_MeshSubMeshName.c_str());
	std::pair<PhysxSdk::CollectionObjMap::iterator, bool> obj_res;
	if (ShareSerializeCollection)
	{
		std::string Key = str_printf("%s %d PxTriangleMesh", mesh->m_Key, m_MeshSubMeshId);
		obj_res = PhysxSdk::getSingleton().m_CollectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
		if (!obj_res.second)
		{
			return obj_res.first->second->is<physx::PxTriangleMesh>();
		}
	}

	const D3DXATTRIBUTERANGE& att = mesh->m_AttribTable[m_MeshSubMeshId];
	physx::PxTriangleMeshDesc desc;
	desc.points.count = att.VertexCount;
	desc.points.stride = mesh->GetNumBytesPerVertex();
	desc.points.data = (unsigned char*)mesh->LockVertexBuffer() + att.VertexStart * desc.points.stride;
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
	physx::PxDefaultMemoryOutputStream writeBuffer;
	bool status = PhysxSdk::getSingleton().m_Cooking->cookTriangleMesh(desc, writeBuffer);
	mesh->UnlockIndexBuffer();
	mesh->UnlockVertexBuffer();
	if (!status)
	{
		return NULL;
	}
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	physx::PxTriangleMesh * tri_mesh = PhysxSdk::getSingleton().m_sdk->createTriangleMesh(readBuffer);
	if (ShareSerializeCollection)
	{
		obj_res.first->second.reset(tri_mesh, PhysxDeleter<physx::PxTriangleMesh>());
	}
	else
	{
		m_PxMesh.reset(tri_mesh, PhysxDeleter<physx::PxTriangleMesh>());
	}
	return tri_mesh;
}

void MeshComponent::CreateTriangleMeshShape(bool ShareSerializeCollection)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		return;
	}

	if (m_Actor->m_PxActor->getType() == physx::PxActorType::eRIGID_DYNAMIC
		&& !m_Actor->m_PxActor->is<physx::PxRigidBody>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	physx::PxMaterial * matertial = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, ShareSerializeCollection);

	physx::PxTriangleMesh * tri_mesh = CreateTriangleMesh(ShareSerializeCollection);

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxTriangleMeshGeometry(tri_mesh, mesh_scaling, physx::PxMeshGeometryFlags()),
		*matertial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	m_PxShape->userData = this;
}

physx::PxConvexMesh * MeshComponent::CreateConvexMesh(bool bInflateConvex, bool ShareSerializeCollection)
{
	OgreMeshPtr mesh = m_Mesh ? m_Mesh : my::ResourceMgr::getSingleton().LoadMesh(m_MeshPath.c_str(), m_MeshSubMeshName.c_str());
	std::pair<PhysxSdk::CollectionObjMap::iterator, bool> obj_res;
	if (ShareSerializeCollection)
	{
		std::string Key = str_printf("%s %d PxConvexMesh", mesh->m_Key, m_MeshSubMeshId);
		obj_res = PhysxSdk::getSingleton().m_CollectionObjs.insert(std::make_pair(Key, boost::shared_ptr<physx::PxBase>()));
		if (!obj_res.second)
		{
			return obj_res.first->second->is<physx::PxConvexMesh>();
		}
	}

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
	physx::PxDefaultMemoryOutputStream writeBuffer;
	bool status = PhysxSdk::getSingleton().m_Cooking->cookConvexMesh(desc, writeBuffer);
	mesh->UnlockVertexBuffer();
	if (!status)
	{
		return NULL;
	}
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	physx::PxConvexMesh * cvx_mesh = PhysxSdk::getSingleton().m_sdk->createConvexMesh(readBuffer);
	if (ShareSerializeCollection)
	{
		obj_res.first->second.reset(cvx_mesh, PhysxDeleter<physx::PxConvexMesh>());
	}
	else
	{
		m_PxMesh.reset(cvx_mesh, PhysxDeleter<physx::PxConvexMesh>());
	}
	return cvx_mesh;
}

void MeshComponent::CreateConvexMeshShape(bool bInflateConvex, bool ShareSerializeCollection)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		return;
	}

	if (m_Actor->m_PxActor->getType() == physx::PxActorType::eRIGID_DYNAMIC
		&& !m_Actor->m_PxActor->is<physx::PxRigidBody>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	physx::PxMaterial * matertial = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, ShareSerializeCollection);

	physx::PxConvexMesh * cvx_mesh = CreateConvexMesh(bInflateConvex, ShareSerializeCollection);

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxConvexMeshGeometry(cvx_mesh, mesh_scaling),
		*matertial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	m_PxShape->userData = this;
}

void MeshComponent::ClearShape(void)
{
	Component::ClearShape();

	m_PxMesh.reset();
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

	boost::shared_ptr<physx::PxCollection> collection(PxCreateCollection(), PhysxDeleter<physx::PxCollection>());
	collection->add(*m_Cloth);
	physx::PxSerialization::complete(*collection, *pxar->m_Registry, pxar->m_Collection.get());
	physx::PxDefaultMemoryOutputStream ostr;
	physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *pxar->m_Registry, pxar->m_Collection.get());
	unsigned int ClothSize = ostr.getSize();
	ar << BOOST_SERIALIZATION_NVP(ClothSize);
	ar << boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));

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

	unsigned int ClothSize;
	ar >> BOOST_SERIALIZATION_NVP(ClothSize);
	m_SerializeBuff.reset((unsigned char *)_aligned_malloc(ClothSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
	ar >> boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(m_SerializeBuff.get(), ClothSize));
	boost::shared_ptr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *pxar->m_Registry, pxar->m_Collection.get()), PhysxDeleter<physx::PxCollection>());
	const unsigned int numObjs = collection->getNbObjects();
	for (unsigned int i = 0; i < numObjs; i++)
	{
		physx::PxBase * obj = &collection->getObject(i);
		switch (obj->getConcreteType())
		{
		case physx::PxConcreteType::eCLOTH_FABRIC:
			m_Fabric.reset(obj->is<physx::PxClothFabric>(), PhysxDeleter<physx::PxClothFabric>());
			break;
		case physx::PxConcreteType::eCLOTH:
			m_Cloth.reset(obj->is<physx::PxCloth>(), PhysxDeleter<physx::PxCloth>());
			break;
		}
	}

	ar >> BOOST_SERIALIZATION_NVP(m_ClothSpheres);
}

void ClothComponent::CopyFrom(const ClothComponent & rhs)
{
	Component::CopyFrom(rhs);
	// TODO:
}

ComponentPtr ClothComponent::Clone(void) const
{
	ClothComponentPtr ret(new ClothComponent());
	ret->CopyFrom(*this);
	return ret;
}

void ClothComponent::CreateClothFromMesh(my::OgreMeshPtr mesh, DWORD AttribId)
{
	if (m_VertexData.empty())
	{
		const D3DXATTRIBUTERANGE& att = mesh->m_AttribTable[AttribId];
		m_VertexStride = mesh->GetNumBytesPerVertex();
		m_VertexData.resize(att.VertexCount * m_VertexStride);
		memcpy(&m_VertexData[0], (unsigned char *)mesh->LockVertexBuffer() + att.VertexStart * m_VertexStride, m_VertexData.size());
		mesh->UnlockVertexBuffer();

		m_IndexData.resize(att.FaceCount * 3);
		if (mesh->GetNumVertices() > USHRT_MAX)
		{
			THROW_CUSEXCEPTION(str_printf("create deformation mesh with overflow index size %u", m_IndexData.size()));
		}
		VOID * pIndices = mesh->LockIndexBuffer();
		for (unsigned int face_i = 0; face_i < att.FaceCount; face_i++)
		{
			// ! take care of att.VertexStart
			if(mesh->GetOptions() & D3DXMESH_32BIT)
			{
				m_IndexData[face_i * 3 + 0] = (WORD)*((DWORD *)pIndices + att.FaceStart * 3 + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = (WORD)*((DWORD *)pIndices + att.FaceStart * 3 + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = (WORD)*((DWORD *)pIndices + att.FaceStart * 3 + face_i * 3 + 2);
			}
			else
			{
				m_IndexData[face_i * 3 + 0] = *((WORD *)pIndices + att.FaceStart * 3 + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = *((WORD *)pIndices + att.FaceStart * 3 + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = *((WORD *)pIndices + att.FaceStart * 3 + face_i * 3 + 2);
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
		unsigned char * pVertices = (unsigned char *)&m_VertexData[0];
		for(unsigned int i = 0; i < m_particles.size(); i++) {
			unsigned char * pVertex = pVertices + i * m_VertexStride;
			m_particles[i].pos = (physx::PxVec3 &)m_VertexElems.GetPosition(pVertex);
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

		physx::PxClothMeshDesc desc;
		desc.points.data = &m_VertexData[0] + m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
		desc.points.count = att.VertexCount;
		desc.points.stride = m_VertexStride;
		desc.triangles.data = &m_IndexData[0];
		desc.triangles.count = att.FaceCount;
		desc.triangles.stride = 3 * sizeof(unsigned short);
		desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		boost::shared_ptr<physx::PxClothFabric> fabric(PxClothFabricCreate(
			*PhysxSdk::getSingleton().m_sdk, desc, (physx::PxVec3&)my::Vector3::Gravity, true), PhysxDeleter<physx::PxClothFabric>());
		m_Cloth.reset(PhysxSdk::getSingleton().m_sdk->createCloth(
			physx::PxTransform(physx::PxIdentity), *fabric, &m_particles[0], physx::PxClothFlags()), PhysxDeleter<physx::PxCloth>());
	}
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

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

	if (m_Cloth)
	{
		scene->m_PxScene->addActor(*m_Cloth);
	}

	scene->m_EventPxThreadSubstep.connect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, boost::placeholders::_1));
}

void ClothComponent::ReleaseResource(void)
{
	m_Decl.Release();

	PhysxScene* scene = dynamic_cast<PhysxScene*>(m_Actor->m_Node->GetTopNode());

	_ASSERT(!m_Cloth || m_Cloth->getScene() == scene->m_PxScene.get());

	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, boost::placeholders::_1));

	if (m_Cloth)
	{
		scene->m_PxScene->removeActor(*m_Cloth);

		scene->removeRenderActorsFromPhysicsActor(m_Cloth.get());
	}

	Component::ReleaseResource();
}

void ClothComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(!m_VertexData.empty());

	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	Animator* animator = m_Actor->GetAnimator();

	if (animator && !animator->m_DualQuats.empty() && m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
	{
		shader->SetMatrixArray(handle_dualquat, &animator->m_DualQuats[0], animator->m_DualQuats.size());
	}
}

void ClothComponent::SetPxPoseOrbyPxThread(const physx::PxTransform& pose)
{
	if (m_Cloth)
	{
		m_Cloth->setTargetPose(pose);
	}
}

my::AABB ClothComponent::CalculateAABB(void) const
{
	if (!m_VertexData.empty())
	{
		AABB ret = AABB::Invalid();
		unsigned char * pVertices = (unsigned char *)&m_VertexData[0];
		const unsigned int NumVertices = m_VertexData.size() / m_VertexStride;
		for (unsigned int i = 0; i < NumVertices; i++)
		{
			unsigned char * pVertex = pVertices + i * m_VertexStride;
			ret.unionSelf(m_VertexElems.GetPosition(pVertex));
		}
		return ret;
	}
	return Component::CalculateAABB();
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
			Animator* animator = m_Actor->GetAnimator();
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
			Animator* animator = m_Actor->GetAnimator();
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
		Animator* animator = m_Actor->GetAnimator();
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

void EmitterComponent::CopyFrom(const EmitterComponent & rhs)
{
	Component::CopyFrom(rhs);
	m_EmitterFaceType = rhs.m_EmitterFaceType;
	m_EmitterVelType = rhs.m_EmitterVelType;
}

ComponentPtr EmitterComponent::Clone(void) const
{
	EmitterComponentPtr ret(new EmitterComponent());
	ret->CopyFrom(*this);
	return ret;
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
						pipeline->PushEmitter(PassID, pipeline->m_ParticleQuadVb.m_ptr, pipeline->m_ParticleQuadIb.m_ptr,
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveMinVertexIndex],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveNumVertices],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex],
							RenderPipeline::m_ParticlePrimitiveInfo[m_EmitterPrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount],
							particles1, particle_num1, shader, this, m_Material.get(), 0);
					}

					if (particle_num2 > 0)
					{
						pipeline->PushEmitter(PassID, pipeline->m_ParticleQuadVb.m_ptr, pipeline->m_ParticleQuadIb.m_ptr,
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

template<class Archive>
void SphericalEmitter::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
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
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
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

void SphericalEmitter::CopyFrom(const SphericalEmitter & rhs)
{
	EmitterComponent::CopyFrom(rhs);
	// TODO:
}

ComponentPtr SphericalEmitter::Clone(void) const
{
	SphericalEmitterPtr ret(new SphericalEmitter());
	ret->CopyFrom(*this);
	return ret;
}

void SphericalEmitter::RequestResource(void)
{
	EmitterComponent::RequestResource();

	m_SpawnTime = D3DContext::getSingleton().m_fTotalTime;
}

void SphericalEmitter::ReleaseResource(void)
{
	EmitterComponent::ReleaseResource();
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
					Random(-m_HalfSpawnArea.z, m_HalfSpawnArea.z)).transformCoord(m_Actor->m_World),
				Vector3::SphericalToCartesian(
					m_SpawnSpeed,
					m_SpawnInclination.Interpolate(SpawnTimeCycle, 0),
					m_SpawnAzimuth.Interpolate(SpawnTimeCycle, 0)).transformNormal(m_Actor->m_World),
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
				Vector3(
					Random(-m_HalfSpawnArea.x, m_HalfSpawnArea.x),
					Random(-m_HalfSpawnArea.y, m_HalfSpawnArea.y),
					Random(-m_HalfSpawnArea.z, m_HalfSpawnArea.z)),
				Vector3::SphericalToCartesian(
					m_SpawnSpeed,
					m_SpawnInclination.Interpolate(SpawnTimeCycle, 0),
					m_SpawnAzimuth.Interpolate(SpawnTimeCycle, 0)),
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

void SphericalEmitter::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	ParticleList::array_range array_one = m_ParticleList.array_one();

	ParticleList::array_range array_two = m_ParticleList.array_two();

	AddParticlePairToPipeline(pipeline, PassMask, array_one.first, array_one.second, array_two.first, array_two.second);
}

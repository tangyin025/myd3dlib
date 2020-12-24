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

BOOST_CLASS_EXPORT(StaticEmitterComponent)

BOOST_CLASS_EXPORT(SphericalEmitterComponent)

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

void Component::EnterPhysxScene(PhysxScene * scene)
{
}

void Component::LeavePhysxScene(PhysxScene * scene)
{
}

void Component::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
}

void Component::Update(float fElapsedTime)
{
}

my::AABB Component::CalculateAABB(void) const
{
	return AABB(-1, 1);
}

bool Component::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	return true;
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

void Component::CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateBoxShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxBoxGeometry(hx, hy, hz), *m_PxMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

	m_PxShape->userData = this;
}

void Component::CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateCapsuleShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxCapsuleGeometry(radius, halfHeight), *m_PxMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

	m_PxShape->userData = this;
}

void Component::CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, unsigned int filterWord0)
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

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxPlaneGeometry(), *m_PxMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

	m_PxShape->userData = this;
}

void Component::CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateSphereShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxSphereGeometry(radius), *m_PxMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

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
	ar << BOOST_SERIALIZATION_NVP(m_bUseAnimation);
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
	ar >> BOOST_SERIALIZATION_NVP(m_bUseAnimation);
}

void MeshComponent::CopyFrom(const MeshComponent & rhs)
{
	Component::CopyFrom(rhs);
	m_MeshPath = rhs.m_MeshPath;
	m_MeshSubMeshName = rhs.m_MeshSubMeshName;
	m_MeshSubMeshId = rhs.m_MeshSubMeshId;
	m_MeshColor = rhs.m_MeshColor;
	m_bInstance = rhs.m_bInstance;
	m_bUseAnimation = rhs.m_bUseAnimation;
}

ComponentPtr MeshComponent::Clone(void) const
{
	MeshComponentPtr ret(new MeshComponent());
	ret->CopyFrom(*this);
	return ret;
}

void MeshComponent::OnReady(my::IORequest * request)
{
	m_Mesh = boost::dynamic_pointer_cast<my::OgreMesh>(request->m_res);

	if (m_MeshEventReady)
	{
		ComponentEventArg arg(this);
		m_MeshEventReady(&arg);
	}
}

void MeshComponent::RequestResource(void)
{
	Component::RequestResource();

	if (!m_MeshPath.empty())
	{
		_ASSERT(!m_Mesh);

		my::ResourceMgr::getSingleton().LoadMeshAsync(m_MeshPath.c_str(), m_MeshSubMeshName.c_str(), this);
	}
}

void MeshComponent::ReleaseResource(void)
{
	if (!m_MeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(MeshIORequest::BuildKey(m_MeshPath.c_str(), m_MeshSubMeshName.c_str()), this);

		m_Mesh.reset();
	}

	Component::ReleaseResource();
}

void MeshComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	if (m_bUseAnimation && m_Actor && m_Actor->m_Animation)
	{
		if (!m_Actor->m_Animation->m_DualQuats.empty())
		{
			shader->SetMatrixArray(handle_dualquat, &m_Actor->m_Animation->m_DualQuats[0], m_Actor->m_Animation->m_DualQuats.size());
		}
	}
}

void MeshComponent::Update(float fElapsedTime)
{
}

my::AABB MeshComponent::CalculateAABB(void) const
{
	if (m_Mesh)
	{
		return m_Mesh->m_aabb;
	}
	return Component::CalculateAABB();
}

bool MeshComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_Actor);

	bool ret = false;

	if (m_Mesh)
	{
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
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
					if (m_bUseAnimation)
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
							if (m_bUseAnimation && m_Actor && m_Actor->m_Animation)
							{
								BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));
							}
						}

						if (m_bInstance)
						{
							pipeline->PushMeshInstance(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), m_MeshSubMeshId);
						}
						else
						{
							pipeline->PushMesh(PassID, m_Mesh.get(), m_MeshSubMeshId, shader, this, m_Material.get(), m_MeshSubMeshId);
						}

						ret = true;
					}
				}
			}
		}
	}

	return ret;
}

void MeshComponent::CreateTriangleMeshShape(unsigned int filterWord0)
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

	if (!m_Mesh)
	{
		return;
	}

	const D3DXATTRIBUTERANGE& att = m_Mesh->m_AttribTable[m_MeshSubMeshId];
	boost::shared_ptr<physx::PxTriangleMesh> triangle_mesh;
	physx::PxTriangleMeshDesc desc;
	desc.points.count = att.VertexCount;
	desc.points.stride = m_Mesh->GetNumBytesPerVertex();
	desc.points.data = (unsigned char *)m_Mesh->LockVertexBuffer() + att.VertexStart * desc.points.stride;
	desc.triangles.count = att.FaceCount;
	if (m_Mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
	}
	desc.triangles.data = (unsigned char *)m_Mesh->LockIndexBuffer() + att.FaceStart * desc.triangles.stride;
	physx::PxDefaultMemoryOutputStream writeBuffer;
	bool status = PhysxSdk::getSingleton().m_Cooking->cookTriangleMesh(desc, writeBuffer);
	m_Mesh->UnlockIndexBuffer();
	m_Mesh->UnlockVertexBuffer();
	if (!status)
	{
		return;
	}
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	triangle_mesh.reset(PhysxSdk::getSingleton().m_sdk->createTriangleMesh(readBuffer), PhysxDeleter<physx::PxTriangleMesh>());

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxTriangleMeshGeometry(triangle_mesh.get(), mesh_scaling, physx::PxMeshGeometryFlags()),
		*m_PxMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

	m_PxShape->userData = this;
}

void MeshComponent::CreateConvexMeshShape(bool bInflateConvex, unsigned int filterWord0)
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

	if (!m_Mesh)
	{
		return;
	}

	const D3DXATTRIBUTERANGE& att = m_Mesh->m_AttribTable[m_MeshSubMeshId];
	boost::shared_ptr<physx::PxConvexMesh> convex_mesh;
	physx::PxConvexMeshDesc desc;
	desc.points.count = att.VertexCount;
	desc.points.stride = m_Mesh->GetNumBytesPerVertex();
	desc.points.data = (unsigned char *)m_Mesh->LockVertexBuffer() + att.VertexStart * desc.points.stride;
	desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
	if (bInflateConvex)
	{
		desc.flags |= physx::PxConvexFlag::eINFLATE_CONVEX;
	}
	desc.vertexLimit = 256;
	physx::PxDefaultMemoryOutputStream writeBuffer;
	bool status = PhysxSdk::getSingleton().m_Cooking->cookConvexMesh(desc, writeBuffer);
	m_Mesh->UnlockVertexBuffer();
	if (!status)
	{
		return;
	}
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	convex_mesh.reset(PhysxSdk::getSingleton().m_sdk->createConvexMesh(readBuffer), PhysxDeleter<physx::PxConvexMesh>());

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxConvexMeshGeometry(convex_mesh.get(), mesh_scaling),
		*m_PxMaterial, true, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

	m_PxShape->userData = this;
}

ClothComponent::~ClothComponent(void)
{
	if (m_Cloth && m_Cloth->getScene())
	{
		_ASSERT(false); LeavePhysxScene((PhysxScene*)m_Cloth->getScene()->userData);
	}
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
	PhysxSerializationContext* pxar = dynamic_cast<PhysxSerializationContext*>(&ar);
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
	ar << BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar << BOOST_SERIALIZATION_NVP(m_VertexElems);
	ar << BOOST_SERIALIZATION_NVP(m_particles);

	boost::shared_ptr<physx::PxCollection> collection(PxCreateCollection(), PhysxDeleter<physx::PxCollection>());
	collection->add(*m_Cloth);
	physx::PxSerialization::complete(*collection, *pxar->m_Registry);
	physx::PxDefaultMemoryOutputStream ostr;
	physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *pxar->m_Registry);
	unsigned int ClothSize = ostr.getSize();
	ar << BOOST_SERIALIZATION_NVP(ClothSize);
	ar << boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));

	ar << BOOST_SERIALIZATION_NVP(m_ClothSpheres);
}

template<class Archive>
void ClothComponent::load(Archive & ar, const unsigned int version)
{
	PhysxSerializationContext* pxar = dynamic_cast<PhysxSerializationContext*>(&ar);
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
	ar >> BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar >> BOOST_SERIALIZATION_NVP(m_VertexElems);
	ar >> BOOST_SERIALIZATION_NVP(m_particles);

	unsigned int ClothSize;
	ar >> BOOST_SERIALIZATION_NVP(ClothSize);
	m_SerializeBuff.reset((unsigned char *)_aligned_malloc(ClothSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
	ar >> boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(m_SerializeBuff.get(), ClothSize));
	boost::shared_ptr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *pxar->m_Registry), PhysxDeleter<physx::PxCollection>());
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
}

void ClothComponent::ReleaseResource(void)
{
	m_Decl.Release();

	Component::ReleaseResource();
}

void ClothComponent::EnterPhysxScene(PhysxScene * scene)
{
	Component::EnterPhysxScene(scene);

	if (m_Cloth)
	{
		scene->m_PxScene->addActor(*m_Cloth);
	}

	scene->m_EventPxThreadSubstep.connect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, boost::placeholders::_1));
}

void ClothComponent::LeavePhysxScene(PhysxScene * scene)
{
	_ASSERT(!m_Cloth || m_Cloth->getScene() == scene->m_PxScene.get());

	scene->m_EventPxThreadSubstep.disconnect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, boost::placeholders::_1));

	if (m_Cloth)
	{
		scene->m_PxScene->removeActor(*m_Cloth);
	}

	Component::LeavePhysxScene(scene);
}

void ClothComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(!m_VertexData.empty());

	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);

	if (m_bUseAnimation && m_Actor && m_Actor->m_Animation)
	{
		if (!m_Actor->m_Animation->m_DualQuats.empty())
		{
			shader->SetMatrixArray(handle_dualquat, &m_Actor->m_Animation->m_DualQuats[0], m_Actor->m_Animation->m_DualQuats.size());
		}
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

bool ClothComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	bool ret = false;

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
					D3DXMACRO macro[2] = { { 0 } };
					int j = 0;
					if (m_bUseAnimation)
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
							if (m_bUseAnimation && m_Actor && m_Actor->m_Animation)
							{
								BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));
							}
						}

						pipeline->PushIndexedPrimitiveUP(PassID, m_Decl, D3DPT_TRIANGLELIST,
							0, m_VertexData.size() / m_VertexStride, m_IndexData.size() / 3, &m_IndexData[0], D3DFMT_INDEX16, &m_VertexData[0], m_VertexStride, shader, this, m_Material.get(), 0);

						ret = true;
					}
				}
			}
		}
	}

	return ret;
}

void ClothComponent::Update(float fElapsedTime)
{
	UpdateCloth();
}

void ClothComponent::UpdateCloth(void)
{
	if (m_Cloth)
	{
		m_Cloth->setTargetPose(physx::PxTransform((physx::PxVec3&)m_Actor->m_Position, (physx::PxQuat&)m_Actor->m_Rotation));

		_ASSERT(m_particles.size() == m_VertexData.size() / m_VertexStride);
		physx::PxClothParticleData * readData = m_Cloth->lockParticleData(physx::PxDataAccessFlag::eWRITABLE);
		if (readData)
		{
			unsigned char * pVertices = &m_VertexData[0];
			const DWORD NbParticles = m_Cloth->getNbParticles();
			if (m_bUseAnimation && m_Actor && m_Actor->m_Animation && !m_Actor->m_Animation->m_DualQuats.empty())
			{
				for (unsigned int i = 0; i < NbParticles; i++)
				{
					void * pVertex = pVertices + i * m_VertexStride;
					if (readData->particles[i].invWeight == 0)
					{
						my::Vector3 pos = m_Actor->m_Animation->m_DualQuats.TransformVertexWithDualQuaternionList(
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

		if (m_bUseAnimation && m_Actor && m_Actor->m_Animation && !m_Actor->m_Animation->m_DualQuats.empty())
		{
			for (unsigned int i = 0; i < m_ClothSpheres.size(); i++)
			{
				m_ClothSpheresTmp[i].radius = m_ClothSpheres[i].first.radius;
				if (m_ClothSpheres[i].second >= 0)
				{
					Matrix4 & dual = m_Actor->m_Animation->m_DualQuats[m_ClothSpheres[i].second];
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

void EmitterComponent::RequestResource(void)
{
	Component::RequestResource();
}

void EmitterComponent::ReleaseResource(void)
{
	Component::ReleaseResource();
}

void EmitterComponent::Update(float fElapsedTime)
{
}

void EmitterComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);
}

my::AABB EmitterComponent::CalculateAABB(void) const
{
	_ASSERT(m_Actor);

	if (!m_ParticleList.empty())
	{
		Matrix4 worldToLocal = m_Actor->m_World.inverse();
		AABB ret = AABB::Invalid();
		ParticleList::const_iterator part_iter = m_ParticleList.begin();
		for (; part_iter != m_ParticleList.end(); part_iter++)
		{
			ret.unionSelf(AABB(part_iter->m_Position.transformCoord(worldToLocal), part_iter->m_Size.x * 0.5f));
		}
		return ret;
	}
	return Component::CalculateAABB();
}

bool EmitterComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	bool ret = false;

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

					pipeline->PushEmitter(PassID, this, shader, m_Material.get(), 0, this);

					ret = true;
				}
			}
		}
	}

	return ret;
}

template<class Archive>
void StaticEmitterComponent::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	ParticleList::capacity_type buffer_capacity = m_ParticleList.capacity();
	ar << BOOST_SERIALIZATION_NVP(buffer_capacity);
	boost::serialization::stl::save_collection<Archive, ParticleList>(ar, m_ParticleList);
}

template<class Archive>
void StaticEmitterComponent::load(Archive& ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	ParticleList::capacity_type buffer_capacity;
	ar >> BOOST_SERIALIZATION_NVP(buffer_capacity);
	m_ParticleList.set_capacity(buffer_capacity);
	boost::serialization::item_version_type item_version(0);
	boost::serialization::collection_size_type count;
	ar >> BOOST_SERIALIZATION_NVP(count);
	ar >> BOOST_SERIALIZATION_NVP(item_version);
	m_ParticleList.resize(count);
	boost::serialization::stl::collection_load_impl<Archive, ParticleList>(ar, m_ParticleList, count, item_version);
}

void StaticEmitterComponent::CopyFrom(const StaticEmitterComponent & rhs)
{
	EmitterComponent::CopyFrom(rhs);
	// TODO:
}

ComponentPtr StaticEmitterComponent::Clone(void) const
{
	StaticEmitterComponentPtr ret(new StaticEmitterComponent());
	ret->CopyFrom(*this);
	return ret;
}

void StaticEmitterComponent::Update(float fElapsedTime)
{
	EmitterComponent::Update(fElapsedTime);
}

template<class Archive>
void SphericalEmitterComponent::save(Archive & ar, const unsigned int version) const
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
void SphericalEmitterComponent::load(Archive & ar, const unsigned int version)
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

void SphericalEmitterComponent::CopyFrom(const SphericalEmitterComponent & rhs)
{
	EmitterComponent::CopyFrom(rhs);
	// TODO:
}

ComponentPtr SphericalEmitterComponent::Clone(void) const
{
	SphericalEmitterComponentPtr ret(new SphericalEmitterComponent());
	ret->CopyFrom(*this);
	return ret;
}

void SphericalEmitterComponent::RequestResource(void)
{
	EmitterComponent::RequestResource();

	m_SpawnTime = D3DContext::getSingleton().m_fTotalTime;
}

void SphericalEmitterComponent::Update(float fElapsedTime)
{
	_ASSERT(m_SpawnInterval > 0);

	EmitterComponent::Update(fElapsedTime);

	RemoveParticleBefore(D3DContext::getSingleton().m_fTotalTime - m_ParticleLifeTime);

	for (; m_SpawnTime < D3DContext::getSingleton().m_fTotalTime; m_SpawnTime += m_SpawnInterval)
	{
		const float SpawnTimeCycle = Round<float>(m_SpawnTime, 0, m_SpawnCycle);

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
}

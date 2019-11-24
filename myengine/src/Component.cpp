#include "Component.h"
#include "Actor.h"
#include "myDxutApp.h"
#include "myEffect.h"
#include "myResource.h"
#include "Animator.h"
#include "Material.h"
#include "PhysXContext.h"
#include "RenderPipeline.h"
#include "libc.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
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

template<>
void Component::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Type);
	ar << BOOST_SERIALIZATION_NVP(m_LodMask);
}

template<>
void Component::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Type);
	ar >> BOOST_SERIALIZATION_NVP(m_LodMask);
}

void Component::CopyFrom(const Component & rhs)
{
	m_Type = rhs.m_Type;
	m_LodMask = rhs.m_LodMask;
}

ComponentPtr Component::Clone(void) const
{
	ComponentPtr ret(new Component());
	ret->CopyFrom(*this);
	return ret;
}

void Component::RequestResource(void)
{
}

void Component::ReleaseResource(void)
{
}

void Component::OnEnterPxScene(PhysXSceneContext * scene)
{
}

void Component::OnLeavePxScene(PhysXSceneContext * scene)
{
}

void Component::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
}

void Component::OnShaderChanged(void)
{
}

void Component::Update(float fElapsedTime)
{
}

void Component::OnWorldUpdated(void)
{
}

my::AABB Component::CalculateAABB(void) const
{
	return AABB(-1, 1);
}

void Component::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
}

void Component::CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateBoxShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxBoxGeometry(hx, hy, hz), *m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
}

void Component::CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateCapsuleShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxCapsuleGeometry(radius, halfHeight), *m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
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
		&& !m_Actor->m_PxActor->isRigidBody()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxPlaneGeometry(), *m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
}

void Component::CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		DxutApp::getSingleton().m_EventLog("Component::CreateSphereShape failed: !m_Actor || !m_Actor->m_PxActor");
		return;
	}

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxSphereGeometry(radius), *m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot));

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
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

template<>
void MeshComponent::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_MeshPath);
	ar << BOOST_SERIALIZATION_NVP(m_bInstance);
	ar << BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar << BOOST_SERIALIZATION_NVP(m_MaterialList);
}

template<>
void MeshComponent::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshPath);
	ar >> BOOST_SERIALIZATION_NVP(m_bInstance);
	ar >> BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar >> BOOST_SERIALIZATION_NVP(m_MaterialList);
}

void MeshComponent::CopyFrom(const MeshComponent & rhs)
{
	Component::CopyFrom(rhs);
	m_MeshPath = rhs.m_MeshPath;
	m_bInstance = rhs.m_bInstance;
	m_bUseAnimation = rhs.m_bUseAnimation;
	m_MaterialList.resize(rhs.m_MaterialList.size());
	for (unsigned int i = 0; i < rhs.m_MaterialList.size(); i++)
	{
		m_MaterialList[i] = rhs.m_MaterialList[i]->Clone();
	}
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
		m_MeshEventReady(&my::ControlEventArgs(NULL));
	}
}

void MeshComponent::RequestResource(void)
{
	Component::RequestResource();

	if (!m_MeshPath.empty())
	{
		_ASSERT(!m_Mesh);

		my::ResourceMgr::getSingleton().LoadMeshAsync(m_MeshPath.c_str(), this);
	}

	MaterialPtrList::iterator mtl_iter = m_MaterialList.begin();
	for (; mtl_iter != m_MaterialList.end(); mtl_iter++)
	{
		(*mtl_iter)->RequestResource();
	}
}

void MeshComponent::ReleaseResource(void)
{
	if (!m_MeshPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_MeshPath, this);

		m_Mesh.reset();
	}

	MaterialPtrList::iterator mtl_iter = m_MaterialList.begin();
	for (; mtl_iter != m_MaterialList.end(); mtl_iter++)
	{
		(*mtl_iter)->ReleaseResource();
	}

	Component::ReleaseResource();
}

void MeshComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	if (m_bUseAnimation && m_Actor && m_Actor->m_Animator)
	{
		if (!m_Actor->m_Animator->m_DualQuats.empty())
		{
			shader->SetMatrixArray(handle_dualquat, &m_Actor->m_Animator->m_DualQuats[0], m_Actor->m_Animator->m_DualQuats.size());
		}
	}
}

void MeshComponent::OnShaderChanged(void)
{
	handle_World = NULL;
	handle_dualquat = NULL;
	MaterialPtrList::iterator mtl_iter = m_MaterialList.begin();
	for (; mtl_iter != m_MaterialList.end(); mtl_iter++)
	{
		(*mtl_iter)->ParseShaderParameters();
	}
}

void MeshComponent::Update(float fElapsedTime)
{
}

my::AABB MeshComponent::CalculateAABB(void) const
{
	if (m_Mesh)
	{
		m_Mesh->m_aabb;
	}
	return Component::CalculateAABB();
}

void MeshComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_Actor);

	if (m_Mesh)
	{
		for (DWORD i = 0; i < m_MaterialList.size(); i++)
		{
			if (m_MaterialList[i] && (m_MaterialList[i]->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (m_MaterialList[i]->m_PassMask & PassMask))
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
						my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, macro, m_MaterialList[i]->m_Shader.c_str(), PassID);
						if (shader)
						{
							if (!handle_World)
							{
								BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
								if (m_bUseAnimation && m_Actor && m_Actor->m_Animator)
								{
									BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));
								}
							}

							if (m_bInstance)
							{
								pipeline->PushMeshInstance(PassID, m_Mesh.get(), i, shader, this, m_MaterialList[i].get(), i);
							}
							else
							{
								pipeline->PushMesh(PassID, m_Mesh.get(), i, shader, this, m_MaterialList[i].get(), i);
							}
						}
					}
				}
			}
		}
	}
}

void MeshComponent::CreateTriangleMeshShape(unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		return;
	}

	if (m_Actor->m_PxActor->getType() == physx::PxActorType::eRIGID_DYNAMIC
		&& !m_Actor->m_PxActor->isRigidBody()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	if (!m_Mesh)
	{
		return;
	}

	std::string key = my::ResourceMgr::getSingleton().GetResourceKey(m_Mesh);
	if (key.empty())
	{
		return;
	}

	key += "triangle";
	boost::shared_ptr<physx::PxTriangleMesh> triangle_mesh;
	PhysXSceneContext::PxObjectMap::iterator collection_obj_iter = PhysXSceneContext::getSingleton().m_CollectionObjs.find(key);
	if (collection_obj_iter != PhysXSceneContext::getSingleton().m_CollectionObjs.end())
	{
		triangle_mesh.reset(collection_obj_iter->second, collection_obj_iter->second->is<physx::PxTriangleMesh>());
	}
	else
	{
		physx::PxTriangleMeshDesc desc;
		desc.points.count = m_Mesh->GetNumVertices();
		desc.points.stride = m_Mesh->GetNumBytesPerVertex();
		desc.points.data = m_Mesh->LockVertexBuffer();
		desc.triangles.count = m_Mesh->GetNumFaces();
		if (m_Mesh->GetOptions() & D3DXMESH_32BIT)
		{
			desc.triangles.stride = 3 * sizeof(DWORD);
		}
		else
		{
			desc.triangles.stride = 3 * sizeof(WORD);
			desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		}
		desc.triangles.data = m_Mesh->LockIndexBuffer();
		physx::PxDefaultMemoryOutputStream writeBuffer;
		bool status = PhysXContext::getSingleton().m_Cooking->cookTriangleMesh(desc, writeBuffer);
		m_Mesh->UnlockIndexBuffer();
		m_Mesh->UnlockVertexBuffer();
		if (!status)
		{
			return;
		}
		physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		triangle_mesh.reset(PhysXContext::getSingleton().m_sdk->createTriangleMesh(readBuffer), PhysXDeleter<physx::PxTriangleMesh>());
		PhysXSceneContext::getSingleton().m_CollectionObjs.insert(std::make_pair(key, triangle_mesh));
	}

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat::createIdentity());
	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(physx::PxTriangleMeshGeometry(triangle_mesh.get(), mesh_scaling, physx::PxMeshGeometryFlags()),
		*m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
}

void MeshComponent::CreateConvexMeshShape(bool bInflateConvex, unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
	{
		return;
	}

	if (m_Actor->m_PxActor->getType() == physx::PxActorType::eRIGID_DYNAMIC
		&& !m_Actor->m_PxActor->isRigidBody()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	if (!m_Mesh)
	{
		return;
	}

	std::string key = my::ResourceMgr::getSingleton().GetResourceKey(m_Mesh);
	if (key.empty())
	{
		return;
	}

	key += "convex";
	if (bInflateConvex)
	{
		key += "inflate";
	}
	boost::shared_ptr<physx::PxConvexMesh> convex_mesh;
	PhysXSceneContext::PxObjectMap::iterator collection_obj_iter = PhysXSceneContext::getSingleton().m_CollectionObjs.find(key);
	if (collection_obj_iter != PhysXSceneContext::getSingleton().m_CollectionObjs.end())
	{
		convex_mesh.reset(collection_obj_iter->second, collection_obj_iter->second->is<physx::PxConvexMesh>());
	}
	else
	{
		physx::PxConvexMeshDesc desc;
		desc.points.count = m_Mesh->GetNumVertices();
		desc.points.stride = m_Mesh->GetNumBytesPerVertex();
		desc.points.data = m_Mesh->LockVertexBuffer();
		desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		if (bInflateConvex)
		{
			desc.flags |= physx::PxConvexFlag::eINFLATE_CONVEX;
		}
		desc.vertexLimit = 256;
		physx::PxDefaultMemoryOutputStream writeBuffer;
		bool status = PhysXContext::getSingleton().m_Cooking->cookConvexMesh(desc, writeBuffer);
		m_Mesh->UnlockVertexBuffer();
		if (!status)
		{
			return;
		}
		physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		convex_mesh.reset(PhysXContext::getSingleton().m_sdk->createConvexMesh(readBuffer), PhysXDeleter<physx::PxConvexMesh>());
		PhysXSceneContext::getSingleton().m_CollectionObjs.insert(std::make_pair(key, convex_mesh));
	}

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)m_Actor->m_Scale, physx::PxQuat::createIdentity());
	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(physx::PxConvexMeshGeometry(convex_mesh.get(), mesh_scaling),
		*m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
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
	}
}

template<>
void ClothComponent::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_AttribTable);
	unsigned int VertexSize = m_VertexData.size();
	ar << BOOST_SERIALIZATION_NVP(VertexSize);
	ar << boost::serialization::make_nvp("m_VertexData", boost::serialization::binary_object((void *)&m_VertexData[0], VertexSize));
	ar << BOOST_SERIALIZATION_NVP(m_VertexStride);
	unsigned int IndexSize = m_IndexData.size() * sizeof(unsigned short);
	ar << BOOST_SERIALIZATION_NVP(IndexSize);
	ar << boost::serialization::make_nvp("m_IndexData", boost::serialization::binary_object((void *)&m_IndexData[0], IndexSize));
	ar << BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar << BOOST_SERIALIZATION_NVP(m_MaterialList);
	ar << BOOST_SERIALIZATION_NVP(m_VertexElems);
	ar << BOOST_SERIALIZATION_NVP(m_particles);

	boost::shared_ptr<physx::PxCollection> collection(PxCreateCollection(), PhysXDeleter<physx::PxCollection>());
	collection->add(*m_Cloth);
	physx::PxSerialization::complete(*collection, *PhysXSceneContext::getSingleton().m_Registry, PhysXSceneContext::getSingleton().m_Collection.get());
	physx::PxDefaultMemoryOutputStream ostr;
	physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *PhysXSceneContext::getSingleton().m_Registry, PhysXSceneContext::getSingleton().m_Collection.get());
	unsigned int ClothSize = ostr.getSize();
	ar << BOOST_SERIALIZATION_NVP(ClothSize);
	ar << boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	ar << BOOST_SERIALIZATION_NVP(m_particles);
}

template<>
void ClothComponent::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_AttribTable);
	unsigned int VertexSize;
	ar >> BOOST_SERIALIZATION_NVP(VertexSize);
	m_VertexData.resize(VertexSize);
	ar >> boost::serialization::make_nvp("m_VertexData", boost::serialization::binary_object((void *)&m_VertexData[0], VertexSize));
	ar >> BOOST_SERIALIZATION_NVP(m_VertexStride);
	unsigned int IndexSize;
	ar >> BOOST_SERIALIZATION_NVP(IndexSize);
	m_IndexData.resize(IndexSize / sizeof(unsigned short));
	ar >> boost::serialization::make_nvp("m_IndexData", boost::serialization::binary_object((void *)&m_IndexData[0], IndexSize));
	ar >> BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar >> BOOST_SERIALIZATION_NVP(m_MaterialList);
	ar >> BOOST_SERIALIZATION_NVP(m_VertexElems);
	ar >> BOOST_SERIALIZATION_NVP(m_particles);

	unsigned int ClothSize;
	ar >> BOOST_SERIALIZATION_NVP(ClothSize);
	m_SerializeBuff.reset((unsigned char *)_aligned_malloc(ClothSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
	ar >> boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(m_SerializeBuff.get(), ClothSize));
	boost::shared_ptr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *PhysXSceneContext::getSingleton().m_Registry, PhysXSceneContext::getSingleton().m_Collection.get()), PhysXDeleter<physx::PxCollection>());
	ar >> BOOST_SERIALIZATION_NVP(m_particles);
	const unsigned int numObjs = collection->getNbObjects();
	for (unsigned int i = 0; i < numObjs; i++)
	{
		physx::PxBase * obj = &collection->getObject(i);
		switch (obj->getConcreteType())
		{
		case physx::PxConcreteType::eCLOTH_FABRIC:
			m_Fabric.reset(obj->is<physx::PxClothFabric>(), PhysXDeleter<physx::PxClothFabric>());
			break;
		case physx::PxConcreteType::eCLOTH:
			m_Cloth.reset(obj->is<physx::PxCloth>(), PhysXDeleter<physx::PxCloth>());
			break;
		}
	}

	std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);
	HRESULT hr;
	if (FAILED(hr = D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(&velist[0], &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}
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

void ClothComponent::CreateClothFromMesh(my::OgreMeshPtr mesh)
{
	if (m_VertexData.empty())
	{
		m_VertexStride = mesh->GetNumBytesPerVertex();
		m_VertexData.resize(mesh->GetNumVertices() * m_VertexStride);
		memcpy(&m_VertexData[0], mesh->LockVertexBuffer(), m_VertexData.size());
		mesh->UnlockVertexBuffer();

		m_IndexData.resize(mesh->GetNumFaces() * 3);
		if (mesh->GetNumVertices() > USHRT_MAX)
		{
			THROW_CUSEXCEPTION(str_printf("create deformation mesh with overflow index size %u", m_IndexData.size()));
		}
		VOID * pIndices = mesh->LockIndexBuffer();
		for (unsigned int face_i = 0; face_i < mesh->GetNumFaces(); face_i++)
		{
			if(mesh->GetOptions() & D3DXMESH_32BIT)
			{
				m_IndexData[face_i * 3 + 0] = (WORD)*((DWORD *)pIndices + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = (WORD)*((DWORD *)pIndices + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = (WORD)*((DWORD *)pIndices + face_i * 3 + 2);
			}
			else
			{
				m_IndexData[face_i * 3 + 0] = *((WORD *)pIndices + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = *((WORD *)pIndices + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = *((WORD *)pIndices + face_i * 3 + 2);
			}
		}
		mesh->UnlockIndexBuffer();

		m_AttribTable = mesh->m_AttribTable;
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

		m_particles.resize(mesh->GetNumVertices());
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
		desc.points.count = mesh->GetNumVertices();
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.triangles.data = &m_IndexData[0];
		desc.triangles.count = mesh->GetNumFaces();
		desc.triangles.stride = 3 * sizeof(unsigned short);
		desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		boost::shared_ptr<physx::PxClothFabric> fabric(PxClothFabricCreate(
			*PhysXContext::getSingleton().m_sdk, desc, (physx::PxVec3&)my::Vector3::Gravity, true), PhysXDeleter<physx::PxClothFabric>());
		m_Cloth.reset(PhysXContext::getSingleton().m_sdk->createCloth(
			physx::PxTransform::createIdentity(), *fabric, &m_particles[0], physx::PxClothFlags()), PhysXDeleter<physx::PxCloth>());
	}
}

void ClothComponent::RequestResource(void)
{
	Component::RequestResource();

	MaterialPtrList::iterator mtl_iter = m_MaterialList.begin();
	for (; mtl_iter != m_MaterialList.end(); mtl_iter++)
	{
		(*mtl_iter)->RequestResource();
	}
}

void ClothComponent::ReleaseResource(void)
{
	MaterialPtrList::iterator mtl_iter = m_MaterialList.begin();
	for (; mtl_iter != m_MaterialList.end(); mtl_iter++)
	{
		(*mtl_iter)->ReleaseResource();
	}

	Component::ReleaseResource();
}

void ClothComponent::OnEnterPxScene(PhysXSceneContext * scene)
{
	Component::OnEnterPxScene(scene);

	if (m_Cloth)
	{
		scene->m_PxScene->addActor(*m_Cloth);

		scene->m_EventPxThreadSubstep.connect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, _1));
	}
}

void ClothComponent::OnLeavePxScene(PhysXSceneContext * scene)
{
	if (m_Cloth)
	{
		scene->m_PxScene->removeActor(*m_Cloth);

		scene->m_EventPxThreadSubstep.disconnect(boost::bind(&ClothComponent::OnPxThreadSubstep, this, _1));
	}

	Component::OnLeavePxScene(scene);
}

void ClothComponent::OnResetDevice(void)
{
}

void ClothComponent::OnLostDevice(void)
{
}

void ClothComponent::OnDestroyDevice(void)
{
	m_Decl.Release();
}

void ClothComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(!m_VertexData.empty());

	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	if (m_bUseAnimation && m_Actor && m_Actor->m_Animator)
	{
		if (!m_Actor->m_Animator->m_DualQuats.empty())
		{
			shader->SetMatrixArray(handle_dualquat, &m_Actor->m_Animator->m_DualQuats[0], m_Actor->m_Animator->m_DualQuats.size());
		}
	}
}

void ClothComponent::OnShaderChanged(void)
{
	handle_World = NULL;
	handle_dualquat = NULL;
	MaterialPtrList::iterator mtl_iter = m_MaterialList.begin();
	for (; mtl_iter != m_MaterialList.end(); mtl_iter++)
	{
		(*mtl_iter)->ParseShaderParameters();
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
		for (unsigned int i = 0; i < m_AttribTable.size(); i++)
		{
			_ASSERT(!m_VertexData.empty());
			_ASSERT(!m_IndexData.empty());
			_ASSERT(0 != m_VertexStride);
			if (m_MaterialList[i] && (m_MaterialList[i]->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (m_MaterialList[i]->m_PassMask & PassMask))
					{
						my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, NULL, m_MaterialList[i]->m_Shader.c_str(), PassID);
						if (shader)
						{
							if (!handle_World)
							{
								BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
								if (m_bUseAnimation && m_Actor && m_Actor->m_Animator)
								{
									BOOST_VERIFY(handle_dualquat = shader->GetParameterByName(NULL, "g_dualquat"));
								}
							}

							pipeline->PushIndexedPrimitiveUP(PassID, m_Decl, D3DPT_TRIANGLELIST,
								m_AttribTable[i].VertexStart,
								m_AttribTable[i].VertexCount,
								m_AttribTable[i].FaceCount,
								&m_IndexData[m_AttribTable[i].FaceStart * 3],
								D3DFMT_INDEX16,
								&m_VertexData[0],
								m_VertexStride, shader, this, m_MaterialList[i].get(), i);
						}
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
			if (m_bUseAnimation && m_Actor && m_Actor->m_Animator && !m_Actor->m_Animator->m_DualQuats.empty())
			{
				for (unsigned int i = 0; i < NbParticles; i++)
				{
					void * pVertex = pVertices + i * m_VertexStride;
					if (readData->particles[i].invWeight == 0)
					{
						my::Vector3 pos = m_Actor->m_Animator->m_DualQuats.TransformVertexWithDualQuaternionList(
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

void ClothComponent::OnWorldUpdated(void)
{
	_ASSERT(m_Actor);

	if (m_Cloth)
	{
		m_Cloth->setTargetPose(physx::PxTransform((physx::PxVec3&)m_Actor->m_Position, (physx::PxQuat&)m_Actor->m_Rotation));
	}
}

void ClothComponent::OnPxThreadSubstep(float dtime)
{
	_ASSERT(m_Actor);

	if (!m_ClothSpheres.empty())
	{
		m_ClothSpheresTmp.resize(m_ClothSpheres.size());

		if (m_bUseAnimation && m_Actor && m_Actor->m_Animator && !m_Actor->m_Animator->m_DualQuats.empty())
		{
			for (unsigned int i = 0; i < m_ClothSpheres.size(); i++)
			{
				m_ClothSpheresTmp[i].radius = m_ClothSpheres[i].first.radius;
				if (m_ClothSpheres[i].second >= 0)
				{
					Matrix4 & dual = m_Actor->m_Animator->m_DualQuats[m_ClothSpheres[i].second];
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
	// TODO:
}

void EmitterComponent::Spawn(const my::Vector3 & Position, const my::Vector3 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle)
{
	_ASSERT(m_Actor);

	if (!m_EmitterToWorld)
	{
		Emitter::Spawn(Position, Velocity, Color, Size, Angle);
	}
	else
	{
		Emitter::Spawn(Position.transformCoord(m_Actor->m_World), Velocity.transformNormal(m_Actor->m_World), Color, Size * m_Actor->m_Scale.xy, Angle);
	}
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

	if (!m_Decl)
	{
		IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
		std::vector<D3DVERTEXELEMENT9> elems = m_VertexElems.BuildVertexElementList(0);
		elems.insert(elems.end(), RenderPipeline::m_ParticleIEList.begin(), RenderPipeline::m_ParticleIEList.end());
		D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
		elems.push_back(ve_end);
		HRESULT hr;
		V(pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl));

		m_VertexStride = D3DXGetDeclVertexSize(&elems[0], 0);

		_ASSERT(!m_vb.m_ptr);
		m_vb.CreateVertexBuffer(m_VertexStride * 4, 0, 0, D3DPOOL_MANAGED);
		unsigned char * pVertices = (unsigned char *)m_vb.Lock(0, m_VertexStride * 4);
		m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 0, Vector2(0, 0));
		m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 1, Vector2(0, 1));
		m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 2, Vector2(1, 1));
		m_VertexElems.SetTexcoord(pVertices + m_VertexStride * 3, Vector2(1, 0));
		m_vb.Unlock();

		_ASSERT(!m_ib.m_ptr);
		m_ib.CreateIndexBuffer(sizeof(WORD) * 4, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
		WORD * pIndices = (WORD *)m_ib.Lock(0, sizeof(WORD) * 4);
		pIndices[0] = 0;
		pIndices[1] = 1;
		pIndices[2] = 2;
		pIndices[3] = 3;
		m_ib.Unlock();
	}

	m_Material->RequestResource();
}

void EmitterComponent::ReleaseResource(void)
{
	m_Decl.Release();

	m_vb.OnDestroyDevice();

	m_ib.OnDestroyDevice();

	m_Material->ReleaseResource();

	Component::ReleaseResource();
}

void EmitterComponent::Update(float fElapsedTime)
{
}

void EmitterComponent::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	if (!m_EmitterToWorld)
	{
		shader->SetMatrix(handle_World, m_Actor->m_World);
	}
	else
	{
		shader->SetMatrix(handle_World, Matrix4::identity);
	}

	shader->SetVector(handle_ParticleOffset, m_ParticleOffset);
}

void EmitterComponent::OnShaderChanged(void)
{
	handle_World = NULL;
	handle_ParticleOffset = NULL;
	m_Material->ParseShaderParameters();
}

my::AABB EmitterComponent::CalculateAABB(void) const
{
	if (!m_ParticleList.empty())
	{
		AABB ret = AABB::Invalid();
		ParticleList::const_iterator part_iter = m_ParticleList.begin();
		for (; part_iter != m_ParticleList.end(); part_iter++)
		{
			ret.unionSelf(AABB(part_iter->m_Position, part_iter->m_Size.x * 0.5f));
		}
		return ret;
	}
	return Component::CalculateAABB();
}

void EmitterComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (m_Decl && m_Material && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				D3DXMACRO macro[2] = { {0} };
				macro[0].Name = "FACETOCAMERA";
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeParticle, macro, m_Material->m_Shader.c_str(), PassID);
				if (shader)
				{
					if (!handle_World)
					{
						BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
						BOOST_VERIFY(handle_ParticleOffset = shader->GetParameterByName(NULL, "g_ParticleOffset"));
					}

					if (!m_EmitterToWorld)
					{
						pipeline->PushEmitter(PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLEFAN, 4, m_VertexStride, 2, this, shader, this, m_Material.get(), 0);
					}
					else
					{
						pipeline->PushWorldEmitter(PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLEFAN, 4, m_VertexStride, 2, this, shader, this, m_Material.get(), 0);
					}
				}
			}
		}
	}
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
}

template<>
void SphericalEmitterComponent::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
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
	ar << BOOST_SERIALIZATION_NVP(m_SpawnLoopTime);
}

template<>
void SphericalEmitterComponent::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
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
	ar >> BOOST_SERIALIZATION_NVP(m_SpawnLoopTime);
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

void SphericalEmitterComponent::Update(float fElapsedTime)
{
	RemoveDeadParticle(m_ParticleLifeTime);

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	float SpawnTime = fmod(D3DContext::getSingleton().m_fTotalTime, m_SpawnLoopTime);

	while(m_RemainingSpawnTime >= 0)
	{
		Spawn(
			Vector3(
				Random(m_HalfSpawnArea.x, m_HalfSpawnArea.x),
				Random(m_HalfSpawnArea.y, m_HalfSpawnArea.y),
				Random(m_HalfSpawnArea.z, m_HalfSpawnArea.z)),
			Vector3::SphericalToCartesian(
				m_SpawnSpeed,
				m_SpawnInclination.Interpolate(SpawnTime, 0),
				m_SpawnAzimuth.Interpolate(SpawnTime, 0)),
			Vector4(
				m_SpawnColorR.Interpolate(SpawnTime, 1),
				m_SpawnColorG.Interpolate(SpawnTime, 1),
				m_SpawnColorB.Interpolate(SpawnTime, 1),
				m_SpawnColorA.Interpolate(SpawnTime, 1)),
			Vector2(
				m_SpawnSizeX.Interpolate(SpawnTime, 1)),
			m_SpawnAngle.Interpolate(SpawnTime, 0));

		m_RemainingSpawnTime -= m_SpawnInterval;
	}
}

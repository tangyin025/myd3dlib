#include "StdAfx.h"
#include "Component.h"
#include "Actor.h"
#include "Terrain.h"
#include "Animator.h"
#include "PhysXContext.h"
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

BOOST_CLASS_EXPORT(Material)

BOOST_CLASS_EXPORT(Component)

BOOST_CLASS_EXPORT(RenderComponent)

BOOST_CLASS_EXPORT(MeshComponent)

BOOST_CLASS_EXPORT(ClothComponent)

BOOST_CLASS_EXPORT(StaticEmitterComponent)

BOOST_CLASS_EXPORT(SphericalEmitterComponent)
//
//BOOST_CLASS_EXPORT(RigidComponent)

void Material::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetVector("g_MeshColor", m_MeshColor);
	shader->SetTexture("g_MeshTexture", m_MeshTexture.m_Res.get());
	shader->SetTexture("g_NormalTexture", m_NormalTexture.m_Res.get());
	shader->SetTexture("g_SpecularTexture", m_SpecularTexture.m_Res.get());
}

void Material::RequestResource(void)
{
	if (!m_MeshTexture.m_Path.empty())
	{
		m_MeshTexture.RequestResource();
	}
	if (!m_NormalTexture.m_Path.empty())
	{
		m_NormalTexture.RequestResource();
	}
	if (!m_SpecularTexture.m_Path.empty())
	{
		m_SpecularTexture.RequestResource();
	}
}

void Material::ReleaseResource(void)
{
	m_MeshTexture.ReleaseResource();
	m_NormalTexture.ReleaseResource();
	m_SpecularTexture.ReleaseResource();
}

template<>
void Component::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar << BOOST_SERIALIZATION_NVP(m_Type);
	ar << BOOST_SERIALIZATION_NVP(m_Position);
	ar << BOOST_SERIALIZATION_NVP(m_Rotation);
	ar << BOOST_SERIALIZATION_NVP(m_Scale);
	ar << BOOST_SERIALIZATION_NVP(m_World);
	ar << BOOST_SERIALIZATION_NVP(m_Animator);
	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
}

template<>
void Component::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar >> BOOST_SERIALIZATION_NVP(m_Type);
	ar >> BOOST_SERIALIZATION_NVP(m_Position);
	ar >> BOOST_SERIALIZATION_NVP(m_Rotation);
	ar >> BOOST_SERIALIZATION_NVP(m_Scale);
	ar >> BOOST_SERIALIZATION_NVP(m_World);
	ar >> BOOST_SERIALIZATION_NVP(m_Animator);
	if (m_Animator)
	{
		m_Animator->m_Cmp = this;
	}
	ar >> BOOST_SERIALIZATION_NVP(m_Cmps);
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for(; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Parent = this;
	}
}

void Component::RequestResource(void)
{
	m_Requested = true;

	if (m_Animator)
	{
		m_Animator->RequestResource();
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->RequestResource();
	}
}

void Component::ReleaseResource(void)
{
	m_Requested = false;

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ReleaseResource();
	}

	if (m_Animator)
	{
		m_Animator->ReleaseResource();
	}
}

void Component::OnEnterPxScene(physx::PxScene * scene)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnEnterPxScene(scene);
	}
}

void Component::OnLeavePxScene(physx::PxScene * scene)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnLeavePxScene(scene);
	}
}

void Component::Update(float fElapsedTime)
{
	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->Update(fElapsedTime);
	}
}

my::Matrix4 Component::CalculateLocal(void) const
{
	return my::Matrix4::Compose(m_Scale, m_Rotation, m_Position);
}

void Component::UpdateWorld(const my::Matrix4 & World)
{
	m_World = CalculateLocal() * World;

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateWorld(m_World);
	}
}

my::AABB Component::CalculateAABB(void) const
{
	AABB ret = AABB::Invalid();
	ComponentPtrList::const_iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		ret.unionSelf((*cmp_iter)->CalculateAABB().transform((*cmp_iter)->CalculateLocal()));
	}
	return ret;
}

void Component::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask);
	}
}

void Component::UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateLod(ViewedPos, TargetPos);
	}
}

void Component::AddComponent(ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Parent);
	m_Cmps.push_back(cmp);
	cmp->m_Parent = this;
}

void Component::RemoveComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = std::find(m_Cmps.begin(), m_Cmps.end(), cmp);
	if (cmp_iter != m_Cmps.end())
	{
		_ASSERT((*cmp_iter)->m_Parent == this);
		m_Cmps.erase(cmp_iter);
		(*cmp_iter)->m_Parent = NULL;
	}
}

void Component::ClearAllComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Parent = NULL;
	}
	m_Cmps.clear();
}

bool Component::IsTopParent(ComponentType type)
{
	return type == ComponentTypeActor
		|| type == ComponentTypeCharacter;
}

Actor * Component::GetTopParent(void)
{
	if (m_Parent)
	{
		return m_Parent->GetTopParent();
	}
	_ASSERT(IsTopParent(m_Type));
	return dynamic_cast<Actor *>(this);
}

template<>
void MeshComponent::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar << BOOST_SERIALIZATION_NVP(m_MeshRes);
	ar << BOOST_SERIALIZATION_NVP(m_bInstance);
	ar << BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar << BOOST_SERIALIZATION_NVP(m_MaterialList);
	ar << BOOST_SERIALIZATION_NVP(m_StaticCollision);
}

template<>
void MeshComponent::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar >> BOOST_SERIALIZATION_NVP(m_MeshRes);
	ar >> BOOST_SERIALIZATION_NVP(m_bInstance);
	ar >> BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar >> BOOST_SERIALIZATION_NVP(m_MaterialList);
	ar >> BOOST_SERIALIZATION_NVP(m_StaticCollision);
}

void MeshComponent::RequestResource(void)
{
	Component::RequestResource();

	m_MeshRes.RequestResource();

	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->RequestResource();
	}
}

void MeshComponent::ReleaseResource(void)
{
	m_MeshRes.ReleaseResource();

	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->ReleaseResource();
	}

	Component::ReleaseResource();
}

void MeshComponent::Update(float fElapsedTime)
{
	Component::Update(fElapsedTime);
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(AttribId < m_MaterialList.size());

	shader->SetFloat("g_Time", (float)D3DContext::getSingleton().m_fAbsoluteTime);

	shader->SetMatrix("g_World", m_World);

	if (m_bUseAnimation && m_Parent && m_Parent->m_Animator)
	{
		if (!m_Parent->m_Animator->m_DualQuats.empty())
		{
			shader->SetMatrixArray("g_dualquat", &m_Parent->m_Animator->m_DualQuats[0], m_Parent->m_Animator->m_DualQuats.size());
		}
	}

	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

my::AABB MeshComponent::CalculateAABB(void) const
{
	AABB ret = RenderComponent::CalculateAABB();
	if (m_MeshRes.m_Res)
	{
		ret.unionSelf(m_MeshRes.m_Res->m_aabb);
	}
	return ret;
}

void MeshComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_MeshRes.m_Res)
	{
		for (DWORD i = 0; i < m_MaterialList.size(); i++)
		{
			if (m_MaterialList[i] && (m_MaterialList[i]->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (m_MaterialList[i]->m_PassMask & PassMask))
					{
						my::Effect * shader = pipeline->QueryShader(
							m_bUseAnimation ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, m_bInstance, m_MaterialList[i].get(), PassID);
						if (shader)
						{
							if (m_bInstance)
							{
								pipeline->PushMeshInstance(PassID, m_MeshRes.m_Res.get(), i, m_World, shader, this);
							}
							else
							{
								pipeline->PushMesh(PassID, m_MeshRes.m_Res.get(), i, shader, this);
							}
						}
					}
				}
			}
		}
	}

	Component::AddToPipeline(frustum, pipeline, PassMask);
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
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
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

	PhysXPtr<physx::PxSerializationRegistry> registry(physx::PxSerialization::createSerializationRegistry(*PhysXContext::getSingleton().m_sdk));
	PhysXPtr<physx::PxCollection> collection(PxCreateCollection());
	collection->add(*m_Cloth);
	physx::PxSerialization::complete(*collection, *registry);
	physx::PxDefaultMemoryOutputStream ostr;
	physx::PxSerialization::serializeCollectionToBinary(ostr, *collection, *registry);
	unsigned int ClothSize = ostr.getSize();
	ar << BOOST_SERIALIZATION_NVP(ClothSize);
	ar << boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	ar << BOOST_SERIALIZATION_NVP(m_particles);
}

template<>
void ClothComponent::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
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
	PhysXPtr<physx::PxSerializationRegistry> registry(physx::PxSerialization::createSerializationRegistry(*PhysXContext::getSingleton().m_sdk));
	PhysXPtr<physx::PxCollection> collection(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(),*registry,NULL));
	ar >> BOOST_SERIALIZATION_NVP(m_particles);
	const unsigned int numObjs = collection->getNbObjects();
	for (unsigned int i = 0; i < numObjs; i++)
	{
		physx::PxBase * obj = &collection->getObject(i);
		switch (obj->getConcreteType())
		{
		case physx::PxConcreteType::eCLOTH_FABRIC:
			m_Fabric.reset(obj->is<physx::PxClothFabric>());
			break;
		case physx::PxConcreteType::eCLOTH:
			m_Cloth.reset(obj->is<physx::PxCloth>());
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

void ClothComponent::CreateClothFromMesh(my::OgreMeshPtr mesh, unsigned int bone_id)
{
	if (m_VertexData.empty())
	{
		m_VertexStride = mesh->GetNumBytesPerVertex();
		m_VertexData.resize(mesh->GetNumVertices() * m_VertexStride);
		memcpy(&m_VertexData[0], mesh->LockVertexBuffer(), m_VertexData.size());
		mesh->UnlockVertexBuffer();

		m_IndexData.resize(mesh->GetNumFaces() * 3);
		if (m_IndexData.size() > USHRT_MAX)
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

		m_particles.resize(mesh->GetNumVertices());
		unsigned char * pVertices = (unsigned char *)&m_VertexData[0];
		for(unsigned int i = 0; i < m_particles.size(); i++) {
			unsigned char * pVertex = pVertices + i * m_VertexStride;
			m_particles[i].pos = (physx::PxVec3 &)m_VertexElems.GetPosition(pVertex);
			if (m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
			{
				BOOST_STATIC_ASSERT(4 == my::D3DVertexElementSet::MAX_BONE_INDICES);
				_ASSERT(m_VertexElems.elems[D3DDECLUSAGE_BLENDWEIGHT][0].Type == D3DDECLTYPE_FLOAT4);
				unsigned char * pIndices = (unsigned char *)&m_VertexElems.GetBlendIndices(pVertex);
				my::Vector4 & Weights = m_VertexElems.GetBlendWeight(pVertex);
				m_particles[i].invWeight = 0;
				for (unsigned int j = 0; j < D3DVertexElementSet::MAX_BONE_INDICES; j++)
				{
					m_particles[i].invWeight += (pIndices[j] == bone_id ? Weights[j] : 0);
				}
			}
			else
				m_particles[i].invWeight = 1.0f;
		}

		physx::PxClothMeshDesc desc;
		desc.points.data = (unsigned char *)mesh->LockVertexBuffer(0) + mesh->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
		desc.points.count = mesh->GetNumVertices();
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.triangles.data = mesh->LockIndexBuffer();
		desc.triangles.count = mesh->GetNumFaces();
		if (mesh->GetOptions() & D3DXMESH_32BIT)
		{
			desc.triangles.stride = 3 * sizeof(DWORD);
		}
		else
		{
			desc.triangles.stride = 3 * sizeof(WORD);
			desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		}
		PhysXPtr<physx::PxClothFabric> fabric(PxClothFabricCreate(
			*PhysXContext::getSingleton().m_sdk, desc, (physx::PxVec3&)my::Vector3::Gravity, true));
		m_Cloth.reset(PhysXContext::getSingleton().m_sdk->createCloth(
			physx::PxTransform::createIdentity(), *fabric, &m_particles[0], physx::PxClothFlags()));
	}
}

void ClothComponent::RequestResource(void)
{
	RenderComponent::RequestResource();

	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->RequestResource();
	}
}

void ClothComponent::ReleaseResource(void)
{
	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->ReleaseResource();
	}

	RenderComponent::ReleaseResource();
}

void ClothComponent::OnEnterPxScene(physx::PxScene * scene)
{
	RenderComponent::OnEnterPxScene(scene);

	if (m_Cloth)
	{
		scene->addActor(*m_Cloth);
	}
}

void ClothComponent::OnLeavePxScene(physx::PxScene * scene)
{
	RenderComponent::OnLeavePxScene(scene);

	if (m_Cloth)
	{
		scene->removeActor(*m_Cloth);
	}
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

	if (m_Cloth)
	{
		if (IsRequested())
		{
			PhysXSceneContext::getSingleton().m_PxScene->removeActor(*m_Cloth);
		}
		m_Fabric.reset();
		m_Cloth.reset();
	}

	m_SerializeBuff.reset();
}

void ClothComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(!m_VertexData.empty());
	_ASSERT(AttribId < m_MaterialList.size());

	shader->SetFloat("g_Time", (float)D3DContext::getSingleton().m_fAbsoluteTime);

	shader->SetMatrix("g_World", m_World);

	if (m_bUseAnimation && m_Parent && m_Parent->m_Animator)
	{
		if (!m_Parent->m_Animator->m_DualQuats.empty())
		{
			shader->SetMatrixArray("g_dualquat", &m_Parent->m_Animator->m_DualQuats[0], m_Parent->m_Animator->m_DualQuats.size());
		}
	}

	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

my::AABB ClothComponent::CalculateAABB(void) const
{
	AABB ret = RenderComponent::CalculateAABB();
	if (!m_VertexData.empty())
	{
		unsigned char * pVertices = (unsigned char *)&m_VertexData[0];
		const unsigned int NumVertices = m_VertexData.size() / m_VertexStride;
		for (unsigned int i = 0; i < NumVertices; i++)
		{
			unsigned char * pVertex = pVertices + i * m_VertexStride;
			ret.unionSelf(m_VertexElems.GetPosition(pVertex));
		}
	}
	return ret;
}

void ClothComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
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
						my::Effect * shader = pipeline->QueryShader(
							m_bUseAnimation ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, false, m_MaterialList[i].get(), PassID);
						if (shader)
						{
							pipeline->PushIndexedPrimitiveUP(PassID, m_Decl, D3DPT_TRIANGLELIST,
								m_AttribTable[i].VertexStart,
								m_AttribTable[i].VertexCount,
								m_AttribTable[i].FaceCount,
								&m_IndexData[m_AttribTable[i].FaceStart * 3],
								D3DFMT_INDEX16,
								&m_VertexData[0],
								m_VertexStride, i, shader, this);
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
			m_NewParticles.resize(NbParticles);
			if (m_bUseAnimation
				&& m_Parent && m_Parent->m_Animator && !m_Parent->m_Animator->m_DualQuats.empty()
				&& m_VertexElems.elems[D3DDECLUSAGE_BLENDINDICES][0].Type == D3DDECLTYPE_UBYTE4)
			{
				for (unsigned int i = 0; i < NbParticles; i++)
				{
					void * pVertex = pVertices + i * m_VertexStride;
					m_NewParticles[i].invWeight = readData->particles[i].invWeight;
					my::Vector3 pos = m_Parent->m_Animator->m_DualQuats.TransformVertexWithDualQuaternionList(
						(my::Vector3 &)m_particles[i].pos,
						m_VertexElems.GetBlendIndices(pVertex),
						m_VertexElems.GetBlendWeight(pVertex));
					m_NewParticles[i].pos = (physx::PxVec3 &)pos.lerp((Vector3 &)readData->particles[i].pos, m_NewParticles[i].invWeight);
					m_VertexElems.SetPosition(pVertex, (my::Vector3 &)m_NewParticles[i].pos);
				}
			}
			else
			{
				for (unsigned int i = 0; i < NbParticles; i++)
				{
					void * pVertex = pVertices + i * m_VertexStride;
					m_NewParticles[i].invWeight = readData->particles[i].invWeight;
					m_NewParticles[i].pos = readData->particles[i].pos;
					m_VertexElems.SetPosition(pVertex, (my::Vector3 &)m_NewParticles[i].pos);
				}
			}
			readData->unlock();
			m_Cloth->setParticles(&m_NewParticles[0], NULL);

			my::OgreMesh::ComputeNormalFrame(
				pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);

			my::OgreMesh::ComputeTangentFrame(
				pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);
		}
		m_Cloth->setTargetPose(physx::PxTransform((physx::PxMat44 &)m_World));
	}
}

void EmitterComponent::RequestResource(void)
{
	Component::RequestResource();

	if (m_Material)
	{
		m_Material->RequestResource();
	}
}

void EmitterComponent::ReleaseResource(void)
{
	if (m_Material)
	{
		m_Material->ReleaseResource();
	}

	Component::ReleaseResource();
}

void EmitterComponent::Update(float fElapsedTime)
{
	if (m_Emitter)
	{
		m_Emitter->Update(fElapsedTime);
	}

	Component::Update(fElapsedTime);
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);

	shader->SetFloat("g_Time", m_Emitter->m_Time);

	shader->SetMatrix("g_World", m_World);

	if (m_Material)
	{
		m_Material->OnSetShader(shader, AttribId);
	}
}

my::AABB EmitterComponent::CalculateAABB(void) const
{
	AABB ret = RenderComponent::CalculateAABB();
	ret.unionSelf(my::AABB(-1,1));
	return ret;
}

void EmitterComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_Material && m_Emitter && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeParticle, false, m_Material.get(), PassID);
				if (shader)
				{
					pipeline->PushEmitter(PassID, m_Emitter.get(), 0, shader, this);
				}
			}
		}
	}

	Component::AddToPipeline(frustum, pipeline, PassMask);
}

void StaticEmitterComponent::Update(float fElapsedTime)
{
	EmitterComponent::Update(fElapsedTime);
}

void SphericalEmitterComponent::Update(float fElapsedTime)
{
	EmitterComponent::Update(fElapsedTime);

	m_Emitter->RemoveDeadParticle(m_ParticleLifeTime);

	m_RemainingSpawnTime += fElapsedTime;

	_ASSERT(m_SpawnInterval > 0);

	float SpawnTime = fmod(m_Emitter->m_Time, m_SpawnLoopTime);

	while(m_RemainingSpawnTime >= 0)
	{
		m_Emitter->Spawn(
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
//
//void RigidComponent::CreateRigidActor(const my::Matrix4 & World)
//{
//	my::Vector3 pos, scale; my::Quaternion rot;
//	World.Decompose(scale, rot, pos);
//	m_RigidActor.reset(PhysXContext::getSingleton().m_sdk->createRigidStatic(PxTransform((PxVec3&)pos, (PxQuat&)rot)));
//}
//
//template<>
//void RigidComponent::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
//{
//	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
//	PxDefaultMemoryOutputStream ostr;
//	PhysXPtr<PxCollection> collection(PhysXContext::getSingleton().m_sdk->createCollection());
//	m_RigidActor->collectForExport(*collection);
//	collection->addExternalRef(*PhysXContext::getSingleton().m_PxMaterial, (PxSerialObjectRef)SerializeRefMaterial);
//	collection->setObjectRef(*m_RigidActor, (PxSerialObjectRef)SerializeRefActor);
//	collection->serialize(ostr, false);
//	unsigned int BuffSize = ostr.getSize();
//	ar << BOOST_SERIALIZATION_NVP(BuffSize);
//	ar << boost::serialization::make_nvp("m_RigidActor", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
//}
//
//template<>
//void RigidComponent::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
//{
//	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
//	unsigned int BuffSize;
//	ar >> BOOST_SERIALIZATION_NVP(BuffSize);
//	m_SerializeBuff.reset((unsigned char *)_aligned_malloc(BuffSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
//	ar >> boost::serialization::make_nvp("m_RigidActor", boost::serialization::binary_object(m_SerializeBuff.get(), BuffSize));
//	PhysXPtr<PxUserReferences> externalRefs(PhysXContext::getSingleton().m_sdk->createUserReferences());
//	externalRefs->setObjectRef(*PhysXContext::getSingleton().m_PxMaterial, (PxSerialObjectRef)SerializeRefMaterial);
//	PhysXPtr<PxUserReferences> userRefs(PhysXContext::getSingleton().m_sdk->createUserReferences());
//	PhysXPtr<PxCollection> collection(PhysXContext::getSingleton().m_sdk->createCollection());
//	collection->deserialize(m_SerializeBuff.get(), userRefs.get(), externalRefs.get());
//	m_RigidActor.reset(userRefs->getObjectFromRef((PxSerialObjectRef)SerializeRefActor)->is<PxRigidActor>());
//}
//
//void RigidComponent::RequestResource(void)
//{
//	Component::RequestResource();
//	PhysXSceneContext::getSingleton().m_PxScene->addActor(*m_RigidActor);
//}
//
//void RigidComponent::ReleaseResource(void)
//{
//	PhysXSceneContext::getSingleton().m_PxScene->removeActor(*m_RigidActor);
//	Component::ReleaseResource();
//}

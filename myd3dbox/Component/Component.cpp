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

BOOST_CLASS_EXPORT(EmitterComponent)

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
	ar >> BOOST_SERIALIZATION_NVP(m_Cmps);
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for(; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Parent = this;
	}
}

void Component::RequestResource(void)
{
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

my::Matrix4 Component::CalculateWorld(void) const
{
	return my::Matrix4::Compose(m_Scale, m_Rotation, m_Position);
}

void Component::UpdateWorld(void)
{
	m_World = CalculateWorld();
	if (m_Parent)
	{
		m_World = m_World * m_Parent->m_World;
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateWorld();
	}
}

my::AABB Component::CalculateAABB(void) const
{
	AABB ret = AABB::Invalid();
	ComponentPtrList::const_iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		ret.unionSelf((*cmp_iter)->CalculateAABB().transform((*cmp_iter)->CalculateWorld()));
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

Actor * Component::GetTopParent(void)
{
	if (m_Parent)
	{
		return m_Parent->GetTopParent();
	}
	_ASSERT(m_Type == ComponentTypeActor);
	return dynamic_cast<Actor *>(this);
}

namespace boost { 
	namespace serialization {
		template<class Archive>
		inline void serialize(
			Archive & ar,
			PxClothParticle & t,
			const unsigned int file_version
			){
				ar & BOOST_SERIALIZATION_NVP(t.pos.x);
				ar & BOOST_SERIALIZATION_NVP(t.pos.y);
				ar & BOOST_SERIALIZATION_NVP(t.pos.z);
				ar & BOOST_SERIALIZATION_NVP(t.invWeight);
		}
	}
}

template<>
void MeshComponent::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar << BOOST_SERIALIZATION_NVP(m_MeshRes);
	ar << BOOST_SERIALIZATION_NVP(m_bInstance);
	ar << BOOST_SERIALIZATION_NVP(m_bUseAnimation);
	ar << BOOST_SERIALIZATION_NVP(m_bUseCloth);
	if (m_bUseCloth)
	{
		PxDefaultMemoryOutputStream ostr;
		PhysXPtr<PxCollection> collection(PhysXContext::getSingleton().m_sdk->createCollection());
		m_Cloth->collectForExport(*collection);
		collection->setObjectRef(*m_Cloth, (PxSerialObjectRef)1234);
		collection->serialize(ostr, false);
		unsigned int BuffSize = ostr.getSize();
		ar << BOOST_SERIALIZATION_NVP(BuffSize);
		ar << boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
		ar << BOOST_SERIALIZATION_NVP(m_particles);
	}
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
	ar >> BOOST_SERIALIZATION_NVP(m_bUseCloth);
	if (m_bUseCloth)
	{
		unsigned int BuffSize;
		ar >> BOOST_SERIALIZATION_NVP(BuffSize);
		m_SerializeBuff.reset((unsigned char *)_aligned_malloc(BuffSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
		ar >> boost::serialization::make_nvp("m_Cloth", boost::serialization::binary_object(m_SerializeBuff.get(), BuffSize));
		PhysXPtr<PxUserReferences> userRefs(PhysXContext::getSingleton().m_sdk->createUserReferences());
		PhysXPtr<PxCollection> collection(PhysXContext::getSingleton().m_sdk->createCollection());
		collection->deserialize(m_SerializeBuff.get(), userRefs.get(), NULL);
		m_Cloth.reset(userRefs->getObjectFromRef((PxSerialObjectRef)1234)->is<PxCloth>());
		ar >> BOOST_SERIALIZATION_NVP(m_particles);
	}
	ar >> BOOST_SERIALIZATION_NVP(m_MaterialList);
	ar >> BOOST_SERIALIZATION_NVP(m_StaticCollision);
}

void MeshComponent::RequestResource(void)
{
	Component::RequestResource();

	m_MeshRes.RequestResource();

	if (m_bUseCloth && m_Cloth)
	{
		PhysXSceneContext::getSingleton().m_PxScene->addActor(*m_Cloth);
	}

	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->RequestResource();
	}
}

void MeshComponent::ReleaseResource(void)
{
	m_MeshRes.ReleaseResource();

	if (m_bUseCloth && m_Cloth)
	{
		PhysXSceneContext::getSingleton().m_PxScene->removeActor(*m_Cloth);
	}

	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->ReleaseResource();
	}

	Component::ReleaseResource();
}

void MeshComponent::Update(float fElapsedTime)
{
	if (m_bUseCloth && m_Cloth)
	{
		_ASSERT(m_particles.size() == m_MeshRes.m_Res->GetNumVertices());
		PxClothReadData * readData = m_Cloth->lockClothReadData();
		if (readData)
		{
			unsigned char * pVertices = (unsigned char *)m_MeshRes.m_Res->LockVertexBuffer(0);
			const DWORD NbParticles = m_Cloth->getNbParticles();
			m_NewParticles.resize(NbParticles);
			for (unsigned int i = 0; i < NbParticles; i++)
			{
				void * pVertex = pVertices + i * m_MeshRes.m_Res->GetNumBytesPerVertex();
				m_NewParticles[i].invWeight = readData->particles[i].invWeight;
				if (0 == m_NewParticles[i].invWeight)
				{
					if (m_bUseAnimation && m_Parent && m_Parent->m_Animator && !m_Parent->m_Animator->m_DualQuats.empty())
					{
						m_NewParticles[i].pos = (PxVec3 &)m_Parent->m_Animator->m_DualQuats.TransformVertexWithDualQuaternionList(
							(my::Vector3 &)m_particles[i].pos,
							m_MeshRes.m_Res->m_VertexElems.GetBlendIndices(pVertex),
							m_MeshRes.m_Res->m_VertexElems.GetBlendWeight(pVertex));
					}
					else
					{
						m_NewParticles[i].pos = m_particles[i].pos;
					}
				}
				else
				{
					m_NewParticles[i].pos = readData->particles[i].pos;
				}
				m_MeshRes.m_Res->m_VertexElems.SetPosition(pVertex, (my::Vector3 &)m_NewParticles[i].pos);
			}
			readData->unlock();
			m_Cloth->setParticles(&m_NewParticles[0], NULL);
			m_Cloth->setTargetPose(PxTransform((PxMat44 &)m_World));

			void * pIndices = m_MeshRes.m_Res->LockIndexBuffer(0);
			my::OgreMesh::ComputeNormalFrame(pVertices, NbParticles,
				m_MeshRes.m_Res->GetNumBytesPerVertex(), pIndices, true, m_MeshRes.m_Res->GetNumFaces(), m_MeshRes.m_Res->m_VertexElems);
			my::OgreMesh::ComputeTangentFrame(pVertices, NbParticles,
				m_MeshRes.m_Res->GetNumBytesPerVertex(), pIndices, true, m_MeshRes.m_Res->GetNumFaces(), m_MeshRes.m_Res->m_VertexElems);
			m_MeshRes.m_Res->UnlockIndexBuffer();
			m_MeshRes.m_Res->UnlockVertexBuffer();
		}
	}

	Component::Update(fElapsedTime);
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(AttribId < m_MaterialList.size());

	shader->SetFloat("g_Time", (float)D3DContext::getSingleton().m_fAbsoluteTime);

	shader->SetMatrix("g_World", m_World);

	if (!m_bUseCloth && m_bUseAnimation && m_Parent && m_Parent->m_Animator)
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
							(!m_bUseCloth && m_bUseAnimation) ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, m_bInstance, m_MaterialList[i].get(), PassID);
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

void SphericalEmitterComponent::RequestResource(void)
{
	Component::RequestResource();

	if (m_Material)
	{
		m_Material->RequestResource();
	}
}

void SphericalEmitterComponent::ReleaseResource(void)
{
	if (m_Material)
	{
		m_Material->ReleaseResource();
	}

	Component::ReleaseResource();
}

void SphericalEmitterComponent::Update(float fElapsedTime)
{
	m_Emitter->Update(fElapsedTime);

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

	Component::Update(fElapsedTime);
}

void SphericalEmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);

	shader->SetFloat("g_Time", m_Emitter->m_Time);

	shader->SetMatrix("g_World", m_World);

	if (m_Material)
	{
		m_Material->OnSetShader(shader, AttribId);
	}
}

my::AABB SphericalEmitterComponent::CalculateAABB(void) const
{
	AABB ret = RenderComponent::CalculateAABB();
	ret.unionSelf(my::AABB(-1,1));
	return ret;
}

void SphericalEmitterComponent::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
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

#include "StdAfx.h"
#include "Component.h"
#include "Terrain.h"
#include "Animator.h"
//#include "PhysXContext.h"
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
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Position);
	ar << BOOST_SERIALIZATION_NVP(m_Rotation);
	ar << BOOST_SERIALIZATION_NVP(m_Scale);
	ar << BOOST_SERIALIZATION_NVP(m_Animator);
	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
}

template<>
void Component::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar >> BOOST_SERIALIZATION_NVP(m_Type);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Position);
	ar >> BOOST_SERIALIZATION_NVP(m_Rotation);
	ar >> BOOST_SERIALIZATION_NVP(m_Scale);
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
	m_World = my::Matrix4::Compose(m_Scale, m_Rotation, m_Position);
	if (m_Parent)
	{
		m_World = m_World * m_Parent->m_World;
	}

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

Component * Component::GetTopParent(void)
{
	if (m_Parent)
	{
		return m_Parent->GetTopParent();
	}
	return this;
}

Animator * Component::GetAnimator(void)
{
	if (m_Animator)
	{
		return m_Animator.get();
	}
	if (m_Parent)
	{
		return m_Parent->GetAnimator();
	}
	return NULL;
}
//
//const my::AABB & Component::GetCmpOctAABB(const Component * cmp)
//{
//	//if (cmp->m_OctNode)
//	//{
//	//	return cmp->m_OctNode->m_aabb;
//	//}
//	return cmp->m_aabb;
//}
//
//my::Matrix4 Component::GetCmpWorld(const Component * cmp)
//{
//	//switch (cmp->m_Type)
//	//{
//	//case ComponentTypeRigid:
//	//	{
//	//		PxTransform pose = dynamic_cast<const RigidComponent *>(cmp)->m_RigidActor->getGlobalPose();
//	//		return Matrix4::Compose(Vector3(1,1,1), (Quaternion&)pose.q, (Vector3&)pose.p);
//	//	}
//	//}
//	return cmp->m_World;
//}
//
//void Component::SetCmpWorld(Component * cmp, const my::Matrix4 & World)
//{
//	cmp->m_World = World;
//	//switch (cmp->m_Type)
//	//{
//	//case ComponentTypeRigid:
//	//	{
//	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
//	//		Vector3 scale, pos; Quaternion rot;
//	//		World.Decompose(scale, rot, pos);
//	//		rigid_cmp->m_RigidActor->setGlobalPose(PxTransform((PxVec3&)pos, (PxQuat&)rot));
//	//	}
//	//	break;
//	//}
//}

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

	if (m_bAnimation)
	{
		Animator * anim = GetAnimator();
		if (anim && !anim->m_DualQuats.empty())
		{
			shader->SetMatrixArray("g_dualquat", &anim->m_DualQuats[0], anim->m_DualQuats.size());
		}
	}

	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
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
						my::Effect * shader = pipeline->QueryShader(m_bAnimation ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, m_bInstance, m_MaterialList[i].get(), PassID);
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

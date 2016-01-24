#include "StdAfx.h"
#include "Component.h"
#include "Animator.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Material)

BOOST_CLASS_EXPORT(Component)

BOOST_CLASS_EXPORT(RenderComponent)

BOOST_CLASS_EXPORT(MeshComponent)

BOOST_CLASS_EXPORT(EmitterComponent)

Material::Material(void)
	: m_PassMask(RenderPipeline::PassMaskNone)
{
}

Material::~Material(void)
{
}

void Material::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	if (!m_MeshTexture.m_Path.empty())
	{
		shader->SetTexture("g_MeshTexture", m_MeshTexture.m_Res.get());
	}
	if (!m_NormalTexture.m_Path.empty())
	{
		shader->SetTexture("g_NormalTexture", m_NormalTexture.m_Res.get());
	}
	if (!m_SpecularTexture.m_Path.empty())
	{
		shader->SetTexture("g_SpecularTexture", m_SpecularTexture.m_Res.get());
	}
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

const my::AABB & Component::GetOctAABB(void) const
{
	if (m_OctNode)
	{
		return m_OctNode->m_aabb;
	}
	return m_aabb;
}

const my::AABB & Component::GetComponentAABB(void) const
{
	if (m_OctNode)
	{
		OctNodeBase::OctComponentSet::const_iterator cmp_iter = m_OctNode->m_Components.find(const_cast<Component *>(this));
		if (cmp_iter != m_OctNode->m_Components.end())
		{
			return cmp_iter->second;
		}
	}
	return m_aabb;
}

void MeshComponent::RequestResource(void)
{
	RenderComponent::RequestResource();

	m_MeshRes.RequestResource();
	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->RequestResource();
	}

	if (m_Animator)
	{
		m_Animator->RequestResource();
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

	if (m_Animator)
	{
		m_Animator->ReleaseResource();
	}

	RenderComponent::ReleaseResource();
}

void MeshComponent::Update(float fElapsedTime)
{
	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(AttribId < m_MaterialList.size());

	shader->SetMatrix("g_World", m_World);

	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray("g_dualquat", &m_Animator->m_DualQuats[0], m_Animator->m_DualQuats.size());
	}

	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void MeshComponent::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
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
						my::Effect * shader = pipeline->QueryShader(m_Animator ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, m_bInstance, m_MaterialList[i].get(), PassID);
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
}

void EmitterComponent::RequestResource(void)
{
	RenderComponent::RequestResource();

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

	RenderComponent::ReleaseResource();
}

void EmitterComponent::Update(float fElapsedTime)
{
	if (m_Emitter)
	{
		m_Emitter->Update(fElapsedTime);
	}
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);

	shader->SetMatrix("g_World", m_World);

	if (m_Material)
	{
		m_Material->OnSetShader(shader, AttribId);
	}
}

void EmitterComponent::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
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
}

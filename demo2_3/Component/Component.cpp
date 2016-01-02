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

BOOST_CLASS_EXPORT(Material::ParameterValue)

BOOST_CLASS_EXPORT(Material::ParameterValueTexture)

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

void Material::ParameterValueTexture::OnSetShader(my::Effect * shader, DWORD AttribId, const char * name)
{
	shader->SetTexture(name, m_Res.get());
}

void Material::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	ParameterList::iterator param_iter = m_Params.begin();
	for (; param_iter != m_Params.end(); param_iter++)
	{
		param_iter->second->OnSetShader(shader, AttribId, param_iter->first.c_str());
	}
}

void Material::RequestResource(void)
{
	ParameterList::iterator param_iter = m_Params.begin();
	for (; param_iter != m_Params.end(); param_iter++)
	{
		switch(param_iter->second->m_Type)
		{
		case ParameterValue::ParameterValueTypeTexture:
			boost::dynamic_pointer_cast<ParameterValueTexture>(param_iter->second)->RequestResource();
		}
	}
}

void Material::ReleaseResource(void)
{
	ParameterList::iterator param_iter = m_Params.begin();
	for (; param_iter != m_Params.end(); param_iter++)
	{
		switch(param_iter->second->m_Type)
		{
		case ParameterValue::ParameterValueTypeTexture:
			boost::dynamic_pointer_cast<ParameterValueTexture>(param_iter->second)->ReleaseResource();
		}
	}
}

MeshComponent::LOD & MeshComponent::GetLod(unsigned int Id)
{
	if (m_Lods.size() < (Id + 1))
	{
		m_Lods.resize(Id + 1);
	}
	return m_Lods[Id];
}

void MeshComponent::RequestResource(void)
{
	RenderComponent::RequestResource();

	LODList::iterator lod_iter = m_Lods.begin();
	for (; lod_iter != m_Lods.end(); lod_iter++)
	{
		lod_iter->m_MeshRes.RequestResource();
		MaterialPtrList::iterator mat_iter = lod_iter->m_MaterialList.begin();
		for (; mat_iter != lod_iter->m_MaterialList.end(); mat_iter++)
		{
			(*mat_iter)->RequestResource();
		}
	}

	if (m_Animator)
	{
		m_Animator->RequestResource();
	}
}

void MeshComponent::ReleaseResource(void)
{
	LODList::iterator lod_iter = m_Lods.begin();
	for (; lod_iter != m_Lods.end(); lod_iter++)
	{
		lod_iter->m_MeshRes.ReleaseResource();
		MaterialPtrList::iterator mat_iter = lod_iter->m_MaterialList.begin();
		for (; mat_iter != lod_iter->m_MaterialList.end(); mat_iter++)
		{
			(*mat_iter)->ReleaseResource();
		}
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
	_ASSERT(m_LodId < m_Lods.size());

	_ASSERT(AttribId < m_Lods[m_LodId].m_MaterialList.size());

	shader->SetMatrix("g_World", m_World);

	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray("g_dualquat", &m_Animator->m_DualQuats[0], m_Animator->m_DualQuats.size());
	}

	m_Lods[m_LodId].m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void MeshComponent::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_LodId < m_Lods.size())
	{
		LOD & lod = m_Lods[m_LodId];

		if (lod.m_MeshRes.m_Res)
		{
			for (DWORD i = 0; i < lod.m_MaterialList.size(); i++)
			{
				if (lod.m_MaterialList[i] && (lod.m_MaterialList[i]->m_PassMask & PassMask))
				{
					for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
					{
						if (RenderPipeline::PassTypeToMask(PassID) & (lod.m_MaterialList[i]->m_PassMask & PassMask))
						{
							my::Effect * shader = pipeline->QueryShader(m_Animator ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, m_bInstance, lod.m_MaterialList[i].get(), PassID);
							if (shader)
							{
								if (m_bInstance)
								{
									pipeline->PushMeshInstance(PassID, lod.m_MeshRes.m_Res.get(), i, m_World, shader, this);
								}
								else
								{
									pipeline->PushMesh(PassID, lod.m_MeshRes.m_Res.get(), i, shader, this);
								}
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

	m_Material->OnSetShader(shader, AttribId);
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

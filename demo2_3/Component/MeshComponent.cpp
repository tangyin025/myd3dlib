#include "StdAfx.h"
#include "MeshComponent.h"
#include "../Game.h"

using namespace my;

void MeshComponent::OnMaterialLoaded(DWORD i, my::DeviceRelatedObjectBasePtr res)
{
	_ASSERT(i < m_Materials.size());
	m_Materials[i].first = boost::dynamic_pointer_cast<Material>(res);
}

void MeshComponent::OnEffectLoaded(DWORD i, my::DeviceRelatedObjectBasePtr res)
{
	_ASSERT(i < m_Materials.size());
	m_Materials[i].second = boost::dynamic_pointer_cast<Effect>(res);
}

void MeshComponent::UpdateLod(float dist)
{
	if (m_Materials.size() < m_Mesh->m_MaterialNameList.size())
	{
		m_Materials.resize(m_Mesh->m_MaterialNameList.size(), MaterialPair(MaterialPtr(), EffectPtr()));
		for (DWORD i = 0; i < m_Mesh->m_MaterialNameList.size(); i++)
		{
			Game::getSingleton().LoadMaterialAsync(str_printf("material/%s.txt", m_Mesh->m_MaterialNameList[i].c_str()), boost::bind(&MeshComponent::OnMaterialLoaded, this, i, _1));
			Game::getSingleton().LoadEffectAsync("shader/SimpleSample.fx", EffectMacroPairList(), boost::bind(&MeshComponent::OnEffectLoaded, this, i, _1));
		}
	}
}

void MeshComponent::Draw(void)
{
	for(DWORD i = 0; i < m_Materials.size(); i++)
	{
		MaterialPairList::reference mat_pair = m_Materials[i];
		if (mat_pair.first && mat_pair.second)
		{
			mat_pair.second->SetMatrix("g_World", m_World);
			mat_pair.second->SetTexture("g_MeshTexture", mat_pair.first->m_DiffuseTexture);
			mat_pair.second->SetTechnique("RenderScene");
			UINT passes = mat_pair.second->Begin();
			for(UINT p = 0; p < passes; p++)
			{
				mat_pair.second->BeginPass(p);
				m_Mesh->DrawSubset(i);
				mat_pair.second->EndPass();
			}
			mat_pair.second->End();
		}
	}
}

void SkeletonMeshComponent::Draw(void)
{
	for(DWORD i = 0; i < m_Materials.size(); i++)
	{
		MaterialPairList::reference mat_pair = m_Materials[i];
		if (mat_pair.first && mat_pair.second)
		{
			mat_pair.second->SetMatrix("g_World", m_World);
			mat_pair.second->SetTexture("g_MeshTexture", mat_pair.first->m_DiffuseTexture);
			mat_pair.second->SetMatrixArray("g_dualquat", &m_DualQuats[0], m_DualQuats.size());
			mat_pair.second->SetTechnique("RenderScene");
			UINT passes = mat_pair.second->Begin();
			for(UINT p = 0; p < passes; p++)
			{
				mat_pair.second->BeginPass(p);
				m_Mesh->DrawSubset(i);
				mat_pair.second->EndPass();
			}
			mat_pair.second->End();
		}
	}
}

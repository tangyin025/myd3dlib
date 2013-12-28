#include "StdAfx.h"
#include "MeshComponent.h"

using namespace my;

void MeshComponent::Draw(DrawState State, const my::Matrix4 & ParentWorld)
{
	Matrix4 World = m_World * ParentWorld;
	for(DWORD i = 0; i < m_Materials.size(); i++)
	{
		MaterialPairList::reference mat_pair = m_Materials[i];
		_ASSERT(mat_pair.second);
		mat_pair.second->SetMatrix("g_World", World);
		mat_pair.second->SetTexture("g_MeshTexture", mat_pair.first->m_DiffuseTexture);
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

void SkeletonMeshComponent::Draw(DrawState State, const my::Matrix4 & ParentWorld)
{
	Matrix4 World = m_World * ParentWorld;
	for(DWORD i = 0; i < m_Materials.size(); i++)
	{
		MaterialPairList::reference mat_pair = m_Materials[i];
		_ASSERT(mat_pair.second);
		mat_pair.second->SetMatrix("g_World", World);
		mat_pair.second->SetTexture("g_MeshTexture", mat_pair.first->m_DiffuseTexture);
		mat_pair.second->SetMatrixArray("g_dualquat", &m_DualQuats[0], m_DualQuats.size());
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

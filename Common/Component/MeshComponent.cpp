#include "StdAfx.h"
#include "MeshComponent.h"

using namespace my;

void StaticMeshComponent::OnPostRender(my::Effect * shader, DrawStage draw_stage, DWORD AttriId)
{
	switch (draw_stage)
	{
	case DrawStageCBuffer:
		shader->SetMatrix("g_World", m_World);
		shader->SetTexture("g_MeshTexture", m_Materials[AttriId]->m_DiffuseTexture);
		shader->SetTechnique("RenderScene");
		break;
	}
}

void SkeletonMeshComponent::OnPostRender(my::Effect * shader, DrawStage draw_stage, DWORD AttriId)
{
	switch (draw_stage)
	{
	case DrawStageCBuffer:
		shader->SetMatrix("g_World", m_World);
		shader->SetTexture("g_MeshTexture", m_Materials[AttriId]->m_DiffuseTexture);
		shader->SetMatrixArray("g_dualquat", &m_DualQuats[0], m_DualQuats.size());
		shader->SetTechnique("RenderScene");
		break;
	}
}

#include "StdAfx.h"
#include "MeshComponent.h"
#include "../Game.h"

using namespace my;

void MeshLOD::OnPreRender(my::Effect * shader, DWORD draw_stage, DWORD AttriId)
{
	switch (draw_stage)
	{
	case Game::DrawStageCBuffer:
		shader->SetTexture("g_MeshTexture", m_Materials[AttriId]->m_DiffuseTexture);
		shader->SetTechnique("RenderScene");
		break;
	}
}

void StaticMeshComponent::OnPreRender(my::Effect * shader, DWORD draw_stage)
{
	switch (draw_stage)
	{
	case Game::DrawStageCBuffer:
		shader->SetMatrix("g_World", m_World);
		break;
	}
}

void SkeletonMeshComponent::OnPreRender(my::Effect * shader, DWORD draw_stage)
{
	switch (draw_stage)
	{
	case Game::DrawStageCBuffer:
		shader->SetMatrix("g_World", m_World);
		shader->SetMatrixArray("g_dualquat", &m_DualQuats[0], m_DualQuats.size());
		break;
	}
}

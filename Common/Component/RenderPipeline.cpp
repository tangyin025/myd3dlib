#include "StdAfx.h"
#include "RenderPipeline.h"

#define SHADOW_MAP_SIZE 512

using namespace my;

RenderPipeline::RenderPipeline(void)
{
}

RenderPipeline::~RenderPipeline(void)
{
	OnDestroyDevice();
}

HRESULT RenderPipeline::OnCreateDevice(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

HRESULT RenderPipeline::OnResetDevice(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_ShadowMap.CreateAdjustedTexture(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! 所有的 render target必须使用具有相同 multisample的 depth stencil
	//DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
	m_ShadowMapDS.CreateDepthStencilSurface(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	m_GBufferN.CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	m_GBufferL.CreateTexture(
		pd3dDevice, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT);

	return S_OK;
}

void RenderPipeline::OnLostDevice(void)
{
	m_ShadowMap.OnDestroyDevice();

	m_ShadowMapDS.OnDestroyDevice();

	m_GBufferN.OnDestroyDevice();

	m_GBufferL.OnDestroyDevice();
}

void RenderPipeline::OnDestroyDevice(void)
{
}

void RenderPipeline::Draw(MeshComponent * mesh_cmp)
{
	for (DWORD i = 0; i < mesh_cmp->m_Materials.size(); i++)
	{
		_ASSERT(mesh_cmp->m_Mesh);
		if (mesh_cmp->m_Materials[i])
		{
			my::EffectPtr shader = QueryShader(mesh_cmp->MESH_TYPE, MeshComponent::DrawStageCBuffer, mesh_cmp->m_Materials[i].get());
			if (shader)
			{
				mesh_cmp->OnPostRender(shader.get(), MeshComponent::DrawStageCBuffer, i);

				UINT passes = shader->Begin();
				for (UINT p = 0; p < passes; p++)
				{
					shader->BeginPass(p);
					mesh_cmp->m_Mesh->DrawSubset(i);
					shader->EndPass();
				}
				shader->End();
			}
		}
	}
}

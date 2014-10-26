#include "StdAfx.h"
#include "RenderPipeline.h"
#include "MeshComponent.h"

using namespace my;

RenderPipeline::RenderPipeline(void)
{
}

RenderPipeline::~RenderPipeline(void)
{
	OnDestroy();
}

HRESULT RenderPipeline::OnCreate(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

HRESULT RenderPipeline::OnReset(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
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

void RenderPipeline::OnLost(void)
{
	m_ShadowMap.OnDestroyDevice();

	m_ShadowMapDS.OnDestroyDevice();

	m_GBufferN.OnDestroyDevice();

	m_GBufferL.OnDestroyDevice();
}

void RenderPipeline::OnDestroy(void)
{
}

#pragma once

#include "MeshComponent.h"

class RenderPipeline
{
public:
	my::Texture2D m_ShadowMap;

	my::Surface m_ShadowMapDS;

	my::Texture2D m_GBufferN;

	my::Texture2D m_GBufferL;

public:
	RenderPipeline(void);

	virtual ~RenderPipeline(void);

	HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	virtual my::EffectPtr QueryShader(MeshComponent::MeshType mesh_type, MeshComponent::DrawStage draw_stage, const my::Material * material) = 0;

	void Draw(MeshComponent * mesh_cmp);
};

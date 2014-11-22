#pragma once

#include "MeshComponent.h"

class RenderPipeline
{
public:
	my::ResourceMgr * m_ResMgr;

	typedef boost::tuple<MeshComponent::MeshType, MeshComponent::DrawStage, const my::Material *> ShaderKeyType;

	typedef boost::unordered_map<ShaderKeyType, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	my::EffectPtr m_SimpleSample;

	my::Texture2D m_ShadowMap;

	my::Surface m_ShadowMapDS;

	my::Texture2D m_GBufferN;

	my::Texture2D m_GBufferL;

public:
	RenderPipeline(void);

	virtual ~RenderPipeline(void);

	HRESULT OnCreateDevice(my::ResourceMgr * res_mgr, IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	HRESULT OnResetDevice(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	void OnShaderLoaded(my::DeviceRelatedObjectBasePtr res, ShaderKeyType key);

	my::EffectPtr QueryShader(MeshComponent::MeshType mesh_type, MeshComponent::DrawStage draw_stage, const my::Material * material);

	void Draw(MeshComponent * mesh_cmp);
};

#pragma once

#include "MeshComponent.h"

class RenderPipeline
{
public:
	enum DrawStage
	{
		DrawStageShadow,
		DrawStageNBuffer,
		DrawStageDBuffer,
		DrawStageCBuffer,
	};

	my::ResourceMgr * m_ResMgr;

	typedef boost::tuple<MeshComponent::MeshType, DrawStage, const my::Material *> ShaderKeyType;

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

	typedef std::pair<int, const int> Counter;

	typedef boost::shared_ptr<Counter> CounterPtr;

	void OnMeshLodMaterialLoaded(my::DeviceRelatedObjectBasePtr res, boost::weak_ptr<MeshLOD> weak_mesh_lod, unsigned int i, CounterPtr counter);

	void OnMeshLodMeshLoaded(my::DeviceRelatedObjectBasePtr res, boost::weak_ptr<MeshLOD> weak_mesh_lod);

	void LoadMeshLodAsync(MeshLODPtr mesh_lod, const std::string & mesh_path);

	void OnShaderLoaded(my::DeviceRelatedObjectBasePtr res, ShaderKeyType key);

	my::EffectPtr QueryShader(MeshComponent::MeshType mesh_type, DrawStage draw_stage, const my::Material * material);

	void DrawMesh(MeshComponent * mesh_cmp, DWORD lod);

	void DrawMeshLOD(MeshComponent::MeshType mesh_type, MeshLOD * mesh_lod);
};

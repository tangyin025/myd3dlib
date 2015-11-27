#pragma once

#include <myResource.h>
#include "ActorComponent.h"
#include "Animator.h"
#include "FModContext.h"

class ComponentLevel;

class ActorResourceMgr
	: public my::ResourceMgr
{
public:
	typedef boost::tuple<RenderPipeline::MeshType, bool, std::string> ShaderCacheKey;

	typedef boost::unordered_map<ShaderCacheKey, my::EffectPtr> ShaderCacheMap;

	ShaderCacheMap m_ShaderCache;

	void OnMaterialParameterValueTextureLoaded(
		boost::weak_ptr<Material::ParameterValueTexture> weak_value_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnMeshComponentMaterialLoaded(
		boost::weak_ptr<MeshComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId,
		bool bInstance);

	void OnMeshComponentMeshLoaded(
		boost::weak_ptr<MeshComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		bool bInstance);

	void OnEmitterComponentEmitterLoaded(
		boost::weak_ptr<EmitterComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnEmitterComponentMaterialLoaded(
		boost::weak_ptr<EmitterComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnAnimatorSkeletonLoaded(
		boost::weak_ptr<Animator> weak_ani_ptr,
		my::DeviceRelatedObjectBasePtr res);

public:
	ActorResourceMgr(void)
	{
	}

	virtual ~ActorResourceMgr(void)
	{
	}

	HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

	void OnLostDevice(void);

	void OnDestroyDevice(void);

	void ClearAllShaders(void);

	boost::shared_ptr<my::Emitter> CreateEmitter(const std::string & path);

	void SaveEmitter(const std::string & path, boost::shared_ptr<my::Emitter> emitter);

	boost::shared_ptr<Material> CreateMaterial(const std::string & path);

	void SaveMaterial(const std::string & path, boost::shared_ptr<Material> material);
};

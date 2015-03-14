#pragma once

#include <myResource.h>
#include "RenderComponent.h"

class ResourceMgrEx
	: public my::ResourceMgr
{
protected:
	void OnMaterialDiffuseTextureLoaded(
		boost::weak_ptr<Material> weak_mat_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnMeshComponentLODMaterialLoaded(
		boost::weak_ptr<MeshComponent::MeshLOD> weak_lod_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId,
		bool bInstance);

	void OnMeshComponentLODMeshLoaded(
		boost::weak_ptr<MeshComponent::MeshLOD> weak_lod_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId,
		bool bInstance);

	void OnEmitterComponentEmitterLoaded(
		boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId);

	void OnEmitterComponentMaterialLoaded(
		boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId);

public:
	ResourceMgrEx(void)
	{
	}

	virtual ~ResourceMgrEx(void)
	{
	}

	void LoadMaterialAsync(const std::string & path, const my::ResourceCallback & callback);

	boost::shared_ptr<Material> LoadMaterial(const std::string & path);

	void SaveMaterial(const std::string & path, boost::shared_ptr<Material> material);

	MeshComponentPtr CreateMeshComponentFromFile(const std::string & path);

	EmitterMeshComponentPtr CreateEmitterComponentFromFile(const std::string & path);
};

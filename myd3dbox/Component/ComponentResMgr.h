#pragma once

#include <myResource.h>
#include "ActorComponent.h"
#include "FModContext.h"

class ComponentResMgr
	: public my::ResourceMgr
	, public my::SingleInstance<ComponentResMgr>
{
protected:
	void OnMaterialDiffuseTextureLoaded(
		boost::weak_ptr<Material> weak_mat_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnMeshComponentMaterialLoaded(
		boost::weak_ptr<MeshComponent> weak_lod_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId,
		bool bInstance);

	void OnMeshComponentMeshLoaded(
		boost::weak_ptr<MeshComponent> weak_lod_ptr,
		my::DeviceRelatedObjectBasePtr res,
		bool bInstance);

	void OnEmitterComponentEmitterLoaded(
		boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnEmitterComponentMaterialLoaded(
		boost::weak_ptr<EmitterMeshComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res);

	void OnClothComponentMaterialLoaded(
		boost::weak_ptr<ClothComponent> weak_lod_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId);

	void OnClothComponentMeshLoaded(
		boost::weak_ptr<ClothComponent> weak_lod_ptr,
		my::DeviceRelatedObjectBasePtr res,
		boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
		boost::shared_ptr<my::BoneHierarchy> hierarchy,
		DWORD root_i,
		boost::shared_ptr<PxClothCollisionData> collData);

public:
	ComponentResMgr(void)
	{
	}

	virtual ~ComponentResMgr(void)
	{
	}

	static void CookTriangleMesh(PxCooking * Cooking, my::OStreamPtr ostream, my::MeshPtr mesh);

	static void CookTriangleMeshToFile(PxCooking * Cooking, std::string path, my::MeshPtr mesh);

	static PxTriangleMesh * CreateTriangleMesh(PxPhysics * sdk, my::IStreamPtr istream);

	static void CookClothFabric(PxCooking * Cooking, const my::Vector3 & Gravity, my::OStreamPtr ostream, my::MeshPtr mesh, WORD PositionOffset);

	static void CookClothFabricToFile(PxCooking * Cooking, const my::Vector3 & Gravity, std::string path, my::MeshPtr mesh, WORD PositionOffset);

	static PxClothFabric * CreateClothFabric(PxPhysics * sdk, my::IStreamPtr istream);

	void LoadMaterialAsync(const std::string & path, const my::ResourceCallback & callback);

	boost::shared_ptr<Material> LoadMaterial(const std::string & path);

	void SaveMaterial(const std::string & path, boost::shared_ptr<Material> material);

	MeshComponentPtr CreateMeshComponentFromFile(const std::string & path, bool bInstance);

	EmitterMeshComponentPtr CreateEmitterComponentFromFile(const std::string & path);

	ClothComponentPtr CreateClothComponentFromFile(
		boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
		const std::string & path,
		const my::BoneHierarchy & hierarchy,
		DWORD root_i,
		const PxClothCollisionData& collData);

	FMOD::Sound * CreateFModSound(FMOD::System * system, const std::string & path, FMOD_MODE mode);

	FMOD::Sound * CreateFModStream(FMOD::System * system, const std::string & path, FMOD_MODE mode);
};

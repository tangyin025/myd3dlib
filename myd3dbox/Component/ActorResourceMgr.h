#pragma once

#include <myResource.h>
#include "ActorComponent.h"
#include "FModContext.h"

class ActorResourceMgr
	: public my::ResourceMgr
{
protected:
	void OnMaterialDiffuseTextureLoaded(
		boost::weak_ptr<Material> weak_mat_ptr,
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

	void OnClothComponentMaterialLoaded(
		boost::weak_ptr<ClothComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		DWORD AttribId);

	void OnClothComponentMeshLoaded(
		boost::weak_ptr<ClothComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
		boost::shared_ptr<my::BoneHierarchy> hierarchy,
		DWORD root_i,
		boost::shared_ptr<PxClothCollisionData> collData);

	void OnClothComponentSkeletonLoaded(
		boost::weak_ptr<ClothComponent> weak_cmp_ptr,
		my::DeviceRelatedObjectBasePtr res,
		boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
		std::string mesh_path,
		std::string root_name,
		boost::shared_ptr<PxClothCollisionData> collData);

public:
	ActorResourceMgr(void)
	{
	}

	virtual ~ActorResourceMgr(void)
	{
	}

	static void CookTriangleMesh(PxCooking * Cooking, my::OStreamPtr ostream, my::MeshPtr mesh);

	static void CookTriangleMeshToFile(PxCooking * Cooking, std::string path, my::MeshPtr mesh);

	static PxTriangleMesh * CreateTriangleMesh(PxPhysics * sdk, my::IStreamPtr istream);

	static void CookClothFabric(PxCooking * Cooking, const my::Vector3 & Gravity, my::OStreamPtr ostream, my::MeshPtr mesh, WORD PositionOffset);

	static void CookClothFabricToFile(PxCooking * Cooking, const my::Vector3 & Gravity, std::string path, my::MeshPtr mesh, WORD PositionOffset);

	static PxClothFabric * CreateClothFabric(PxPhysics * sdk, my::IStreamPtr istream);

	boost::shared_ptr<my::Emitter> CreateEmitter(const std::string & path);

	void SaveEmitter(const std::string & path, boost::shared_ptr<my::Emitter> emitter);

	boost::shared_ptr<Material> CreateMaterial(const std::string & path);

	void SaveMaterial(const std::string & path, boost::shared_ptr<Material> material);

	MeshComponentPtr AddMeshComponent(Actor * owner, boost::shared_ptr<my::Mesh> mesh, bool bInstance);

	MeshComponentPtr AddMeshComponentFromFile(Actor * owner, const std::string & path, bool bInstance);

	SkeletonMeshComponentPtr AddSkeletonMeshComponent(Actor * owner, boost::shared_ptr<my::Mesh> mesh, bool bInstance);

	SkeletonMeshComponentPtr AddSkeletonMeshComponentFromFile(Actor * owner, const std::string & path, bool bInstance);

	EmitterComponentPtr AddEmitterComponentFromFile(Actor * owner, const std::string & path);

	ClothComponentPtr AddClothComponentFromFile(
		Actor * owner,
		boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
		const std::string & mesh_path,
		const std::string & skel_path,
		const std::string & root_name,
		const PxClothCollisionData& collData);
};

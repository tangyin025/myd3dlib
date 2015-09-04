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

	MeshComponent * CreateMeshComponent(ComponentLevel * owner, boost::shared_ptr<my::Mesh> mesh, const my::AABB & aabb, const my::Matrix4 & World, bool bInstance);

	MeshComponent * CreateMeshComponentFromFile(ComponentLevel * owner, const std::string & path, const my::AABB & aabb, const my::Matrix4 & World, bool bInstance);

	void CreateMeshComponentList(ComponentLevel * owner, boost::shared_ptr<my::OgreMeshSet> mesh_set);

	//EmitterComponentPtr AddEmitterComponent(Actor * owner, boost::shared_ptr<my::Emitter> emitter);

	//EmitterComponentPtr AddEmitterComponentFromFile(Actor * owner, const std::string & path);

	//ClothComponentPtr AddClothComponentFromFile(
	//	Actor * owner,
	//	boost::tuple<PxCooking *, PxPhysics *, PxScene *> PxContext,
	//	const std::string & mesh_path,
	//	const std::string & skel_path,
	//	const std::string & root_name,
	//	const PxClothCollisionData& collData);

	//AnimatorPtr AddAnimatorFromFile(Actor * owner, const std::string & path);
};

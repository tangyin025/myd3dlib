#pragma once

#include "myOctree.h"
#include "myEmitter.h"
#include "myMesh.h"
#include "PhysXPtr.h"
#include <atlbase.h>
#include <boost/serialization/nvp.hpp>

namespace my
{
	class Effect;
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;

class Actor;

class PhysXSceneContext;

class RenderPipeline;

class Component;

typedef boost::shared_ptr<Component> ComponentPtr;

class Component
{
public:
	enum ComponentType
	{
		ComponentTypeComponent,
		ComponentTypeActor,
		ComponentTypeCharacter,
		ComponentTypeMesh,
		ComponentTypeCloth,
		ComponentTypeEmitter,
		ComponentTypeStaticEmitter,
		ComponentTypeSphericalEmitter,
		ComponentTypeTerrain,
		//ComponentTypeRigid,
	};

	ComponentType m_Type;

	Actor * m_Actor;

	PhysXPtr<physx::PxMaterial> m_PxMaterial;

	PhysXPtr<physx::PxShape> m_PxShape;

protected:
	Component(void)
		: m_Type(ComponentTypeComponent)
		, m_Actor(NULL)
	{
	}

	Component(ComponentType Type)
		: m_Type(Type)
		, m_Actor(NULL)
	{
	}

public:
	virtual ~Component(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void CopyFrom(const Component & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(PhysXSceneContext * scene);

	virtual void OnLeavePxScene(PhysXSceneContext * scene);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual void OnWorldUpdated(void);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz);

	void CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight);

	void CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot);

	void CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius);

	virtual void ClearShape(void);
};

class MeshComponent
	: public Component
	, public my::IResourceCallback
{
public:
	std::string m_MeshPath;

	my::OgreMeshPtr m_Mesh;

	my::ControlEvent m_MeshEventReady;

	bool m_bInstance;

	bool m_bUseAnimation;

	bool m_bNavigation;

	MaterialPtrList m_MaterialList;

public:
	MeshComponent(void)
		: Component(ComponentTypeMesh)
		, m_bInstance(false)
		, m_bUseAnimation(false)
		, m_bNavigation(false)
	{
	}

	virtual ~MeshComponent(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void AddMaterial(MaterialPtr material)
	{
		m_MaterialList.push_back(material);
	}

	void CopyFrom(const MeshComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void OnReady(my::DeviceResourceBasePtr res);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void CreateTriangleMeshShape(const my::Vector3 & Scale);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class ClothComponent
	: public Component
	, public my::DeviceResourceBase
{
public:
	std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	std::vector<unsigned char> m_VertexData;

	DWORD m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	bool m_bUseAnimation;

	MaterialPtrList m_MaterialList;

	my::D3DVertexElementSet m_VertexElems;

	std::vector<physx::PxClothParticle> m_particles;

	std::vector<physx::PxClothParticle> m_NewParticles;

	boost::shared_ptr<unsigned char> m_SerializeBuff;

	PhysXPtr<physx::PxClothFabric> m_Fabric;

	PhysXPtr<physx::PxCloth> m_Cloth;

public:
	ClothComponent(void)
		: Component(ComponentTypeCloth)
		, m_bUseAnimation(false)
	{
	}

	virtual ~ClothComponent(void)
	{
	}

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void AddMaterial(MaterialPtr material)
	{
		m_MaterialList.push_back(material);
	}

	void CopyFrom(const ClothComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	void CreateClothFromMesh(my::OgreMeshPtr mesh, unsigned int bone_id);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(PhysXSceneContext * scene);

	virtual void OnLeavePxScene(PhysXSceneContext * scene);

	virtual void OnResetDevice(void);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	virtual void Update(float fElapsedTime);

	void UpdateCloth(void);

	virtual void OnWorldUpdated(void);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public Component
	, public my::Emitter
{
public:
	MaterialPtr m_Material;

	bool m_EmitterToWorld;

protected:
	EmitterComponent(void)
		: Component(ComponentTypeEmitter)
		, Emitter(1)
		, m_EmitterToWorld(false)
	{
	}

public:
	EmitterComponent(ComponentType type, unsigned int capacity)
		: Component(type)
		, Emitter(capacity)
		, m_EmitterToWorld(false)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		ar & BOOST_SERIALIZATION_NVP(m_Material);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterToWorld);
	}

	void CopyFrom(const EmitterComponent & rhs);

	void Spawn(const my::Vector3 & Position, const my::Vector3 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class StaticEmitterComponent
	: public EmitterComponent
{
public:
	StaticEmitterComponent(void)
		: EmitterComponent(ComponentTypeStaticEmitter, 1)
	{
	}

	virtual ~StaticEmitterComponent(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
		ar & BOOST_SERIALIZATION_NVP(m_ParticleList);
	}

	void CopyFrom(const StaticEmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<StaticEmitterComponent> StaticEmitterComponentPtr;

class SphericalEmitterComponent
	: public EmitterComponent
{
public:
	float m_ParticleLifeTime;

	float m_RemainingSpawnTime;

	float m_SpawnInterval;

	my::Vector3 m_HalfSpawnArea;

	float m_SpawnSpeed;

	my::Spline m_SpawnInclination;

	my::Spline m_SpawnAzimuth;

	my::Spline m_SpawnColorR;

	my::Spline m_SpawnColorG;

	my::Spline m_SpawnColorB;

	my::Spline m_SpawnColorA;

	my::Spline m_SpawnSizeX;

	my::Spline m_SpawnSizeY;

	my::Spline m_SpawnAngle;

	float m_SpawnLoopTime;

public:
	SphericalEmitterComponent(void)
		: EmitterComponent(ComponentTypeSphericalEmitter, PARTICLE_INSTANCE_MAX)
		, m_ParticleLifeTime(FLT_MAX)
		, m_RemainingSpawnTime(0)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnLoopTime(5)
	{
	}

	virtual ~SphericalEmitterComponent(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
		ar & BOOST_SERIALIZATION_NVP(m_ParticleLifeTime);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnInterval);
		ar & BOOST_SERIALIZATION_NVP(m_HalfSpawnArea);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnSpeed);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnInclination);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnAzimuth);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnColorR);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnColorG);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnColorB);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnColorA);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnSizeX);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnSizeY);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnAngle);
		ar & BOOST_SERIALIZATION_NVP(m_SpawnLoopTime);
	}

	void CopyFrom(const SphericalEmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<SphericalEmitterComponent> SphericalEmitterComponentPtr;

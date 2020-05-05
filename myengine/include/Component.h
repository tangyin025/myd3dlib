#pragma once

#include <PxPhysicsAPI.h>
#include "myOctree.h"
#include "myEmitter.h"
#include "myMesh.h"
#include <atlbase.h>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

namespace my
{
	class Effect;
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;

class Actor;

class PhysxSceneContext;

class RenderPipeline;

class Component;

typedef boost::shared_ptr<Component> ComponentPtr;

class Component : public boost::enable_shared_from_this<Component>
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
		ComponentTypeQuadTerrain,
	};

	ComponentType m_Type;

	enum LODMask
	{
		LOD0 = 1 << 0,
		LOD1 = 1 << 1,
		LOD2 = 1 << 2,
		LOD0_1 = LOD0 | LOD1,
		LOD1_2 = LOD1 | LOD2,
		LOD0_1_2 = LOD0 | LOD1 | LOD2,
	};

	LODMask m_LodMask;

	Actor * m_Actor;

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxShape> m_PxShape;

protected:
	Component(void)
		: m_Type(ComponentTypeComponent)
		, m_LodMask(LOD0_1_2)
		, m_Actor(NULL)
	{
	}

	Component(ComponentType Type)
		: m_Type(Type)
		, m_LodMask(LOD0_1_2)
		, m_Actor(NULL)
	{
	}

public:
	virtual ~Component(void);

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

	virtual void EnterPhysxScene(PhysxSceneContext * scene);

	virtual void LeavePhysxScene(PhysxSceneContext * scene);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void OnShaderChanged(void);

	virtual void Update(float fElapsedTime);

	virtual void OnWorldUpdated(void);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, unsigned int filterWord0);

	void CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, unsigned int filterWord0);

	void CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, unsigned int filterWord0);

	void CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, unsigned int filterWord0);

	virtual void ClearShape(void);
};

class MeshComponent
	: public Component
	, public my::IResourceCallback
{
public:
	std::string m_MeshPath;

	std::string m_MeshSubMeshName;

	my::OgreMeshPtr m_Mesh;

	my::EventFunction m_MeshEventReady;

	bool m_bInstance;

	bool m_bUseAnimation;

	MaterialPtrList m_MaterialList;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_dualquat;

public:
	MeshComponent(void)
		: Component(ComponentTypeMesh)
		, m_bInstance(false)
		, m_bUseAnimation(false)
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_dualquat(NULL)
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

	virtual void OnReady(my::IORequest * request);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void OnShaderChanged(void);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateTriangleMeshShape(unsigned int filterWord0);

	void CreateConvexMeshShape(bool bInflateConvex, unsigned int filterWord0);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class ClothComponent
	: public Component
{
public:
	std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	std::vector<unsigned char> m_VertexData;

	UINT m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	bool m_bUseAnimation;

	MaterialPtrList m_MaterialList;

	my::D3DVertexElementSet m_VertexElems;

	std::vector<physx::PxClothParticle> m_particles;

	boost::shared_ptr<unsigned char> m_SerializeBuff;

	boost::shared_ptr<physx::PxClothFabric> m_Fabric;

	boost::shared_ptr<physx::PxCloth> m_Cloth;

	typedef std::pair<physx::PxClothCollisionSphere, int> ClothCollisionSpherePair;

	typedef std::vector<ClothCollisionSpherePair> ClothCollisionSpherePairList;
	
	ClothCollisionSpherePairList m_ClothSpheres;

	std::vector<physx::PxClothCollisionSphere> m_ClothSpheresTmp;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_dualquat;

public:
	ClothComponent(void)
		: Component(ComponentTypeCloth)
		, m_bUseAnimation(false)
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_dualquat(NULL)
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

	void CreateClothFromMesh(my::OgreMeshPtr mesh);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void EnterPhysxScene(PhysxSceneContext * scene);

	virtual void LeavePhysxScene(PhysxSceneContext * scene);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void OnShaderChanged(void);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void Update(float fElapsedTime);

	void UpdateCloth(void);

	virtual void OnWorldUpdated(void);

	void OnPxThreadSubstep(float dtime);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public Component
	, public my::Emitter
{
public:
	my::D3DVertexElementSet m_VertexElems;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	UINT m_NumVertices;

	UINT m_VertexStride;

	UINT m_PrimitiveCount;

	my::VertexBuffer m_vb;

	my::IndexBuffer m_ib;

	MaterialPtr m_Material;

	enum FaceType
	{
		FaceTypeX			= 0,
		FaceTypeY			= 1,
		FaceTypeZ			= 2,
		FaceTypeCamera		= 3,
		FaceTypeAngle		= 4,
		FaceTypeAngleCamera = 5,
	};

	FaceType m_EmitterFaceType;

	enum VelocityType
	{
		VelocityTypeNone	= 0,
		VelocityTypeVel		= 1,
	};

	VelocityType m_EmitterVelType;

	float m_EmitterTime;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_EmitterScale;

protected:
	EmitterComponent(void)
		: Component(ComponentTypeEmitter)
		, Emitter(1)
		, m_EmitterFaceType(FaceTypeX)
		, m_EmitterVelType(VelocityTypeVel)
		, m_EmitterTime(0)
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_EmitterScale(NULL)
	{
	}

public:
	EmitterComponent(ComponentType type, unsigned int capacity)
		: Component(type)
		, Emitter(capacity)
		, m_EmitterFaceType(FaceTypeX)
		, m_EmitterVelType(VelocityTypeVel)
		, m_EmitterTime(0)
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_EmitterScale(NULL)
	{
		WORD offset = 0;
		m_VertexElems.InsertPositionElement(offset);
		offset += sizeof(my::Vector3);
		m_VertexElems.InsertTexcoordElement(offset, 0);
		offset += sizeof(my::Vector2);
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		ar & BOOST_SERIALIZATION_NVP(m_VertexElems);
		ar & BOOST_SERIALIZATION_NVP(m_Material);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterFaceType);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterVelType);
	}

	void CopyFrom(const EmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void OnShaderChanged(void);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class StaticEmitterComponent
	: public EmitterComponent
{
protected:
	StaticEmitterComponent(void)
	{
	}

public:
	StaticEmitterComponent(unsigned int capacity)
		: EmitterComponent(ComponentTypeStaticEmitter, capacity)
	{
	}

	virtual ~StaticEmitterComponent(void)
	{
	}

	friend class boost::serialization::access;

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

	float m_SpawnCycle;

protected:
	SphericalEmitterComponent(void)
		: EmitterComponent(ComponentTypeSphericalEmitter, 1)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0, 0, 0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
	{
	}

public:
	SphericalEmitterComponent(unsigned int capacity)
		: EmitterComponent(ComponentTypeSphericalEmitter, capacity)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
	{
	}

	virtual ~SphericalEmitterComponent(void)
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

	void CopyFrom(const SphericalEmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<SphericalEmitterComponent> SphericalEmitterComponentPtr;

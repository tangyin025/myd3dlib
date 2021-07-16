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

class Actor;

class PhysxScene;

class RenderPipeline;

class Component;

typedef boost::shared_ptr<Component> ComponentPtr;

class Component
	: public my::NamedObject
	, public boost::enable_shared_from_this<Component>
{
public:
	enum ComponentType
	{
		ComponentTypeComponent,
		ComponentTypeActor,
		ComponentTypeController,
		ComponentTypeMesh,
		ComponentTypeCloth,
		ComponentTypeStaticEmitter,
		ComponentTypeSphericalEmitter,
		ComponentTypeTerrain,
		ComponentTypeScript,
		ComponentTypeAnimator,
		ComponentTypeNavigation,
	};

	ComponentType m_Type;

	enum LODMask
	{
		LOD0 = 1 << 0,
		LOD1 = 1 << 1,
		LOD2 = 1 << 2,
		LOD_CULLING = 1 << 3,
		LOD0_1 = LOD0 | LOD1,
		LOD1_2 = LOD1 | LOD2,
		LOD0_1_2 = LOD0 | LOD1 | LOD2,
	};

	LODMask m_LodMask;

	Actor * m_Actor;

	bool m_Requested;

	MaterialPtr m_Material;

	boost::shared_ptr<physx::PxMaterial> m_PxMaterial;

	boost::shared_ptr<physx::PxShape> m_PxShape;

protected:
	Component(void)
		: m_Type(ComponentTypeComponent)
		, m_LodMask(LOD0_1_2)
		, m_Actor(NULL)
		, m_Requested(false)
	{
	}

	Component(ComponentType Type, const char * Name)
		: NamedObject(Name)
		, m_Type(Type)
		, m_LodMask(LOD0_1_2)
		, m_Actor(NULL)
		, m_Requested(false)
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

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void CopyFrom(const Component & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
	{
	}

	virtual void SetPxPoseOrbyPxThread(const physx::PxTransform& pose)
	{
	}

	virtual void Update(float fElapsedTime)
	{
	}

	virtual void OnPxThreadSubstep(float dtime)
	{
	}

	virtual my::AABB CalculateAABB(void) const
	{
		return my::AABB::Invalid();
	}

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
	{
	}

	void SetMaterial(MaterialPtr material);

	MaterialPtr GetMaterial(void) const;

	physx::PxMaterial * CreatePhysxMaterial(float staticFriction, float dynamicFriction, float restitution, bool ShareSerializeCollection);

	void CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, bool ShareSerializeCollection);

	void CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, bool ShareSerializeCollection);

	void CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, bool ShareSerializeCollection);

	void CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, bool ShareSerializeCollection);

	void SetSimulationFilterWord0(unsigned int filterWord0);

	unsigned int GetSimulationFilterWord0(void) const;

	void SetQueryFilterWord0(unsigned int filterWord0);

	void SetShapeFlag(physx::PxShapeFlag::Enum Flag, bool Value);

	bool GetShapeFlag(physx::PxShapeFlag::Enum Flag) const;

	unsigned int GetQueryFilterWord0(void) const;

	virtual void ClearShape(void);
};

class MeshComponent
	: public Component
{
public:
	std::string m_MeshPath;

	std::string m_MeshSubMeshName;

	int m_MeshSubMeshId;

	my::OgreMeshPtr m_Mesh;

	my::Vector4 m_MeshColor;

	bool m_bInstance;

	boost::shared_ptr<physx::PxBase> m_PxMesh;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_MeshColor;

	D3DXHANDLE handle_dualquat;

protected:
	MeshComponent(void)
		: m_MeshSubMeshId(0)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, m_bInstance(false)
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
		, handle_dualquat(NULL)
	{
	}

public:
	MeshComponent(const char * Name)
		: Component(ComponentTypeMesh, Name)
		, m_MeshSubMeshId(0)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, m_bInstance(false)
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
		, handle_dualquat(NULL)
	{
	}

	virtual ~MeshComponent(void);

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

	void CopyFrom(const MeshComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	void OnMeshReady(my::DeviceResourceBasePtr res);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	physx::PxTriangleMesh * CreateTriangleMesh(bool ShareSerializeCollection);

	void CreateTriangleMeshShape(bool ShareSerializeCollection);

	physx::PxConvexMesh * CreateConvexMesh(bool bInflateConvex, bool ShareSerializeCollection);

	void CreateConvexMeshShape(bool bInflateConvex, bool ShareSerializeCollection);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class ClothComponent
	: public Component
{
public:
	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	std::vector<unsigned char> m_VertexData;

	UINT m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	my::Vector4 m_MeshColor;

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

	D3DXHANDLE handle_MeshColor;

	D3DXHANDLE handle_dualquat;

protected:
	ClothComponent(void)
		: m_MeshColor(my::Vector4(1, 1, 1, 1))
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
		, handle_dualquat(NULL)
	{
	}

public:
	ClothComponent(const char * Name)
		: Component(ComponentTypeCloth, Name)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, handle_Time(NULL)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
		, handle_dualquat(NULL)
	{
	}

	virtual ~ClothComponent(void);

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

	void CopyFrom(const ClothComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	void CreateClothFromMesh(my::OgreMeshPtr mesh, DWORD AttribId);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void SetPxPoseOrbyPxThread(const physx::PxTransform& pose);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void Update(float fElapsedTime);

	void UpdateCloth(void);

	void OnPxThreadSubstep(float dtime);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public Component
{
public:
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

	enum SpaceType
	{
		SpaceTypeWorld		= 0,
		SpaceTypeLocal		= 1,
	};

	SpaceType m_EmitterSpaceType;

	enum VelocityType
	{
		VelocityTypeNone	= 0,
		VelocityTypeVel		= 1,
	};

	VelocityType m_EmitterVelType;

	enum PrimitiveType
	{
		PrimitiveTypeTri = 0,
		PrimitiveTypeQuad = 1,
	};

	PrimitiveType m_EmitterPrimitiveType;

	D3DXHANDLE handle_World;

protected:
	EmitterComponent(void)
		: m_EmitterFaceType(FaceTypeX)
		, m_EmitterSpaceType(SpaceTypeWorld)
		, m_EmitterVelType(VelocityTypeNone)
		, m_EmitterPrimitiveType(PrimitiveTypeQuad)
		, handle_World(NULL)
	{
	}

public:
	EmitterComponent(ComponentType Type, const char * Name, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: Component(Type, Name)
		, m_EmitterFaceType(_FaceType)
		, m_EmitterSpaceType(_SpaceTypeWorld)
		, m_EmitterVelType(_VelocityType)
		, m_EmitterPrimitiveType(_PrimitiveType)
		, handle_World(NULL)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterFaceType);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterSpaceType);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterVelType);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterPrimitiveType);
	}

	void CopyFrom(const EmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	void AddParticlePairToPipeline(RenderPipeline* pipeline, unsigned int PassMask, my::Emitter::Particle* particles1, unsigned int particle_num1, my::Emitter::Particle* particles2, unsigned int particle_num2);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class SphericalEmitter
	: public EmitterComponent
	, public my::Emitter
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

	float m_SpawnTime;

protected:
	SphericalEmitter(void)
		: m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0, 0, 0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
		, m_SpawnTime(FLT_MAX)
	{
	}

public:
	SphericalEmitter(const char * Name, unsigned int Capacity, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: EmitterComponent(ComponentTypeSphericalEmitter, Name, _FaceType, _SpaceTypeWorld, _VelocityType, _PrimitiveType)
		, Emitter(Capacity)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
		, m_SpawnTime(FLT_MAX)
	{
	}

	virtual ~SphericalEmitter(void)
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

	void CopyFrom(const SphericalEmitter & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

typedef boost::shared_ptr<SphericalEmitter> SphericalEmitterPtr;

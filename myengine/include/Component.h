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

struct ComponentEventArg : public my::EventArg
{
public:
	Component * self;

	ComponentEventArg(Component * _self)
		: self(_self)
	{
	}
};

class Component
	: public my::NamedObject
	, public boost::enable_shared_from_this<Component>
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

	virtual void EnterPhysxScene(PhysxScene * scene);

	virtual void LeavePhysxScene(PhysxScene * scene);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void SetMaterial(MaterialPtr material);

	MaterialPtr GetMaterial(void) const;

	void CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz, unsigned int filterWord0);

	void CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight, unsigned int filterWord0);

	void CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot, unsigned int filterWord0);

	void CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, unsigned int filterWord0);

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
	, public my::IResourceCallback
{
public:
	std::string m_MeshPath;

	std::string m_MeshSubMeshName;

	int m_MeshSubMeshId;

	my::OgreMeshPtr m_Mesh;

	my::EventFunction m_MeshEventReady;

	my::Vector4 m_MeshColor;

	bool m_bInstance;

	bool m_bUseAnimation;

	D3DXHANDLE handle_Time;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_MeshColor;

	D3DXHANDLE handle_dualquat;

protected:
	MeshComponent(void)
		: m_MeshSubMeshId(0)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, m_bInstance(false)
		, m_bUseAnimation(false)
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
		, m_bUseAnimation(false)
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

	virtual void OnReady(my::IORequest * request);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateTriangleMeshShape(unsigned int filterWord0);

	void CreateConvexMeshShape(bool bInflateConvex, unsigned int filterWord0);
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

	bool m_bUseAnimation;

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
		, m_bUseAnimation(false)
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
		, m_bUseAnimation(false)
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

	virtual void EnterPhysxScene(PhysxScene * scene);

	virtual void LeavePhysxScene(PhysxScene * scene);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual my::AABB CalculateAABB(void) const;

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void Update(float fElapsedTime);

	void UpdateCloth(void);

	void OnPxThreadSubstep(float dtime);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public Component
	, public my::Emitter
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

	enum VelocityType
	{
		VelocityTypeNone	= 0,
		VelocityTypeVel		= 1,
	};

	VelocityType m_EmitterVelType;

	D3DXHANDLE handle_World;

protected:
	EmitterComponent(void)
		: Emitter(1)
		, m_EmitterFaceType(FaceTypeX)
		, m_EmitterVelType(VelocityTypeNone)
		, handle_World(NULL)
	{
	}

public:
	EmitterComponent(ComponentType Type, const char * Name, unsigned int Capacity, FaceType _FaceType, VelocityType _VelocityType)
		: Component(Type, Name)
		, Emitter(Capacity)
		, m_EmitterFaceType(_FaceType)
		, m_EmitterVelType(_VelocityType)
		, handle_World(NULL)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterFaceType);
		ar & BOOST_SERIALIZATION_NVP(m_EmitterVelType);
	}

	void CopyFrom(const EmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual my::AABB CalculateAABB(void) const;

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
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
	StaticEmitterComponent(const char * Name, unsigned int Capacity)
		: EmitterComponent(ComponentTypeStaticEmitter, Name, Capacity, FaceTypeCamera, VelocityTypeNone)
	{
	}

	virtual ~StaticEmitterComponent(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive& ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
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

	float m_SpawnTime;

protected:
	SphericalEmitterComponent(void)
		: m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0, 0, 0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
		, m_SpawnTime(FLT_MAX)
	{
	}

public:
	SphericalEmitterComponent(const char * Name, unsigned int Capacity)
		: EmitterComponent(ComponentTypeSphericalEmitter, Name, Capacity, FaceTypeCamera, VelocityTypeVel)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
		, m_SpawnTime(FLT_MAX)
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

	virtual void RequestResource(void);

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<SphericalEmitterComponent> SphericalEmitterComponentPtr;

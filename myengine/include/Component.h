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

	class UIRender;
};

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

class Actor;

class PhysxScene;

class RenderPipeline;

class Component;

typedef boost::shared_ptr<Component> ComponentPtr;

class Animator;

class Component
	: public my::NamedObject
	, public my::ShaderResourceBase
	, public boost::enable_shared_from_this<Component>
{
public:
	enum ComponentType
	{
		ComponentTypeComponent,
		ComponentTypeActor,
		ComponentTypeController,
		ComponentTypeMesh,
		ComponentTypeStaticMesh,
		ComponentTypeCloth,
		ComponentTypeEmitter,
		ComponentTypeStaticEmitter,
		ComponentTypeCircularEmitter,
		ComponentTypeSphericalEmitter,
		ComponentTypeTerrain,
		ComponentTypeAnimator,
		ComponentTypeNavigation,
		ComponentTypeSteering,
		ComponentTypeScript,
	};

	enum LODMask
	{
		LOD0 = 1 << 0,
		LOD1 = 1 << 1,
		LOD2 = 1 << 2,
		LOD0_1 = LOD0 | LOD1,
		LOD1_2 = LOD1 | LOD2,
		LOD0_1_2 = LOD0 | LOD1 | LOD2,
	};

	enum { TypeID = ComponentTypeComponent };

	LODMask m_LodMask;

	Actor * m_Actor;

	bool m_Requested;

	MaterialPtr m_Material;

	physx::PxGeometryType::Enum m_PxGeometryType;

	boost::shared_ptr<physx::PxShape> m_PxShape;

protected:
	Component(void)
		: m_LodMask(LOD0_1_2)
		, m_Actor(NULL)
		, m_Requested(false)
		, m_PxGeometryType(physx::PxGeometryType::eINVALID)
	{
	}

	Component(const char * Name)
		: NamedObject(Name)
		, m_LodMask(LOD0_1_2)
		, m_Actor(NULL)
		, m_Requested(false)
		, m_PxGeometryType(physx::PxGeometryType::eINVALID)
	{
	}

public:
	virtual ~Component(void);

	bool operator ==(const Component & rhs) const
	{
		return this == &rhs;
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	virtual void OnResetShader(void)
	{
	}

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
	{
	}

	virtual void SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot)
	{
	}

	virtual void Update(float fElapsedTime)
	{
	}

	virtual void OnPxThreadSubstep(float dtime)
	{
	}

	virtual void OnTrigger(my::EventArg * arg)
	{
	}

	virtual void OnContact(my::EventArg* arg)
	{
	}

	virtual void OnPxThreadShapeHit(my::EventArg * arg)
	{
	}

	virtual void OnPxThreadControllerHit(my::EventArg * arg)
	{
	}

	virtual void OnPxThreadObstacleHit(my::EventArg * arg)
	{
	}

	virtual void OnGUI(my::UIRender * ui_render, float fElapsedTime, const my::Vector2 & Viewport)
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

	MaterialPtr GetMaterial(void) const
	{
		return m_Material;
	}

	physx::PxMaterial * CreatePhysxMaterial(float staticFriction, float dynamicFriction, float restitution);

	void CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz);

	void CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight);

	void CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot);

	void CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius);

	virtual void SetSimulationFilterWord0(unsigned int filterWord0);

	virtual unsigned int GetSimulationFilterWord0(void) const;

	virtual void SetQueryFilterWord0(unsigned int filterWord0);

	virtual unsigned int GetQueryFilterWord0(void) const;

	virtual void SetShapeFlags(unsigned int Flags);

	virtual unsigned int GetShapeFlags(void) const;

	physx::PxGeometryType::Enum GetGeometryType(void) const;

	void SetShapeLocalPose(const my::Bone & pose);

	my::Bone GetShapeLocalPose(void) const;

	virtual void ClearShape(void);

	unsigned int GetSiblingId(void) const;

	void SetSiblingId(unsigned int i);
};

class MeshComponent
	: public Component
{
public:
	enum { TypeID = ComponentTypeMesh };

	std::string m_MeshPath;

	int m_MeshSubMeshId;

	my::OgreMeshPtr m_Mesh;

	my::Vector4 m_MeshColor;

	enum InstanceType
	{
		InstanceTypeNone = 0,
		InstanceTypeInstance,
		InstanceTypeBatch,
	};

	InstanceType m_InstanceType;

	std::string m_PxMeshPath;

	class PhysxBaseResource : public my::DeviceResourceBase
	{
	public:
		physx::PxBase* m_ptr;

		PhysxBaseResource(void)
			: m_ptr(NULL)
		{
		}

		virtual ~PhysxBaseResource(void);

		void OnResetDevice(void)
		{
		}

		void OnLostDevice(void)
		{
		}

		void OnDestroyDevice(void)
		{
		}

		void Create(physx::PxBase* ptr);
	};

	typedef boost::shared_ptr<PhysxBaseResource> PhysxBaseResourcePtr;

	PhysxBaseResourcePtr m_PxMesh;

	unsigned int m_DescSimulationFilterWord0;

	unsigned int m_DescQueryFilterWord0;

	unsigned int m_DescShapeFlags;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_MeshColor;

	D3DXHANDLE handle_dualquat;

protected:
	MeshComponent(void)
		: m_MeshSubMeshId(0)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, m_InstanceType(InstanceTypeNone)
		, m_DescSimulationFilterWord0(0)
		, m_DescQueryFilterWord0(0)
		, m_DescShapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
		, handle_dualquat(NULL)
	{
	}

public:
	MeshComponent(const char * Name)
		: Component(Name)
		, m_MeshSubMeshId(0)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, m_InstanceType(InstanceTypeNone)
		, m_DescSimulationFilterWord0(0)
		, m_DescQueryFilterWord0(0)
		, m_DescShapeFlags(physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eSCENE_QUERY_SHAPE)
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void OnResetShader(void);

	void OnMeshReady(my::DeviceResourceBasePtr res);

	void OnPxMeshReady(my::DeviceResourceBasePtr res, physx::PxGeometryType::Enum type);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateTriangleMeshShape(my::OgreMesh * mesh, int sub_mesh_id, const char * TriangleMeshPath);

	void CreateConvexMeshShape(my::OgreMesh * mesh, int sub_mesh_id, const char * ConvexMeshPath, bool bInflateConvex);

	virtual void SetSimulationFilterWord0(unsigned int filterWord0);

	virtual unsigned int GetSimulationFilterWord0(void) const;

	virtual void SetQueryFilterWord0(unsigned int filterWord0);

	virtual unsigned int GetQueryFilterWord0(void) const;

	virtual void SetShapeFlags(unsigned int Flags);

	virtual unsigned int GetShapeFlags(void) const;

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class ClothComponent
	: public Component
{
public:
	enum { TypeID = ComponentTypeCloth };

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	std::vector<unsigned char> m_VertexData;

	UINT m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	my::Vector4 m_MeshColor;

	my::D3DVertexElementSet m_VertexElems;

	std::vector<physx::PxClothParticle> m_particles;

	std::string m_ClothFabricPath;

	boost::shared_ptr<physx::PxClothFabric> m_ClothFabric;

	boost::shared_ptr<physx::PxCloth> m_Cloth;

	typedef std::pair<physx::PxClothCollisionSphere, int> ClothCollisionSpherePair;

	typedef std::vector<ClothCollisionSpherePair> ClothCollisionSpherePairList;
	
	ClothCollisionSpherePairList m_ClothSphereBones;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_MeshColor;

protected:
	ClothComponent(void)
		: m_MeshColor(my::Vector4(1, 1, 1, 1))
		, handle_World(NULL)
		, handle_MeshColor(NULL)
	{
	}

public:
	ClothComponent(const char * Name)
		: Component(Name)
		, m_MeshColor(my::Vector4(1, 1, 1, 1))
		, handle_World(NULL)
		, handle_MeshColor(NULL)
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void OnResetShader(void);

	void CreateClothFromMesh(const char * ClothFabricPath, my::OgreMeshPtr mesh, int sub_mesh_id, const my::Vector3 & gravity);

	void CreateVirtualParticles(int level);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void Update(float fElapsedTime);

	void UpdateVertexData(unsigned char * pVertices, const physx::PxClothParticle * particles, unsigned int NbParticles, Animator* animator) const;

	void OnPxThreadSubstep(float dtime);

	void SetClothFlags(unsigned int Flags);

	unsigned int GetClothFlags(void) const;

	void SetExternalAcceleration(const my::Vector3& acceleration);

	my::Vector3 GetExternalAcceleration(void) const;
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public Component
{
public:
	enum { TypeID = ComponentTypeEmitter };

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
	EmitterComponent(const char * Name, FaceType _FaceType, SpaceType _SpaceType, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: Component(Name)
		, m_EmitterFaceType(_FaceType)
		, m_EmitterSpaceType(_SpaceType)
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void OnResetShader(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	void AddParticlePairToPipeline(RenderPipeline* pipeline, unsigned int PassMask, my::Emitter::Particle* particles1, unsigned int particle_num1, my::Emitter::Particle* particles2, unsigned int particle_num2);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class CircularEmitter
	: public EmitterComponent
	, public my::Emitter
{
public:
	enum { TypeID = ComponentTypeCircularEmitter };

protected:
	CircularEmitter(void)
	{
	}

public:
	CircularEmitter(const char * Name, unsigned int Capacity, FaceType _FaceType, SpaceType _SpaceType, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: EmitterComponent(Name, _FaceType, _SpaceType, _VelocityType, _PrimitiveType)
		, Emitter(Capacity)
	{
	}

	virtual ~CircularEmitter(void)
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

typedef boost::shared_ptr<CircularEmitter> CircularEmitterPtr;

class SphericalEmitter
	: public CircularEmitter
{
public:
	enum { TypeID = ComponentTypeSphericalEmitter };

	float m_ParticleLifeTime;

	float m_SpawnInterval;

	my::Vector3 m_HalfSpawnArea;

	my::Spline m_SpawnInclination;

	my::Spline m_SpawnAzimuth;

	float m_SpawnSpeed;

	my::Spline m_SpawnColorR;

	my::Spline m_SpawnColorG;

	my::Spline m_SpawnColorB;

	my::Spline m_SpawnColorA;

	my::Spline m_SpawnSizeX;

	my::Spline m_SpawnSizeY;

	my::Spline m_SpawnAngle;

	float m_SpawnCycle;

	float m_SpawnTime;

	int m_SpawnBoneId;

	my::Bone m_SpawnLocalPose;

protected:
	SphericalEmitter(void)
		: m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0, 0, 0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
		, m_SpawnTime(FLT_MAX)
		, m_SpawnBoneId(-1)
		, m_SpawnLocalPose(my::Vector3(0, 0, 0))
	{
	}

public:
	SphericalEmitter(const char * Name, unsigned int Capacity, FaceType _FaceType, SpaceType _SpaceType, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
		: CircularEmitter(Name, Capacity, _FaceType, _SpaceType, _VelocityType, _PrimitiveType)
		, m_ParticleLifeTime(FLT_MAX)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnCycle(5)
		, m_SpawnTime(FLT_MAX)
		, m_SpawnBoneId(-1)
		, m_SpawnLocalPose(my::Vector3(0, 0, 0))
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

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;
};

typedef boost::shared_ptr<SphericalEmitter> SphericalEmitterPtr;

#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "ResourceBundle.h"
#include "PhysXPtr.h"
#include <boost/serialization/nvp.hpp>

class Material;

typedef boost::shared_ptr<Material> MaterialPtr;

class Material
{
public:
	enum PassMask
	{
		PassMaskNone			= 0,
		PassMaskLight			= 1 << RenderPipeline::PassTypeLight,
		PassMaskOpaque			= 1 << RenderPipeline::PassTypeShadow | 1 << RenderPipeline::PassTypeNormal | 1 << RenderPipeline::PassTypeOpaque,
		PassMaskTransparent		= 1 << RenderPipeline::PassTypeTransparent,
	};

	enum BlendMode
	{
		BlendModeNone = 0,
		BlendModeAlpha,
		BlendModeAdditive,
	};

	std::string m_Shader;

	unsigned int m_PassMask;

	DWORD m_CullMode;

	BOOL m_ZEnable;

	BOOL m_ZWriteEnable;

	DWORD m_BlendMode;

	my::Vector4 m_MeshColor;

	ResourceBundle<my::BaseTexture> m_MeshTexture;

	ResourceBundle<my::BaseTexture> m_NormalTexture;

	ResourceBundle<my::BaseTexture> m_SpecularTexture;

public:
	Material(void)
		: m_PassMask(PassMaskNone)
		, m_CullMode(D3DCULL_CW)
		, m_ZEnable(TRUE)
		, m_ZWriteEnable(TRUE)
		, m_BlendMode(BlendModeNone)
		, m_MeshColor(1,1,1,1)
	{
	}

	virtual ~Material(void)
	{
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Shader);
		ar & BOOST_SERIALIZATION_NVP(m_PassMask);
		ar & BOOST_SERIALIZATION_NVP(m_CullMode);
		ar & BOOST_SERIALIZATION_NVP(m_ZEnable);
		ar & BOOST_SERIALIZATION_NVP(m_ZWriteEnable);
		ar & BOOST_SERIALIZATION_NVP(m_BlendMode);
		ar & BOOST_SERIALIZATION_NVP(m_MeshColor);
		ar & BOOST_SERIALIZATION_NVP(m_MeshTexture);
		ar & BOOST_SERIALIZATION_NVP(m_NormalTexture);
		ar & BOOST_SERIALIZATION_NVP(m_SpecularTexture);
	}

	void CopyFrom(const Material & rhs);

	virtual MaterialPtr Clone(void) const;

	void RequestResource(void);

	void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);
};

typedef std::vector<MaterialPtr> MaterialPtrList;

class Actor;

class PhysXSceneContext;

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

	bool m_Requested;

	PhysXPtr<physx::PxMaterial> m_PxMaterial;

	PhysXPtr<physx::PxShape> m_PxShape;

protected:
	Component(ComponentType Type, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: m_Type(Type)
		, m_Actor(NULL)
		, m_Requested(false)
	{
	}

public:
	Component(void)
		: m_Type(ComponentTypeComponent)
		, m_Actor(NULL)
		, m_Requested(false)
	{
	}

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

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void CopyFrom(const Component & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(PhysXSceneContext * scene);

	virtual void OnLeavePxScene(PhysXSceneContext * scene);

	virtual void Update(float fElapsedTime);

	virtual void OnWorldChanged(void);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void CreateBoxShape(const my::Vector3 & pos, const my::Quaternion & rot, float hx, float hy, float hz);

	void CreateCapsuleShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius, float halfHeight);

	void CreatePlaneShape(const my::Vector3 & pos, const my::Quaternion & rot);

	void CreateSphereShape(const my::Vector3 & pos, const my::Quaternion & rot, float radius);

	virtual void ClearShape(void);
};

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
protected:
	RenderComponent(ComponentType Type, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: Component(Type, Position, Rotation, Scale)
	{
	}

public:
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	}

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId) = 0;
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	ResourceBundle<my::OgreMesh> m_MeshRes;

	bool m_bInstance;

	bool m_bUseAnimation;

	MaterialPtrList m_MaterialList;

public:
	MeshComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: RenderComponent(ComponentTypeMesh, Position, Rotation, Scale)
		, m_bInstance(false)
		, m_bUseAnimation(false)
	{
	}

	MeshComponent(void)
		: RenderComponent(ComponentTypeMesh, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
		, m_bInstance(false)
		, m_bUseAnimation(false)
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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void CreateTriangleMeshShape(const my::Vector3 & Scale);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class ClothComponent
	: public RenderComponent
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
	ClothComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: RenderComponent(ComponentTypeCloth, Position, Rotation, Scale)
		, m_bUseAnimation(false)
	{
	}

	ClothComponent(void)
		: RenderComponent(ComponentTypeCloth, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
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

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	virtual void Update(float fElapsedTime);

	void UpdateCloth(void);

	virtual void OnWorldChanged(void);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public RenderComponent
{
public:
	my::EmitterPtr m_Emitter;

	MaterialPtr m_Material;

protected:
	EmitterComponent(ComponentType type, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: RenderComponent(type, Position, Rotation, Scale)
	{
	}

public:
	EmitterComponent(void)
		: RenderComponent(ComponentTypeEmitter, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
		ar & BOOST_SERIALIZATION_NVP(m_Emitter);
		ar & BOOST_SERIALIZATION_NVP(m_Material);
	}

	void CopyFrom(const EmitterComponent & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class StaticEmitterComponent
	: public EmitterComponent
{
public:
	StaticEmitterComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: EmitterComponent(ComponentTypeStaticEmitter, Position, Rotation, Scale)
	{
	}

	StaticEmitterComponent(void)
		: EmitterComponent(ComponentTypeStaticEmitter, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
	{
	}

	virtual ~StaticEmitterComponent(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
		ar & boost::serialization::make_nvp("ParticleList", m_Emitter->m_ParticleList);
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
	SphericalEmitterComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: EmitterComponent(ComponentTypeSphericalEmitter, Position, Rotation, Scale)
		, m_ParticleLifeTime(FLT_MAX)
		, m_RemainingSpawnTime(0)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnLoopTime(5)
	{
	}

	SphericalEmitterComponent(void)
		: EmitterComponent(ComponentTypeSphericalEmitter, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
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

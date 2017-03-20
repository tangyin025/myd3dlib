#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"
#include <boost/serialization/nvp.hpp>

class Material
{
public:
	std::string m_Shader;

	unsigned int m_PassMask;

	my::Vector4 m_MeshColor;

	ResourceBundle<my::BaseTexture> m_MeshTexture;

	ResourceBundle<my::BaseTexture> m_NormalTexture;

	ResourceBundle<my::BaseTexture> m_SpecularTexture;

public:
	Material(void)
		: m_PassMask(RenderPipeline::PassMaskNone)
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
		ar & BOOST_SERIALIZATION_NVP(m_MeshColor);
		ar & BOOST_SERIALIZATION_NVP(m_MeshTexture);
		ar & BOOST_SERIALIZATION_NVP(m_NormalTexture);
		ar & BOOST_SERIALIZATION_NVP(m_SpecularTexture);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	void RequestResource(void);

	void ReleaseResource(void);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;

class Component;

typedef boost::shared_ptr<Component> ComponentPtr;

class Component
	: public my::OctActor
{
public:
	enum ComponentType
	{
		ComponentTypeUnknown,
		ComponentTypeComponent,
		ComponentTypeActor,
		ComponentTypeMesh,
		ComponentTypeEmitter,
		ComponentTypeSphericalEmitter,
		ComponentTypeTerrain,
		//ComponentTypeRigid,
	};

	ComponentType m_Type;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	AnimatorPtr m_Animator;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	Component * m_Parent;

public:
	Component(ComponentType Type, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: m_Type(Type)
		, m_Position(Position)
		, m_Rotation(Rotation)
		, m_Scale(Scale)
		, m_World(my::Matrix4::Identity())
		, m_Parent(NULL)
	{
	}

	Component(void)
		: m_Type(ComponentTypeUnknown)
		, m_Position(0,0,0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1,1,1)
		, m_World(my::Matrix4::Identity())
		, m_Parent(NULL)
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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	void UpdateWorld(void);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(ComponentPtr cmp);

	Component * GetTopParent(void);

	Animator * GetAnimator(void);
};

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(ComponentType Type, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: Component(Type, Position, Rotation, Scale)
	{
	}

	RenderComponent(ComponentType Type)
		: Component(Type, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	ResourceBundle<my::OgreMesh> m_MeshRes;

	bool m_bAnimation;

	bool m_bInstance;

	MaterialPtrList m_MaterialList;

	bool m_StaticCollision;

public:
	MeshComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, bool bAnimation, bool bInstance)
		: RenderComponent(ComponentTypeMesh, Position, Rotation, Scale)
		, m_bAnimation(bAnimation)
		, m_bInstance(bInstance)
		, m_StaticCollision(false)
	{
	}

	MeshComponent(void)
		: RenderComponent(ComponentTypeMesh)
		, m_bAnimation(false)
		, m_bInstance(false)
		, m_StaticCollision(false)
	{
	}

	~MeshComponent(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
		ar & BOOST_SERIALIZATION_NVP(m_MeshRes);
		ar & BOOST_SERIALIZATION_NVP(m_bAnimation);
		ar & BOOST_SERIALIZATION_NVP(m_bInstance);
		ar & BOOST_SERIALIZATION_NVP(m_MaterialList);
		ar & BOOST_SERIALIZATION_NVP(m_StaticCollision);
	}

	void AddMaterial(MaterialPtr mat)
	{
		m_MaterialList.push_back(mat);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class EmitterComponent
	: public RenderComponent
{
public:
	my::EmitterPtr m_Emitter;

	MaterialPtr m_Material;

public:
	EmitterComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: RenderComponent(ComponentTypeEmitter, Position, Rotation, Scale)
	{
	}

	EmitterComponent(void)
		: RenderComponent(ComponentTypeEmitter)
	{
	}

	~EmitterComponent(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
		ar & BOOST_SERIALIZATION_NVP(m_Emitter);
		ar & BOOST_SERIALIZATION_NVP(m_Material);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class SphericalEmitterComponent
	: public RenderComponent
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

	my::EmitterPtr m_Emitter;

	MaterialPtr m_Material;

public:
	SphericalEmitterComponent(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: RenderComponent(ComponentTypeSphericalEmitter, Position, Rotation, Scale)
		, m_ParticleLifeTime(FLT_MAX)
		, m_RemainingSpawnTime(0)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnLoopTime(5)
	{
	}

	SphericalEmitterComponent(void)
		: RenderComponent(ComponentTypeSphericalEmitter)
		, m_ParticleLifeTime(FLT_MAX)
		, m_RemainingSpawnTime(0)
		, m_SpawnInterval(FLT_MAX)
		, m_HalfSpawnArea(0,0,0)
		, m_SpawnSpeed(0)
		, m_SpawnLoopTime(5)
	{
	}

	~SphericalEmitterComponent(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
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
		ar & BOOST_SERIALIZATION_NVP(m_Material);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<SphericalEmitterComponent> SphericalEmitterComponentPtr;
//
//class RigidComponent
//	: public Component
//{
//public:
//	// ! deallocate the memory block when the objects within have been released by PhysX
//	boost::shared_ptr<unsigned char> m_SerializeBuff;
//
//	PhysXPtr<PxRigidActor> m_RigidActor;
//
//	enum SerializeRef
//	{
//		SerializeRefNone = 0,
//		SerializeRefMaterial,
//		SerializeRefActor,
//	};
//
//	void CreateRigidActor(const my::Matrix4 & World);
//
//public:
//	RigidComponent(const my::Matrix4 & Local)
//		: Component(ComponentTypeRigid, World)
//	{
//		CreateRigidActor(World);
//	}
//
//	RigidComponent(void)
//		: Component(ComponentTypeRigid, my::AABB::Invalid(), my::Matrix4::Identity())
//	{
//		// ! create rigid actor from serialize
//	}
//
//	~RigidComponent(void)
//	{
//		if (IsRequested())
//		{
//			ReleaseResource();
//		}
//		m_RigidActor.reset();
//		m_SerializeBuff.reset();
//	}
//
//	friend class boost::serialization::access;
//
//	template<class Archive>
//	void save(Archive & ar, const unsigned int version) const;
//
//	template<class Archive>
//	void load(Archive & ar, const unsigned int version);
//
//	template<class Archive>
//	void serialize(Archive & ar, const unsigned int version)
//	{
//		boost::serialization::split_member(ar, *this, version);
//	}
//
//	virtual void RequestResource(void);
//
//	virtual void ReleaseResource(void);
//};
//
//typedef boost::shared_ptr<RigidComponent> RigidComponentPtr;

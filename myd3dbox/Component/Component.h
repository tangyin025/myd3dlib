#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"
#include <boost/serialization/nvp.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

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
	: public boost::enable_shared_from_this<Component>
{
public:
	enum ComponentType
	{
		ComponentTypeUnknown,
		ComponentTypeActor,
		ComponentTypeMesh,
		ComponentTypeEmitter,
		//ComponentTypeRigid,
		ComponentTypeTerrain,
	};

	ComponentType m_Type;

	enum DirtyFlag
	{
		DirtyFlagWorld = 0x01,
	};

	unsigned int m_DirtyFlag;

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	AnimatorPtr m_Animator;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	Component * m_Parent;

public:
	Component(ComponentType Type, const my::AABB & aabb, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: m_Type(Type)
		, m_DirtyFlag(0)
		, m_aabb(aabb)
		, m_Position(Position)
		, m_Rotation(Rotation)
		, m_Scale(Scale)
		, m_World(my::Matrix4::Identity())
		, m_Parent(NULL)
	{
	}

	Component(void)
		: m_Type(ComponentTypeUnknown)
		, m_DirtyFlag(0)
		, m_aabb(-1,1)
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

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Type);
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Position);
		ar & BOOST_SERIALIZATION_NVP(m_Rotation);
		ar & BOOST_SERIALIZATION_NVP(m_Scale);
		ar & BOOST_SERIALIZATION_NVP(m_Animator);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(ComponentPtr cmp);

	Animator * GetHierarchyAnimator(void);

	//static const my::AABB & GetCmpOctAABB(const Component * cmp);

	//static my::Matrix4 GetCmpWorld(const Component * cmp);

	//static void SetCmpWorld(Component * cmp, const my::Matrix4 & Local);
};

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(ComponentType Type, const my::AABB & aabb, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: Component(Type, aabb, Position, Rotation, Scale)
	{
	}

	RenderComponent(ComponentType Type)
		: Component(Type, my::AABB(-1,1), my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
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
	MeshComponent(const my::AABB & aabb, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, bool bAnimation, bool bInstance)
		: RenderComponent(ComponentTypeMesh, aabb, Position, Rotation, Scale)
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
	EmitterComponent(const my::AABB & aabb, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: RenderComponent(ComponentTypeEmitter, aabb, Position, Rotation, Scale)
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
//	RigidComponent(const my::AABB & aabb, const my::Matrix4 & Local)
//		: Component(ComponentTypeRigid, aabb, World)
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

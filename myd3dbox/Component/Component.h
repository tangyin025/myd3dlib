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

class Component
{
public:
	enum ComponentType
	{
		ComponentTypeUnknown,
		ComponentTypeMesh,
		ComponentTypeEmitter,
		//ComponentTypeRigid,
		ComponentTypeTerrain,
	};

	ComponentType m_Type;

	my::AABB m_aabb;

	my::Matrix4 m_World;

	bool m_Requested;

public:
	Component(ComponentType Type, const my::AABB & aabb, const my::Matrix4 & World)
		: m_Type(Type)
		, m_aabb(aabb)
		, m_World(World)
		, m_Requested(false)
	{
	}

	Component(void)
		: m_Type(ComponentTypeUnknown)
		, m_aabb(my::AABB::Invalid())
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
	{
	}

	virtual ~Component(void)
	{
		//if (m_OctNode)
		//{
		//	m_OctNode->RemoveComponent(this);
		//}
		// ! Derived class must ReleaseResource menually
		_ASSERT(!IsRequested());
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Type);
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_World);
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	virtual void RequestResource(void)
	{
		_ASSERT(!IsRequested());
		m_Requested = true;
	}

	virtual void ReleaseResource(void)
	{
		_ASSERT(IsRequested());
		m_Requested = false;
	}

	virtual void Update(float fElapsedTime)
	{
	}

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
	{
	}

	static const my::AABB & GetCmpOctAABB(const Component * cmp);

	static my::Matrix4 GetCmpWorld(const Component * cmp);

	static void SetCmpWorld(Component * cmp, const my::Matrix4 & World);
};

typedef boost::shared_ptr<Component> ComponentPtr;

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(ComponentType Type, const my::AABB & aabb, const my::Matrix4 & World)
		: Component(Type, aabb, World)
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

	bool m_bInstance;

	MaterialPtrList m_MaterialList;

	AnimatorPtr m_Animator;

	bool m_StaticCollision;

public:
	MeshComponent(const my::AABB & aabb, const my::Matrix4 & World, bool bInstance)
		: RenderComponent(ComponentTypeMesh, aabb, World)
		, m_StaticCollision(false)
	{
	}

	MeshComponent(void)
		: RenderComponent(ComponentTypeMesh, my::AABB::Invalid(), my::Matrix4::Identity())
		, m_StaticCollision(false)
	{
	}

	~MeshComponent(void)
	{
		if (IsRequested())
		{
			ReleaseResource();
		}
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
	EmitterComponent(const my::AABB & aabb, const my::Matrix4 & World)
		: RenderComponent(ComponentTypeEmitter, aabb, World)
	{
	}

	EmitterComponent(void)
		: RenderComponent(ComponentTypeEmitter, my::AABB::Invalid(), my::Matrix4::Identity())
	{
	}

	~EmitterComponent(void)
	{
		if (IsRequested())
		{
			ReleaseResource();
		}
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
//	RigidComponent(const my::AABB & aabb, const my::Matrix4 & World)
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

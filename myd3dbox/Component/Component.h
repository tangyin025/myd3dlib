#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"
#include "PhysXPtr.h"
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
	: public my::OctComponent
{
public:
	enum ComponentType
	{
		ComponentTypeUnknown,
		ComponentTypeMesh,
		ComponentTypeEmitter,
		ComponentTypeRigid,
		ComponentTypeTerrain,
	};

	ComponentType m_Type;

	my::AABB m_aabb;

	bool m_Requested;

public:
	Component(const my::AABB & aabb, ComponentType Type)
		: m_Type(Type)
		, m_aabb(aabb)
		, m_Requested(false)
	{
	}

	Component(void)
		: m_Type(ComponentTypeUnknown)
		, m_aabb(my::AABB(-FLT_MAX,FLT_MAX))
		, m_Requested(false)
	{
	}

	virtual ~Component(void)
	{
		if (m_OctNode)
		{
			m_OctNode->RemoveComponent(this);
		}
		// ! Derived class must ReleaseResource menually
		_ASSERT(!IsRequested());
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Type);
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
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

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
	{
	}

	static const my::AABB & GetComponentAABB(const Component * cmp);

	static const my::AABB & GetComponentOctAABB(const Component * cmp);

	static my::Matrix4 GetComponentWorld(const Component * cmp);

	static void SetComponentWorld(Component * cmp, const my::Matrix4 & World);
};

typedef boost::shared_ptr<Component> ComponentPtr;

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb, ComponentType Type)
		: Component(aabb, Type)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask) = 0;
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	my::Matrix4 m_World;

	struct LOD
	{
		ResourceBundle<my::Mesh> m_MeshRes;

		bool m_bInstance;

		float m_MaxDistance;

		LOD(const char * Path, bool bInstance, float MaxDistance)
			: m_MeshRes(Path)
			, m_bInstance(bInstance)
			, m_MaxDistance(MaxDistance)
		{
		}

		LOD(void)
			: m_bInstance(false)
			, m_MaxDistance(3000.0f)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_MeshRes);
			ar & BOOST_SERIALIZATION_NVP(m_bInstance);
			ar & BOOST_SERIALIZATION_NVP(m_MaxDistance);
		}
	};

	typedef std::vector<LOD> LODList;

	LODList m_lods;

	unsigned int m_lod;

	MaterialPtrList m_MaterialList;

	AnimatorPtr m_Animator;

public:
	MeshComponent(const my::AABB & aabb, const my::Matrix4 & World, bool bInstance)
		: RenderComponent(aabb, ComponentTypeMesh)
		, m_World(World)
		, m_lod(0)
	{
	}

	MeshComponent(void)
		: RenderComponent(my::AABB(-FLT_MAX,FLT_MAX), ComponentTypeMesh)
		, m_World(my::Matrix4::Identity())
		, m_lod(0)
	{
	}

	~MeshComponent(void)
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
		ar & BOOST_SERIALIZATION_NVP(m_World);
		ar & BOOST_SERIALIZATION_NVP(m_lods);
		ar & BOOST_SERIALIZATION_NVP(m_MaterialList);
		ar & BOOST_SERIALIZATION_NVP(m_Animator);
	}

	void AddMaterial(MaterialPtr mat)
	{
		m_MaterialList.push_back(mat);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class EmitterComponent
	: public RenderComponent
{
public:
	my::Matrix4 m_World;

	my::EmitterPtr m_Emitter;

	MaterialPtr m_Material;

public:
	EmitterComponent(const my::AABB & aabb, const my::Matrix4 & World)
		: RenderComponent(aabb, ComponentTypeEmitter)
		, m_World(World)
	{
	}

	EmitterComponent(void)
		: RenderComponent(my::AABB(-FLT_MAX,FLT_MAX), ComponentTypeEmitter)
		, m_World(my::Matrix4::Identity())
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
		ar & BOOST_SERIALIZATION_NVP(m_World);
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

class RigidComponent
	: public Component
{
public:
	// ! deallocate the memory block when the objects within have been released by PhysX
	boost::shared_ptr<unsigned char> m_SerializeBuff;

	PhysXPtr<PxRigidActor> m_RigidActor;

	enum SerializeRef
	{
		SerializeRefNone = 0,
		SerializeRefMaterial,
		SerializeRefActor,
	};

	void CreateRigidActor(const my::Matrix4 & World);

public:
	RigidComponent(const my::AABB & aabb, const my::Matrix4 & World)
		: Component(aabb, ComponentTypeRigid)
	{
		CreateRigidActor(World);
	}

	RigidComponent(void)
		: Component(my::AABB(-FLT_MAX,FLT_MAX), ComponentTypeRigid)
	{
		// ! create rigid actor from serialize
	}

	~RigidComponent(void)
	{
		if (IsRequested())
		{
			ReleaseResource();
		}
		m_RigidActor.reset();
		m_SerializeBuff.reset();
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
};

typedef boost::shared_ptr<RigidComponent> RigidComponentPtr;

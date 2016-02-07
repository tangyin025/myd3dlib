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

	ResourceBundle<my::BaseTexture> m_MeshTexture;

	ResourceBundle<my::BaseTexture> m_NormalTexture;

	ResourceBundle<my::BaseTexture> m_SpecularTexture;

public:
	Material(void);

	virtual ~Material(void);

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Shader);
		ar & BOOST_SERIALIZATION_NVP(m_PassMask);
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
	};

	ComponentType m_Type;

	my::AABB m_aabb;

	my::Matrix4 m_World;

	bool m_Requested;

public:
	Component(const my::AABB & aabb, const my::Matrix4 & World, ComponentType Type)
		: m_Type(Type)
		, m_aabb(aabb)
		, m_World(World)
		, m_Requested(false)
	{
	}

	Component(void)
		: m_Type(ComponentTypeUnknown)
		, m_aabb(my::AABB(-FLT_MAX,FLT_MAX))
		, m_World(my::Matrix4::Identity())
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

	const my::AABB & GetOctAABB(void) const;

	const my::AABB & GetComponentAABB(void) const;
};

typedef boost::shared_ptr<Component> ComponentPtr;

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb, const my::Matrix4 & World, ComponentType Type)
		: Component(aabb, World, Type)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId) = 0;

	virtual void AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask) = 0;
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	ResourceBundle<my::Mesh> m_MeshRes;

	MaterialPtrList m_MaterialList;

	bool m_bInstance;

	AnimatorPtr m_Animator;

public:
	MeshComponent(const my::AABB & aabb, const my::Matrix4 & World, bool bInstance)
		: RenderComponent(aabb, World, ComponentTypeMesh)
		, m_bInstance(bInstance)
	{
	}

	MeshComponent(void)
		: RenderComponent(my::AABB(-FLT_MAX,FLT_MAX), my::Matrix4::Identity(), ComponentTypeMesh)
		, m_bInstance(false)
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
		ar & BOOST_SERIALIZATION_NVP(m_MeshRes);
		ar & BOOST_SERIALIZATION_NVP(m_MaterialList);
		ar & BOOST_SERIALIZATION_NVP(m_bInstance);
		ar & BOOST_SERIALIZATION_NVP(m_Animator);
	}

	void AddMaterial(MaterialPtr mat)
	{
		m_MaterialList.push_back(mat);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask);
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
		: RenderComponent(aabb, World, ComponentTypeEmitter)
	{
	}

	EmitterComponent(void)
		: RenderComponent(my::AABB(-FLT_MAX,FLT_MAX), my::Matrix4::Identity(), ComponentTypeEmitter)
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

	virtual void AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

class RigidComponent
	: public Component
{
public:
	typedef boost::shared_ptr<physx::PxGeometry> PxGeometryPtr;

	typedef std::pair<PxGeometryPtr, physx::PxTransform> PxGeometryPtrPair;

	typedef std::vector<PxGeometryPtrPair> PxGeometryPtrPairList;

	PxGeometryPtrPairList m_GeometryList;

	PhysXPtr<physx::PxRigidActor> m_RigidActor;

public:
	RigidComponent(const my::AABB & aabb, const my::Matrix4 & World)
		: Component(aabb, World, ComponentTypeRigid)
	{
	}

	RigidComponent(void)
		: Component(my::AABB(-FLT_MAX,FLT_MAX), my::Matrix4::Identity(), ComponentTypeRigid)
	{
	}

	~RigidComponent(void)
	{
		if (IsRequested())
		{
			ReleaseResource();
		}
	}

	friend class boost::serialization::access;
	
	template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
		ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		unsigned int Count = m_GeometryList.size();
		ar << BOOST_SERIALIZATION_NVP(Count);
		for (unsigned int i = 0; i < m_GeometryList.size(); i++)
		{
			const PxGeometryPtrPair & pair = m_GeometryList[i];
			physx::PxGeometryType::Enum type = pair.first->getType();
			ar << BOOST_SERIALIZATION_NVP(type);
			switch (type)
			{
			case physx::PxGeometryType::eSPHERE:
				{
					physx::PxSphereGeometry * sphere = static_cast<physx::PxSphereGeometry *>(pair.first.get());
					ar << boost::serialization::make_nvp("radius", sphere->radius);
				}
				break;
			case physx::PxGeometryType::ePLANE:
				{
					physx::PxPlaneGeometry * plane = static_cast<physx::PxPlaneGeometry *>(pair.first.get());
				}
				break;
			case physx::PxGeometryType::eCAPSULE:
				{
					physx::PxCapsuleGeometry * capsule = static_cast<physx::PxCapsuleGeometry *>(pair.first.get());
					ar << boost::serialization::make_nvp("radius", capsule->radius);
					ar << boost::serialization::make_nvp("halfHeight", capsule->halfHeight);
				}
				break;
			case physx::PxGeometryType::eBOX:
				{
					physx::PxBoxGeometry * box = static_cast<physx::PxBoxGeometry *>(pair.first.get());
					ar << boost::serialization::make_nvp("halfExtents", (my::Vector3&)box->halfExtents);
				}
				break;
			}
			ar << boost::serialization::make_nvp("q", (my::Quaternion&)pair.second.q);
			ar << boost::serialization::make_nvp("p", (my::Vector3&)pair.second.p);
		}
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
		ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
		unsigned int Count;
		ar >> BOOST_SERIALIZATION_NVP(Count); m_GeometryList.resize(Count);
		for (unsigned int i = 0; i < m_GeometryList.size(); i++)
		{
			PxGeometryPtrPair & pair = m_GeometryList[i];
			physx::PxGeometryType::Enum type;
			ar >> BOOST_SERIALIZATION_NVP(type);
			switch (type)
			{
			case physx::PxGeometryType::eSPHERE:
				{
					physx::PxSphereGeometry * sphere = new physx::PxSphereGeometry; pair.first.reset(sphere);
					ar >> boost::serialization::make_nvp("radius", sphere->radius);
				}
				break;
			case physx::PxGeometryType::ePLANE:
				{
					physx::PxPlaneGeometry * plane = new physx::PxPlaneGeometry; pair.first.reset(plane);
				}
				break;
			case physx::PxGeometryType::eCAPSULE:
				{
					physx::PxCapsuleGeometry * capsule = new physx::PxCapsuleGeometry; pair.first.reset(capsule);
					ar >> boost::serialization::make_nvp("radius", capsule->radius);
					ar >> boost::serialization::make_nvp("halfHeight", capsule->halfHeight);
				}
				break;
			case physx::PxGeometryType::eBOX:
				{
					physx::PxBoxGeometry * box = new physx::PxBoxGeometry; pair.first.reset(box);
					ar >> boost::serialization::make_nvp("halfExtents", (my::Vector3 &)box->halfExtents);
				}
				break;
			}
			ar >> boost::serialization::make_nvp("q", (my::Quaternion&)pair.second.q);
			ar >> boost::serialization::make_nvp("p", (my::Vector3&)pair.second.p);
		}
    }

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);
};

typedef boost::shared_ptr<RigidComponent> RigidComponentPtr;

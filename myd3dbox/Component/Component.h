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

	~MeshComponent()
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

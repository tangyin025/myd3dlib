#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"

#include <boost/serialization/nvp.hpp>

class Material
{
public:
	class ParameterValue
	{
	public:
		enum ParameterValueType
		{
			ParameterValueTypeUnknown,
			ParameterValueTypeTexture,
		};

		ParameterValueType m_Type;

	public:
		ParameterValue(ParameterValueType type)
			: m_Type(type)
		{
		}

		virtual ~ParameterValue(void)
		{
		}

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId, const char * name) = 0;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_Type);
		}
	};

	typedef boost::shared_ptr<ParameterValue> ParameterValuePtr;

	class ParameterValueTexture : public ParameterValue
	{
	public:
		std::string m_TexturePath;

		my::BaseTexturePtr m_Texture;

	public:
		ParameterValueTexture(void)
			: ParameterValue(ParameterValueTypeTexture)
		{
		}

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId, const char * name);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ParameterValue);
			ar & BOOST_SERIALIZATION_NVP(m_TexturePath);
		}
	};

	typedef boost::shared_ptr<ParameterValueTexture> ParameterValueTexturePtr;

	class Parameter : public std::pair<std::string, boost::shared_ptr<ParameterValue> >
	{
	public:
		Parameter(void)
		{
		}

		Parameter(const std::string & name, ParameterValuePtr value)
			: pair(name, value)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(first);
			ar & BOOST_SERIALIZATION_NVP(second);
		}
	};

	typedef std::vector<Parameter> ParameterList;

	std::string m_Shader;

	ParameterList m_Params;

	unsigned int m_PassMask;

public:
	Material(void);

	virtual ~Material(void);

	void AddParameter(const std::string & name, ParameterValuePtr value)
	{
		m_Params.push_back(Parameter(name, value));
	}

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Shader);
		ar & BOOST_SERIALIZATION_NVP(m_Params);
		ar & BOOST_SERIALIZATION_NVP(m_PassMask);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<Material> MaterialPtr;

typedef std::vector<MaterialPtr> MaterialPtrList;

class ComponentContext
	: public my::SingleInstance<ComponentContext>
	, public my::OctRoot
{
public:
	ComponentContext(float minx, float miny, float minz, float maxx, float maxy, float maxz, float MinBlock)
		: OctRoot(minx, miny, minz, maxx, maxy, maxz, MinBlock)
	{
	}

	ComponentContext(const my::Vector3 & _Min, const my::Vector3 & _Max, float MinBlock)
		: OctRoot(_Min, _Max, MinBlock)
	{
	}

	ComponentContext(const my::AABB & aabb, float MinBlock)
		: OctRoot(aabb, MinBlock)
	{
	}
};

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

	my::Matrix4 m_World;

public:
	Component(const my::AABB & aabb, const my::Matrix4 & World, ComponentType Type)
		: OctComponent(aabb)
		, m_Type(Type)
		, m_World(World)
	{
		ComponentContext::getSingleton().AddComponent(this, 0.1f);
	}

	virtual ~Component(void)
	{
		if (m_OctNode)
		{
			m_OctNode->RemoveComponent(this);
		}
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Type);
		ar & BOOST_SERIALIZATION_NVP(m_World);
	}

	virtual void Update(float fElapsedTime)
	{
	}

	virtual my::RayResult RayTest(const my::Ray & ray) const
	{
		return my::RayResult(false, FLT_MAX);
	}

	virtual bool FrustumTest(const my::Frustum & frustum) const
	{
		return false;
	}
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

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	std::string m_MeshPath;

	my::MeshPtr m_Mesh;

	my::D3DVertexElementSet m_VertexElems;

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
		: RenderComponent(my::AABB(-1,1), my::Matrix4::Identity(), ComponentTypeMesh)
		, m_bInstance(false)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
		ar & BOOST_SERIALIZATION_NVP(m_MeshPath);
		ar & BOOST_SERIALIZATION_NVP(m_MaterialList);
		ar & BOOST_SERIALIZATION_NVP(m_bInstance);
	}

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual my::RayResult RayTest(const my::Ray & ray) const;

	virtual bool FrustumTest(const my::Frustum & frustum) const;
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
		: RenderComponent(my::AABB(-1,1), my::Matrix4::Identity(), ComponentTypeEmitter)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
		ar & BOOST_SERIALIZATION_NVP(m_Emitter);
		ar & BOOST_SERIALIZATION_NVP(m_Material);
	}

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

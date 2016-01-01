#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"
#include <boost/serialization/nvp.hpp>

template <class T>
class ResourceBundle : public my::IResourceCallback
{
public:
	bool m_Ready;

	std::string m_ResPath;

	boost::shared_ptr<T> m_Res;

public:
	ResourceBundle(const char * Path)
		: m_Ready(false)
		, m_ResPath(Path)
	{
	}

	ResourceBundle(void)
		: m_Ready(false)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_ResPath);
	}

	virtual void OnReady(my::DeviceRelatedObjectBasePtr res)
	{
		m_Res = boost::dynamic_pointer_cast<T>(res);
		m_Ready = true;
	}

	bool IsReady(void) const
	{
		return m_Ready;
	}

	void RequestResource(void);

	void ReleaseResource(void)
	{
		if (IsRequested())
		{
			my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_ResPath, this);
		}
	}
};

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

	class ParameterValueTexture : public ParameterValue, public ResourceBundle<my::BaseTexture>
	{
	public:
		ParameterValueTexture(const char * Path)
			: ParameterValue(ParameterValueTypeTexture)
			, ResourceBundle(Path)
		{
		}

		ParameterValueTexture(void)
			: ParameterValue(ParameterValueTypeTexture)
		{
		}

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId, const char * name);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ParameterValue);
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResourceBundle);
		}
	};

	typedef boost::shared_ptr<ParameterValueTexture> ParameterValueTexturePtr;

	class Parameter : public std::pair<std::string, boost::shared_ptr<ParameterValue> >
	{
	public:
		Parameter(const char * name, ParameterValuePtr value)
			: pair(name, value)
		{
		}

		Parameter(void)
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

	ParameterList m_Params;

	std::string m_Shader;

	unsigned int m_PassMask;

public:
	Material(void);

	virtual ~Material(void);

	void AddParameter(const char * name, ParameterValuePtr value)
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

	void RequestResource(void);

	void ReleaseResource(void);
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

	bool m_Requested;

public:
	Component(const my::AABB & aabb, const my::Matrix4 & World, ComponentType Type)
		: OctComponent(aabb)
		, m_Type(Type)
		, m_World(World)
		, m_Requested(false)
	{
		ComponentContext::getSingleton().AddComponent(this, 0.1f);
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
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Type);
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
	struct LOD
	{
		ResourceBundle<my::Mesh> m_MeshRes;

		MaterialPtrList m_MaterialList;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_MeshRes);
			ar & BOOST_SERIALIZATION_NVP(m_MaterialList);
		}
	};

	typedef std::vector<LOD> LODList;

	LODList m_Lods;

	unsigned int m_LodId;

	bool m_bInstance;

	AnimatorPtr m_Animator;

public:
	MeshComponent(const my::AABB & aabb, const my::Matrix4 & World, bool bInstance)
		: RenderComponent(aabb, World, ComponentTypeMesh)
		, m_LodId(0)
		, m_bInstance(bInstance)
	{
	}

	MeshComponent(void)
		: RenderComponent(my::AABB(-1,1), my::Matrix4::Identity(), ComponentTypeMesh)
		, m_LodId(0)
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
		ar & BOOST_SERIALIZATION_NVP(m_Lods);
		ar & BOOST_SERIALIZATION_NVP(m_bInstance);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

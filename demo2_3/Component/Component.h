#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"

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

	const ComponentType m_Type;

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

	virtual void OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	my::MeshPtr m_Mesh;

	my::D3DVertexElementSet m_VertexElems;

	MaterialPtrList m_MaterialList;

	bool m_bInstance;

	AnimatorPtr m_Animator;

public:
	MeshComponent(const my::AABB & aabb, const my::Matrix4 & World)
		: RenderComponent(aabb, World, ComponentTypeMesh)
		, m_bInstance(false)
	{
	}

	virtual void OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual my::RayResult RayTest(const my::Ray & ray) const;

	virtual bool FrustumTest(const my::Frustum & frustum) const;
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class IndexdPrimitiveUPComponent
	: public my::DeviceRelatedObjectBase
	, public RenderComponent
{
public:
	std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::Cache m_VertexData;

	DWORD m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	MaterialPtrList m_MaterialList;

	AnimatorPtr m_Animator;

public:
	IndexdPrimitiveUPComponent(const my::AABB & aabb, const my::Matrix4 & World, ComponentType Type)
		: RenderComponent(aabb, World, Type)
		, m_VertexStride(0)
	{
	}

	virtual void OnResetDevice(void);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	virtual void OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<IndexdPrimitiveUPComponent> IndexdPrimitiveUPComponentPtr;

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

	virtual void Update(float fElapsedTime);

	virtual void OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;

#pragma once

#include "Component.h"

class TerrainNode;

typedef boost::shared_ptr<TerrainNode> TerrainNodePtr;

class TerrainNode
{
public:
	my::AABB m_aabb;

	typedef boost::array<TerrainNodePtr, 4> ChildArray;

	ChildArray m_Childs;

public:
	TerrainNode(const my::AABB & aabb);

	bool OnQuery(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void QueryAll(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void Query(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

class Terrain2 : public Component, TerrainNode
{
public:
	Terrain2(void)
		: Component(ComponentTypeTerrain2)
		, TerrainNode(my::AABB(-1000,1000))
	{
	}

	virtual ~Terrain2(void)
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

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void OnShaderChanged(void);

	virtual void Update(float fElapsedTime);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain2> Terrain2Ptr;

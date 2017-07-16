#pragma once

class Actor;
class RenderPipeline;
class WorldL;
class PhysXSceneContext;

class Octree : public my::OctNode<0>
{
public:
	WorldL * m_World;

public:
	Octree(void)
		: m_World(NULL)
	{
	}

	Octree(WorldL * world, const my::AABB & aabb, float MinBlock)
		: m_World(world)
		, OctNode(NULL, aabb, MinBlock)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("OctNode0", boost::serialization::base_object< my::OctNode<0> >(*this));
	}
};

class WorldL
{
public:
	static const int LEVEL_SIZE = 16;

	static const int LEVEL_EDGE = 1;

	struct QueryCallback
	{
	public:
		virtual void operator() (Octree * level, const CPoint & level_id) = 0;
	};

	long m_Dimension;

	CPoint m_LevelId;

	typedef std::vector<Octree> OctreeList;

	OctreeList m_levels;

	typedef boost::unordered_set<Actor *> OctActorSet;

	OctActorSet m_ViewedActors;

public:
	WorldL(void)
		: m_Dimension(0)
		, m_LevelId(0,0)
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

	Octree * GetLevel(const CPoint & level_id)
	{
		_ASSERT(level_id.x >= 0 && level_id.x < m_Dimension);
		_ASSERT(level_id.y >= 0 && level_id.y < m_Dimension);
		return &m_levels[level_id.y * m_Dimension + level_id.x];
	}

	CPoint GetLevelId(const Octree * level_ptr)
	{
		int i = level_ptr - &m_levels[0];
		_ASSERT(i >= 0 && i < m_Dimension * m_Dimension);
		return CPoint(i % m_Dimension, i / m_Dimension);
	}

	void CreateLevels(long dimension);

	void ClearAllLevels(void);

	void QueryLevel(const CPoint & level_id, QueryCallback * callback);

	void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void ResetViewedActors(const my::Vector3 & ViewPos, PhysXSceneContext * scene, float ViewDist, float ViewThreshold);

	my::Matrix4 CalculateActorParentWorld(Actor * actor);

	void UpdateViewedActorsWorld(void);

	void ResetLevelId(const CPoint & level_id, PhysXSceneContext * scene);

	bool ResetLevelId(my::Vector3 & ViewPos, PhysXSceneContext * scene);
};

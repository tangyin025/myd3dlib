#pragma once

class Actor;
class RenderPipeline;
class WorldL;

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
	int m_Dimension;

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

	Octree & GetLevel(const CPoint & level_id)
	{
		return m_levels[level_id.y * m_Dimension + level_id.x];
	}

	CPoint GetLevelId(const Octree * level_ptr)
	{
		int dis = (level_ptr - &m_levels[0]) / sizeof(OctreeList::value_type);
		if (dis < m_Dimension * m_Dimension)
		{
			return CPoint(dis % m_Dimension, dis / m_Dimension);
		}
		return CPoint(0, 0);
	}

	void ChangeLevelId(const CPoint & new_id);

	void CreateLevels(int dimension);

	void ClearAllLevels(void);

	void _QueryRenderComponent(const CPoint & level_id, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void _ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewPos, physx::PxScene * scene);

	void ResetViewedActors(const my::Vector3 & ViewPos, physx::PxScene * scene);
};

#pragma once

class Actor;
class RenderPipeline;
class WorldL;
class PhysXSceneContext;

class OctLevel : public my::OctRoot
{
public:
	WorldL * m_World;

public:
	OctLevel(WorldL * world, const my::AABB & aabb, float MinBlock)
		: m_World(world)
		, OctRoot(aabb, MinBlock)
	{
	}

	OctLevel(void)
		: m_World(NULL)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	}

	CPoint GetId(void) const;

	my::Vector3 GetOffset(void) const;
};

typedef boost::shared_ptr<OctLevel> OctreePtr;

class WorldL
{
public:
	static const int LEVEL_SIZE = 16;

	static const int LEVEL_EDGE = 1;

	struct QueryCallback
	{
	public:
		virtual void operator() (OctLevel * level, const CPoint & level_id) = 0;
	};

	long m_Dimension;

	CPoint m_LevelId;

	typedef std::vector<OctLevel> OctreeList;

	OctreeList m_levels;

	typedef boost::unordered_set<Actor *> OctActorSet;

	OctActorSet m_ViewedActors;

public:
	WorldL(void)
		: m_Dimension(0)
		, m_LevelId(0,0)
	{
	}

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	OctLevel * GetLevel(const CPoint & level_id)
	{
		_ASSERT(level_id.x >= 0 && level_id.x < m_Dimension);
		_ASSERT(level_id.y >= 0 && level_id.y < m_Dimension);
		return &m_levels[level_id.y * m_Dimension + level_id.x];
	}

	CPoint GetLevelId(const OctLevel * level_ptr)
	{
		int i = level_ptr - &m_levels[0];
		_ASSERT(i >= 0 && i < m_Dimension * m_Dimension);
		return CPoint(i % m_Dimension, i / m_Dimension);
	}

	static my::Vector3 CalculateOffset(const CPoint & level0, const CPoint & level1)
	{
		return my::Vector3((float)(level1.x - level0.x) * LEVEL_SIZE, 0, (float)(level1.y - level0.y) * LEVEL_SIZE);
	}

	void CreateLevels(long dimension);

	void ClearAllLevels(void);

	void QueryLevel(const CPoint & level_id, QueryCallback * callback);

	void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void ResetViewedActors(const my::Vector3 & ViewPos, PhysXSceneContext * scene, float ViewDist, float ViewThreshold);

	void AdjustLevelIdAndPosition(CPoint & level_id, my::Vector3 & pos);

	void OnActorPoseChanged(Actor * actor, CPoint level_id);

	void ResetLevelId(const CPoint & level_id, PhysXSceneContext * scene);

	bool ResetLevelId(my::Vector3 & ViewPos, PhysXSceneContext * scene);
};

inline CPoint OctLevel::GetId(void) const
{
	return m_World->GetLevelId(this);
}

inline my::Vector3 OctLevel::GetOffset(void) const
{
	return WorldL::CalculateOffset(m_World->m_LevelId, GetId());
}

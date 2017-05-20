#pragma once

class Actor;
class RenderPipeline;

class OctTree : public my::OctNode<0>
{
protected:
	OctTree(void)
	{
	}

public:
	OctTree(const my::AABB & aabb, float MinBlock)
		: OctNode(NULL, aabb, MinBlock)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::make_nvp("OctNode0", boost::serialization::base_object< my::OctNode<0> >(*this));
	}
};

class WorldL
{
public:
	int m_Dim;

	CPoint m_LevelId;

	typedef std::vector<OctTree> OctTreeList;

	OctTreeList m_levels;

	typedef boost::unordered_set<Actor *> OctActorSet;

	OctActorSet m_ViewedActors;

public:
	WorldL(void)
		: m_Dim(0)
		, m_LevelId(0,0)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		BOOST_SERIALIZATION_NVP(m_Dim);
		BOOST_SERIALIZATION_NVP(m_levels);
	}

	OctTree & GetLevel(const CPoint & level_id)
	{
		return m_levels[level_id.y * m_Dim + level_id.x];
	}

	void CreateLevels(int x, int y);

	void ClearAllLevels(void);

	void _QueryRenderComponent(const CPoint & level_id, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void _ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewPos);

	void ResetViewedActors(const my::Vector3 & ViewPos);
};

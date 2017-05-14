#pragma once

class Actor;
class RenderPipeline;

class WorldL
{
public:
	int m_Dim;

	typedef std::vector<my::OctTree> OctTreeList;

	OctTreeList m_levels;

	typedef boost::unordered_set<Actor *> OctActorSet;

	OctActorSet m_ViewedActors;

public:
	WorldL(int Dim)
		: m_Dim(Dim)
		, m_levels(m_Dim * m_Dim, my::OctTree(my::AABB(0, 512), 0.1f))
	{
	}

	WorldL(void)
		: m_Dim(0)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		BOOST_SERIALIZATION_NVP(m_Dim);
		BOOST_SERIALIZATION_NVP(m_levels);
	}

	void _QueryRenderComponent(const CPoint & level_id, const CPoint & offset, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void QueryRenderComponent(const CPoint & level_id, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	void _ResetViewedActors(const CPoint & level_id, const CPoint & offset, const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);

	void ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);
};

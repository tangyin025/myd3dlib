#pragma once

class World
{
public:
	unsigned int m_Dim;

	typedef std::vector<my::OctTree> OctTreeList;

	OctTreeList m_levels;

public:
	World(unsigned int Dim)
		: m_Dim(Dim)
		, m_levels(m_Dim * m_Dim, my::OctTree(my::AABB(0, 512), 0.1f))
	{
	}

	World(void)
		: m_Dim(0)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		BOOST_SERIALIZATION_NVP(m_Dim);
		BOOST_SERIALIZATION_NVP(m_levels);
	}
};

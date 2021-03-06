#pragma once

#include "Component.h"

class dtNavMesh;

class dtNavMeshQuery;

class Navigation : public Component
{
public:
	boost::shared_ptr<dtNavMesh> m_navMesh;

	boost::shared_ptr<dtNavMeshQuery> m_navQuery;

protected:
	Navigation(void);

public:
	Navigation(const char* Name);

	virtual ~Navigation(void);

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive& ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void CopyFrom(const Component& rhs);

	virtual ComponentPtr Clone(void) const;
};

typedef boost::shared_ptr<Navigation> NavigationPtr;

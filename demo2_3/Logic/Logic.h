#pragma once

#include "Character.h"
#include "../myd3dbox/Component/Component.h"

class Logic
{
public:
	typedef std::vector<ComponentPtr> ComponentPtrList;
	
	ComponentPtrList m_cmps;

	LocalPlayerPtr m_player;

public:
	Logic(void);

	virtual ~Logic(void);

	void Create(void);

	void Update(float fElapsedTime);

	void Destroy(void);
};

typedef boost::shared_ptr<Logic> LogicPtr;

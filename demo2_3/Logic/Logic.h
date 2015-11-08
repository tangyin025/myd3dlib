#pragma once

#include "mySingleton.h"
#include "Character.h"

class Logic : public my::SingleInstance<Logic>
{
public:
	Logic(void);

	virtual ~Logic(void);
};

typedef boost::shared_ptr<Logic> LogicPtr;

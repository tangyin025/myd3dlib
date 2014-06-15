#pragma once

#include "../Character/LocalPlayer.h"

class Logic
{
protected:
	LocalPlayerPtr m_LocalPlayer;

public:
	Logic(void)
		: m_LocalPlayer(new LocalPlayer)
	{
	}

	virtual ~Logic(void)
	{
	}

	void Create(void);

	void Update(float fElapsedTime);

	void Destroy(void);
};

typedef boost::shared_ptr<Logic> LogicPtr;

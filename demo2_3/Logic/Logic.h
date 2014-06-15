#pragma once

#include "Character.h"

class Logic
{
protected:
	CharacterPtr m_LocalPlayer;

	enum LogicState
	{
		LogicStateLoading,
		LogicStateMain,
	};

	LogicState m_State;

public:
	Logic(void)
		: m_LocalPlayer(new Character)
		, m_State(LogicStateLoading)
	{
	}

	virtual ~Logic(void)
	{
	}

	void Create(void);
	void Update(float fElapsedTime);
	void Destroy(void);

	void ShiftState(LogicState State);
	void OnEnterState(void);
	void OnLeaveState(void);
};

typedef boost::shared_ptr<Logic> LogicPtr;

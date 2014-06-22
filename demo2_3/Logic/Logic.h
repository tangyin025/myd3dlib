#pragma once

#include "Character.h"
#include "../Component/MeshComponent.h"

class Logic
{
protected:
	enum LogicState
	{
		LogicStateMain,
	};

	LogicState m_State;

	physx_ptr<PxRigidActor> m_StaticSceneActor;

	CharacterPtr m_LocalPlayer;

public:
	Logic(void)
		: m_State(LogicStateMain)
		, m_LocalPlayer(new Character)
	{
	}

	virtual ~Logic(void)
	{
	}

	void Create(void);

	void Update(float fElapsedTime);

	//bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Destroy(void);

	void ShiftState(LogicState State);

	void OnEnterState(void);

	void OnLeaveState(void);

	void OnMouseMove(LONG x, LONG y, LONG z);

	void OnKeyDown(DWORD vk);
};

typedef boost::shared_ptr<Logic> LogicPtr;

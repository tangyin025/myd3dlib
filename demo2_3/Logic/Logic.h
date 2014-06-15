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

	//my::OgreMeshSetPtr m_SceneMeshSet;

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

	void Destroy(void);

	void ShiftState(LogicState State);

	void OnEnterState(void);

	void OnLeaveState(void);

	//void OnSceneMeshLoaded(my::DeviceRelatedObjectBasePtr res);
};

typedef boost::shared_ptr<Logic> LogicPtr;

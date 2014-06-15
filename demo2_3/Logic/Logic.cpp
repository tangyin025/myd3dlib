#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

void Logic::Create(void)
{
	OnEnterState();
}

void Logic::Update(float fElapsedTime)
{
	switch(m_State)
	{
	case LogicStateLoading:
		{
			Game::getSingleton().PushGrid();
		}
		break;
	case LogicStateMain:
		{
		}
		break;
	default:
		break;
	}
}

void Logic::Destroy(void)
{
	OnLeaveState();
}

void Logic::ShiftState(LogicState State)
{
	OnLeaveState();
	m_State = State;
	OnEnterState();
}

void Logic::OnEnterState(void)
{
	switch(m_State)
	{
	case LogicStateLoading:
		{
			Game::getSingleton().ExecuteCode("dofile \"GameStateMain.lua\"");
		}
		break;
	case LogicStateMain:
		{
		}
		break;
	default:
		break;
	}
}

void Logic::OnLeaveState(void)
{
	switch(m_State)
	{
	case LogicStateLoading:
		{
		}
		break;
	case LogicStateMain:
		{
		}
		break;
	default:
		break;
	}
}

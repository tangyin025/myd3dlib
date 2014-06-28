#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

using namespace my;

Logic::Logic(void)
	: m_State(LogicStateMain)
	, m_FixedTickTimer(1/60.0f)
{
	m_FixedTickTimer.m_EventTimer = boost::bind(&Logic::OnFixedTick, this, _1);
	m_FixedTickTimer.m_Managed = true;
}

Logic::~Logic(void)
{
}

void Logic::Create(void)
{
	OnEnterState();
}

void Logic::Update(float fElapsedTime)
{
	m_FixedTickTimer.Step(fElapsedTime, 4);

	switch(m_State)
	{
	case LogicStateMain:
		{
			Game::getSingleton().PushGrid();
		}
		break;
	default:
		break;
	}
}

void Logic::OnFixedTick(float fElapsedTime)
{
	switch(m_State)
	{
	case LogicStateMain:
		{
			m_LocalPlayer->Update(fElapsedTime);
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
	if (State != m_State)
	{
		OnLeaveState();
		m_State = State;
		OnEnterState();
	}
}

void Logic::OnEnterState(void)
{
	switch(m_State)
	{
	case LogicStateMain:
		{
			Game::getSingleton().ExecuteCode("dofile \"StateMain.lua\"");
			//PhysXTriangleMeshPtr tri_mesh = Game::getSingleton().LoadTriangleMesh("mesh/scene_tm.phy");
			//m_StaticSceneActor.reset(Game::getSingleton().m_sdk->createRigidStatic(PxTransform::createIdentity()));
			//PxShape * shape = m_StaticSceneActor->createShape(PxTriangleMeshGeometry(tri_mesh->m_ptr), *Game::getSingleton().m_PxMaterial);
			//shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
			//Game::getSingleton().m_Scene->addActor(*m_StaticSceneActor);

			Game::getSingleton().m_Scene->addActor(*PxCreateDynamic(
				*Game::getSingleton().m_sdk, PxTransform::createIdentity(), PxSphereGeometry(0.5f), *Game::getSingleton().m_PxMaterial, 1));

			m_LocalPlayer.reset(new Character());
			m_LocalPlayer->Create();

			Game::getSingleton().m_MouseMovedEvent = boost::bind(&Logic::OnMouseMove, this, _1);
			Game::getSingleton().m_MousePressedEvent = boost::bind(&Logic::OnMouseBtnDown, this, _1);
			Game::getSingleton().m_MouseReleasedEvent = boost::bind(&Logic::OnMouseBtnUp, this, _1);
			Game::getSingleton().m_KeyPressedEvent = boost::bind(&Logic::OnKeyDown, this, _1);
			Game::getSingleton().m_KeyReleasedEvent = boost::bind(&Logic::OnKeyUp, this, _1);
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
	case LogicStateMain:
		{
			m_StaticSceneActor.reset();
			m_LocalPlayer.reset();
		}
		break;
	default:
		break;
	}
}

void Logic::OnMouseMove(InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
}

void Logic::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Logic::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Logic::OnKeyDown(InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
}

void Logic::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
}

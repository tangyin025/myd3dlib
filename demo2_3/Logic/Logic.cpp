#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

using namespace my;

void Logic::Create(void)
{
	OnEnterState();
}

void Logic::Update(float fElapsedTime)
{
	switch(m_State)
	{
	case LogicStateMain:
		{
			Game::getSingleton().PushGrid();
			m_LocalPlayer->Update(fElapsedTime);
		}
		break;
	default:
		break;
	}
}
//
//bool Logic::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	return false;
//}

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
			PhysXTriangleMeshPtr tri_mesh = Game::getSingleton().LoadTriangleMesh("mesh/scene_tm.phy");
			m_StaticSceneActor.reset(Game::getSingleton().m_sdk->createRigidStatic(PxTransform::createIdentity()));
			PxShape * shape = m_StaticSceneActor->createShape(PxTriangleMeshGeometry(tri_mesh->m_ptr), *Game::getSingleton().m_PxMaterial);
			shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
			Game::getSingleton().m_Scene->addActor(*m_StaticSceneActor);
			m_LocalPlayer->Create();
			Game::getSingleton().m_MouseMovedEvent = boost::bind(&Logic::OnMouseMove, this, _1, _2, _3);
			Game::getSingleton().m_KeyPressedEvent = boost::bind(&Logic::OnKeyDown, this, _1);
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
			m_LocalPlayer->Destroy();
		}
		break;
	default:
		break;
	}
}

void Logic::OnMouseMove(LONG x, LONG y, LONG z)
{
	Game::getSingleton().m_ScrInfos[1] = str_printf(L"%ld, %ld, %ld", x, y, z);
}

void Logic::OnKeyDown(DWORD vk)
{
	Game::getSingleton().m_ScrInfos[2] = str_printf(L"%s", Keyboard::TranslateVirtualKey(vk));
}

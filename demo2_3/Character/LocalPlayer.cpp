#include "StdAfx.h"
#include "LocalPlayer.h"
#include "../Game.h"

void LocalPlayer::Create(void)
{
	PxCapsuleControllerDesc cDesc;
	cDesc.radius = 0.3f;
	cDesc.height = 1.5f;
	cDesc.position.y = 5;
	cDesc.material = Game::getSingleton().m_PxMaterial.get();
	m_controller.reset(
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_Scene.get(), cDesc));
}

void LocalPlayer::Update(float fElapsedTime)
{
	m_controller->move(PxVec3(0,-9*fElapsedTime,0), 0.001f, fElapsedTime, PxControllerFilters());
}

void LocalPlayer::Destroy(void)
{
	m_controller.reset();
}

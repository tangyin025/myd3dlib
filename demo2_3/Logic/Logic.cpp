#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

using namespace my;

Logic::Logic(void)
{
}

Logic::~Logic(void)
{
}

void Logic::Create(void)
{
	m_player.reset(new LocalPlayer());
	m_player->Create();
}

void Logic::Update(float fElapsedTime)
{
	m_player->Update(fElapsedTime);

	FirstPersonCamera * camera = dynamic_cast<FirstPersonCamera *>(Game::getSingleton().m_Camera.get());
	camera->m_Eye = m_player->getPosition() + m_player->m_LookDir * 5;
	camera->m_Eular = m_player->m_LookAngles; // 右手系空间相机朝向-z轴
	Game::getSingleton().ResetViewedCmps(camera->m_Eye, m_player->getPosition());
}

void Logic::OnPxThreadSubstep(float dtime)
{
	m_player->OnPxThreadSubstep(dtime);
}

void Logic::Destroy(void)
{
	m_player->Destroy();
	m_player.reset();
	m_cmps.clear();
}

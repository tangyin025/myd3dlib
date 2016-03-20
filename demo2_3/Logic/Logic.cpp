#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

using namespace my;

Logic::Logic(void)
	: m_FixedTickTimer(1/60.0f)
{
	m_FixedTickTimer.m_EventTimer = boost::bind(&Logic::OnFixedTick, this, _1);
	m_FixedTickTimer.m_Managed = true;
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
	FirstPersonCamera * camera = dynamic_cast<FirstPersonCamera *>(Game::getSingleton().m_Camera.get());
	const Matrix4 RotM = Matrix4::RotationYawPitchRoll(m_player->m_LookAngles.y, m_player->m_LookAngles.x, m_player->m_LookAngles.z);
	const Vector3 & Dir = RotM.row<2>().xyz;
	camera->m_Eye = m_player->getPosition() + Dir * 5;
	camera->m_Eular = m_player->m_LookAngles; // 右手系空间相机朝向-z轴

	Game::getSingleton().ResetViewedCmps(camera->m_Eye, m_player->getPosition());

	m_FixedTickTimer.Step(fElapsedTime, 4);
}

void Logic::OnFixedTick(float fElapsedTime)
{
	m_player->Update(fElapsedTime);
}

void Logic::Destroy(void)
{
	m_player->Destroy();
	m_player.reset();
	m_cmps.clear();
}

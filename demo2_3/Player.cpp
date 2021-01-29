#include "stdafx.h"
#include "Player.h"
#include "Game.h"
#include "Animation.h"

using namespace my;

Player::Player(void)
	: m_LookAngle(0, 0, 0)
	, m_LookDist(3)
	, m_MoveAxis(0, 0)
{
}

Player::Player(const char * Name, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius, float ContactOffset, unsigned int filterWord0)
	: Character(Name, Position, Rotation, Scale, aabb, Height, Radius, ContactOffset, filterWord0)
	, m_LookAngle(0, 0, 0)
	, m_LookDist(3)
	, m_MoveAxis(0, 0)
{
}

Player::~Player(void)
{
}

void Player::Update(float fElapsedTime)
{
	if (m_MoveAxis.x != 0 || m_MoveAxis.y != 0)
	{
		m_TargetSpeed = Game::getSingleton().m_keyboard->IsKeyDown(KC_LSHIFT) ? 10.0f : 2.0f;
		m_TargetOrientation = m_LookAngle.y + atan2f((float)m_MoveAxis.x, (float)m_MoveAxis.y) + D3DXToRadian(180);
	}
	else
	{
		m_TargetSpeed = 0;
	}

	Character::Update(fElapsedTime);

	m_LookMatrix = Matrix4::RotationYawPitchRoll(m_LookAngle.y, m_LookAngle.x, m_LookAngle.z);

	PerspectiveCamera * camera = static_cast<PerspectiveCamera *>(Game::getSingleton().m_Camera.get());
	camera->m_Eular = m_LookAngle;
	camera->m_Eye = m_Position + Vector3(0, 0.75f, 0) + m_LookMatrix[2].xyz * m_LookDist;
	Game::getSingleton().m_SkyLightCam.m_Eye = m_Position;
}

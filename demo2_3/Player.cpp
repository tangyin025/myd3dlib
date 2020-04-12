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
	Init();
}

Player::Player(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb, float Height, float Radius, float ContactOffset)
	: Character(Position, Rotation, Scale, aabb, Height, Radius, ContactOffset)
	, m_LookAngle(0, 0, 0)
	, m_LookDist(3)
	, m_MoveAxis(0, 0)
{
	Init();
}

Player::~Player(void)
{
	Destroy();
}

void Player::Init(void)
{
	Game::getSingleton().m_mouse->m_MovedEvent.connect(boost::bind(&Player::OnMouseMove, this, _1));
	Game::getSingleton().m_mouse->m_PressedEvent.connect(boost::bind(&Player::OnMouseBtnDown, this, _1));
	Game::getSingleton().m_mouse->m_ReleasedEvent.connect(boost::bind(&Player::OnMouseBtnUp, this, _1));
	Game::getSingleton().m_keyboard->m_PressedEvent.connect(boost::bind(&Player::OnKeyDown, this, _1));
	Game::getSingleton().m_keyboard->m_ReleasedEvent.connect(boost::bind(&Player::OnKeyUp, this, _1));
	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent.connect(boost::bind(&Player::OnJoystickAxisMove, this, _1));
		Game::getSingleton().m_joystick->m_PovMovedEvent.connect(boost::bind(&Player::OnJoystickPovMove, this, _1));
		Game::getSingleton().m_joystick->m_BtnPressedEvent.connect(boost::bind(&Player::OnJoystickBtnDown, this, _1));
		Game::getSingleton().m_joystick->m_BtnReleasedEvent.connect(boost::bind(&Player::OnJoystickBtnUp, this, _1));
	}
	Game::getSingleton().m_wnd->m_ActivateEvent.connect(boost::bind(&Player::OnWindowActivate, this, _1));
}

void Player::Destroy(void)
{
	Game::getSingleton().m_mouse->m_MovedEvent.disconnect(boost::bind(&Player::OnMouseMove, this, _1));
	Game::getSingleton().m_mouse->m_PressedEvent.disconnect(boost::bind(&Player::OnMouseBtnDown, this, _1));
	Game::getSingleton().m_mouse->m_ReleasedEvent.disconnect(boost::bind(&Player::OnMouseBtnUp, this, _1));
	Game::getSingleton().m_keyboard->m_PressedEvent.disconnect(boost::bind(&Player::OnKeyDown, this, _1));
	Game::getSingleton().m_keyboard->m_ReleasedEvent.disconnect(boost::bind(&Player::OnKeyUp, this, _1));
	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent.disconnect(boost::bind(&Player::OnJoystickAxisMove, this, _1));
		Game::getSingleton().m_joystick->m_PovMovedEvent.disconnect(boost::bind(&Player::OnJoystickPovMove, this, _1));
		Game::getSingleton().m_joystick->m_BtnPressedEvent.disconnect(boost::bind(&Player::OnJoystickBtnDown, this, _1));
		Game::getSingleton().m_joystick->m_BtnReleasedEvent.disconnect(boost::bind(&Player::OnJoystickBtnUp, this, _1));
	}
	Game::getSingleton().m_wnd->m_ActivateEvent.disconnect(boost::bind(&Player::OnWindowActivate, this, _1));
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

	PerspectiveCamera * camera = static_cast<PerspectiveCamera *>(Game::getSingleton().m_Camera.get());
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_LookAngle.y, m_LookAngle.x, m_LookAngle.z);
	camera->m_Eular = m_LookAngle;
	camera->m_Eye = m_Position + Vector3(0, 0.75f, 0) + Rotation[2].xyz * m_LookDist;
	Game::getSingleton().m_SkyLightCam->m_Eye = m_Position;
}

void Player::OnMouseMove(my::EventArg * arg)
{
	if (m_EventMouseMove)
	{
		m_EventMouseMove(arg);
	}
}

void Player::OnMouseBtnDown(my::EventArg * arg)
{
	if (m_EventMouseBtnDown)
	{
		m_EventMouseBtnDown(arg);
	}
}

void Player::OnMouseBtnUp(my::EventArg * arg)
{
	if (m_EventMouseBtnUp)
	{
		m_EventMouseBtnUp(arg);
	}
}

void Player::OnKeyDown(my::EventArg * arg)
{
	if (m_EventKeyDown)
	{
		m_EventKeyDown(arg);
	}
}

void Player::OnKeyUp(my::EventArg * arg)
{
	if (m_EventKeyUp)
	{
		m_EventKeyUp(arg);
	}
}

void Player::OnJoystickAxisMove(my::EventArg * arg)
{
	if (m_EventJoystickAxisMove)
	{
		m_EventJoystickAxisMove(arg);
	}
}

void Player::OnJoystickPovMove(my::EventArg * arg)
{
	if (m_EventJoystickPovMove)
	{
		m_EventJoystickPovMove(arg);
	}
}

void Player::OnJoystickBtnDown(my::EventArg * arg)
{
	if (m_EventJoystickBtnDown)
	{
		m_EventJoystickBtnDown(arg);
	}
}

void Player::OnJoystickBtnUp(my::EventArg * arg)
{
	if (m_EventJoystickBtnUp)
	{
		m_EventJoystickBtnUp(arg);
	}
}

void Player::OnWindowActivate(bool bActivated)
{
	if (!bActivated)
	{
		m_MoveAxis.SetPoint(0, 0);
	}
}

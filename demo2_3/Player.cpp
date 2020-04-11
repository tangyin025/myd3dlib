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

void Player::OnMouseMove(my::InputEventArg * arg)
{
	//MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
	//if (mmarg.x != 0)
	//{
	//	m_LookAngle.y += -D3DXToRadian(mmarg.x);
	//}
	//if (mmarg.y != 0)
	//{
	//	m_LookAngle.x += -D3DXToRadian(mmarg.y);
	//}
	//if (mmarg.z != 0)
	//{
	//	m_LookDist -= (float)mmarg.z / 480;
	//}
	if (m_EventMouseMove)
	{
		m_EventMouseMove(arg);
	}
}

void Player::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Player::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Player::OnKeyDown(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case KC_SPACE:
	{
		m_Velocity.y = 5.0f;
		if (m_Animation)
		{
			m_Animation->Play("jumpforward", "Bip01_Spine1", 0.3f, 0.3f, 1.0f);
		}
		break;
	}
	case KC_W:
		m_MoveAxis.y += 1;
		break;
	case KC_A:
		m_MoveAxis.x += 1;
		break;
	case KC_S:
		m_MoveAxis.y -= 1;
		break;
	case KC_D:
		m_MoveAxis.x -= 1;
		break;
	case KC_LCONTROL:
		Game::getSingleton().m_mouse->Unacquire();
		Game::getSingleton().m_mouse->SetCooperativeLevel(Game::getSingleton().m_wnd->m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		break;
	}
}

void Player::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case KC_W:
		m_MoveAxis.y -= 1;
		break;
	case KC_A:
		m_MoveAxis.x -= 1;
		break;
	case KC_S:
		m_MoveAxis.y += 1;
		break;
	case KC_D:
		m_MoveAxis.x += 1;
		break;
	case KC_LCONTROL:
		Game::getSingleton().m_mouse->Unacquire();
		Game::getSingleton().m_mouse->SetCooperativeLevel(Game::getSingleton().m_wnd->m_hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		break;
	}
}

void Player::OnJoystickAxisMove(my::InputEventArg * arg)
{
	JoystickAxisEventArg & jaarg = *dynamic_cast<JoystickAxisEventArg *>(arg);
}

void Player::OnJoystickPovMove(my::InputEventArg * arg)
{
	JoystickPovEventArg & jparg = *dynamic_cast<JoystickPovEventArg *>(arg);
}

void Player::OnJoystickBtnDown(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void Player::OnJoystickBtnUp(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void Player::OnWindowActivate(bool bActivated)
{
	if (!bActivated)
	{
		m_MoveAxis.SetPoint(0, 0);
	}
}

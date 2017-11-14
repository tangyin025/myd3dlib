#include "stdafx.h"
#include "PlayerController.h"
#include "Game.h"

using namespace my;

PlayerController::PlayerController(void)
	: m_LookAngle(0,0,0)
	, m_MoveAxis(0,0)
{
	Init();
}

PlayerController::PlayerController(Actor * actor)
	: CharacterController(actor)
	, m_LookAngle(0, 0, 0)
	, m_MoveAxis(0, 0)
{
	Init();
}

PlayerController::~PlayerController(void)
{
	Destroy();
}

void PlayerController::Init(void)
{
	Game::getSingleton().m_mouse->m_MovedEvent.connect(boost::bind(&PlayerController::OnMouseMove, this, _1));
	Game::getSingleton().m_mouse->m_PressedEvent.connect(boost::bind(&PlayerController::OnMouseBtnDown, this, _1));
	Game::getSingleton().m_mouse->m_ReleasedEvent.connect(boost::bind(&PlayerController::OnMouseBtnUp, this, _1));
	Game::getSingleton().m_keyboard->m_PressedEvent.connect(boost::bind(&PlayerController::OnKeyDown, this, _1));
	Game::getSingleton().m_keyboard->m_ReleasedEvent.connect(boost::bind(&PlayerController::OnKeyUp, this, _1));
	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent.connect(boost::bind(&PlayerController::OnJoystickAxisMove, this, _1));
		Game::getSingleton().m_joystick->m_PovMovedEvent.connect(boost::bind(&PlayerController::OnJoystickPovMove, this, _1));
		Game::getSingleton().m_joystick->m_BtnPressedEvent.connect(boost::bind(&PlayerController::OnJoystickBtnDown, this, _1));
		Game::getSingleton().m_joystick->m_BtnReleasedEvent.connect(boost::bind(&PlayerController::OnJoystickBtnUp, this, _1));
	}
}

void PlayerController::Destroy(void)
{
	Game::getSingleton().m_mouse->m_MovedEvent.disconnect(boost::bind(&PlayerController::OnMouseMove, this, _1));
	Game::getSingleton().m_mouse->m_PressedEvent.disconnect(boost::bind(&PlayerController::OnMouseBtnDown, this, _1));
	Game::getSingleton().m_mouse->m_ReleasedEvent.disconnect(boost::bind(&PlayerController::OnMouseBtnUp, this, _1));
	Game::getSingleton().m_keyboard->m_PressedEvent.disconnect(boost::bind(&PlayerController::OnKeyDown, this, _1));
	Game::getSingleton().m_keyboard->m_ReleasedEvent.disconnect(boost::bind(&PlayerController::OnKeyUp, this, _1));
	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent.disconnect(boost::bind(&PlayerController::OnJoystickAxisMove, this, _1));
		Game::getSingleton().m_joystick->m_PovMovedEvent.disconnect(boost::bind(&PlayerController::OnJoystickPovMove, this, _1));
		Game::getSingleton().m_joystick->m_BtnPressedEvent.disconnect(boost::bind(&PlayerController::OnJoystickBtnDown, this, _1));
		Game::getSingleton().m_joystick->m_BtnReleasedEvent.disconnect(boost::bind(&PlayerController::OnJoystickBtnUp, this, _1));
	}
}

void PlayerController::Update(float fElapsedTime)
{
	float move_step_sq = m_MoveAxis.magnitudeSq();
	if (move_step_sq > 0.01)
	{
		m_MoveAcceleration = 20.0f;
		m_MoveOrientation = m_LookAngle.y + atan2f(m_MoveAxis.x, m_MoveAxis.y);
	}
	else
	{
		m_MoveAcceleration = 0;
	}

	CharacterController::Update(fElapsedTime);

	PerspectiveCamera * camera = static_cast<PerspectiveCamera *>(Game::getSingleton().m_Camera.get());
	Character * character = dynamic_cast<Character *>(m_Actor);
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_LookAngle.y, m_LookAngle.x, m_LookAngle.z);
	Vector3 ViewPos = character->GetWorldPosition();
	camera->m_Eular = m_LookAngle;
	camera->m_Eye = ViewPos + Rotation[2].xyz * 10;
	Game::getSingleton().m_SkyLightCam->m_Eye = ViewPos;
}

void PlayerController::OnMouseMove(my::InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
	if (mmarg.x != 0)
	{
		m_LookAngle.y += -D3DXToRadian(mmarg.x);
	}
	if (mmarg.y != 0)
	{
		m_LookAngle.x += -D3DXToRadian(mmarg.y);
	}
}

void PlayerController::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void PlayerController::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void PlayerController::OnKeyDown(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	Character * character = dynamic_cast<Character *>(m_Actor);
	switch (karg.kc)
	{
	case KC_SPACE:
		character->m_Velocity.y = 5.0f;
		break;
	case KC_W:
		m_MoveAxis.y = 1.0f;
		break;
	case KC_A:
		m_MoveAxis.x = 1.0f;
		break;
	case KC_S:
		m_MoveAxis.y = -1.0f;
		break;
	case KC_D:
		m_MoveAxis.x = -1.0f;
		break;
	}
}

void PlayerController::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case KC_W:
		if (m_MoveAxis.y > 0)
		{
			m_MoveAxis.y = 0;
		}
		break;
	case KC_A:
		if (m_MoveAxis.x > 0)
		{
			m_MoveAxis.x = 0;
		}
		break;
	case KC_S:
		if (m_MoveAxis.y < 0)
		{
			m_MoveAxis.y = 0;
		}
		break;
	case KC_D:
		if (m_MoveAxis.x < 0)
		{
			m_MoveAxis.x = 0;
		}
		break;
	}
}

void PlayerController::OnJoystickAxisMove(my::InputEventArg * arg)
{
	JoystickAxisEventArg & jaarg = *dynamic_cast<JoystickAxisEventArg *>(arg);
}

void PlayerController::OnJoystickPovMove(my::InputEventArg * arg)
{
	JoystickPovEventArg & jparg = *dynamic_cast<JoystickPovEventArg *>(arg);
}

void PlayerController::OnJoystickBtnDown(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void PlayerController::OnJoystickBtnUp(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

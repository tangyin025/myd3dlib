#include "stdafx.h"
#include "PlayerController.h"
#include "Game.h"
#include "Animator.h"

using namespace my;

PlayerController::PlayerController(void)
	: m_LookAngle(0, 0, 0)
	, m_LookDist(3)
	, m_MoveAxis(0, 0)
{
	Init();
}

PlayerController::PlayerController(Character * character)
	: CharacterController(character)
	, m_LookAngle(0, 0, 0)
	, m_LookDist(3)
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
	Game::getSingleton().m_wnd->m_ActivateEvent.connect(boost::bind(&PlayerController::OnWindowActivate, this, _1));
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
	Game::getSingleton().m_wnd->m_ActivateEvent.disconnect(boost::bind(&PlayerController::OnWindowActivate, this, _1));
}

void PlayerController::Update(float fElapsedTime)
{
	if (m_MoveAxis.x != 0 || m_MoveAxis.y != 0)
	{
		m_Character->m_TargetSpeed = Game::getSingleton().m_keyboard->IsKeyDown(KC_LSHIFT) ? 10.0f : 2.0f;
		m_Character->m_TargetOrientation = m_LookAngle.y + atan2f((float)m_MoveAxis.x, (float)m_MoveAxis.y) + D3DXToRadian(180);
	}
	else
	{
		m_Character->m_TargetSpeed = 0;
	}

	CharacterController::Update(fElapsedTime);

	PerspectiveCamera * camera = static_cast<PerspectiveCamera *>(Game::getSingleton().m_Camera.get());
	Matrix4 Rotation = Matrix4::RotationYawPitchRoll(m_LookAngle.y, m_LookAngle.x, m_LookAngle.z);
	camera->m_Eular = m_LookAngle;
	camera->m_Eye = m_Character->m_Position + Vector3(0, 0.75f, 0) + Rotation[2].xyz * m_LookDist;
	Game::getSingleton().m_SkyLightCam->m_Eye = m_Character->m_Position;
	Game::getSingleton().m_TargetActor = m_Character;
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
	if (mmarg.z != 0)
	{
		m_LookDist -= (float)mmarg.z / 480;
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
	switch (karg.kc)
	{
	case KC_SPACE:
	{
		m_Character->m_Velocity.y = 5.0f;
		if (m_Character->m_Animator && m_Character->m_Animator->m_Node)
		{
			AnimationNodeSlotPtr node_slot = boost::dynamic_pointer_cast<AnimationNodeSlot>(m_Character->m_Animator->m_Node);
			if (node_slot)
			{
				node_slot->Play("jumpforward", "Bip01", 0.3f, 0.3f, 2.0f, 1.0f);
			}
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

void PlayerController::OnKeyUp(my::InputEventArg * arg)
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

void PlayerController::OnWindowActivate(bool bActivated)
{
	if (!bActivated)
	{
		m_MoveAxis.SetPoint(0, 0);
	}
}

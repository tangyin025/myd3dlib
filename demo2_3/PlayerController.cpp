#include "stdafx.h"
#include "PlayerController.h"
#include "Game.h"

using namespace my;

PlayerController::PlayerController(void)
	: m_LookAngle(0,0,0)
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

PlayerController::~PlayerController(void)
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
	Controller::Update(fElapsedTime);
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
	switch (karg.kc)
	{
	case VK_SPACE:
		karg.handled = true;
		break;
	case 'W':
		karg.handled = true;
		break;
	case 'A':
		karg.handled = true;
		break;
	case 'S':
		karg.handled = true;
		break;
	case 'D':
		karg.handled = true;
		break;
	}
}

void PlayerController::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case 'W':
		karg.handled = true;
		break;
	case 'A':
		karg.handled = true;
		break;
	case 'S':
		karg.handled = true;
		break;
	case 'D':
		karg.handled = true;
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

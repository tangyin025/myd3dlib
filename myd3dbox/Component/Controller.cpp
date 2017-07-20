#include "stdafx.h"
#include "Controller.h"

using namespace my;

void ControllerMgr::AddController(ControllerPtr controller)
{
	m_controllers.insert(controller);
}

void ControllerMgr::RemoveController(ControllerPtr controller)
{
	m_controllers.erase(controller);
}

void ControllerMgr::ClearAllControllers(void)
{
	m_controllers.clear();
}

void ControllerMgr::Update(float fElapsedTime)
{
	ControllerPtrSet::iterator controller_iter = m_controllers.begin();
	for (; controller_iter != m_controllers.end(); controller_iter++)
	{
		(*controller_iter)->Update(fElapsedTime);
	}
}

Controller::Controller(void)
{
}

Controller::~Controller(void)
{
}

void Controller::Update(float fElapsedTime)
{
}

PlayerController::PlayerController(void)
{
	InputMgr::getSingleton().m_MouseMovedEvent = boost::bind(&PlayerController::OnMouseMove, this, _1);
	InputMgr::getSingleton().m_MousePressedEvent = boost::bind(&PlayerController::OnMouseBtnDown, this, _1);
	InputMgr::getSingleton().m_MouseReleasedEvent = boost::bind(&PlayerController::OnMouseBtnUp, this, _1);
	InputMgr::getSingleton().m_KeyPressedEvent = boost::bind(&PlayerController::OnKeyDown, this, _1);
	InputMgr::getSingleton().m_KeyReleasedEvent = boost::bind(&PlayerController::OnKeyUp, this, _1);

	if (InputMgr::getSingleton().m_joystick)
	{
		InputMgr::getSingleton().m_joystick->m_AxisMovedEvent = boost::bind(&PlayerController::OnJoystickAxisMove, this, _1);
		InputMgr::getSingleton().m_joystick->m_PovMovedEvent = boost::bind(&PlayerController::OnJoystickPovMove, this, _1);
		InputMgr::getSingleton().m_joystick->m_BtnPressedEvent = boost::bind(&PlayerController::OnJoystickBtnDown, this, _1);
		InputMgr::getSingleton().m_joystick->m_BtnReleasedEvent = boost::bind(&PlayerController::OnJoystickBtnUp, this, _1);
	}
}

PlayerController::~PlayerController(void)
{
	InputMgr::getSingleton().m_MouseMovedEvent.clear();
	InputMgr::getSingleton().m_MousePressedEvent.clear();
	InputMgr::getSingleton().m_MouseReleasedEvent.clear();
	InputMgr::getSingleton().m_KeyPressedEvent.clear();
	InputMgr::getSingleton().m_KeyReleasedEvent.clear();

	if (InputMgr::getSingleton().m_joystick)
	{
		InputMgr::getSingleton().m_joystick->m_AxisMovedEvent.clear();
		InputMgr::getSingleton().m_joystick->m_PovMovedEvent.clear();
		InputMgr::getSingleton().m_joystick->m_BtnPressedEvent.clear();
		InputMgr::getSingleton().m_joystick->m_BtnReleasedEvent.clear();
	}
}

void PlayerController::OnMouseMove(my::InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
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
}

void PlayerController::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
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

#include "stdafx.h"
#include "Controller.h"
#include "Character.h"

using namespace my;

Controller::Controller(void)
	: m_Actor(NULL)
{
}

Controller::~Controller(void)
{
}

void Controller::Update(float fElapsedTime)
{
}

PlayerController::PlayerController(void)
	: m_LookAngles(0,0,0)
	, m_LookDir(0,0,1)
	, m_LookDist(5)
	, m_FaceAngle(0)
	, m_FaceAngleInerp(0)
	, m_InputLtRt(0)
	, m_InputUpDn(0)
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

void PlayerController::Update(float fElapsedTime)
{
	Controller::Update(fElapsedTime);

	_ASSERT(m_Actor->m_Type == Component::ComponentTypeCharacter);
	Character * character = static_cast<Character *>(m_Actor);
	if (m_InputLtRt != 0 || m_InputUpDn != 0)
	{
		const Vector3 Right(m_LookDir.z, 0, -m_LookDir.x);
		const Vector3 Final(Vector3(
			m_InputLtRt * Right.x + m_InputUpDn * m_LookDir.x, 0, m_InputLtRt * Right.z + m_InputUpDn * m_LookDir.z).normalize());
		const float Speed = 5.0f;
		character->m_Velocity.x = Final.x * Speed;
		character->m_Velocity.z = Final.z * Speed;
		m_FaceAngle = atan2(character->m_Velocity.x, character->m_Velocity.z);
		if (m_FaceAngle > m_FaceAngleInerp + D3DX_PI)
		{
			m_FaceAngleInerp += 2 * D3DX_PI;
		}
		else if (m_FaceAngle < m_FaceAngleInerp - D3DX_PI)
		{
			m_FaceAngleInerp -= 2 * D3DX_PI;
		}
	}
	else
	{
		character->m_Velocity.x = 0;
		character->m_Velocity.z = 0;
	}

	m_FaceAngleInerp = Lerp(m_FaceAngleInerp, m_FaceAngle, 1.0f - powf(0.8f, 30 * fElapsedTime));

	const Matrix4 RotM = Matrix4::RotationYawPitchRoll(m_LookAngles.y, m_LookAngles.x, m_LookAngles.z);
	m_LookDir = RotM.row<2>().xyz;
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
	_ASSERT(m_Actor->m_Type == Component::ComponentTypeCharacter);
	Character * character = static_cast<Character *>(m_Actor);
	switch (karg.kc)
	{
	case VK_SPACE:
		character->m_Velocity.y = 5;
		karg.handled = true;
		break;
	case 'W':
		m_InputUpDn = -1;
		karg.handled = true;
		break;
	case 'A':
		m_InputLtRt = -1;
		karg.handled = true;
		break;
	case 'S':
		m_InputUpDn = 1;
		karg.handled = true;
		break;
	case 'D':
		m_InputLtRt = 1;
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
		if (m_InputUpDn == -1)
		{
			m_InputUpDn = 0;
		}
		karg.handled = true;
		break;
	case 'A':
		if (m_InputLtRt == -1)
		{
			m_InputLtRt = 0;
		}
		karg.handled = true;
		break;
	case 'S':
		if (m_InputUpDn == 1)
		{
			m_InputUpDn = 0;
		}
		karg.handled = true;
		break;
	case 'D':
		if (m_InputLtRt == 1)
		{
			m_InputLtRt = 0;
		}
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

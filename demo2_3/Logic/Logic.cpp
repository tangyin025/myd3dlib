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
	Game::getSingleton().m_MouseMovedEvent = boost::bind(&Logic::OnMouseMove, this, _1);
	Game::getSingleton().m_MousePressedEvent = boost::bind(&Logic::OnMouseBtnDown, this, _1);
	Game::getSingleton().m_MouseReleasedEvent = boost::bind(&Logic::OnMouseBtnUp, this, _1);
	Game::getSingleton().m_KeyPressedEvent = boost::bind(&Logic::OnKeyDown, this, _1);
	Game::getSingleton().m_KeyReleasedEvent = boost::bind(&Logic::OnKeyUp, this, _1);

	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent = boost::bind(&Logic::OnJoystickAxisMove, this, _1);
		Game::getSingleton().m_joystick->m_PovMovedEvent = boost::bind(&Logic::OnJoystickPovMove, this, _1);
		Game::getSingleton().m_joystick->m_BtnPressedEvent = boost::bind(&Logic::OnJoystickBtnDown, this, _1);
		Game::getSingleton().m_joystick->m_BtnReleasedEvent = boost::bind(&Logic::OnJoystickBtnUp, this, _1);
	}

	m_LocalPlayer.reset(new Character());
	m_LocalPlayer->Create();
}

void Logic::Update(float fElapsedTime)
{
	FirstPersonCamera * camera = dynamic_cast<FirstPersonCamera *>(Game::getSingleton().m_Camera.get());
	const Matrix4 RotM = Matrix4::RotationYawPitchRoll(m_LocalPlayer->m_LookAngles.y, m_LocalPlayer->m_LookAngles.x, m_LocalPlayer->m_LookAngles.z);
	const Vector3 & Dir = RotM.row<2>().xyz;
	camera->m_Eye = m_LocalPlayer->getPosition() + Dir * 5;
	camera->m_Eular = m_LocalPlayer->m_LookAngles; // 右手系空间相机朝向-z轴

	m_FixedTickTimer.Step(fElapsedTime, 4);
}

void Logic::OnFixedTick(float fElapsedTime)
{
	m_LocalPlayer->Update(fElapsedTime);
}

void Logic::Destroy(void)
{
	m_StaticSceneActor.reset();
	m_LocalPlayer.reset();

	Game::getSingleton().m_MouseMovedEvent.clear();
	Game::getSingleton().m_MousePressedEvent.clear();
	Game::getSingleton().m_MouseReleasedEvent.clear();
	Game::getSingleton().m_KeyPressedEvent.clear();
	Game::getSingleton().m_KeyReleasedEvent.clear();

	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent.clear();
		Game::getSingleton().m_joystick->m_PovMovedEvent.clear();
		Game::getSingleton().m_joystick->m_BtnPressedEvent.clear();
		Game::getSingleton().m_joystick->m_BtnReleasedEvent.clear();
	}
}

void Logic::OnMouseMove(InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
	if (mmarg.x != 0)
	{
		m_LocalPlayer->m_LookAngles.y += -D3DXToRadian(mmarg.x);
	}
	if (mmarg.y != 0)
	{
		m_LocalPlayer->m_LookAngles.x += -D3DXToRadian(mmarg.y);
	}
}

void Logic::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Logic::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void Logic::OnKeyDown(InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case VK_SPACE:
		m_LocalPlayer->Jump();
		karg.handled = true;
		break;
	case 'W':
		m_LocalPlayer->AddMoveState(Character::MoveStateFront);
		karg.handled = true;
		break;
	case 'A':
		m_LocalPlayer->AddMoveState(Character::MoveStateLeft);
		karg.handled = true;
		break;
	case 'S':
		m_LocalPlayer->AddMoveState(Character::MoveStateBack);
		karg.handled = true;
		break;
	case 'D':
		m_LocalPlayer->AddMoveState(Character::MoveStateRight);
		karg.handled = true;
		break;
	}
}

void Logic::OnKeyUp(my::InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case 'W':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateFront);
		karg.handled = true;
		break;
	case 'A':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateLeft);
		karg.handled = true;
		break;
	case 'S':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateBack);
		karg.handled = true;
		break;
	case 'D':
		m_LocalPlayer->RemoveMoveState(Character::MoveStateRight);
		karg.handled = true;
		break;
	}
}

void Logic::OnJoystickAxisMove(my::InputEventArg * arg)
{
	JoystickAxisEventArg & jaarg = *dynamic_cast<JoystickAxisEventArg *>(arg);
}

void Logic::OnJoystickPovMove(my::InputEventArg * arg)
{
	JoystickPovEventArg & jparg = *dynamic_cast<JoystickPovEventArg *>(arg);
}

void Logic::OnJoystickBtnDown(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void Logic::OnJoystickBtnUp(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

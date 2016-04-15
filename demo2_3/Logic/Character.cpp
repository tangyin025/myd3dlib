#include "StdAfx.h"
#include "Character.h"
#include "../Game.h"

using namespace my;

Character::Character(void)
	: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector3(0,0,0), 1, 0.8f)
	, m_LookAngles(0,0,0)
{
}

Character::~Character(void)
{
	_ASSERT(!m_controller);
}

void Character::Create(void)
{
	PxCapsuleControllerDesc cDesc;
	cDesc.radius = 0.15f;
	cDesc.height = 1.0f;
	cDesc.position.y = 5;
	cDesc.material = Game::getSingleton().m_PxMaterial.get();
	cDesc.callback = this;
	cDesc.behaviorCallback = this;
	m_controller.reset(
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_PxScene.get(), cDesc));

	Game::getSingleton().m_EventPxThreadSubstep.connect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
}

void Character::Update(float fElapsedTime)
{
	const Matrix4 RotM = Matrix4::RotationYawPitchRoll(m_LookAngles.y, m_LookAngles.x, m_LookAngles.z);
	m_LookDir = RotM.row<2>().xyz;
}

void Character::OnPxThreadSubstep(float dtime)
{
	// 注意，多线程调用，不要在这里更新模型渲染数据数据
	velocity.y = velocity.y + PhysXContext::Gravity.y * dtime;
	m_controller->move((PxVec3&)(velocity * dtime), 0.001f, dtime, PxControllerFilters());
	setPosition((Vector3&)toVec3(m_controller->getPosition()));
}

void Character::Destroy(void)
{
	m_controller.reset();

	Game::getSingleton().m_EventPxThreadSubstep.disconnect(boost::bind(&Character::OnPxThreadSubstep, this, _1));
}

void Character::onShapeHit(const PxControllerShapeHit& hit)
{
	if (hit.worldNormal.y > 0 && hit.worldNormal.y > fabs(hit.worldNormal.x) && hit.worldNormal.y > fabs(hit.worldNormal.z))
	{
		velocity.y = 0;
	}
}

void Character::onControllerHit(const PxControllersHit& hit)
{
}

void Character::onObstacleHit(const PxControllerObstacleHit& hit)
{
}

PxU32 Character::getBehaviorFlags(const PxShape& shape)
{
	return 0;
}

PxU32 Character::getBehaviorFlags(const PxController& controller)
{
	return 0;
}

PxU32 Character::getBehaviorFlags(const PxObstacle& obstacle)
{
	return 0;
}

void LocalPlayer::Create(void)
{
	Player::Create();

	Game::getSingleton().m_MouseMovedEvent = boost::bind(&LocalPlayer::OnMouseMove, this, _1);
	Game::getSingleton().m_MousePressedEvent = boost::bind(&LocalPlayer::OnMouseBtnDown, this, _1);
	Game::getSingleton().m_MouseReleasedEvent = boost::bind(&LocalPlayer::OnMouseBtnUp, this, _1);
	Game::getSingleton().m_KeyPressedEvent = boost::bind(&LocalPlayer::OnKeyDown, this, _1);
	Game::getSingleton().m_KeyReleasedEvent = boost::bind(&LocalPlayer::OnKeyUp, this, _1);

	if (Game::getSingleton().m_joystick)
	{
		Game::getSingleton().m_joystick->m_AxisMovedEvent = boost::bind(&LocalPlayer::OnJoystickAxisMove, this, _1);
		Game::getSingleton().m_joystick->m_PovMovedEvent = boost::bind(&LocalPlayer::OnJoystickPovMove, this, _1);
		Game::getSingleton().m_joystick->m_BtnPressedEvent = boost::bind(&LocalPlayer::OnJoystickBtnDown, this, _1);
		Game::getSingleton().m_joystick->m_BtnReleasedEvent = boost::bind(&LocalPlayer::OnJoystickBtnUp, this, _1);
	}
}

void LocalPlayer::Update(float fElapsedTime)
{
	Player::Update(fElapsedTime);

	if (m_InputLtRt != 0 || m_InputUpDn != 0)
	{
		const Vector3 Right(m_LookDir.z, 0, -m_LookDir.x);
		const Vector3 Final(Vector3(
			m_InputLtRt * Right.x + m_InputUpDn * m_LookDir.x, 0, m_InputLtRt * Right.z + m_InputUpDn * m_LookDir.z).normalize());
		const float Speed = 5.0f;
		velocity.x = Final.x * Speed;
		velocity.z = Final.z * Speed;
	}
	else
	{
		velocity.x = 0;
		velocity.z = 0;
	}
}

void LocalPlayer::Destroy(void)
{
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

	Player::Destroy();
}

void LocalPlayer::OnMouseMove(InputEventArg * arg)
{
	MouseMoveEventArg & mmarg = *dynamic_cast<MouseMoveEventArg *>(arg);
	if (mmarg.x != 0)
	{
		m_LookAngles.y += -D3DXToRadian(mmarg.x);
	}
	if (mmarg.y != 0)
	{
		m_LookAngles.x += -D3DXToRadian(mmarg.y);
	}
}

void LocalPlayer::OnMouseBtnDown(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void LocalPlayer::OnMouseBtnUp(my::InputEventArg * arg)
{
	MouseBtnEventArg & mbarg = *dynamic_cast<MouseBtnEventArg *>(arg);
}

void LocalPlayer::OnKeyDown(InputEventArg * arg)
{
	KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
	switch (karg.kc)
	{
	case VK_SPACE:
		velocity.y = 10;
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

void LocalPlayer::OnKeyUp(my::InputEventArg * arg)
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

void LocalPlayer::OnJoystickAxisMove(my::InputEventArg * arg)
{
	JoystickAxisEventArg & jaarg = *dynamic_cast<JoystickAxisEventArg *>(arg);
}

void LocalPlayer::OnJoystickPovMove(my::InputEventArg * arg)
{
	JoystickPovEventArg & jparg = *dynamic_cast<JoystickPovEventArg *>(arg);
}

void LocalPlayer::OnJoystickBtnDown(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

void LocalPlayer::OnJoystickBtnUp(my::InputEventArg * arg)
{
	JoystickBtnEventArg & jbarg = *dynamic_cast<JoystickBtnEventArg *>(arg);
}

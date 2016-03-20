#include "StdAfx.h"
#include "Character.h"
#include "../Game.h"

using namespace my;

Character::Character(void)
	: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), PhysXContext::Gravity, my::Vector3(0,0,0), 1, 0.8f)
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
	cDesc.radius = 0.3f;
	cDesc.height = 1.5f;
	cDesc.position.y = 5;
	cDesc.material = Game::getSingleton().m_PxMaterial.get();
	cDesc.callback = this;
	cDesc.behaviorCallback = this;
	m_controller.reset(
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_PxScene.get(), cDesc));
}

void Character::Update(float fElapsedTime)
{
	addVelocity(acceleration * fElapsedTime);
	_stprintf_s(&Game::getSingleton().m_ScrInfos[6][0], Game::getSingleton().m_ScrInfos[6].size(), _T("%f, %f"), velocity.y, damping);
	m_controller->move((PxVec3&)(velocity * fElapsedTime), 0.001f, fElapsedTime, PxControllerFilters());
	setPosition(Vector3((float)m_controller->getPosition().x, (float)m_controller->getPosition().y, (float)m_controller->getPosition().z));
}

void Character::Destroy(void)
{
	m_controller.reset();
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

void LocalPlayer::OnKeyUp(my::InputEventArg * arg)
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

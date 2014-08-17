#include "StdAfx.h"
#include "Character.h"
#include "../Game.h"

using namespace my;

Character::Character(void)
	: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), PhysXContext::Gravity, my::Vector3(0,0,0), 1, 0.8f)
	, m_LookDir(0)
	, m_MoveState(0)
	, m_IsOnGround(false)
{
}

Character::~Character(void)
{
	Destroy();
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
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_Scene.get(), cDesc));
}

void Character::Update(float fElapsedTime)
{
	const float MoveSpeed = 5;
	if (m_IsOnGround)
	{
		switch (m_MoveState)
		{
		case MoveStateFront:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(0));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(0));
			damping = 1.0f;
			break;
		case MoveStateFront | MoveStateLeft:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(45));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(45));
			damping = 1.0f;
			break;
		case MoveStateLeft:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(90));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(90));
			damping = 1.0f;
			break;
		case MoveStateLeft | MoveStateBack:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(135));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(135));
			damping = 1.0f;
			break;
		case MoveStateBack:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(180));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(180));
			damping = 1.0f;
			break;
		case MoveStateBack | MoveStateRight:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(225));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(225));
			damping = 1.0f;
			break;
		case MoveStateRight:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(270));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(270));
			damping = 1.0f;
			break;
		case MoveStateRight | MoveStateFront:
			velocity.x = MoveSpeed * sin(m_LookDir + D3DXToRadian(315));
			velocity.z = MoveSpeed * cos(m_LookDir + D3DXToRadian(315));
			damping = 1.0f;
			break;
		default:
			damping = 0.000001f;
			break;
		}
	}
	else
		damping = 1.0f;
	addVelocity(acceleration * fElapsedTime);
	velocity *= pow(damping, fElapsedTime);
	Game::getSingleton().m_ScrInfos[1] = str_printf(_T("%f, %f"), velocity.y, damping);
	m_IsOnGround = false;
	m_controller->move((PxVec3&)(velocity * fElapsedTime), 0.001f, fElapsedTime, PxControllerFilters());
	setPosition(Vector3(m_controller->getPosition().x, m_controller->getPosition().y, m_controller->getPosition().z));
}

void Character::Destroy(void)
{
	m_controller.reset();
}

void Character::AddMoveState(MoveState state)
{
	m_MoveState |= state;
}

void Character::RemoveMoveState(MoveState state)
{
	m_MoveState &= ~state;
}

void Character::Jump(void)
{
	if (m_IsOnGround)
	{
		velocity.y = 5;
		damping = 1.0f;
	}
}

void Character::onShapeHit(const PxControllerShapeHit& hit)
{
	if (hit.dir.y < -0.5f)
	{
		velocity.y = 0;
		m_IsOnGround = true;
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

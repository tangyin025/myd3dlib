#include "StdAfx.h"
#include "Character.h"
#include "../Game.h"

using namespace my;

Character::Character(void)
	: Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), PhysXContext::Gravity, my::Vector3(0,0,0), 1, 0.8f)
	, m_LookDir(0)
	, m_MoveState(0)
	, m_CollisionState(0)
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
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_PxScene.get(), cDesc));
}

void Character::Update(float fElapsedTime)
{
	const float MoveSpeed = 5;
	if (m_CollisionState & CollisionStateGround)
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
	{
		damping = 0.8f;
	}
	addVelocity(acceleration * fElapsedTime);
	velocity *= pow(damping, fElapsedTime);
	//Game::getSingleton().m_ScrInfos[1] = str_printf(_T("%f, %f"), velocity.y, damping);
	m_CollisionState = 0;
	m_controller->move((PxVec3&)(velocity * fElapsedTime), 0.001f, fElapsedTime, PxControllerFilters());
	setPosition(Vector3((float)m_controller->getPosition().x, (float)m_controller->getPosition().y, (float)m_controller->getPosition().z));
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
	if (m_CollisionState & CollisionStateGround)
	{
		addVelocity(Vector3(0,5,0));
		m_CollisionState &= ~CollisionStateGround;
	}
}

void Character::onShapeHit(const PxControllerShapeHit& hit)
{
	if (hit.worldNormal.y > 0 && hit.worldNormal.y > fabs(hit.worldNormal.x) && hit.worldNormal.y > fabs(hit.worldNormal.z))
	{
		velocity.y = 0;
		m_CollisionState |= CollisionStateGround;
	}
	else
	{
		m_CollisionState |= CollisionStateWall;
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

#include "StdAfx.h"
#include "Character.h"
#include "../Game.h"

using namespace my;

void Character::Create(void)
{
	PxCapsuleControllerDesc cDesc;
	cDesc.radius = 0.3f;
	cDesc.height = 1.5f;
	cDesc.position.y = 5;
	cDesc.material = Game::getSingleton().m_PxMaterial.get();
	m_controller.reset(
		Game::getSingleton().m_ControllerMgr->createController(*Game::getSingleton().m_sdk, Game::getSingleton().m_Scene.get(), cDesc));
}

void Character::Update(float fElapsedTime)
{
	Vector3 resultingAcc = acceleration + forceAccum * inverseMass;
	addVelocity(resultingAcc * fElapsedTime);		
	velocity *= pow(damping, fElapsedTime);
	m_controller->move((PxVec3&)(velocity * fElapsedTime), 0.001f, fElapsedTime, PxControllerFilters());
}

void Character::Destroy(void)
{
	m_controller.reset();
}

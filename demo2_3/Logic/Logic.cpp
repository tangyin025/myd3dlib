#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

void Logic::Create(void)
{
	// ========================================================================================================
	// ÎïÀí³¡¾°
	// ========================================================================================================
	my::IStreamPtr ifs = Game::getSingleton().OpenIStream("mesh/scene_tm.phy");
	PxRigidActor * actor = Game::getSingleton().m_sdk->createRigidStatic(PxTransform::createIdentity());
	PxShape * shape = actor->createShape(PxTriangleMeshGeometry(physx_ptr<PxTriangleMesh>(Game::getSingleton().CreateTriangleMesh(ifs)).get()), *Game::getSingleton().m_PxMaterial);
	shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
	Game::getSingleton().m_Scene->addActor(*actor);

	m_LocalPlayer->Create();
}

void Logic::Update(float fElapsedTime)
{
	m_LocalPlayer->Update(fElapsedTime);
}

void Logic::Destroy(void)
{
	m_LocalPlayer->Destroy();
}

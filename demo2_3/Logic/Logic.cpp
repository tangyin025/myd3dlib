#include "StdAfx.h"
#include "Logic.h"
#include "../Game.h"

using namespace my;

void Logic::Create(void)
{
	OnEnterState();
}

void Logic::Update(float fElapsedTime)
{
	switch(m_State)
	{
	case LogicStateMain:
		{
			Game::getSingleton().PushGrid();
			m_LocalPlayer->Update(fElapsedTime);
		}
		break;
	default:
		break;
	}
}

void Logic::Destroy(void)
{
	OnLeaveState();
}

void Logic::ShiftState(LogicState State)
{
	if (State != m_State)
	{
		OnLeaveState();
		m_State = State;
		OnEnterState();
	}
}

void Logic::OnEnterState(void)
{
	switch(m_State)
	{
	case LogicStateMain:
		{
			Game::getSingleton().ExecuteCode("dofile \"StateMain.lua\"");
			//Game::getSingleton().LoadMeshSetAsync("mesh/scene.mesh.xml", boost::bind(&Logic::OnSceneMeshLoaded, this, _1));
			my::IStreamPtr ifs = Game::getSingleton().OpenIStream("mesh/scene_tm.phy");
			m_StaticSceneActor.reset(Game::getSingleton().m_sdk->createRigidStatic(PxTransform::createIdentity()));
			PxShape * shape = m_StaticSceneActor->createShape(
				PxTriangleMeshGeometry(physx_ptr<PxTriangleMesh>(Game::getSingleton().CreateTriangleMesh(ifs)).get()), *Game::getSingleton().m_PxMaterial);
			shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
			Game::getSingleton().m_Scene->addActor(*m_StaticSceneActor);
			m_LocalPlayer->Create();
		}
		break;
	default:
		break;
	}
}

void Logic::OnLeaveState(void)
{
	switch(m_State)
	{
	case LogicStateMain:
		{
			m_StaticSceneActor.reset();
			m_LocalPlayer->Destroy();
		}
		break;
	default:
		break;
	}
}
//
//void Logic::OnSceneMeshLoaded(my::DeviceRelatedObjectBasePtr res)
//{
//	m_SceneMeshSet = boost::dynamic_pointer_cast<OgreMeshSet>(res);
//
//	OgreMeshSet::iterator mesh_iter = m_SceneMeshSet->begin();
//	for(; mesh_iter != m_SceneMeshSet->end(); mesh_iter++)
//	{
//		MeshComponentPtr cmp(new MeshComponent((*mesh_iter)->m_aabb));
//		cmp->m_Mesh = *mesh_iter;
//		Game::getSingleton().m_OctScene->PushComponent(cmp, 0.1f);
//	}
//}

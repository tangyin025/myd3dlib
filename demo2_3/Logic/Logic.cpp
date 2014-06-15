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
	case LogicStateLoading:
		{
			Game::getSingleton().PushGrid();
		}
		break;
	case LogicStateMain:
		{
			Game::getSingleton().PushGrid();
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
	OnLeaveState();
	m_State = State;
	OnEnterState();
}

void Logic::OnEnterState(void)
{
	switch(m_State)
	{
	case LogicStateLoading:
		{
			Game::getSingleton().ExecuteCode("dofile \"StateLoading.lua\"");

			Game::getSingleton().LoadMeshSetAsync("mesh/scene.mesh.xml", boost::bind(&Logic::OnSceneMeshLoaded, this, _1));
		}
		break;
	case LogicStateMain:
		{
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
	case LogicStateLoading:
		{
		}
		break;
	case LogicStateMain:
		{
		}
		break;
	default:
		break;
	}
}

void Logic::OnSceneMeshLoaded(my::DeviceRelatedObjectBasePtr res)
{
	m_SceneMeshSet = boost::dynamic_pointer_cast<OgreMeshSet>(res);

	OgreMeshSet::iterator mesh_iter = m_SceneMeshSet->begin();
	for(; mesh_iter != m_SceneMeshSet->end(); mesh_iter++)
	{
		MeshComponentPtr cmp(new MeshComponent((*mesh_iter)->m_aabb));
		cmp->m_Mesh = *mesh_iter;
		Game::getSingleton().m_OctScene->PushComponent(cmp, 0.1f);
	}
}

#include "StdAfx.h"
#include "GameState.h"
#include "Scene.h"
//
//#ifdef _DEBUG
//#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
//#endif

using namespace my;

GameStateMain::GameStateMain(void)
{
	Game::getSingleton().ExecuteCode("dofile(\"demo2_3.lua\")");

	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration.get()));
	m_overlappingPairCache.reset(new btAxisSweep3(btVector3(-1000,-1000,-1000), btVector3(1000,1000,1000)));
	m_constraintSolver.reset(new btSequentialImpulseConstraintSolver());
	m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(
		m_dispatcher.get(), m_overlappingPairCache.get(), m_constraintSolver.get(), m_collisionConfiguration.get()));

	m_collisionShapes;

	m_camera.reset(new ModuleViewCamera(D3DXToRadian(75), 4/3.0f, 0.1f, 3000.0f));
	m_camera->m_Rotation = Vector3(D3DXToRadian(-45), D3DXToRadian(45), 0);
	m_camera->m_Distance = 10.0f;
}

GameStateMain::~GameStateMain(void)
{
}

void GameStateMain::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_dynamicsWorld->stepSimulation(fElapsedTime, 10);

	m_camera->OnFrameMove(fTime, fElapsedTime);
}

void GameStateMain::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 161, 161, 161), 1, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_camera->m_Proj);

		BaseScene::DrawLine(pd3dDevice, Vector3(-10,0,0), Vector3(10,0,0), D3DCOLOR_ARGB(255,0,0,0));
		BaseScene::DrawLine(pd3dDevice, Vector3(0,0,-10), Vector3(0,0,10), D3DCOLOR_ARGB(255,0,0,0));
		for(int i = 1; i <= 10; i++)
		{
			BaseScene::DrawLine(pd3dDevice, Vector3(-10,0, (float)i), Vector3(10,0, (float)i), D3DCOLOR_ARGB(255,127,127,127));
			BaseScene::DrawLine(pd3dDevice, Vector3(-10,0,-(float)i), Vector3(10,0,-(float)i), D3DCOLOR_ARGB(255,127,127,127));
			BaseScene::DrawLine(pd3dDevice, Vector3( (float)i,0,-10), Vector3( (float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
			BaseScene::DrawLine(pd3dDevice, Vector3(-(float)i,0,-10), Vector3(-(float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
		}

		V(pd3dDevice->EndScene());
	}
}

LRESULT GameStateMain::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

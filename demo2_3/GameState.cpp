#include "StdAfx.h"
#include "GameState.h"
#include "Scene.h"
//
//#ifdef _DEBUG
//#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
//#endif

using namespace my;

HRESULT GameStateLoad::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnD3D9CreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	Game::getSingleton().ExecuteCode("dofile(\"demo2_3.lua\")");

	return S_OK;
}

HRESULT GameStateLoad::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnD3D9ResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	return S_OK;
}

void GameStateLoad::OnD3D9LostDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnD3D9LostDevice", D3DCOLOR_ARGB(255,255,255,0));
}

void GameStateLoad::OnD3D9DestroyDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnD3D9DestroyDevice", D3DCOLOR_ARGB(255,255,255,0));
}

void GameStateLoad::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Game::getSingleton().ExecuteCode("game:process_event(GameEventLoadOver())");
}

void GameStateLoad::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 255), 1, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		V(pd3dDevice->EndScene());
	}
}

LRESULT GameStateLoad::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

HRESULT GameStatePlay::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStatePlay::OnD3D9CreateDevice", D3DCOLOR_ARGB(255,255,255,0));

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

	return S_OK;
}

HRESULT GameStatePlay::OnD3D9ResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStatePlay::OnD3D9ResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	return S_OK;
}

void GameStatePlay::OnD3D9LostDevice(void)
{
	Game::getSingleton().AddLine(L"GameStatePlay::OnD3D9LostDevice", D3DCOLOR_ARGB(255,255,255,0));
}

void GameStatePlay::OnD3D9DestroyDevice(void)
{
	Game::getSingleton().AddLine(L"GameStatePlay::OnD3D9DestroyDevice", D3DCOLOR_ARGB(255,255,255,0));
}

void GameStatePlay::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_dynamicsWorld->stepSimulation(fElapsedTime, 10);

	m_camera->OnFrameMove(fTime, fElapsedTime);
}

void GameStatePlay::OnD3D9FrameRender(
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

LRESULT GameStatePlay::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

#include "StdAfx.h"
#include "GameState.h"

GameStateLoad::GameStateLoad(void)
{
	//THROW_CUSEXCEPTION("aaa");
}

GameStateLoad::~GameStateLoad(void)
{
}

HRESULT GameStateLoad::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnD3D9CreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	Game::getSingleton().ExecuteCode("dofile(\"demo2_3.lua\")");

	//THROW_CUSEXCEPTION("aaa");

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
}

void GameStateLoad::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 255, 72), 1, 0));

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

GameStatePlay::GameStatePlay(void)
{
	//THROW_CUSEXCEPTION("aaa");
}

GameStatePlay::~GameStatePlay(void)
{
}

HRESULT GameStatePlay::OnD3D9CreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStatePlay::OnD3D9CreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	//THROW_CUSEXCEPTION("aaa");

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
}

void GameStatePlay::OnD3D9FrameRender(
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

LRESULT GameStatePlay::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

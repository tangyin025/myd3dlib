#include "Scene.h"
#include "Game.h"

Scene::Scene(void)
{
}

void Scene::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
}

void Scene::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	if(m_camera)
	{
		m_camera->UpdateViewProj();

		HRESULT hr;
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 255, 72, 72), 1, 0));
	}
}

#include "StdAfx.h"
#include "GameState.h"
//
//#ifdef _DEBUG
//#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
//#endif

using namespace my;

GameStateLoad::GameStateLoad(void)
{
}

GameStateLoad::~GameStateLoad(void)
{
}

void GameStateLoad::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Game::getSingleton().ExecuteCode("game:process_event(GameEventLoadOver())");

	Game::getSingleton().ExecuteCode("dofile(\"demo2_3.lua\")");

	Game::getSingleton().CurrentState()->OnFrameMove(fTime, fElapsedTime);
}

void GameStateLoad::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 255), 1, 0));
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

GameStateMain::GameStateMain(void)
{
	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration.get()));
	m_overlappingPairCache.reset(new btAxisSweep3(btVector3(-1000,-1000,-1000), btVector3(1000,1000,1000)));
	m_constraintSolver.reset(new btSequentialImpulseConstraintSolver());
	m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(
		m_dispatcher.get(), m_overlappingPairCache.get(), m_constraintSolver.get(), m_collisionConfiguration.get()));

	m_Camera.reset(new ModuleViewCamera(D3DXToRadian(75), 4/3.0f, 0.1f, 3000.0f));
	m_Camera->m_Rotation = Vector3(D3DXToRadian(-45), D3DXToRadian(45), 0);
	m_Camera->m_Distance = 10.0f;

	m_CubeMapFaces[D3DCUBEMAP_FACE_POSITIVE_X] = Game::getSingleton().LoadTexture("cubescene_rt.jpg");
	m_CubeMapFaces[D3DCUBEMAP_FACE_NEGATIVE_X] = Game::getSingleton().LoadTexture("cubescene_lf.jpg");
	m_CubeMapFaces[D3DCUBEMAP_FACE_POSITIVE_Y] = Game::getSingleton().LoadTexture("cubescene_up.jpg");
	m_CubeMapFaces[D3DCUBEMAP_FACE_NEGATIVE_Y] = Game::getSingleton().LoadTexture("cubescene_dn.jpg");
	m_CubeMapFaces[D3DCUBEMAP_FACE_POSITIVE_Z] = Game::getSingleton().LoadTexture("cubescene_fr.jpg");
	m_CubeMapFaces[D3DCUBEMAP_FACE_NEGATIVE_Z] = Game::getSingleton().LoadTexture("cubescene_bk.jpg");
}

GameStateMain::~GameStateMain(void)
{
}

void GameStateMain::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_dynamicsWorld->stepSimulation(fElapsedTime, 10);

	m_Camera->OnFrameMove(fTime, fElapsedTime);

	CharacterPtrList::iterator character_iter = m_characters.begin();
	for(; character_iter != m_characters.end(); character_iter++)
	{
		(*character_iter)->OnFrameMove(fTime, fElapsedTime);
	}
}

void GameStateMain::OnD3D9FrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	Effect * SimpleSample = Game::getSingleton().m_SimpleSample.get();
	CComPtr<IDirect3DSurface9> oldRt;
	V(pd3dDevice->GetRenderTarget(0, &oldRt));
	CComPtr<IDirect3DSurface9> oldDs;
	V(pd3dDevice->GetDepthStencilSurface(&oldDs));

	Vector4 LightDir(1,1,-1,0);
	((Vector3 &)LightDir).normalizeSelf();
	Vector3 LightTag(0,1,0);
	Matrix4 LightViewProj =
		Matrix4::LookAtLH(LightTag + LightDir, LightTag, Vector3(0,1,0)) *
		Matrix4::OrthoLH(3, 3, -50, 50);

	V(pd3dDevice->SetRenderTarget(0, Game::getSingleton().m_ShadowMapRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(Game::getSingleton().m_ShadowMapDS->m_ptr));
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		Effect * ShadowMap = Game::getSingleton().m_ShadowMap.get();
		ShadowMap->SetTechnique("RenderSkinedShadow");
		CharacterPtrList::iterator character_iter = m_characters.begin();
		for(; character_iter != m_characters.end(); character_iter++)
		{
			Matrix4 world =
				Matrix4::Scaling((*character_iter)->m_Scale) *
				Matrix4::RotationQuaternion((*character_iter)->m_Rotation) *
				Matrix4::Translation((*character_iter)->m_Position);
			ShadowMap->SetMatrix("g_mWorldViewProjection", world * LightViewProj);
			SimpleSample->SetMatrixArray("g_dualquat", &(*character_iter)->m_dualQuaternionList[0], (*character_iter)->m_dualQuaternionList.size());
			EffectMesh * mesh = (*character_iter)->m_meshLOD[(*character_iter)->m_LODLevel].get();
			UINT cPasses = ShadowMap->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				ShadowMap->BeginPass(p);
				for(UINT i = 0; i < mesh->GetMaterialNum(); i++)
				{
					mesh->DrawSubset(i);
				}
				ShadowMap->EndPass();
			}
			ShadowMap->End();
		}

		V(pd3dDevice->EndScene());
	}

	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
	V(pd3dDevice->SetDepthStencilSurface(Game::getSingleton().m_CubeMapDS->m_ptr));
	for(DWORD Face = 0; Face < _countof(m_CubeMapFaces); Face++)
	{
		Game::getSingleton().m_CubeMapRT->DrawToSurface(pd3dDevice, (D3DCUBEMAP_FACES)Face, m_CubeMapFaces[Face]);
	}
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));

	V(pd3dDevice->SetRenderTarget(0, oldRt));
	V(pd3dDevice->SetDepthStencilSurface(oldDs));
	oldRt.Release();
	oldDs.Release();

	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 161, 161, 161), 1, 0));

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);

		//DrawLine(pd3dDevice, Vector3(-10,0,0), Vector3(10,0,0), D3DCOLOR_ARGB(255,0,0,0));
		//DrawLine(pd3dDevice, Vector3(0,0,-10), Vector3(0,0,10), D3DCOLOR_ARGB(255,0,0,0));
		//for(int i = 1; i <= 10; i++)
		//{
		//	DrawLine(pd3dDevice, Vector3(-10,0, (float)i), Vector3(10,0, (float)i), D3DCOLOR_ARGB(255,127,127,127));
		//	DrawLine(pd3dDevice, Vector3(-10,0,-(float)i), Vector3(10,0,-(float)i), D3DCOLOR_ARGB(255,127,127,127));
		//	DrawLine(pd3dDevice, Vector3( (float)i,0,-10), Vector3( (float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
		//	DrawLine(pd3dDevice, Vector3(-(float)i,0,-10), Vector3(-(float)i,0,10), D3DCOLOR_ARGB(255,127,127,127));
		//}

		SimpleSample->SetFloat("g_fTime", (float)Game::getSingleton().GetTime());
		SimpleSample->SetMatrix("g_mWorld", Matrix4::Identity());
		SimpleSample->SetMatrix("g_mWorldViewProjection", m_Camera->m_View * m_Camera->m_Proj);
		SimpleSample->SetMatrix("g_mLightViewProjection", LightViewProj);
		SimpleSample->SetVector("g_EyePos", m_Camera->m_View.inverse()[3]); // ! Need optimize
		SimpleSample->SetVector("g_LightDir", LightDir);
		SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));
		SimpleSample->SetTexture("g_CubeTexture", Game::getSingleton().m_CubeMapRT->m_ptr);
		SimpleSample->SetTexture("g_ShadowTexture", Game::getSingleton().m_ShadowMapRT->m_ptr);
		EffectMeshPtrList::iterator effect_mesh_iter = m_staticMeshes.begin();
		for(; effect_mesh_iter != m_staticMeshes.end(); effect_mesh_iter++)
		{
			(*effect_mesh_iter)->Draw(pd3dDevice, fElapsedTime);
		}

		CharacterPtrList::iterator character_iter = m_characters.begin();
		for(; character_iter != m_characters.end(); character_iter++)
		{
			Matrix4 world =
				Matrix4::Scaling((*character_iter)->m_Scale) *
				Matrix4::RotationQuaternion((*character_iter)->m_Rotation) *
				Matrix4::Translation((*character_iter)->m_Position);
			SimpleSample->SetMatrix("g_mWorld", world);
			SimpleSample->SetMatrix("g_mWorldViewProjection", world * m_Camera->m_View * m_Camera->m_Proj);
			(*character_iter)->Draw(pd3dDevice, fElapsedTime);
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

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

void GameStateLoad::OnFrameRender(
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

void GameStateMain::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	CComPtr<IDirect3DSurface9> oldRt;
	V(pd3dDevice->GetRenderTarget(0, &oldRt));
	CComPtr<IDirect3DSurface9> oldDs;
	V(pd3dDevice->GetDepthStencilSurface(&oldDs));
	Effect * SimpleSample = Game::getSingleton().m_SimpleSample.get();

	Vector4 LightDir(1,1,-1,0);
	((Vector3 &)LightDir).normalizeSelf();
	Vector3 LightTag(0,1,0);
	Matrix4 LightViewProj =
		Matrix4::LookAtLH(LightTag + LightDir, LightTag, Vector3(0,1,0)) *
		Matrix4::OrthoLH(3, 3, -50, 50);
	Vector4 EyePos = m_Camera->m_View.inverse()[3]; // ! Need optimize

	my::Texture * ShadowTextureRT = Game::getSingleton().m_ShadowTextureRT.get();
	V(pd3dDevice->SetRenderTarget(0, ShadowTextureRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(Game::getSingleton().m_ShadowTextureDS->m_ptr));
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

	my::Texture * ScreenTextureRT = Game::getSingleton().m_ScreenTextureRT.get();
	V(pd3dDevice->SetRenderTarget(0, ScreenTextureRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(Game::getSingleton().m_ScreenTextureDS->m_ptr));
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

		Matrix4 world = Matrix4::Identity();
		SimpleSample->SetFloat("g_fTime", (float)fTime);
		SimpleSample->SetMatrix("g_mWorld", world);
		SimpleSample->SetMatrix("g_mWorldViewProjection", world * m_Camera->m_View * m_Camera->m_Proj);
		SimpleSample->SetMatrix("g_mLightViewProjection", LightViewProj);
		SimpleSample->SetVector("g_EyePos", EyePos);
		SimpleSample->SetVector("g_EyePosOS", EyePos.transform(world.inverse()));
		SimpleSample->SetVector("g_LightDir", LightDir);
		SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));
		SimpleSample->SetTexture("g_ShadowTexture", ShadowTextureRT->m_ptr);
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
			SimpleSample->SetVector("g_EyePosOS", EyePos.transform(world.inverse()));
			(*character_iter)->Draw(pd3dDevice, fElapsedTime);
		}

		V(pd3dDevice->EndScene());
	}

	V(pd3dDevice->SetRenderTarget(0, oldRt));
	V(pd3dDevice->SetDepthStencilSurface(oldDs));
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));
	//if(SUCCEEDED(hr = pd3dDevice->StretchRect(oldRt, NULL, ScreenTextureRT->GetSurfaceLevel(0), NULL, D3DTEXF_LINEAR)))
	{
		D3DSURFACE_DESC desc = ScreenTextureRT->GetLevelDesc(0);
		struct Vertex
		{
			FLOAT x, y, z, w;
			FLOAT u, v;
		};

		Vertex quad[4] =
		{
			{ -0.5f,					-0.5f,						0.5f, 1.0f, 0.0f, 0.0f },
			{ (FLOAT)desc.Width - 0.5f,	-0.5f,						0.5f, 1.0f, 1.0f, 0.0f },
			{ -0.5f,					(FLOAT)desc.Height - 0.5f,	0.5f, 1.0f, 0.0f, 1.0f },
			{ (FLOAT)desc.Width - 0.5f,	(FLOAT)desc.Height - 0.5f,	0.5f, 1.0f, 1.0f, 1.0f },
		};

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			V(pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
			V(pd3dDevice->SetTexture(0, ScreenTextureRT->m_ptr));
			V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0])));
			V(pd3dDevice->EndScene());
		}
	}
	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE));
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

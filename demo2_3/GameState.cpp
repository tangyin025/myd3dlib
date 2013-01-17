#include "StdAfx.h"
#include "GameState.h"
//
//#ifdef _DEBUG
//#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
//#endif

using namespace my;
//
//void DrawHelper::DrawLine(
//	IDirect3DDevice9 * pd3dDevice,
//	const Vector3 & v0,
//	const Vector3 & v1,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	Vertex v[2];
//	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
//	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawSphere(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	DrawSpereStage(pd3dDevice, radius, 0, 20, 0, Color, world);
//}
//
//void DrawHelper::DrawBox(
//	IDirect3DDevice9 * pd3dDevice,
//	const Vector3 & halfSize,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	Vertex v[8];
//	v[0].x = -halfSize.x; v[0].y = -halfSize.y; v[0].z = -halfSize.z; v[0].color = Color;
//	v[1].x =  halfSize.x; v[1].y = -halfSize.y; v[1].z = -halfSize.z; v[1].color = Color;
//	v[2].x = -halfSize.x; v[2].y =  halfSize.y; v[2].z = -halfSize.z; v[2].color = Color;
//	v[3].x =  halfSize.x; v[3].y =  halfSize.y; v[3].z = -halfSize.z; v[3].color = Color;
//	v[4].x = -halfSize.x; v[4].y =  halfSize.y; v[4].z =  halfSize.z; v[4].color = Color;
//	v[5].x =  halfSize.x; v[5].y =  halfSize.y; v[5].z =  halfSize.z; v[5].color = Color;
//	v[6].x = -halfSize.x; v[6].y = -halfSize.y; v[6].z =  halfSize.z; v[6].color = Color;
//	v[7].x =  halfSize.x; v[7].y = -halfSize.y; v[7].z =  halfSize.z; v[7].color = Color;
//
//	unsigned short idx[12 * 2];
//	int i = 0;
//	idx[i++] = 0; idx[i++] = 1; idx[i++] = 1; idx[i++] = 3; idx[i++] = 3; idx[i++] = 2; idx[i++] = 2; idx[i++] = 0;
//	idx[i++] = 0; idx[i++] = 6; idx[i++] = 1; idx[i++] = 7; idx[i++] = 3; idx[i++] = 5; idx[i++] = 2; idx[i++] = 4;
//	idx[i++] = 6; idx[i++] = 7; idx[i++] = 7; idx[i++] = 5; idx[i++] = 5; idx[i++] = 4; idx[i++] = 4; idx[i++] = 6;
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, _countof(v), _countof(idx) / 2, idx, D3DFMT_INDEX16, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawTriangle(
//	IDirect3DDevice9 * pd3dDevice,
//	const Vector3 & v0,
//	const Vector3 & v1,
//	const Vector3 & v2,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	Vertex v[4];
//	v[0].x = v0.x; v[0].y = v0.y; v[0].z = v0.z; v[0].color = Color;
//	v[1].x = v1.x; v[1].y = v1.y; v[1].z = v1.z; v[1].color = Color;
//	v[2].x = v2.x; v[2].y = v2.y; v[2].z = v2.z; v[2].color = Color;
//	v[3] = v[0];
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, _countof(v) - 1, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawSpereStage(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	int VSTAGE_BEGIN,
//	int VSTAGE_END,
//	float offsetY,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	const int VSTAGE = 20;
//	const int HSTAGE = 20;
//	Vertex v[VSTAGE * HSTAGE * 4];
//	for(int j = VSTAGE_BEGIN; j < VSTAGE_END; j++)
//	{
//		for(int i = 0; i < HSTAGE; i++)
//		{
//			float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
//			float Fi[2] = {D3DX_PI / VSTAGE * j, D3DX_PI / VSTAGE * (j + 1)};
//			Vertex * pv = &v[(j * HSTAGE + i) * 4];
//			pv[0].x = radius * sin(Fi[0]) * cos(Theta[0]);
//			pv[0].y = radius * cos(Fi[0]) + offsetY;
//			pv[0].z = radius * sin(Fi[0]) * sin(Theta[0]);
//			pv[0].color = Color;
//
//			pv[1].x = radius * sin(Fi[0]) * cos(Theta[1]);
//			pv[1].y = radius * cos(Fi[0]) + offsetY;
//			pv[1].z = radius * sin(Fi[0]) * sin(Theta[1]);
//			pv[1].color = Color;
//
//			pv[2] = pv[0];
//
//			pv[3].x = radius * sin(Fi[1]) * cos(Theta[0]);
//			pv[3].y = radius * cos(Fi[1]) + offsetY;
//			pv[3].z = radius * sin(Fi[1]) * sin(Theta[0]);
//			pv[3].color = Color;
//		}
//	}
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawCylinderStage(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	float y0,
//	float y1,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	struct Vertex
//	{
//		float x, y, z;
//		D3DCOLOR color;
//	};
//
//	const int HSTAGE = 20;
//	Vertex v[HSTAGE * 4];
//	for(int i = 0; i < HSTAGE; i++)
//	{
//		float Theta[2] = {2 * D3DX_PI / HSTAGE * i, 2 * D3DX_PI / HSTAGE * (i + 1)};
//		Vertex * pv = &v[i * 4];
//		pv[0].x = radius * cos(Theta[0]);
//		pv[0].y = y0;
//		pv[0].z = radius * sin(Theta[0]);
//		pv[0].color = Color;
//
//		pv[1].x = radius * cos(Theta[1]);
//		pv[1].y = y0;
//		pv[1].z = radius * sin(Theta[1]);
//		pv[1].color = Color;
//
//		pv[2] = pv[0];
//
//		pv[3].x = radius * cos(Theta[0]);
//		pv[3].y = y1;
//		pv[3].z = radius * sin(Theta[0]);
//		pv[3].color = Color;
//	}
//
//	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
//	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
//	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&world);
//	pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, _countof(v) / 2, v, sizeof(v[0]));
//}
//
//void DrawHelper::DrawCapsule(
//	IDirect3DDevice9 * pd3dDevice,
//	float radius,
//	float height,
//	D3DCOLOR Color,
//	const Matrix4 & world)
//{
//	float y0 = height * 0.5f;
//	float y1 = -y0;
//	DrawSpereStage(pd3dDevice, radius, 0, 10, y0, Color, world);
//	DrawSpereStage(pd3dDevice, radius, 10, 20, y1, Color, world);
//	DrawCylinderStage(pd3dDevice, radius, y0, y1, Color, world);
//}

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
}

HRESULT GameStateLoad::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	Game::getSingleton().ExecuteCode("dofile \"GameStateLoad.lua\"");

	return S_OK;
}

HRESULT GameStateLoad::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	return S_OK;
}

void GameStateLoad::OnLostDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));
}

void GameStateLoad::OnDestroyDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateLoad::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));
}

void GameStateLoad::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,45,50,170), 1.0f, 0));
}

GameStateMain::GameStateMain(void)
{
}

GameStateMain::~GameStateMain(void)
{
}

HRESULT GameStateMain::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateMain::OnCreateDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration.get()));
	m_overlappingPairCache.reset(new btAxisSweep3(btVector3(-1000,-1000,-1000), btVector3(1000,1000,1000)));
	m_constraintSolver.reset(new btSequentialImpulseConstraintSolver());
	m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(
		m_dispatcher.get(), m_overlappingPairCache.get(), m_constraintSolver.get(), m_collisionConfiguration.get()));

	m_SimpleSample = Game::getSingleton().LoadEffect("SimpleSample.fx");

	m_ShadowMap = Game::getSingleton().LoadEffect("ShadowMap.fx");

	m_ShadowTextureRT.reset(new my::Texture());

	m_ShadowTextureDS.reset(new my::Surface());

	Game::getSingleton().ExecuteCode("dofile \"GameStateMain.lua\"");

	if(!m_Camera)
	{
		THROW_CUSEXCEPTION("camera must be created");
	}

	return S_OK;
}

HRESULT GameStateMain::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateMain::OnResetDevice", D3DCOLOR_ARGB(255,255,255,0));

	const DWORD SHADOW_MAP_SIZE = 512;
	m_ShadowTextureRT->CreateAdjustedTexture(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT);

	// ! 所有的 render target必须使用具有相同 multisample的 depth stencil
	//DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
	m_ShadowTextureDS->CreateDepthStencilSurface(
		pd3dDevice, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFMT_D24X8);

	if(m_Camera && m_Camera->EventAlign)
		m_Camera->EventAlign(EventArgsPtr(new EventArgs()));

	return S_OK;
}

void GameStateMain::OnLostDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateMain::OnLostDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_ShadowTextureRT->OnDestroyDevice();

	m_ShadowTextureDS->OnDestroyDevice();
}

void GameStateMain::OnDestroyDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateMain::OnDestroyDevice", D3DCOLOR_ARGB(255,255,255,0));

	m_staticMeshes.clear();

	m_characters.clear();
}

void GameStateMain::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_dynamicsWorld->stepSimulation(fElapsedTime, 0);

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

	Vector3 LightDir(Vector3(1,1,-1).normalize());
	Vector3 LightTag(0,1,0);
	Matrix4 LightViewProj =
		Matrix4::LookAtLH(LightTag + LightDir, LightTag, Vector3(0,1,0)) *
		Matrix4::OrthoLH(3, 3, -50, 50);
	Vector4 EyePos = m_Camera->m_View.inverse()[3]; // ! Need optimize

	V(pd3dDevice->SetRenderTarget(0, m_ShadowTextureRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowTextureDS->m_ptr));
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		m_ShadowMap->SetTechnique("RenderSkinedShadow");
		CharacterPtrList::iterator character_iter = m_characters.begin();
		for(; character_iter != m_characters.end(); character_iter++)
		{
			// ! for uninitialized dual quaternion list
			(*character_iter)->m_dualQuaternionList.resize((*character_iter)->m_skeletonLOD[(*character_iter)->m_LODLevel]->m_boneBindPose.size());

			Matrix4 world =
				Matrix4::Scaling((*character_iter)->m_Scale) *
				Matrix4::RotationQuaternion((*character_iter)->m_Rotation) *
				Matrix4::Translation((*character_iter)->m_Position);
			m_ShadowMap->SetMatrix("g_mWorldViewProjection", world * LightViewProj);
			m_SimpleSample->SetMatrixArray("g_dualquat", &(*character_iter)->m_dualQuaternionList[0], (*character_iter)->m_dualQuaternionList.size());
			EffectMesh * mesh = (*character_iter)->m_meshLOD[(*character_iter)->m_LODLevel].get();
			UINT cPasses = m_ShadowMap->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_ShadowMap->BeginPass(p);
				for(UINT i = 0; i < mesh->m_Mesh->GetMaterialNum(); i++)
				{
					mesh->m_Mesh->DrawSubset(i);
				}
				m_ShadowMap->EndPass();
			}
			m_ShadowMap->End();
		}

		V(pd3dDevice->EndScene());
	}

	V(pd3dDevice->SetRenderTarget(0, oldRt));
	V(pd3dDevice->SetDepthStencilSurface(oldDs));
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
		m_SimpleSample->SetFloat("g_fTime", (float)fTime);
		m_SimpleSample->SetMatrix("g_mWorld", world);
		m_SimpleSample->SetMatrix("g_mWorldViewProjection", world * m_Camera->m_View * m_Camera->m_Proj);
		m_SimpleSample->SetMatrix("g_mLightViewProjection", LightViewProj);
		m_SimpleSample->SetVector("g_EyePos", EyePos);
		m_SimpleSample->SetVector("g_EyePosOS", EyePos.transform(world.inverse()));
		m_SimpleSample->SetVector("g_LightDir", Vector4(LightDir.x, LightDir.y, LightDir.z, 0));
		m_SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));
		m_SimpleSample->SetTexture("g_ShadowTexture", m_ShadowTextureRT->m_ptr);
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
			m_SimpleSample->SetMatrix("g_mWorld", world);
			m_SimpleSample->SetMatrix("g_mWorldViewProjection", world * m_Camera->m_View * m_Camera->m_Proj);
			m_SimpleSample->SetVector("g_EyePosOS", EyePos.transform(world.inverse()));
			m_SimpleSample->SetMatrixArray("g_dualquat", &(*character_iter)->m_dualQuaternionList[0], (*character_iter)->m_dualQuaternionList.size());
			(*character_iter)->Draw(pd3dDevice, fElapsedTime);
		}

		V(pd3dDevice->EndScene());
	}
}

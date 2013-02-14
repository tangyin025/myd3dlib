#include "StdAfx.h"
#include "GameState.h"
//
//#ifdef _DEBUG
//#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
//#endif

using namespace my;

HRESULT GameStateInit::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateInit::OnCreateDevice", D3DCOLOR_ARGB(255,255,128,0));

	return S_OK;
}

HRESULT GameStateInit::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateInit::OnResetDevice", D3DCOLOR_ARGB(255,255,128,0));

	return S_OK;
}

void GameStateInit::OnLostDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateInit::OnLostDevice", D3DCOLOR_ARGB(255,255,128,0));
}

void GameStateInit::OnDestroyDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateInit::OnDestroyDevice", D3DCOLOR_ARGB(255,255,128,0));
}

void GameStateInit::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	Game::getSingleton().process_event(GameEventInit());
}

void GameStateInit::OnFrameRender(
	IDirect3DDevice9 * pd3dDevice,
	double fTime,
	float fElapsedTime)
{
	_ASSERT(false);
}

LRESULT GameStateInit::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	return 0;
}

HRESULT GameStateMain::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	Game::getSingleton().AddLine(L"GameStateMain::OnCreateDevice", D3DCOLOR_ARGB(255,255,128,0));

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
	Game::getSingleton().AddLine(L"GameStateMain::OnResetDevice", D3DCOLOR_ARGB(255,255,128,0));

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
	Game::getSingleton().AddLine(L"GameStateMain::OnLostDevice", D3DCOLOR_ARGB(255,255,128,0));

	m_ShadowTextureRT->OnDestroyDevice();

	m_ShadowTextureDS->OnDestroyDevice();
}

void GameStateMain::OnDestroyDevice(void)
{
	Game::getSingleton().AddLine(L"GameStateMain::OnDestroyDevice", D3DCOLOR_ARGB(255,255,128,0));

	m_staticMeshes.clear();

	m_characters.clear();
}

void GameStateMain::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	m_dynamicsWorld->stepSimulation(fElapsedTime, 4);

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

	Vector3 LightDir(Vector3(1,1,1).normalize());
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

LRESULT GameStateMain::MsgProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	bool * pbNoFurtherProcessing)
{
	LRESULT lr;
	if(lr = m_Camera->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
		return lr;

	return 0;
}

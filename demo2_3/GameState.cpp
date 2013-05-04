#include "StdAfx.h"
#include "GameState.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

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

	m_SimpleSample = Game::getSingleton().LoadEffect("shader/SimpleSample.fx");

	m_ShadowMap = Game::getSingleton().LoadEffect("shader/ShadowMap.fx");

	m_ShadowTextureRT.reset(new my::Texture());

	m_ShadowTextureDS.reset(new my::Surface());

	if(!(m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocator, m_DefaultErrorCallback)))
	{
		THROW_CUSEXCEPTION("PxCreateFoundation failed");
	}

	if(!(m_ProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_Foundation)))
	{
		THROW_CUSEXCEPTION("PxProfileZoneManager::createProfileZoneManager failed");
	}

	if(!(m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, m_ProfileZoneManager)))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!PxInitExtensions(*m_Physics))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	if(!(m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!(m_CpuDispatcher = PxDefaultCpuDispatcherCreate(1)))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}

	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_CpuDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_Scene = m_Physics->createScene(sceneDesc)))
	{
		THROW_CUSEXCEPTION("m_Physics->createScene failed");
	}

	if(!Game::getSingleton().ExecuteCode("dofile \"GameStateMain.lua\""))
	{
		return E_FAIL;
	}

	_ASSERT(m_Camera);

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

	if(m_Camera->EventAlign)
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

	m_StaticMeshes.clear();

	m_Characters.clear();

	if(m_Scene)
		m_Scene->release();

	if(m_CpuDispatcher)
		m_CpuDispatcher->release();

	if(m_Cooking)
		m_Cooking->release();

	PxCloseExtensions();

	if(m_Physics)
		m_Physics->release();

	if(m_ProfileZoneManager)
		m_ProfileZoneManager->release();

	if(m_Foundation)
		m_Foundation->release();
}

void GameStateMain::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	EmitterMgr::Update(fTime, fElapsedTime);

	m_Camera->OnFrameMove(fTime, fElapsedTime);

	CharacterPtrList::iterator character_iter = m_Characters.begin();
	for(; character_iter != m_Characters.end(); character_iter++)
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
		Matrix4::LookAtRH(LightTag + LightDir, LightTag, Vector3(0,1,0)) *
		Matrix4::OrthoRH(3, 3, -50, 50);

	V(pd3dDevice->SetRenderTarget(0, m_ShadowTextureRT->GetSurfaceLevel(0)));
	V(pd3dDevice->SetDepthStencilSurface(m_ShadowTextureDS->m_ptr));
	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		m_ShadowMap->SetTechnique("RenderSkinedShadow");
		CharacterPtrList::iterator character_iter = m_Characters.begin();
		for(; character_iter != m_Characters.end(); character_iter++)
		{
			// ! for uninitialized dual quaternion list
			(*character_iter)->m_dualQuaternionList.resize((*character_iter)->m_Skeleton->m_boneBindPose.size());

			Matrix4 World =
				Matrix4::Scaling((*character_iter)->m_Scale) *
				Matrix4::RotationQuaternion((*character_iter)->m_Rotation) *
				Matrix4::Translation((*character_iter)->m_Position);
			m_ShadowMap->SetMatrix("g_World", World);
			m_ShadowMap->SetMatrix("g_ViewProj", LightViewProj);
			m_SimpleSample->SetMatrixArray("g_dualquat", &(*character_iter)->m_dualQuaternionList[0], (*character_iter)->m_dualQuaternionList.size());
			OgreMesh * mesh = (*character_iter)->m_Mesh.get();
			UINT cPasses = m_ShadowMap->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_ShadowMap->BeginPass(p);
				for(UINT i = 0; i < mesh->GetMaterialNum(); i++)
				{
					mesh->DrawSubset(i);
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

		Matrix4 World = Matrix4::Identity();
		m_SimpleSample->SetFloat("g_Time", (float)fTime);
		m_SimpleSample->SetMatrix("g_World", World);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		m_SimpleSample->SetMatrix("g_ViewProjLS", LightViewProj);
		m_SimpleSample->SetVector("g_EyePos", m_Camera->m_Position);
		m_SimpleSample->SetVector("g_EyePosOS", m_Camera->m_Position.transformCoord(World.inverse()));
		m_SimpleSample->SetVector("g_LightDir", LightDir);
		m_SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));
		m_SimpleSample->SetTexture("g_ShadowTexture", m_ShadowTextureRT->m_ptr);
		OgreMeshPtrList::iterator mesh_iter = m_StaticMeshes.begin();
		for(; mesh_iter != m_StaticMeshes.end(); mesh_iter++)
		{
			DWORD i = 0;
			for(; i < (*mesh_iter)->m_MaterialNameList.size(); i++)
			{
				Game::MaterialPtrMap::iterator mat_iter =
					Game::getSingleton().m_MaterialMap.find((*mesh_iter)->m_MaterialNameList[i]);
				if(mat_iter != Game::getSingleton().m_MaterialMap.end())
				{
					mat_iter->second->DrawMeshSubset(mesh_iter->get(), i);
				}
			}
		}

		CharacterPtrList::iterator character_iter = m_Characters.begin();
		for(; character_iter != m_Characters.end(); character_iter++)
		{
			Matrix4 World =
				Matrix4::Scaling((*character_iter)->m_Scale) *
				Matrix4::RotationQuaternion((*character_iter)->m_Rotation) *
				Matrix4::Translation((*character_iter)->m_Position);
			m_SimpleSample->SetMatrix("g_World", World);
			m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
			m_SimpleSample->SetVector("g_EyePosOS", m_Camera->m_Position.transformCoord(World.inverse()));
			m_SimpleSample->SetMatrixArray("g_dualquat", &(*character_iter)->m_dualQuaternionList[0], (*character_iter)->m_dualQuaternionList.size());
			DWORD i = 0;
			for(; i < (*character_iter)->m_Mesh->m_MaterialNameList.size(); i++)
			{
				Game::MaterialPtrMap::iterator mat_iter =
					Game::getSingleton().m_MaterialMap.find((*character_iter)->m_Mesh->m_MaterialNameList[i]);
				if(mat_iter != Game::getSingleton().m_MaterialMap.end())
				{
					mat_iter->second->DrawMeshSubset((*character_iter)->m_Mesh.get(), i);
				}
			}
		}

		Game::getSingleton().m_EmitterInst->Begin();
		EmitterMgr::Draw(Game::getSingleton().m_EmitterInst.get(), m_Camera.get(), fTime, fElapsedTime);
		Game::getSingleton().m_EmitterInst->End();

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

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

	if(!PhysxScene::OnInit())
	{
		return E_FAIL;
	}

	m_SimpleSample = Game::getSingleton().LoadEffect("shader/SimpleSample.fx");

	m_CheckerTexture = Game::getSingleton().LoadTexture("texture/Checker.bmp");

	m_ShadowMap = Game::getSingleton().LoadEffect("shader/ShadowMap.fx");

	m_ShadowTextureRT.reset(new my::Texture());

	m_ShadowTextureDS.reset(new my::Surface());

	if(!Game::getSingleton().ExecuteCode("dofile \"GameStateMain.lua\""))
	{
		return E_FAIL;
	}

	_ASSERT(m_Camera);

	/************************************************************************/
	/* 物理 sample 示例                                                     */
	/************************************************************************/
	PhysxPtr<PxRigidActor> actor;
	if(!(actor.reset(PxCreatePlane(*PhysxSample::getSingleton().m_Physics, PxPlane(PxVec3(0,0,0), PxVec3(0,1,0)), *m_Material)),
		actor))
	{
		THROW_CUSEXCEPTION("PxCreatePlane failed");
	}
	m_Scene->addActor(*actor);
	m_Actors.push_back(static_pointer_cast<PxActor>(actor));

	//if(!(actor.reset(PxCreateDynamic(*PhysxSample::getSingleton().m_Physics, PxTransform(PxVec3(0,100,0)), PxSphereGeometry(1), *m_Material, 1)),
	//	actor))
	//{
	//	THROW_CUSEXCEPTION("PxCreateDynamic failed");
	//}
	//m_Scene->addActor(*actor);
	//m_Actors.push_back(static_pointer_cast<PxActor>(actor));

	//my::OgreMeshPtr mesh = Game::getSingleton().LoadMesh("mesh/plane.mesh.xml");
	//void * pVertices = mesh->LockVertexBuffer();
	//void * pIndices = mesh->LockIndexBuffer();

	//PxTriangleMeshDesc meshDesc;
	//meshDesc.points.count = mesh->GetNumVertices();
	//meshDesc.points.stride = mesh->GetNumBytesPerVertex();
	//meshDesc.points.data = pVertices;

	//meshDesc.triangles.count = mesh->GetNumFaces();
	//_ASSERT(mesh->GetOptions() & D3DXMESH_32BIT);
	//meshDesc.triangles.stride = 3 * sizeof(DWORD);
	//meshDesc.triangles.data = pIndices;

	//PhysxPtr<PxTriangleMesh> triMesh(PxToolkit::createTriangleMesh32(*PhysxSample::getSingleton().m_Physics, *PhysxSample::getSingleton().m_Cooking, &meshDesc));
	//if(!triMesh)
	//{
	//	THROW_CUSEXCEPTION("PxToolkit::createTriangleMesh32 failed");
	//}

	//if(!(actor.reset(PxCreateStatic(*PhysxSample::getSingleton().m_Physics, PxTransform::createIdentity(), PxTriangleMeshGeometry(triMesh.get(), PxMeshScale()), *m_Material)),
	//	actor))
	//{
	//	THROW_CUSEXCEPTION("PxCreateStatic failed");
	//}
	//m_Scene->addActor(*actor);
	//m_Actors.push_back(static_pointer_cast<PxActor>(actor));

	//mesh->UnlockIndexBuffer();
	//mesh->UnlockVertexBuffer();

	/************************************************************************/
	/* Apex 破碎示例                                                        */
	/************************************************************************/
	CachePtr cache = Game::getSingleton().OpenArchiveStream("Wall.apb")->GetWholeCache();
	PhysxPtr<physx::PxFileBuf> stream(PhysxSample::getSingleton().m_ApexSDK->createMemoryReadStream(&(*cache)[0], cache->size()));
	NxParameterized::Serializer::SerializeType iSerType = PhysxSample::getSingleton().m_ApexSDK->getSerializeType(*stream);
	PhysxPtr<NxParameterized::Serializer> ser(PhysxSample::getSingleton().m_ApexSDK->createSerializer(iSerType));
	NxParameterized::Serializer::DeserializedData data;
	NxParameterized::Serializer::ErrorType serError = ser->deserialize(*stream, data);

	NxParameterized::Interface * params = data[0];
	PhysxPtr<physx::apex::NxDestructibleAsset> asset(
		static_cast<physx::NxDestructibleAsset *>(PhysxSample::getSingleton().m_ApexSDK->createAsset(params, "Asset Name")));
	m_DestructibleAssets.push_back(asset);

	params = asset->getDefaultActorDesc();
	NxParameterized::setParamBool(*params, "destructibleParameters.flags.CRUMBLE_SMALLEST_CHUNKS", true);
	NxParameterized::setParamF32(*params, "destructibleParameters.forceToDamage", 0.1f);
	NxParameterized::setParamF32(*params, "destructibleParameters.damageThreshold", 10.0f);
	NxParameterized::setParamF32(*params, "destructibleParameters.damageCap", 10.0f);
	NxParameterized::setParamF32(*params, "destructibleParameters.damageToRadius", 0.0f);
	NxParameterized::setParamF32(*params, "destructibleParameters.fractureImpulseScale", 2.0f);
	NxParameterized::setParamBool(*params, "formExtendedStructures", true);
	{
		int depthParametersCount = 0;
		NxParameterized::getParamArraySize(*params, "depthParameters", depthParametersCount);
		NxParameterized::setParamI32(*params, "destructibleParameters.impactDamageDefaultDepth", depthParametersCount - 1);
		if(depthParametersCount > 0)
		{
			const unsigned int bufferCount = 128;
			for(physx::PxU32 index = 0; index < static_cast<unsigned int>(depthParametersCount); ++index)
			{
				char buffer[bufferCount] = {0};
				sprintf_s(buffer, bufferCount, "depthParameters[%d].OVERRIDE_IMPACT_DAMAGE", index);
				NxParameterized::setParamBool(*params, buffer, false);
			}
		}
	}
	NxParameterized::setParamU32(*params, "p3ShapeDescTemplate.simulationFilterData.word0", 2);
	NxParameterized::setParamU32(*params, "p3ShapeDescTemplate.simulationFilterData.word2", ~0);
	NxParameterized::setParamF32(*params, "p3BodyDescTemplate.density", 1.0f);
	NxParameterized::setParamBool(*params, "dynamic", false);
	physx::PxMat44 wallPose = physx::PxMat44::createIdentity();
	wallPose(1, 1) =  0;
	wallPose(2, 2) =  0;
	wallPose(1, 2) =  1;
	wallPose(2, 1) = -1;
	wallPose(1, 3) = 5.7747002f;
	NxParameterized::setParamMat44(*params, "globalPose", wallPose);
	NxParameterized::setParamVec3(*params, "scale", PxVec3(0.5f));
	PhysxPtr<physx::apex::NxDestructibleActor> apexActor(
		static_cast<physx::NxDestructibleActor *>(asset->createApexActor(*params, *m_ApexScene)));
	m_DestructibleActors.push_back(apexActor);

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

	PhysxScene::OnShutdown();
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

	D3DVIEWPORT9 vp;
	Game::getSingleton().GetD3D9Device()->GetViewport(&vp);
	static const physx::PxU32 viewIDlookAtRightHand = m_ApexScene->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_RH);
	static const physx::PxU32 projIDperspectiveCubicRightHand = m_ApexScene->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);
	m_ApexScene->setViewMatrix(PxMat44(&(m_Camera->m_View.transpose()._11)), viewIDlookAtRightHand);
	m_ApexScene->setProjMatrix(PxMat44(&(m_Camera->m_Proj.transpose()._11)), projIDperspectiveCubicRightHand);
	m_ApexScene->setProjParams(m_Camera->m_Nz, m_Camera->m_Fz, D3DXToDegree(m_Camera->m_Fov), vp.Width, vp.Height, projIDperspectiveCubicRightHand);
	m_ApexScene->setUseViewProjMatrix(viewIDlookAtRightHand, projIDperspectiveCubicRightHand);

	PhysxScene::OnTickPreRender(fElapsedTime);
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

	Vector3 LightDir(Vector3(-1,-1,-1).normalize());
	Vector3 LightTag(0,1,0);
	Matrix4 LightViewProj = Matrix4::LookAtRH(LightTag - LightDir, LightTag, Vector3(0,1,0)) * Matrix4::OrthoRH(3, 3, -50, 50);

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

		for(size_t i = 0; i < m_DestructibleActors.size(); i++)
		{
			m_DestructibleActors[i]->lockRenderResources();
			m_DestructibleActors[i]->updateRenderResources();
			m_DestructibleActors[i]->dispatchRenderResources(Game::getSingleton().m_ApexRenderer);
			m_DestructibleActors[i]->unlockRenderResources();
		}

		// ! The Right tick post render should be called after d3ddevice->present, for vertical sync reason
		PhysxScene::OnTickPostRender(fElapsedTime);

		m_ApexScene->prepareRenderResourceContexts();

		PhysxScene::DrawRenderBuffer(Game::getSingleton().GetD3D9Device(), m_Scene->getRenderBuffer());

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

	switch(uMsg)
	{
	case WM_RBUTTONUP:
		CRect ClientRect;
		GetClientRect(hWnd, &ClientRect);
		Vector2 ptScreen((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f);
		Vector3 ptProj(Lerp(-1.0f, 1.0f, ptScreen.x / ClientRect.right), Lerp(1.0f, -1.0f, ptScreen.y / ClientRect.bottom), 1.0f);
		Vector3 dir = (ptProj.transformCoord(m_Camera->m_InverseViewProj) - m_Camera->m_Position).normalize();

		PxVec3 rayOrigin(m_Camera->m_Position.x, m_Camera->m_Position.y, m_Camera->m_Position.z);
		PxVec3 rayDirection(dir.x, dir.y, dir.z);

		physx::apex::NxDestructibleActor * hitActor = NULL;
		physx::PxF32 hitTime = PX_MAX_F32;
		physx::PxVec3 hitNormal(0.0f);
		physx::PxI32 hitChunkIndex = physx::apex::NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
		physx::PxF32 time = 0;
		physx::PxVec3 normal(0.0f);
		for(size_t i = 0; i < m_DestructibleActors.size(); i++)
		{
			const physx::PxI32 chunkIndex = m_DestructibleActors[i]->rayCast(time, normal, rayOrigin, rayDirection, physx::apex::NxDestructibleActorRaycastFlags::AllChunks);
			if(chunkIndex != physx::apex::NxModuleDestructibleConst::INVALID_CHUNK_INDEX && time < hitTime)
			{
				hitActor = m_DestructibleActors[i].get();
				hitTime = time;
				hitNormal = normal;
				hitChunkIndex = chunkIndex;
			}
		}

		if(hitActor)
		{
			hitActor->applyDamage(10.0f, 10.0f, rayOrigin + (hitTime * rayDirection), rayDirection, hitChunkIndex);
		}
		break;
	}

	return 0;
}

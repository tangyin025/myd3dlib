
#include "stdafx.h"
#include "Game.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo : public Game
{
	physx_ptr<PxRigidActor> m_actor;

	physx_ptr<physx::apex::NxApexAsset> m_apex_asset;

	physx_ptr<physx::apex::NxDestructibleActor> m_dest_actor;

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		ExecuteCode("dofile \"GameStateMain.lua\"");

		//************************************************************************/
		//* Apex ∆∆ÀÈ æ¿˝                                                        */
		//************************************************************************/
		if(!(m_actor.reset(PxCreatePlane(*m_Physics, PxPlane(PxVec3(0,0,0), PxVec3(0,1,0)), *m_Material)),
			m_actor))
		{
			THROW_CUSEXCEPTION(_T("PxCreatePlane failed"));
		}
		m_Scene->addActor(*m_actor);

		CachePtr cache = OpenArchiveStream("Wall.apx")->GetWholeCache();
		physx_ptr<physx::PxFileBuf> stream(m_ApexSDK->createMemoryReadStream(&(*cache)[0], cache->size()));
		NxParameterized::Serializer::SerializeType iSerType = m_ApexSDK->getSerializeType(*stream);
		physx_ptr<NxParameterized::Serializer> ser(m_ApexSDK->createSerializer(iSerType));
		NxParameterized::Serializer::DeserializedData data;
		NxParameterized::Serializer::ErrorType serError = ser->deserialize(*stream, data);

		NxParameterized::Interface * params = data[0];
		m_apex_asset.reset(m_ApexSDK->createAsset(params, "Asset Name"));

		params = m_apex_asset->getDefaultActorDesc();
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
		m_dest_actor.reset(static_cast<physx::NxDestructibleActor *>(m_apex_asset->createApexActor(*params, *m_ApexScene)));

		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}
		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		Game::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		m_actor.reset();
		m_dest_actor.reset();
		m_apex_asset.reset();

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);

		Matrix4 World = Matrix4::Identity();
		m_SimpleSample->SetFloat("g_Time", (float)fTime);
		m_SimpleSample->SetMatrix("g_World", World);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		//m_SimpleSample->SetMatrix("g_ViewProjLS", Vector3(1,1,1));
		m_SimpleSample->SetVector("g_EyePos", m_Camera->m_Position);
		m_SimpleSample->SetVector("g_EyePosOS", m_Camera->m_Position.transformCoord(World.inverse()));
		m_SimpleSample->SetVector("g_LightDir", Vector3(-1,-1,-1));
		m_SimpleSample->SetVector("g_LightDiffuse", Vector4(1,1,1,1));
		//m_SimpleSample->SetTexture("g_ShadowTexture", m_ShadowTextureRT);

		//DrawRenderBuffer(pd3dDevice);

		m_dest_actor->lockRenderResources();
		m_dest_actor->updateRenderResources();
		m_dest_actor->dispatchRenderResources(m_ApexRenderer);
		m_dest_actor->unlockRenderResources();

		Game::OnFrameRender(pd3dDevice, fTime, fElapsedTime);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT lr;
		if(lr = Game::MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
		{
			return lr;
		}

		switch(uMsg)
		{
		case WM_RBUTTONUP:
			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			std::pair<Vector3, Vector3> ray = m_Camera->CalculateRay(
				Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());

			PxVec3 rayOrigin(ray.first.x, ray.first.y, ray.first.z);
			PxVec3 rayDirection(ray.second.x, ray.second.y, ray.second.z);

			physx::PxF32 time = 0;
			physx::PxVec3 normal(0.0f);
			const physx::PxI32 chunkIndex = m_dest_actor->rayCast(time, normal, rayOrigin, rayDirection, physx::apex::NxDestructibleActorRaycastFlags::AllChunks);
			if(chunkIndex != physx::apex::NxModuleDestructibleConst::INVALID_CHUNK_INDEX && time < PX_MAX_F32)
			{
				m_dest_actor->applyDamage(10.0f, 10.0f, rayOrigin + (time * rayDirection), rayDirection, chunkIndex);
			}
		}
		return 0;
	}
};

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	// …Ë÷√crtdbgº‡ ”ƒ⁄¥Ê–π¬©
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}

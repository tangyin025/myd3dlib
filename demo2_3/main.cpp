
#include "stdafx.h"
#include "Game.h"
#include "MeshComponent.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
	, public physx::apex::NxUserRenderer
{
public:
	EffectPtr m_SimpleSample;

	SkeletonMeshComponentPtr m_mesh;
	OgreSkeletonAnimationPtr m_skel_anim;
	BoneList m_skel_pose;
	BoneList m_skel_pose_heir1;
	BoneList m_skel_pose_heir2;

	std::vector<physx_ptr<PxActor> > m_Actors;
	physx_ptr<physx::apex::NxApexAsset> m_ApexAsset;
	physx_ptr<physx::apex::NxDestructibleActor> m_DestructibleActor;

	void DrawTextAtWorld(
		const Vector3 & pos,
		LPCWSTR lpszText,
		D3DCOLOR Color,
		Font::Align align = Font::AlignCenterMiddle)
	{
		const Vector3 ptProj = pos.transformCoord(m_Camera->m_ViewProj);
		if(ptProj.z > 0.0f && ptProj.z < 1.0f)
		{
			const Vector2 vp = DialogMgr::GetDlgViewport();
			const Vector2 ptVp(Lerp(0.0f, vp.x, (ptProj.x + 1) / 2), Lerp(0.0f, vp.y, (1 - ptProj.y) / 2));
			m_Font->DrawString(m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);
		}
	}

	void renderResource(const physx::apex::NxApexRenderContext& context)
	{
		m_RenderObjList.push_back(static_cast<ApexRenderResource *>(context.renderResource));
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		ExecuteCode("dofile \"GameStateMain.lua\"");

		m_SimpleSample = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());

		m_mesh = SkeletonMeshComponentPtr(new SkeletonMeshComponent());
		m_mesh->m_Mesh = LoadMesh("mesh/casual19_m_highpoly.mesh.xml");
		std::vector<std::string>::const_iterator mat_name_iter = m_mesh->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != m_mesh->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			EffectMacroPairList macros;
			macros.push_back(EffectMacroPair("VS_SKINED_DQ",""));
			MaterialPtr mat = LoadMaterial(str_printf("material/%s.txt", mat_name_iter->c_str()));
			m_mesh->m_Materials.push_back(MeshComponent::MaterialPair(mat, LoadEffect("shader/SimpleSample.fx", macros)));
		}
		m_mesh->m_World = Matrix4::Scaling(0.05f,0.05f,0.05f);
		m_skel_anim = LoadSkeleton("mesh/casual19_m_highpoly.skeleton.xml");

		// Apex 破碎示例
		physx_ptr<PxRigidActor> actor;
		if(!(actor.reset(PxCreatePlane(*m_Physics, PxPlane(PxVec3(0,0,0), PxVec3(0,1,0)), *m_Material)),
			actor))
		{
			THROW_CUSEXCEPTION(_T("PxCreatePlane failed"));
		}
		m_Scene->addActor(*actor);
		m_Actors.push_back(static_pointer_cast<PxActor>(actor));
		CachePtr cache = OpenStream("Wall.apx")->GetWholeCache();
		physx_ptr<physx::PxFileBuf> stream(m_ApexSDK->createMemoryReadStream(&(*cache)[0], cache->size()));
		NxParameterized::Serializer::SerializeType iSerType = m_ApexSDK->getSerializeType(*stream);
		physx_ptr<NxParameterized::Serializer> ser(m_ApexSDK->createSerializer(iSerType));
		NxParameterized::Serializer::DeserializedData data;
		NxParameterized::Serializer::ErrorType serError = ser->deserialize(*stream, data);
		NxParameterized::Interface * params = data[0];
		m_ApexAsset.reset(m_ApexSDK->createAsset(params, "Asset Name"));
		params = m_ApexAsset->getDefaultActorDesc();
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
		m_DestructibleActor.reset(static_cast<physx::NxDestructibleActor *>(m_ApexAsset->createApexActor(*params, *m_ApexScene)));

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
		// 注意顺序
		m_DestructibleActor.reset();
		m_ApexAsset.reset();
		m_Actors.clear();

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		// 设置动画
		static float anim_time = 0;
		anim_time = fmod(anim_time + fElapsedTime, m_skel_anim->GetAnimation("walk").GetTime());
		m_skel_pose.resize(m_skel_anim->m_boneBindPose.size());
		m_skel_anim->BuildAnimationPose(
			m_skel_pose,
			m_skel_anim->m_boneHierarchy,
			m_skel_anim->GetBoneIndex("Bip01"),
			"walk",
			anim_time);
		m_skel_pose[m_skel_anim->GetBoneIndex("Bip01")].m_position.z = 0; // 固定根节点的z轴移动
		m_skel_pose_heir1.clear();
		m_skel_pose_heir1.resize(m_skel_anim->m_boneBindPose.size());
		m_skel_anim->m_boneBindPose.BuildHierarchyBoneList(
			m_skel_pose_heir1,
			m_skel_anim->m_boneHierarchy,
			m_skel_anim->GetBoneIndex("Bip01"));
		m_skel_pose_heir2.clear();
		m_skel_pose_heir2.resize(m_skel_anim->m_boneBindPose.size());
		m_skel_pose.BuildHierarchyBoneList(
			m_skel_pose_heir2,
			m_skel_anim->m_boneHierarchy,
			m_skel_anim->GetBoneIndex("Bip01"));
		m_mesh->m_DualQuats.clear();
		m_mesh->m_DualQuats.resize(m_skel_anim->m_boneBindPose.size());
		m_skel_pose_heir1.BuildDualQuaternionList(m_mesh->m_DualQuats, m_skel_pose_heir2);
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);

		DrawHelper::DrawGrid(pd3dDevice);

		m_RenderObjList.clear();
		m_RenderObjList.push_back(m_mesh.get());
		m_DestructibleActor->lockRenderResources();
		m_DestructibleActor->updateRenderResources();
		m_DestructibleActor->dispatchRenderResources(*this);
		m_DestructibleActor->unlockRenderResources();

		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		RenderObjList::iterator mesh_cmp_iter = m_RenderObjList.begin();
		for(; mesh_cmp_iter != m_RenderObjList.end(); mesh_cmp_iter++)
		{
			(*mesh_cmp_iter)->Draw(MeshComponentBase::DrawStateOpaque, Matrix4::identity);
		}

		//PhysXSceneContext::DrawRenderBuffer(pd3dDevice); // ! Do not use this method while the simulation is running

		m_EmitterInst->Begin();
		EmitterMgr::Draw(m_EmitterInst.get(), m_Camera->m_ViewProj, m_Camera->m_Orientation, fTime, fElapsedTime);
		m_EmitterInst->End();

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
		DrawTextAtWorld(Vector3(12,0,0), L"x", D3DCOLOR_ARGB(255,255,255,0));
		DrawTextAtWorld(Vector3(0,0,12), L"z", D3DCOLOR_ARGB(255,255,255,0));
		DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime);
		_ASSERT(m_Font);
		m_UIRender->SetWorld(Matrix4::identity);
		m_Font->DrawString(m_UIRender.get(), m_strFPS, Rectangle::LeftTop(5,5,500,10), D3DCOLOR_ARGB(255,255,255,0));
		m_UIRender->End();
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
			const physx::PxI32 chunkIndex = m_DestructibleActor->rayCast(time, normal, rayOrigin, rayDirection, physx::apex::NxDestructibleActorRaycastFlags::AllChunks);
			if(chunkIndex != physx::apex::NxModuleDestructibleConst::INVALID_CHUNK_INDEX && time < PX_MAX_F32)
			{
				m_DestructibleActor->applyDamage(10.0f, 10.0f, rayOrigin + (time * rayDirection), rayDirection, chunkIndex);
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
	// 设置crtdbg监视内存泄漏
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}

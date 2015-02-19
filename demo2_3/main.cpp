#include "stdafx.h"
#include "Game.h"
//#include "Logic/MeshComponent.h"
#include "Logic/Logic.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
{
public:
	// ========================================================================================================
	// 骨骼动画
	// ========================================================================================================
	SkeletonMeshComponentPtr m_mesh_cmp;
	OgreSkeletonAnimationPtr m_skel_anim;
	BoneList m_skel_pose;
	BoneList m_skel_pose_heir1;
	BoneList m_skel_pose_heir2;

	MeshInstancePtr m_mesh_ins;

	//// ========================================================================================================
	//// 大场景
	//// ========================================================================================================
	//OgreMeshSetPtr m_meshSet;

	//// ========================================================================================================
	//// 布料系统
	//// ========================================================================================================
	//std::vector<PxClothParticle> m_clothPositions;
	//MeshComponentPtr m_clothMesh;
	//PxCloth * m_cloth;

	//// ========================================================================================================
	//// 逻辑系统
	//// ========================================================================================================
	//LogicPtr m_Logic;

	Demo::Demo(void)
		//: m_Logic(new Logic)
	{
	}

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

	//MeshComponentPtr CreateMeshComponent(my::OgreMeshPtr mesh)
	//{
	//	MeshComponentPtr mesh_cmp(new StaticMeshComponent());
	//	mesh_cmp->Min = mesh->m_aabb.Min;
	//	mesh_cmp->Max = mesh->m_aabb.Max;
	//	MeshLODPtr lod(new MeshLOD());
	//	lod->m_Mesh = mesh;
	//	std::vector<std::string>::const_iterator mat_name_iter = lod->m_Mesh->m_MaterialNameList.begin();
	//	for(; mat_name_iter != lod->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
	//	{
	//		lod->m_Materials.push_back(LoadMaterial(str_printf("material/%s.xml", mat_name_iter->c_str())));
	//	}
	//	mesh_cmp->m_Lod[0] = lod;
	//	return mesh_cmp;
	//}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

		ExecuteCode("dofile \"Hud.lua\"");

		// ========================================================================================================
		// 骨骼动画
		// ========================================================================================================
		//SaveSimplyMesh("aaa.mesh.xml", LoadMesh("mesh/sportive03_f.mesh.xml"), 400);
		//SaveSimplyMesh("bbb.mesh.xml", LoadMesh("mesh/sportive03_f.mesh.xml"), 40);

		m_mesh_cmp.reset(new SkeletonMeshComponent());
		MeshComponent::LODPtr lod(new MeshComponent::LOD);
		lod->m_Mesh = LoadMesh("mesh/sportive03_f.mesh.xml");
		std::vector<std::string>::const_iterator mat_name_iter = lod->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != lod->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			lod->m_Materials.push_back(LoadMaterial(str_printf("material/%s.xml", mat_name_iter->c_str())));
		}
		m_mesh_cmp->m_lods.push_back(lod);
		m_mesh_cmp->m_World = Matrix4::Scaling(0.05f,0.05f,0.05f);
		m_skel_anim = LoadSkeleton("mesh/sportive03_f.skeleton.xml");

		m_mesh_ins = LoadMesh("mesh/tube.mesh.xml");
		m_mesh_ins->CreateInstance(pd3dDevice);
		AddResource("____eraweraw", m_mesh_ins);

		//// ========================================================================================================
		//// 大场景
		//// ========================================================================================================
		//m_meshSet = LoadMeshSet("mesh/scene.mesh.xml");
		//OgreMeshSet::iterator mesh_iter = m_meshSet->begin();
		//for(; mesh_iter != m_meshSet->end(); mesh_iter++)
		//{
		//	m_OctScene->PushComponent(CreateMeshComponent(*mesh_iter), 0.1f);
		//}

		//// ========================================================================================================
		//// 布料系统
		//// ========================================================================================================
		//PxClothCollisionSphere spheres[2] =
		//{
		//	{PxVec3(-5.0f, -5.0f, 0.0f), 3.f},
		//	{PxVec3( 5.0f, -5.0f, 0.0f), 3.f}
		//};
		//// A tapered capsule
		//PxU32 capsulePairs[] = { 0, 1 };
		//PxClothCollisionData collisionData;
		//collisionData.spheres = spheres;
		//collisionData.numSpheres = 2;
		//collisionData.pairIndexBuffer = capsulePairs;
		//collisionData.numPairs = 1;
		//m_clothMesh = CreateMeshComponent(LoadMesh("mesh/plane.mesh.xml"));
		//m_clothPositions.resize(m_clothMesh->m_Mesh->GetNumVertices());
		//unsigned char * pVertices = (unsigned char *)m_clothMesh->m_Mesh->LockVertexBuffer();
		//for(int i = 0; i < m_clothPositions.size(); i++) {
		//	void * pVertex = pVertices + i * m_clothMesh->m_Mesh->GetNumBytesPerVertex();
		//	m_clothPositions[i].pos = (PxVec3 &)m_clothMesh->m_Mesh->m_VertexElems.GetPosition(pVertex);
		//	m_clothPositions[i].invWeight = 1 / 1.0f;
		//}
		//m_clothMesh->m_Mesh->UnlockVertexBuffer();
		//my::MemoryOStreamPtr ofs(new my::MemoryOStream);
		//CookClothFabric(ofs, m_clothMesh->m_Mesh);
		//ifs.reset(new MemoryIStream(&(*ofs->m_cache)[0], ofs->m_cache->size()));
		//physx_ptr<PxClothFabric> clothFabric(CreateClothFabric(ifs));
		//m_cloth = m_sdk->createCloth(PxTransform(PxVec3(0,10,0), PxQuat(0,0,0,1)), *clothFabric, &m_clothPositions[0], collisionData, PxClothFlags());
		//m_PxScene->addActor(*m_cloth);

		//// ========================================================================================================
		//// 逻辑系统
		//// ========================================================================================================
		//m_Logic->Create();

		ExecuteCode("dofile \"StateMain.lua\"");

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
		//// 注意顺序
		//m_Logic->Destroy();

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		if (m_Camera)
		{
			m_Camera->OnFrameMove(fTime, fElapsedTime);
		}

		m_ScrInfos[0] = str_printf(L"%.2f", m_fFps);

		// ========================================================================================================
		// 骨骼动画
		// ========================================================================================================
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
		m_mesh_cmp->m_DualQuats.clear();
		m_mesh_cmp->m_DualQuats.resize(m_skel_anim->m_boneBindPose.size());
		m_skel_pose_heir1.BuildDualQuaternionList(m_mesh_cmp->m_DualQuats, m_skel_pose_heir2);

		//// ========================================================================================================
		//// 布料系统
		//// ========================================================================================================
		//PxClothReadData * readData = m_cloth->lockClothReadData();
		//if(readData)
		//{
		//	unsigned char * pVertices = (unsigned char *)m_clothMesh->m_Mesh->LockVertexBuffer();
		//	for (unsigned int i = 0; i < m_cloth->getNbParticles(); i++)
		//	{
		//		void * pVertex = pVertices + i * m_clothMesh->m_Mesh->GetNumBytesPerVertex();
		//		m_clothMesh->m_Mesh->m_VertexElems.SetPosition(pVertex, (Vector3&)readData->particles[i].pos);
		//	}
		//	m_clothMesh->m_Mesh->UnlockVertexBuffer();
		//	readData->unlock();
		//}
		//PxTransform Trans = m_cloth->getGlobalPose();
		//m_clothMesh->m_World = Matrix4::Compose(Vector3(1,1,1),(Quaternion&)Trans.q, (Vector3&)Trans.p);

		//// ========================================================================================================
		//// 逻辑系统
		//// ========================================================================================================
		//m_Logic->Update(fElapsedTime);
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		PushGrid();

		// ========================================================================================================
		// 骨骼动画
		// ========================================================================================================
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		//float dist_sq = -m_mesh_cmp->m_World[3].xyz.transform(m_Camera->m_View).z;
		//DWORD lod;
		//if (dist_sq < 15)
		//{
		//	lod = 0;
		//	if (!m_mesh_cmp->m_Lod[lod])
		//	{
		//		m_mesh_cmp->m_Lod[lod].reset(new MeshLOD());
		//		LoadMeshLodAsync(m_mesh_cmp->m_Lod[lod], "mesh/sportive03_f.mesh.xml");
		//	}
		//}
		//else if(dist_sq < 30)
		//{
		//	lod = 1;
		//	if (!m_mesh_cmp->m_Lod[lod])
		//	{
		//		m_mesh_cmp->m_Lod[lod].reset(new MeshLOD());
		//		LoadMeshLodAsync(m_mesh_cmp->m_Lod[lod], "aaa.mesh.xml");
		//	}
		//}
		//else
		//{
		//	lod = 2;
		//	if (!m_mesh_cmp->m_Lod[lod])
		//	{
		//		m_mesh_cmp->m_Lod[lod].reset(new MeshLOD());
		//		LoadMeshLodAsync(m_mesh_cmp->m_Lod[lod], "bbb.mesh.xml");
		//	}
		//}
		//DrawMesh(m_mesh_cmp.get(), lod);

		Matrix4 * mat = m_mesh_ins->LockInstanceData(2);
		mat[0] = Matrix4::Translation(10,0,0);
		mat[1] = Matrix4::Translation(-10,0,0);
		m_mesh_ins->UnlockInstanceData();

		m_SimpleSampleInst->SetTexture("g_MeshTexture", m_TexChecker);
		m_SimpleSampleInst->SetMatrix("g_World", Matrix4::identity);
		m_SimpleSampleInst->SetTechnique("RenderScene");
		UINT passes = m_SimpleSampleInst->Begin(0);
		for (UINT p = 0; p < passes; p++)
		{
			m_SimpleSampleInst->BeginPass(p);
			m_mesh_ins->DrawSubsetInstance(0, 2);
			m_SimpleSampleInst->EndPass();
		}
		m_SimpleSampleInst->End();

		m_mesh_cmp->QueryMesh(this, RenderPipeline::DrawStageCBuffer);

		//// ========================================================================================================
		//// 布料系统
		//// ========================================================================================================
		//m_clothMesh->Draw();

		Game::OnFrameRender(pd3dDevice, fTime, fElapsedTime);
	}

	virtual void OnUIRender(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime)
	{
		// 绘制坐标
		DrawTextAtWorld(Vector3(12,0,0), L"x", D3DCOLOR_ARGB(255,255,255,0));
		DrawTextAtWorld(Vector3(0,0,12), L"z", D3DCOLOR_ARGB(255,255,255,0));

		Game::OnUIRender(ui_render, fTime, fElapsedTime);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT lr;
		if (m_Camera && (lr = m_Camera->MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)))
		{
			return lr;
		}

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
			break;
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

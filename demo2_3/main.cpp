#include "stdafx.h"
#include "Game.h"
#include "MeshComponent.h"
#include "Logic/Logic.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
{
public:
	//// ========================================================================================================
	//// 骨骼动画
	//// ========================================================================================================
	//SkeletonMeshComponentPtr m_mesh;
	//OgreSkeletonAnimationPtr m_skel_anim;
	//BoneList m_skel_pose;
	//BoneList m_skel_pose_heir1;
	//BoneList m_skel_pose_heir2;

	//// ========================================================================================================
	//// 大场景
	//// ========================================================================================================
	//OgreMeshSetPtr m_meshSet;
	//MaterialPtr m_lambert1;
	//OctreeRootPtr m_scene;

	//// ========================================================================================================
	//// 布料系统
	//// ========================================================================================================
	//std::vector<PxClothParticle> m_clothPositions;
	//MeshComponentPtr m_clothMesh;
	//PxCloth * m_cloth;

	// ========================================================================================================
	// 逻辑系统
	// ========================================================================================================
	LogicPtr m_Logic;

	Demo::Demo(void)
		: m_Logic(new Logic)
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

	MeshComponentPtr CreateMeshComponent(my::OgreMeshPtr mesh)
	{
		MeshComponentPtr comp(new MeshComponent(mesh->m_aabb));
		comp->m_Mesh = mesh;
		std::vector<std::string>::const_iterator mat_name_iter = comp->m_Mesh->m_MaterialNameList.begin();
		for(; mat_name_iter != comp->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		{
			comp->m_Materials.push_back(MeshComponent::MaterialPair(
				LoadMaterial(str_printf("material/%s.txt", mat_name_iter->c_str())), LoadEffect("shader/SimpleSample.fx", EffectMacroPairList())));
		}
		comp->m_World = Matrix4::identity;
		return comp;
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

		m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
		m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
		m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
		m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

		//// ========================================================================================================
		//// 骨骼动画
		//// ========================================================================================================
		//m_mesh = SkeletonMeshComponentPtr(new SkeletonMeshComponent(my::AABB(my::Vector3(-1,-1,-1), my::Vector3(1,1,1))));
		//m_mesh->m_Mesh = LoadMesh("mesh/casual19_m_highpoly.mesh.xml");
		//std::vector<std::string>::const_iterator mat_name_iter = m_mesh->m_Mesh->m_MaterialNameList.begin();
		//for(; mat_name_iter != m_mesh->m_Mesh->m_MaterialNameList.end(); mat_name_iter++)
		//{
		//	EffectMacroPairList macros;
		//	macros.push_back(EffectMacroPair("VS_SKINED_DQ",""));
		//	MaterialPtr mat = LoadMaterial(str_printf("material/%s.txt", mat_name_iter->c_str()));
		//	m_mesh->m_Materials.push_back(MeshComponent::MaterialPair(mat, LoadEffect("shader/SimpleSample.fx", macros)));
		//}
		//m_mesh->m_World = Matrix4::Scaling(0.05f,0.05f,0.05f);
		//m_skel_anim = LoadSkeleton("mesh/casual19_m_highpoly.skeleton.xml");

		//// ========================================================================================================
		//// 大场景
		//// ========================================================================================================
		//m_meshSet = LoadMeshSet("mesh/scene.mesh.xml");
		//m_lambert1 = LoadMaterial("material/lambert1.txt");
		//m_scene.reset(new OctreeRoot(my::AABB(Vector3(-256,-256,-256),Vector3(256,256,256))));
		//OgreMeshSet::iterator mesh_iter = m_meshSet->begin();
		//for(; mesh_iter != m_meshSet->end(); mesh_iter++)
		//{
		//	m_scene->PushComponent(CreateMeshComponent(*mesh_iter), 0.1f);
		//}

		//// ========================================================================================================
		//// 物理场景
		//// ========================================================================================================
		//my::IStreamPtr ifs = my::FileIStream::Open(_T("D:\\Works\\VC++\\D3DSolution\\demo2_3\\Media\\mesh\\scene_tm.phy"));
		//PxRigidActor * actor = m_sdk->createRigidStatic(PxTransform::createIdentity());
		//PxShape * shape = actor->createShape(PxTriangleMeshGeometry(physx_ptr<PxTriangleMesh>(CreateTriangleMesh(ifs)).get()), *m_PxMaterial);
		//shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
		//m_Scene->addActor(*actor);

		//for(int x = -10; x <= 10; x+= 2)
		//	for(int z= -10; z <= 10; z+= 2)
		//		m_Scene->addActor(*PxCreateDynamic(
		//			*m_sdk, PxTransform(PxVec3(x,10,z),PxQuat(0,0,0,1)), PxSphereGeometry(0.3), *m_PxMaterial, 1));

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
		//m_Scene->addActor(*m_cloth);

		// ========================================================================================================
		// 逻辑系统
		// ========================================================================================================
		m_Logic->Create();

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
		m_Logic->Destroy();

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		//// ========================================================================================================
		//// 骨骼动画
		//// ========================================================================================================
		//static float anim_time = 0;
		//anim_time = fmod(anim_time + fElapsedTime, m_skel_anim->GetAnimation("walk").GetTime());
		//m_skel_pose.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_anim->BuildAnimationPose(
		//	m_skel_pose,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("Bip01"),
		//	"walk",
		//	anim_time);
		//m_skel_pose[m_skel_anim->GetBoneIndex("Bip01")].m_position.z = 0; // 固定根节点的z轴移动
		//m_skel_pose_heir1.clear();
		//m_skel_pose_heir1.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_anim->m_boneBindPose.BuildHierarchyBoneList(
		//	m_skel_pose_heir1,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("Bip01"));
		//m_skel_pose_heir2.clear();
		//m_skel_pose_heir2.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_pose.BuildHierarchyBoneList(
		//	m_skel_pose_heir2,
		//	m_skel_anim->m_boneHierarchy,
		//	m_skel_anim->GetBoneIndex("Bip01"));
		//m_mesh->m_DualQuats.clear();
		//m_mesh->m_DualQuats.resize(m_skel_anim->m_boneBindPose.size());
		//m_skel_pose_heir1.BuildDualQuaternionList(m_mesh->m_DualQuats, m_skel_pose_heir2);

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

		// ========================================================================================================
		// 逻辑系统
		// ========================================================================================================
		m_Logic->Update(fElapsedTime);
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

		//// ========================================================================================================
		//// 骨骼动画
		//// ========================================================================================================
		//m_RenderObjList.clear();
		//m_RenderObjList.push_back(m_mesh.get());
		//RenderObjList::iterator mesh_cmp_iter = m_RenderObjList.begin();
		//for(; mesh_cmp_iter != m_RenderObjList.end(); mesh_cmp_iter++)
		//{
		//	static_cast<MeshComponent *>(*mesh_cmp_iter)->Draw();
		//}

		//// ========================================================================================================
		//// 大场景
		//// ========================================================================================================
		//struct QueryCallbackFunc
		//{
		//	void operator() (Component * comp)
		//	{
		//		static_cast<MeshComponent *>(comp)->Draw();
		//	}
		//};
		//Frustum frustum(Frustum::ExtractMatrix(m_Camera->m_ViewProj));
		//m_scene->QueryComponent(frustum, QueryCallbackFunc());

		//// ========================================================================================================
		//// 布料系统
		//// ========================================================================================================
		//m_clothMesh->Draw();

		m_ScrInfos[0] = str_printf(L"%.2f", m_fFps);
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

#include "stdafx.h"
#include "Game.h"
#include "MeshComponent.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
	, public DrawHelper
{
public:
	EffectPtr m_SimpleSample;

	physx_ptr<PxMaterial> m_material;

	//// ========================================================================================================
	//// 骨骼动画
	//// ========================================================================================================
	//SkeletonMeshComponentPtr m_mesh;
	//OgreSkeletonAnimationPtr m_skel_anim;
	//BoneList m_skel_pose;
	//BoneList m_skel_pose_heir1;
	//BoneList m_skel_pose_heir2;

	//// ========================================================================================================
	//// 场景
	//// ========================================================================================================
	//OgreMeshSetPtr m_meshSet;
	//MaterialPtr m_lambert1;
	//OctreeRootPtr m_scene;

	FirstPersonCamera m_TestCam;

	physx_ptr<PxRigidActor> m_actor;

	Demo::Demo(void)
		: m_TestCam(D3DXToRadian(75), 1.333333f, 1, 5)
	{
		m_TestCam.m_Rotation.y = D3DXToRadian(180);
		m_TestCam.m_Position.y = 5;
		m_TestCam.OnFrameMove(0,0);
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

	//void renderResource(const physx::apex::NxApexRenderContext& context)
	//{
	//	static_cast<ApexRenderResource *>(context.renderResource)->Draw();
	//}

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

		m_SimpleSample = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());

		m_material.reset(m_sdk->createMaterial(0.5f, 0.5f, 0.1f));

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
		//// 场景模型
		//// ========================================================================================================
		//m_meshSet = LoadMeshSet("mesh/scene.mesh.xml");
		//m_lambert1 = LoadMaterial("material/lambert1.txt");
		//m_scene.reset(new OctreeRoot(my::AABB(Vector3(-256,-256,-256),Vector3(256,256,256))));
		//OgreMeshSet::iterator mesh_iter = m_meshSet->begin();
		//for(; mesh_iter != m_meshSet->end(); mesh_iter++)
		//{
		//	m_scene->PushComponent(CreateMeshComponent(*mesh_iter), 0.1f);
		//}

		// ========================================================================================================
		// 物理场景
		// ========================================================================================================
		//my::OgreMeshPtr mesh = LoadMesh("mesh/tube.mesh.xml");
		//my::OStreamPtr ofs = my::FileOStream::Open(_T("aaa"));
		//CookTriangleMesh(ofs, mesh);
		//ofs.reset();

		//my::IStreamPtr ifs = my::FileIStream::Open(_T("aaa"));
		//m_actor.reset(PxCreateStatic(
		//	*m_sdk, PxTransform(PxVec3(0,0,0), PxQuat(0,0,0,1)), PxTriangleMeshGeometry(CreateTriangleMesh(ifs)), *m_material));
		//ifs.reset();
		my::IStreamPtr ifs = my::FileIStream::Open(_T("D:\\Works\\VC++\\D3DSolution\\demo2_3\\Media\\mesh\\scene_tm.phy"));
		PxRigidActor * actor = m_sdk->createRigidStatic(PxTransform(PxVec3(0,0,0), PxQuat(0,0,0,1)));
		PxShape * shape = actor->createShape(PxTriangleMeshGeometry(CreateTriangleMesh(ifs)), *m_material);
		shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
		m_Scene->addActor(*actor);

		for(int x = -10; x <= 10; x+= 2)
		{
			for(int z= -10; z <= 10; z+= 2)
			{
				m_Scene->addActor(*PxCreateDynamic(
					*m_sdk, PxTransform(PxVec3(x,10,z),PxQuat(0,0,0,1)), PxSphereGeometry(0.3), *m_material, 1));
			}
		}

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
		m_material.reset();
		m_actor.reset();

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		//// ========================================================================================================
		//// 设置动画
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
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);
		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);

		BeginLine();
		//PushWireAABB(my::AABB(Vector3(-1,-1,-1), Vector3(1,1,1)), D3DCOLOR_ARGB(255,255,255,255), m_TestCam.m_InverseViewProj);
		//Frustum frustum(Frustum::ExtractMatrix(m_TestCam.m_ViewProj));
		//for(int i = -5; i <= 5; i++)
		//{
		//	for(int j = -5; j <= 5; j++)
		//	{
		//		for(int k = -5; k <= 5; k++)
		//		{
		//			my::AABB aabb(m_TestCam.m_Position+Vector3(i,j,k)-0.3f,m_TestCam.m_Position+Vector3(i,j,k)+0.3f);
		//			D3DCOLOR Color;
		//			switch(IntersectionTests::IntersectAABBAndFrustum(aabb, frustum))
		//			{
		//			case IntersectionTests::IntersectionTypeInside:
		//				Color = D3DCOLOR_ARGB(255,0,255,0);
		//				break;
		//			case IntersectionTests::IntersectionTypeIntersect:
		//				Color = D3DCOLOR_ARGB(255,255,0,0);
		//				break;
		//			default:
		//				Color = D3DCOLOR_ARGB(255,255,255,255);
		//				break;
		//			}
		//			PushWireAABB(aabb, Color);
		//		}
		//	}
		//}
		PhysXSceneContext::PushRenderBuffer(this); // ! PxScene::getRenderBuffer() not allowed while simulation is running.
		PushGrid();
		EndLine(pd3dDevice, Matrix4::identity);

		//// ========================================================================================================
		//// 绘制骨骼动画
		//// ========================================================================================================
		//m_RenderObjList.clear();
		//m_RenderObjList.push_back(m_mesh.get());
		//RenderObjList::iterator mesh_cmp_iter = m_RenderObjList.begin();
		//for(; mesh_cmp_iter != m_RenderObjList.end(); mesh_cmp_iter++)
		//{
		//	static_cast<MeshComponent *>(*mesh_cmp_iter)->Draw();
		//}

		//// ========================================================================================================
		//// 绘制场景
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

		// ========================================================================================================
		// 绘制粒子
		// ========================================================================================================
		m_EmitterInst->Begin();
		EmitterMgr::Draw(m_EmitterInst.get(), m_Camera->m_ViewProj, m_Camera->m_Orientation, fTime, fElapsedTime);
		m_EmitterInst->End();

		// ========================================================================================================
		// 绘制网格坐标
		// ========================================================================================================
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

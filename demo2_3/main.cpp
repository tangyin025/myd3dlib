
#include "stdafx.h"
#include "Game.h"
#include "MeshComponent.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
{
public:
	EffectPtr m_SimpleSample;

	//SkeletonMeshComponentPtr m_mesh;
	//OgreSkeletonAnimationPtr m_skel_anim;
	//BoneList m_skel_pose;
	//BoneList m_skel_pose_heir1;
	//BoneList m_skel_pose_heir2;

	FirstPersonCamera m_TestCam;
	OgreMeshSetPtr m_meshSet;
	MaterialPtr m_lambert1;
	OctreeRootPtr m_root;

	Demo::Demo(void)
		: m_TestCam(D3DXToRadian(75), 1.333333f, 1, 5)
	{
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

		//// ========================================================================================================
		//// ��������
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

		m_meshSet = LoadMeshSet("mesh/scene.mesh.xml");
		m_lambert1 = LoadMaterial("material/lambert1.txt");
		m_root.reset(new OctreeRoot(my::AABB(Vector3(-256,-256,-256),Vector3(256,256,256))));
		OgreMeshSet::iterator mesh_iter = m_meshSet->begin();
		for(; mesh_iter != m_meshSet->end(); mesh_iter++)
		{
			m_root->PushComponent(CreateMeshComponent(*mesh_iter), 0.1f);
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
		// ע��˳��

		Game::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		//// ========================================================================================================
		//// ���ö���
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
		//m_skel_pose[m_skel_anim->GetBoneIndex("Bip01")].m_position.z = 0; // �̶����ڵ��z���ƶ�
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

		DrawHelper::DrawGrid(pd3dDevice);
		//DrawHelper::DrawAABB(pd3dDevice, my::AABB(Vector3(-1,-1,-1), Vector3(1,1,1)), D3DCOLOR_ARGB(255,255,255,255), m_TestCam.m_InverseViewProj);
		//struct Vertex
		//{
		//	float x, y, z;
		//	D3DCOLOR color;
		//	Vertex(float _x, float _y, float _z, D3DCOLOR _color)
		//		: x(_x), y(_y), z(_z), color(_color)
		//	{
		//	}
		//};
		//std::vector<Vertex> v;
		//Frustum frustum(Frustum::ExtractMatrix(m_TestCam.m_ViewProj));
		//for(int i = -5; i <= 5; i++)
		//{
		//	for(int j = -5; j <= 5; j++)
		//	{
		//		for(int k = -5; k <= 5; k++)
		//		{
		//			my::AABB aabb(Vector3(i,j,k)-0.3f,Vector3(i,j,k)+0.3f);
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
		//			v.push_back(Vertex(aabb.Min.x, aabb.Min.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Min.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Min.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Max.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Max.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Max.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Max.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Min.y, aabb.Min.z, Color));

		//			v.push_back(Vertex(aabb.Max.x, aabb.Min.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Min.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Min.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Max.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Max.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Max.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Max.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Min.y, aabb.Min.z, Color));

		//			v.push_back(Vertex(aabb.Min.x, aabb.Min.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Min.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Min.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Min.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Max.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Max.y, aabb.Max.z, Color));
		//			v.push_back(Vertex(aabb.Min.x, aabb.Max.y, aabb.Min.z, Color));
		//			v.push_back(Vertex(aabb.Max.x, aabb.Max.y, aabb.Min.z, Color));
		//		}
		//	}
		//}
		//pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		//pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		//pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity);
		//pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, v.size() / 2, &v[0], sizeof(v[0]));

		//// ========================================================================================================
		//// �����������
		//// ========================================================================================================
		//m_RenderObjList.clear();
		//m_RenderObjList.push_back(m_mesh.get());
		//RenderObjList::iterator mesh_cmp_iter = m_RenderObjList.begin();
		//for(; mesh_cmp_iter != m_RenderObjList.end(); mesh_cmp_iter++)
		//{
		//	(*mesh_cmp_iter)->Draw();
		//}

		////PhysXSceneContext::DrawRenderBuffer(pd3dDevice); // ! Do not use this method while the simulation is running

		struct QueryCallbackFunc
		{
			void operator() (Component * comp)
			{
				static_cast<MeshComponent *>(comp)->Draw();
			}
		};
		Frustum frustum(Frustum::ExtractMatrix(m_Camera->m_ViewProj));
		m_root->QueryComponent(frustum, QueryCallbackFunc());

		// ========================================================================================================
		// ��������
		// ========================================================================================================
		m_EmitterInst->Begin();
		EmitterMgr::Draw(m_EmitterInst.get(), m_Camera->m_ViewProj, m_Camera->m_Orientation, fTime, fElapsedTime);
		m_EmitterInst->End();

		// ========================================================================================================
		// ������������
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
	// ����crtdbg�����ڴ�й©
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}

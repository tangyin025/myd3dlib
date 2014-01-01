
#include "stdafx.h"
#include "Game.h"
#include "MeshComponent.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo : public Game
{
public:
	EffectPtr m_SimpleSample;

	SkeletonMeshComponentPtr m_mesh;
	OgreSkeletonAnimationPtr m_skel_anim;
	BoneList m_skel_pose;
	BoneList m_skel_pose_heir1;
	BoneList m_skel_pose_heir2;

	typedef std::vector<MeshComponentBase *> MeshComponentBaseList;

	MeshComponentBaseList m_Meshes;

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
		m_mesh->m_World = Matrix4::identity;
		m_Meshes.push_back(m_mesh.get());

		m_skel_anim = LoadSkeleton("mesh/casual19_m_highpoly.skeleton.xml");

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
		m_Meshes.clear();

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

		m_SimpleSample->SetMatrix("g_ViewProj", m_Camera->m_ViewProj);
		MeshComponentBaseList::iterator mesh_cmp_iter = m_Meshes.begin();
		for(; mesh_cmp_iter != m_Meshes.end(); mesh_cmp_iter++)
		{
			(*mesh_cmp_iter)->Draw(MeshComponentBase::DrawStateOpaque, Matrix4::Scaling(0.05f,0.05f,0.05f));
		}

		m_EmitterInst->Begin();
		EmitterMgr::Draw(m_EmitterInst.get(), m_Camera.get(), fTime, fElapsedTime);
		m_EmitterInst->End();

		m_UIRender->Begin();
		m_UIRender->SetWorld(Matrix4::identity);
		m_UIRender->SetViewProj(DialogMgr::m_Camera.m_ViewProj);
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

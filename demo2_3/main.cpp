#include "stdafx.h"
#include "Game.h"
#include "Component/Terrain.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>
#include <PrintCallStack.h>

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
{
public:
	Demo::Demo(void)
	{
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		// ========================================================================================================
		// 示例代码
		// ========================================================================================================

		//FModContext::SetMediaPath("sound\\");
		//FModContext::LoadEventFile("sound\\aaa.fev");
		//ExecuteCode("dofile \"Main.lua\"");
		//RemoveDlg(m_Console);
		//InsertDlg(m_Console);

		//MeshComponentPtr mesh_cmp(new MeshComponent(my::AABB(-100,100), my::Matrix4::Scaling(Vector3(0.05f)), false));
		//mesh_cmp->m_MeshRes.m_Path = "mesh/casual19_m_highpoly.mesh.xml";
		//MaterialPtr lambert1(new Material());
		//lambert1->m_Shader = "lambert1.fx";
		//lambert1->m_PassMask = Material::PassMaskOpaque;
		//lambert1->m_MeshColor = Vector4(1,1,1,1);
		//lambert1->m_MeshTexture.m_Path = "texture/casual19_m_35.jpg";
		//lambert1->m_NormalTexture.m_Path = "texture/casual19_m_35_normal.png";
		//lambert1->m_SpecularTexture.m_Path = "texture/casual19_m_35_spec.png";
		//mesh_cmp->m_MaterialList.push_back(lambert1);
		//SimpleAnimatorPtr anim(new SimpleAnimator());
		//anim->m_SkeletonRes.m_Path = "mesh/casual19_m_highpoly.skeleton.xml";
		//mesh_cmp->m_Animator = anim;
		//mesh_cmp->RequestResource();
		//m_Root.AddActor(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
		//m_cmps.push_back(mesh_cmp);

		//mesh_cmp.reset(new MeshComponent(my::AABB(-10,10), my::Matrix4::Identity(), false));
		//mesh_cmp->m_MeshRes.m_Path = "mesh/plane.mesh.xml";
		//lambert1.reset(new Material());
		//lambert1->m_Shader = "lambert1.fx";
		//lambert1->m_PassMask = Material::PassMaskOpaque;
		//lambert1->m_MeshColor = Vector4(1,1,1,1);
		//lambert1->m_MeshTexture.m_Path = "texture/Checker.bmp";
		//lambert1->m_NormalTexture.m_Path = "texture/Normal.dds";
		//lambert1->m_SpecularTexture.m_Path = "texture/White.dds";
		//mesh_cmp->m_MaterialList.push_back(lambert1);
		//mesh_cmp->RequestResource();
		//m_Root.AddActor(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
		//m_cmps.push_back(mesh_cmp);

		//EmitterComponentPtr emit_cmp(new EmitterComponent(my::AABB(-10,10), my::Matrix4::Identity()));
		//SphericalEmitterPtr emit(new SphericalEmitter());
		//emit->m_ParticleLifeTime=10.0f;
		//emit->m_SpawnInterval=1/100.0f;
		//emit->m_SpawnSpeed=5;
		//emit->m_SpawnInclination.AddNode(0,D3DXToRadian(45),0,0);
		//float Azimuth=D3DXToRadian(360)*8;
		//emit->m_SpawnAzimuth.AddNode(0,0,Azimuth/10,Azimuth/10);
		//emit->m_SpawnAzimuth.AddNode(10,Azimuth,Azimuth/10,Azimuth/10);
		//emit->m_SpawnColorA.AddNode(0,255,0,0);
		//emit->m_SpawnColorA.AddNode(10,0,0,0);
		//emit->m_SpawnColorR.AddNode(0,255,0,0);
		//emit->m_SpawnColorR.AddNode(10,0,0,0);
		//emit->m_SpawnColorG.AddNode(0,255,0,0);
		//emit->m_SpawnColorG.AddNode(10,0,0,0);
		//emit->m_SpawnColorB.AddNode(0,255,0,0);
		//emit->m_SpawnColorB.AddNode(10,0,0,0);
		//emit->m_SpawnSizeX.AddNode(0,1,0,0);
		//emit->m_SpawnSizeX.AddNode(10,10,0,0);
		//emit->m_SpawnSizeY.AddNode(0,1,0,0);
		//emit->m_SpawnSizeY.AddNode(10,10,0,0);
		//emit_cmp->m_Emitter = emit;
		//MaterialPtr particle1(new Material());
		//particle1->m_MeshTexture.m_Path = "texture/flare.dds";
		//particle1->m_PassMask = Material::PassMaskTransparent;
		//particle1->m_Shader = "particle1.fx";
		//emit_cmp->m_Material = particle1;
		//emit_cmp->RequestResource();
		//m_Root.AddActor(emit_cmp.get(), emit_cmp->m_aabb.transform(emit_cmp->m_World), 0.1f);
		//m_cmps.push_back(emit_cmp);

		//RigidComponentPtr rigid_cmp(new RigidComponent(my::AABB(-5,5),my::Matrix4::Identity()));
		//rigid_cmp->m_RigidActor->createShape(PxBoxGeometry(1,1,1), *m_PxMaterial, PxTransform::createIdentity());
		//rigid_cmp->RequestResource();
		//m_cmps.push_back(rigid_cmp);

		//// 保存场景
		//std::ofstream ofs("aaa.xml");
		//boost::archive::polymorphic_oarchive oa(ofs);
		//oa << boost::serialization::make_nvp("level", m_cmps);

		// 读取场景
		//IStreamBuff buff(OpenIStream("level.xml"));
		//std::istream istr(&buff);
		//std::ifstream istr("aaa.xml");
		//boost::archive::polymorphic_iarchive ia(istr);
		//ia >> boost::serialization::make_nvp("level", m_Logic->m_cmps);
		//for (unsigned int i = 0; i < m_Logic->m_cmps.size(); i++)
		//{
		//	m_Root.AddActor(m_Logic->m_cmps[i].get(), m_Logic->m_cmps[i]->m_aabb.transform(Component::GetCmpWorld(m_Logic->m_cmps[i].get())), 0.1f);
		//}

		//MeshComponentPtr mesh_cmp(new MeshComponent(my::AABB(-100,100), my::Matrix4::Scaling(Vector3(0.01f)), false));
		//mesh_cmp->m_lods.resize(1);
		//mesh_cmp->m_lods[0].m_MeshRes.m_Path = "mesh/casual19_m_highpoly.mesh.xml";
		//MaterialPtr lambert1(new Material());
		//lambert1->m_Shader = "lambert1.fx";
		//lambert1->m_PassMask = Material::PassMaskOpaque;
		//lambert1->m_MeshColor = Vector4(1,1,1,1);
		//lambert1->m_MeshTexture.m_Path = "texture/casual19_m_35.jpg";
		//lambert1->m_NormalTexture.m_Path = "texture/casual19_m_35_normal.png";
		//lambert1->m_SpecularTexture.m_Path = "texture/casual19_m_35_spec.png";
		//mesh_cmp->m_MaterialList.push_back(lambert1);
		//AnimatorPtr anim(new Animator());
		//anim->m_SkeletonRes.m_Path = "mesh/casual19_m_highpoly.skeleton.xml";
		//AnimationNodeSequencePtr node(new AnimationNodeSequence());
		//node->m_Name = "walk";
		//node->m_Root = "Bip01";
		//anim->m_Node = node;
		//node->m_Owner = anim.get();
		//mesh_cmp->m_Animator = anim;
		//m_Logic->m_cmps.push_back(mesh_cmp);
		//m_Root.AddActor(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);

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
		Game::OnDestroyDevice();
	}

	virtual void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		IRenderContext * pRC,
		double fTime,
		float fElapsedTime)
	{
		PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), Matrix4::RotationX(D3DXToRadian(-90)));

		swprintf_s(&m_ScrInfo[0][0], m_ScrInfo[0].size(), L"Fps: %.2f", m_fFps);
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			swprintf_s(&m_ScrInfo[1+PassID][0], m_ScrInfo[1+PassID].size(), L"%S: %d", RenderPipeline::PassTypeToStr(PassID), m_PassDrawCall[PassID]);
		}

		Game::OnRender(pd3dDevice, pBackBufferSurfaceDesc, pRC, fTime, fElapsedTime);
	}

	virtual void OnUIRender(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime)
	{
		// 绘制坐标
		DrawStringAtWorld(Vector3(12,0,0), L"x", D3DCOLOR_ARGB(255,255,255,0));
		DrawStringAtWorld(Vector3(0,0,12), L"z", D3DCOLOR_ARGB(255,255,255,0));

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
			Ray ray = boost::static_pointer_cast<my::FirstPersonCamera>(m_Camera)->CalculateRay(
				Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());
			break;
		}
		return 0;
	}
};

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

LONG WINAPI OnException(_EXCEPTION_POINTERS* ExceptionInfo)
{
	WriteMiniDump(ExceptionInfo, _T("aaa.dmp"));
	return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	// 设置crtdbg监视内存泄漏
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SetUnhandledExceptionFilter(OnException);

	return Demo().Run();
}

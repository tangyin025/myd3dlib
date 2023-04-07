#include "stdafx.h"
#include "Client.h"
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
	: public Client
{
public:
	Demo::Demo(void)
	{
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Client::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
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
		//MaterialPtr mtl(new Material());
		//mtl->m_Shader = "mtl_BlinnPhong.fx";
		//mtl->m_PassMask = Material::PassMaskOpaque;
		//mtl->m_MeshColor = Vector4(1,1,1,1);
		//mtl->m_MeshTexture.m_Path = "texture/casual19_m_35.jpg";
		//mtl->m_NormalTexture.m_Path = "texture/casual19_m_35_normal.png";
		//mtl->m_SpecularTexture.m_Path = "texture/casual19_m_35_spec.png";
		//mesh_cmp->m_MaterialList.push_back(mtl);
		//SimpleAnimatorPtr anim(new SimpleAnimator());
		//anim->m_SkeletonRes.m_Path = "mesh/casual19_m_highpoly.skeleton.xml";
		//mesh_cmp->m_Animation = anim;
		//mesh_cmp->RequestResource();
		//m_Root.AddEntity(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
		//m_cmps.push_back(mesh_cmp);

		//mesh_cmp.reset(new MeshComponent(my::AABB(-10,10), my::Matrix4::Identity(), false));
		//mesh_cmp->m_MeshRes.m_Path = "mesh/plane.mesh.xml";
		//mtl.reset(new Material());
		//mtl->m_Shader = "mtl_BlinnPhong.fx";
		//mtl->m_PassMask = Material::PassMaskOpaque;
		//mtl->m_MeshColor = Vector4(1,1,1,1);
		//mtl->m_MeshTexture.m_Path = "texture/Checker.bmp";
		//mtl->m_NormalTexture.m_Path = "texture/Normal.dds";
		//mtl->m_SpecularTexture.m_Path = "texture/White.dds";
		//mesh_cmp->m_MaterialList.push_back(mtl);
		//mesh_cmp->RequestResource();
		//m_Root.AddEntity(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
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
		//MaterialPtr mtl(new Material());
		//mtl->m_MeshTexture.m_Path = "texture/flare.dds";
		//mtl->m_PassMask = Material::PassMaskTransparent;
		//mtl->m_Shader = "mtl_particle1.fx";
		//emit_cmp->m_Material = mtl;
		//emit_cmp->RequestResource();
		//m_Root.AddEntity(emit_cmp.get(), emit_cmp->m_aabb.transform(emit_cmp->m_World), 0.1f);
		//m_cmps.push_back(emit_cmp);

		//RigidComponentPtr rigid_cmp(new RigidComponent(my::AABB(-5,5),my::Matrix4::Identity()));
		//rigid_cmp->m_RigidActor->createShape(PxBoxGeometry(1,1,1), *m_PxMaterial, PxTransform(physx::PxIdentity));
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
		//	m_Root.AddEntity(m_Logic->m_cmps[i].get(), m_Logic->m_cmps[i]->m_aabb.transform(Component::GetCmpWorld(m_Logic->m_cmps[i].get())), 0.1f);
		//}

		//MeshComponentPtr mesh_cmp(new MeshComponent(my::AABB(-100,100), my::Matrix4::Scaling(Vector3(0.01f)), false));
		//mesh_cmp->m_lods.resize(1);
		//mesh_cmp->m_lods[0].m_MeshRes.m_Path = "mesh/casual19_m_highpoly.mesh.xml";
		//MaterialPtr mtl(new Material());
		//mtl->m_Shader = "mtl_BlinnPhong.fx";
		//mtl->m_PassMask = Material::PassMaskOpaque;
		//mtl->m_MeshColor = Vector4(1,1,1,1);
		//mtl->m_MeshTexture.m_Path = "texture/casual19_m_35.jpg";
		//mtl->m_NormalTexture.m_Path = "texture/casual19_m_35_normal.png";
		//mtl->m_SpecularTexture.m_Path = "texture/casual19_m_35_spec.png";
		//mesh_cmp->m_MaterialList.push_back(mtl);
		//AnimatorPtr anim(new Animator());
		//anim->m_SkeletonRes.m_Path = "mesh/casual19_m_highpoly.skeleton.xml";
		//AnimationNodeSequencePtr node(new AnimationNodeSequence());
		//node->m_Name = "walk";
		//node->m_Root = "Bip01";
		//anim->m_Node = node;
		//node->m_Owner = anim.get();
		//mesh_cmp->m_Animation = anim;
		//m_Logic->m_cmps.push_back(mesh_cmp);
		//m_Root.AddEntity(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);

		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Client::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}
		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		Client::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		Client::OnDestroyDevice();
	}

	//virtual void OnRender(
	//	IDirect3DDevice9 * pd3dDevice,
	//	const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
	//	IRenderContext * pRC,
	//	double fTime,
	//	float fElapsedTime)
	//{
	//	//// 绘制网格
	//	//PushGrid(12, 5, 5, D3DCOLOR_ARGB(255, 127, 127, 127), D3DCOLOR_ARGB(255, 0, 0, 0), Matrix4::RotationX(D3DXToRadian(-90)));

	//	Client::OnRender(pd3dDevice, pBackBufferSurfaceDesc, pRC, fTime, fElapsedTime);
	//}

	virtual void OnUIRender(
		my::UIRender * ui_render,
		double fTime,
		float fElapsedTime)
	{
		//// 绘制坐标
		//Vector3 pt = m_Camera->WorldToScreen(Vector3(12, 0, 0), GetDlgViewport());
		//if (pt.z > 0.0f && pt.z < 1.0f)
		//{
		//	m_Font->PushString(ui_render, L"x", my::Rectangle(pt.xy, pt.xy), D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignCenterMiddle);
		//}

		//pt = m_Camera->WorldToScreen(Vector3(0, 0, 12), GetDlgViewport());
		//if (pt.z > 0.0f && pt.z < 1.0f)
		//{
		//	m_Font->PushString(ui_render, L"z", my::Rectangle(pt.xy, pt.xy), D3DCOLOR_ARGB(255, 255, 255, 0), my::Font::AlignCenterMiddle);
		//}

		Client::OnUIRender(ui_render, fTime, fElapsedTime);

		// 绘制帧率
		swprintf_s(&m_ScrInfo[0][0], m_ScrInfo[0].size(), L"Fps: %.2f", m_fFps);
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			swprintf_s(&m_ScrInfo[1 + PassID][0], m_ScrInfo[1 + PassID].size(), L"%S: %d, %d", RenderPipeline::PassTypeToStr(PassID), m_PassDrawCall[PassID], m_PassBatchDrawCall[PassID]);
		}

		swprintf_s(&m_ScrInfo[1 + RenderPipeline::PassTypeNum][0], m_ScrInfo[1 + RenderPipeline::PassTypeNum].size(), L"PassTypeUILayer: %d", ui_render->m_LayerDrawCall);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT lr;
		if(lr = Client::MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing) || *pbNoFurtherProcessing)
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
	SYSTEMTIME time;
	GetSystemTime(&time);
	TCHAR buff[MAX_PATH];
	_stprintf_s(buff, _countof(buff), _T("aaa_%04u_%02u_%02u_%02u_%02u_%02u_%03u.dmp"), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	WriteMiniDump(ExceptionInfo, buff);
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

	// MSDN：Microsoft Visual C++ Floating-Point Optimization，当分子分母同时为0依然不会异常，只能使用_ASSERT(!isnan(value))
	unsigned int control_word;
	_controlfp_s(&control_word, _EM_INEXACT | _EM_UNDERFLOW | _EM_OVERFLOW /*| _EM_ZERODIVIDE*/ | _EM_INVALID | _EM_DENORMAL, _MCW_EM);
#endif

	SetUnhandledExceptionFilter(OnException);

	return Demo().Run();
}

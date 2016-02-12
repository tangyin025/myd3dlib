#include "stdafx.h"
#include "Game.h"
#include "../myd3dbox/Component/Terrain.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo
	: public Game
{
public:
	std::vector<ComponentPtr> m_cmps;

	TerrainPtr m_Terrain;

public:
	Demo::Demo(void)
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
			m_Font->PushString(m_UIRender.get(), lpszText, my::Rectangle(ptVp, ptVp), Color, align);
		}
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		RegisterFileDir("Media");
		RegisterZipDir("Media.zip");
		RegisterFileDir("..\\demo2_3\\Media");
		RegisterZipDir("..\\demo2_3\\Media.zip");

		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		FModContext::SetMediaPath("sound\\");
		FModContext::LoadEventFile("aaa.fev");

		ExecuteCode("dofile \"Hud.lua\"");
		ExecuteCode("dofile \"StateMain.lua\"");
		RemoveDlg(m_Console);
		InsertDlg(m_Console);

		// ========================================================================================================
		// 示例代码
		// ========================================================================================================

		//MeshComponentPtr mesh_cmp(new MeshComponent(my::AABB(-100,100), my::Matrix4::Scaling(Vector3(0.05f)), false));
		//mesh_cmp->m_MeshRes.m_Path = "mesh/casual19_m_highpoly.mesh.xml";
		//MaterialPtr lambert1(new Material());
		//lambert1->m_Shader = "lambert1.fx";
		//lambert1->m_PassMask = RenderPipeline::PassMaskOpaque;
		//lambert1->m_MeshColor = Vector4(1,1,1,1);
		//lambert1->m_MeshTexture.m_Path = "texture/casual19_m_35.jpg";
		//lambert1->m_NormalTexture.m_Path = "texture/casual19_m_35_normal.png";
		//lambert1->m_SpecularTexture.m_Path = "texture/casual19_m_35_spec.png";
		//mesh_cmp->m_MaterialList.push_back(lambert1);
		//SimpleAnimatorPtr anim(new SimpleAnimator());
		//anim->m_SkeletonRes.m_Path = "mesh/casual19_m_highpoly.skeleton.xml";
		//mesh_cmp->m_Animator = anim;
		//mesh_cmp->RequestResource();
		//m_Root.AddComponent(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
		//m_cmps.push_back(mesh_cmp);

		//mesh_cmp.reset(new MeshComponent(my::AABB(-10,10), my::Matrix4::Identity(), false));
		//mesh_cmp->m_MeshRes.m_Path = "mesh/plane.mesh.xml";
		//lambert1.reset(new Material());
		//lambert1->m_Shader = "lambert1.fx";
		//lambert1->m_PassMask = RenderPipeline::PassMaskOpaque;
		//lambert1->m_MeshColor = Vector4(1,1,1,1);
		//lambert1->m_MeshTexture.m_Path = "texture/Checker.bmp";
		//lambert1->m_NormalTexture.m_Path = "texture/Normal.dds";
		//lambert1->m_SpecularTexture.m_Path = "texture/White.dds";
		//mesh_cmp->m_MaterialList.push_back(lambert1);
		//mesh_cmp->RequestResource();
		//m_Root.AddComponent(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
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
		//particle1->m_PassMask = RenderPipeline::PassMaskTransparent;
		//particle1->m_Shader = "particle1.fx";
		//emit_cmp->m_Material = particle1;
		//emit_cmp->RequestResource();
		//m_Root.AddComponent(emit_cmp.get(), emit_cmp->m_aabb.transform(emit_cmp->m_World), 0.1f);
		//m_cmps.push_back(emit_cmp);

		//RigidComponentPtr rigid_cmp(new RigidComponent(my::AABB(-5,5),my::Matrix4::Identity()));
		//rigid_cmp->m_RigidActor->createShape(PxBoxGeometry(1,1,1), *m_PxMaterial, PxTransform::createIdentity());
		//rigid_cmp->RequestResource();
		//m_cmps.push_back(rigid_cmp);

		//// 保存场景
		//std::ofstream ofs("aaa.xml");
		//boost::archive::xml_oarchive oa(ofs);
		//oa << boost::serialization::make_nvp("level", m_cmps);

		// 读取场景
		//IStreamBuff buff(OpenIStream("level.xml"));
		//std::istream istr(&buff);
		std::ifstream istr("ccc.xml");
		boost::archive::xml_iarchive ia(istr);
		ia >> boost::serialization::make_nvp("level", m_cmps);
		ia >> boost::serialization::make_nvp("terrain", m_Terrain);
		for (unsigned int i = 0; i < m_cmps.size(); i++)
		{
			m_Root.AddComponent(m_cmps[i].get(), m_cmps[i]->m_aabb.transform(Component::GetComponentWorld(m_cmps[i].get())), 0.1f);
			m_cmps[i]->RequestResource();
		}
		m_Terrain->RequestResource();
		for (unsigned int i = 0; i < m_Terrain->m_Chunks.size(); i++)
		{
			TerrainChunk * chunk = m_Terrain->m_Chunks[i].get();
			chunk->RequestResource();
			m_Root.AddComponent(chunk, chunk->m_aabb.transform(m_Terrain->m_World), 0.1f);
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
		m_cmps.clear();
		m_Terrain.reset();
		Game::OnDestroyDevice();
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		struct CallBack : public my::IQueryCallback
		{
			float m_fElapsedTime;
			CallBack(float fElapsedTime)
				: m_fElapsedTime(fElapsedTime)
			{
			}
			void operator() (OctComponent * oct_cmp, IntersectionTests::IntersectionType)
			{
				_ASSERT(dynamic_cast<Component *>(oct_cmp));
				Component * cmp = static_cast<Component *>(oct_cmp);
				cmp->Update(m_fElapsedTime);
			}
		};

		m_Root.QueryComponentAll(&CallBack(fElapsedTime));

		PushGrid(12, 5, 5, D3DCOLOR_ARGB(255,127,127,127), D3DCOLOR_ARGB(255,0,0,0), my::Matrix4::RotationX(D3DXToRadian(-90)));

		swprintf_s(&m_ScrInfos[0][0], m_ScrInfos[0].size(), L"Fps: %.2f", m_fFps);
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			swprintf_s(&m_ScrInfos[1+PassID][0], m_ScrInfos[1+PassID].size(), L"%S: %d", RenderPipeline::PassTypeToStr(PassID), m_PassDrawCall[PassID]);
		}

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

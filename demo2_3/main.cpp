#include "stdafx.h"
#include "Game.h"
//#include "Logic/MeshComponent.h"
#include "Logic/Logic.h"
#include "Component/Animator.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
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

		OgreMeshPtr mesh = LoadMesh("mesh/casual19_m_highpoly.mesh.xml");
		Matrix4 World(Matrix4::Scaling(Vector3(0.05f)));
		MeshComponentPtr mesh_cmp(new MeshComponent(mesh->m_aabb.transform(World), World, false));
		mesh_cmp->m_Mesh = mesh;
		for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
		{
			MaterialPtr material;
			char buff[128];
			sprintf_s(buff, sizeof(buff), "material/%s.xml", mesh->m_MaterialNameList[i].c_str());
			CachePtr cache = OpenIStream(buff)->GetWholeCache();
			membuf mb((char *)&(*cache)[0], cache->size());
			std::istream istr(&mb);
			boost::archive::xml_iarchive ar(istr);
			ar >> boost::serialization::make_nvp("Material", material);
			Material::ParameterList::iterator param_iter = material->m_Params.begin();
			for (; param_iter != material->m_Params.end(); param_iter++)
			{
				switch (param_iter->second->m_Type)
				{
				case Material::ParameterValue::ParameterValueTypeTexture:
					{
						Material::ParameterValueTexturePtr param = boost::dynamic_pointer_cast<Material::ParameterValueTexture>(param_iter->second);
						param->m_Res = LoadTexture(param->m_ResPath);
					}
					break;
				}
			}
			mesh_cmp->m_MaterialList.push_back(material);
		}
		m_cmps.push_back(mesh_cmp);

		// 保存场景
		std::ofstream ofs("scene.component_list.xml");
		boost::archive::xml_oarchive oa(ofs);
		oa << boost::serialization::make_nvp("scene.component_list", m_cmps);

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

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
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

#include "stdafx.h"
#include "Game.h"
//#include "Logic/MeshComponent.h"
#include "Logic/Logic.h"
#include "../myd3dbox/Component/Animator.h"

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
		RegisterFileDir("Media");
		RegisterZipDir("Media.zip");
		RegisterFileDir("..\\demo2_3\\Media");
		RegisterZipDir("..\\demo2_3\\Media.zip");

		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
		m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

		ExecuteCode("dofile \"Hud.lua\"");
		ExecuteCode("dofile \"StateMain.lua\"");
		RemoveDlg(m_Console);
		InsertDlg(m_Console);

		//// ========================================================================================================
		//// 声音系统
		//// ========================================================================================================
		//FMOD::Sound      *sound1;
		//FMOD::Channel    *channel = 0;
		//result = m_FModSystem->createSound("sound/drumloop.wav", FMOD_HARDWARE, NULL, &sound1);
		//FMOD_ERRCHECK(result);
		//result = sound1->setMode(FMOD_LOOP_NORMAL);    /* drumloop.wav has embedded loop points which automatically makes looping turn on, */
		//FMOD_ERRCHECK(result);                           /* so turn it off here.  We could have also just put FMOD_LOOP_OFF in the above CreateSound call. */
		//result = m_FModSystem->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);
		//FMOD_ERRCHECK(result);

		ActorPtr actor(new Actor(my::AABB(-FLT_MAX,FLT_MAX)));
		m_Actors.push_back(actor);
		MeshComponentPtr cmp = actor->CreateComponent<MeshComponent>();
		MeshComponentLoadMeshFromFile(cmp, "mesh/casual19_m_highpoly.mesh.xml");
		cmp->m_World = Matrix4::Scaling(0.1f,0.1f,0.1f);
		cmp = actor->CreateComponent<MeshComponent>();
		MeshComponentLoadMeshFromFile(cmp, "mesh/plane.mesh.xml");

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

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		Game::OnFrameMove(fTime, fElapsedTime);

		m_ScrInfos[0] = str_printf(L"%.2f", m_fFps);
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		PushGrid();

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
			Ray ray = m_Camera->CalculateRay(
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

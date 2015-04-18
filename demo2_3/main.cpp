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
	// ========================================================================================================
	// π«˜¿∂Øª≠
	// ========================================================================================================
	ActorPtr m_actor;
	MeshComponentPtr m_mesh_ins;
	EmitterComponentPtr m_emitter;
	ClothComponentPtr m_cloth_mesh;

	void OnKeyDown(my::InputEventArg * arg)
	{
		KeyboardEventArg & karg = *dynamic_cast<KeyboardEventArg *>(arg);
		Vector3 scale, pos; Quaternion rot;
		m_cloth_mesh->m_World.Decompose(scale, rot, pos);
		switch (karg.kc)
		{
		case VK_UP:
			pos.x += 1;
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		case VK_DOWN:
			pos.x -= 1;
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		case VK_LEFT:
			rot *= Quaternion::RotationAxis(Vector3(1,0,0), D3DXToRadian(30));
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		case VK_RIGHT:
			rot *= Quaternion::RotationAxis(Vector3(1,0,0), D3DXToRadian(-30));
			m_cloth_mesh->m_World = Matrix4::Compose(scale, rot, pos);
			break;
		}
	}

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

		// ========================================================================================================
		// π«˜¿∂Øª≠
		// ========================================================================================================
		m_actor.reset(new Actor());
		m_actor->m_Animator.reset(new SimpleAnimator());
		m_actor->m_Animator->m_Animation = LoadSkeleton("mesh/cloth.skeleton.xml");
		m_mesh_ins = AddMeshComponentFromFile(m_actor.get(), "mesh/tube.mesh.xml",true);
		m_emitter = AddEmitterComponentFromFile(m_actor.get(), "emitter/emitter_01.xml");
		m_cloth_mesh = AddClothComponentFromFile(m_actor.get(),
			boost::make_tuple(m_Cooking.get(), m_sdk.get(), m_PxScene.get()), "mesh/cloth.mesh.xml", "mesh/cloth.skeleton.xml", "joint5", PxClothCollisionData());
		m_Actors.push_back(m_actor);

		// ========================================================================================================
		// …˘“ÙœµÕ≥
		// ========================================================================================================
		FMOD::Sound      *sound1;
		FMOD::Channel    *channel = 0;
		result = m_FModSystem->createSound("sound/drumloop.wav", FMOD_HARDWARE, NULL, &sound1);
		FMOD_ERRCHECK(result);
		result = sound1->setMode(FMOD_LOOP_NORMAL);    /* drumloop.wav has embedded loop points which automatically makes looping turn on, */
		FMOD_ERRCHECK(result);                           /* so turn it off here.  We could have also just put FMOD_LOOP_OFF in the above CreateSound call. */
		result = m_FModSystem->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);
		FMOD_ERRCHECK(result);

		ExecuteCode("dofile \"StateMain.lua\"");

		Game::getSingleton().m_KeyPressedEvent = boost::bind(&Demo::OnKeyDown, this, _1);

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

		if (m_Camera)
		{
			m_Camera->OnFrameMove(fTime, fElapsedTime);
		}

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
		// ªÊ÷∆◊¯±Í
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
	// …Ë÷√crtdbgº‡ ”ƒ⁄¥Ê–π¬©
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}

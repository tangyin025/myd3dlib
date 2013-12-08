
#include "stdafx.h"
#include "Game.h"

using namespace my;

// ------------------------------------------------------------------------------------------
// Demo
// ------------------------------------------------------------------------------------------

class Demo : public Game
{
public:
	MeshPtr m_static_mesh;

	MeshPtr m_skined_mesh;

	OgreSkeletonAnimationPtr m_skel_anim;

	EffectPtr m_effect;

	MaterialPtr m_material;

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = Game::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		ExecuteCode("dofile \"GameStateMain.lua\"");

		m_Console->SetVisible(false);

		m_static_mesh = LoadMesh("mesh/tube.mesh.xml");

		m_skined_mesh = LoadMesh("mesh/tube.mesh.xml");

		m_skel_anim = LoadSkeleton("mesh/tube.skeleton.xml");

		m_effect = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());

		m_material = LoadMaterial("material/lambert1.txt");

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
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_Camera->m_View);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Camera->m_Proj);

		DrawHelper::DrawGrid(pd3dDevice);

		m_effect->SetVector("g_MaterialAmbientColor", Vector4(0,0,0,0));
		m_effect->SetVector("g_MaterialDiffuseColor", Vector4(1,1,1,1));
		m_effect->SetVector("g_LightDir", Vector3(1,1,1));
		m_effect->SetVector("g_LightDiffuse", Vector4(1,1,1,1));
		m_effect->SetTexture("g_MeshTexture", m_material->m_DiffuseTexture);
		m_effect->SetMatrix("g_mWorld", Matrix4::Identity());
		m_effect->SetMatrix("g_mWorldViewProjection", m_Camera->m_ViewProj);
		UINT passes = m_effect->Begin();
		for(UINT p = 0; p < passes; p++)
		{
			m_effect->BeginPass(p);
			m_static_mesh->DrawSubset(0);
			m_effect->EndPass();
		}
		m_effect->End();

		Game::OnFrameRender(pd3dDevice, fTime, fElapsedTime);
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
	// …Ë÷√crtdbgº‡ ”ƒ⁄¥Ê–π¬©
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	return Demo().Run();
}

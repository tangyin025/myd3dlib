
#include <mySingleton.h>
#include <myDxutApp.h>
#include <DXUTCamera.h>
#include <myException.h>
#include <myResource.h>
#include <myMesh.h>
#include <libc.h>

// ------------------------------------------------------------------------------------------
// MyDemo
// ------------------------------------------------------------------------------------------

class MyDemo : public my::DxutApp, public my::Singleton<MyDemo>
{
protected:
	CModelViewerCamera m_camera;

	CComPtr<ID3DXEffect> m_effect;

	CComPtr<ID3DXMesh> m_mesh;

	D3DMATERIAL9 m_material;

	CComPtr<IDirect3DTexture9> m_texture;

	void OnInit(void)
	{
		DxutApp::OnInit();

		// 初始化全局资源组
		my::ResourceMgr::getSingleton().RegisterZipArchive(L"Data.zip");
		my::ResourceMgr::getSingleton().RegisterFileDir(L"..\\..\\Common\\medias");
	}

	HRESULT OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutApp::OnD3D9CreateDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 初始化相机
		D3DXVECTOR3 vecEye(0.0f, 0.0f, 50.0f);
		D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
		m_camera.SetViewParams(&vecEye, &vecAt);
		m_camera.SetModelCenter(D3DXVECTOR3(0.0f, 15.0f, 0.0f));

		// 读取D3DX Effect文件
		my::ArchiveCachePtr cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("SimpleSample.fx")));
		CComPtr<ID3DXBuffer> d3dxbuffer;
		hres = D3DXCreateEffect(
			pd3dDevice, &(*cache)[0], cache->size(), NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &m_effect, &d3dxbuffer);
		if(FAILED(hres))
		{
			THROW_CUSEXCEPTION(
				str_printf(_T("compilation errors: \n%s"), mstringToWString((LPCSTR)d3dxbuffer->GetBufferPointer()).c_str()));
		}
		FAILED_THROW_D3DEXCEPTION(m_effect->SetTechnique("RenderScene"));

		// 从资源管理器中读出模型文件
		DWORD dwNumSubMeshes;
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_hres_all.mesh.xml")));
		my::LoadMeshFromOgreMesh(std::string((char *)&(*cache)[0], cache->size()), pd3dDevice, &dwNumSubMeshes, &m_mesh);

		// 所有的mesh使用同一种材质，同一张贴图
		m_material.Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
		m_material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		// 创建贴图
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_texture.jpg")));
		FAILED_THROW_D3DEXCEPTION(D3DXCreateTextureFromFileInMemory(pd3dDevice, &(*cache)[0], cache->size(), &m_texture));

		return S_OK;
	}

	HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutApp::OnD3D9ResetDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 重新设置相机的投影
		float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
		m_camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
		m_camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

		FAILED_THROW_D3DEXCEPTION(m_effect->OnResetDevice());

		return S_OK;
	}

	void OnD3D9LostDevice(void)
	{
		DxutApp::OnD3D9LostDevice();

		// 在这里处理在reset中创建的资源
		m_effect->OnLostDevice();
	}

	void OnD3D9DestroyDevice(void)
	{
		DxutApp::OnD3D9DestroyDevice();

		// 在这里销毁在create中创建的资源
		m_effect.Release();
		m_mesh.Release();
		m_texture.Release();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		DxutApp::OnFrameMove(fTime, fElapsedTime);

		// 在这里更新场景
		m_camera.FrameMove(fElapsedTime);
	}

	void RenderFrame(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		HRESULT hr;
		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 清理缓存背景及depth stencil
			V(pd3dDevice->Clear(
				0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));

			// 获得相机投影矩阵
			D3DXMATRIXA16 mWorld = *m_camera.GetWorldMatrix();
			D3DXMATRIXA16 mProj = *m_camera.GetProjMatrix();
			D3DXMATRIXA16 mView = *m_camera.GetViewMatrix();
			D3DXMATRIXA16 mWorldViewProjection = mWorld * mView * mProj;

			// 更新D3DX Effect值
			V(m_effect->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection));
			V(m_effect->SetMatrix("g_mWorld", &mWorld));
			V(m_effect->SetFloat("g_fTime", (float)fTime));

			V(m_effect->SetVector("g_MaterialAmbientColor", (D3DXVECTOR4 *)&m_material.Ambient));
			V(m_effect->SetVector("g_MaterialDiffuseColor", (D3DXVECTOR4 *)&m_material.Diffuse));
			V(m_effect->SetTexture("g_MeshTexture", m_texture));
			V(m_effect->SetFloatArray("g_LightDir", (float *)&D3DXVECTOR3(0.0f, 0.0f, -1.0f), 3));
			V(m_effect->SetVector("g_LightDiffuse", &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f)));

			// 渲染模型的两个部分，注意，头发的部分不要背面剔除
			UINT cPasses;
			V(m_effect->Begin(&cPasses, 0));
			for(UINT p = 0; p < cPasses; ++p)
			{
				V(m_effect->BeginPass(p));
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
				V(m_mesh->DrawSubset(1));
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
				V(m_mesh->DrawSubset(0));
				V(m_effect->EndPass());
			}
			V(m_effect->End());

			V(pd3dDevice->EndScene());
		}
	}

	LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		LRESULT hres;
		if(FAILED(hres = DxutApp::MsgProc(
			hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing)
		{
			return hres;
		}

		// 相机消息处理
		return m_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
	}
};

my::Singleton<MyDemo>::DrivedClassPtr my::Singleton<MyDemo>::s_ptr;

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

	return MyDemo::getSingleton().Run(true, 800, 600);
}

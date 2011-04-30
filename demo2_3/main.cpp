
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

	static const unsigned int SHADOWMAP_SIZE = 1024;

	CComPtr<IDirect3DTexture9> m_shadowMapRT;

	CComPtr<IDirect3DSurface9> m_shadowMapDS;

	bool IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		if(!DxutApp::IsD3D9DeviceAcceptable(
			pCaps, AdapterFormat, BackBufferFormat, bWindowed))
		{
			return false;
		}

		IDirect3D9 * pD3D = DXUTGetD3D9Object();
		if(FAILED(pD3D->CheckDeviceFormat(
			pCaps->AdapterOrdinal, pCaps->DeviceType, AdapterFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_CUBETEXTURE, D3DFMT_R32F)))
		{
			return false;
		}

		return true;
	}

	void OnInit(void)
	{
		DxutApp::OnInit();

		// 初始化资源管理器收索路径
		my::ResourceMgr::getSingleton().RegisterFileDir(_T("."));
		my::ResourceMgr::getSingleton().RegisterFileDir(_T("..\\..\\Common\\medias"));
		my::ResourceMgr::getSingleton().RegisterZipArchive(_T("Data.zip"));
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
		if(FAILED(hres = D3DXCreateEffect(
			pd3dDevice, &(*cache)[0], cache->size(), NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &m_effect, &d3dxbuffer)))
		{
			THROW_CUSEXCEPTION(
				str_printf(_T("compilation errors: \n%s"), mstringToWString((LPCSTR)d3dxbuffer->GetBufferPointer()).c_str()));
		}

		// 从资源管理器中读出模型文件
		DWORD dwNumSubMeshes;
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_hres_all.mesh.xml")));
		my::LoadMeshFromOgreMesh(std::string((char *)&(*cache)[0], cache->size()), pd3dDevice, &dwNumSubMeshes, &m_mesh);

		// 所有的mesh使用同一种材质，同一张贴图
		m_material.Ambient = D3DXCOLOR(0.27f, 0.27f, 0.27f, 1.0f);
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

		// 重置d3dx effect
		FAILED_THROW_D3DEXCEPTION(m_effect->OnResetDevice());

		// 创建shadow map render target
		FAILED_THROW_D3DEXCEPTION(pd3dDevice->CreateTexture(
			SHADOWMAP_SIZE,
			SHADOWMAP_SIZE,
			1,
			D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F,
			D3DPOOL_DEFAULT,
			&m_shadowMapRT,
			NULL));

		// 创建shadow map depth scentil
		DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
		FAILED_THROW_D3DEXCEPTION(pd3dDevice->CreateDepthStencilSurface(
			SHADOWMAP_SIZE,
			SHADOWMAP_SIZE,
			d3dSettings.d3d9.pp.AutoDepthStencilFormat,
			D3DMULTISAMPLE_NONE,
			0,
			TRUE,
			&m_shadowMapDS,
			NULL));

		return S_OK;
	}

	void OnD3D9LostDevice(void)
	{
		DxutApp::OnD3D9LostDevice();

		// 在这里处理在reset中创建的资源
		m_effect->OnLostDevice();
		m_shadowMapRT.Release();
		m_shadowMapDS.Release();
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
		// 获得相机投影矩阵
		D3DXMATRIXA16 mWorld = *m_camera.GetWorldMatrix();
		D3DXMATRIXA16 mProj = *m_camera.GetProjMatrix();
		D3DXMATRIXA16 mView = *m_camera.GetViewMatrix();
		D3DXMATRIXA16 mWorldViewProjection = mWorld * mView * mProj;

		// 计算光照的透视变换
		D3DXMATRIXA16 mViewLight;
		D3DXMatrixLookAtLH(
			&mViewLight,
			&D3DXVECTOR3(0.0f, 0.0f, 50.0f),
			&D3DXVECTOR3(0.0f, 0.0f, 0.0f),
			&D3DXVECTOR3(0.0f, 1.0f, 0.0f));
		D3DXMATRIXA16 mProjLight;
		D3DXMatrixOrthoLH(&mProjLight, 50, 50, 25, 75);
		D3DXMATRIXA16 mWorldViewProjLight = mWorld * mViewLight * mProjLight;

		HRESULT hr;
		LPDIRECT3DSURFACE9 pOldRT = NULL;
		V(pd3dDevice->GetRenderTarget(0, &pOldRT));
		LPDIRECT3DSURFACE9 pShadowSurf;
		V(m_shadowMapRT->GetSurfaceLevel(0, &pShadowSurf));
		V(pd3dDevice->SetRenderTarget(0, pShadowSurf));
		SAFE_RELEASE(pShadowSurf);
		LPDIRECT3DSURFACE9 pOldDS = NULL;
		V(pd3dDevice->GetDepthStencilSurface(&pOldDS));
		V(pd3dDevice->SetDepthStencilSurface(m_shadowMapDS));
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新d3dx effect变量
			V(m_effect->SetMatrix("g_mWorldViewProjectionLight", &mWorldViewProjLight));
			V(m_effect->SetTechnique("RenderShadow"));

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
		V(pd3dDevice->SetRenderTarget(0, pOldRT));
		V(pd3dDevice->SetDepthStencilSurface(pOldDS));
		SAFE_RELEASE(pOldRT);
		SAFE_RELEASE(pOldDS);

		// 清理缓存背景及depth stencil
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新D3DX Effect值
			V(m_effect->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection));
			V(m_effect->SetMatrix("g_mWorld", &mWorld));
			V(m_effect->SetFloat("g_fTime", (float)fTime));

			V(m_effect->SetVector("g_MaterialAmbientColor", (D3DXVECTOR4 *)&m_material.Ambient));
			V(m_effect->SetVector("g_MaterialDiffuseColor", (D3DXVECTOR4 *)&m_material.Diffuse));
			V(m_effect->SetTexture("g_MeshTexture", m_texture));
			V(m_effect->SetFloatArray("g_LightDir", (float *)&D3DXVECTOR3(0.0f, 0.0f, -1.0f), 3));
			V(m_effect->SetVector("g_LightDiffuse", &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f)));

			V(m_effect->SetTexture("g_ShadowTexture", m_shadowMapRT));
			V(m_effect->SetMatrix("g_mWorldViewProjectionLight", &mWorldViewProjLight));
			V(m_effect->SetTechnique("RenderScene"));

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

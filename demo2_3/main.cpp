
#include "myd3dlib.h"
#include <DXUTCamera.h>

// ------------------------------------------------------------------------------------------
// MyDemo
// ------------------------------------------------------------------------------------------

class MyDemo : public my::DxutSample
{
protected:
	CModelViewerCamera m_camera;

	my::EffectPtr m_effect;

	my::MeshPtr m_mesh;

	my::TexturePtr m_texture;

	static const unsigned int SHADOWMAP_SIZE = 1024;

	my::TexturePtr m_shadowMapRT;

	my::SurfacePtr m_shadowMapDS;

	HRESULT OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutSample::OnD3D9CreateDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 初始化相机
		D3DXVECTOR3 vecEye(0.0f, 0.0f, -50.0f);
		D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
		m_camera.SetViewParams(&vecEye, &vecAt);
		m_camera.SetModelCenter(D3DXVECTOR3(0.0f, 15.0f, 0.0f));

		// 读取D3DX Effect文件
		my::CachePtr cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("SimpleSample.fx")));
		m_effect = my::Effect::CreateEffect(pd3dDevice, &(*cache)[0], cache->size());

		// 从资源管理器中读出模型文件
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_hres_all.mesh.xml")));
		m_mesh = my::Mesh::CreateMeshFromOgreMesh(pd3dDevice, (char *)&(*cache)[0], cache->size());

		// 创建贴图
		cache = my::ReadWholeCacheFromStream(
			my::ResourceMgr::getSingleton().OpenArchiveStream(_T("jack_texture.jpg")));
		m_texture = my::Texture::CreateTextureFromFileInMemory(pd3dDevice, &(*cache)[0], cache->size());

		return S_OK;
	}

	HRESULT OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hres;
		if(FAILED(hres = DxutSample::OnD3D9ResetDevice(
			pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hres;
		}

		// 重新设置相机的投影
		float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
		m_camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
		m_camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

		// 创建用于shadow map的render target，使用D3DXCreateTexture可以为不支持设备创建兼容贴图
		m_shadowMapRT = my::Texture::CreateAdjustedTexture(
			pd3dDevice,
			SHADOWMAP_SIZE,
			SHADOWMAP_SIZE,
			1,
			D3DUSAGE_RENDERTARGET,
			D3DFMT_R32F,
			D3DPOOL_DEFAULT);

		// 创建用于shadow map的depth scentil
		DXUTDeviceSettings d3dSettings = DXUTGetDeviceSettings();
		m_shadowMapDS = my::Surface::CreateDepthStencilSurface(
			pd3dDevice,
			SHADOWMAP_SIZE,
			SHADOWMAP_SIZE,
			d3dSettings.d3d9.pp.AutoDepthStencilFormat);

		return S_OK;
	}

	void OnD3D9LostDevice(void)
	{
		DxutSample::OnD3D9LostDevice();

		// 在这里处理在reset中创建的资源
		m_shadowMapRT = my::TexturePtr();
		m_shadowMapDS = my::SurfacePtr();
	}

	void OnD3D9DestroyDevice(void)
	{
		DxutSample::OnD3D9DestroyDevice();
	}

	void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		DxutSample::OnFrameMove(fTime, fElapsedTime);

		// 在这里更新场景
		m_camera.FrameMove(fElapsedTime);
	}

	void OnRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		// 获得相机投影矩阵
		my::Matrix4 mWorld = *(my::Matrix4 *)m_camera.GetWorldMatrix();
		my::Matrix4 mProj = *(my::Matrix4 *)m_camera.GetProjMatrix();
		my::Matrix4 mView = *(my::Matrix4 *)m_camera.GetViewMatrix();
		my::Matrix4 mWorldViewProjection = mWorld * mView * mProj;

		// 计算光照的透视变换
		my::Matrix4 mViewLight(my::Matrix4::LookAtLH(
			my::Vector3(0.0f, 0.0f, -50.0f),
			my::Vector3(0.0f, 0.0f, 0.0f),
			my::Vector3(0.0f, 1.0f, 0.0f)));
		my::Matrix4 mProjLight(my::Matrix4::OrthoLH(50, 50, 25, 75));
		my::Matrix4 mWorldViewProjLight = mWorld * mViewLight * mProjLight;

		// 将shadow map作为render target，注意保存恢复原来的render target
		HRESULT hr;
		CComPtr<IDirect3DSurface9> oldRt;
		V(pd3dDevice->GetRenderTarget(0, &oldRt));
		V(pd3dDevice->SetRenderTarget(0, m_shadowMapRT->GetSurfaceLevel(0)));
		CComPtr<IDirect3DSurface9> oldDs = NULL;
		V(pd3dDevice->GetDepthStencilSurface(&oldDs));
		V(pd3dDevice->SetDepthStencilSurface(m_shadowMapDS->m_ptr));
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00ffffff, 1.0f, 0));
		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新d3dx effect变量
			m_effect->SetMatrix("g_mWorldViewProjectionLight", mWorldViewProjLight);
			m_effect->SetTechnique("RenderShadow");

			// 渲染模型的两个部分，注意，头发的部分不要背面剔除
			UINT cPasses = m_effect->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_effect->BeginPass(p);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
				m_mesh->DrawSubset(1);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
				m_mesh->DrawSubset(0);
				m_effect->EndPass();
			}
			m_effect->End();

			V(pd3dDevice->EndScene());
		}

		// 注意恢复原来的render target
		V(pd3dDevice->SetRenderTarget(0, oldRt));
		V(pd3dDevice->SetDepthStencilSurface(oldDs));
		oldRt.Release();
		oldDs.Release();

		// 清理缓存背景及depth stencil
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 72, 72, 72), 1, 0));

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			// 更新D3DX Effect值
			m_effect->SetMatrix("g_mWorldViewProjection", mWorldViewProjection);
			m_effect->SetMatrix("g_mWorld", mWorld);
			m_effect->SetFloat("g_fTime", (float)fTime);

			// 所有的mesh使用同一种材质，同一张贴图
			m_effect->SetVector("g_MaterialAmbientColor", my::Vector4(0.27f, 0.27f, 0.27f, 1.0f));
			m_effect->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			m_effect->SetTexture("g_MeshTexture", m_texture->m_ptr);
			m_effect->SetFloatArray("g_LightDir", (float *)&my::Vector3(0.0f, 0.0f, 1.0f), 3);
			m_effect->SetVector("g_LightDiffuse", my::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

			// 设置阴影贴图，及光源变换
			m_effect->SetTexture("g_ShadowTexture", m_shadowMapRT->m_ptr);
			m_effect->SetMatrix("g_mWorldViewProjectionLight", mWorldViewProjLight);
			m_effect->SetTechnique("RenderScene");

			// 渲染模型的两个部分，注意，头发的部分不要背面剔除
			UINT cPasses = m_effect->Begin();
			for(UINT p = 0; p < cPasses; ++p)
			{
				m_effect->BeginPass(p);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
				m_mesh->DrawSubset(1);
				V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
				m_mesh->DrawSubset(0);
				m_effect->EndPass();
			}
			m_effect->End();

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
		if(FAILED(hres = DxutSample::MsgProc(
			hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing)) || *pbNoFurtherProcessing)
		{
			return hres;
		}

		// 相机消息处理
		return m_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
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

	// 初始化资源管理器收索路径
	my::ResourceMgr::getSingleton().RegisterFileDir(_T("."));
	my::ResourceMgr::getSingleton().RegisterFileDir(_T("..\\..\\Common\\medias"));
	my::ResourceMgr::getSingleton().RegisterZipArchive(_T("Data.zip"));

	return MyDemo().Run(true, 800, 600);
}

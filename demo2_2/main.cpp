#include <myD3dLib.h>
#include <boost/bind.hpp>

class MyDemo
	: public my::DxutApp
	, public my::ResourceMgr
{
protected:
	CComPtr<ID3DXFont> m_font;

	CComPtr<ID3DXSprite> m_sprite;

public:
	virtual bool IsDeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		return true;
	}

	virtual bool ModifyDeviceSettings(
		DXUTD3D9DeviceSettings * pDeviceSettings)
	{
		return true;
	}

	void foo(my::DeviceRelatedObjectBasePtr res)
	{
		//my::TexturePtr tex = boost::dynamic_pointer_cast<my::Texture>(res);

		//my::OgreMeshPtr mesh = boost::dynamic_pointer_cast<my::OgreMesh>(res);

		//my::OgreSkeletonAnimationPtr skel = boost::dynamic_pointer_cast<my::OgreSkeletonAnimation>(res);

		//my::EffectPtr eff = boost::dynamic_pointer_cast<my::Effect>(res);

		my::FontPtr fnt = boost::dynamic_pointer_cast<my::Font>(res);
	}

	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if(FAILED(hr = D3DXCreateFont(
			pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"), &m_font)))
		{
			return hr;
		}

		ResourceMgr::RegisterFileDir("../demo2_3/Media");

		ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

		//LoadTextureAsync("texture/galileo_cross.dds", boost::bind(&MyDemo::foo, this, _1));

		//my::BaseTexturePtr tex = LoadTexture("texture/galileo_cross.dds");

		//LoadMeshAsync(("sportive03_f.mesh.xml", boost::bind(&MyDemo::foo, this, _1));

		//LoadSkeletonAsync(("sportive03_f.skeleton.xml", boost::bind(&MyDemo::foo, this, _1));

		//my::EffectPtr eff = LoadEffect("shader/SimpleSample.fx", EffectMacroPairList());

		//LoadFontAsync("font/wqy-microhei.ttc", 13, boost::bind(&MyDemo::foo, this, _1));

		my::MaterialPtr mat = LoadMaterial("material/lambert1.xml");

		return S_OK;
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		m_font->OnResetDevice();

		if(FAILED(hr = D3DXCreateSprite(pd3dDevice, &m_sprite)))
		{
			return hr;
		}

		ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		m_font->OnLostDevice();

		m_sprite.Release();

		ResourceMgr::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		m_font.Release();

		ResourceMgr::OnDestroyDevice();
	}

	virtual void OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
		CheckResource();
	}

	virtual void OnFrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		// Clear the render target and the zbuffer 
		V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 45, 50, 170 ), 1.0f, 0 ) );

		// Render the scene
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			V(m_sprite->Begin(D3DXSPRITE_ALPHABLEND));
			V(m_font->DrawTextW(m_sprite, m_strFPS, wcslen(m_strFPS), CRect(5,5,100,100), DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(1.0f,1.0f,0.0f,1.0f)));
			V(m_sprite->End());
			V( pd3dDevice->EndScene() );
		}
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		return 0;
	}
};

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	return MyDemo().Run();
}

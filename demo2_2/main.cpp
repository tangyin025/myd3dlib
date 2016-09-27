#include <myD3dLib.h>
#include <boost/bind.hpp>
#include <Opcode.h>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

using namespace my;

class Demo
	: public DxutApp
	, public ResourceMgr
	, public DialogMgr
{
protected:
	CComPtr<ID3DXFont> m_font;

	CComPtr<ID3DXSprite> m_sprite;

	std::vector<DeviceResourceBasePtr> m_reses;

	my::UIRenderPtr m_UIRender;

public:
	Demo(void)
	{
	}

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

		m_UIRender.reset(new my::UIRender(pd3dDevice));

		if (!(m_UIRender->m_TexWhite = LoadTexture("texture/White.dds")))
		{
			THROW_CUSEXCEPTION("create m_UIRender->m_TexWhite failed");
		}

		//DialogPtr dlg(new Dialog());
		//dlg->m_Skin.reset(new ControlSkin());
		//dlg->m_Skin->m_FontPath = "font/wqy-microhei.ttc";
		//dlg->m_Skin->m_FontHeight = 13;
		//dlg->m_Skin->m_Font = my::ResourceMgr::getSingleton().LoadFont(dlg->m_Skin->m_FontPath, dlg->m_Skin->m_FontHeight);
		//dlg->m_Skin->m_Image.reset(new ControlImage());
		//dlg->m_Skin->m_Image->m_TexturePath = "texture/CommonUI.png";
		//dlg->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture(dlg->m_Skin->m_Image->m_TexturePath);
		//dlg->m_Skin->m_Image->m_Rect = CRect(52,43,67,58);
		//dlg->m_Skin->m_Image->m_Border = CRect(7,7,7,7);

		//std::ofstream ostr("ui.xml");
		//boost::archive::xml_oarchive oa(ostr);
		//oa << boost::serialization::make_nvp("ui", dlg);

		DialogPtr dlg;
		std::ifstream istr("ui.xml");
		boost::archive::xml_iarchive ia(istr);
		ia >> boost::serialization::make_nvp("ui", dlg);

		DialogMgr::InsertDlg(dlg);

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
		m_UIRender.reset();

		m_font.Release();

		RemoveAllDlg();

		ResourceMgr::OnDestroyDevice();
	}

	void OnFrameRender(
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
			wchar_t buff[256];
			int len = swprintf_s(buff, _countof(buff), L"%.2f", m_fFps);
			V(m_font->DrawTextW(m_sprite, buff, len, CRect(5,5,100,100), DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(1.0f,1.0f,0.0f,1.0f)));
			V(m_sprite->End());

			m_UIRender->Begin();
			m_UIRender->SetWorld(Matrix4::identity);
			m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
			DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime);
			m_UIRender->End();
			V( pd3dDevice->EndScene() );
		}
	}

	virtual void OnFrameTick(double fTime, float fElapsedTime)
	{
		CheckIORequests();

		OnFrameRender(m_d3dDevice, fTime, fElapsedTime);

		Present(0,0,0,0);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		if((*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam)))
		{
			return 0;
		}
		return 0;
	}

	virtual void OnResourceFailed(const std::string & error_str)
	{
	}
};

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	return Demo().Run();
}

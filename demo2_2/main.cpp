#include <myD3dLib.h>
#include <boost/bind.hpp>
#include <Opcode.h>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

using namespace my;

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	UINT m_Passes;

public:
	EffectUIRender(IDirect3DDevice9 * pd3dDevice, my::EffectPtr effect)
		: UIRender(pd3dDevice)
		, m_UIEffect(effect)
		, m_Passes(0)
	{
		_ASSERT(m_UIEffect);
	}

	void Begin(void)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetVector("g_ScreenDim", Vector4(
				(float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Width, (float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Height, 0, 0));
			m_Passes = m_UIEffect->Begin();
		}
	}

	void End(void)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->End();
			m_Passes = 0;
		}
	}

	void SetWorld(const Matrix4 & World)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetMatrix("g_World", World);
		}
	}

	void SetViewProj(const Matrix4 & ViewProj)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetMatrix("g_ViewProj", ViewProj);
		}
	}

	void Flush(void)
	{
		if(m_UIEffect->m_ptr)
		{
			for(UINT p = 0; p < m_Passes; p++)
			{
				m_UIEffect->BeginPass(p);
				UIRender::Flush();
				m_UIEffect->EndPass();
			}
		}
	}
};

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

		m_UIRender.reset(new EffectUIRender(pd3dDevice, LoadEffect("shader/UIEffect.fx", "")));

		DialogPtr dlg(new Dialog());
		dlg->m_Color = D3DCOLOR_ARGB(150,0,0,0);
		dlg->m_Size = Vector2(640,480);
		dlg->m_Skin.reset(new ControlSkin());
		dlg->m_Skin->m_Image.reset(new ControlImage());
		dlg->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		dlg->m_Skin->m_Image->m_Rect = my::Rectangle(154,43,156,45);
		dlg->m_Skin->m_Image->m_Border = Vector4(0,0,0,0);

		ControlPtr lbl_title(new Control());
		lbl_title->m_Location = Vector2(17,13);
		lbl_title->m_Size = Vector2(256,42);
		lbl_title->m_Color = D3DCOLOR_ARGB(255,255,255,255);
		lbl_title->m_Skin.reset(new ControlSkin());
		lbl_title->m_Skin->m_Image.reset(new ControlImage());
		lbl_title->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		lbl_title->m_Skin->m_Image->m_Rect = my::Rectangle(0,0,256,42);
		lbl_title->m_Skin->m_Image->m_Border = Vector4(0,0,0,0);
		dlg->InsertControl(lbl_title);

		ButtonPtr btn_ok(new Button());
		btn_ok->m_Location = Vector2(230,439);
		btn_ok->m_Size = Vector2(80,32);
		btn_ok->m_Text = L"OK";
		ButtonSkinPtr btn_ok_skin(new ButtonSkin());
		btn_ok_skin->m_Font = my::ResourceMgr::getSingleton().LoadFont("font/wqy-microhei.ttc", 13);
		btn_ok_skin->m_Image.reset(new ControlImage());
		btn_ok_skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		btn_ok_skin->m_Image->m_Rect = my::Rectangle(52,43,68,59);
		btn_ok_skin->m_Image->m_Border = Vector4(7,7,7,7);
		btn_ok_skin->m_DisabledImage.reset(new ControlImage());
		btn_ok_skin->m_DisabledImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		btn_ok_skin->m_DisabledImage->m_Rect = my::Rectangle(1,43,17,59);
		btn_ok_skin->m_DisabledImage->m_Border = Vector4(7,7,7,7);
		btn_ok_skin->m_PressedImage.reset(new ControlImage());
		btn_ok_skin->m_PressedImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		btn_ok_skin->m_PressedImage->m_Rect = my::Rectangle(18,43,34,59);
		btn_ok_skin->m_PressedImage->m_Border = Vector4(7,7,7,7);
		btn_ok_skin->m_MouseOverImage.reset(new ControlImage());
		btn_ok_skin->m_MouseOverImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		btn_ok_skin->m_MouseOverImage->m_Rect = my::Rectangle(35,43,51,59);
		btn_ok_skin->m_MouseOverImage->m_Border = Vector4(7,7,7,7);
		btn_ok_skin->m_TextAlign = Font::AlignCenterMiddle;
		btn_ok_skin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
		btn_ok_skin->m_PressedOffset = Vector2(1,2);
		btn_ok->m_Skin = btn_ok_skin;
		dlg->InsertControl(btn_ok);

		std::ofstream ostr("ui.xml");
		boost::archive::polymorphic_xml_oarchive oa(ostr);
		oa << boost::serialization::make_nvp("ui", dlg);

		//DialogPtr dlg;
		//std::ifstream istr("ui.xml");
		//boost::archive::polymorphic_xml_iarchive ia(istr);
		//ia >> boost::serialization::make_nvp("ui", dlg);

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
};

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	return Demo().Run();
}

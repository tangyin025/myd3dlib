#include <myD3dLib.h>
#include <boost/bind/bind.hpp>
//#include <Opcode.h>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/multi_array.hpp>
#include <fstream>
#include <PrintCallStack.h>
#include <shlobj.h>
#include "Material.h"
#include "SoundContext.h"

using namespace my;

class EffectUIRender
	: public my::UIRender
{
public:
	my::EffectPtr m_UIEffect;

	D3DXHANDLE handle_ScreenDim;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_ViewProj;

	UINT m_Passes;

public:
	EffectUIRender(void)
		: m_Passes(0)
		, handle_ScreenDim(NULL)
		, handle_World(NULL)
		, handle_ViewProj(NULL)
	{
	}

	HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if (FAILED(hr = UIRender::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_UIEffect = my::ResourceMgr::getSingleton().LoadEffect("shader/UIEffect.fx", "");
		if (!m_UIEffect)
		{
			return S_FALSE;
		}

		BOOST_VERIFY(handle_ScreenDim = m_UIEffect->GetParameterByName(NULL, "g_ScreenDim"));
		BOOST_VERIFY(handle_World = m_UIEffect->GetParameterByName(NULL, "g_World"));
		BOOST_VERIFY(handle_ViewProj = m_UIEffect->GetParameterByName(NULL, "g_ViewProj"));

		return S_OK;
	}

	HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if (FAILED(hr = UIRender::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		return S_OK;
	}

	void OnLostDevice(void)
	{
		UIRender::OnLostDevice();
	}

	void OnDestroyDevice(void)
	{
		UIRender::OnDestroyDevice();

		m_Device.Release();

		m_UIEffect.reset();
	}

	void Begin(void)
	{
		m_LayerDrawCall = 0;

		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetVector(handle_ScreenDim, Vector4(
				(float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Width, (float)DxutApp::getSingleton().m_BackBufferSurfaceDesc.Height, 0, 0));
			m_Passes = m_UIEffect->Begin(D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE);
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
			m_UIEffect->SetMatrix(handle_World, World);
		}
	}

	void SetViewProj(const Matrix4 & ViewProj)
	{
		if(m_UIEffect->m_ptr)
		{
			m_UIEffect->SetMatrix(handle_ViewProj, ViewProj);
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
	, public FontLibrary
	, public ResourceMgr
	, public DialogMgr
	, public SoundContext
	, public my::InputMgr
{
protected:
	CComPtr<ID3DXFont> m_d3dfont;

	CComPtr<ID3DXSprite> m_sprite;

	std::vector<DeviceResourceBasePtr> m_reses;

	my::UIRenderPtr m_UIRender;

	my::FontPtr m_font;

	my::Texture2DPtr m_Tex;

	my::DialogPtr m_Dlg;

	Mp3 m_mp3;

public:
	Demo(void)
		: m_UIRender(new EffectUIRender())
		, InputMgr(0)
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
		if (FAILED(hr = DxutApp::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		if(FAILED(hr = D3DXCreateFont(
			pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"), &m_d3dfont)))
		{
			return hr;
		}

		ResourceMgr::RegisterFileDir("../demo2_3/Media");

		if (FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		if (!SoundContext::Init(m_wnd->m_hWnd))
		{
			return S_FALSE;
		}

		InputMgr::Create(m_hinst, m_wnd->m_hWnd);

		ResourceMgr::StartIORequestProc(2);

		if (FAILED(hr = m_UIRender->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_font = LoadFont("font/wqy-microhei.ttc", 120, 0);
		if (!m_font)
		{
			return S_FALSE;
		}

		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL,
			CSIDL_APPDATA | CSIDL_FLAG_CREATE,
			NULL,
			0,
			szPath)))
		{
			PathAppend(szPath, TEXT("New Doc.txt"));
			//HANDLE hFile = CreateFile(szPath, ...);
		}

		//DialogPtr dlg(new Dialog());
		//dlg->m_Color = D3DCOLOR_ARGB(150,0,0,0);
		//dlg->m_Size = Vector2(640,480);
		//dlg->m_Skin.reset(new ControlSkin());
		//dlg->m_Skin->m_Image.reset(new ControlImage());
		//dlg->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		//dlg->m_Skin->m_Image->m_Rect = my::Rectangle(154,43,156,45);
		//dlg->m_Skin->m_Image->m_Border = Vector4(0,0,0,0);

		//ControlPtr lbl_title(new Control());
		//lbl_title->m_Location = Vector2(17,13);
		//lbl_title->m_Size = Vector2(256,42);
		//lbl_title->m_Color = D3DCOLOR_ARGB(255,255,255,255);
		//lbl_title->m_Skin.reset(new ControlSkin());
		//lbl_title->m_Skin->m_Image.reset(new ControlImage());
		//lbl_title->m_Skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		//lbl_title->m_Skin->m_Image->m_Rect = my::Rectangle(0,0,256,42);
		//lbl_title->m_Skin->m_Image->m_Border = Vector4(0,0,0,0);
		//dlg->InsertControl(lbl_title);

		//ButtonPtr btn_ok(new Button());
		//btn_ok->m_Location = Vector2(230,439);
		//btn_ok->m_Size = Vector2(80,32);
		//btn_ok->m_Text = L"OK";
		//ButtonSkinPtr btn_ok_skin(new ButtonSkin());
		//btn_ok_skin->m_Font = my::ResourceMgr::getSingleton().LoadFont("font/wqy-microhei.ttc", 13);
		//btn_ok_skin->m_Image.reset(new ControlImage());
		//btn_ok_skin->m_Image->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		//btn_ok_skin->m_Image->m_Rect = my::Rectangle(52,43,68,59);
		//btn_ok_skin->m_Image->m_Border = Vector4(7,7,7,7);
		//btn_ok_skin->m_DisabledImage.reset(new ControlImage());
		//btn_ok_skin->m_DisabledImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		//btn_ok_skin->m_DisabledImage->m_Rect = my::Rectangle(1,43,17,59);
		//btn_ok_skin->m_DisabledImage->m_Border = Vector4(7,7,7,7);
		//btn_ok_skin->m_PressedImage.reset(new ControlImage());
		//btn_ok_skin->m_PressedImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		//btn_ok_skin->m_PressedImage->m_Rect = my::Rectangle(18,43,34,59);
		//btn_ok_skin->m_PressedImage->m_Border = Vector4(7,7,7,7);
		//btn_ok_skin->m_MouseOverImage.reset(new ControlImage());
		//btn_ok_skin->m_MouseOverImage->m_Texture = my::ResourceMgr::getSingleton().LoadTexture("texture/CommonUI.png");
		//btn_ok_skin->m_MouseOverImage->m_Rect = my::Rectangle(35,43,51,59);
		//btn_ok_skin->m_MouseOverImage->m_Border = Vector4(7,7,7,7);
		//btn_ok_skin->m_TextAlign = Font::AlignCenterMiddle;
		//btn_ok_skin->m_TextColor = D3DCOLOR_ARGB(255,255,255,255);
		//btn_ok_skin->m_PressedOffset = Vector2(1,2);
		//btn_ok->m_Skin = btn_ok_skin;
		//dlg->InsertControl(btn_ok);

		//std::ofstream ostr("ui.xml");
		//boost::archive::polymorphic_xml_oarchive oa(ostr);
		//oa << boost::serialization::make_nvp("ui", dlg);

		////DialogPtr dlg;
		////std::ifstream istr("ui.xml");
		////boost::archive::polymorphic_xml_iarchive ia(istr);
		////ia >> boost::serialization::make_nvp("ui", dlg);

		//DialogMgr::InsertDlg(dlg);

		m_Tex.reset(new my::Texture2D());
		m_Tex->CreateTextureFromFile(_T("four-holes.png"));
		D3DSURFACE_DESC desc = m_Tex->GetLevelDesc(0);

		//Material mtl;
		//mtl.m_Shader = "shader/mtl_BlinnPhong.fx";
		//mtl.ParseShaderParameters();

		m_Dlg.reset(new Dialog(NamedObject::MakeUniqueName("dialog").c_str()));
		m_Dlg->m_EnableDrag = true;
		m_Dlg->m_x = UDim(0, 10);
		m_Dlg->m_y = UDim(0, 30);
		m_Dlg->m_Width = UDim(0, desc.Width);
		m_Dlg->m_Height = UDim(0, desc.Height);
		m_Dlg->m_Skin.reset(new ControlSkin());
		m_Dlg->m_Skin->m_Image.reset(new ControlImage());
		m_Dlg->m_Skin->m_Image->m_Texture = m_Tex;
		m_Dlg->m_Skin->m_Image->m_Rect = my::Rectangle(0, 0, desc.Width, desc.Height);
		m_Dlg->m_EventMouseClick = boost::bind(&Demo::OnMouseClick, this, boost::placeholders::_1);
		DialogMgr::InsertDlg(m_Dlg.get());

		ControlImagePtr image(new ControlImage());
		image->m_Texture = LoadTexture("texture/CommonUI.png");
		image->m_Rect = Rectangle::LeftTop(52, 43, 16, 16);
		image->m_Border = Vector4(7, 7, 7, 7);

		ControlImagePtr image2(new ControlImage());
		image2->m_Texture = LoadTexture("texture/CommonUI.png");
		image2->m_Rect = Rectangle::LeftTop(1, 43, 16, 16);
		image2->m_Border = Vector4(7, 7, 7, 7);

		ListBoxSkinPtr skin(new ListBoxSkin());
		skin->m_Font = LoadFont("font/wqy-microhei.ttc", 13, 0);
		skin->m_TextColor = D3DCOLOR_ARGB(255, 255, 0, 0);
		skin->m_TextAlign = my::Font::AlignCenterMiddle;
		skin->m_Image = image2;
		//skin->m_ScrollBarUpBtnNormalImage = image;
		//skin->m_ScrollBarUpBtnDisabledImage = image2;
		//skin->m_ScrollBarDownBtnNormalImage = image;
		//skin->m_ScrollBarDownBtnDisabledImage = image2;
		//skin->m_ScrollBarThumbBtnNormalImage = image;
		//skin->m_ScrollBarImage = image2;

		return S_OK;
	}

	void OnMouseClick(EventArg* arg)
	{
		static CPoint last_pt(0, 0);
		m_Tex->OnDestroyDevice();
		m_Tex->CreateTextureFromFile(_T("four-holes.png"));
		D3DSURFACE_DESC desc = m_Tex->GetLevelDesc(0);

		MouseEventArg * mouse_arg = dynamic_cast<MouseEventArg *>(arg);
		_ASSERT(mouse_arg);
		Vector2 ptLocal = mouse_arg->pt - mouse_arg->sender->m_Rect.LeftTop();
		D3DLOCKED_RECT lr = m_Tex->LockRect(NULL);
		CPoint pt((int)ptLocal.x, (int)ptLocal.y);
		my::AStar2D<DWORD> searcher(desc.Height, lr.Pitch, (DWORD*)lr.pBits, D3DCOLOR_ARGB(0,0,0,0), last_pt, pt, 32767);
		if (searcher.solve())
		{
			DWORD hover = D3DCOLOR_ARGB(255, 0, 255, 0);
			std::map<CPoint, CPoint>::const_iterator from_iter = searcher.from.begin();
			for (; from_iter != searcher.from.end(); from_iter++)
			{
				searcher.map[from_iter->second.y][from_iter->second.x] = hover;
			}
			DWORD color = D3DCOLOR_ARGB(255, 255, 0, 0);
			searcher.map[pt.y][pt.x] = color;
			from_iter = searcher.from.find(pt);
			for (; from_iter != searcher.from.end(); from_iter = searcher.from.find(from_iter->second))
			{
				searcher.map[from_iter->second.y][from_iter->second.x] = color;
			}
			last_pt = pt;
		}
		m_Tex->UnlockRect();
	}

	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		if (FAILED(hr = DxutApp::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		if (FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		if (FAILED(hr = m_UIRender->OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
		{
			return hr;
		}

		m_d3dfont->OnResetDevice();

		if(FAILED(hr = D3DXCreateSprite(pd3dDevice, &m_sprite)))
		{
			return hr;
		}

		DialogMgr::SetDlgViewport(Vector2(600 * (float)pBackBufferSurfaceDesc->Width / pBackBufferSurfaceDesc->Height, 600), D3DXToRadian(75.0f));

		FontLibrary::m_Scale = Vector2(pBackBufferSurfaceDesc->Height / DialogMgr::GetDlgViewport().y);

		FontLibrary::m_EventScaleChanged(FontLibrary::m_Scale);

		return S_OK;
	}

	virtual void OnLostDevice(void)
	{
		m_d3dfont->OnLostDevice();

		m_sprite.Release();

		m_UIRender->OnLostDevice();

		ResourceMgr::OnLostDevice();

		DxutApp::OnLostDevice();
	}

	virtual void OnDestroyDevice(void)
	{
		m_Tex.reset();

		m_d3dfont.Release();

		RemoveAllDlg();

		m_UIRender->OnDestroyDevice();

		InputMgr::Destroy();

		SoundContext::Shutdown();

		ResourceMgr::OnDestroyDevice();

		DxutApp::OnDestroyDevice();
	}

	virtual void OnFrameTick(double fTime, float fElapsedTime)
	{
		CheckIORequests(0);

		SoundContext::ReleaseIdleBuffer(fElapsedTime);

		InputMgr::Capture(fTime, fElapsedTime);

		if (m_keyboard->IsKeyPress(KC_1))
		{
			my::WavPtr wav(new Wav());
			wav->CreateWavFromFile(_T("../demo2_3/Media/sound/jaguar.wav"));
			SoundContext::Play(wav, false, Vector3(-3,0,0), Vector3(0,0,0), 1.0f, 5.0f);
		}

		if (m_keyboard->IsKeyPress(KC_2))
		{
			m_mp3.Play(my::FileIStream::Open(_T("castle1_34.mp3")), false);
		}

		// Clear the render target and the zbuffer 
		V(m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0));

		// Render the scene
		if (SUCCEEDED(m_d3dDevice->BeginScene()))
		{
			V(m_sprite->Begin(D3DXSPRITE_ALPHABLEND));
			wchar_t buff[256];
			int len = swprintf_s(buff, _countof(buff), L"%.2f", m_fFps);
			V(m_d3dfont->DrawTextW(m_sprite, buff, len, CRect(5, 5, 100, 100), DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f)));
			m_font->DrawString(m_sprite, L"哈哈", my::Rectangle(500, 100, 600, 200), D3DCOLOR_ARGB(255, 255, 90, 30), my::Font::AlignCenterMiddle, D3DCOLOR_ARGB(255, 255, 255, 255), 3.0f);
			V(m_sprite->End());

			m_UIRender->Begin();
			m_UIRender->SetWorld(Matrix4::identity);
			m_UIRender->SetViewProj(DialogMgr::m_ViewProj);
			DialogMgr::Draw(m_UIRender.get(), fTime, fElapsedTime, DialogMgr::GetDlgViewport());
			m_UIRender->PushString(my::Rectangle(500, 300, 600, 400), L"哈哈", D3DCOLOR_ARGB(255, 255, 90, 30), my::Font::AlignCenterMiddle, D3DCOLOR_ARGB(255, 255, 255, 255), 3.0f, m_font.get());
			m_UIRender->End();
			V(m_d3dDevice->EndScene());
		}

		Present(0,0,0,0);
	}

	virtual LRESULT MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		*pbNoFurtherProcessing = ImeEditBox::StaticMsgProc(hWnd, uMsg, wParam, lParam);
		if (*pbNoFurtherProcessing)
		{
			return 0;
		}

		*pbNoFurtherProcessing = DialogMgr::MsgProc(hWnd, uMsg, wParam, lParam);
		if(*pbNoFurtherProcessing)
		{
			return 0;
		}
		return 0;
	}
};

#include "myDxutApp11.h"

LONG WINAPI OnException(_EXCEPTION_POINTERS* ExceptionInfo)
{
	WriteMiniDump(ExceptionInfo, _T("aaa.dmp"));
	return EXCEPTION_EXECUTE_HANDLER;
}

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	SetUnhandledExceptionFilter(OnException);

	return Demo().Run();

	//return DxutApp11().Run();
}

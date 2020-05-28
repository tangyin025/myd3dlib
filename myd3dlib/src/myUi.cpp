#include "myUi.h"
#include "myDxutApp.h"
#include "myResource.h"
#include "myCollision.h"
#include "myUtility.h"
#include "ImeUi.h"
#include "libc.h"
#include <fstream>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace my;

BOOST_CLASS_EXPORT(ControlSkin)

BOOST_CLASS_EXPORT(Control)

BOOST_CLASS_EXPORT(Static)

BOOST_CLASS_EXPORT(ProgressBarSkin)

BOOST_CLASS_EXPORT(ProgressBar)

BOOST_CLASS_EXPORT(ButtonSkin)

BOOST_CLASS_EXPORT(Button)

BOOST_CLASS_EXPORT(EditBoxSkin)

BOOST_CLASS_EXPORT(EditBox)

BOOST_CLASS_EXPORT(ImeEditBox)

BOOST_CLASS_EXPORT(ScrollBarSkin)

BOOST_CLASS_EXPORT(ScrollBar)

BOOST_CLASS_EXPORT(CheckBox)

BOOST_CLASS_EXPORT(ComboBoxSkin)

BOOST_CLASS_EXPORT(ComboBox)

BOOST_CLASS_EXPORT(Dialog)

UIRender::UIRender(void)
	: m_WhiteTex(new my::Texture2D())
{
	UILayerList::iterator layer_iter = m_Layer.begin();
	for (; layer_iter != m_Layer.end(); layer_iter++)
	{
		layer_iter->first = NULL;
	}
}

UIRender::~UIRender(void)
{
	_ASSERT(!m_Device);
	_ASSERT(!m_WhiteTex->m_ptr);
}

HRESULT UIRender::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	m_Device = pd3dDevice;

	m_WhiteTex->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8);
	RECT rc = { 0, 0, 1, 1 };
	D3DLOCKED_RECT SrcLrc = m_WhiteTex->LockRect(&rc, 0, 0);
	D3DCOLOR * DstBits = (D3DCOLOR *)SrcLrc.pBits;
	*DstBits = D3DCOLOR_ARGB(255, 255, 255, 255);
	m_WhiteTex->UnlockRect();

	return S_OK;
}

HRESULT UIRender::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	return S_OK;
}

void UIRender::OnLostDevice(void)
{
}

void UIRender::OnDestroyDevice(void)
{
	m_Device.Release();
}

void UIRender::Begin(void)
{
	V(m_Device->SetVertexShader(NULL));
	V(m_Device->SetPixelShader(NULL));
	V(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(m_Device->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
	V(m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	V(m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
	V(m_Device->SetRenderState(D3DRS_ZENABLE, FALSE));
	V(m_Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	V(m_Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	V(m_Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
	V(m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
}

void UIRender::End(void)
{
	Flush();
}

void UIRender::SetWorld(const Matrix4 & World)
{
	V(m_Device->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&World));
}

void UIRender::SetViewProj(const Matrix4 & ViewProj)
{
	V(m_Device->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&ViewProj));
	V(m_Device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Matrix4::identity));
}

void UIRender::Flush(void)
{
	UILayerList::iterator layer_iter = m_Layer.begin();
	for (; layer_iter != m_Layer.end(); layer_iter++)
	{
		if (layer_iter->first && layer_iter->first->m_ptr && !layer_iter->second.empty())
		{
			V(m_Device->SetTexture(0, layer_iter->first->m_ptr));
			V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
			V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, layer_iter->second.size() / 3, &layer_iter->second[0], sizeof(CUSTOMVERTEX)));
			layer_iter->second.clear();
		}
	}
}

void UIRender::PushVertexSimple(VertexList & vertex_list, unsigned int start, float x, float y, float z, float u, float v, D3DCOLOR color)
{
	CUSTOMVERTEX & vtx = vertex_list[start];
	vtx.x = x;
	vtx.y = y;
	vtx.z = z;
	vtx.u = u;
	vtx.v = v;
	vtx.color = color;
}

void UIRender::PushRectangleSimple(VertexList & vertex_list, unsigned int start, const my::Rectangle & rect, const my::Rectangle & UvRect, D3DCOLOR color)
{
	PushVertexSimple(vertex_list, start++, rect.l, rect.t, 0, UvRect.l, UvRect.t, color);
	PushVertexSimple(vertex_list, start++, rect.r, rect.t, 0, UvRect.r, UvRect.t, color);
	PushVertexSimple(vertex_list, start++, rect.l, rect.b, 0, UvRect.l, UvRect.b, color);
	PushVertexSimple(vertex_list, start++, rect.r, rect.b, 0, UvRect.r, UvRect.b, color);
	PushVertexSimple(vertex_list, start++, rect.l, rect.b, 0, UvRect.l, UvRect.b, color);
	PushVertexSimple(vertex_list, start++, rect.r, rect.t, 0, UvRect.r, UvRect.t, color);
}

void UIRender::PushRectangle(const my::Rectangle & rect, const my::Rectangle & UvRect, D3DCOLOR color, BaseTexture * texture, UILayerType type)
{
	_ASSERT(texture);
	if (m_Layer[type].first != texture)
	{
		Flush();
		m_Layer[type].first = texture;
	}
	VertexList & vertex_list = m_Layer[type].second;
	unsigned int start = vertex_list.size();
	vertex_list.resize(start + 6);
	PushRectangleSimple(vertex_list, start, rect, UvRect, color);
}

void UIRender::PushWindowSimple(VertexList & vertex_list, unsigned int start, const my::Rectangle & rect, DWORD color, const my::Rectangle & WindowRect, const Vector4 & WindowBorder, const CSize & TextureSize)
{
	Rectangle OutUvRect(WindowRect.l / TextureSize.cx,  WindowRect.t / TextureSize.cy, WindowRect.r / TextureSize.cx, WindowRect.b / TextureSize.cy);
	Rectangle InRect(rect.l + WindowBorder.x, rect.t + WindowBorder.y, rect.r - WindowBorder.z, rect.b - WindowBorder.w);
	Rectangle InUvRect((WindowRect.l + WindowBorder.x) / TextureSize.cx, (WindowRect.t + WindowBorder.y) / TextureSize.cy, (WindowRect.r - WindowBorder.z) / TextureSize.cx, (WindowRect.b - WindowBorder.w) / TextureSize.cy);
	PushRectangleSimple(vertex_list, start, Rectangle(rect.l, rect.t, InRect.l, InRect.t), Rectangle(OutUvRect.l, OutUvRect.t, InUvRect.l, InUvRect.t), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(InRect.l, rect.t, InRect.r, InRect.t), Rectangle(InUvRect.l, OutUvRect.t, InUvRect.r, InUvRect.t), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(InRect.r, rect.t, rect.r, InRect.t), Rectangle(InUvRect.r, OutUvRect.t, OutUvRect.r, InUvRect.t), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(rect.l, InRect.t, InRect.l, InRect.b), Rectangle(OutUvRect.l, InUvRect.t, InUvRect.l, InUvRect.b), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(InRect.l, InRect.t, InRect.r, InRect.b), Rectangle(InUvRect.l, InUvRect.t, InUvRect.r, InUvRect.b), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(InRect.r, InRect.t, rect.r, InRect.b), Rectangle(InUvRect.r, InUvRect.t, OutUvRect.r, InUvRect.b), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(rect.l, InRect.b, InRect.l, rect.b), Rectangle(OutUvRect.l, InUvRect.b, InUvRect.l, OutUvRect.b), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(InRect.l, InRect.b, InRect.r, rect.b), Rectangle(InUvRect.l, InUvRect.b, InUvRect.r, OutUvRect.b), color);
	start += 6;
	PushRectangleSimple(vertex_list, start, Rectangle(InRect.r, InRect.b, rect.r, rect.b), Rectangle(InUvRect.r, InUvRect.b, OutUvRect.r, OutUvRect.b), color);
	start += 6;
}

void UIRender::PushWindow(const my::Rectangle & rect, DWORD color, const my::Rectangle & WindowRect, const Vector4 & WindowBorder, const CSize & TextureSize, BaseTexture * texture, UILayerType type)
{
	_ASSERT(texture);
	if (m_Layer[type].first != texture)
	{
		Flush();
		m_Layer[type].first = texture;
	}
	VertexList & vertex_list = m_Layer[type].second;
	unsigned int start = vertex_list.size();
	vertex_list.resize(start + 6 * 9);
	PushWindowSimple(vertex_list, start, rect, color, WindowRect, WindowBorder, TextureSize);
}

template<class Archive>
void ControlImage::save(Archive & ar, const unsigned int version) const
{
	std::string TexturePath(m_Texture->m_Key);
	ar << BOOST_SERIALIZATION_NVP(TexturePath);
	ar << BOOST_SERIALIZATION_NVP(m_Rect);
	ar << BOOST_SERIALIZATION_NVP(m_Border);
}

template<class Archive>
void ControlImage::load(Archive & ar, const unsigned int version)
{
	std::string TexturePath;
	ar >> BOOST_SERIALIZATION_NVP(TexturePath);
	if (!TexturePath.empty())
	{
		m_Texture = my::ResourceMgr::getSingleton().LoadTexture(TexturePath.c_str());
	}
	ar >> BOOST_SERIALIZATION_NVP(m_Rect);
	ar >> BOOST_SERIALIZATION_NVP(m_Border);
}

template
void ControlImage::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void ControlImage::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void ControlImage::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void ControlImage::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void ControlImage::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void ControlImage::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void ControlImage::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void ControlImage::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

ControlSkin::~ControlSkin(void)
{
}

template<class Archive>
void ControlSkin::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Color);
	ar << BOOST_SERIALIZATION_NVP(m_Image);
	std::vector<std::string> FontSeq;
	boost::algorithm::split(FontSeq, m_Font->m_Key, boost::is_any_of(" "), boost::algorithm::token_compress_off);
	std::string FontPath = FontSeq.size() > 1 ? FontSeq[0] : std::string();
	int FontHeight = FontSeq.size() > 1 ? boost::lexical_cast<int>(FontSeq[1]) : 13;
	ar << BOOST_SERIALIZATION_NVP(FontPath);
	ar << BOOST_SERIALIZATION_NVP(FontHeight);
	ar << BOOST_SERIALIZATION_NVP(m_TextColor);
	ar << BOOST_SERIALIZATION_NVP(m_TextAlign);
	ar << BOOST_SERIALIZATION_NVP(m_VisibleShowSound);
	ar << BOOST_SERIALIZATION_NVP(m_VisibleHideSound);
	ar << BOOST_SERIALIZATION_NVP(m_MouseEnterSound);
	ar << BOOST_SERIALIZATION_NVP(m_MouseLeaveSound);
	ar << BOOST_SERIALIZATION_NVP(m_MouseClickSound);
}

template<class Archive>
void ControlSkin::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Color);
	ar >> BOOST_SERIALIZATION_NVP(m_Image);
	std::string FontPath;
	ar >> BOOST_SERIALIZATION_NVP(FontPath);
	int FontHeight;
	ar >> BOOST_SERIALIZATION_NVP(FontHeight);
	if (!FontPath.empty())
	{
		m_Font = my::ResourceMgr::getSingleton().LoadFont(FontPath.c_str(), FontHeight);
	}
	ar >> BOOST_SERIALIZATION_NVP(m_TextColor);
	ar >> BOOST_SERIALIZATION_NVP(m_TextAlign);
	ar >> BOOST_SERIALIZATION_NVP(m_VisibleShowSound);
	ar >> BOOST_SERIALIZATION_NVP(m_VisibleHideSound);
	ar >> BOOST_SERIALIZATION_NVP(m_MouseEnterSound);
	ar >> BOOST_SERIALIZATION_NVP(m_MouseLeaveSound);
	ar >> BOOST_SERIALIZATION_NVP(m_MouseClickSound);
}

template
void ControlSkin::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void ControlSkin::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void ControlSkin::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void ControlSkin::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void ControlSkin::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void ControlSkin::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void ControlSkin::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void ControlSkin::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void ControlSkin::DrawImage(UIRender * ui_render, ControlImagePtr Image, const my::Rectangle & rect, DWORD color)
{
	if(Image && Image->m_Texture)
	{
		D3DSURFACE_DESC desc = Image->m_Texture->GetLevelDesc();
		if (Image->m_Border.x == 0 && Image->m_Border.y == 0 && Image->m_Border.z == 0 && Image->m_Border.w == 0)
		{
			Rectangle UvRect(Image->m_Rect.l / desc.Width,  Image->m_Rect.t / desc.Height, Image->m_Rect.r / desc.Width, Image->m_Rect.b / desc.Height);
			ui_render->PushRectangle(rect, UvRect, color, Image->m_Texture.get(), UIRender::UILayerTexture);
		}
		else
		{
			ui_render->PushWindow(rect, color, Image->m_Rect, Image->m_Border, CSize(desc.Width, desc.Height), Image->m_Texture.get(), UIRender::UILayerTexture);
		}
	}
	else
	{
		ui_render->PushRectangle(rect, Rectangle(0, 0, 1, 1), color, ui_render->m_WhiteTex.get(), UIRender::UILayerTexture);
	}
}

void ControlSkin::DrawString(UIRender * ui_render, LPCWSTR pString, const my::Rectangle & rect, DWORD TextColor, Font::Align TextAlign)
{
	if(m_Font)
	{
		m_Font->PushString(ui_render, pString, rect, TextColor, TextAlign);
	}
}

Control * Control::s_FocusControl = NULL;

Control * Control::s_CaptureControl = NULL;

Control * Control::s_MouseOverControl = NULL;

Control::~Control(void)
{
	// ! must clear global reference
	if (this == s_FocusControl)
	{
		SetFocusControl(NULL);
	}

	if (this == s_CaptureControl)
	{
		SetCaptureControl(NULL);
	}

	if (this == s_MouseOverControl)
	{
		SetMouseOverControl(NULL, Vector2(FLT_MAX, FLT_MAX));
	}

	// ! must detach parent relationship
	ClearAllControl();

	if (m_Parent)
	{
		_ASSERT(false); //m_Parent->RemoveControl(shared_from_this());
	}
}

template<class Archive>
void Control::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Name);
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
	ar << BOOST_SERIALIZATION_NVP(m_bEnabled);
	ar << BOOST_SERIALIZATION_NVP(m_bVisible);
	ar << BOOST_SERIALIZATION_NVP(m_nHotkey);
	ar << BOOST_SERIALIZATION_NVP(m_Location);
	ar << BOOST_SERIALIZATION_NVP(m_Size);
	ar << BOOST_SERIALIZATION_NVP(m_Skin);
}

template<class Archive>
void Control::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Name);
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	ar >> BOOST_SERIALIZATION_NVP(m_bEnabled);
	ar >> BOOST_SERIALIZATION_NVP(m_bVisible);
	ar >> BOOST_SERIALIZATION_NVP(m_nHotkey);
	ar >> BOOST_SERIALIZATION_NVP(m_Location);
	ar >> BOOST_SERIALIZATION_NVP(m_Size);
	ar >> BOOST_SERIALIZATION_NVP(m_Skin);

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_Parent = this;
	}
}

template
void Control::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void Control::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void Control::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void Control::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void Control::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void Control::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void Control::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void Control::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void Control::SetFocusControl(Control * control)
{
	if (s_FocusControl != control)
	{
		if (s_FocusControl)
		{
			s_FocusControl->OnFocusOut();
		}

		Control * old_control = s_FocusControl;

		s_FocusControl = control;

		if (s_FocusControl)
		{
			control->OnFocusIn();

			if (!old_control)
			{
				D3DContext::getSingleton().OnControlFocus(true);
			}
		}
		else
		{
			if (old_control)
			{
				D3DContext::getSingleton().OnControlFocus(false);
			}
		}
	}
}

void Control::SetCaptureControl(Control * control)
{
	if (s_CaptureControl != control)
	{
		s_CaptureControl = control;
	}
}

void Control::SetMouseOverControl(Control * control, const Vector2 & pt)
{
	if (s_MouseOverControl != control)
	{
		if (s_MouseOverControl)
		{
			s_MouseOverControl->OnMouseLeave(pt);
		}

		s_MouseOverControl = control;

		if (control)
		{
			control->OnMouseEnter(pt);
		}
	}
}

void Control::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if (m_Skin)
		{
			m_Skin->DrawImage(ui_render, m_Skin->m_Image, Rect, m_Skin->m_Color);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

bool Control::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Control::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Control::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Control::CanHaveFocus(void)
{
	return false;
}

void Control::OnFocusIn(void)
{
	m_bHasFocus = true;
}

void Control::OnFocusOut(void)
{
	m_bHasFocus = false;
}

void Control::OnMouseEnter(const Vector2 & pt)
{
	if(m_bEnabled && m_bVisible)
	{
		m_bMouseOver = true;

		if (m_Skin && !m_Skin->m_MouseEnterSound.empty())
		{
			D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseEnterSound.c_str());
		}

		if (m_EventMouseEnter)
		{
			MouseEventArg arg(this, pt);
			m_EventMouseEnter(&arg);
		}
	}
}

void Control::OnMouseLeave(const Vector2 & pt)
{
	if(m_bEnabled && m_bVisible)
	{
		m_bMouseOver = false;

		if (m_EventMouseLeave)
		{
			MouseEventArg arg(this, pt);
			m_EventMouseLeave(&arg);
		}

		if (m_Skin && !m_Skin->m_MouseLeaveSound.empty())
		{
			D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseLeaveSound.c_str());
		}
	}
}

void Control::OnHotkey(void)
{
}

bool Control::HitTest(const Vector2 & pt)
{
	return Rectangle::LeftTop(Vector2(0,0), m_Size).PtInRect(ScreenToLocal(pt));
}

void Control::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
}

bool Control::GetEnabled(void)
{
	return m_bEnabled;
}

void Control::SetVisible(bool bVisible)
{
	if (m_bVisible != bVisible)
	{
		m_bVisible = bVisible;

		if (m_bVisible)
		{
			if (m_Skin && !m_Skin->m_VisibleShowSound.empty())
			{
				D3DContext::getSingleton().OnControlSound(m_Skin->m_VisibleShowSound.c_str());
			}
		}
		else
		{
			if (m_Skin && !m_Skin->m_VisibleHideSound.empty())
			{
				D3DContext::getSingleton().OnControlSound(m_Skin->m_VisibleHideSound.c_str());
			}
		}

		if (m_EventVisibleChanged)
		{
			VisibleEventArg arg(this, bVisible);
			m_EventVisibleChanged(&arg);
		}

		if (!m_bVisible)
		{
			if (s_FocusControl && ContainsControl(s_FocusControl))
			{
				SetFocusControl(NULL);
			}
		}
	}
}

bool Control::GetVisible(void)
{
	return m_bVisible;
}

bool Control::RayToWorld(const Ray & ray, Vector2 & ptWorld)
{
	if (m_Parent)
	{
		return m_Parent->RayToWorld(ray, ptWorld);
	}
	return false;
}

void Control::InsertControl(ControlPtr control)
{
	_ASSERT(!control->m_Parent);

	m_Childs.push_back(control);

	control->m_Parent = this;
}

void Control::RemoveControl(ControlPtr control)
{
	ControlPtrList::iterator ctrl_iter = std::find(m_Childs.begin(), m_Childs.end(), control);
	if(ctrl_iter != m_Childs.end())
	{
		_ASSERT((*ctrl_iter)->m_Parent == this);

		(*ctrl_iter)->m_Parent = NULL;

		m_Childs.erase(ctrl_iter);
	}
	else
	{
		_ASSERT(false);
	}
}

void Control::ClearAllControl(void)
{
	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter = m_Childs.begin())
	{
		RemoveControl(*ctrl_iter);
	}
}

bool Control::ContainsControl(Control * control)
{
	if (this == control)
	{
		return true;
	}

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		if ((*ctrl_iter)->ContainsControl(control))
		{
			return true;
		}
	}
	return false;
}

Control * Control::FindControl(const std::string & name)
{
	if (m_Name == name)
	{
		return this;
	}

	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		Control * ret = (*ctrl_iter)->FindControl(name);
		if (ret)
		{
			return ret;
		}
	}
	return NULL;
}

Control * Control::GetChildAtPoint(const Vector2 & pt)
{
	ControlPtrList::const_iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		Control * ctrl = (*ctrl_iter)->GetChildAtPoint(pt);
		if (ctrl)
		{
			return ctrl;
		}
	}

	if (HitTest(pt))
	{
		return this;
	}
	return NULL;
}

Vector2 Control::LocalToScreen(const Vector2 & pt) const
{
	if (m_Parent)
	{
		return m_Parent->LocalToScreen(m_Location + pt);
	}
	return m_Location + pt;
}

Vector2 Control::ScreenToLocal(const Vector2 & pt) const
{
	if (m_Parent)
	{
		return m_Parent->ScreenToLocal(pt) - m_Location;
	}
	return pt - m_Location;
}

bool Control::SetFocusRecursive(void)
{
	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		if ((*ctrl_iter)->CanHaveFocus() && (*ctrl_iter)->SetFocusRecursive())
		{
			return true;
		}
	}

	if (CanHaveFocus())
	{
		SetFocusControl(this);
		return true;
	}
	return false;
}

void Control::SetHotkey(UINT nHotkey)
{
	m_nHotkey = nHotkey;
}

UINT Control::GetHotkey(void)
{
	return m_nHotkey;
}

void Static::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(m_Skin)
		{
			m_Skin->DrawString(ui_render, m_Text.c_str(), Rect, m_Skin->m_TextColor, m_Skin->m_TextAlign);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

void ProgressBar::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if (m_Skin)
		{
			ProgressBarSkinPtr Skin = boost::dynamic_pointer_cast<ProgressBarSkin>(m_Skin);
			_ASSERT(Skin);

			Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Skin->m_Color);

			m_BlendProgress = Lerp(m_BlendProgress, m_Progress, 1.0f - powf(0.8f, 30 * fElapsedTime));
			Rect.r = Lerp(Rect.l, Rect.r, Max(0.0f, Min(1.0f, m_BlendProgress)));
			Skin->DrawImage(ui_render, Skin->m_ForegroundImage, Rect, m_Skin->m_Color);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

void Button::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(m_Skin)
		{
			ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin>(m_Skin);
			_ASSERT(Skin);

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, Rect, m_Skin->m_Color);
			}
			else
			{
				if(m_bPressed)
				{
					Rect = Rect.offset(Skin->m_PressedOffset);
					Skin->DrawImage(ui_render, Skin->m_PressedImage, Rect, m_Skin->m_Color);
				}
				else
				{
					if(m_bMouseOver /*|| m_bHasFocus*/)
					{
						Rect = Rect.offset(-Skin->m_PressedOffset);
						Skin->DrawImage(ui_render, Skin->m_MouseOverImage, Rect, m_Skin->m_Color);
					}
					else
					{
						Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Skin->m_Color);
					}
				}
			}

			Skin->DrawString(ui_render, m_Text.c_str(), Rect, Skin->m_TextColor, m_Skin->m_TextAlign);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

bool Button::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_RETURN:
				m_bPressed = true;
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_KEYUP:
			switch(wParam)
			{
			case VK_RETURN:
				if(m_bPressed)
				{
					m_bPressed = false;
					SetCaptureControl(NULL);

					if (m_Skin && !m_Skin->m_MouseClickSound.empty())
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
					}

					if (m_EventMouseClick)
					{
						MouseEventArg arg(this, Vector2(0, 0));
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;
		}
	}
	return false;
}

bool Button::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (HitTest(pt))
			{
				m_bPressed = true;
				SetFocusControl(this);
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if (m_bPressed)
			{
				SetCaptureControl(NULL);
				m_bPressed = false;

				if (HitTest(pt))
				{
					if (m_Skin && !m_Skin->m_MouseClickSound.empty())
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
					}

					if (m_EventMouseClick)
					{
						MouseEventArg arg(this, pt);
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;
		}
	}
	return false;
}

bool Button::CanHaveFocus(void)
{
	return m_bVisible && m_bEnabled;
}

void Button::OnHotkey(void)
{
	if(m_bEnabled && m_bVisible)
	{
		if (m_Skin && !m_Skin->m_MouseClickSound.empty())
		{
			D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
		}

		if(m_EventMouseClick)
		{
			MouseEventArg arg(this, Vector2(0, 0));
			m_EventMouseClick(&arg);
		}
	}
}

bool Button::HitTest(const Vector2 & pt)
{
	return Control::HitTest(pt);
}

void EditBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		DWORD dwAbsoluteTime = timeGetTime();
		if(dwAbsoluteTime - m_dwLastBlink >= m_dwBlink )
		{
			m_bCaretOn = !m_bCaretOn;
			m_dwLastBlink = dwAbsoluteTime;
		}

		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(m_Skin)
		{
			EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
			_ASSERT(Skin);

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, Rect, m_Skin->m_Color);
			}
			else if(m_bHasFocus)
			{
				Skin->DrawImage(ui_render, Skin->m_FocusedImage, Rect, m_Skin->m_Color);
			}
			else
			{
				Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Skin->m_Color);
			}

			if(Skin->m_Font)
			{
				Rectangle TextRect = Rect.shrink(m_Border);

				float x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
				float caret_x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
				if(m_nSelStart != m_nCaret)
				{
					float sel_start_x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nSelStart);
					float sel_left_x = __min(caret_x, sel_start_x) - x1st;
					float sel_right_x = __max(caret_x, sel_start_x) - x1st;

					Rectangle SelRect(
						Max(TextRect.l, TextRect.l + sel_left_x),
						TextRect.t,
						Min(TextRect.r, TextRect.l + sel_right_x),
						TextRect.b);

					Skin->DrawImage(ui_render, Skin->m_CaretImage, SelRect, Skin->m_SelBkColor);
				}

				Skin->m_Font->PushString(ui_render, m_Text.c_str() + m_nFirstVisible, TextRect, Skin->m_TextColor, Font::AlignLeftMiddle);

				if(m_bHasFocus && m_bCaretOn && !ImeEditBox::s_bHideCaret)
				{
					Rectangle CaretRect(
						TextRect.l + caret_x - x1st - 1,
						TextRect.t,
						TextRect.l + caret_x - x1st + 1,
						TextRect.b);

					if(!m_bInsertMode)
					{
						float charWidth;
						if(m_nCaret < (int)m_Text.length())
						{
							const Font::CharacterInfo & info = Skin->m_Font->GetCharacterInfo(m_Text[m_nCaret]);
							charWidth = info.horiAdvance;
						}
						else
						{
							const Font::CharacterInfo & info = Skin->m_Font->GetCharacterInfo(L'_');
							charWidth = info.horiAdvance;
						}
						CaretRect.r = TextRect.l + caret_x - x1st + charWidth;
					}

					Skin->DrawImage(ui_render, Skin->m_CaretImage, CaretRect, Skin->m_CaretColor);
				}
			}
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

bool EditBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_CHAR:
			{
				switch((WCHAR)wParam)
				{
				case VK_BACK:
					if(m_nCaret != m_nSelStart)
					{
						DeleteSelectionText();
						if(m_EventChange)
						{
							ControlEventArg arg(this);
							m_EventChange(&arg);
						}
					}
					else if(m_nCaret > 0)
					{
						PlaceCaret(m_nCaret - 1);
						m_nSelStart = m_nCaret;
						m_Text.erase(m_nCaret, 1);
						if(m_EventChange)
						{
							ControlEventArg arg(this);
							m_EventChange(&arg);
						}
					}
					ResetCaretBlink();
					break;

                case 24:        // Ctrl-X Cut
                case VK_CANCEL: // Ctrl-C Copy
					CopyToClipboard();
					if((WCHAR)wParam == 24)
					{
						DeleteSelectionText();
						if(m_EventChange)
						{
							ControlEventArg arg(this);
							m_EventChange(&arg);
						}
					}
					break;

				case 22:		// Ctrl-V Paste
					PasteFromClipboard();
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
					break;

				case 1:
					//if(m_nSelStart == m_nCaret)
					{
						m_nSelStart = 0;
						PlaceCaret(m_Text.length());
					}
					break;

				case VK_RETURN:
					if(m_EventEnter)
					{
						ControlEventArg arg(this);
						m_EventEnter(&arg);
					}
					break;

				// Junk characters we don't want in the string
				case 26:  // Ctrl Z
				case 2:   // Ctrl B
				case 14:  // Ctrl N
				case 19:  // Ctrl S
				case 4:   // Ctrl D
				case 6:   // Ctrl F
				case 7:   // Ctrl G
				case 10:  // Ctrl J
				case 11:  // Ctrl K
				case 12:  // Ctrl L
				case 17:  // Ctrl Q
				case 23:  // Ctrl W
				case 5:   // Ctrl E
				case 18:  // Ctrl R
				case 20:  // Ctrl T
				case 25:  // Ctrl Y
				case 21:  // Ctrl U
				case 9:   // Ctrl I
				case 15:  // Ctrl O
				case 16:  // Ctrl P
				case 27:  // Ctrl [
				case 29:  // Ctrl ]
				case 28:  // Ctrl \ 
					break;

				default:
					if(m_nCaret != m_nSelStart)
						DeleteSelectionText();

					if(!m_bInsertMode && m_nCaret < (int)m_Text.length())
					{
						m_Text[m_nCaret] = (WCHAR)wParam;
						PlaceCaret(m_nCaret + 1);
						m_nSelStart = m_nCaret;
					}
					else
					{
						m_Text.insert(m_nCaret, 1, (WCHAR)wParam);
						PlaceCaret(m_nCaret + 1);
						m_nSelStart = m_nCaret;
					}
					ResetCaretBlink();
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool EditBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_TAB:
				break;

			case VK_HOME:
				PlaceCaret(0);
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_END:
				PlaceCaret(m_Text.length());
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_INSERT:
				if(GetKeyState(VK_CONTROL) < 0)
				{
					CopyToClipboard();
				}
				else if(GetKeyState(VK_SHIFT) < 0)
				{
					PasteFromClipboard();
				}
				else
				{
					m_bInsertMode = !m_bInsertMode;
				}
				ResetCaretBlink();
				break;

			case VK_DELETE:
				if(m_nCaret != m_nSelStart)
				{
					DeleteSelectionText();
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
				}
				else
				{
					m_Text.erase(m_nCaret, 1);
					if(m_EventChange)
					{
						ControlEventArg arg(this);
						m_EventChange(&arg);
					}
				}
				ResetCaretBlink();
				return true;

			case VK_LEFT:
				if(GetKeyState(VK_CONTROL) < 0)
				{
					PlaceCaret(GetPriorItemPos(m_nCaret));
				}
				else if(m_nCaret > 0)
				{
					if(m_nCaret != m_nSelStart && GetKeyState(VK_SHIFT) >= 0)
						PlaceCaret(__min(m_nCaret, m_nSelStart));
					else
						PlaceCaret(m_nCaret - 1);
				}
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_RIGHT:
				if(GetKeyState(VK_CONTROL) < 0)
				{
					PlaceCaret(GetNextItemPos(m_nCaret));
				}
				else if(m_nCaret < (int)m_Text.length())
				{
					PlaceCaret(m_nCaret + 1);
				}
				if(GetKeyState(VK_SHIFT) >= 0)
				{
					m_nSelStart = m_nCaret;
				}
				ResetCaretBlink();
				return true;

			case VK_UP:
			case VK_DOWN:
				ResetCaretBlink();
				return true;

			default:
				return wParam != VK_ESCAPE;
			}
			break;
		}
	}
	return false;
}

bool EditBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(HitTest(pt))
			{
				m_bMouseDrag = true;
				SetFocusControl(this);
				SetCaptureControl(this);

				if(m_Skin && m_Skin->m_Font)
				{
					Vector2 ptLocal = ScreenToLocal(pt);
					float x1st = m_Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
					float x = ptLocal.x - m_Border.x + x1st;
					int nCP = m_Skin->m_Font->XtoCP(m_Text.c_str(), x);
					if(nCP < (int)m_Text.length())
					{
						float xLeft = m_Skin->m_Font->CPtoX(m_Text.c_str(), nCP);
						const Font::CharacterInfo & info = m_Skin->m_Font->GetCharacterInfo(m_Text[nCP]);
						if(x > xLeft + info.horiAdvance * 0.5f)
						{
							nCP += 1;
						}
					}

					if(uMsg == WM_LBUTTONDBLCLK)
					{
						m_nSelStart = GetPriorItemPos(nCP);

						PlaceCaret(GetNextItemPos(nCP));
					}
					else
					{
						PlaceCaret(nCP);

						if(GetKeyState(VK_SHIFT) >= 0)
						{
							m_nSelStart = m_nCaret;
						}
					}

					ResetCaretBlink();
				}
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if(m_bMouseDrag)
			{
				SetCaptureControl(NULL);
				m_bMouseDrag = false;
			}
			break;

		case WM_MOUSEMOVE:
			if(m_bMouseDrag)
			{
				if(m_Skin && m_Skin->m_Font)
				{
					Vector2 ptLocal = ScreenToLocal(pt);
					float x1st = m_Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
					float x = ptLocal.x - m_Border.x + x1st;
					int nCP = m_Skin->m_Font->XtoCP(m_Text.c_str(), x);
					if(nCP < (int)m_Text.length())
					{
						float xLeft = m_Skin->m_Font->CPtoX(m_Text.c_str(), nCP);
						const Font::CharacterInfo & info = m_Skin->m_Font->GetCharacterInfo(m_Text[nCP]);
						if(x > xLeft + info.horiAdvance * 0.5f)
						{
							nCP += 1;
						}
					}

					PlaceCaret(nCP);
				}
			}
			break;
		}
	}
	return false;
}

bool EditBox::CanHaveFocus(void)
{
	return m_bVisible && m_bEnabled;
}

void EditBox::OnFocusIn(void)
{
	Control::OnFocusIn();

	ResetCaretBlink();
}

bool EditBox::HitTest(const Vector2 & pt)
{
	return Control::HitTest(pt);
}

void EditBox::SetText(const std::wstring & Text)
{
	m_Text = Text;
	PlaceCaret(Text.length());
	m_nSelStart = m_nCaret;
}

const std::wstring & EditBox::GetText(void) const
{
	return m_Text;
}

void EditBox::PlaceCaret(int nCP)
{
	m_nCaret = nCP;

	if(m_Skin && m_Skin->m_Font)
	{
		float x1st = m_Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
		float x = m_Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
		float x2;
		if(m_nCaret < (int)m_Text.length())
		{
			const Font::CharacterInfo & info = m_Skin->m_Font->GetCharacterInfo(m_Text[m_nCaret]);
			x2 = x + info.horiAdvance;
		}
		else
		{
			x2 = x;
		}

		if(x <= x1st) // ! '=' to rewrite 1st visible, if it was large than caret
		{
			m_nFirstVisible = m_nCaret;
		}
		else
		{
			float xNewLeft = x2 - (m_Size.x - m_Border.x - m_Border.z);
			if(xNewLeft > x1st)
			{
				int nCPNew1st = m_Skin->m_Font->XtoCP(m_Text.c_str(), xNewLeft);
				float xNew1st = m_Skin->m_Font->CPtoX(m_Text.c_str(), nCPNew1st);
				if(xNew1st < xNewLeft)
				{
					nCPNew1st++;
				}
				m_nFirstVisible = nCPNew1st;
			}
		}
	}
}

void EditBox::ResetCaretBlink(void)
{
	m_bCaretOn = true;
	m_dwLastBlink = timeGetTime();
}

void EditBox::DeleteSelectionText(void)
{
	int nFirst = __min(m_nCaret, m_nSelStart);
	int nLast = __max(m_nCaret, m_nSelStart);
	PlaceCaret(nFirst);
	m_nSelStart = m_nCaret;
	m_Text.erase(nFirst, nLast - nFirst);
}

void EditBox::CopyToClipboard(void)
{
	if(m_nCaret != m_nSelStart && OpenClipboard(NULL))
	{
		EmptyClipboard();

        HGLOBAL hBlock = GlobalAlloc( GMEM_MOVEABLE, sizeof( WCHAR ) * ( m_Text.length() + 1 ) );
        if( hBlock )
        {
            WCHAR* pwszText = ( WCHAR* )GlobalLock( hBlock );
            if( pwszText )
            {
                int nFirst = __min( m_nCaret, m_nSelStart );
                int nLast = __max( m_nCaret, m_nSelStart );
                if( nLast - nFirst > 0 )
                    CopyMemory( pwszText, m_Text.c_str() + nFirst, ( nLast - nFirst ) * sizeof( WCHAR ) );
                pwszText[nLast - nFirst] = L'\0';  // Terminate it
                GlobalUnlock( hBlock );
            }
            SetClipboardData( CF_UNICODETEXT, hBlock );
        }
        CloseClipboard();
        // We must not free the object until CloseClipboard is called.
        if( hBlock )
            GlobalFree( hBlock );
	}
}

void EditBox::PasteFromClipboard(void)
{
	DeleteSelectionText();

    if( OpenClipboard( NULL ) )
    {
        HANDLE handle = GetClipboardData( CF_UNICODETEXT );
        if( handle )
        {
            // Convert the ANSI string to Unicode, then
            // insert to our buffer.
            WCHAR* pwszText = ( WCHAR* )GlobalLock( handle );
            if( pwszText )
            {
                // Copy all characters up to null.
				m_Text.insert(m_nCaret, pwszText);
				PlaceCaret( m_nCaret + lstrlenW( pwszText ) );
                m_nSelStart = m_nCaret;
                GlobalUnlock( handle );
            }
        }
        CloseClipboard();
    }
}

int EditBox::GetPriorItemPos(int nCP)
{
	int i = Max(Min(nCP, (int)m_Text.length()) - 1, 0);
	int state = 0;
	for(; i >= 0; i--)
	{
		switch(state)
		{
		case 0:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else if(L' ' == m_Text[i])
				state = 0;
			else
				state = 2;
			break;

		case 1:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else
				return i + 1;
			break;

		case 2:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				return i + 1;
			else
				state = 2;
			break;
		}
	}
	return 0;
}

int EditBox::GetNextItemPos(int nCP)
{
	int i = Max(Min(nCP, (int)m_Text.length() - 1), 0);
	int state = 0;
	for(; i < (int)m_Text.length(); i++)
	{
		switch(state)
		{
		case 0:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else
				state = 2;
			break;

		case 1:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				state = 1;
			else if(L' ' == m_Text[i])
				state = 3;
			else
				return i;
			break;

		case 2:
			if(IsCharAlphaW(m_Text[i]) || IsCharAlphaNumericW(m_Text[i]))
				return i;
			else
				state = 2;
			break;

		case 3:
			if(L' ' == m_Text[i])
				state = 3;
			else
				return i;
		}
	}
	return m_Text.length();
}

bool ImeEditBox::s_bHideCaret = false;

std::wstring ImeEditBox::s_CompString;

void ImeEditBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		EditBox::Draw(ui_render, fElapsedTime, Offset);

	    ImeUi_RenderUI();

		if(m_bHasFocus)
		{
			RenderIndicator(ui_render, fElapsedTime, Offset);

			RenderComposition(ui_render, fElapsedTime, Offset);

			if(ImeUi_IsShowCandListWindow())
				RenderCandidateWindow(ui_render, fElapsedTime, Offset);
		}
	}
}

bool ImeEditBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_DESTROY:
			ImeUi_Uninitialize();
			break;
		}

		if(!ImeUi_IsEnabled())
			return EditBox::MsgProc(hWnd, uMsg, wParam, lParam);

		bool trapped = false;
		ImeUi_ProcessMessage(hWnd, uMsg, wParam, lParam, &trapped);
		if(!trapped)
			trapped = EditBox::MsgProc(hWnd, uMsg, wParam, lParam); // ! Toggle trapped if processed this msg

		return trapped;
	}
	return false;
}

bool ImeEditBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (EditBox::HandleMouse(uMsg, pt, wParam, lParam))
	{
		return true;
	}
	return false;
}

void ImeEditBox::OnFocusIn(void)
{
	ImeUi_EnableIme(true);

	EditBox::OnFocusIn();
}

void ImeEditBox::OnFocusOut(void)
{
	ImeUi_FinalizeString();
	ImeUi_EnableIme(false);

	EditBox::OnFocusOut();
}

void ImeEditBox::Initialize(HWND hWnd)
{
    ImeUiCallback_DrawRect = NULL;
    ImeUiCallback_Malloc = malloc;
    ImeUiCallback_Free = free;
    ImeUiCallback_DrawFans = NULL;

    ImeUi_Initialize(hWnd);
    
    ImeUi_EnableIme(true);
}

void ImeEditBox::Uninitialize(void)
{
    ImeUi_EnableIme(false);

    ImeUi_Uninitialize();
}

bool ImeEditBox::StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!ImeUi_IsEnabled())
		return false;

	switch(uMsg)
	{
	case WM_INPUTLANGCHANGE:
		return true;

	case WM_IME_SETCONTEXT:
		lParam = 0;
		return false;

	case WM_IME_STARTCOMPOSITION:
		ResetCompositionString();
		s_bHideCaret = true;
		return true;

	case WM_IME_ENDCOMPOSITION:
		s_bHideCaret = false;
		return false;

	case WM_IME_COMPOSITION:
		return false;
	}
	return false;
}

void ImeEditBox::ResetCompositionString(void)
{
	s_CompString.clear();
}

void ImeEditBox::EnableImeSystem(bool bEnable)
{
	ImeUi_EnableIme(bEnable);
}

void ImeEditBox::RenderIndicator(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
}

void ImeEditBox::RenderComposition(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_Skin && m_Skin->m_Font)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
		_ASSERT(Skin);

		s_CompString = ts2ws(ImeUi_GetCompositionString());

		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		Rectangle TextRect = Rect.shrink(m_Border);

		float x, x1st;
		x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
		x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
		Vector2 extent = Skin->m_Font->CalculateStringExtent(s_CompString.c_str());

		Rectangle rc(TextRect.l + x - x1st, TextRect.t, TextRect.l + x - x1st + extent.x, TextRect.b);
		if(rc.r > TextRect.r)
			rc.offsetSelf(TextRect.l - rc.l, TextRect.Height());

		Skin->DrawImage(ui_render, Skin->m_CaretImage, rc, m_CompWinColor);

		Skin->m_Font->PushString(ui_render, s_CompString.c_str(), rc, Skin->m_TextColor, Font::AlignLeftTop);

		float caret_x = Skin->m_Font->CPtoX(s_CompString.c_str(), ImeUi_GetImeCursorChars());
		if(m_bCaretOn)
		{
			Rectangle CaretRect(rc.l + caret_x - 1, rc.t, rc.l + caret_x + 1, rc.b);

			Skin->DrawImage(ui_render, Skin->m_CaretImage, CaretRect, Skin->m_CaretColor);
		}
	}
}

void ImeEditBox::RenderCandidateWindow(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_Skin && m_Skin->m_Font)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
		_ASSERT(Skin);

		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		Rectangle TextRect = Rect.shrink(m_Border);

		float x, x1st, comp_x;
		x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
		x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
		Vector2 extent = Skin->m_Font->CalculateStringExtent(s_CompString.c_str());

		Rectangle CompRect(TextRect.l + x - x1st, TextRect.t, TextRect.l + x - x1st + extent.x, TextRect.b);
		if(CompRect.r > TextRect.r)
			CompRect.offsetSelf(TextRect.l - CompRect.l, TextRect.Height());

		comp_x = Skin->m_Font->CPtoX(s_CompString.c_str(), ImeUi_GetImeCursorChars());

		float WidthRequired = 0;
		float HeightRequired = 0;
		float SingleLineHeight = 0;

		std::wstring horizontalText;
		for(UINT i = 0; i < MAX_CANDLIST && *ImeUi_GetCandidate(i) != L'\0'; i++)
		{
			horizontalText += ts2ws(ImeUi_GetCandidate(i));
		}
		extent = Skin->m_Font->CalculateStringExtent(horizontalText.c_str());

		Rectangle CandRect(Rectangle::LeftTop(CompRect.l + comp_x, CompRect.b, extent.x, (float)Skin->m_Font->m_LineHeight));

		Skin->DrawImage(ui_render, Skin->m_CaretImage, CandRect, m_CandidateWinColor);

		Skin->m_Font->PushString(ui_render, horizontalText.c_str(), CandRect, Skin->m_TextColor, Font::AlignLeftTop);
	}
}

void ScrollBar::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
    // Check if the arrow button has been held for a while.
    // If so, update the thumb position to simulate repeated
    // scroll.
	if(m_Arrow != CLEAR)
	{
		DWORD dwAbsoluteTime = timeGetTime();
		switch(m_Arrow)
		{
		case CLICKED_UP:
			if(330 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll(-1);
				m_Arrow = HELD_UP;
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;

		case HELD_UP:
			if(50 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll(-1);
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;

		case CLICKED_DOWN:
			if(330 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll( 1);
				m_Arrow = HELD_DOWN;
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;

		case HELD_DOWN:
			if(50 < dwAbsoluteTime - m_dwArrowTS)
			{
				Scroll( 1);
				m_dwArrowTS = dwAbsoluteTime;
			}
			break;
		}
	}

	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(m_Skin)
		{
			ScrollBarSkinPtr Skin = boost::dynamic_pointer_cast<ScrollBarSkin>(m_Skin);
			_ASSERT(Skin);

			Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Skin->m_Color);

			Rectangle UpButtonRect(Rectangle::LeftTop(Rect.l, Rect.t, m_Size.x, m_UpDownButtonHeight));

			Rectangle DownButtonRect(Rectangle::RightBottom(Rect.r, Rect.b, m_Size.x, m_UpDownButtonHeight));

			if(m_bEnabled && m_nEnd - m_nStart > m_nPageSize)
			{
				Skin->DrawImage(ui_render, Skin->m_UpBtnNormalImage, UpButtonRect, m_Skin->m_Color);

				Skin->DrawImage(ui_render, Skin->m_DownBtnNormalImage, DownButtonRect, m_Skin->m_Color);

				float fTrackHeight = m_Size.y - m_UpDownButtonHeight * 2;
				float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
				int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
				float fThumbTop = UpButtonRect.b + (float)(m_nPosition - m_nStart) / nMaxPosition * (fTrackHeight - fThumbHeight);
				Rectangle ThumbButtonRect(Rect.l, fThumbTop, Rect.r, fThumbTop + fThumbHeight);

				Skin->DrawImage(ui_render, Skin->m_ThumbBtnNormalImage, ThumbButtonRect, m_Skin->m_Color);
			}
			else
			{
				Skin->DrawImage(ui_render, Skin->m_UpBtnDisabledImage, UpButtonRect, m_Skin->m_Color);

				Skin->DrawImage(ui_render, Skin->m_DownBtnDisabledImage, DownButtonRect, m_Skin->m_Color);
			}
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

bool ScrollBar::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//if(WM_CAPTURECHANGED == uMsg)
	//{
	//	if((HWND)lParam != DxutApp::getSingleton().GetHWND())
	//		m_bDrag = false;
	//}
	return false;
}

bool ScrollBar::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ScrollBar::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			Vector2 ptLocal = ScreenToLocal(pt);
			Rectangle UpButtonRect(Rectangle::LeftTop(Vector2(0,0), Vector2(m_Size.x, m_UpDownButtonHeight)));
			if(UpButtonRect.PtInRect(ptLocal))
			{
				if(m_nPosition > m_nStart)
					--m_nPosition;
				m_Arrow = CLICKED_UP;
				m_dwArrowTS = timeGetTime();
				SetCaptureControl(this);
				return true;
			}

			Rectangle DownButtonRect(Rectangle::RightBottom(m_Size, Vector2(m_Size.x, m_UpDownButtonHeight)));
			if(DownButtonRect.PtInRect(ptLocal))
			{
				if(m_nPosition + m_nPageSize < m_nEnd)
					++m_nPosition;
				m_Arrow = CLICKED_DOWN;
				m_dwArrowTS = timeGetTime();
				SetCaptureControl(this);
				return true;
			}

			float fTrackHeight = m_Size.y - m_UpDownButtonHeight * 2;
			float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
			int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
			float fMaxThumb = fTrackHeight - fThumbHeight;
			float fThumbTop = UpButtonRect.b + (float)(m_nPosition - m_nStart) / nMaxPosition * fMaxThumb;
			Rectangle ThumbButtonRect(0, fThumbTop, m_Size.x, fThumbTop + fThumbHeight);
			if(ThumbButtonRect.PtInRect(ptLocal))
			{
				m_bDrag = true;
				m_fThumbOffsetY = ptLocal.y - fThumbTop;
				SetCaptureControl(this);
				return true;
			}

			if(ptLocal.x >= ThumbButtonRect.l && ptLocal.x < ThumbButtonRect.r)
			{
				if(ptLocal.y >= UpButtonRect.b && ptLocal.y < ThumbButtonRect.t)
				{
					Scroll(-m_nPageSize);
					SetCaptureControl(this);
					return true;
				}
				else if(ptLocal.y >= ThumbButtonRect.b && ptLocal.y < DownButtonRect.t)
				{
					Scroll( m_nPageSize);
					SetCaptureControl(this);
					return true;
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			SetCaptureControl(NULL);
			m_bDrag = false;
			m_Arrow = CLEAR;
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			Vector2 ptLocal = ScreenToLocal(pt);
			Rectangle TrackRect(0, m_UpDownButtonHeight, m_Size.x, m_Size.y - m_UpDownButtonHeight);
			float fTrackHeight = m_Size.y - m_UpDownButtonHeight * 2;
			float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
			int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
			float fMaxThumb = fTrackHeight - fThumbHeight;
			float fThumbTop = ptLocal.y - m_fThumbOffsetY;

			if(fThumbTop < TrackRect.t)
				fThumbTop = TrackRect.t;
			else if(fThumbTop + fThumbHeight > TrackRect.b)
				fThumbTop = TrackRect.b - fThumbHeight;

			m_nPosition = (int)(m_nStart + (fThumbTop - TrackRect.t + fMaxThumb / (nMaxPosition * 2)) * nMaxPosition / fMaxThumb);
			return true;
		}
		break;
	}
	return false;
}

bool ScrollBar::CanHaveFocus(void)
{
	return m_bVisible && m_bEnabled;
}

void ScrollBar::Scroll(int nDelta)
{
	m_nPosition = Max(m_nStart, Min(m_nEnd - m_nPageSize, m_nPosition + nDelta));
}

void ScrollBar::ScrollTo(int nPosition)
{
	if (nPosition >= m_nStart)
	{
		if (nPosition < m_nPosition)
		{
			m_nPosition = nPosition;
		}
		else if (nPosition >= m_nPosition + m_nPageSize)
		{
			m_nPosition = Min(m_nEnd - m_nPageSize, nPosition + 1 - m_nPageSize);
		}
	}
}

void CheckBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		if(m_Skin)
		{
			ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin>(m_Skin);
			_ASSERT(Skin);

			Rectangle BtnRect(Rectangle::LeftMiddle(
				Offset.x + m_Location.x, Offset.y + m_Location.y + m_Size.y * 0.5f, m_CheckBtnSize.x, m_CheckBtnSize.y));

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, BtnRect, m_Skin->m_Color);
			}
			else
			{
				if(m_Checked)
				{
					Skin->DrawImage(ui_render, Skin->m_PressedImage, BtnRect, m_Skin->m_Color);
				}
				else
				{
					Skin->DrawImage(ui_render, Skin->m_Image, BtnRect, m_Skin->m_Color);
				}

				if(m_bMouseOver /*|| m_bHasFocus*/)
				{
					Skin->DrawImage(ui_render, Skin->m_MouseOverImage, BtnRect, m_Skin->m_Color);
				}
			}

			Rectangle TextRect(Rectangle::LeftTop(BtnRect.r, Offset.y + m_Location.y, m_Size.x - m_CheckBtnSize.x, m_Size.y));

			Skin->DrawString(ui_render, m_Text.c_str(), TextRect, Skin->m_TextColor, m_Skin->m_TextAlign);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Offset + m_Location);
		}
	}
}

bool CheckBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				m_Checked = true;

				if (m_Skin && !m_Skin->m_MouseClickSound.empty())
				{
					D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
				}

				if (m_EventMouseClick)
				{
					MouseEventArg arg(this, LocalToScreen(m_Location));
					m_EventMouseClick(&arg);
				}
				return true;
			}
			break;
		}
	}
	return false;
}

bool CheckBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(HitTest(pt))
			{
				m_bPressed = true;
				SetFocusControl(this);
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if(m_bPressed)
			{
				m_bPressed = false;
				SetCaptureControl(NULL);

				if(HitTest(pt))
				{
					m_Checked = true;

					if (m_Skin && !m_Skin->m_MouseClickSound.empty())
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
					}

					if(m_EventMouseClick)
					{
						MouseEventArg arg(this, pt);
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;
		}
	}
	return false;
}

template<class Archive>
void ComboBox::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Button);
	ar << BOOST_SERIALIZATION_NVP(m_DropdownSize);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollbarWidth);
	ar << BOOST_SERIALIZATION_NVP(m_ScrollbarUpDownBtnHeight);
	ar << BOOST_SERIALIZATION_NVP(m_Border);
	ar << BOOST_SERIALIZATION_NVP(m_ItemHeight);
}

template<class Archive>
void ComboBox::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Button);
	ar >> BOOST_SERIALIZATION_NVP(m_DropdownSize);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollbarWidth);
	ar >> BOOST_SERIALIZATION_NVP(m_ScrollbarUpDownBtnHeight);
	ar >> BOOST_SERIALIZATION_NVP(m_Border);
	ar >> BOOST_SERIALIZATION_NVP(m_ItemHeight);
	OnLayout();
}

template
void ComboBox::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void ComboBox::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void ComboBox::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void ComboBox::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void ComboBox::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void ComboBox::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void ComboBox::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void ComboBox::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void ComboBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(m_Skin)
		{
			ComboBoxSkinPtr Skin = boost::dynamic_pointer_cast<ComboBoxSkin>(m_Skin);
			_ASSERT(Skin);

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, Rect, m_Skin->m_Color);
			}
			else
			{
				if(m_bOpened)
				{
					ui_render->Flush();

					Rectangle DropdownRect(Rectangle::LeftTop(Rect.l, Rect.b, m_DropdownSize.x, m_DropdownSize.y));

					Rect = Rect.offset(Skin->m_PressedOffset);

					Skin->DrawImage(ui_render, Skin->m_PressedImage, Rect, m_Skin->m_Color);

					Skin->DrawImage(ui_render, Skin->m_DropdownImage, DropdownRect, m_Skin->m_Color);

					// ! ScrollBar source copy
					Rectangle ScrollBarRect(Rectangle::LeftTop(DropdownRect.r, DropdownRect.t, m_ScrollbarWidth, m_DropdownSize.y));

					Skin->DrawImage(ui_render, Skin->m_ScrollBarImage, ScrollBarRect, m_Skin->m_Color);

					Rectangle UpButtonRect(Rectangle::LeftTop(ScrollBarRect.l, ScrollBarRect.t, m_ScrollbarWidth, m_ScrollbarUpDownBtnHeight));

					Rectangle DownButtonRect(Rectangle::RightBottom(ScrollBarRect.r, ScrollBarRect.b, m_ScrollbarWidth, m_ScrollbarUpDownBtnHeight));

					if(m_ScrollBar.m_bEnabled && m_ScrollBar.m_nEnd - m_ScrollBar.m_nStart > m_ScrollBar.m_nPageSize)
					{
						Skin->DrawImage(ui_render, Skin->m_ScrollBarUpBtnNormalImage, UpButtonRect, m_Skin->m_Color);

						Skin->DrawImage(ui_render, Skin->m_ScrollBarDownBtnNormalImage, DownButtonRect, m_Skin->m_Color);

						float fTrackHeight = m_DropdownSize.y - m_ScrollbarUpDownBtnHeight * 2;
						float fThumbHeight = fTrackHeight * m_ScrollBar.m_nPageSize / (m_ScrollBar.m_nEnd - m_ScrollBar.m_nStart);
						int nMaxPosition = m_ScrollBar.m_nEnd - m_ScrollBar.m_nStart - m_ScrollBar.m_nPageSize;
						float fThumbTop = UpButtonRect.b + (float)(m_ScrollBar.m_nPosition - m_ScrollBar.m_nStart) / nMaxPosition * (fTrackHeight - fThumbHeight);
						Rectangle ThumbButtonRect(ScrollBarRect.l, fThumbTop, ScrollBarRect.r, fThumbTop + fThumbHeight);

						Skin->DrawImage(ui_render, Skin->m_ScrollBarThumbBtnNormalImage, ThumbButtonRect, m_Skin->m_Color);
					}
					else
					{
						Skin->DrawImage(ui_render, Skin->m_ScrollBarUpBtnDisabledImage, UpButtonRect, m_Skin->m_Color);

						Skin->DrawImage(ui_render, Skin->m_ScrollBarDownBtnDisabledImage, DownButtonRect, m_Skin->m_Color);
					}

					int i = m_ScrollBar.m_nPosition;
					float y = DropdownRect.t + m_Border.y;
					for(; i < (int)m_Items.size() && y <= DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect(Rectangle::LeftTop(DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight));

						if(i == m_iFocused)
						{
							Skin->DrawImage(ui_render, Skin->m_DropdownItemMouseOverImage, ItemRect, m_Skin->m_Color);
						}

						ComboBoxItem * item = m_Items[i].get();
						Rectangle ItemTextRect = ItemRect.shrink(m_Border.x, 0, m_Border.z, 0);
						Skin->DrawString(ui_render, item->strText.c_str(), ItemTextRect, Skin->m_DropdownItemTextColor, Skin->m_DropdownItemTextAlign);
					}
				}
				else
				{
					if(m_bMouseOver /*|| m_bHasFocus*/)
					{
						Rect = Rect.offset(-Skin->m_PressedOffset);
						Skin->DrawImage(ui_render, Skin->m_MouseOverImage, Rect, m_Skin->m_Color);
					}
					else
					{
						Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Skin->m_Color);
					}
				}
			}

			Rectangle TextRect = Rect.shrink(m_Border);
			if(m_iSelected >= 0 && m_iSelected < (int)m_Items.size())
				Skin->DrawString(ui_render, m_Items[m_iSelected]->strText.c_str(), TextRect, Skin->m_TextColor, m_Skin->m_TextAlign);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->Draw(ui_render, fElapsedTime, Rect.LeftTop());
		}
	}
}

bool ComboBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ComboBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				if (!m_bOpened)
				{
					m_bOpened = true;
					m_iFocused = m_iSelected;
					m_ScrollBar.ScrollTo(m_iFocused);
					return true;
				}
				else
				{
					if (m_iSelected != m_iFocused)
					{
						m_iSelected = m_iFocused;

						if (m_EventSelectionChanged)
						{
							ControlEventArg arg(this);
							m_EventSelectionChanged(&arg);
						}
					}
					m_bOpened = false;
					return true;
				}
			}
			else if (wParam == VK_UP)
			{
				if (m_bOpened)
				{
					if (m_iFocused > 0)
					{
						m_ScrollBar.ScrollTo(--m_iFocused);
					}
					return true;
				}
			}
			else if (wParam == VK_DOWN)
			{
				if (m_bOpened)
				{
					if (m_iFocused + 1 < (int)m_Items.size())
					{
						m_ScrollBar.ScrollTo(++m_iFocused);
					}
					return true;
				}
			}
			else if (wParam == VK_ESCAPE)
			{
				if (m_bOpened)
				{
					m_bOpened = false;
					return true;
				}
			}
			break;
		}
	}
	return false;
}

bool ComboBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		if(m_bHasFocus && m_bOpened)
		{
			Vector2 ptLocal = ScreenToLocal(pt);
			if(m_ScrollBar.HandleMouse(uMsg, Vector2(ptLocal.x, ptLocal.y - m_Size.y), wParam, lParam))
			{
				// ! overload scrollbars capture
				SetFocusControl(this);
				SetCaptureControl(this);
				return true;
			}
		}

		Rectangle DropdownRect(Rectangle::LeftTop(0, m_Size.y, m_DropdownSize.x, m_DropdownSize.y));

		switch(uMsg)
		{
		case WM_MOUSEMOVE:
			if(m_bHasFocus && m_bOpened)
			{
				Vector2 ptLocal = ScreenToLocal(pt);
				if(DropdownRect.PtInRect(ptLocal))
				{
					int i = m_ScrollBar.m_nPosition;
					float y = DropdownRect.t;
					for(; i < (int)m_Items.size() && y <= DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect(Rectangle::LeftTop(DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight));

						if(ItemRect.PtInRect(ptLocal))
						{
							m_iFocused = i;
							break;
						}
					}
					return true;
				}
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(HitTest(pt))
			{
				m_bPressed = true;
				m_bOpened = !m_bOpened;
				m_iFocused = m_iSelected;
				m_ScrollBar.ScrollTo(m_iFocused);
				SetFocusControl(this);
				SetCaptureControl(this);
				return true;
			}

			if(m_bHasFocus && m_bOpened)
			{
				Vector2 ptLocal = ScreenToLocal(pt);
				if(DropdownRect.PtInRect(ptLocal))
				{
					int i = m_ScrollBar.m_nPosition;
					float y = DropdownRect.t;
					for(; i < (int)m_Items.size() && y <= DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect(Rectangle::LeftTop(DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight));

						if(ItemRect.PtInRect(ptLocal))
						{
							if(m_iSelected != i)
							{
								m_iSelected = i;

								if(m_EventSelectionChanged)
								{
									ControlEventArg arg(this);
									m_EventSelectionChanged(&arg);
								}
							}
							m_bOpened = false;
							break;
						}
					}
					m_bPressed = true;
					SetCaptureControl(this);
					return true;
				}
			}
			break;

		case WM_LBUTTONUP:
			if(m_bPressed)
			{
				m_bPressed = false;
				SetCaptureControl(NULL);
				return true;
			}
			break;

		case WM_MOUSEWHEEL:
			if(m_bHasFocus)
			{
	            int zDelta = (short)HIWORD(wParam) / WHEEL_DELTA;
				if(m_bOpened)
				{
					UINT uLines;
					SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0);
					m_ScrollBar.Scroll(-zDelta * uLines);
				}
			}
			break;
		}
	}
	return false;
}

void ComboBox::OnFocusOut(void)
{
	m_bOpened = false;

	Control::OnFocusOut();
}

void ComboBox::OnLayout(void)
{
	m_ScrollBar.m_Location = Vector2(m_DropdownSize.x, 0);

	m_ScrollBar.m_Size = Vector2(m_ScrollbarWidth, m_DropdownSize.y);

	m_ScrollBar.m_nPageSize = (int)((m_DropdownSize.y - m_Border.y - m_Border.w) / m_ItemHeight);
}

void ComboBox::SetDropdownSize(const Vector2 & DropdownSize)
{
	m_DropdownSize = DropdownSize;

	OnLayout();
}

const Vector2 & ComboBox::GetDropdownSize(void) const
{
	return m_DropdownSize;
}

void ComboBox::SetBorder(const Vector4 & Border)
{
	m_Border = Border;

	OnLayout();
}

const Vector4 & ComboBox::GetBorder(void) const
{
	return m_Border;
}

void ComboBox::SetItemHeight(float ItemHeight)
{
	m_ItemHeight = ItemHeight;

	OnLayout();
}

float ComboBox::GetItemHeight(void) const
{
	return m_ItemHeight;
}

void ComboBox::SetSelected(int iSelected)
{
	m_iSelected = m_iFocused = iSelected;

	if(m_iSelected >= 0 && m_iSelected < (int)m_Items.size())
	{
		m_ScrollBar.m_nPosition = Min(m_iSelected, Max(0, m_ScrollBar.m_nEnd - m_ScrollBar.m_nPageSize));
	}
}

int ComboBox::GetSelected(void) const
{
	return m_iSelected;
}

void ComboBox::AddItem(const std::wstring & strText)
{
	_ASSERT(!strText.empty());

	ComboBoxItemPtr item(new ComboBoxItem(strText));

	m_Items.push_back(item);

	m_ScrollBar.m_nEnd = m_Items.size();
}

void ComboBox::RemoveAllItems(void)
{
	m_Items.clear();

	m_ScrollBar.m_nEnd = m_Items.size();
}

bool ComboBox::ContainsItem(const std::wstring & strText, UINT iStart) const
{
	return -1 != FindItem(strText, iStart);
}

int ComboBox::FindItem(const std::wstring & strText, UINT iStart) const
{
	struct Finder
	{
		const std::wstring & str;

		Finder(const std::wstring & _str)
			: str(_str)
		{
		}

		bool operator() (ComboBoxItemPtr item)
		{
			return item->strText == str;
		}
	};

	ComboBoxItemPtrList::const_iterator item_iter = std::find_if(m_Items.begin() + iStart, m_Items.end(), Finder(strText));
	if(item_iter != m_Items.end())
	{
		return std::distance(m_Items.begin(), item_iter);
	}
	return -1;
}

void * ComboBox::GetItemData(int index)
{
	return m_Items[index]->pData;
}

void ComboBox::SetItemData(int index, void * pData)
{
	m_Items[index]->pData = pData;
}

unsigned int ComboBox::GetItemDataUInt(int index)
{
	return PtrToUint(GetItemData(index));
}

void ComboBox::SetItemData(int index, unsigned int uData)
{
	SetItemData(index, UintToPtr(uData));
}

UINT ComboBox::GetNumItems(void)
{
	return m_Items.size();
}

Dialog::~Dialog(void)
{
	if (m_Parent)
	{
		m_Parent->RemoveDlg(this);
	}
}


template<class Archive>
void Dialog::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
	ar << BOOST_SERIALIZATION_NVP(m_World);
	ar << BOOST_SERIALIZATION_NVP(m_EnableDrag);
}

template<class Archive>
void Dialog::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
	ar >> BOOST_SERIALIZATION_NVP(m_World);
	ar >> BOOST_SERIALIZATION_NVP(m_EnableDrag);
}

template
void Dialog::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void Dialog::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void Dialog::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void Dialog::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void Dialog::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void Dialog::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void Dialog::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void Dialog::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void Dialog::Draw(UIRender * ui_render, float fElapsedTime)
{
	Control::Draw(ui_render, fElapsedTime, Vector2(0,0));
}

bool Dialog::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool Dialog::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		if (uMsg == WM_KEYDOWN)
		{
			ControlPtrList::iterator ctrl_iter = m_Childs.begin();
			for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
			{
				if ((*ctrl_iter)->GetHotkey() == wParam)
				{
					(*ctrl_iter)->OnHotkey();
					return true;
				}
			}
		}

		switch (uMsg)
		{
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				if (!s_FocusControl || !ContainsControl(s_FocusControl))
				{
					if (SetFocusRecursive())
					{
						if (s_FocusControl != s_MouseOverControl)
						{
							SetMouseOverControl(s_FocusControl, s_FocusControl->LocalToScreen(s_FocusControl->m_Location));
						}
						return true;
					}
				}
				else
				{
					Control * next_focus_ctrl = NULL;
					float next_focus_ctrl_diff = FLT_MAX;
					ControlPtrList::iterator ctrl_iter = m_Childs.begin();
					for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
					{
						if ((*ctrl_iter)->CanHaveFocus())
						{
							float diff = 0;
							switch (wParam)
							{
							case VK_UP:
								if ((*ctrl_iter)->m_Location.x < s_FocusControl->m_Location.x + s_FocusControl->m_Size.x
									&& (*ctrl_iter)->m_Location.x + (*ctrl_iter)->m_Size.x > s_FocusControl->m_Location.x)
								{
									diff = s_FocusControl->m_Location.y - (*ctrl_iter)->m_Location.y;
								}
								break;
							case VK_DOWN:
								if ((*ctrl_iter)->m_Location.x < s_FocusControl->m_Location.x + s_FocusControl->m_Size.x
									&& (*ctrl_iter)->m_Location.x + (*ctrl_iter)->m_Size.x > s_FocusControl->m_Location.x)
								{
									diff = (*ctrl_iter)->m_Location.y - s_FocusControl->m_Location.y;
								}
								break;
							case VK_LEFT:
								if ((*ctrl_iter)->m_Location.y < s_FocusControl->m_Location.y + s_FocusControl->m_Size.y
									&& (*ctrl_iter)->m_Location.y + (*ctrl_iter)->m_Size.y > s_FocusControl->m_Location.y)
								{
									diff = s_FocusControl->m_Location.x - (*ctrl_iter)->m_Location.x;
								}
								break;
							case VK_RIGHT:
								if ((*ctrl_iter)->m_Location.y < s_FocusControl->m_Location.y + s_FocusControl->m_Size.y
									&& (*ctrl_iter)->m_Location.y + (*ctrl_iter)->m_Size.y > s_FocusControl->m_Location.y)
								{
									diff = (*ctrl_iter)->m_Location.x - s_FocusControl->m_Location.x;
								}
								break;
							}
							if (diff > 0 && diff < next_focus_ctrl_diff)
							{
								next_focus_ctrl = ctrl_iter->get();
								next_focus_ctrl_diff = diff;
							}
						}
					}
					if (next_focus_ctrl)
					{
						SetFocusControl(next_focus_ctrl);
					}
					if (s_FocusControl != s_MouseOverControl)
					{
						SetMouseOverControl(s_FocusControl, s_FocusControl->LocalToScreen(s_FocusControl->m_Location));
					}
					return true;
				}
				break;
			}
		}
	}
	return false;
}

bool Dialog::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if (m_bEnabled && m_bVisible)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (HitTest(pt))
			{
				m_bPressed = true;
				m_MouseOffset = pt - m_Location;
				if (!s_FocusControl || !ContainsControl(s_FocusControl))
				{
					SetFocusRecursive();
				}
				SetCaptureControl(this);
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if (m_bPressed)
			{
				SetCaptureControl(NULL);
				m_bPressed = false;

				if (m_bMouseDrag)
				{
					m_bMouseDrag = false;
				}
				else
				{
					if (m_Skin && !m_Skin->m_MouseClickSound.empty())
					{
						D3DContext::getSingleton().OnControlSound(m_Skin->m_MouseClickSound.c_str());
					}

					if (m_EventMouseClick)
					{
						MouseEventArg arg(this, pt);
						m_EventMouseClick(&arg);
					}
				}
				return true;
			}
			break;

		case WM_MOUSEMOVE:
			if (m_bPressed)
			{
				if (m_EnableDrag && !m_bMouseDrag)
				{
					m_bMouseDrag = true;
				}

				if (m_bMouseDrag)
				{
					m_Location = pt - m_MouseOffset;
				}
			}
			break;
		}
	}
	return false;
}

bool Dialog::CanHaveFocus(void)
{
	return m_bVisible && m_bEnabled;
}

void Dialog::SetVisible(bool bVisible)
{
	Control::SetVisible(bVisible);

	if (m_bVisible)
	{
		if (!s_FocusControl || !ContainsControl(s_FocusControl))
		{
			SetFocusRecursive();
		}
	}
}

bool Dialog::RayToWorld(const Ray & ray, Vector2 & ptWorld)
{
	Vector3 dialogNormal = m_World[2].xyz.normalize();
	float dialogDist = m_World[3].xyz.dot(dialogNormal);
	RayResult result = IntersectionTests::rayAndHalfSpace(ray.p, ray.d, Plane::NormalDistance(dialogNormal, dialogDist));
	if (result.first)
	{
		Vector3 ptInt(ray.p + ray.d * result.second);
		ptWorld = ptInt.transformCoord(m_World.inverse()).xy;
		return true;
	}
	return false;
}

DialogPtr Dialog::LoadFromFile(const char * path)
{
	IStreamBuff buff(my::ResourceMgr::getSingleton().OpenIStream(path));
	std::istream istr(&buff);
	LPCSTR Ext = PathFindExtensionA(path);
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia;
	if (_stricmp(Ext, ".xml") == 0)
	{
		ia.reset(new boost::archive::polymorphic_xml_iarchive(istr));
	}
	else if (_stricmp(Ext, ".txt") == 0)
	{
		ia.reset(new boost::archive::polymorphic_text_iarchive(istr));
	}
	else
	{
		ia.reset(new boost::archive::polymorphic_binary_iarchive(istr));
	}
	DialogPtr ret;
	*ia >> boost::serialization::make_nvp("Dialog", ret);
	return ret;
}

void Dialog::SaveToFile(const char * path) const
{
	std::ofstream ostr(my::ResourceMgr::getSingleton().GetFullPath(path), std::ios::binary, _OPENPROT);
	LPCSTR Ext = PathFindExtensionA(path);
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa;
	if (_stricmp(Ext, ".xml") == 0)
	{
		oa.reset(new boost::archive::polymorphic_xml_oarchive(ostr));
	}
	else if (_stricmp(Ext, ".txt") == 0)
	{
		oa.reset(new boost::archive::polymorphic_text_oarchive(ostr));
	}
	else
	{
		oa.reset(new boost::archive::polymorphic_binary_oarchive(ostr));
	}
	*oa << boost::serialization::make_nvp("Dialog", boost::dynamic_pointer_cast<const Dialog>(shared_from_this()));
}

void DialogMgr::SetDlgViewport(const Vector2 & Viewport, float fov)
{
	m_Eye = Vector3(Viewport.x * 0.5f, Viewport.y * 0.5f, -Viewport.y * 0.5f * cot(fov / 2));

	m_View = Matrix4::LookAtRH(m_Eye, Vector3(m_Eye.x, m_Eye.y, 0), Vector3(0, -1, 0));

	m_Proj = Matrix4::PerspectiveFovRH(fov, Viewport.x / Viewport.y, 0.1f, 3000.0f);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();

	DialogList::iterator dlg_iter = m_DlgList.begin();
	for(; dlg_iter != m_DlgList.end(); dlg_iter++)
	{
		if((*dlg_iter)->m_EventAlign)
		{
			ControlEventArg arg(*dlg_iter);
			(*dlg_iter)->m_EventAlign(&arg);
		}
	}
}

Ray DialogMgr::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	Vector3 At = Vector3(Lerp(-1.0f, 1.0f, pt.x / dim.cx), Lerp(1.0f, -1.0f, pt.y / dim.cy), 0.0f).transformCoord(m_InverseViewProj);

	return Ray(m_Eye, (At - m_Eye).normalize());
}

Vector2 DialogMgr::GetDlgViewport(void) const
{
	return Vector2(-m_View._41*2, m_View._42*2);
}

void DialogMgr::Draw(UIRender * ui_render, double fTime, float fElapsedTime)
{
	ui_render->SetViewProj(m_ViewProj);

	DialogList::iterator dlg_iter = m_DlgList.begin();
	for(; dlg_iter != m_DlgList.end(); dlg_iter++)
	{
		ui_render->SetWorld((*dlg_iter)->m_World);

		(*dlg_iter)->Draw(ui_render, fElapsedTime);

		ui_render->Flush();
	}
}

bool DialogMgr::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Control::s_FocusControl) {
		if (Control::s_FocusControl->MsgProc(hWnd, uMsg, wParam, lParam)) {
			return true;
		}
	}

	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			if (Control::s_FocusControl) {
				if (Control::s_FocusControl->HandleKeyboard(uMsg, wParam, lParam)) {
					return true;
				}
			}

			DialogList::reverse_iterator dlg_iter = m_DlgList.rbegin();
			for(; dlg_iter != m_DlgList.rend(); dlg_iter++)
			{
				if((*dlg_iter)->HandleKeyboard(uMsg, wParam, lParam))
					return true;
			}
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
		{
			CRect ClientRect;
			GetClientRect(hWnd, &ClientRect);
			Ray ray = CalculateRay(Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());

			if (Control::s_CaptureControl) {
				Vector2 pt;
				if (Control::s_CaptureControl->RayToWorld(ray, pt))
				{
					if (Control::s_CaptureControl->HandleMouse(uMsg, pt, wParam, lParam)) {
						return true;
					}
				}
				break;
			}

			if (Control::s_FocusControl && dynamic_cast<ComboBox *>(Control::s_FocusControl)) {
				Vector2 pt;
				if (Control::s_FocusControl->RayToWorld(ray, pt))
				{
					if (Control::s_FocusControl->HandleMouse(uMsg, pt, wParam, lParam)) {
						return true;
					}
				}
				//break;
			}

			DialogList::reverse_iterator dlg_iter = m_DlgList.rbegin();
			for(; dlg_iter != m_DlgList.rend(); dlg_iter++)
			{
				// ! 只处理看得见的 Dialog
				if((*dlg_iter)->GetVisible())
				{
					Vector2 pt;
					if ((*dlg_iter)->RayToWorld(ray, pt))
					{
						if ((*dlg_iter)->HitTest(pt))
						{
							Control * ControlPtd = (*dlg_iter)->GetChildAtPoint(pt);
							if (ControlPtd)
							{
								Control::SetMouseOverControl(ControlPtd, pt);

								if(ControlPtd->HandleMouse(uMsg, pt, wParam, lParam))
								{
									return true;
								}
							}

							if (ControlPtd != *dlg_iter)
							{
								if ((*dlg_iter)->HandleMouse(uMsg, pt, wParam, lParam))
								{
									return true;
								}
							}
							break;
						}
					}
				}
			}

			if (dlg_iter == m_DlgList.rend() && Control::s_MouseOverControl)
			{
				Control::SetMouseOverControl(NULL, Vector2(FLT_MAX, FLT_MAX));
			}

			if (uMsg == WM_LBUTTONDOWN && dlg_iter == m_DlgList.rend() && Control::s_FocusControl)
			{
				Control::SetFocusControl(NULL);
			}
		}
		break;
	}

	return false;
}

void DialogMgr::InsertDlg(Dialog * dlg)
{
	_ASSERT(!dlg->m_Parent);

	m_DlgList.push_back(dlg);

	dlg->m_Parent = this;

	if(dlg->m_EventAlign)
	{
		ControlEventArg arg(dlg);
		dlg->m_EventAlign(&arg);
	}
}

void DialogMgr::RemoveDlg(Dialog * dlg)
{
	DialogList::iterator dlg_iter = std::find(m_DlgList.begin(), m_DlgList.end(), dlg);
	if(dlg_iter != m_DlgList.end())
	{
		_ASSERT((*dlg_iter)->m_Parent == this);

		if (Control::s_FocusControl && (*dlg_iter)->ContainsControl(Control::s_FocusControl))
		{
			Control::SetFocusControl(NULL);
		}

		(*dlg_iter)->m_Parent = NULL;

		m_DlgList.erase(dlg_iter);
	}
	else
	{
		_ASSERT(false);
	}
}

void DialogMgr::RemoveAllDlg()
{
	DialogList::iterator dlg_iter = m_DlgList.begin();
	for (; dlg_iter != m_DlgList.end(); dlg_iter = m_DlgList.begin())
	{
		RemoveDlg(*dlg_iter);
	}
}

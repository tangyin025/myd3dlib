#include "stdafx.h"
#include "myUi.h"
#include "myDxutApp.h"
#include "myCollision.h"
#include "ImeUi.h"
#include "libc.h"

using namespace my;

UIRender::~UIRender(void)
{
}

void UIRender::Begin(void)
{
	// ! Default UIRender rendering ui elements under Fixed Pipeline
	V(m_Device->GetRenderState(D3DRS_CULLMODE, &State[0]));
	V(m_Device->GetRenderState(D3DRS_LIGHTING, &State[1]));
	V(m_Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &State[2]));
	V(m_Device->GetRenderState(D3DRS_SRCBLEND, &State[3]));
	V(m_Device->GetRenderState(D3DRS_DESTBLEND, &State[4]));
	V(m_Device->GetRenderState(D3DRS_ZENABLE, &State[5]));
	V(m_Device->GetSamplerState(0, D3DSAMP_MAGFILTER, &State[6]));
	V(m_Device->GetSamplerState(0, D3DSAMP_MINFILTER, &State[7]));
	V(m_Device->GetSamplerState(0, D3DSAMP_MIPFILTER, &State[8]));
	V(m_Device->GetTextureStageState(0, D3DTSS_TEXCOORDINDEX, &State[9]));
	V(m_Device->GetTextureStageState(0, D3DTSS_COLOROP, &State[10]));
	V(m_Device->GetTextureStageState(0, D3DTSS_COLORARG1, &State[11]));
	V(m_Device->GetTextureStageState(0, D3DTSS_COLORARG2, &State[12]));
	V(m_Device->GetTextureStageState(0, D3DTSS_ALPHAOP, &State[13]));
	V(m_Device->GetTextureStageState(0, D3DTSS_ALPHAARG1, &State[14]));
	V(m_Device->GetTextureStageState(0, D3DTSS_ALPHAARG2, &State[15]));

	V(m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(m_Device->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
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
	V(m_Device->SetRenderState(D3DRS_CULLMODE, State[0]));
	V(m_Device->SetRenderState(D3DRS_LIGHTING, State[1]));
	V(m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, State[2]));
	V(m_Device->SetRenderState(D3DRS_SRCBLEND, State[3]));
	V(m_Device->SetRenderState(D3DRS_DESTBLEND, State[4]));
	V(m_Device->SetRenderState(D3DRS_ZENABLE, State[5]));
	V(m_Device->SetSamplerState(0, D3DSAMP_MAGFILTER, State[6]));
	V(m_Device->SetSamplerState(0, D3DSAMP_MINFILTER, State[7]));
	V(m_Device->SetSamplerState(0, D3DSAMP_MIPFILTER, State[8]));
	V(m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, State[9]));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLOROP, State[10]));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, State[11]));
	V(m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, State[12]));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, State[13]));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, State[14]));
	V(m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, State[15]));
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

void UIRender::SetTexture(const BaseTexturePtr & Texture)
{
	V(m_Device->SetTexture(0, Texture ? Texture->m_ptr : NULL));
}

void UIRender::ClearVertexList(void)
{
	vertex_count = 0;
}

void UIRender::DrawVertexList(void)
{
	if(vertex_count > 0)
	{
		V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
		V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_count / 3, vertex_list, sizeof(vertex_list[0])));
	}
}

void UIRender::PushVertex(float x, float y, float z, float u, float v, D3DCOLOR color)
{
	if(vertex_count < _countof(vertex_list))
	{
		vertex_list[vertex_count].x = x;
		vertex_list[vertex_count].y = y;
		vertex_list[vertex_count].z = z;
		vertex_list[vertex_count].u = u;
		vertex_list[vertex_count].v = v;
		vertex_list[vertex_count].color = color;
		vertex_count++;
	}
}

void UIRender::PushRectangle(const my::Rectangle & rect, const my::Rectangle & UvRect, D3DCOLOR color)
{
	PushVertex(rect.l, rect.t, 0, UvRect.l, UvRect.t, color);
	PushVertex(rect.r, rect.t, 0, UvRect.r, UvRect.t, color);
	PushVertex(rect.l, rect.b, 0, UvRect.l, UvRect.b, color);
	PushVertex(rect.r, rect.b, 0, UvRect.r, UvRect.b, color);
	PushVertex(rect.l, rect.b, 0, UvRect.l, UvRect.b, color);
	PushVertex(rect.r, rect.t, 0, UvRect.r, UvRect.t, color);
}

void UIRender::DrawRectangle(const my::Rectangle & rect, DWORD color, const my::Rectangle & UvRect)
{
	ClearVertexList();
	PushRectangle(rect, UvRect, color);
	DrawVertexList();
}

void UIRender::PushWindow(const my::Rectangle & rect, DWORD color, const CSize & WindowSize, const Vector4 & WindowBorder)
{
	Rectangle InRect(rect.l + WindowBorder.x, rect.t + WindowBorder.y, rect.r - WindowBorder.z, rect.b - WindowBorder.w);
	Rectangle InUvRect(WindowBorder.x / WindowSize.cx, WindowBorder.y / WindowSize.cy, (WindowSize.cx - WindowBorder.z) / WindowSize.cx, (WindowSize.cy - WindowBorder.w) / WindowSize.cy);
	PushRectangle(Rectangle(rect.l, rect.t, InRect.l, InRect.t), Rectangle(0, 0, InUvRect.l, InUvRect.t), color);
	PushRectangle(Rectangle(InRect.l, rect.t, InRect.r, InRect.t), Rectangle(InUvRect.l, 0, InUvRect.r, InUvRect.t), color);
	PushRectangle(Rectangle(InRect.r, rect.t, rect.r, InRect.t), Rectangle(InUvRect.r, 0, 1, InUvRect.t), color);
	PushRectangle(Rectangle(rect.l, InRect.t, InRect.l, InRect.b), Rectangle(0, InUvRect.t, InUvRect.l, InUvRect.b), color);
	PushRectangle(Rectangle(InRect.l, InRect.t, InRect.r, InRect.b), Rectangle(InUvRect.l, InUvRect.t, InUvRect.r, InUvRect.b), color);
	PushRectangle(Rectangle(InRect.r, InRect.t, rect.r, InRect.b), Rectangle(InUvRect.r, InUvRect.t, 1, InUvRect.b), color);
	PushRectangle(Rectangle(rect.l, InRect.b, InRect.l, rect.b), Rectangle(0, InUvRect.b, InUvRect.l, 1), color);
	PushRectangle(Rectangle(InRect.l, InRect.b, InRect.r, rect.b), Rectangle(InUvRect.l, InUvRect.b, InUvRect.r, 1), color);
	PushRectangle(Rectangle(InRect.r, InRect.b, rect.r, rect.b), Rectangle(InUvRect.r, InUvRect.b, 1, 1), color);
}

void UIRender::DrawWindow(const my::Rectangle & rect, DWORD color, const CSize & WindowSize, const Vector4 & WindowBorder)
{
	ClearVertexList();
	PushWindow(rect, color, WindowSize, WindowBorder);
	DrawVertexList();
}

ControlSkin::~ControlSkin(void)
{
}

void ControlSkin::DrawImage(UIRender * ui_render, ControlImagePtr Image, const my::Rectangle & rect, DWORD color)
{
	if(Image && Image->m_Texture)
	{
		ui_render->SetTexture(Image->m_Texture);
		ui_render->DrawWindow(rect, color, CSize(Image->m_Texture->GetLevelDesc().Width, Image->m_Texture->GetLevelDesc().Height), Image->m_Border);
	}
	else
	{
		ui_render->SetTexture(BaseTexturePtr());
		ui_render->DrawRectangle(rect, color, Rectangle(0,0,1,1));
	}
}

void ControlSkin::DrawString(UIRender * ui_render, LPCWSTR pString, const my::Rectangle & rect, DWORD TextColor, Font::Align TextAlign)
{
	if(m_Font)
	{
		m_Font->DrawString(ui_render, pString, rect, TextColor, TextAlign);
	}
}

Control::~Control(void)
{
	// ! must clear global reference
	ReleaseFocus();

	ReleaseCapture();

	ReleaseMouseOver();

	// ! must detach parent relationship
	ClearAllControl();
}

void Control::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(m_Skin && m_Color & D3DCOLOR_ARGB(255,0,0,0))
		{
			m_Skin->DrawImage(ui_render, m_Skin->m_Image, Rect, m_Color);
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

void Control::OnMouseEnter(void)
{
	m_bMouseOver = true;
}

void Control::OnMouseLeave(void)
{
	m_bMouseOver = false;
}

void Control::OnHotkey(void)
{
}

bool Control::ContainsPoint(const Vector2 & pt)
{
	return Rectangle::LeftTop(Vector2(0,0), m_Size).PtInRect(WorldToLocal(pt));
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
	m_bVisible = bVisible;
}

bool Control::GetVisible(void)
{
	return m_bVisible;
}

void Control::Refresh(void)
{
}

bool Control::RayToWorld(const std::pair<Vector3, Vector3> & ray, Vector2 & ptWorld)
{
	if (m_Parent)
	{
		return m_Parent->RayToWorld(ray, ptWorld);
	}
	return false;
}

void Control::SetHotkey(UINT nHotkey)
{
	m_nHotkey = nHotkey;
}

Control * Control::GetChildAtPoint(const Vector2 & pt) const
{
	ControlPtrList::const_iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		if ((*ctrl_iter)->ContainsPoint(pt))
		{
			Control * ctrl = (*ctrl_iter)->GetChildAtPoint(pt);
			if (ctrl)
			{
				return ctrl;
			}
			return ctrl_iter->get();
		}
	}
	return NULL;
}

Vector2 Control::LocalToWorld(const Vector2 & pt) const
{
	if (m_Parent)
	{
		return m_Parent->LocalToWorld(m_Location + pt);
	}
	return m_Location + pt;
}

Vector2 Control::WorldToLocal(const Vector2 & pt) const
{
	if (m_Parent)
	{
		return m_Parent->WorldToLocal(pt) - m_Location;
	}
	return pt - m_Location;
}

UINT Control::GetHotkey(void)
{
	return m_nHotkey;
}

void Control::SetFocus(void)
{
	if (Dialog::s_FocusControl != this)
	{
		if (Dialog::s_FocusControl)
		{
			Dialog::s_FocusControl->OnFocusOut();
		}

		Dialog::s_FocusControl = this;

		OnFocusIn();
	}
}

void Control::ReleaseFocus(void)
{
	if (Dialog::s_FocusControl == this)
	{
		OnFocusOut();

		Dialog::s_FocusControl = NULL;
	}
}

void Control::SetCapture(void)
{
	Dialog::s_CaptureControl = this;
}

void Control::ReleaseCapture(void)
{
	if (Dialog::s_CaptureControl == this)
	{
		Dialog::s_CaptureControl = NULL;
	}
}

void Control::SetMouseOver(void)
{
	if (Dialog::s_MouseOverControl != this)
	{
		if (Dialog::s_MouseOverControl)
		{
			Dialog::s_MouseOverControl->OnMouseLeave();
		}

		Dialog::s_MouseOverControl = this;

		OnMouseEnter();
	}
}

void Control::ReleaseMouseOver(void)
{
	if (Dialog::s_MouseOverControl == this)
	{
		OnMouseLeave();

		Dialog::s_MouseOverControl = NULL;
	}
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
}

void Control::ClearAllControl(void)
{
	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for (; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		_ASSERT((*ctrl_iter)->m_Parent == this);

		(*ctrl_iter)->m_Parent = NULL;
	}
	m_Childs.clear();
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
		ProgressBarSkinPtr Skin = boost::dynamic_pointer_cast<ProgressBarSkin>(m_Skin);

		if(Skin && m_Color & D3DCOLOR_ARGB(255,0,0,0))
		{
			Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

			Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Color);

			m_BlendProgress = Lerp(m_BlendProgress, m_Progress, 1.0f - powf(0.8f, 30 * fElapsedTime));
			Rect.r = Lerp(Rect.l, Rect.r, Max(0.0f, Min(1.0f, m_BlendProgress)));
			Skin->DrawImage(ui_render, Skin->m_ForegroundImage, Rect, m_Color);
		}
	}
}

void Button::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin>(m_Skin);

		if(Skin)
		{
			Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, Rect, m_Color);
			}
			else
			{
				if(m_bPressed)
				{
					Rect = Rect.offset(Skin->m_PressedOffset);
					Skin->DrawImage(ui_render, Skin->m_PressedImage, Rect, m_Color);
				}
				else
				{
					D3DXCOLOR DstColor(m_Color);
					if(m_bMouseOver /*|| m_bHasFocus*/)
					{
						Rect = Rect.offset(-Skin->m_PressedOffset);
					}
					else
					{
						DstColor.a = 0;
					}
					Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Color);
					D3DXColorLerp(&m_BlendColor, &m_BlendColor, &DstColor, 1.0f - powf(0.8f, 30 * fElapsedTime));
					Skin->DrawImage(ui_render, Skin->m_MouseOverImage, Rect, m_BlendColor);
				}
			}

			Skin->DrawString(ui_render, m_Text.c_str(), Rect, Skin->m_TextColor, m_Skin->m_TextAlign);
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
			case VK_SPACE:
				m_bPressed = true;
				return true;
			}
			break;

		case WM_KEYUP:
			switch(wParam)
			{
			case VK_SPACE:
				if(m_bPressed)
				{
					m_bPressed = false;

					if(EventClick)
						EventClick(&EventArgs());
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
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(ContainsPoint(pt))
			{
				m_bPressed = true;
				SetFocus();
				SetCapture();
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if(m_bPressed)
			{
				ReleaseCapture();
				m_bPressed = false;

				if(ContainsPoint(pt))
				{
					if(EventClick)
						EventClick(&EventArgs());
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
	if(EventClick)
		EventClick(&EventArgs());
}

bool Button::ContainsPoint(const Vector2 & pt)
{
	return Control::ContainsPoint(pt);
}

void Button::Refresh(void)
{
	m_BlendColor = m_Color;
}

void EditBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);

		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(Skin)
		{
			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, Rect, m_Color);
			}
			else if(m_bHasFocus)
			{
				Skin->DrawImage(ui_render, Skin->m_FocusedImage, Rect, m_Color);
			}
			else
			{
				Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Color);
			}
		}

		DWORD dwAbsoluteTime = timeGetTime();
		if(dwAbsoluteTime - m_dwLastBlink >= m_dwBlink )
		{
			m_bCaretOn = !m_bCaretOn;
			m_dwLastBlink = dwAbsoluteTime;
		}

		if(Skin && Skin->m_Font)
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

				ui_render->SetTexture(BaseTexturePtr());
				ui_render->DrawRectangle(SelRect, Skin->m_SelBkColor, Rectangle(0,0,1,1));
			}

			Skin->m_Font->DrawString(ui_render, m_Text.c_str() + m_nFirstVisible, TextRect, Skin->m_TextColor, Font::AlignLeftMiddle);

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

				ui_render->SetTexture(BaseTexturePtr());
				ui_render->DrawRectangle(CaretRect, Skin->m_CaretColor, Rectangle(0,0,1,1));
			}
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
						if(EventChange)
							EventChange(&EventArgs());
					}
					else if(m_nCaret > 0)
					{
						PlaceCaret(m_nCaret - 1);
						m_nSelStart = m_nCaret;
						m_Text.erase(m_nCaret, 1);
						if(EventChange)
							EventChange(&EventArgs());
					}
					ResetCaretBlink();
					break;

                case 24:        // Ctrl-X Cut
                case VK_CANCEL: // Ctrl-C Copy
					CopyToClipboard();
					if((WCHAR)wParam == 24)
					{
						DeleteSelectionText();
						if(EventChange)
							EventChange(&EventArgs());
					}
					break;

				case 22:		// Ctrl-V Paste
					PasteFromClipboard();
					if(EventChange)
						EventChange(&EventArgs());
					break;

				case 1:
					//if(m_nSelStart == m_nCaret)
					{
						m_nSelStart = 0;
						PlaceCaret(m_Text.length());
					}
					break;

				case VK_RETURN:
					if(EventEnter)
						EventEnter(&EventArgs());
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
					if(EventChange)
						EventChange(&EventArgs());
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
					if(EventChange)
						EventChange(&EventArgs());
				}
				else
				{
					m_Text.erase(m_nCaret, 1);
					if(EventChange)
						EventChange(&EventArgs());
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
			if(ContainsPoint(pt))
			{
				m_bMouseDrag = true;
				SetFocus();
				SetCapture();

				if(m_Skin && m_Skin->m_Font)
				{
					Vector2 ptLocal = WorldToLocal(pt);
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
				ReleaseCapture();
				m_bMouseDrag = false;
			}
			break;

		case WM_MOUSEMOVE:
			if(m_bMouseDrag)
			{
				if(m_Skin && m_Skin->m_Font)
				{
					Vector2 ptLocal = WorldToLocal(pt);
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

bool EditBox::ContainsPoint(const Vector2 & pt)
{
	return Control::ContainsPoint(pt);
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
	if(m_bEnabled && m_bVisible)
	{
		return EditBox::HandleMouse(uMsg, pt, wParam, lParam);
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
	EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
	if(Skin && Skin->m_Font)
	{
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

		ui_render->SetTexture(BaseTexturePtr());
		ui_render->DrawRectangle(rc, m_CompWinColor, Rectangle(0,0,1,1));

		Skin->m_Font->DrawString(ui_render, s_CompString.c_str(), rc, Skin->m_TextColor, Font::AlignLeftTop);

		float caret_x = Skin->m_Font->CPtoX(s_CompString.c_str(), ImeUi_GetImeCursorChars());
		if(m_bCaretOn)
		{
			Rectangle CaretRect(rc.l + caret_x - 1, rc.t, rc.l + caret_x + 1, rc.b);

			ui_render->SetTexture(BaseTexturePtr());
			ui_render->DrawRectangle(CaretRect, Skin->m_CaretColor, Rectangle(0,0,1,1));
		}
	}
}

void ImeEditBox::RenderCandidateWindow(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin>(m_Skin);
	if(Skin && Skin->m_Font)
	{
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

		ui_render->SetTexture(BaseTexturePtr());
		ui_render->DrawRectangle(CandRect, m_CandidateWinColor, Rectangle(0,0,1,1));

		Skin->m_Font->DrawString(ui_render, horizontalText.c_str(), CandRect, Skin->m_TextColor, Font::AlignLeftTop);
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
		ScrollBarSkinPtr Skin = boost::dynamic_pointer_cast<ScrollBarSkin>(m_Skin);

		Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

		if(Skin)
		{
			Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Color);

			Rectangle UpButtonRect(Rectangle::LeftTop(Rect.l, Rect.t, m_Size.x, m_UpDownButtonHeight));

			Rectangle DownButtonRect(Rectangle::RightBottom(Rect.r, Rect.b, m_Size.x, m_UpDownButtonHeight));

			if(m_bEnabled && m_nEnd - m_nStart > m_nPageSize)
			{
				Skin->DrawImage(ui_render, Skin->m_UpBtnNormalImage, UpButtonRect, m_Color);

				Skin->DrawImage(ui_render, Skin->m_DownBtnNormalImage, DownButtonRect, m_Color);

				float fTrackHeight = m_Size.y - m_UpDownButtonHeight * 2;
				float fThumbHeight = fTrackHeight * m_nPageSize / (m_nEnd - m_nStart);
				int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
				float fThumbTop = UpButtonRect.b + (float)(m_nPosition - m_nStart) / nMaxPosition * (fTrackHeight - fThumbHeight);
				Rectangle ThumbButtonRect(Rect.l, fThumbTop, Rect.r, fThumbTop + fThumbHeight);

				Skin->DrawImage(ui_render, Skin->m_ThumbBtnNormalImage, ThumbButtonRect, m_Color);
			}
			else
			{
				Skin->DrawImage(ui_render, Skin->m_UpBtnDisabledImage, UpButtonRect, m_Color);

				Skin->DrawImage(ui_render, Skin->m_DownBtnDisabledImage, DownButtonRect, m_Color);
			}
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
			Vector2 ptLocal = WorldToLocal(pt);
			Rectangle UpButtonRect(Rectangle::LeftTop(Vector2(0,0), Vector2(m_Size.x, m_UpDownButtonHeight)));
			if(UpButtonRect.PtInRect(ptLocal))
			{
				if(m_nPosition > m_nStart)
					--m_nPosition;
				m_Arrow = CLICKED_UP;
				m_dwArrowTS = timeGetTime();
				SetCapture();
				return true;
			}

			Rectangle DownButtonRect(Rectangle::RightBottom(m_Size, Vector2(m_Size.x, m_UpDownButtonHeight)));
			if(DownButtonRect.PtInRect(ptLocal))
			{
				if(m_nPosition + m_nPageSize < m_nEnd)
					++m_nPosition;
				m_Arrow = CLICKED_DOWN;
				m_dwArrowTS = timeGetTime();
				SetCapture();
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
				SetCapture();
				return true;
			}

			if(ptLocal.x >= ThumbButtonRect.l && ptLocal.x < ThumbButtonRect.r)
			{
				if(ptLocal.y >= UpButtonRect.b && ptLocal.y < ThumbButtonRect.t)
				{
					Scroll(-m_nPageSize);
					SetCapture();
					return true;
				}
				else if(ptLocal.y >= ThumbButtonRect.b && ptLocal.y < DownButtonRect.t)
				{
					Scroll( m_nPageSize);
					SetCapture();
					return true;
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			ReleaseCapture();
			m_bDrag = false;
			m_Arrow = CLEAR;
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if(m_bDrag)
		{
			Vector2 ptLocal = WorldToLocal(pt);
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

void CheckBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin>(m_Skin);

		if(Skin)
		{
			Rectangle BtnRect(Rectangle::LeftMiddle(
				Offset.x + m_Location.x, Offset.y + m_Location.y + m_Size.y * 0.5f, m_CheckBtnSize.x, m_CheckBtnSize.y));

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, BtnRect, m_Color);
			}
			else
			{
				if(m_Checked)
				{
					Skin->DrawImage(ui_render, Skin->m_PressedImage, BtnRect, m_Color);
				}
				else
				{
					Skin->DrawImage(ui_render, Skin->m_Image, BtnRect, m_Color);
				}

				D3DXCOLOR DstColor(m_Color);
				if(m_bMouseOver /*|| m_bHasFocus*/)
				{
				}
				else
				{
					DstColor.a = 0;
				}
				D3DXColorLerp(&m_BlendColor, &m_BlendColor, &DstColor, 1.0f - powf(0.8f, 30 * fElapsedTime));
				Skin->DrawImage(ui_render, Skin->m_MouseOverImage, BtnRect, m_BlendColor);
			}

			Rectangle TextRect(Rectangle::LeftTop(BtnRect.r, Offset.y + m_Location.y, m_Size.x - m_CheckBtnSize.x, m_Size.y));

			Skin->DrawString(ui_render, m_Text.c_str(), TextRect, Skin->m_TextColor, m_Skin->m_TextAlign);
		}
	}
}

bool CheckBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
			if(ContainsPoint(pt))
			{
				m_bPressed = true;
				SetFocus();
				SetCapture();
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if(m_bPressed)
			{
				m_bPressed = false;
				ReleaseCapture();

				if(ContainsPoint(pt))
				{
					m_Checked = true;

					if(EventClick)
						EventClick(&EventArgs());
				}
				return true;
			}
			break;
		}
	}
	return false;
}

void ComboBox::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		ComboBoxSkinPtr Skin = boost::dynamic_pointer_cast<ComboBoxSkin>(m_Skin);

		if(m_Skin)
		{
			Rectangle Rect(Rectangle::LeftTop(Offset + m_Location, m_Size));

			if(!m_bEnabled)
			{
				Skin->DrawImage(ui_render, Skin->m_DisabledImage, Rect, m_Color);
			}
			else
			{
				if(m_bOpened)
				{
					Rectangle DropdownRect(Rectangle::LeftTop(Rect.l, Rect.b, m_DropdownSize.x, m_DropdownSize.y));

					Rect = Rect.offset(Skin->m_PressedOffset);

					Skin->DrawImage(ui_render, Skin->m_PressedImage, Rect, m_Color);

					Skin->DrawImage(ui_render, Skin->m_DropdownImage, DropdownRect, m_Color);

					// ! ScrollBar source copy
					Rectangle ScrollBarRect(Rectangle::LeftTop(DropdownRect.r, DropdownRect.t, m_ScrollbarWidth, m_DropdownSize.y));

					Skin->DrawImage(ui_render, Skin->m_ScrollBarImage, ScrollBarRect, m_Color);

					Rectangle UpButtonRect(Rectangle::LeftTop(ScrollBarRect.l, ScrollBarRect.t, m_ScrollbarWidth, m_ScrollbarUpDownBtnHeight));

					Rectangle DownButtonRect(Rectangle::RightBottom(ScrollBarRect.r, ScrollBarRect.b, m_ScrollbarWidth, m_ScrollbarUpDownBtnHeight));

					if(m_ScrollBar.m_bEnabled && m_ScrollBar.m_nEnd - m_ScrollBar.m_nStart > m_ScrollBar.m_nPageSize)
					{
						Skin->DrawImage(ui_render, Skin->m_ScrollBarUpBtnNormalImage, UpButtonRect, m_Color);

						Skin->DrawImage(ui_render, Skin->m_ScrollBarDownBtnNormalImage, DownButtonRect, m_Color);

						float fTrackHeight = m_DropdownSize.y - m_ScrollbarUpDownBtnHeight * 2;
						float fThumbHeight = fTrackHeight * m_ScrollBar.m_nPageSize / (m_ScrollBar.m_nEnd - m_ScrollBar.m_nStart);
						int nMaxPosition = m_ScrollBar.m_nEnd - m_ScrollBar.m_nStart - m_ScrollBar.m_nPageSize;
						float fThumbTop = UpButtonRect.b + (float)(m_ScrollBar.m_nPosition - m_ScrollBar.m_nStart) / nMaxPosition * (fTrackHeight - fThumbHeight);
						Rectangle ThumbButtonRect(ScrollBarRect.l, fThumbTop, ScrollBarRect.r, fThumbTop + fThumbHeight);

						Skin->DrawImage(ui_render, Skin->m_ScrollBarThumbBtnNormalImage, ThumbButtonRect, m_Color);
					}
					else
					{
						Skin->DrawImage(ui_render, Skin->m_ScrollBarUpBtnDisabledImage, UpButtonRect, m_Color);

						Skin->DrawImage(ui_render, Skin->m_ScrollBarDownBtnDisabledImage, DownButtonRect, m_Color);
					}

					int i = m_ScrollBar.m_nPosition;
					float y = DropdownRect.t + m_Border.y;
					for(; i < (int)m_Items.size() && y <= DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect(Rectangle::LeftTop(DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight));

						if(i == m_iFocused)
						{
							Skin->DrawImage(ui_render, Skin->m_DropdownItemMouseOverImage, ItemRect, m_Color);
						}

						ComboBoxItem * item = m_Items[i].get();
						Rectangle ItemTextRect = ItemRect.shrink(m_Border.x, 0, m_Border.z, 0);
						Skin->DrawString(ui_render, item->strText.c_str(), ItemTextRect, Skin->m_TextColor, Font::AlignLeftMiddle);
					}
				}
				else
				{
					D3DXCOLOR DstColor(m_Color);
					if(m_bMouseOver /*|| m_bHasFocus*/)
					{
						Rect = Rect.offset(-Skin->m_PressedOffset);
					}
					else
					{
						DstColor.a = 0;
					}
					Skin->DrawImage(ui_render, Skin->m_Image, Rect, m_Color);
					D3DXColorLerp(&m_BlendColor, &m_BlendColor, &DstColor, 1.0f - powf(0.8f, 30 * fElapsedTime));
					Skin->DrawImage(ui_render, Skin->m_MouseOverImage, Rect, m_BlendColor);
				}
			}

			Rectangle TextRect = Rect.shrink(m_Border);
			if(m_iSelected >= 0 && m_iSelected < (int)m_Items.size())
				Skin->DrawString(ui_render, m_Items[m_iSelected]->strText.c_str(), TextRect, Skin->m_TextColor, m_Skin->m_TextAlign);
		}
	}
}

bool ComboBox::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ComboBox::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ComboBox::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		if(m_bHasFocus && m_bOpened)
		{
			Vector2 ptLocal = WorldToLocal(pt);
			if(m_ScrollBar.HandleMouse(uMsg, Vector2(ptLocal.x, ptLocal.y - m_Size.y), wParam, lParam))
			{
				// ! overload scrollbars capture
				SetFocus();
				SetCapture();
				return true;
			}
		}

		Rectangle DropdownRect(Rectangle::LeftTop(0, m_Size.y, m_DropdownSize.x, m_DropdownSize.y));

		switch(uMsg)
		{
		case WM_MOUSEMOVE:
			if(m_bHasFocus && m_bOpened)
			{
				Vector2 ptLocal = WorldToLocal(pt);
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
			if(ContainsPoint(pt))
			{
				m_bPressed = true;
				m_bOpened = !m_bOpened;
				SetFocus();
				SetCapture();
				return true;
			}

			if(m_bHasFocus && m_bOpened)
			{
				Vector2 ptLocal = WorldToLocal(pt);
				if(DropdownRect.PtInRect(ptLocal))
				{
					int i = m_ScrollBar.m_nPosition;
					float y = DropdownRect.t;
					for(; i < (int)m_Items.size() && y <= DropdownRect.b - m_ItemHeight; i++, y += m_ItemHeight)
					{
						Rectangle ItemRect(Rectangle::LeftTop(DropdownRect.l, y, m_DropdownSize.x, m_ItemHeight));

						if(ItemRect.PtInRect(ptLocal))
						{
							m_bOpened = false;

							if(m_iSelected != i)
							{
								ReleaseCapture();

								m_iSelected = i;

								if(EventSelectionChanged)
									EventSelectionChanged(&EventArgs());
							}

							break;
						}
					}
					return true;
				}
			}
			break;

		case WM_LBUTTONUP:
			if(m_bPressed && ContainsPoint(pt))
			{
				m_bPressed = false;

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
				else
				{
					if(zDelta > 0)
					{
						if(m_iFocused > 0)
						{
							m_iFocused--;

							if(m_iSelected != m_iFocused)
							{
								m_iSelected = m_iFocused;

								if(EventSelectionChanged)
									EventSelectionChanged(&EventArgs());
							}
						}
					}
					else
					{
						if(m_iFocused + 1 < (int)m_Items.size())
						{
							m_iFocused++;

							if(m_iSelected != m_iFocused)
							{
								m_iSelected = m_iFocused;

								if(EventSelectionChanged)
									EventSelectionChanged(&EventArgs());
							}
						}
					}
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

Control * Dialog::s_FocusControl = NULL;

Control * Dialog::s_CaptureControl = NULL;

Control * Dialog::s_MouseOverControl = NULL;

void Dialog::Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset)
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
		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			if((*ctrl_iter)->GetHotkey() == wParam)
			{
				(*ctrl_iter)->OnHotkey();
				return true;
			}
		}
	}
	return false;
}

bool Dialog::HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam)
{
	if(m_bEnabled && m_bVisible)
	{
		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if(ContainsPoint(pt))
			{
				m_bMouseDrag = true;
				m_MouseOffset = pt - m_Location;
				SetFocus();
				SetCapture();
				return true;
			}
			break;

		case WM_LBUTTONUP:
			if(m_bMouseDrag)
			{
				ReleaseCapture();
				m_bMouseDrag = false;
			}
			break;

		case WM_MOUSEMOVE:
			if(m_bMouseDrag)
			{
				m_Location = pt - m_MouseOffset;
			}
			break;
		}
	}
	return false;
}

void Dialog::SetVisible(bool bVisible)
{
	if(!(m_bVisible = bVisible))
	{
		//ControlPtr ControlFocus = s_ControlFocus.lock();
		//if(ControlFocus && ContainsControl(ControlFocus))
		//{
		//	ControlFocus->OnFocusOut();

		//	s_ControlFocus.reset();
		//}
	}
	else
	{
		//ForceFocusControl();

		Refresh();
	}
}

void Dialog::Refresh(void)
{
	ControlPtrList::iterator ctrl_iter = m_Childs.begin();
	for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
	{
		(*ctrl_iter)->Refresh();
	}

	if(EventRefresh)
		EventRefresh(&EventArgs());
}

bool Dialog::RayToWorld(const std::pair<Vector3, Vector3> & ray, Vector2 & ptWorld)
{
	Vector3 dialogNormal = m_World[2].xyz.normalize();
	float dialogDist = m_World[3].xyz.dot(dialogNormal);
	IntersectionTests::TestResult result = IntersectionTests::rayAndHalfSpace(ray.first, ray.second, Plane::NormalDistance(dialogNormal, dialogDist));

	if (result.first)
	{
		Vector3 ptInt(ray.first + ray.second * result.second);
		ptWorld = ptInt.transformCoord(m_World.inverse()).xy;
		return true;
	}
	return false;
}

void DialogMgr::SetDlgViewport(const Vector2 & vp, float fov)
{
	m_ViewPosition = Vector3(vp.x * 0.5f, vp.y * 0.5f, -vp.y * 0.5f * cot(fov / 2));

	m_View = Matrix4::LookAtRH(m_ViewPosition, Vector3(m_ViewPosition.x, m_ViewPosition.y, 0), Vector3(0, -1, 0));

	m_Proj = Matrix4::PerspectiveFovRH(fov, vp.x / vp.y, 0.1f, 3000.0f);

	m_ViewProj = m_View * m_Proj;

	m_InverseViewProj = m_ViewProj.inverse();

	DialogPtrList::iterator dlg_iter = m_DlgList.begin();
	for(; dlg_iter != m_DlgList.end(); dlg_iter++)
	{
		if((*dlg_iter)->EventAlign)
			(*dlg_iter)->EventAlign(&EventArgs());
	}
}

std::pair<Vector3, Vector3> DialogMgr::CalculateRay(const Vector2 & pt, const CSize & dim)
{
	return IntersectionTests::CalculateRay(m_InverseViewProj, m_ViewPosition, pt, Vector2((float)dim.cx, (float)dim.cy));
}

Vector2 DialogMgr::GetDlgViewport(void) const
{
	return Vector2(-m_View._41*2, m_View._42*2);
}

void DialogMgr::Draw(UIRender * ui_render, double fTime, float fElapsedTime)
{
	ui_render->SetViewProj(m_ViewProj);

	DialogPtrList::reverse_iterator dlg_iter = m_DlgList.rbegin();
	for(; dlg_iter != m_DlgList.rend(); dlg_iter++)
	{
		ui_render->SetWorld((*dlg_iter)->m_World);

		(*dlg_iter)->Draw(ui_render, fElapsedTime);
	}
}

bool DialogMgr::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Dialog::s_FocusControl) {
		if (Dialog::s_FocusControl->MsgProc(hWnd, uMsg, wParam, lParam)) {
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
			if (Dialog::s_FocusControl) {
				if (Dialog::s_FocusControl->HandleKeyboard(uMsg, wParam, lParam)) {
					return true;
				}
			}

			if (uMsg == WM_KEYDOWN)
			{
				DialogPtrList::iterator dlg_iter = m_DlgList.begin();
				for(; dlg_iter != m_DlgList.end(); dlg_iter++)
				{
					if((*dlg_iter)->HandleKeyboard(uMsg, wParam, lParam))
						return true;
				}
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
			std::pair<Vector3, Vector3> ray = CalculateRay(Vector2((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f), ClientRect.Size());

			if (Dialog::s_CaptureControl) {
				Vector2 pt;
				if (Dialog::s_CaptureControl->RayToWorld(ray, pt))
				{
					if (Dialog::s_CaptureControl->HandleMouse(uMsg, pt, wParam, lParam)) {
						return true;
					}
				}
			}

			DialogPtrList::iterator dlg_iter = m_DlgList.begin();
			for(; dlg_iter != m_DlgList.end(); dlg_iter++)
			{
				// ! 只处理看得见的 Dialog
				if((*dlg_iter)->GetVisible())
				{
					Vector2 pt;
					if ((*dlg_iter)->RayToWorld(ray, pt))
					{
						if ((*dlg_iter)->ContainsPoint(pt))
						{
							Control * ControlPtd = (*dlg_iter)->GetChildAtPoint(pt);
							if (ControlPtd)
							{
								ControlPtd->SetMouseOver();

								if(ControlPtd->HandleMouse(uMsg, pt, wParam, lParam))
								{
									return true;
								}
							}
							else
							{
								(*dlg_iter)->SetMouseOver();
							}

							if((*dlg_iter)->HandleMouse(uMsg, pt, wParam, lParam))
							{
								return true;
							}
							break;
						}
					}
				}
			}

			if (dlg_iter == m_DlgList.end() && Dialog::s_MouseOverControl)
			{
				Dialog::s_MouseOverControl->ReleaseMouseOver();
			}
		}
		break;
	}

	return false;
}

void DialogMgr::InsertDlg(DialogPtr dlg)
{
	m_DlgList.push_back(dlg);

	if(dlg->EventAlign)
		dlg->EventAlign(&EventArgs());
}

void DialogMgr::RemoveDlg(DialogPtr dlg)
{
	DialogPtrList::iterator dlg_iter = std::find(m_DlgList.begin(), m_DlgList.end(), dlg);
	if(dlg_iter != m_DlgList.end())
	{
		m_DlgList.erase(dlg_iter);
	}
}

void DialogMgr::RemoveAllDlg()
{
	m_DlgList.clear();
}

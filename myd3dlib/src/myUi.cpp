
#include "stdafx.h"
#include "myd3dlib.h"
#include <ImeUi.h>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

void UIRender::BuildOrthoMatrices(DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj)
{
	outView = Matrix4::LookAtLH(
		Vector3(Width * 0.5f, Height * 0.5f, 1),
		Vector3(Width * 0.5f, Height * 0.5f, 0),
		Vector3(0, -1, 0));

	outProj = Matrix4::OrthoLH((float)Width, (float)Height, -50.0f, 50.0f);
}

void UIRender::BuildPerspectiveMatrices(float fovy, DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj)
{
	float Dist = Height * 0.5f * cot(fovy / 2);

	outView = Matrix4::LookAtLH(
		Vector3(Width * 0.5f, Height * 0.5f, Dist),
		Vector3(Width * 0.5f, Height * 0.5f, 0),
		Vector3(0, -1, 0));

	outProj = Matrix4::PerspectiveFovLH(fovy, (float)Width / Height, 0.1f, 3000.0f);
}

void UIRender::Begin(IDirect3DDevice9 * pd3dDevice)
{
	ResourceMgr::getSingleton().m_stateBlock->Capture();

	HRESULT hr;
	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	V(pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE));

	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	V(pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));

	V(pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
	V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
	V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
	V(pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
	V(pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	V(pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
}

void UIRender::End(IDirect3DDevice9 * pd3dDevice)
{
	ResourceMgr::getSingleton().m_stateBlock->Apply();
}

my::Rectangle UIRender::CalculateUVRect(const SIZE & textureSize, const RECT & textureRect)
{
	return Rectangle(
		(float)textureRect.left / textureSize.cx,
		(float)textureRect.top / textureSize.cy,
		(float)textureRect.right / textureSize.cx,
		(float)textureRect.bottom / textureSize.cy);
}

// ! Floor UI unit & subtract 0.5 units to correctly align texels with pixels
#define ALIGN_UI_UNIT(v) (floor(v) - 0.5f)

size_t UIRender::BuildRectangleVertices(
	CUSTOMVERTEX * pBuffer,
	size_t bufferSize,
	const my::Rectangle & rect,
	DWORD color,
	const my::Rectangle & uvRect)
{
	if(bufferSize >= 6)
	{
		pBuffer[0].x = ALIGN_UI_UNIT(rect.l);
		pBuffer[0].y = ALIGN_UI_UNIT(rect.t);
		pBuffer[0].z = 0;
		pBuffer[0].color = color;
		pBuffer[0].u = uvRect.l;
		pBuffer[0].v = uvRect.t;

		pBuffer[1].x = ALIGN_UI_UNIT(rect.r);
		pBuffer[1].y = ALIGN_UI_UNIT(rect.t);
		pBuffer[1].z = 0;
		pBuffer[1].color = color;
		pBuffer[1].u = uvRect.r;
		pBuffer[1].v = uvRect.t;

		pBuffer[2].x = ALIGN_UI_UNIT(rect.l);
		pBuffer[2].y = ALIGN_UI_UNIT(rect.b);
		pBuffer[2].z = 0;
		pBuffer[2].color = color;
		pBuffer[2].u = uvRect.l;
		pBuffer[2].v = uvRect.b;

		pBuffer[3].x = ALIGN_UI_UNIT(rect.r);
		pBuffer[3].y = ALIGN_UI_UNIT(rect.b);
		pBuffer[3].z = 0;
		pBuffer[3].color = color;
		pBuffer[3].u = uvRect.r;
		pBuffer[3].v = uvRect.b;

		pBuffer[4] = pBuffer[2];
		pBuffer[5] = pBuffer[1];

		return 6;
	}
	return 0;
}

void UIRender::DrawRectangle(
	IDirect3DDevice9 * pd3dDevice,
	const my::Rectangle & rect,
	DWORD color,
	const my::Rectangle & uvRect)
{
	CUSTOMVERTEX vertex_list[6];
	size_t vertNum = BuildRectangleVertices(vertex_list, _countof(vertex_list), rect, color, uvRect);

	//Begin(pd3dDevice);

	HRESULT hr;
	V(pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertNum / 3, vertex_list, sizeof(*vertex_list)));

	//End(pd3dDevice);
}

void UIRender::DrawRectangle(
	IDirect3DDevice9 * pd3dDevice,
	const my::Rectangle & rect,
	DWORD color)
{
	CUSTOMVERTEX vertex_list[6];
	size_t vertNum = BuildRectangleVertices(vertex_list, _countof(vertex_list), rect, color, Rectangle(0,0,1,1));

	//Begin(pd3dDevice);

    //pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
    //pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );

	HRESULT hr;
	V(pd3dDevice->SetTexture(0, NULL));
	V(pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX));
	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertNum / 3, vertex_list, sizeof(*vertex_list)));

	//V(pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
	//V(pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));

	//End(pd3dDevice);
}

void Control::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_bVisible)
	{
		if(m_Skin && m_Skin->m_Texture)
		{
			D3DSURFACE_DESC desc = m_Skin->m_Texture->GetLevelDesc();
			CSize TextureSize(desc.Width, desc.Height);
			V(pd3dDevice->SetTexture(0, m_Skin->m_Texture->m_ptr));
			UIRender::DrawRectangle(
				pd3dDevice, Rectangle::LeftTop(m_Location, m_Size), m_Color, UIRender::CalculateUVRect(TextureSize, m_Skin->m_TextureRect));
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

bool Control::OnHotkey(void)
{
	return false;
}

bool Control::ContainsPoint(const Vector2 & pt)
{
	return pt.x >= m_Location.x && pt.x < m_Location.x + m_Size.x && pt.y >= m_Location.y && pt.y < m_Location.y + m_Size.y;
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

void Static::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_bVisible)
	{
		if(m_Skin && m_Skin->m_Font)
		{
			m_Skin->m_Font->DrawString(m_Text.c_str(), Rectangle::LeftTop(m_Location, m_Size), m_Skin->m_TextColor, m_Skin->m_TextAlign);
		}
	}
}

bool Static::ContainsPoint(const Vector2 & pt)
{
	return false;
}

void Button::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_bVisible)
	{
		ButtonSkinPtr Skin = boost::dynamic_pointer_cast<ButtonSkin, ControlSkin>(m_Skin);

		Rectangle Rect(Rectangle::LeftTop(m_Location, m_Size));

		if(Skin && Skin->m_Texture)
		{
			D3DSURFACE_DESC desc = Skin->m_Texture->GetLevelDesc();
			CSize TextureSize(desc.Width, desc.Height);
			V(pd3dDevice->SetTexture(0, Skin->m_Texture->m_ptr));

			if(!m_bEnabled)
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_DisabledTexRect));
			}
			else if(m_bPressed)
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_PressedTexRect));
			}
			else if(m_bMouseOver)
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_MouseOverTexRect));
			}
			else
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_TextureRect));
			}
		}

		if(Skin && Skin->m_Font)
		{
			if(m_bPressed)
			{
				Skin->m_Font->DrawString(m_Text.c_str(), Rect.offset(Skin->m_PressedOffset), Skin->m_TextColor, Skin->m_TextAlign);
			}
			else
			{
				Skin->m_Font->DrawString(m_Text.c_str(), Rect, Skin->m_TextColor, Skin->m_TextAlign);
			}
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
						EventClick(this_ptr.lock());
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

				// ! DXUT dependency
				SetCapture(DXUTGetHWND());

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
						EventClick(this_ptr.lock());
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

bool Button::ContainsPoint(const Vector2 & pt)
{
	return Control::ContainsPoint(pt);
}

void EditBox::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_bVisible)
	{
		EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin, ControlSkin>(m_Skin);

		Rectangle Rect(Rectangle::LeftTop(m_Location, m_Size));

		if(Skin && Skin->m_Texture)
		{
			D3DSURFACE_DESC desc = Skin->m_Texture->GetLevelDesc();
			CSize TextureSize(desc.Width, desc.Height);
			V(pd3dDevice->SetTexture(0, Skin->m_Texture->m_ptr));

			if(!m_bEnabled)
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_DisabledTexRect));
			}
			else if(m_bHasFocus)
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_FocusedTexRect));
			}
			else
			{
				UIRender::DrawRectangle(
					pd3dDevice, Rect, m_Color, UIRender::CalculateUVRect(TextureSize, Skin->m_TextureRect));
			}
		}

		if(DXUTGetGlobalTimer()->GetAbsoluteTime() - m_dfLastBlink >= m_dfBlink )
		{
			// ! DXUT dependency
			m_bCaretOn = !m_bCaretOn;
			m_dfLastBlink = DXUTGetGlobalTimer()->GetAbsoluteTime();
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

				UIRender::DrawRectangle(pd3dDevice, SelRect, Skin->m_SelBkColor);
			}

			Skin->m_Font->DrawString(m_Text.c_str() + m_nFirstVisible, TextRect, Skin->m_TextColor, Font::AlignLeftMiddle);

			if(m_bHasFocus && m_bCaretOn && !ImeEditBox::s_bHideCaret)
			{
				Rectangle CaretRect(
					TextRect.l + caret_x - x1st - 1,
					TextRect.t,
					TextRect.l + caret_x - x1st + 1,
					TextRect.b);

				if(!m_bInsertMode)
				{
					int charWidth;
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

				UIRender::DrawRectangle(pd3dDevice, CaretRect, Skin->m_CaretColor);
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
							EventChange(this_ptr.lock());
					}
					else if(m_nCaret > 0)
					{
						PlaceCaret(m_nCaret - 1);
						m_nSelStart = m_nCaret;
						m_Text.erase(m_nCaret, 1);
						if(EventChange)
							EventChange(this_ptr.lock());
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
							EventChange(this_ptr.lock());
					}
					break;

				case 22:		// Ctrl-V Paste
					PasteFromClipboard();
					if(EventChange)
						EventChange(this_ptr.lock());
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
						EventEnter(this_ptr.lock());
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
						EventChange(this_ptr.lock());
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
						EventChange(this_ptr.lock());
				}
				else
				{
					m_Text.erase(m_nCaret, 1);
					if(EventChange)
						EventChange(this_ptr.lock());
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

				// ! DXUT dependency
				SetCapture(DXUTGetHWND());

				if(m_Skin && m_Skin->m_Font)
				{
					float x1st = m_Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
					float x = pt.x - m_Location.x - m_Border.x + x1st;
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

						m_nSelStart = m_nCaret;
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
					float x1st = m_Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
					float x = pt.x - m_Location.x - m_Border.x + x1st;
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

		if(x < x1st)
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
	// ! DXUT dependency
	m_bCaretOn = true;
	m_dfLastBlink = DXUTGetGlobalTimer()->GetAbsoluteTime();
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

void ImeEditBox::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_bVisible)
	{
		EditBox::OnRender(pd3dDevice, fElapsedTime);

	    ImeUi_RenderUI();

		if(m_bHasFocus)
		{
			RenderIndicator(pd3dDevice, fElapsedTime);

			RenderComposition(pd3dDevice, fElapsedTime);

			if(ImeUi_IsShowCandListWindow())
				RenderCandidateWindow(pd3dDevice, fElapsedTime);
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
			EditBox::MsgProc(hWnd, uMsg, wParam, lParam);

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

void ImeEditBox::RenderIndicator(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
}

void ImeEditBox::RenderComposition(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin, ControlSkin>(m_Skin);
	if(Skin)
	{
		s_CompString = ImeUi_GetCompositionString();

		Rectangle Rect(Rectangle::LeftTop(m_Location, m_Size));

		Rectangle TextRect = Rect.shrink(m_Border);

		float x, x1st;
		x = Skin->m_Font->CPtoX(m_Text.c_str(), m_nCaret);
		x1st = Skin->m_Font->CPtoX(m_Text.c_str(), m_nFirstVisible);
		Vector2 extent = Skin->m_Font->CalculateStringExtent(s_CompString.c_str());

		Rectangle rc(TextRect.l + x - x1st, TextRect.t, TextRect.l + x - x1st + extent.x, TextRect.b);
		if(rc.r > TextRect.r)
			rc.offsetSelf(TextRect.l - rc.l, TextRect.Height());

		UIRender::DrawRectangle(pd3dDevice, rc, m_CompWinColor);

		Skin->m_Font->DrawString(s_CompString.c_str(), rc, Skin->m_TextColor, Font::AlignLeftTop);

		float caret_x = Skin->m_Font->CPtoX(s_CompString.c_str(), ImeUi_GetImeCursorChars());
		if(m_bCaretOn)
		{
			Rectangle CaretRect(rc.l + caret_x - 1, rc.t, rc.l + caret_x + 1, rc.b);

			UIRender::DrawRectangle(pd3dDevice, CaretRect, Skin->m_CaretColor);
		}
	}
}

void ImeEditBox::RenderCandidateWindow(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	EditBoxSkinPtr Skin = boost::dynamic_pointer_cast<EditBoxSkin, ControlSkin>(m_Skin);
	if(Skin)
	{
		Rectangle Rect(Rectangle::LeftTop(m_Location, m_Size));

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
			horizontalText += ImeUi_GetCandidate(i);
		}
		extent = Skin->m_Font->CalculateStringExtent(horizontalText.c_str());

		Rectangle CandRect(Rectangle::LeftTop(CompRect.l + comp_x, CompRect.b, extent.x, (float)Skin->m_Font->m_LineHeight));

		UIRender::DrawRectangle(pd3dDevice, CandRect, m_CandidateWinColor);

		Skin->m_Font->DrawString(horizontalText.c_str(), CandRect, Skin->m_TextColor, Font::AlignLeftTop);
	}
}

void Dialog::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	//V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_Transform));
	//V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_ViewMatrix));
	//V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_ProjMatrix));

	if(m_bVisible)
	{
		Control::OnRender(pd3dDevice, fElapsedTime);

		ControlPtrList::iterator ctrl_iter = m_Controls.begin();
		for(; ctrl_iter != m_Controls.end(); ctrl_iter++)
		{
			(*ctrl_iter)->OnRender(pd3dDevice, fElapsedTime);
		}
	}
}

bool Dialog::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ControlPtr ControlFocus = ResourceMgr::getSingleton().m_ControlFocus.lock();

	if(ControlFocus && ControlFocus->GetEnabled()
		&& ControlFocus->MsgProc(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if(ControlFocus && ControlFocus->GetEnabled())
		{
			if(ControlFocus->HandleKeyboard(uMsg, wParam, lParam))
				return true;
		}

		if(uMsg == WM_KEYDOWN && !ControlFocus)
		{
			ControlPtrList::iterator ctrl_iter = m_Controls.begin();
			for(; ctrl_iter != m_Controls.end(); ctrl_iter++)
			{
				if((*ctrl_iter)->OnHotkey())
				{
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
			Matrix4 invViewMatrix = m_ViewMatrix.inverse();
			const Vector3 & viewX = invViewMatrix[0];
			const Vector3 & viewY = invViewMatrix[1];
			const Vector3 & viewZ = invViewMatrix[2];
			const Vector3 & ptEye = invViewMatrix[3];

			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			Vector2 ptScreen((short)LOWORD(lParam) + 0.5f, (short)HIWORD(lParam) + 0.5f);
			Vector2 ptProj(Lerp(-1.0f, 1.0f, ptScreen.x / ClientRect.right) / m_ProjMatrix._11, Lerp(1.0f, -1.0f, ptScreen.y / ClientRect.bottom) / m_ProjMatrix._22);
			Vector3 dir = (viewX * ptProj.x + viewY * ptProj.y + viewZ).normalize();

			Vector3 dialogNormal = Vector3(0, 0, 1).transformNormal(m_Transform);
			float dialogDistance = ((Vector3 &)m_Transform[3]).dot(dialogNormal);
			IntersectionTests::TestResult result = IntersectionTests::rayAndHalfSpace(ptEye, dir, dialogNormal, dialogDistance);

			if(result.first)
			{
				Vector3 ptInt(ptEye + dir * result.second);
				Vector3 pt = ptInt.transformCoord(m_Transform.inverse());
				if(ControlFocus && ControlFocus->GetEnabled())
				{
					if(ControlFocus->HandleMouse(uMsg, pt, wParam, lParam))
						return true;
				}

				ControlPtr ControlPtd = GetControlAtPoint(pt);
				if(ControlPtd && ControlPtd->GetEnabled())
				{
					if(ControlPtd->HandleMouse(uMsg, pt, wParam, lParam))
					{
						RequestFocus(ControlPtd);
						return true;
					}
				}
				else if(uMsg == WM_LBUTTONDOWN && ControlFocus)
				{
					ControlFocus->OnFocusOut();
					ResourceMgr::getSingleton().m_ControlFocus.reset();
				}

				if(ControlPtd != m_ControlMouseOver)
				{
					if(m_ControlMouseOver)
						m_ControlMouseOver->OnMouseLeave();

					m_ControlMouseOver = ControlPtd;
					if(ControlPtd)
						ControlPtd->OnMouseEnter();
				}
			}
		}
		break;
	}
	return false;
}

ControlPtr Dialog::GetControlAtPoint(const Vector2 & pt)
{
	ControlPtrList::iterator ctrl_iter = m_Controls.begin();
	for(; ctrl_iter != m_Controls.end(); ctrl_iter++)
	{
		if((*ctrl_iter)->ContainsPoint(pt) && (*ctrl_iter)->GetVisible() && (*ctrl_iter)->GetEnabled())
		{
			return *ctrl_iter;
		}
	}
	return ControlPtr();
}

void Dialog::RequestFocus(ControlPtr control)
{
	if(!control->CanHaveFocus())
		return;

	ControlPtr ControlFocus = ResourceMgr::getSingleton().m_ControlFocus.lock();
	if(ControlFocus)
	{
		if(ControlFocus == control)
			return;

		ControlFocus->OnFocusOut();
	}

	control->OnFocusIn();
	ResourceMgr::getSingleton().m_ControlFocus = control;
}

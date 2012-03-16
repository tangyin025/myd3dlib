
#include "stdafx.h"
#include "myd3dlib.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

void UIRender::BuildOrthoMatrices(DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj)
{
	outView = Matrix4::LookAtLH(
		my::Vector3(Width * 0.5f, Height * 0.5f, 1),
		my::Vector3(Width * 0.5f, Height * 0.5f, 0),
		my::Vector3(0, -1, 0));

	outProj = Matrix4::OrthoLH((float)Width, (float)Height, -50.0f, 50.0f);
}

void UIRender::BuildPerspectiveMatrices(float fovy, DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj)
{
	float Dist = Height * 0.5f * cot(fovy / 2);

	outView = Matrix4::LookAtLH(
		my::Vector3(Width * 0.5f, Height * 0.5f, Dist),
		my::Vector3(Width * 0.5f, Height * 0.5f, 0),
		my::Vector3(0, -1, 0));

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

// ! Floor UI unit & subtract 0.5 units to correctly align texels with pixels
#define ALIGN_UI_UNIT(v) (floor(v) - 0.5f)

size_t UIRender::BuildRectangleVertices(
	CUSTOMVERTEX * pBuffer,
	size_t bufferSize,
	const my::Rectangle & rect,
	const my::Rectangle & uvRect,
	DWORD color)
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
	const my::Rectangle & uvRect,
	DWORD color)
{
	CUSTOMVERTEX vertex_list[6];
	size_t vertNum = BuildRectangleVertices(vertex_list, _countof(vertex_list), rect, uvRect, color);

	Begin(pd3dDevice);

	HRESULT hr;
	V(pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX));

	V(pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertNum / 3, vertex_list, sizeof(*vertex_list)));

	End(pd3dDevice);
}

void Control::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	if(m_bVisible)
	{
		if(m_Color & D3DCOLOR_ARGB(255,0,0,0) && m_Skin && m_Skin->m_Texture)
		{
			V(pd3dDevice->SetTexture(0, m_Skin->m_Texture->m_ptr));
			UIRender::DrawRectangle(pd3dDevice, my::Rectangle::LeftTop(m_Location, m_Size), m_Skin->m_TextureUV, m_Color);
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
			m_Skin->m_Font->DrawString(m_Text.c_str(), my::Rectangle::LeftTop(m_Location, m_Size), m_Skin->m_TextColor, m_Skin->m_TextAlign);
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

		my::Rectangle Rect(my::Rectangle::LeftTop(m_Location, m_Size));

		if(Skin && Skin->m_Texture)
		{
			V(pd3dDevice->SetTexture(0, Skin->m_Texture->m_ptr));
			if(!m_bEnabled)
			{
				UIRender::DrawRectangle(pd3dDevice, Rect, Skin->m_DisabledTexUV, m_Color);
			}
			else if(m_bPressed)
			{
				UIRender::DrawRectangle(pd3dDevice, Rect, Skin->m_PressedTexUV, m_Color);
			}
			else if(m_bMouseOver)
			{
				UIRender::DrawRectangle(pd3dDevice, Rect, Skin->m_MouseOverTexUV, m_Color);
			}
			else
			{
				UIRender::DrawRectangle(pd3dDevice, Rect, Skin->m_TextureUV, m_Color);
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
						EventClick(this);
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
				m_bPressed = false;
				ReleaseCapture();

				if(ContainsPoint(pt))
				{
					if(EventClick)
						EventClick(this);
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

void Dialog::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	HRESULT hr;
	V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&m_World));
	V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&m_View));
	V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&m_Proj));

	ControlPtrList::iterator ctrl_iter = m_Controls.begin();
	for(; ctrl_iter != m_Controls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->OnRender(pd3dDevice, fElapsedTime);
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
			Vector2 pt(LOWORD(lParam), HIWORD(lParam));
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

			if(ControlPtd != m_ControlMouseOver)
			{
				if(m_ControlMouseOver)
					m_ControlMouseOver->OnMouseLeave();

				m_ControlMouseOver = ControlPtd;
				if(ControlPtd)
					ControlPtd->OnMouseEnter();
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

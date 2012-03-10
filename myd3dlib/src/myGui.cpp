
#include "stdafx.h"
#include "myd3dlib.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

void UIRender::Begin(IDirect3DDevice9 * pd3dDevice)
{
	ResourceMgr::getSingleton().m_stateBlock->Capture();

	HRESULT hr;

	//D3DVIEWPORT9 vp;
	//V(pd3dDevice->GetViewport(&vp));
	//// subtract 0.5 units to correctly align texels with pixels
	//V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::Translation(my::Vector3(-0.5f, -0.5f, 0.0f))));
	//V(pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&Matrix4::LookAtLH(my::Vector3(0,0,1), my::Vector3(0,0,0), my::Vector3(0,-1,0))));
	//V(pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&Matrix4::OrthoOffCenterLH(0, vp.Width, -(float)vp.Height, 0, -50, 50)));

	V(pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	V(pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	V(pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	V(pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	V(pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

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

size_t UIRender::BuildRectangleVertices(
	CUSTOMVERTEX * pBuffer,
	size_t bufferSize,
	const my::Rectangle & rect,
	const my::Rectangle & uvRect,
	DWORD color)
{
	if(bufferSize >= 6)
	{
		pBuffer[0].x = rect.l;
		pBuffer[0].y = rect.t;
		pBuffer[0].z = 0;
		pBuffer[0].color = color;
		pBuffer[0].u = uvRect.l;
		pBuffer[0].v = uvRect.t;

		pBuffer[1].x = rect.r;
		pBuffer[1].y = rect.t;
		pBuffer[1].z = 0;
		pBuffer[1].color = color;
		pBuffer[1].u = uvRect.r;
		pBuffer[1].v = uvRect.t;

		pBuffer[2].x = rect.l;
		pBuffer[2].y = rect.b;
		pBuffer[2].z = 0;
		pBuffer[2].color = color;
		pBuffer[2].u = uvRect.l;
		pBuffer[2].v = uvRect.b;

		pBuffer[3].x = rect.r;
		pBuffer[3].y = rect.b;
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

void UIControl::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		my::Vector2 Location = Offset + m_Location;

		if(m_Color & D3DCOLOR_ARGB(255,0,0,0) && m_Elem)
		{
			V(pd3dDevice->SetTexture(0, m_Elem->m_Texture->m_ptr));
			UIRender::DrawRectangle(pd3dDevice, my::Rectangle::LeftTop(Location, m_Size), m_Elem->m_TextureUV, m_Color);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->OnRender(pd3dDevice, fElapsedTime, Location);
		}
	}
}

bool UIControl::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool UIControl::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool UIControl::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool UIControl::CanHaveFocus(void)
{
	return false;
}

void UIControl::OnFocusIn(void)
{
	m_bHasFocus = false;
}

void UIControl::OnMouseEnter(void)
{
	m_bMouseOver = true;
}

void UIControl::OnMouseLeave(void)
{
	m_bMouseOver = false;
}

void UIControl::OnHotkey(void)
{
}

bool UIControl::ContainsPoint(const Vector2 & pt)
{
	return false;
}

void UIControl::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
}

bool UIControl::GetEnabled(void)
{
	return m_bEnabled;
}

void UIControl::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
}

bool UIControl::GetVisible(void)
{
	return m_bVisible;
}

void UIStatic::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset)
{
	if(m_bVisible)
	{
		my::Vector2 Location = Offset + m_Location;
		my::Rectangle Rect(my::Rectangle::LeftTop(Location, m_Size));

		if(m_Color & D3DCOLOR_ARGB(255,0,0,0) && m_Elem)
		{
			V(pd3dDevice->SetTexture(0, m_Elem->m_Texture->m_ptr));
			UIRender::DrawRectangle(pd3dDevice, Rect, m_Elem->m_TextureUV, m_Color);
		}

		if(m_Elem && m_Elem->m_Font)
		{
			m_Elem->m_Font->DrawString(m_Text.c_str(), Rect, m_TextColor, m_TextAlign);
		}

		ControlPtrList::iterator ctrl_iter = m_Childs.begin();
		for(; ctrl_iter != m_Childs.end(); ctrl_iter++)
		{
			(*ctrl_iter)->OnRender(pd3dDevice, fElapsedTime, Location);
		}
	}
}

bool UIStatic::ContainsPoint(const Vector2 & pt)
{
	return false;
}

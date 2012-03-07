
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
	//V(pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&Matrix4::identity));
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
		// subtract 0.5 units to correctly align texels with pixels
		pBuffer[0].x = rect.l - 0.5f;
		pBuffer[0].y = rect.t - 0.5f;
		pBuffer[0].z = 0;
		pBuffer[0].color = color;
		pBuffer[0].u = uvRect.l;
		pBuffer[0].v = uvRect.t;

		pBuffer[1].x = rect.r - 0.5f;
		pBuffer[1].y = rect.t - 0.5f;
		pBuffer[1].z = 0;
		pBuffer[1].color = color;
		pBuffer[1].u = uvRect.r;
		pBuffer[1].v = uvRect.t;

		pBuffer[2].x = rect.l - 0.5f;
		pBuffer[2].y = rect.b - 0.5f;
		pBuffer[2].z = 0;
		pBuffer[2].color = color;
		pBuffer[2].u = uvRect.l;
		pBuffer[2].v = uvRect.b;

		pBuffer[3].x = rect.r - 0.5f;
		pBuffer[3].y = rect.b - 0.5f;
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

void UIElement::OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	UIRender::DrawRectangle(pd3dDevice, my::Rectangle::LeftTop(100,100,100,100), my::Rectangle(0,0,1,1), D3DCOLOR_ARGB(255,255,255,255));
}

bool UIElement::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return false;
}

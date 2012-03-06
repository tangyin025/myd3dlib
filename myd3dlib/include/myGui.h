
#pragma once

namespace my
{
	class UIElement
	{
	public:
		struct CUSTOMVERTEX
		{
			FLOAT x, y, z;
			DWORD color;
			FLOAT u, v;
		};

		static const DWORD D3DFVF_CUSTOMVERTEX = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

		static size_t BuildQuadrangleVertices(
			CUSTOMVERTEX * pBuffer,
			size_t bufferSize,
			const Rectangle & rect,
			DWORD color,
			const Rectangle & uvRect);

		static void Begin(IDirect3DDevice9 * pd3dDevice);

		static void End(IDirect3DDevice9 * pd3dDevice);

	public:
		UIElement(void)
		{
		}

		virtual ~UIElement(void)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}

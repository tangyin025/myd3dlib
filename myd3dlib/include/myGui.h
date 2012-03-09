
#pragma once

namespace my
{
	class UIRender
	{
	public:
		struct CUSTOMVERTEX
		{
			FLOAT x, y, z;
			DWORD color;
			FLOAT u, v;
		};

		static const DWORD D3DFVF_CUSTOMVERTEX = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

		// Rendering UI under Fixed Pipeline is not recommended
		static void Begin(IDirect3DDevice9 * pd3dDevice);

		static void End(IDirect3DDevice9 * pd3dDevice);

		static size_t BuildRectangleVertices(
			CUSTOMVERTEX * pBuffer,
			size_t bufferSize,
			const Rectangle & rect,
			const Rectangle & uvRect,
			DWORD color);

		// Example of Draw BuildRectangleVertices
		static void DrawRectangle(
			IDirect3DDevice9 * pd3dDevice,
			const Rectangle & rect,
			const Rectangle & uvRect,
			DWORD color);
	};

	class UIElement
	{
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

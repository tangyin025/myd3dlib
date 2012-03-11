
#pragma once

#include "FastDelegate.h"

namespace my
{
	class UIRender
	{
	public:
		typedef Font::CUSTOMVERTEX CUSTOMVERTEX;

		static const DWORD D3DFVF_CUSTOMVERTEX = Font::D3DFVF_CUSTOMVERTEX;

		// Resolution-Dependent UI Transforms
		static void SetupOrthoMatrices(IDirect3DDevice9 * pd3dDevice, DWORD Width, DWORD Height);

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

	class UIControlSkin
	{
	public:
		TexturePtr m_Texture;

		Rectangle m_TextureUV;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

	public:
		UIControlSkin(void)
			: m_TextureUV(0,0,1,1)
			, m_TextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_TextAlign(my::Font::AlignLeftTop)
		{
		}

		virtual ~UIControlSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<UIControlSkin> UIControlSkinPtr;

	class UIControl
	{
	public:
		bool m_bEnabled;

		bool m_bVisible;

		bool m_bMouseOver;

		bool m_bHasFocus;

		bool m_bIsDefault;

		Vector2 m_Location;

		Vector2 m_Size;

		D3DCOLOR m_Color;

		UIControlSkinPtr m_Skin;

		HRESULT hr;

	public:
		UIControl(void)
			: m_bEnabled(true)
			, m_bVisible(true)
			, m_bMouseOver(false)
			, m_bHasFocus(false)
			, m_bIsDefault(false)
			, m_Location(100, 100)
			, m_Size(100, 100)
			, m_Color(D3DCOLOR_ARGB(0,0,0,0))
		{
		}

		virtual ~UIControl(void)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual void OnMouseEnter(void);

		virtual void OnMouseLeave(void);

		virtual void OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void SetEnabled(bool bEnabled);

		virtual bool GetEnabled(void);

		virtual void SetVisible(bool bVisible);

		virtual bool GetVisible(void);
	};

	typedef boost::shared_ptr<UIControl> UIControlPtr;

	class UIStatic : public UIControl
	{
	public:
		std::wstring m_Text;

	public:
		UIStatic(void)
			: m_Text(L"")
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<UIStatic> UIStaticPtr;

	class UIButtonSkin : public UIControlSkin
	{
	public:
		Rectangle m_DisabledTexUV;

		Rectangle m_PressedTexUV;

		Rectangle m_MouseOverTexUV;

	public:
		UIButtonSkin(void)
			: m_DisabledTexUV(0,0,1,1)
			, m_PressedTexUV(0,0,1,1)
			, m_MouseOverTexUV(0,0,1,1)
		{
		}
	};

	typedef boost::shared_ptr<UIButtonSkin> UIButtonSkinPtr;

	typedef fastdelegate::FastDelegate1<UIControl *> UIEvent;

	class UIButton : public UIStatic
	{
	public:
		bool m_bPressed;

		UIEvent EventClick;

	public:
		UIButton(void)
			: m_bPressed(false)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<UIButton> UIButtonPtr;
}

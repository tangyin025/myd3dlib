
#pragma once

#include "FastDelegate.h"

namespace my
{
	class UIRender
	{
	public:
		typedef Font::CUSTOMVERTEX CUSTOMVERTEX;

		static const DWORD D3DFVF_CUSTOMVERTEX = Font::D3DFVF_CUSTOMVERTEX;

		static void BuildOrthoMatrices(DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj);

		static void BuildPerspectiveMatrices(float fovy, DWORD Width, DWORD Height, Matrix4 & outView, Matrix4 & outProj);

		// Rendering UI under Fixed Pipeline is not recommended
		static void Begin(IDirect3DDevice9 * pd3dDevice);

		static void End(IDirect3DDevice9 * pd3dDevice);

		static Rectangle CalculateUVRect(const SIZE & textureSize, const RECT & textureRect);

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

	class ControlSkin
	{
	public:
		TexturePtr m_Texture;

		SIZE m_TextureSize;

		RECT m_TextureRect;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

	public:
		ControlSkin(void)
			: m_TextureRect(CRect(0,0,0,0))
			, m_TextureSize(CSize(0,0))
			, m_TextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_TextAlign(my::Font::AlignLeftTop)
		{
		}

		virtual ~ControlSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<ControlSkin> ControlSkinPtr;

	class Control
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

		ControlSkinPtr m_Skin;

		HRESULT hr;

	public:
		Control(void)
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

		virtual ~Control(void)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		virtual void OnMouseEnter(void);

		virtual void OnMouseLeave(void);

		virtual bool OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void SetEnabled(bool bEnabled);

		virtual bool GetEnabled(void);

		virtual void SetVisible(bool bVisible);

		virtual bool GetVisible(void);
	};

	typedef boost::shared_ptr<Control> ControlPtr;

	class Static : public Control
	{
	public:
		std::wstring m_Text;

	public:
		Static(void)
			: m_Text(L"")
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Static> StaticPtr;

	class ButtonSkin : public ControlSkin
	{
	public:
		RECT m_DisabledTexRect;

		RECT m_PressedTexRect;

		RECT m_MouseOverTexRect;

		Vector2 m_PressedOffset;

	public:
		ButtonSkin(void)
			: m_DisabledTexRect(CRect(0,0,0,0))
			, m_PressedTexRect(CRect(0,0,0,0))
			, m_MouseOverTexRect(CRect(0,0,0,0))
			, m_PressedOffset(0,0)
		{
		}
	};

	typedef boost::shared_ptr<ButtonSkin> ButtonSkinPtr;

	typedef fastdelegate::FastDelegate1<Control *> ControlEvent;

	class Button : public Static
	{
	public:
		bool m_bPressed;

		ControlEvent EventClick;

	public:
		Button(void)
			: m_bPressed(false)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Button> ButtonPtr;

	class Dialog : public Control
	{
	public:
		typedef std::vector<ControlPtr> ControlPtrList;

		ControlPtrList m_Controls;

		ControlPtr m_ControlMouseOver;

		struct Viewport
		{
			DWORD Width;

			DWORD Height;

			float Fovy;
		};

		Viewport m_Viewport;

		struct Camera
		{
			Matrix4 World;

			Matrix4 View;

			Matrix4 Proj;
		};

		Camera m_Camera;

		Matrix4 m_Transform;

	public:
		Dialog(void);

		virtual ~Dialog(void)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		ControlPtr GetControlAtPoint(const Vector2 & pt);

		void RequestFocus(ControlPtr control);
	};

	typedef boost::shared_ptr<Dialog> DialogPtr;
}

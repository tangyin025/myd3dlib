
#pragma once

namespace my
{
	class UIRender
	{
	public:
		typedef Font::CUSTOMVERTEX CUSTOMVERTEX;

		static const DWORD D3DFVF_CUSTOMVERTEX = Font::D3DFVF_CUSTOMVERTEX;

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

	class UIControl;

	typedef boost::shared_ptr<UIControl> UIControlPtr;

	class UIControl
	{
	public:
		class Element
		{
		public:
			TexturePtr m_Texture;

			Rectangle m_TextureUV;

			FontPtr m_Font;
		};

		typedef boost::shared_ptr<Element> ElementPtr;

		bool m_bEnabled;

		bool m_bVisible;

		bool m_bMouseOver;

		bool m_bHasFocus;

		bool m_bIsDefault;

		Vector2 m_Location;

		Vector2 m_Size;

		D3DCOLOR m_Color;

		ElementPtr m_Elem;

		HRESULT hr;

		typedef std::vector<UIControlPtr> ControlPtrList;

		ControlPtrList m_Childs;

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

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset = Vector2(0, 0));

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam);

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

	class UIStatic : public UIControl
	{
	public:
		std::wstring m_Text;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

	public:
		UIStatic(void)
			: m_Text(L"")
			, m_TextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_TextAlign(my::Font::AlignCenterMiddle)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<UIStatic> UIStaticPtr;
}

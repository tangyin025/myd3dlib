
#pragma once

#include <boost/weak_ptr.hpp>
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
			DWORD color,
			const Rectangle & uvRect);

		// Example of Draw BuildRectangleVertices
		static void DrawRectangle(
			IDirect3DDevice9 * pd3dDevice,
			const Rectangle & rect,
			DWORD color,
			const Rectangle & uvRect);

		static void DrawRectangle(
			IDirect3DDevice9 * pd3dDevice,
			const Rectangle & rect,
			DWORD color);
	};

	class ControlSkin
	{
	public:
		TexturePtr m_Texture;

		RECT m_TextureRect;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

	public:
		ControlSkin(void)
			: m_TextureRect(CRect(0,0,0,0))
			, m_TextColor(D3DCOLOR_ARGB(255,255,255,0))
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

		boost::weak_ptr<Control> this_ptr;

	public:
		Control(void)
			: m_bEnabled(true)
			, m_bVisible(true)
			, m_bMouseOver(false)
			, m_bHasFocus(false)
			, m_bIsDefault(false)
			, m_Location(100, 100)
			, m_Size(100, 100)
			, m_Color(D3DCOLOR_ARGB(255,255,255,255))
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

	typedef fastdelegate::FastDelegate1<ControlPtr> ControlEvent;

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

	class EditBoxSkin : public ControlSkin
	{
	public:
		RECT m_DisabledTexRect;

		RECT m_FocusedTexRect;

		D3DCOLOR m_SelTextColor;

		D3DCOLOR m_SelBkColor;

		D3DCOLOR m_CaretColor;

	public:
		EditBoxSkin(void)
			: m_DisabledTexRect(CRect(0,0,0,0))
			, m_FocusedTexRect(CRect(0,0,0,0))
			, m_SelTextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_SelBkColor(D3DCOLOR_ARGB(197,0,0,0))
			, m_CaretColor(D3DCOLOR_ARGB(255,255,255,255))
		{
		}
	};

	typedef boost::shared_ptr<EditBoxSkin> EditBoxSkinPtr;

	class EditBox : public Control
	{
	public:
		std::wstring m_Text;

		int m_nCaret;

		bool m_bCaretOn;
    
		double m_dfBlink;

		double m_dfLastBlink;

		int m_nFirstVisible;

		Vector4 m_Border;

		bool m_bMouseDrag;

		int m_nSelStart;

		bool m_bInsertMode;

		ControlEvent EventChange;

		ControlEvent EventEnter;

	public:
		EditBox(void)
			: m_nCaret(0)
			, m_bCaretOn(true)
			, m_dfBlink(GetCaretBlinkTime() * 0.001f)
			, m_dfLastBlink(0)
			, m_nFirstVisible(0)
			, m_Border(0,0,0,0)
			, m_bMouseDrag(false)
			, m_nSelStart(0)
			, m_bInsertMode(true)
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		void PlaceCaret(int nCP);

		void ResetCaretBlink(void);

		void DeleteSelectionText(void);

		void CopyToClipboard(void);

		void PasteFromClipboard(void);

		int GetPriorItemPos(int nCP);

		int GetNextItemPos(int nCP);
	};

	typedef boost::shared_ptr<EditBox> EditBoxPtr;

	class ImeEditBox : public EditBox
	{
	public:
		static bool s_bHideCaret;

		static std::wstring s_CompString;

		D3DCOLOR m_CompWinColor;

		D3DCOLOR m_CandidateWinColor;

	public:
		ImeEditBox(void)
			: m_CompWinColor(D3DCOLOR_ARGB(197,0,0,0))
			, m_CandidateWinColor(D3DCOLOR_ARGB(197,0,0,0))
		{
		}

		virtual void OnRender(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		static void Initialize(HWND hWnd);

		static void Uninitialize(void);

		static bool StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void ResetCompositionString(void);

		static void EnableImeSystem(bool bEnable);

		void RenderIndicator(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		void RenderComposition(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		void RenderCandidateWindow(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);
	};

	typedef boost::shared_ptr<ImeEditBox> ImeEditBoxPtr;

	class Dialog : public Control
	{
	public:
		typedef std::vector<ControlPtr> ControlPtrList;

		ControlPtrList m_Controls;

		ControlPtr m_ControlMouseOver;

		Matrix4 m_Transform;

		Matrix4 m_ViewMatrix;

		Matrix4 m_ProjMatrix;

	public:
		Dialog(void)
			: m_Transform(Matrix4::Identity())
		{
			UIRender::BuildPerspectiveMatrices(D3DXToRadian(75.0f), 800, 600, m_ViewMatrix, m_ProjMatrix);
		}

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

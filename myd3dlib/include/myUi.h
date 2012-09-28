#pragma once

#include "myFont.h"
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>
#include <set>

namespace my
{
	class UIRender
	{
	public:
		typedef Font::CUSTOMVERTEX CUSTOMVERTEX;

		static const DWORD D3DFVF_CUSTOMVERTEX = Font::D3DFVF_CUSTOMVERTEX;

		static void BuildOrthoMatrices(float Width, float Height, Matrix4 & outView, Matrix4 & outProj);

		static void BuildPerspectiveMatrices(float fovy, float Width, float Height, Matrix4 & outView, Matrix4 & outProj);

		// Rendering UI under Fixed Pipeline is not recommended
		static void Begin(IDirect3DDevice9 * pd3dDevice);

		static void End(IDirect3DDevice9 * pd3dDevice);

		static Rectangle CalculateUVRect(const CSize & textureSize, const CRect & textureRect);

		static size_t BuildRectangleVertices(
			CUSTOMVERTEX * pBuffer,
			size_t bufferSize,
			const Rectangle & rect,
			DWORD color,
			const Rectangle & uvRect);

		// Example of Draw BuildRectangleVertices
		static void DrawRectangle(IDirect3DDevice9 * pd3dDevice, const Rectangle & rect, DWORD color);
	};

	class EventArgs
	{
	public:
		virtual ~EventArgs(void)
		{
		}
	};

	typedef boost::shared_ptr<EventArgs> EventArgsPtr;

	typedef boost::function<void (EventArgsPtr)> ControlEvent;

	class ControlImage
	{
	public:
		BaseTexturePtr m_Texture;

		D3DSURFACE_DESC desc;

		Vector4 m_Border;

		ControlImage(BaseTexturePtr Texture, const Vector4 & Border)
			: m_Texture(Texture)
			, desc(Texture->GetLevelDesc(0))
			, m_Border(Border)
		{
		}

		size_t BuildVertices(UIRender::CUSTOMVERTEX * pBuffer, size_t buffer_size, const Rectangle & rect, DWORD color);

		void Draw(IDirect3DDevice9 * pd3dDevice, const Rectangle & rect, DWORD color);
	};

	typedef boost::shared_ptr<ControlImage> ControlImagePtr;

	class ControlSkin
	{
	public:
		ControlImagePtr m_Image;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

	public:
		ControlSkin(void)
			: m_TextColor(D3DCOLOR_ARGB(255,255,255,0))
			, m_TextAlign(my::Font::AlignLeftTop)
		{
		}

		virtual ~ControlSkin(void)
		{
		}

		void DrawImage(IDirect3DDevice9 * pd3dDevice, ControlImagePtr Image, const Rectangle & rect, DWORD color);

		void DrawString(LPCWSTR pString, const Rectangle & rect, DWORD TextColor, Font::Align TextAlign);
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

		UINT m_nHotkey;

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
			, m_nHotkey(0)
			, m_Location(100, 100)
			, m_Size(100, 100)
			, m_Color(D3DCOLOR_ARGB(255,255,255,255))
			, m_Skin(new ControlSkin())
		{
		}

		virtual ~Control(void)
		{
		}

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		virtual void OnMouseEnter(void);

		virtual void OnMouseLeave(void);

		virtual void OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void SetEnabled(bool bEnabled);

		virtual bool GetEnabled(void);

		virtual void SetVisible(bool bVisible);

		virtual bool GetVisible(void);

		void SetHotkey(UINT nHotkey);

		UINT GetHotkey(void);
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

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Static> StaticPtr;

	class ButtonSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_DisabledImage;

		ControlImagePtr m_PressedImage;

		ControlImagePtr m_MouseOverImage;

	public:
		ButtonSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<ButtonSkin> ButtonSkinPtr;

	class Button : public Static
	{
	public:
		bool m_bPressed;

		D3DXCOLOR m_BlendColor;

		Vector2 m_PressedOffset;

		ControlEvent EventClick;

	public:
		Button(void)
			: m_bPressed(false)
			, m_BlendColor(m_Color)
			, m_PressedOffset(0,0)
		{
			m_Skin.reset(new ButtonSkin());
		}

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Button> ButtonPtr;

	class EditBoxSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_DisabledImage;

		ControlImagePtr m_FocusedImage;

		D3DCOLOR m_SelTextColor;

		D3DCOLOR m_SelBkColor;

		D3DCOLOR m_CaretColor;

	public:
		EditBoxSkin(void)
			: m_SelTextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_SelBkColor(D3DCOLOR_ARGB(197,0,0,0))
			, m_CaretColor(D3DCOLOR_ARGB(255,255,255,255))
		{
		}
	};

	typedef boost::shared_ptr<EditBoxSkin> EditBoxSkinPtr;

	class EditBox : public Static
	{
	public:
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
			m_Skin.reset(new EditBoxSkin());
		}

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void SetText(const std::wstring & Text);

		virtual const std::wstring & GetText(void) const;

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

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		static void Initialize(HWND hWnd);

		static void Uninitialize(void);

		static bool StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void ResetCompositionString(void);

		static void EnableImeSystem(bool bEnable);

		void RenderIndicator(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		void RenderComposition(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		void RenderCandidateWindow(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);
	};

	typedef boost::shared_ptr<ImeEditBox> ImeEditBoxPtr;

	class ScrollBarSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_UpBtnNormalImage;

		ControlImagePtr m_UpBtnDisabledImage;

		ControlImagePtr m_DownBtnNormalImage;

		ControlImagePtr m_DownBtnDisabledImage;

		ControlImagePtr m_ThumbBtnNormalImage;

	public:
		ScrollBarSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<ScrollBarSkin> ScrollBarSkinPtr;

	class ScrollBar : public Control
	{
	public:
		bool m_bDrag;

		float m_UpDownButtonHeight;

		int m_nPosition;

		int m_nPageSize;

		int m_nStart;

		int m_nEnd;

		enum ARROWSTATE
		{
			CLEAR,
			CLICKED_UP,
			CLICKED_DOWN,
			HELD_UP,
			HELD_DOWN,
		};

		ARROWSTATE m_Arrow;

		double m_dArrowTS;

		float m_fThumbOffsetY;

	public:
		ScrollBar(void)
			: m_bDrag(false)
			, m_UpDownButtonHeight(20)
			, m_nPosition(0)
			, m_nPageSize(1)
			, m_nStart(0)
			, m_nEnd(10)
			, m_Arrow(CLEAR)
			, m_dArrowTS(0)
			, m_fThumbOffsetY(0)
		{
			m_Skin.reset(new ScrollBarSkin());
		}

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		void Scroll(int nDelta);
	};

	typedef boost::shared_ptr<ScrollBar> ScrollBarPtr;

	class ComboBoxSkin : public ButtonSkin
	{
	public:
		ControlImagePtr m_DropdownImage;

		ControlImagePtr m_DropdownItemMouseOverImage;

		ControlImagePtr m_ScrollBarUpBtnNormalImage;

		ControlImagePtr m_ScrollBarUpBtnDisabledImage;

		ControlImagePtr m_ScrollBarDownBtnNormalImage;

		ControlImagePtr m_ScrollBarDownBtnDisabledImage;

		ControlImagePtr m_ScrollBarThumbBtnNormalImage;

		ControlImagePtr m_ScrollBarImage;

	public:
		ComboBoxSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<ComboBoxSkin> ComboBoxSkinPtr;

	class ComboBoxItem
	{
	public:
		std::wstring strText;

		void * pData;

		CRect rcActive;

		bool bVisible;

		ComboBoxItem(const std::wstring _strText, void * _pData = NULL)
			: strText(_strText)
			, pData(_pData)
			, rcActive(0,0,0,0)
			, bVisible(true)
		{
		}
	};

	typedef boost::shared_ptr<ComboBoxItem> ComboBoxItemPtr;

	class ComboBox : public Button
	{
	public:
		float m_DropdownWidth;

		float m_DropdownHeight;

		ScrollBar m_ScrollBar;

		float m_ScrollbarWidth;

		float m_ScrollbarUpDownBtnHeight;

		Vector4 m_Border;

		bool m_bOpened;

		typedef std::vector<ComboBoxItemPtr> ComboBoxItemPtrList;

		ComboBoxItemPtrList m_Items;

		float m_ItemHeight;

		int m_iFocused;

		int m_iSelected;

		ControlEvent EventSelectionChanged;

	public:
		ComboBox(void)
			: m_DropdownWidth(100)
			, m_DropdownHeight(100)
			, m_ScrollbarWidth(20)
			, m_ScrollbarUpDownBtnHeight(20)
			, m_Border(0,0,0,0)
			, m_bOpened(false)
			, m_ItemHeight(15)
			, m_iFocused(0)
			, m_iSelected(-1)
		{
			m_Skin.reset(new ComboBoxSkin());

			OnLayout();
		}

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnLayout(void);

		void SetDropdownWidth(float DropdownWidth);

		float GetDropdownWidth(void) const;

		void SetDropdownHeight(float DropdownHeight);

		float GetDropdownHeight(void) const;

		void SetBorder(const Vector4 & Border);

		const Vector4 & GetBorder(void) const;

		void SetItemHeight(float ItemHeight);

		float GetItemHeight(void) const;

		void SetSelected(int iSelected);

		int GetSelected(void) const;

		static void * ul2p(const unsigned long ul) { return (void *)(ULONG_PTR)ul; }

		void AddItem(const std::wstring & strText, void * pData);
	};

	class AlignEventArgs : public my::EventArgs
	{
	public:
		my::Vector2 vp;

		AlignEventArgs(const my::Vector2 & _vp)
			: vp(_vp)
		{
		}
	};

	class Dialog : public Control
	{
	public:
		typedef std::vector<ControlPtr> ControlPtrSet;

		ControlPtrSet m_Controls;

		ControlPtr m_ControlMouseOver;

		Matrix4 m_Transform;

		Matrix4 m_View;

		Matrix4 m_Proj;

		bool m_bMouseDrag;

		Vector2 m_MouseOffset;

		ControlEvent EventAlign;

	public:
		Dialog(void)
			: m_Transform(Matrix4::identity)
			, m_bMouseDrag(false)
			, m_MouseOffset(0,0)
		{
			UIRender::BuildPerspectiveMatrices(D3DXToRadian(75.0f), 800, 600, m_View, m_Proj);
		}

		virtual ~Dialog(void)
		{
		}

		virtual void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void SetVisible(bool bVisible);

		ControlPtr GetControlAtPoint(const Vector2 & pt);

		void RequestFocus(ControlPtr control);

		void ForceFocusControl(void);

		bool ContainsControl(ControlPtr control);

		void InsertControl(ControlPtr control)
		{
			m_Controls.push_back(control);
		}

		void ClearAllControl(void)
		{
			m_Controls.clear();
		}
	};

	typedef boost::shared_ptr<Dialog> DialogPtr;
}

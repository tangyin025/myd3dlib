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
		struct CUSTOMVERTEX
		{
			FLOAT x, y, z;
			DWORD color;
			FLOAT u, v;
		};

		static const DWORD D3DFVF_CUSTOMVERTEX = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

		HRESULT hr;

		CComPtr<IDirect3DDevice9> m_Device;

		CUSTOMVERTEX vertex_list[2048];

		size_t vertex_count;

		DWORD State[16];

	public:
		UIRender(IDirect3DDevice9 * pd3dDevice)
			: m_Device(pd3dDevice)
			, vertex_count(0)
		{
		}

		virtual ~UIRender(void)
		{
		}

		static Matrix4 OrthoView(float Width, float Height);

		static Matrix4 OrthoProj(float Width, float Height);

		static Matrix4 PerspectiveView(float Fovy, float Width, float Height);

		static Matrix4 PerspectiveProj(float Fovy, float Width, float Height);

		virtual void Begin(void);

		virtual void End(void);

		virtual void SetTexture(IDirect3DBaseTexture9 * pTexture);

		virtual void SetTransform(const Matrix4 & World, const Matrix4 & View, const Matrix4 & Proj);

		virtual void ClearVertexList(void);

		virtual void DrawVertexList(void);

		void PushVertex(float x, float y, float z, float u, float v, D3DCOLOR color);

		void PushRectangle(const Rectangle & rect, const Rectangle & uvRect, D3DCOLOR color);

		void DrawRectangle(const Rectangle & rect, DWORD color, const Rectangle & uvRect);

		void PushWindow(const Rectangle & rect, DWORD color, const CSize & windowSize, const Vector4 & windowBorder);

		void DrawWindow(const Rectangle & rect, DWORD color, const CSize & windowSize, const Vector4 & windowBorder);
	};

	typedef boost::shared_ptr<UIRender> UIRenderPtr;

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
		TexturePtr m_Texture;

		Vector4 m_Border;

		ControlImage(TexturePtr Texture, const Vector4 & Border)
			: m_Texture(Texture)
			, m_Border(Border)
		{
		}
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
			, m_TextAlign(Font::AlignLeftTop)
		{
		}

		virtual ~ControlSkin(void)
		{
		}

		void DrawImage(UIRender * ui_render, ControlImagePtr Image, const Rectangle & rect, DWORD color);

		void DrawString(UIRender * ui_render, LPCWSTR pString, const Rectangle & rect, DWORD TextColor, Font::Align TextAlign);
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

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

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

		virtual void Refresh(void);

		void SetHotkey(UINT nHotkey);

		UINT GetHotkey(void);
	};

	class Static : public Control
	{
	public:
		std::wstring m_Text;

	public:
		Static(void)
			: m_Text(L"")
		{
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		virtual bool ContainsPoint(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Static> StaticPtr;

	class ButtonSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_DisabledImage;

		ControlImagePtr m_PressedImage;

		ControlImagePtr m_MouseOverImage;

		Vector2 m_PressedOffset;

	public:
		ButtonSkin(void)
			: m_PressedOffset(0,0)
		{
		}
	};

	typedef boost::shared_ptr<ButtonSkin> ButtonSkinPtr;

	class Button : public Static
	{
	public:
		bool m_bPressed;

		D3DXCOLOR m_BlendColor;

		ControlEvent EventClick;

	public:
		Button(void)
			: m_bPressed(false)
			, m_BlendColor(m_Color)
		{
			m_Skin.reset(new ButtonSkin());
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnHotkey(void);

		virtual bool ContainsPoint(const Vector2 & pt);

		virtual void Refresh(void);
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

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

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

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		static void Initialize(HWND hWnd);

		static void Uninitialize(void);

		static bool StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void ResetCompositionString(void);

		static void EnableImeSystem(bool bEnable);

		void RenderIndicator(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		void RenderComposition(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		void RenderCandidateWindow(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);
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

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		void Scroll(int nDelta);
	};

	typedef boost::shared_ptr<ScrollBar> ScrollBarPtr;

	class CheckBox : public Button
	{
	public:
		Vector2 m_CheckBtnSize;

		bool m_Checked;

	public:
		CheckBox(void)
			: m_CheckBtnSize(20,20)
			, m_Checked(false)
		{
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);
	};

	typedef boost::shared_ptr<CheckBox> CheckBoxPtr;

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

		ComboBoxItem(const std::wstring _strText)
			: strText(_strText)
			, pData(NULL)
		{
		}
	};

	typedef boost::shared_ptr<ComboBoxItem> ComboBoxItemPtr;

	class ComboBox : public Button
	{
	public:
		Vector2 m_DropdownSize;

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
			: m_DropdownSize(100,100)
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

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusOut(void);

		virtual void OnLayout(void);

		void SetDropdownSize(const Vector2 & DropdownSize);

		const Vector2 & GetDropdownSize(void) const;

		void SetBorder(const Vector4 & Border);

		const Vector4 & GetBorder(void) const;

		void SetItemHeight(float ItemHeight);

		float GetItemHeight(void) const;

		void SetSelected(int iSelected);

		int GetSelected(void) const;

		void AddItem(const std::wstring & strText);

		void RemoveAllItems(void);

		bool ContainsItem(const std::wstring & strText, UINT iStart = 0) const;

		int FindItem(const std::wstring & strText, UINT iStart = 0) const;

		void * GetItemData(int index);

		void SetItemData(int index, void * pData);

		unsigned int GetItemDataUInt(int index);

		void SetItemData(int index, unsigned int uData);

		UINT GetNumItems(void);
	};

	typedef boost::shared_ptr<ComboBox> ComboBoxPtr;

	typedef boost::shared_ptr<Control> ControlPtr;

	typedef std::vector<ControlPtr> ControlPtrSet;

	class Dialog : public Control
	{
	public:
		static boost::weak_ptr<Control> s_ControlFocus;

		ControlPtrSet m_Controls;

		ControlPtr m_ControlMouseOver;

		Matrix4 m_World;

		bool m_bMouseDrag;

		Vector2 m_MouseOffset;

		ControlEvent EventAlign;

		ControlEvent EventRefresh;

	public:
		Dialog(void)
			: m_World(Matrix4::identity)
			, m_bMouseDrag(false)
			, m_MouseOffset(0,0)
		{
		}

		virtual ~Dialog(void)
		{
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset = Vector2(0,0));

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void SetVisible(bool bVisible);

		virtual void Refresh(void);

		ControlPtr GetControlAtPoint(const Vector2 & pt);

		void RequestFocus(ControlPtr control);

		void ForceFocusControl(void);

		bool ContainsControl(ControlPtr control);

		void InsertControl(ControlPtr control)
		{
			m_Controls.push_back(control);
		}

		void RemoveControl(ControlPtr control)
		{
			ControlPtrSet::iterator ctrl_iter = std::find(m_Controls.begin(), m_Controls.end(), control);
			if(ctrl_iter != m_Controls.end())
			{
				m_Controls.erase(ctrl_iter);
			}
		}

		void ClearAllControl(void)
		{
			m_Controls.clear();
		}
	};

	typedef boost::shared_ptr<Dialog> DialogPtr;
}

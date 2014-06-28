#pragma once

#include "myFont.h"
#include <boost/function.hpp>
#include <vector>
#include <map>

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

		virtual ~UIRender(void);

		virtual void Begin(void);

		virtual void End(void);

		virtual void SetWorld(const Matrix4 & World);

		virtual void SetViewProj(const Matrix4 & ViewProj);

		virtual void SetTexture(const BaseTexturePtr & Texture);

		virtual void ClearVertexList(void);

		virtual void DrawVertexList(void);

		void PushVertex(float x, float y, float z, float u, float v, D3DCOLOR color);

		void PushRectangle(const Rectangle & rect, const Rectangle & UvRect, D3DCOLOR color);

		void DrawRectangle(const Rectangle & rect, DWORD color, const Rectangle & UvRect);

		void PushWindow(const Rectangle & rect, DWORD color, const CSize & WindowSize, const Vector4 & WindowBorder);

		void DrawWindow(const Rectangle & rect, DWORD color, const CSize & WindowSize, const Vector4 & WindowBorder);
	};

	typedef boost::shared_ptr<UIRender> UIRenderPtr;

	class EventArgs
	{
	public:
		virtual ~EventArgs(void)
		{
		}
	};

	typedef boost::function<void (EventArgs *)> ControlEvent;

	class ControlImage
	{
	public:
		BaseTexturePtr m_Texture;

		Vector4 m_Border;

		ControlImage(BaseTexturePtr Texture, const Vector4 & Border)
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

		virtual ~ControlSkin(void);

		void DrawImage(UIRender * ui_render, ControlImagePtr Image, const Rectangle & rect, DWORD color);

		void DrawString(UIRender * ui_render, LPCWSTR pString, const Rectangle & rect, DWORD TextColor, Font::Align TextAlign);
	};

	typedef boost::shared_ptr<ControlSkin> ControlSkinPtr;

	class Control;

	typedef boost::shared_ptr<Control> ControlPtr;

	class Control
	{
	public:
		typedef std::vector<ControlPtr> ControlPtrList;

		ControlPtrList m_Childs;

		Control * m_Parent;

		static Control * s_FocusControl;

		static Control * s_CaptureControl;

		static Control * s_MouseOverControl;

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
			, m_Parent(NULL)
		{
			m_Skin.reset(new ControlSkin());
		}

		virtual ~Control(void);

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

		virtual bool RayToWorld(const std::pair<Vector3, Vector3> & ray, Vector2 & ptWorld);

		void InsertControl(ControlPtr control);

		void RemoveControl(ControlPtr control);

		void ClearAllControl(void);

		Control * GetChildAtPoint(const Vector2 & pt) const;

		Vector2 LocalToWorld(const Vector2 & pt) const;

		Vector2 WorldToLocal(const Vector2 & pt) const;

		void SetFocus(void);

		void ReleaseFocus(void);

		void SetCapture(void);

		void ReleaseCapture(void);

		void SetMouseOver(void);

		void ReleaseMouseOver(void);

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
	};

	typedef boost::shared_ptr<Static> StaticPtr;

	class ProgressBarSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_ForegroundImage;

	public:
		ProgressBarSkin(void)
		{
		}
	};

	typedef boost::shared_ptr<ProgressBarSkin> ProgressBarSkinPtr;

	class ProgressBar : public Static
	{
	public:
		float m_Progress;

		float m_BlendProgress;

	public:
		ProgressBar(void)
			: m_Progress(0)
			, m_BlendProgress(0)
		{
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset);
	};

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

		D3DCOLOR m_SelBkColor;

		D3DCOLOR m_CaretColor;

	public:
		EditBoxSkin(void)
			: m_SelBkColor(D3DCOLOR_ARGB(197,0,0,0))
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
    
		DWORD m_dwBlink;

		DWORD m_dwLastBlink;

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
			, m_dwBlink(GetCaretBlinkTime())
			, m_dwLastBlink(0)
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

		DWORD m_dwArrowTS;

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
			, m_dwArrowTS(0)
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

	class Dialog : public Control
	{
	public:
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

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset = Vector2(0,0));

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void SetVisible(bool bVisible);

		virtual void Refresh(void);

		virtual bool RayToWorld(const std::pair<Vector3, Vector3> & ray, Vector2 & ptWorld);
	};

	typedef boost::shared_ptr<Dialog> DialogPtr;

	class DialogMgr
	{
	public:
		typedef std::vector<DialogPtr> DialogPtrList;

		DialogPtrList m_DlgList;

		Vector3 m_ViewPosition;

		Matrix4 m_View;

		Matrix4 m_Proj;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

	public:
		DialogMgr(void)
		{
			SetDlgViewport(Vector2(800,600), D3DXToRadian(75.0f));
		}

		void SetDlgViewport(const Vector2 & vp, float fov);

		Vector2 GetDlgViewport(void) const;

		std::pair<Vector3, Vector3> CalculateRay(const Vector2 & pt, const CSize & dim);

		void Draw(UIRender * ui_render, double fTime, float fElapsedTime);

		bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void InsertDlg(DialogPtr dlg);

		void RemoveDlg(DialogPtr dlg);

		void RemoveAllDlg();
	};
}

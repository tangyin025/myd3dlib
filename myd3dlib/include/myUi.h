#pragma once

#include "myFont.h"
#include <boost/array.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <vector>
#include <list>

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

		my::Texture2DPtr m_WhiteTex;

		typedef std::vector<CUSTOMVERTEX> VertexList;

		typedef std::pair<BaseTexture *, VertexList> UILayer;

		enum UILayerType
		{
			UILayerTexture,
			UILayerFont,
			UILayerNum,
		};

		typedef boost::array<UILayer, UILayerNum> UILayerList;

		UILayerList m_Layer;

	public:
		UIRender(void);

		virtual ~UIRender(void);

		virtual HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		virtual void Begin(void);

		virtual void End(void);

		virtual void SetWorld(const Matrix4 & World);

		virtual void SetViewProj(const Matrix4 & ViewProj);

		virtual void Flush(void);

		void PushVertexSimple(VertexList & vertex_list, unsigned int start, float x, float y, float z, float u, float v, D3DCOLOR color);

		void PushRectangleSimple(VertexList & vertex_list, unsigned int start, const Rectangle & rect, const Rectangle & UvRect, D3DCOLOR color);

		void PushRectangle(const Rectangle & rect, const Rectangle & UvRect, D3DCOLOR color, BaseTexture * texture, UILayerType type);

		void PushWindowSimple(VertexList & vertex_list, unsigned int start, const Rectangle & rect, DWORD color, const Rectangle & WindowRect, const Vector4 & WindowBorder, const CSize & TextureSize);

		void PushWindow(const Rectangle & rect, DWORD color, const Rectangle & WindowRect, const Vector4 & WindowBorder, const CSize & TextureSize, BaseTexture * texture, UILayerType type);
	};

	typedef boost::shared_ptr<UIRender> UIRenderPtr;

	class ControlImage;

	typedef boost::shared_ptr<ControlImage> ControlImagePtr;

	class ControlImage
	{
	public:
		std::string m_TexturePath;

		BaseTexturePtr m_Texture;

		Rectangle m_Rect;

		Vector4 m_Border;

		bool m_Requested;

		ControlImage(void)
			: m_Rect(0,0,100,100)
			, m_Border(0,0,0,0)
			, m_Requested(false)
		{
		}

		~ControlImage(void);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_TexturePath);
			ar & BOOST_SERIALIZATION_NVP(m_Rect);
			ar & BOOST_SERIALIZATION_NVP(m_Border);
		}

		bool IsRequested(void) const
		{
			return m_Requested;
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void OnImageReady(my::DeviceResourceBasePtr res);

		virtual ControlImagePtr Clone(void) const;
	};

	class ControlSkin;

	typedef boost::shared_ptr<ControlSkin> ControlSkinPtr;

	class ControlSkin
	{
	public:
		D3DCOLOR m_Color;

		ControlImagePtr m_Image;

		std::string m_FontPath;

		int m_FontHeight;

		int m_FontFaceIndex;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

		std::string m_VisibleShowSound;

		std::string m_VisibleHideSound;

		std::string m_MouseEnterSound;

		std::string m_MouseLeaveSound;

		std::string m_MouseClickSound;

		bool m_Requested;

	public:
		ControlSkin(void)
			: m_Color(D3DCOLOR_ARGB(255, 255, 255, 255))
			, m_FontHeight(13)
			, m_FontFaceIndex(0)
			, m_TextColor(D3DCOLOR_ARGB(255, 255, 255, 255))
			, m_TextAlign(Font::AlignLeftTop)
			, m_Requested(false)
		{
		}

		virtual ~ControlSkin(void);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_Color);
			ar & BOOST_SERIALIZATION_NVP(m_Image);
			ar & BOOST_SERIALIZATION_NVP(m_FontPath);
			ar & BOOST_SERIALIZATION_NVP(m_FontHeight);
			ar & BOOST_SERIALIZATION_NVP(m_FontFaceIndex);
			ar & BOOST_SERIALIZATION_NVP(m_TextColor);
			ar & BOOST_SERIALIZATION_NVP(m_TextAlign);
			ar & BOOST_SERIALIZATION_NVP(m_VisibleShowSound);
			ar & BOOST_SERIALIZATION_NVP(m_VisibleHideSound);
			ar & BOOST_SERIALIZATION_NVP(m_MouseEnterSound);
			ar & BOOST_SERIALIZATION_NVP(m_MouseLeaveSound);
			ar & BOOST_SERIALIZATION_NVP(m_MouseClickSound);
		}

		bool IsRequested(void) const
		{
			return m_Requested;
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void OnFontReady(my::DeviceResourceBasePtr res);

		void DrawImage(UIRender * ui_render, const ControlImagePtr & Image, const Rectangle & rect, DWORD color);

		void DrawString(UIRender * ui_render, LPCWSTR pString, const Rectangle & rect, DWORD TextColor, Font::Align TextAlign);

		void CopyFrom(const ControlSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
	};

	class Control;

	typedef boost::shared_ptr<Control> ControlPtr;

	class ControlEventArg : public EventArg
	{
	public:
		Control * sender;

		ControlEventArg(Control * _sender)
			: sender(_sender)
		{
		}

		virtual ~ControlEventArg(void)
		{
		}
	};

	class VisibleEventArg : public ControlEventArg
	{
	public:
		bool Visible;

		VisibleEventArg(Control * _sender, bool _Visible)
			: ControlEventArg(_sender)
			, Visible(_Visible)
		{
		}
	};

	class MouseEventArg : public ControlEventArg
	{
	public:
		Vector2 pt;

		MouseEventArg(Control * _sender, const Vector2 & _pt)
			: ControlEventArg(_sender)
			, pt(_pt)
		{
		}
	};

	class Control
		: public my::NamedObject
		, public boost::enable_shared_from_this<Control>
	{
	public:
		enum ControlType
		{
			ControlTypeControl = 0,
			ControlTypeStatic,
			ControlTypeProgressBar,
			ControlTypeButton,
			ControlTypeEditBox,
			ControlTypeImeEditBox,
			ControlTypeScrollBar,
			ControlTypeCheckBox,
			ControlTypeComboBox,
			ControlTypeListBox,
			ControlTypeDialog,
		};

		typedef std::list<ControlPtr> ControlPtrList;

		ControlPtrList m_Childs;

		Control * m_Parent;

		static Control * s_FocusControl;

		static Control * s_CaptureControl;

		static Control * s_MouseOverControl;

		bool m_Requested;

		bool m_bEnabled;

		bool m_bVisible;

		bool m_bMouseOver;

		bool m_bHasFocus;

		UINT m_nHotkey;

		UDim m_x;

		UDim m_y;

		UDim m_Width;

		UDim m_Height;

		Rectangle m_Rect;

		ControlSkinPtr m_Skin;

		bool m_bPressed;

		EventFunction m_EventVisibleChanged;

		EventFunction m_EventMouseEnter;

		EventFunction m_EventMouseLeave;

		EventFunction m_EventMouseClick;

	protected:
		Control(void)
			: m_Requested(false)
			, m_bEnabled(true)
			, m_bVisible(true)
			, m_bMouseOver(false)
			, m_bHasFocus(false)
			, m_nHotkey(0)
			, m_x(0, 0)
			, m_y(0, 0)
			, m_Width(0, 100)
			, m_Height(0, 100)
			, m_Rect(0, 0, 100, 100)
			, m_Parent(NULL)
			, m_bPressed(false)
		{
		}

	public:
		Control(const char * Name)
			: NamedObject(Name)
			, m_Requested(false)
			, m_bEnabled(true)
			, m_bVisible(true)
			, m_bMouseOver(false)
			, m_bHasFocus(false)
			, m_nHotkey(0)
			, m_x(0, 0)
			, m_y(0, 0)
			, m_Width(0, 100)
			, m_Height(0, 100)
			, m_Rect(0, 0, 100, 100)
			, m_Parent(NULL)
			, m_bPressed(false)
		{
		}

		virtual ~Control(void);

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		bool IsRequested(void) const
		{
			return m_Requested;
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeControl;
		}

		static Control * GetFocusControl(void)
		{
			return s_FocusControl;
		}

		static void SetFocusControl(Control * control);

		static Control * GetCaptureControl(void)
		{
			return s_CaptureControl;
		}

		static void SetCaptureControl(Control * control);

		static void SetMouseOverControl(Control * control, const Vector2 & pt);

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		virtual void OnMouseEnter(const Vector2 & pt);

		virtual void OnMouseLeave(const Vector2 & pt);

		virtual void OnHotkey(void);

		virtual bool HitTest(const Vector2 & pt);

		virtual void OnLayout(void);

		virtual void SetEnabled(bool bEnabled);

		virtual bool GetEnabled(void);

		virtual void SetVisible(bool bVisible);

		virtual bool GetVisible(void);

		virtual bool RayToWorld(const Ray & ray, Vector2 & ptWorld);

		void InsertControl(ControlPtr control);

		void RemoveControl(ControlPtr control);

		void ClearAllControl(void);

		bool ContainsControl(Control * control);

		Control * GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible);

		Control * GetTopControl(void);

		bool SetFocusRecursive(void);

		void GetNearestControl(const Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist);

		void SetHotkey(UINT nHotkey);

		UINT GetHotkey(void);
	};

	class Static : public Control
	{
	public:
		std::wstring m_Text;

	protected:
		Static(void)
		{
		}

	public:
		Static(const char * Name)
			: Control(Name)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeStatic;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);
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

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
			ar & BOOST_SERIALIZATION_NVP(m_ForegroundImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const ProgressBarSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
	};

	typedef boost::shared_ptr<ProgressBarSkin> ProgressBarSkinPtr;

	class ProgressBar : public Static
	{
	public:
		float m_Progress;

		float m_BlendProgress;

	protected:
		ProgressBar(void)
			: m_Progress(0)
			, m_BlendProgress(0)
		{
		}

	public:
		ProgressBar(const char * Name)
			: Static(Name)
			, m_Progress(0)
			, m_BlendProgress(0)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Static);
			ar & BOOST_SERIALIZATION_NVP(m_Progress);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeProgressBar;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);
	};

	typedef boost::shared_ptr<ProgressBar> ProgressBarPtr;

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

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
			ar & BOOST_SERIALIZATION_NVP(m_DisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_PressedImage);
			ar & BOOST_SERIALIZATION_NVP(m_MouseOverImage);
			ar & BOOST_SERIALIZATION_NVP(m_PressedOffset);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const ButtonSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
	};

	typedef boost::shared_ptr<ButtonSkin> ButtonSkinPtr;

	class Button : public Static
	{
	protected:
		D3DXCOLOR m_BlendColor;

		Button(void)
			: m_BlendColor(0, 0, 0, 0)
		{
		}

	public:
		Button(const char * Name)
			: Static(Name)
			, m_BlendColor(0, 0, 0, 0)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Static);
			ar & BOOST_SERIALIZATION_NVP(m_bPressed);
			ar & BOOST_SERIALIZATION_NVP(m_bMouseOver);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeButton;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnHotkey(void);

		virtual bool HitTest(const Vector2 & pt);
	};

	typedef boost::shared_ptr<Button> ButtonPtr;

	class EditBoxSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_DisabledImage;

		ControlImagePtr m_FocusedImage;

		D3DCOLOR m_SelBkColor;

		D3DCOLOR m_CaretColor;

		ControlImagePtr m_CaretImage;

	public:
		EditBoxSkin(void)
			: m_SelBkColor(D3DCOLOR_ARGB(255,255,128,0))
			, m_CaretColor(D3DCOLOR_ARGB(255,255,255,255))
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
			ar & BOOST_SERIALIZATION_NVP(m_DisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_FocusedImage);
			ar & BOOST_SERIALIZATION_NVP(m_SelBkColor);
			ar & BOOST_SERIALIZATION_NVP(m_CaretColor);
			ar & BOOST_SERIALIZATION_NVP(m_CaretImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const EditBoxSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
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

		EventFunction m_EventChange;

		EventFunction m_EventEnter;

	protected:
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
		}

	public:
		EditBox(const char * Name)
			: Static(Name)
			, m_nCaret(0)
			, m_bCaretOn(true)
			, m_dwBlink(GetCaretBlinkTime())
			, m_dwLastBlink(0)
			, m_nFirstVisible(0)
			, m_Border(0, 0, 0, 0)
			, m_bMouseDrag(false)
			, m_nSelStart(0)
			, m_bInsertMode(true)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Static);
			ar & BOOST_SERIALIZATION_NVP(m_Border);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeEditBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnFocusIn(void);

		virtual bool HitTest(const Vector2 & pt);

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

	protected:
		ImeEditBox(void)
			: m_CompWinColor(D3DCOLOR_ARGB(197,0,0,0))
			, m_CandidateWinColor(D3DCOLOR_ARGB(197,0,0,0))
		{
		}

	public:
		ImeEditBox(const char * Name)
			: EditBox(Name)
			, m_CompWinColor(D3DCOLOR_ARGB(197, 0, 0, 0))
			, m_CandidateWinColor(D3DCOLOR_ARGB(197, 0, 0, 0))
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EditBox);
			ar & BOOST_SERIALIZATION_NVP(m_CompWinColor);
			ar & BOOST_SERIALIZATION_NVP(m_CandidateWinColor);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeImeEditBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		static void Initialize(HWND hWnd);

		static void Uninitialize(void);

		static bool StaticMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		static void ResetCompositionString(void);

		static void EnableImeSystem(bool bEnable);

		void RenderIndicator(UIRender * ui_render, float fElapsedTime);

		void RenderComposition(UIRender * ui_render, float fElapsedTime);

		void RenderCandidateWindow(UIRender * ui_render, float fElapsedTime);
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

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
			ar & BOOST_SERIALIZATION_NVP(m_UpBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_UpBtnDisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_DownBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_DownBtnDisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_ThumbBtnNormalImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const ScrollBarSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
	};

	typedef boost::shared_ptr<ScrollBarSkin> ScrollBarSkinPtr;

	class ScrollBar : public Control
	{
	public:
		bool m_bPressed;

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

	protected:
		ScrollBar(void)
			: m_bPressed(false)
			, m_bDrag(false)
			, m_UpDownButtonHeight(20)
			, m_nPosition(0)
			, m_nPageSize(1)
			, m_nStart(0)
			, m_nEnd(10)
			, m_Arrow(CLEAR)
			, m_dwArrowTS(0)
			, m_fThumbOffsetY(0)
		{
		}

	public:
		ScrollBar(const char * Name)
			: Control(Name)
			, m_bPressed(false)
			, m_bDrag(false)
			, m_UpDownButtonHeight(20)
			, m_nPosition(0)
			, m_nPageSize(1)
			, m_nStart(0)
			, m_nEnd(10)
			, m_Arrow(CLEAR)
			, m_dwArrowTS(0)
			, m_fThumbOffsetY(0)
		{
		}

		friend class ComboBox;

		friend class ListBox;

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
			ar & BOOST_SERIALIZATION_NVP(m_nPageSize);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeScrollBar;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		void SimulateRepeatedScroll(void);

		void Scroll(int nDelta);

		void ScrollTo(int nPosition);
	};

	typedef boost::shared_ptr<ScrollBar> ScrollBarPtr;

	class CheckBox : public Button
	{
	public:
		bool m_Checked;

	protected:
		CheckBox(void)
			: m_Checked(false)
		{
		}

	public:
		CheckBox(const char * Name)
			: Button(Name)
			, m_Checked(false)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Button);
			ar & BOOST_SERIALIZATION_NVP(m_Checked);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeCheckBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);
	};

	typedef boost::shared_ptr<CheckBox> CheckBoxPtr;

	class ComboBoxSkin : public ButtonSkin
	{
	public:
		ControlImagePtr m_DropdownImage;

		D3DCOLOR m_DropdownItemTextColor;

		Font::Align m_DropdownItemTextAlign;

		ControlImagePtr m_DropdownItemMouseOverImage;

		ControlImagePtr m_ScrollBarUpBtnNormalImage;

		ControlImagePtr m_ScrollBarUpBtnDisabledImage;

		ControlImagePtr m_ScrollBarDownBtnNormalImage;

		ControlImagePtr m_ScrollBarDownBtnDisabledImage;

		ControlImagePtr m_ScrollBarThumbBtnNormalImage;

		ControlImagePtr m_ScrollBarImage;

	public:
		ComboBoxSkin(void)
			: m_DropdownItemTextColor(D3DCOLOR_ARGB(255,255,255,255))
			, m_DropdownItemTextAlign(Font::AlignLeftTop)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ButtonSkin);
			ar & BOOST_SERIALIZATION_NVP(m_DropdownImage);
			ar & BOOST_SERIALIZATION_NVP(m_DropdownItemMouseOverImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarUpBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarUpBtnDisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarDownBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarDownBtnDisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarThumbBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const ComboBoxSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
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

		Rectangle m_DropdownRect;

		ScrollBar m_ScrollBar;

		float m_ScrollbarWidth;

		float m_ScrollbarUpDownBtnHeight;

		Vector4 m_Border;

		typedef std::vector<ComboBoxItemPtr> ComboBoxItemPtrList;

		ComboBoxItemPtrList m_Items;

		float m_ItemHeight;

		int m_iFocused;

		int m_iSelected;

		EventFunction m_EventSelectionChanged;

		D3DXCOLOR m_BlendColor;

	protected:
		ComboBox(void)
			: m_DropdownSize(100, 100)
			, m_DropdownRect(0, 0, 100, 100)
			, m_ScrollbarWidth(20)
			, m_ScrollbarUpDownBtnHeight(20)
			, m_Border(0, 0, 0, 0)
			, m_ItemHeight(15)
			, m_iFocused(0)
			, m_iSelected(-1)
			, m_BlendColor(0, 0, 0, 0)
		{
		}

	public:
		ComboBox(const char * Name);

		~ComboBox(void)
		{
			m_ScrollBar.m_Parent = NULL;
		}

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeComboBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusOut(void);

		virtual bool HitTest(const Vector2 & pt);

		virtual void OnLayout(void);

		void SetDropdownSize(const Vector2 & DropdownSize);

		const Vector2 & GetDropdownSize(void) const;

		void SetScrollbarWidth(float Width);

		float GetScrollbarWidth(void) const;

		void SetScrollbarUpDownBtnHeight(float Height);

		float GetScrollbarUpDownBtnHeight(void) const;

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

	class ListBoxSkin : public ControlSkin
	{
	public:
		ControlImagePtr m_MouseOverImage;

		ControlImagePtr m_ScrollBarUpBtnNormalImage;

		ControlImagePtr m_ScrollBarUpBtnDisabledImage;

		ControlImagePtr m_ScrollBarDownBtnNormalImage;

		ControlImagePtr m_ScrollBarDownBtnDisabledImage;

		ControlImagePtr m_ScrollBarThumbBtnNormalImage;

		ControlImagePtr m_ScrollBarImage;

	public:
		ListBoxSkin(void)
		{
		}

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
			ar & BOOST_SERIALIZATION_NVP(m_MouseOverImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarUpBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarUpBtnDisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarDownBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarDownBtnDisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarThumbBtnNormalImage);
			ar & BOOST_SERIALIZATION_NVP(m_ScrollBarImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const ListBoxSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
	};

	typedef boost::shared_ptr<ListBoxSkin> ListBoxSkinPtr;

	struct ListBoxItem
	{
		std::wstring strText;

		void * pData;

		RECT rcActive;

		bool bSelected;
	};

	typedef boost::shared_ptr<ListBoxItem> ListBoxItemPtr;

	class ListBox : public Control
	{
	public:
		ScrollBar m_ScrollBar;

		float m_ScrollbarWidth;

		float m_ScrollbarUpDownBtnHeight;

		typedef std::vector<ListBoxItemPtr> ListBoxItemPtrList;

		ListBoxItemPtrList m_Items;

		Vector2 m_ItemSize;

		int m_ItemColumn;

		CPoint m_iFocused;

	protected:
		ListBox(void)
			: m_ScrollbarWidth(20)
			, m_ScrollbarUpDownBtnHeight(20)
			, m_ItemSize(50, 50)
			, m_ItemColumn(1)
			, m_iFocused(-1, -1)
		{
		}

	public:
		ListBox(const char* Name);

		~ListBox(void)
		{
			m_ScrollBar.m_Parent = NULL;
		}

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive& ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive& ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeListBox;
		}

		virtual void Draw(UIRender* ui_render, float fElapsedTime, const Vector2& Offset, const Vector2& Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2& pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void OnLayout(void);

		void SetScrollbarWidth(float Width);

		float GetScrollbarWidth(void) const;

		void SetScrollbarUpDownBtnHeight(float Height);

		float GetScrollbarUpDownBtnHeight(void) const;

		void SetItemSize(const my::Vector2 & ItemSize);

		const my::Vector2 & GetItemSize(void) const;

		void AddItem(const std::wstring& strText);

		void RemoveAllItems(void);
	};

	typedef boost::shared_ptr<ListBox> ListBoxPtr;

	class DialogSkin : public ControlSkin
	{
	public:
		DialogSkin(void)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void CopyFrom(const DialogSkin & rhs);

		virtual ControlSkinPtr Clone(void) const;
	};

	typedef boost::shared_ptr<DialogSkin> DialogSkinPtr;

	class Dialog;

	typedef boost::shared_ptr<Dialog> DialogPtr;

	class DialogMgr;

	class Dialog : public Control
	{
	public:
		DialogMgr * m_Manager;

		Matrix4 m_World;

		bool m_EnableDrag;

		bool m_bMouseDrag;

		Vector2 m_MouseOffset;

		EventFunction m_EventAlign;

	protected:
		Dialog(void)
			: m_Manager(NULL)
			, m_World(Matrix4::identity)
			, m_EnableDrag(true)
			, m_bMouseDrag(false)
			, m_MouseOffset(0,0)
		{
		}

	public:
		Dialog(const char * Name)
			: Control(Name)
			, m_Manager(NULL)
			, m_World(Matrix4::identity)
			, m_EnableDrag(true)
			, m_bMouseDrag(false)
			, m_MouseOffset(0, 0)
		{
		}

		virtual ~Dialog(void);

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		virtual ControlType GetControlType(void) const
		{
			return ControlTypeDialog;
		}

		virtual void Draw(UIRender* ui_render, float fElapsedTime, const Vector2& Offset, const Vector2& Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void);

		virtual void SetVisible(bool bVisible);

		virtual bool RayToWorld(const Ray & ray, Vector2 & ptWorld);
	};

	class DialogMgr
	{
	public:
		typedef std::list<Dialog *> DialogList;

		DialogList m_DlgList;

		Vector3 m_Eye;

		Matrix4 m_View;

		Matrix4 m_Proj;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

	public:
		DialogMgr(void)
		{
			SetDlgViewport(Vector2(800,600), D3DXToRadian(75.0f));
		}

		void SetDlgViewport(const Vector2 & Viewport, float fov);

		Vector2 GetDlgViewport(void) const;

		Ray CalculateRay(const Vector2 & pt, const CSize & dim);

		void Draw(UIRender * ui_render, double fTime, float fElapsedTime);

		bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void InsertDlg(Dialog * dlg);

		void RemoveDlg(Dialog * dlg);

		void RemoveAllDlg();
	};
}

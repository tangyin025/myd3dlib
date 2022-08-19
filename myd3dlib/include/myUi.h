#pragma once

#include "myFont.h"
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
			union
			{
				struct
				{
					FLOAT x, y, z;
				};

				struct
				{
					Vector3 xyz;
				};
			};
			DWORD color;
			FLOAT u, v;
			CUSTOMVERTEX(FLOAT _x, FLOAT _y, FLOAT _z, DWORD _color, FLOAT _u, FLOAT _v)
				: x(_x), y(_y), z(_z), color(_color), u(_u), v(_v)
			{
			}
		};

		static const DWORD D3DFVF_CUSTOMVERTEX = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

		enum ClipType
		{
			ClipLeft	= 0x01,
			ClipTop		= 0x02,
			ClipRight	= 0x04,
			ClipBottom	= 0x08,
		};

		HRESULT hr;

		CComPtr<IDirect3DDevice9> m_Device;

		my::Texture2DPtr m_WhiteTex;

		typedef std::vector<CUSTOMVERTEX> VertexList;

		typedef std::pair<const BaseTexture *, VertexList> UILayer;

		typedef std::vector<UILayer> UILayerList;

		UILayerList m_Layer;

		int m_LayerDrawCall;

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

		static void PushTriangleSimple(VertexList & vertex_list, const CUSTOMVERTEX & v0, const CUSTOMVERTEX & v1, const CUSTOMVERTEX & v2);

		static void PushTriangleSimple(VertexList & vertex_list, const CUSTOMVERTEX & v0, const CUSTOMVERTEX & v1, const CUSTOMVERTEX & v2, const Rectangle & clip, DWORD clipmask);

		static void PushRectangleSimple(VertexList & vertex_list, const Rectangle & rect, const Rectangle & UvRect, D3DCOLOR color);

		static void PushRectangleSimple(VertexList & vertex_list, const Rectangle & rect, const Rectangle & UvRect, D3DCOLOR color, const Rectangle & clip);

		VertexList & GetVertexList(const BaseTexture * texture);

		void PushRectangle(const Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture);

		void PushRectangle(const Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture, const Rectangle & clip);

		void PushRectangle(const Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture, const Matrix4 & transform);

		void PushRectangle(const Rectangle & rect, D3DCOLOR color, const Rectangle & WindowRect, const BaseTexture * texture, const Matrix4 & transform, const Rectangle & clip);

		static void PushWindowSimple(VertexList & vertex_list, const Rectangle & rect, const Rectangle & InRect, const Rectangle & OutUvRect, const Rectangle & InUvRect, DWORD color);

		static void PushWindowSimple(VertexList & vertex_list, const Rectangle & rect, const Rectangle & InRect, const Rectangle & OutUvRect, const Rectangle & InUvRect, DWORD color, const Rectangle & clip);

		void PushWindow(const Rectangle & rect, DWORD color, const Rectangle & WindowRect, const Vector4 & WindowBorder, const BaseTexture * texture);

		void PushWindow(const Rectangle & rect, DWORD color, const Rectangle & WindowRect, const Vector4 & WindowBorder, const BaseTexture * texture, const Rectangle & clip);

		void PushCharacter(float x, float y, const Font::CharacterInfo* info, Font* font, D3DCOLOR color);

		void PushCharacter(float x, float y, const Font::CharacterInfo* info, Font* font, D3DCOLOR color, const Matrix4& transform);

		void PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, Font * font);

		void PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, D3DCOLOR outlineColor, float outlineWidth, Font * font);

		void PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, Font * font, const Matrix4 & transform);

		void PushString(const Rectangle & rect, const wchar_t * str, D3DCOLOR color, Font::Align align, D3DCOLOR outlineColor, float outlineWidth, Font * font, const Matrix4 & transform);
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

		virtual ~ControlImage(void);

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

		void OnTextureReady(my::DeviceResourceBasePtr res);

		virtual ControlImagePtr Clone(void) const;

		virtual void Draw(UIRender * ui_render, const Rectangle & rect, DWORD color);

		virtual void Draw(UIRender * ui_render, const Rectangle & rect, DWORD color, const Rectangle & clip);
	};

	class ControlSkin;

	typedef boost::shared_ptr<ControlSkin> ControlSkinPtr;

	class Wav;

	class ControlSkin : public boost::enable_shared_from_this<ControlSkin>
	{
	public:
		D3DCOLOR m_Color;

		ControlImagePtr m_Image;

		std::string m_VisibleShowSoundPath;

		boost::shared_ptr<Wav> m_VisibleShowSound;

		std::string m_VisibleHideSoundPath;

		boost::shared_ptr<Wav> m_VisibleHideSound;

		std::string m_MouseEnterSoundPath;

		boost::shared_ptr<Wav> m_MouseEnterSound;

		std::string m_MouseLeaveSoundPath;

		boost::shared_ptr<Wav> m_MouseLeaveSound;

		std::string m_MouseClickSoundPath;

		boost::shared_ptr<Wav> m_MouseClickSound;

		bool m_Requested;

	public:
		ControlSkin(void);

		virtual ~ControlSkin(void);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_Color);
			ar & BOOST_SERIALIZATION_NVP(m_Image);
			ar & BOOST_SERIALIZATION_NVP(m_VisibleShowSoundPath);
			ar & BOOST_SERIALIZATION_NVP(m_VisibleHideSoundPath);
			ar & BOOST_SERIALIZATION_NVP(m_MouseEnterSoundPath);
			ar & BOOST_SERIALIZATION_NVP(m_MouseLeaveSoundPath);
			ar & BOOST_SERIALIZATION_NVP(m_MouseClickSoundPath);
		}

		bool IsRequested(void) const
		{
			return m_Requested;
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void OnVisibleShowSoundReady(my::DeviceResourceBasePtr res);

		void OnVisibleHideSoundReady(my::DeviceResourceBasePtr res);

		void OnMouseEnterSoundReady(my::DeviceResourceBasePtr res);

		void OnMouseLeaveSoundReady(my::DeviceResourceBasePtr res);

		void OnMouseClickSoundReady(my::DeviceResourceBasePtr res);

		void DrawImage(UIRender * ui_render, const ControlImagePtr & Image, const Rectangle & rect, DWORD color);

		void DrawImage(UIRender * ui_render, const ControlImagePtr & Image, const Rectangle & rect, DWORD color, const Rectangle & clip);

		virtual ControlSkinPtr Clone(void) const;
	};

	class Control;

	typedef boost::shared_ptr<Control> ControlPtr;

	class ControlEventArg : public EventArg
	{
	public:
		Control * self;

		ControlEventArg(Control * _self)
			: self(_self)
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

		VisibleEventArg(Control * _self, bool _Visible)
			: ControlEventArg(_self)
			, Visible(_Visible)
		{
		}
	};

	class FocusEventArg : public ControlEventArg
	{
	public:
		bool Focused;

		FocusEventArg(Control * _self, bool _Focused)
			: ControlEventArg(_self)
			, Focused(_Focused)
		{
		}
	};

	class MouseEventArg : public ControlEventArg
	{
	public:
		Vector2 pt;

		MouseEventArg(Control * _self, const Vector2 & _pt)
			: ControlEventArg(_self)
			, pt(_pt)
		{
		}
	};

	class Dialog;

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
			ControlTypeScript,
		};

		typedef std::vector<ControlPtr> ControlPtrList;

		ControlPtrList m_Childs;

		Control * m_Parent;

		static Control * s_FocusControl;

		static Control * s_CaptureControl;

		static Control * s_MouseOverControl;

		bool m_Requested;

		bool m_bEnabled;

		bool m_bVisible;

		UINT m_nHotkey;

		UDim m_x;

		UDim m_y;

		UDim m_Width;

		UDim m_Height;

		Rectangle m_Rect;

		ControlSkinPtr m_Skin;

		bool m_bPressed;

		EventFunction m_EventVisibleChanged;

		EventFunction m_EventFocusChanged;

		EventFunction m_EventMouseEnter;

		EventFunction m_EventMouseLeave;

		EventFunction m_EventMouseClick;

	protected:
		Control(void)
			: m_Requested(false)
			, m_bEnabled(true)
			, m_bVisible(true)
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

		virtual DWORD GetControlType(void) const
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

		static Control * GetMouseOverControl(void)
		{
			return s_MouseOverControl;
		}

		static void SetMouseOverControl(Control * control, const Vector2 & pt);

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void) const;

		virtual void OnFocusIn(void);

		virtual void OnFocusOut(void);

		virtual void OnMouseEnter(const Vector2 & pt);

		virtual void OnMouseLeave(const Vector2 & pt);

		virtual void OnHotkey(void);

		virtual bool HitTest(const Vector2 & pt) const;

		virtual void OnLayout(void);

		virtual void SetEnabled(bool bEnabled);

		virtual bool GetEnabled(void) const;

		virtual bool GetEnabledHierarchy(void) const;

		virtual void SetVisible(bool bVisible);

		virtual bool GetVisible(void) const;

		virtual bool GetVisibleHierarchy(void) const;

		virtual void SetFocused(bool bFocused);

		virtual bool GetFocused(void) const;

		virtual void SetCaptured(bool bCaptured);

		virtual bool GetCaptured(void) const;

		virtual void SetMouseOver(bool bMouseOver);

		virtual bool GetMouseOver(void) const;

		virtual bool RayToWorld(const Ray & ray, Vector2 & ptWorld) const;

		virtual void InsertControl(unsigned int i, ControlPtr control);

		virtual void RemoveControl(unsigned int i);

		unsigned int GetChildNum(void) const;

		unsigned int GetSiblingId(void) const;

		void SetSiblingId(unsigned int i);

		virtual void ClearAllControl(void);

		bool ContainsControl(Control * control) const;

		virtual Control * GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible);

		int GetChildAtFrustum(const my::Frustum & ftm, bool bIgnoreVisible, std::vector<Control *> & childs);

		Dialog * GetTopControl(void);

		bool SetFocusRecursive(void);

		virtual void GetNearestControl(const Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist, Control * recursive_self);

		void SetHotkey(UINT nHotkey);

		UINT GetHotkey(void);
	};

	class StaticSkin : public ControlSkin
	{
	public:
		std::string m_FontPath;

		int m_FontHeight;

		int m_FontFaceIndex;

		FontPtr m_Font;

		D3DCOLOR m_TextColor;

		Font::Align m_TextAlign;

		D3DCOLOR m_TextOutlineColor;

		float m_TextOutlineWidth;

	public:
		StaticSkin(void);

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
			ar & BOOST_SERIALIZATION_NVP(m_FontPath);
			ar & BOOST_SERIALIZATION_NVP(m_FontHeight);
			ar & BOOST_SERIALIZATION_NVP(m_FontFaceIndex);
			ar & BOOST_SERIALIZATION_NVP(m_TextColor);
			ar & BOOST_SERIALIZATION_NVP(m_TextAlign);
			ar & BOOST_SERIALIZATION_NVP(m_TextOutlineColor);
			ar & BOOST_SERIALIZATION_NVP(m_TextOutlineWidth);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		void OnFontReady(my::DeviceResourceBasePtr res);

		void DrawString(UIRender * ui_render, const wchar_t * str, const Rectangle & rect);
	};

	typedef boost::shared_ptr<StaticSkin> StaticSkinPtr;

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

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeStatic;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);
	};

	typedef boost::shared_ptr<Static> StaticPtr;

	class ProgressBarSkin : public StaticSkin
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
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StaticSkin);
			ar & BOOST_SERIALIZATION_NVP(m_ForegroundImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);
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

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeProgressBar;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);
	};

	typedef boost::shared_ptr<ProgressBar> ProgressBarPtr;

	class ButtonSkin : public StaticSkin
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
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StaticSkin);
			ar & BOOST_SERIALIZATION_NVP(m_DisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_PressedImage);
			ar & BOOST_SERIALIZATION_NVP(m_MouseOverImage);
			ar & BOOST_SERIALIZATION_NVP(m_PressedOffset);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);
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
		}

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeButton;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void) const;
	};

	typedef boost::shared_ptr<Button> ButtonPtr;

	class EditBoxSkin : public StaticSkin
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
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StaticSkin);
			ar & BOOST_SERIALIZATION_NVP(m_DisabledImage);
			ar & BOOST_SERIALIZATION_NVP(m_FocusedImage);
			ar & BOOST_SERIALIZATION_NVP(m_SelBkColor);
			ar & BOOST_SERIALIZATION_NVP(m_CaretColor);
			ar & BOOST_SERIALIZATION_NVP(m_CaretImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);
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

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeEditBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual bool CanHaveFocus(void) const;

		virtual void OnFocusIn(void);

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

		virtual DWORD GetControlType(void) const
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

	protected:
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
		}

	public:
		ScrollBar(const char * Name)
			: Control(Name)
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
			ar & BOOST_SERIALIZATION_NVP(m_nEnd);
		}

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeScrollBar;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

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

		virtual DWORD GetControlType(void) const
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
			ar & BOOST_SERIALIZATION_NVP(m_DropdownItemTextColor);
			ar & BOOST_SERIALIZATION_NVP(m_DropdownItemTextAlign);
			ar & BOOST_SERIALIZATION_NVP(m_DropdownItemMouseOverImage);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);
	};

	typedef boost::shared_ptr<ComboBoxSkin> ComboBoxSkinPtr;

	class ComboBoxItem
	{
	public:
		const std::wstring strText;

		void * pData;

		explicit ComboBoxItem(const std::wstring & _strText)
			: strText(_strText)
			, pData(NULL)
		{
		}
	};

	class ComboBox : public Button
	{
	public:
		Vector2 m_DropdownSize;

		Rectangle m_DropdownRect;

		ScrollBarPtr m_ScrollBar;

		float m_ScrollbarWidth;

		float m_ScrollbarUpDownBtnHeight;

		Vector4 m_Border;

		typedef std::vector<ComboBoxItem> ComboBoxItemList;

		ComboBoxItemList m_Items;

		float m_ItemHeight;

		int m_iFocused;

		int m_iSelected;

		EventFunction m_EventSelectionChanged;

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
		{
		}

	public:
		ComboBox(const char * Name);

		~ComboBox(void)
		{
			if (m_ScrollBar)
			{
				m_ScrollBar->m_Parent = NULL;
			}
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

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeComboBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnFocusOut(void);

		virtual bool HitTest(const Vector2 & pt) const;

		virtual void OnLayout(void);

		virtual Control * GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible);

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
		ListBoxSkin(void)
		{
		}

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ControlSkin);
		}

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);
	};

	typedef boost::shared_ptr<ListBoxSkin> ListBoxSkinPtr;

	class ListBox : public Control
	{
	public:
		ScrollBarPtr m_ScrollBar;

		float m_ScrollbarWidth;

		float m_ScrollbarUpDownBtnHeight;

		Vector2 m_ItemSize;

		int m_ItemColumn;

	protected:
		ListBox(void)
			: m_ScrollbarWidth(20)
			, m_ScrollbarUpDownBtnHeight(20)
			, m_ItemSize(50, 50)
			, m_ItemColumn(1)
		{
		}

	public:
		ListBox(const char* Name);

		~ListBox(void)
		{
			if (m_ScrollBar)
			{
				m_ScrollBar->m_Parent = NULL;
			}
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

		virtual void RequestResource(void);

		virtual void ReleaseResource(void);

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeListBox;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void OnLayout(void);

		virtual void InsertControl(unsigned int i, ControlPtr control);

		virtual void RemoveControl(unsigned int i);

		virtual void ClearAllControl(void);

		virtual Control * GetChildAtPoint(const Vector2 & pt, bool bIgnoreVisible);

		virtual void GetNearestControl(const Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist, Control * recursive_self);

		void SetScrollbarWidth(float Width);

		float GetScrollbarWidth(void) const;

		void SetScrollbarUpDownBtnHeight(float Height);

		float GetScrollbarUpDownBtnHeight(void) const;

		void SetItemSize(const my::Vector2 & ItemSize);

		const my::Vector2 & GetItemSize(void) const;
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

		boost::weak_ptr<Control> m_LastFocusControl;

	protected:
		Dialog(void)
			: m_Manager(NULL)
			, m_World(Matrix4::Identity())
			, m_EnableDrag(false)
			, m_bMouseDrag(false)
			, m_MouseOffset(0,0)
		{
		}

	public:
		Dialog(const char * Name)
			: Control(Name)
			, m_Manager(NULL)
			, m_World(Matrix4::Identity())
			, m_EnableDrag(false)
			, m_bMouseDrag(false)
			, m_MouseOffset(0, 0)
		{
		}

		virtual ~Dialog(void);

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control);
			ar & BOOST_SERIALIZATION_NVP(m_World);
			ar & BOOST_SERIALIZATION_NVP(m_EnableDrag);
		}

		virtual DWORD GetControlType(void) const
		{
			return ControlTypeDialog;
		}

		virtual void Draw(UIRender * ui_render, float fElapsedTime, const Vector2 & Offset, const Vector2 & Size);

		virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual bool HandleMouse(UINT uMsg, const Vector2 & pt, WPARAM wParam, LPARAM lParam);

		virtual void SetVisible(bool bVisible);

		virtual bool RayToWorld(const Ray & ray, Vector2 & ptWorld) const;

		virtual void GetNearestControl(const Rectangle & rect, DWORD dir, Control ** nearest_ctrl, float & nearest_ctrl_dist, Control * recursive_self);

		virtual void MoveToFront(void) const;
	};

	class DialogMgr : public SingletonInstance<DialogMgr>
	{
	public:
		typedef std::list<Dialog *> DialogList;

		DialogList m_DlgList;

		Vector3 m_Eye;

		Matrix4 m_View;

		Matrix4 m_Proj;

		Matrix4 m_ViewProj;

		Matrix4 m_InverseViewProj;

		typedef boost::function<void (UIRender *, float, const Vector2 &)> UIPassObj;

		typedef std::vector<UIPassObj> UIPassObjList;

		UIPassObjList m_UIPassObjs;

	public:
		DialogMgr(void)
		{
			SetDlgViewport(Vector2(800,600), D3DXToRadian(75.0f));
		}

		~DialogMgr(void)
		{
			_ASSERT(m_UIPassObjs.empty());
		}

		void SetDlgViewport(const Vector2 & Viewport, float fov);

		Vector2 GetDlgViewport(void) const;

		Ray CalculateRay(const Vector2 & pt, const Vector2 & dim);

		void Draw(UIRender * ui_render, double fTime, float fElapsedTime, const Vector2 & Viewport);

		bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void InsertDlg(Dialog * dlg);

		void RemoveDlg(Dialog * dlg);

		void RemoveAllDlg();
	};
}

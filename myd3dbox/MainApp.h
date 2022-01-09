
// myd3dbox.h : main header file for the myd3dbox application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "RenderPipeline.h"
#include "PhysxContext.h"

// CMainApp:
// See myd3dbox.cpp for the implementation of this class
//

class CMainApp : public CWinAppEx
	, public my::D3DContext
	, public my::FontLibrary
	, public my::ResourceMgr
	, public RenderPipeline
	, public PhysxSdk
{
public:
	CMainApp();
	~CMainApp();

	HRESULT hr;
	my::UIRenderPtr m_UIRender;
	my::FontPtr m_Font;

	// cfg
	float default_fov;
	float default_viewed_dist;
	bool default_io_thread_num;
	bool default_load_shader_cache;
	std::string default_font_path;
	int default_font_height;
	int default_font_face_index;
	std::string default_texture;
	std::string default_normal_texture;
	std::string default_specular_texture;
	std::string default_shader;
	std::string default_terrain_shader;
	D3DCOLOR default_dialog_color;
	std::string default_dialog_img;
	my::Rectangle default_dialog_img_rect;
	my::Vector4 default_dialog_img_border;
	D3DCOLOR default_static_text_color;
	my::Font::Align default_static_text_align;
	std::string default_progressbar_img;
	my::Rectangle default_progressbar_img_rect;
	my::Vector4 default_progressbar_img_border;
	D3DCOLOR default_progressbar_text_color;
	my::Font::Align default_progressbar_text_align;
	std::string default_progressbar_foregroundimg;
	my::Rectangle default_progressbar_foregroundimg_rect;
	my::Vector4 default_progressbar_foregroundimg_border;
	std::string default_button_img;
	my::Rectangle default_button_img_rect;
	my::Vector4 default_button_img_border;
	D3DCOLOR default_button_text_color;
	my::Font::Align default_button_text_align;
	my::Vector2 default_button_pressed_offset;
	std::string default_button_disabledimg;
	my::Rectangle default_button_disabledimg_rect;
	my::Vector4 default_button_disabledimg_border;
	std::string default_button_pressedimg;
	my::Rectangle default_button_pressedimg_rect;
	my::Vector4 default_button_pressedimg_border;
	std::string default_button_mouseoverimg;
	my::Rectangle default_button_mouseoverimg_rect;
	my::Vector4 default_button_mouseoverimg_border;
	D3DCOLOR default_editbox_color;
	D3DCOLOR default_editbox_text_color;
	my::Font::Align default_editbox_text_align;
	std::string default_editbox_img;
	my::Rectangle default_editbox_img_rect;
	my::Vector4 default_editbox_img_border;
	std::string default_editbox_disabledimg;
	my::Rectangle default_editbox_disabledimg_rect;
	my::Vector4 default_editbox_disabledimg_border;
	std::string default_editbox_focusedimg;
	my::Rectangle default_editbox_focusedimg_rect;
	my::Vector4 default_editbox_focusedimg_border;
	D3DCOLOR default_editbox_sel_bk_color;
	D3DCOLOR default_editbox_caret_color;
	std::string default_editbox_caretimg;
	my::Rectangle default_editbox_caretimg_rect;
	my::Vector4 default_editbox_caretimg_border;
	std::string default_checkbox_img;
	my::Rectangle default_checkbox_img_rect;
	my::Vector4 default_checkbox_img_border;
	D3DCOLOR default_checkbox_text_color;
	my::Font::Align default_checkbox_text_align;
	my::Vector2 default_checkbox_pressed_offset;
	std::string default_checkbox_disabledimg;
	my::Rectangle default_checkbox_disabledimg_rect;
	my::Vector4 default_checkbox_disabledimg_border;
	std::string default_checkbox_pressedimg;
	my::Rectangle default_checkbox_pressedimg_rect;
	my::Vector4 default_checkbox_pressedimg_border;
	std::string default_checkbox_mouseoverimg;
	my::Rectangle default_checkbox_mouseoverimg_rect;
	my::Vector4 default_checkbox_mouseoverimg_border;
	std::string default_combobox_img;
	my::Rectangle default_combobox_img_rect;
	my::Vector4 default_combobox_img_border;
	D3DCOLOR default_combobox_text_color;
	my::Font::Align default_combobox_text_align;
	my::Vector2 default_combobox_pressed_offset;
	std::string default_combobox_disabledimg;
	my::Rectangle default_combobox_disabledimg_rect;
	my::Vector4 default_combobox_disabledimg_border;
	std::string default_combobox_pressedimg;
	my::Rectangle default_combobox_pressedimg_rect;
	my::Vector4 default_combobox_pressedimg_border;
	std::string default_combobox_mouseoverimg;
	my::Rectangle default_combobox_mouseoverimg_rect;
	my::Vector4 default_combobox_mouseoverimg_border;
	std::string default_combobox_dropdownimg;
	my::Rectangle default_combobox_dropdownimg_rect;
	my::Vector4 default_combobox_dropdownimg_border;
	D3DCOLOR default_combobox_dropdownitem_text_color;
	my::Font::Align default_combobox_dropdownitem_text_align;
	std::string default_combobox_dropdownitem_mouseoverimg;
	my::Rectangle default_combobox_dropdownitem_mouseoverimg_rect;
	my::Vector4 default_combobox_dropdownitem_mouseoverimg_border;
	std::string default_combobox_scrollbarupbtn_normalimg;
	my::Rectangle default_combobox_scrollbarupbtn_normalimg_rect;
	my::Vector4 default_combobox_scrollbarupbtn_normalimg_border;
	std::string default_combobox_scrollbarupbtn_disabledimg;
	my::Rectangle default_combobox_scrollbarupbtn_disabledimg_rect;
	my::Vector4 default_combobox_scrollbarupbtn_disabledimg_border;
	std::string default_combobox_scrollbardownbtn_normalimg;
	my::Rectangle default_combobox_scrollbardownbtn_normalimg_rect;
	my::Vector4 default_combobox_scrollbardownbtn_normalimg_border;
	std::string default_combobox_scrollbardownbtn_disabledimg;
	my::Rectangle default_combobox_scrollbardownbtn_disabledimg_rect;
	my::Vector4 default_combobox_scrollbardownbtn_disabledimg_border;
	std::string default_combobox_scrollbarthumbbtn_normalimg;
	my::Rectangle default_combobox_scrollbarthumbbtn_normalimg_rect;
	my::Vector4 default_combobox_scrollbarthumbbtn_normalimg_border;
	std::string default_combobox_scrollbar_img;
	my::Rectangle default_combobox_scrollbar_img_rect;
	my::Vector4 default_combobox_scrollbar_img_border;
	std::string default_listbox_img;
	my::Rectangle default_listbox_img_rect;
	my::Vector4 default_listbox_img_border;
	D3DCOLOR default_listbox_text_color;
	my::Font::Align default_listbox_text_align;
	std::string default_listbox_scrollbarupbtn_normalimg;
	my::Rectangle default_listbox_scrollbarupbtn_normalimg_rect;
	my::Vector4 default_listbox_scrollbarupbtn_normalimg_border;
	std::string default_listbox_scrollbarupbtn_disabledimg;
	my::Rectangle default_listbox_scrollbarupbtn_disabledimg_rect;
	my::Vector4 default_listbox_scrollbarupbtn_disabledimg_border;
	std::string default_listbox_scrollbardownbtn_normalimg;
	my::Rectangle default_listbox_scrollbardownbtn_normalimg_rect;
	my::Vector4 default_listbox_scrollbardownbtn_normalimg_border;
	std::string default_listbox_scrollbardownbtn_disabledimg;
	my::Rectangle default_listbox_scrollbardownbtn_disabledimg_rect;
	my::Vector4 default_listbox_scrollbardownbtn_disabledimg_border;
	std::string default_listbox_scrollbarthumbbtn_normalimg;
	my::Rectangle default_listbox_scrollbarthumbbtn_normalimg_rect;
	my::Vector4 default_listbox_scrollbarthumbbtn_normalimg_border;
	std::string default_listbox_scrollbar_img;
	my::Rectangle default_listbox_scrollbar_img_rect;
	my::Vector4 default_listbox_scrollbar_img_border;
	D3DXHANDLE technique_RenderSceneColor;
	D3DXHANDLE handle_MeshColor;
	BOOL m_bNeedDraw;
	my::EventSignal m_EventNamedObjectCreate;
	my::EventSignal m_EventNamedObjectDestroy;

	struct NamedObjectEventArgs : public my::EventArg
	{
	public:
		my::NamedObject* pObj;

		NamedObjectEventArgs(my::NamedObject* _pObj)
			: pObj(_pObj)
		{
		}
	};

	BOOL CreateD3DDevice(HWND hWnd);
	BOOL ResetD3DDevice(void);
	void DestroyD3DDevice(void);

// Overrides
public:
	virtual BOOL InitInstance();
	virtual HRESULT OnCreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);
	virtual HRESULT OnResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc);
	virtual void OnLostDevice(void);
	virtual void OnDestroyDevice(void);

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual void OnNamedObjectCreate(my::NamedObject* Object);
	virtual void OnNamedObjectDestroy(my::NamedObject* Object);
};

extern CMainApp theApp;

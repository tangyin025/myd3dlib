// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "RenderPipeline.h"
#include "PhysxContext.h"
#include "SceneContext.h"
#include "DictionaryNode.h"

// CMainApp:
// See myd3dbox.cpp for the implementation of this class
//

class CMainApp : public CWinAppEx
	, public my::D3DContext
	, public my::FontLibrary
	, public my::InputMgr
	, public my::ResourceMgr
	, public my::ParallelTaskManager
	, public RenderPipeline
	, public PhysxSdk
{
public:
	CMainApp();
	~CMainApp();

	HRESULT hr;
	my::UIRenderPtr m_UIRender;
	my::FontPtr m_Font;
	DictionaryNode m_Dicts;

	// cfg
	float default_fov;
	unsigned int default_physx_scene_flags;
	my::Vector3 default_physx_scene_gravity;
	unsigned int default_physx_shape_filterword0;
	float default_physx_joint_localframe;
	float default_physx_joint_limits;
	int default_io_thread_num;
	bool default_load_shader_cache;
	int default_remaining_actor_max;
	float default_grid_length;
	float default_grid_lines_every;
	unsigned int default_grid_subdivisions;
	D3DCOLOR default_grid_color;
	D3DCOLOR default_grid_axis_color;
	float default_ui_grid_size;
	BOOL default_snap_to_grid;
	std::string default_script_pattern;
	float default_emitter_chunk_width;
	std::string default_font_path;
	int default_font_height;
	int default_font_face_index;
	std::string default_dictionary_file;
	std::string default_texture;
	std::string default_normal_texture;
	std::string default_specular_texture;
	std::string default_shader;
	std::string default_terrain_shader;
	float default_terrain_max_height;
	float default_terrain_water_level;
	D3DCOLOR default_dialog_color;
	std::string default_dialog_img;
	CRect default_dialog_img_rect;
	CRect default_dialog_img_border;
	std::string default_static_img;
	CRect default_static_img_rect;
	CRect default_static_img_border;
	D3DCOLOR default_static_text_color;
	my::Font::Align default_static_text_align;
	std::string default_progressbar_img;
	CRect default_progressbar_img_rect;
	CRect default_progressbar_img_border;
	D3DCOLOR default_progressbar_text_color;
	my::Font::Align default_progressbar_text_align;
	std::string default_progressbar_foregroundimg;
	CRect default_progressbar_foregroundimg_rect;
	CRect default_progressbar_foregroundimg_border;
	std::string default_button_img;
	CRect default_button_img_rect;
	CRect default_button_img_border;
	D3DCOLOR default_button_text_color;
	my::Font::Align default_button_text_align;
	my::Vector2 default_button_pressed_offset;
	std::string default_button_disabledimg;
	CRect default_button_disabledimg_rect;
	CRect default_button_disabledimg_border;
	std::string default_button_pressedimg;
	CRect default_button_pressedimg_rect;
	CRect default_button_pressedimg_border;
	std::string default_button_mouseoverimg;
	CRect default_button_mouseoverimg_rect;
	CRect default_button_mouseoverimg_border;
	D3DCOLOR default_editbox_color;
	D3DCOLOR default_editbox_text_color;
	my::Font::Align default_editbox_text_align;
	std::string default_editbox_img;
	CRect default_editbox_img_rect;
	CRect default_editbox_img_border;
	std::string default_editbox_disabledimg;
	CRect default_editbox_disabledimg_rect;
	CRect default_editbox_disabledimg_border;
	std::string default_editbox_focusedimg;
	CRect default_editbox_focusedimg_rect;
	CRect default_editbox_focusedimg_border;
	D3DCOLOR default_editbox_sel_bk_color;
	D3DCOLOR default_editbox_caret_color;
	std::string default_editbox_caretimg;
	CRect default_editbox_caretimg_rect;
	CRect default_editbox_caretimg_border;
	std::string default_checkbox_img;
	CRect default_checkbox_img_rect;
	CRect default_checkbox_img_border;
	D3DCOLOR default_checkbox_text_color;
	my::Font::Align default_checkbox_text_align;
	my::Vector2 default_checkbox_pressed_offset;
	std::string default_checkbox_disabledimg;
	CRect default_checkbox_disabledimg_rect;
	CRect default_checkbox_disabledimg_border;
	std::string default_checkbox_pressedimg;
	CRect default_checkbox_pressedimg_rect;
	CRect default_checkbox_pressedimg_border;
	std::string default_checkbox_mouseoverimg;
	CRect default_checkbox_mouseoverimg_rect;
	CRect default_checkbox_mouseoverimg_border;
	std::string default_combobox_img;
	CRect default_combobox_img_rect;
	CRect default_combobox_img_border;
	D3DCOLOR default_combobox_text_color;
	my::Font::Align default_combobox_text_align;
	my::Vector2 default_combobox_pressed_offset;
	std::string default_combobox_disabledimg;
	CRect default_combobox_disabledimg_rect;
	CRect default_combobox_disabledimg_border;
	std::string default_combobox_pressedimg;
	CRect default_combobox_pressedimg_rect;
	CRect default_combobox_pressedimg_border;
	std::string default_combobox_mouseoverimg;
	CRect default_combobox_mouseoverimg_rect;
	CRect default_combobox_mouseoverimg_border;
	std::string default_combobox_dropdownimg;
	CRect default_combobox_dropdownimg_rect;
	CRect default_combobox_dropdownimg_border;
	D3DCOLOR default_combobox_dropdownitem_text_color;
	my::Font::Align default_combobox_dropdownitem_text_align;
	std::string default_combobox_dropdownitem_mouseoverimg;
	CRect default_combobox_dropdownitem_mouseoverimg_rect;
	CRect default_combobox_dropdownitem_mouseoverimg_border;
	std::string default_combobox_scrollbarupbtn_normalimg;
	CRect default_combobox_scrollbarupbtn_normalimg_rect;
	CRect default_combobox_scrollbarupbtn_normalimg_border;
	std::string default_combobox_scrollbarupbtn_disabledimg;
	CRect default_combobox_scrollbarupbtn_disabledimg_rect;
	CRect default_combobox_scrollbarupbtn_disabledimg_border;
	std::string default_combobox_scrollbardownbtn_normalimg;
	CRect default_combobox_scrollbardownbtn_normalimg_rect;
	CRect default_combobox_scrollbardownbtn_normalimg_border;
	std::string default_combobox_scrollbardownbtn_disabledimg;
	CRect default_combobox_scrollbardownbtn_disabledimg_rect;
	CRect default_combobox_scrollbardownbtn_disabledimg_border;
	std::string default_combobox_scrollbarthumbbtn_normalimg;
	CRect default_combobox_scrollbarthumbbtn_normalimg_rect;
	CRect default_combobox_scrollbarthumbbtn_normalimg_border;
	std::string default_combobox_scrollbar_img;
	CRect default_combobox_scrollbar_img_rect;
	CRect default_combobox_scrollbar_img_border;
	std::string default_listbox_img;
	CRect default_listbox_img_rect;
	CRect default_listbox_img_border;
	std::string default_listbox_scrollbarupbtn_normalimg;
	CRect default_listbox_scrollbarupbtn_normalimg_rect;
	CRect default_listbox_scrollbarupbtn_normalimg_border;
	std::string default_listbox_scrollbarupbtn_disabledimg;
	CRect default_listbox_scrollbarupbtn_disabledimg_rect;
	CRect default_listbox_scrollbarupbtn_disabledimg_border;
	std::string default_listbox_scrollbardownbtn_normalimg;
	CRect default_listbox_scrollbardownbtn_normalimg_rect;
	CRect default_listbox_scrollbardownbtn_normalimg_border;
	std::string default_listbox_scrollbardownbtn_disabledimg;
	CRect default_listbox_scrollbardownbtn_disabledimg_rect;
	CRect default_listbox_scrollbardownbtn_disabledimg_border;
	std::string default_listbox_scrollbarthumbbtn_normalimg;
	CRect default_listbox_scrollbarthumbbtn_normalimg_rect;
	CRect default_listbox_scrollbarthumbbtn_normalimg_border;
	std::string default_listbox_scrollbar_img;
	CRect default_listbox_scrollbar_img_rect;
	CRect default_listbox_scrollbar_img_border;
	float default_player_scale;
	float default_player_height;
	float default_player_radius;
	float default_player_contact_offset;
	float default_player_step_offset;
	float default_player_run_speed;
	float default_player_breaking_speed;
	float default_player_seek_force;
	float default_player_swim_depth;
	float default_player_swim_force;
	float default_player_look_distance;
	float default_player_collision_height;
	std::string default_player_mesh;
	std::string default_player_skel;
	unsigned int default_player_water_filterword0;
	float default_player_water_buoyancy;
	float default_player_water_drag;
	float default_player_air_drag;
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
	virtual const std::wstring & OnControlTranslate(const std::wstring& wstr);

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


// myd3dbox.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
extern "C"
{
#include <lua.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainApp

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMainApp::OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CMainApp construction

CMainApp::CMainApp()
	: InputMgr(0)
{
	m_bNeedDraw = FALSE;
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MFCApplication1.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_UIRender.reset(new my::UIRender());
}

CMainApp::~CMainApp()
{
}

// The one and only CMainApp object

CMainApp theApp;

BOOL CMainApp::CreateD3DDevice(HWND hWnd)
{
	ZeroMemory(&m_DeviceSettings, sizeof(m_DeviceSettings));
	m_DeviceSettings.AdapterOrdinal = D3DADAPTER_DEFAULT;
	m_DeviceSettings.DeviceType = D3DDEVTYPE_HAL;
	m_DeviceSettings.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	m_DeviceSettings.pp.Windowed = TRUE;
	m_DeviceSettings.pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_DeviceSettings.pp.BackBufferFormat = D3DFMT_UNKNOWN;
	m_DeviceSettings.pp.hDeviceWindow = hWnd;
	m_DeviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr;
	if(FAILED(hr = m_d3d9->CreateDevice(
		m_DeviceSettings.AdapterOrdinal,
		m_DeviceSettings.DeviceType,
		hWnd,
		m_DeviceSettings.BehaviorFlags,
		&m_DeviceSettings.pp,
		&m_d3dDevice)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_DeviceObjectsCreated = true;

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if (FAILED(hr = OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_DeviceObjectsReset = true;

	if (FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	return TRUE;
}

BOOL CMainApp::ResetD3DDevice(void)
{
	if(m_DeviceObjectsReset)
	{
		OnLostDevice();
		m_DeviceObjectsReset = false;
	}

	if(FAILED(hr = m_d3dDevice->Reset(&m_DeviceSettings.pp)))
	{
		TRACE(my::D3DException::Translate(hr));
		return FALSE;
	}

	m_DeviceObjectsReset = true;

	CComPtr<IDirect3DSurface9> BackBuffer;
	V(m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer));
	V(BackBuffer->GetDesc(&m_BackBufferSurfaceDesc));

	if (FAILED(hr = OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		OnLostDevice();
		m_DeviceObjectsReset = false;
		return FALSE;
	}

	return TRUE;
}

void CMainApp::DestroyD3DDevice(void)
{
	if (m_DeviceObjectsReset)
	{
		OnLostDevice();
		m_DeviceObjectsReset = false;
	}

	if(m_DeviceObjectsCreated)
	{
		OnDestroyDevice();

		UINT references = m_d3dDevice.Detach()->Release();
#ifdef _DEBUG
		if(references > 0)
		{
			CString msg;
			msg.Format(_T("no zero reference count: %u"), references);
			AfxMessageBox(msg);
		}
#endif
		m_DeviceObjectsCreated = false;
	}
}

namespace boost
{
	namespace program_options
	{
		void validate(boost::any& v,
			const std::vector<std::string>& values,
			my::Vector2*, int)
		{
			static boost::regex r("([+-]?(\\d*[.])?\\d+),([+-]?(\\d*[.])?\\d+)");

			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			boost::smatch match;
			if (boost::regex_match(s, match, r)) {
				v = boost::any(my::Vector2(
					boost::lexical_cast<float>(match[1]),
					boost::lexical_cast<float>(match[3])));
			}
			else {
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
		}

		void validate(boost::any& v,
			const std::vector<std::string>& values,
			my::Vector3*, int)
		{
			static boost::regex r("([+-]?(\\d*[.])?\\d+),([+-]?(\\d*[.])?\\d+),([+-]?(\\d*[.])?\\d+)");

			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			boost::smatch match;
			if (boost::regex_match(s, match, r)) {
				v = boost::any(my::Vector3(
					boost::lexical_cast<float>(match[1]),
					boost::lexical_cast<float>(match[3]),
					boost::lexical_cast<float>(match[5])));
			}
			else {
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
		}

		void validate(boost::any& v,
			const std::vector<std::string>& values,
			CRect*, int)
		{
			static boost::regex r("(\\d+),(\\d+),(\\d+),(\\d+)");

			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			boost::smatch match;
			if (boost::regex_match(s, match, r)) {
				v = boost::any(CRect(
					boost::lexical_cast<int>(match[1]),
					boost::lexical_cast<int>(match[2]),
					boost::lexical_cast<int>(match[3]),
					boost::lexical_cast<int>(match[4])));
			}
			else {
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
		}

		void validate(boost::any& v,
			const std::vector<std::string>& values,
			D3DCOLOR*, int)
		{
			static boost::regex r("(\\d+),(\\d+),(\\d+),(\\d+)");

			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			boost::smatch match;
			if (boost::regex_match(s, match, r)) {
				v = boost::any(D3DCOLOR_ARGB(
					boost::lexical_cast<int>(match[1]),
					boost::lexical_cast<int>(match[2]),
					boost::lexical_cast<int>(match[3]),
					boost::lexical_cast<int>(match[4])));
			}
			else {
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
		}

		void validate(boost::any& v,
			const std::vector<std::string>& values,
			my::Font::Align*, int)
		{
			// Make sure no previous assignment to 'a' was made.
			boost::program_options::validators::check_first_occurrence(v);
			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = boost::program_options::validators::get_single_string(values);

			// Do regex match and convert the interesting part to
			// int.
			if (boost::iequals(s, "lefttop")) {
				v = my::Font::AlignLeftTop;
			}
			else if (boost::iequals(s, "centertop")) {
				v = my::Font::AlignCenterTop;
			}
			else if (boost::iequals(s, "righttop")) {
				v = my::Font::AlignRightTop;
			}
			else if (boost::iequals(s, "leftmiddle")) {
				v = my::Font::AlignLeftMiddle;
			}
			else if (boost::iequals(s, "centermiddle")) {
				v = my::Font::AlignCenterMiddle;
			}
			else if (boost::iequals(s, "rightmiddle")) {
				v = my::Font::AlignRightMiddle;
			}
			else if (boost::iequals(s, "leftbottom")) {
				v = my::Font::AlignLeftBottom;
			}
			else if (boost::iequals(s, "centerbottom")) {
				v = my::Font::AlignCenterBottom;
			}
			else if (boost::iequals(s, "rightbottom")) {
				v = my::Font::AlignRightBottom;
			}
			else {
				throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
			}
		}
	};
};

// CMainApp initialization

BOOL CMainApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	char buff[MAX_PATH];
	GetModuleFileNameA(NULL, buff, _countof(buff));
	std::string cfg_file(PathFindFileNameA(buff), PathFindExtensionA(buff));

	boost::program_options::options_description desc("Options");
	std::vector<std::string> path_list;
	desc.add_options()
		("path", boost::program_options::value(&path_list)->default_value(boost::assign::list_of("..\\demo2_3\\Media")("Media"), ""), "Path")
		("shaderinclude", boost::program_options::value(&m_SystemIncludes)->default_value(boost::assign::list_of("shader"), ""), "Shader Include")
		("default_fov", boost::program_options::value(&default_fov)->default_value(60.0f), "Default fov")
		("default_physx_frame_interval", boost::program_options::value(&m_FrameInterval)->default_value(1 / 60.0f), "Default physx frame interval")
		("default_max_lowed_timestep", boost::program_options::value(&m_MaxAllowedTimestep)->default_value(0.1f), "Default max allowed timestep")
		("default_physx_scene_flags", boost::program_options::value(&default_physx_scene_flags)->default_value(physx::PxSceneFlag::eENABLE_PCM | physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS | physx::PxSceneFlag::eENABLE_CCD), "Default physx scene flags")
		("default_physx_scene_gravity", boost::program_options::value(&default_physx_scene_gravity)->default_value(my::Vector3::Gravity, ""), "Default physx scene gravity")
		("default_physx_shape_filterword0", boost::program_options::value(&default_physx_shape_filterword0)->default_value(0x01), "Default physx shape filterword0")
		("default_physx_joint_localframe", boost::program_options::value(&default_physx_joint_localframe)->default_value(0.1f), "Default physx joint localframe")
		("default_physx_joint_limits", boost::program_options::value(&default_physx_joint_limits)->default_value(0.1f), "Default physx joint limits")
		("default_io_thread_num", boost::program_options::value(&default_io_thread_num)->default_value(3), "Default io thread num")
		("default_load_shader_cache", boost::program_options::value(&default_load_shader_cache)->default_value(true), "Default load shader cache")
		("default_remaining_actor_max", boost::program_options::value(&default_remaining_actor_max)->default_value(30000), "Default remaining actor max")
		("default_grid_length", boost::program_options::value(&default_grid_length)->default_value(12.0f), "Default grid length")
		("default_grid_lines_every", boost::program_options::value(&default_grid_lines_every)->default_value(5.0f), "Default grid lines every")
		("default_grid_subdivisions", boost::program_options::value(&default_grid_subdivisions)->default_value(5), "Default grid subdivisions")
		("default_grid_color", boost::program_options::value(&default_grid_color)->default_value(D3DCOLOR_ARGB(255, 127, 127, 127)), "Default grid color")
		("default_grid_axis_color", boost::program_options::value(&default_grid_axis_color)->default_value(D3DCOLOR_ARGB(255, 0, 0, 0)), "Default grid axis color")
		("default_tool_snap_to_grid", boost::program_options::value(&default_tool_snap_to_grid)->default_value(false), "Default tool snap to grid")
		("default_tool_script_pattern", boost::program_options::value(&default_tool_script_pattern)->default_value("tools/*.lua"), "Default tool script pattern")
		("default_emitter_chunk_width", boost::program_options::value(&default_emitter_chunk_width)->default_value(4.0f), "Default emitter chunk width")
		("default_font_path", boost::program_options::value(&default_font_path)->default_value("font/wqy-microhei.ttc"), "Default font")
		("default_font_height", boost::program_options::value(&default_font_height)->default_value(13), "Default font height")
		("default_font_face_index", boost::program_options::value(&default_font_face_index)->default_value(0), "Default font face index")
		("default_dictionary_file", boost::program_options::value(&default_dictionary_file), "Default dictionary file")
		("default_texture", boost::program_options::value(&default_texture)->default_value("texture/Checker.bmp"), "Default texture")
		("default_normal_texture", boost::program_options::value(&default_normal_texture)->default_value("texture/Normal.dds"), "Default normal texture")
		("default_specular_texture", boost::program_options::value(&default_specular_texture)->default_value("texture/White.dds"), "Default specular texture")
		("default_shader", boost::program_options::value(&default_shader)->default_value("shader/mtl_BlinnPhong.fx"), "Default shader")
		("default_terrain_shader", boost::program_options::value(&default_terrain_shader)->default_value("shader/mtl_Splatmap.fx"), "Default terrain shader")
		("default_terrain_max_height", boost::program_options::value(&default_terrain_max_height)->default_value(900.0f), "Default terrain max height")
		("default_terrain_water_level", boost::program_options::value(&default_terrain_water_level)->default_value(150.0f), "Default terrain water level")
		("default_dialog_color", boost::program_options::value<D3DCOLOR>(&default_dialog_color)->default_value(D3DCOLOR_ARGB(255,255,255,255)), "Default dialog color")
		("default_dialog_img", boost::program_options::value(&default_dialog_img)->default_value("texture/White.dds"), "Default dialog img")
		("default_dialog_img_rect", boost::program_options::value<CRect>(&default_dialog_img_rect)->default_value(CRect(1,1,3,3), ""), "Default dialog img rect")
		("default_dialog_img_border", boost::program_options::value<CRect>(&default_dialog_img_border)->default_value(CRect(0,0,0,0), ""), "Default dialog img border")
		("default_static_text_color", boost::program_options::value<D3DCOLOR>(&default_static_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default static text color")
		("default_static_text_align", boost::program_options::value<my::Font::Align>(&default_static_text_align)->default_value(my::Font::AlignLeftTop), "Default static text align")
		("default_progressbar_img", boost::program_options::value(&default_progressbar_img)->default_value("texture/White.dds"), "Default progressbar img")
		("default_progressbar_img_rect", boost::program_options::value<CRect>(&default_progressbar_img_rect)->default_value(CRect(1,1,3,3), ""), "Default progressbar img rect")
		("default_progressbar_img_border", boost::program_options::value<CRect>(&default_progressbar_img_border)->default_value(CRect(0,0,0,0), ""), "Default progressbar img border")
		("default_progressbar_text_color", boost::program_options::value<D3DCOLOR>(&default_progressbar_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default progressbar text color")
		("default_progressbar_text_align", boost::program_options::value<my::Font::Align>(&default_progressbar_text_align)->default_value(my::Font::AlignLeftMiddle), "Default progressbar text align")
		("default_progressbar_foregroundimg", boost::program_options::value(&default_progressbar_foregroundimg)->default_value("texture/Red.dds"), "Default progressbar foregroundimg")
		("default_progressbar_foregroundimg_rect", boost::program_options::value<CRect>(&default_progressbar_foregroundimg_rect)->default_value(CRect(1,1,3,3), ""), "Default progressbar foregroundimg rect")
		("default_progressbar_foregroundimg_border", boost::program_options::value<CRect>(&default_progressbar_foregroundimg_border)->default_value(CRect(0,0,0,0), ""), "Default progressbar foregroundimg border")
		("default_button_img", boost::program_options::value(&default_button_img)->default_value("texture/White.dds"), "Default button img")
		("default_button_img_rect", boost::program_options::value<CRect>(&default_button_img_rect)->default_value(CRect(1,1,3,3), ""), "Default button img rect")
		("default_button_img_border", boost::program_options::value<CRect>(&default_button_img_border)->default_value(CRect(0,0,0,0), ""), "Default button img border")
		("default_button_text_color", boost::program_options::value<D3DCOLOR>(&default_button_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default button text color")
		("default_button_text_align", boost::program_options::value<my::Font::Align>(&default_button_text_align)->default_value(my::Font::AlignCenterMiddle), "Default button text align")
		("default_button_pressed_offset", boost::program_options::value<my::Vector2>(&default_button_pressed_offset)->default_value(my::Vector2(1, 2), "1,2"), "Default button pressed offset")
		("default_button_disabledimg", boost::program_options::value(&default_button_disabledimg)->default_value("texture/White.dds"), "Default button disabledimg")
		("default_button_disabledimg_rect", boost::program_options::value<CRect>(&default_button_disabledimg_rect)->default_value(CRect(1,1,3,3), ""), "Default button disabledimg rect")
		("default_button_disabledimg_border", boost::program_options::value<CRect>(&default_button_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default button disabledimg border")
		("default_button_pressedimg", boost::program_options::value(&default_button_pressedimg)->default_value("texture/White.dds"), "Default button pressedimg")
		("default_button_pressedimg_rect", boost::program_options::value<CRect>(&default_button_pressedimg_rect)->default_value(CRect(1,1,3,3), ""), "Default button pressedimg rect")
		("default_button_pressedimg_border", boost::program_options::value<CRect>(&default_button_pressedimg_border)->default_value(CRect(0,0,0,0), ""), "Default button pressedimg border")
		("default_button_mouseoverimg", boost::program_options::value(&default_button_mouseoverimg)->default_value("texture/White.dds"), "Default button mouseoverimg")
		("default_button_mouseoverimg_rect", boost::program_options::value<CRect>(&default_button_mouseoverimg_rect)->default_value(CRect(1,1,3,3), ""), "Default button mouseoverimg rect")
		("default_button_mouseoverimg_border", boost::program_options::value<CRect>(&default_button_mouseoverimg_border)->default_value(CRect(0,0,0,0), ""), "Default button mouseoverimg border")
		("default_editbox_color", boost::program_options::value<D3DCOLOR>(&default_editbox_color)->default_value(D3DCOLOR_ARGB(255,255,255,255)), "Default editbox color")
		("default_editbox_text_color", boost::program_options::value<D3DCOLOR>(&default_editbox_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default editbox text color")
		("default_editbox_text_align", boost::program_options::value<my::Font::Align>(&default_editbox_text_align)->default_value(my::Font::AlignLeftMiddle), "Default editbox text align")
		("default_editbox_img", boost::program_options::value(&default_editbox_img)->default_value("texture/White.dds"), "Default editbox img")
		("default_editbox_img_rect", boost::program_options::value<CRect>(&default_editbox_img_rect)->default_value(CRect(1,1,3,3), ""), "Default editbox img rect")
		("default_editbox_img_border", boost::program_options::value<CRect>(&default_editbox_img_border)->default_value(CRect(0,0,0,0), ""), "Default editbox img border")
		("default_editbox_disabledimg", boost::program_options::value(&default_editbox_disabledimg)->default_value("texture/White.dds"), "Default button disabledimg")
		("default_editbox_disabledimg_rect", boost::program_options::value<CRect>(&default_editbox_disabledimg_rect)->default_value(CRect(1,1,3,3), ""), "Default editbox disabledimg rect")
		("default_editbox_disabledimg_border", boost::program_options::value<CRect>(&default_editbox_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default editbox disabledimg border")
		("default_editbox_focusedimg", boost::program_options::value(&default_editbox_focusedimg)->default_value("texture/White.dds"), "Default editbox focusedimg")
		("default_editbox_focusedimg_rect", boost::program_options::value<CRect>(&default_editbox_focusedimg_rect)->default_value(CRect(1,1,3,3), ""), "Default editbox focusedimg rect")
		("default_editbox_focusedimg_border", boost::program_options::value<CRect>(&default_editbox_focusedimg_border)->default_value(CRect(0,0,0,0), ""), "Default editbox focusedimg border")
		("default_editbox_sel_bk_color", boost::program_options::value<D3DCOLOR>(&default_editbox_sel_bk_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default editbox sel_bk color")
		("default_editbox_caret_color", boost::program_options::value<D3DCOLOR>(&default_editbox_caret_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default editbox caret color")
		("default_editbox_caretimg", boost::program_options::value(&default_editbox_caretimg)->default_value("texture/White.dds"), "Default editbox caretimg")
		("default_editbox_caretimg_rect", boost::program_options::value<CRect>(&default_editbox_caretimg_rect)->default_value(CRect(1,1,3,3), ""), "Default editbox caretimg rect")
		("default_editbox_caretimg_border", boost::program_options::value<CRect>(&default_editbox_caretimg_border)->default_value(CRect(0,0,0,0), ""), "Default editbox caretimg border")
		("default_checkbox_img", boost::program_options::value(&default_checkbox_img)->default_value("texture/White.dds"), "Default checkbox img")
		("default_checkbox_img_rect", boost::program_options::value<CRect>(&default_checkbox_img_rect)->default_value(CRect(1,1,3,3), ""), "Default checkbox img rect")
		("default_checkbox_img_border", boost::program_options::value<CRect>(&default_checkbox_img_border)->default_value(CRect(0,0,0,0), ""), "Default checkbox img border")
		("default_checkbox_text_color", boost::program_options::value<D3DCOLOR>(&default_checkbox_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default checkbox text color")
		("default_checkbox_text_align", boost::program_options::value<my::Font::Align>(&default_checkbox_text_align)->default_value(my::Font::AlignLeftMiddle), "Default checkbox text align")
		("default_checkbox_pressed_offset", boost::program_options::value<my::Vector2>(&default_checkbox_pressed_offset)->default_value(my::Vector2(1, 2), "1,2"), "Default checkbox pressed offset")
		("default_checkbox_disabledimg", boost::program_options::value(&default_checkbox_disabledimg)->default_value("texture/White.dds"), "Default checkbox disabledimg")
		("default_checkbox_disabledimg_rect", boost::program_options::value<CRect>(&default_checkbox_disabledimg_rect)->default_value(CRect(1,1,3,3), ""), "Default checkbox disabledimg rect")
		("default_checkbox_disabledimg_border", boost::program_options::value<CRect>(&default_checkbox_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default checkbox disabledimg border")
		("default_checkbox_pressedimg", boost::program_options::value(&default_checkbox_pressedimg)->default_value("texture/White.dds"), "Default checkbox pressedimg")
		("default_checkbox_pressedimg_rect", boost::program_options::value<CRect>(&default_checkbox_pressedimg_rect)->default_value(CRect(1,1,3,3), ""), "Default checkbox pressedimg rect")
		("default_checkbox_pressedimg_border", boost::program_options::value<CRect>(&default_checkbox_pressedimg_border)->default_value(CRect(0,0,0,0), ""), "Default checkbox pressedimg border")
		("default_checkbox_mouseoverimg", boost::program_options::value(&default_checkbox_mouseoverimg)->default_value("texture/White.dds"), "Default checkbox mouseoverimg")
		("default_checkbox_mouseoverimg_rect", boost::program_options::value<CRect>(&default_checkbox_mouseoverimg_rect)->default_value(CRect(1,1,3,3), ""), "Default checkbox mouseoverimg rect")
		("default_checkbox_mouseoverimg_border", boost::program_options::value<CRect>(&default_checkbox_mouseoverimg_border)->default_value(CRect(0,0,0,0), ""), "Default checkbox mouseoverimg border")
		("default_combobox_img", boost::program_options::value(&default_combobox_img)->default_value("texture/White.dds"), "Default combobox img")
		("default_combobox_img_rect", boost::program_options::value<CRect>(&default_combobox_img_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox img rect")
		("default_combobox_img_border", boost::program_options::value<CRect>(&default_combobox_img_border)->default_value(CRect(0,0,0,0), ""), "Default combobox img border")
		("default_combobox_text_color", boost::program_options::value<D3DCOLOR>(&default_combobox_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default combobox text color")
		("default_combobox_text_align", boost::program_options::value<my::Font::Align>(&default_combobox_text_align)->default_value(my::Font::AlignCenterMiddle), "Default combobox text align")
		("default_combobox_pressed_offset", boost::program_options::value<my::Vector2>(&default_combobox_pressed_offset)->default_value(my::Vector2(1, 2), "1,2"), "Default combobox pressed offset")
		("default_combobox_disabledimg", boost::program_options::value(&default_combobox_disabledimg)->default_value("texture/White.dds"), "Default combobox disabledimg")
		("default_combobox_disabledimg_rect", boost::program_options::value<CRect>(&default_combobox_disabledimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox disabledimg rect")
		("default_combobox_disabledimg_border", boost::program_options::value<CRect>(&default_combobox_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox disabledimg border")
		("default_combobox_pressedimg", boost::program_options::value(&default_combobox_pressedimg)->default_value("texture/White.dds"), "Default combobox pressedimg")
		("default_combobox_pressedimg_rect", boost::program_options::value<CRect>(&default_combobox_pressedimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox pressedimg rect")
		("default_combobox_pressedimg_border", boost::program_options::value<CRect>(&default_combobox_pressedimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox pressedimg border")
		("default_combobox_mouseoverimg", boost::program_options::value(&default_combobox_mouseoverimg)->default_value("texture/White.dds"), "Default combobox mouseoverimg")
		("default_combobox_mouseoverimg_rect", boost::program_options::value<CRect>(&default_combobox_mouseoverimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox mouseoverimg rect")
		("default_combobox_mouseoverimg_border", boost::program_options::value<CRect>(&default_combobox_mouseoverimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox mouseoverimg border")
		("default_combobox_dropdownimg", boost::program_options::value(&default_combobox_dropdownimg)->default_value("texture/White.dds"), "Default combobox dropdownimg")
		("default_combobox_dropdownimg_rect", boost::program_options::value<CRect>(&default_combobox_dropdownimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox dropdownimg rect")
		("default_combobox_dropdownimg_border", boost::program_options::value<CRect>(&default_combobox_dropdownimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox dropdownimg border")
		("default_combobox_dropdownitem_text_color", boost::program_options::value<D3DCOLOR>(&default_combobox_dropdownitem_text_color)->default_value(D3DCOLOR_ARGB(255,0,0,0)), "Default combobox dropdownitem text color")
		("default_combobox_dropdownitem_text_align", boost::program_options::value<my::Font::Align>(&default_combobox_dropdownitem_text_align)->default_value(my::Font::AlignCenterMiddle), "Default combobox dropdownitem text align")
		("default_combobox_dropdownitem_mouseoverimg", boost::program_options::value(&default_combobox_dropdownitem_mouseoverimg)->default_value("texture/White.dds"), "Default combobox dropdownitem_mouseoverimg")
		("default_combobox_dropdownitem_mouseoverimg_rect", boost::program_options::value<CRect>(&default_combobox_dropdownitem_mouseoverimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox dropdownitem_mouseoverimg rect")
		("default_combobox_dropdownitem_mouseoverimg_border", boost::program_options::value<CRect>(&default_combobox_dropdownitem_mouseoverimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox dropdownitem_mouseoverimg border")
		("default_combobox_scrollbarupbtn_normalimg", boost::program_options::value(&default_combobox_scrollbarupbtn_normalimg)->default_value("texture/White.dds"), "Default combobox scrollbarupbtn normalimg")
		("default_combobox_scrollbarupbtn_normalimg_rect", boost::program_options::value<CRect>(&default_combobox_scrollbarupbtn_normalimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox scrollbarupbtn normalimg rect")
		("default_combobox_scrollbarupbtn_normalimg_border", boost::program_options::value<CRect>(&default_combobox_scrollbarupbtn_normalimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox scrollbarupbtn normalimg border")
		("default_combobox_scrollbarupbtn_disabledimg", boost::program_options::value(&default_combobox_scrollbarupbtn_disabledimg)->default_value("texture/White.dds"), "Default combobox scrollbarupbtn disabledimg")
		("default_combobox_scrollbarupbtn_disabledimg_rect", boost::program_options::value<CRect>(&default_combobox_scrollbarupbtn_disabledimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox scrollbarupbtn disabledimg rect")
		("default_combobox_scrollbarupbtn_disabledimg_border", boost::program_options::value<CRect>(&default_combobox_scrollbarupbtn_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox scrollbarupbtn disabledimg border")
		("default_combobox_scrollbardownbtn_normalimg", boost::program_options::value(&default_combobox_scrollbardownbtn_normalimg)->default_value("texture/White.dds"), "Default combobox scrollbardownbtn normalimg")
		("default_combobox_scrollbardownbtn_normalimg_rect", boost::program_options::value<CRect>(&default_combobox_scrollbardownbtn_normalimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox scrollbardownbtn normalimg rect")
		("default_combobox_scrollbardownbtn_normalimg_border", boost::program_options::value<CRect>(&default_combobox_scrollbardownbtn_normalimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox scrollbardownbtn normalimg border")
		("default_combobox_scrollbardownbtn_disabledimg", boost::program_options::value(&default_combobox_scrollbardownbtn_disabledimg)->default_value("texture/White.dds"), "Default combobox scrollbardownbtn disabledimg")
		("default_combobox_scrollbardownbtn_disabledimg_rect", boost::program_options::value<CRect>(&default_combobox_scrollbardownbtn_disabledimg_rect)->default_value(CRect(1,1,3,3), "143,16,16"), "Default combobox scrollbardownbtn disabledimg rect")
		("default_combobox_scrollbardownbtn_disabledimg_border", boost::program_options::value<CRect>(&default_combobox_scrollbardownbtn_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox scrollbardownbtn disabledimg border")
		("default_combobox_scrollbarthumbbtn_normalimg", boost::program_options::value(&default_combobox_scrollbarthumbbtn_normalimg)->default_value("texture/White.dds"), "Default combobox scrollbarthumbbtn normalimg")
		("default_combobox_scrollbarthumbbtn_normalimg_rect", boost::program_options::value<CRect>(&default_combobox_scrollbarthumbbtn_normalimg_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox scrollbarthumbbtn normalimg rect")
		("default_combobox_scrollbarthumbbtn_normalimg_border", boost::program_options::value<CRect>(&default_combobox_scrollbarthumbbtn_normalimg_border)->default_value(CRect(0,0,0,0), ""), "Default combobox scrollbarthumbbtn normalimg border")
		("default_combobox_scrollbar_img", boost::program_options::value(&default_combobox_scrollbar_img)->default_value("texture/White.dds"), "Default combobox scrollbar img")
		("default_combobox_scrollbar_img_rect", boost::program_options::value<CRect>(&default_combobox_scrollbar_img_rect)->default_value(CRect(1,1,3,3), ""), "Default combobox scrollbar img rect")
		("default_combobox_scrollbar_img_border", boost::program_options::value<CRect>(&default_combobox_scrollbar_img_border)->default_value(CRect(0,0,0,0), ""), "Default combobox scrollbar img border")
		("default_listbox_img", boost::program_options::value(&default_listbox_img)->default_value("texture/White.dds"), "Default listbox img")
		("default_listbox_img_rect", boost::program_options::value<CRect>(&default_listbox_img_rect)->default_value(CRect(1,1,3,3), ""), "Default listbox img rect")
		("default_listbox_img_border", boost::program_options::value<CRect>(&default_listbox_img_border)->default_value(CRect(0,0,0,0), ""), "Default listbox img border")
		("default_listbox_scrollbarupbtn_normalimg", boost::program_options::value(&default_listbox_scrollbarupbtn_normalimg)->default_value("texture/White.dds"), "Default listbox scrollbarupbtn normalimg")
		("default_listbox_scrollbarupbtn_normalimg_rect", boost::program_options::value<CRect>(&default_listbox_scrollbarupbtn_normalimg_rect)->default_value(CRect(1,1,3,3), ""), "Default listbox scrollbarupbtn normalimg rect")
		("default_listbox_scrollbarupbtn_normalimg_border", boost::program_options::value<CRect>(&default_listbox_scrollbarupbtn_normalimg_border)->default_value(CRect(0,0,0,0), ""), "Default listbox scrollbarupbtn normalimg border")
		("default_listbox_scrollbarupbtn_disabledimg", boost::program_options::value(&default_listbox_scrollbarupbtn_disabledimg)->default_value("texture/White.dds"), "Default listbox scrollbarupbtn disabledimg")
		("default_listbox_scrollbarupbtn_disabledimg_rect", boost::program_options::value<CRect>(&default_listbox_scrollbarupbtn_disabledimg_rect)->default_value(CRect(1,1,3,3), ""), "Default listbox scrollbarupbtn disabledimg rect")
		("default_listbox_scrollbarupbtn_disabledimg_border", boost::program_options::value<CRect>(&default_listbox_scrollbarupbtn_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default listbox scrollbarupbtn disabledimg border")
		("default_listbox_scrollbardownbtn_normalimg", boost::program_options::value(&default_listbox_scrollbardownbtn_normalimg)->default_value("texture/White.dds"), "Default listbox scrollbardownbtn normalimg")
		("default_listbox_scrollbardownbtn_normalimg_rect", boost::program_options::value<CRect>(&default_listbox_scrollbardownbtn_normalimg_rect)->default_value(CRect(1,1,3,3), ""), "Default listbox scrollbardownbtn normalimg rect")
		("default_listbox_scrollbardownbtn_normalimg_border", boost::program_options::value<CRect>(&default_listbox_scrollbardownbtn_normalimg_border)->default_value(CRect(0,0,0,0), ""), "Default listbox scrollbardownbtn normalimg border")
		("default_listbox_scrollbardownbtn_disabledimg", boost::program_options::value(&default_listbox_scrollbardownbtn_disabledimg)->default_value("texture/White.dds"), "Default listbox scrollbardownbtn disabledimg")
		("default_listbox_scrollbardownbtn_disabledimg_rect", boost::program_options::value<CRect>(&default_listbox_scrollbardownbtn_disabledimg_rect)->default_value(CRect(1,1,3,3), "143,16,16"), "Default listbox scrollbardownbtn disabledimg rect")
		("default_listbox_scrollbardownbtn_disabledimg_border", boost::program_options::value<CRect>(&default_listbox_scrollbardownbtn_disabledimg_border)->default_value(CRect(0,0,0,0), ""), "Default listbox scrollbardownbtn disabledimg border")
		("default_listbox_scrollbarthumbbtn_normalimg", boost::program_options::value(&default_listbox_scrollbarthumbbtn_normalimg)->default_value("texture/White.dds"), "Default listbox scrollbarthumbbtn normalimg")
		("default_listbox_scrollbarthumbbtn_normalimg_rect", boost::program_options::value<CRect>(&default_listbox_scrollbarthumbbtn_normalimg_rect)->default_value(CRect(1,1,3,3), ""), "Default listbox scrollbarthumbbtn normalimg rect")
		("default_listbox_scrollbarthumbbtn_normalimg_border", boost::program_options::value<CRect>(&default_listbox_scrollbarthumbbtn_normalimg_border)->default_value(CRect(0,0,0,0), ""), "Default listbox scrollbarthumbbtn normalimg border")
		("default_listbox_scrollbar_img", boost::program_options::value(&default_listbox_scrollbar_img)->default_value("texture/White.dds"), "Default listbox scrollbar img")
		("default_listbox_scrollbar_img_rect", boost::program_options::value<CRect>(&default_listbox_scrollbar_img_rect)->default_value(CRect(1,1,3,3), ""), "Default listbox scrollbar img rect")
		("default_listbox_scrollbar_img_border", boost::program_options::value<CRect>(&default_listbox_scrollbar_img_border)->default_value(CRect(0,0,0,0), ""), "Default listbox scrollbar img border")
		("default_player_scale", boost::program_options::value(&default_player_scale)->default_value(1.0f), "Default player scale")
		("default_player_height", boost::program_options::value(&default_player_height)->default_value(1.0f), "Default player height")
		("default_player_radius", boost::program_options::value(&default_player_radius)->default_value(0.5f), "Default player radius")
		("default_player_contact_offset", boost::program_options::value(&default_player_contact_offset)->default_value(0.1f), "Default player contact offset")
		("default_player_step_offset", boost::program_options::value(&default_player_step_offset)->default_value(0.5f), "Default player step offset")
		("default_player_run_speed", boost::program_options::value(&default_player_run_speed)->default_value(5.0f), "Default player run speed")
		("default_player_breaking_speed", boost::program_options::value(&default_player_breaking_speed)->default_value(20.0f), "Default player breaking speed")
		("default_player_seek_force", boost::program_options::value(&default_player_seek_force)->default_value(50.0f), "Default player seek force")
		("default_player_climb_enter_slope", boost::program_options::value(&default_player_climb_enter_slope)->default_value(sinf(D3DXToRadian(10.0f))), "Default player climb enter slope")
		("default_player_climb_leave_slope", boost::program_options::value(&default_player_climb_leave_slope)->default_value(sinf(D3DXToRadian(30.0f))), "Default player climb leave slope")
		("default_player_swim_depth", boost::program_options::value(&default_player_swim_depth)->default_value(0.5f), "Default player swim depth")
		("default_player_swim_force", boost::program_options::value(&default_player_swim_force)->default_value(15.0f), "Default player swim force")
		("default_player_look_distance", boost::program_options::value(&default_player_look_distance)->default_value(3.0f), "Default player look distance")
		("default_player_collision_height", boost::program_options::value(&default_player_collision_height)->default_value(1000.0f), "Default player collision height")
		("default_player_mesh_list", boost::program_options::value(&default_player_mesh_list), "Default player mesh list")
		("default_player_anim_list", boost::program_options::value(&default_player_anim_list), "Default player anim list")
		("default_player_ik_nodes", boost::program_options::value(&default_player_ik_nodes), "Default player ik nodes")
		("default_player_water_filterword0", boost::program_options::value(&default_player_water_filterword0)->default_value(0x02), "Default player water filterword0")
		("default_player_water_buoyancy", boost::program_options::value(&default_player_water_buoyancy)->default_value(1.5f), "Default player water buoyancy")
		("default_player_water_drag", boost::program_options::value(&default_player_water_drag)->default_value(5.0f), "Default player water drag")
		("default_player_air_drag", boost::program_options::value(&default_player_air_drag)->default_value(0.6f), "Default player air drag")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_config_file<char>((cfg_file + ".cfg").c_str(), desc, true), vm);
	boost::program_options::store(boost::program_options::parse_command_line(__argc, __targv, desc), vm);
	boost::program_options::notify(vm);

	std::vector<std::string>::const_iterator path_iter = path_list.begin();
	for (; path_iter != path_list.end(); path_iter++)
	{
		if (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(path_iter->c_str()))
		{
			ResourceMgr::RegisterFileDir(*path_iter);
		}
		else
		{
			ResourceMgr::RegisterZipDir(*path_iter);
		}
	}

	boost::algorithm::replace_all(default_tool_script_pattern, "/", "\\");

	if (!default_dictionary_file.empty())
	{
		m_Dicts.LoadFromFile(default_dictionary_file.c_str());
	}

	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	if (!PhysxSdk::Init())
	{
		return FALSE;
	}

	m_d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if(!m_d3d9)
	{
		return FALSE;
	}

	//// Register the application's document templates.  Document templates
	////  serve as the connection between documents, frame windows and views
	//CSingleDocTemplate* pDocTemplate;
	//pDocTemplate = new CSingleDocTemplate(
	//	IDR_MAINFRAME,
	//	RUNTIME_CLASS(CMainDoc),
	//	RUNTIME_CLASS(CMainFrame),       // main SDI frame window
	//	RUNTIME_CLASS(CChildView));
	//if (!pDocTemplate)
	//	return FALSE;
	//AddDocTemplate(pDocTemplate);

	CMainFrame * pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	//// Parse command line for standard shell commands, DDE, file open
	//CCommandLineInfo cmdInfo;
	//ParseCommandLine(cmdInfo);

	//// Dispatch commands specified on the command line.  Will return FALSE if
	//// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	//if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;
	pFrame->OnCmdMsg(ID_FILE_NEW, 0, NULL, NULL);

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}


HRESULT CMainApp::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	D3DContext::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

	InputMgr::Create(m_hInstance, m_pMainWnd->m_hWnd);

	if(FAILED(hr = my::ResourceMgr::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	my::ResourceMgr::StartIORequestProc(default_io_thread_num);

	ParallelTaskManager::StartParallelThread(default_io_thread_num);

	if (FAILED(hr = RenderPipeline::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (default_load_shader_cache)
	{
		TCHAR szDir[MAX_PATH];
		GetCurrentDirectory(_countof(szDir), szDir);
		RenderPipeline::LoadShaderCache(szDir);
	}

	if (FAILED(hr = m_UIRender->OnCreateDevice(pd3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (!(m_Font = LoadFont(default_font_path.c_str(), default_font_height, default_font_face_index)))
	{
		TRACE("LoadFont failed");
		return S_FALSE;
	}

	return S_OK;
}

HRESULT CMainApp::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	D3DContext::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

	if(FAILED(hr = ResourceMgr::OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (FAILED(hr = RenderPipeline::OnResetDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (FAILED(hr = m_UIRender->OnResetDevice(pd3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	return S_OK;
}

void CMainApp::OnLostDevice(void)
{
	m_UIRender->OnLostDevice();

	ResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();

	D3DContext::OnLostDevice();
}

void CMainApp::OnDestroyDevice(void)
{
	ParallelTaskManager::StopParallelThread();

	m_UIRender->OnDestroyDevice();

	ResourceMgr::OnDestroyDevice();

	RenderPipeline::OnDestroyDevice();

	InputMgr::Destroy();

	D3DContext::OnDestroyDevice();

	m_UIRender.reset();
}

const std::wstring & CMainApp::OnControlTranslate(const std::wstring& wstr)
{
	DictionaryNode::childmap::iterator iter = m_Dicts.m_children->find(wstr);
	if (iter != m_Dicts.m_children->end())
	{
		return iter->second.m_data;
	}
	return wstr;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CMainApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMainApp customization load/save methods

void CMainApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	GetContextMenuManager()->AddMenu(_T(""), IDR_POPUP_VIEW);
}

void CMainApp::LoadCustomState()
{
}

void CMainApp::SaveCustomState()
{
}

// CMainApp message handlers




BOOL CMainApp::OnIdle(LONG lCount)
{
	// TODO: Add your specialized code here and/or call the base class
	Clock::UpdateClock();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);
	if (pFrame->m_State)
	{
		pFrame->dogc(LUA_GCSTEP, 1);
	}

	LeaveDeviceSection();

	if (!m_IORequestList.empty())
	{
		m_bNeedDraw = TRUE;
	}

	my::InputMgr::Capture(m_fTotalTime, m_fElapsedTime);

	BOOL bContinue = FALSE;
	if (my::ResourceMgr::CheckIORequests(0))
	{
		bContinue = TRUE;
	}

	if (CWinAppEx::OnIdle(lCount))
	{
		bContinue = TRUE;
	}

	EnterDeviceSection();

	if (!pFrame->m_selactors.empty() || !pFrame->m_selctls.empty() || pFrame->m_Player->m_Node)
	{
		pFrame->OnFrameTick(m_fElapsedTime);

		my::EventArg arg;
		pFrame->m_EventSelectionPlaying(&arg);

		bContinue = TRUE;
	}
	else if (!bContinue && m_bNeedDraw)
	{
		my::EventArg arg;
		pFrame->m_EventSelectionPlaying(&arg);

		m_bNeedDraw = FALSE;
	}

	return bContinue;
}

int CMainApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	PhysxSdk::Shutdown();

	return CWinAppEx::ExitInstance();
}

extern BOOL g_bRemoveFromMRU;

CDocument* CMainApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: Add your specialized code here and/or call the base class
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_strPathName = lpszFileName;
	theApp.AddToRecentFileList(pFrame->m_strPathName); // ! invalid lpszFileName by resizing list
	pFrame->OnUpdateFrameTitle(TRUE);
	pFrame->m_wndOutliner.OnDestroyItemList();
	CWaitCursor wait;
	pFrame->ClearFileContext();
	pFrame->InitFileContext();
	if (pFrame->OpenFileContext(pFrame->m_strPathName))
	{
		g_bRemoveFromMRU = FALSE;

		pFrame->OnSelChanged();

		CChildView * pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
		ASSERT_VALID(pView);
		CEnvironmentWnd::CameraPropEventArgs arg(pView);
		pFrame->m_EventCameraPropChanged(&arg);
	}
	pFrame->m_wndOutliner.OnInitItemList();
	return NULL;
}

void CMainApp::OnNamedObjectCreate(my::NamedObject* Object)
{
	NamedObjectEventArgs arg(Object);
	m_EventNamedObjectCreate(&arg);
}

void CMainApp::OnNamedObjectDestroy(my::NamedObject* Object)
{
	NamedObjectEventArgs arg(Object);
	m_EventNamedObjectDestroy(&arg);
}

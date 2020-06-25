
// myd3dbox.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"
#include <boost/program_options.hpp>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include "LuaExtension.inl"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int lua_print(lua_State * L)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_SETSEL, (WPARAM)-1, (LPARAM)-1);

	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
				LUA_QL("print"));
		if (i > 1)
			pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_SETSEL, 0, (LPARAM)_T("\t"));
		else
			pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_SETSEL, 0, (LPARAM)_T("\n"));
		pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_SETSEL, 0, (LPARAM)u8tots(s).c_str());
		lua_pop(L, 1);  /* pop result */
	}
	return 0;
}

typedef struct LoadF {
	int extraline;
	//FILE *f;
	my::IStreamPtr stream;
	char buff[LUAL_BUFFERSIZE];
} LoadF;

static const char *getF(lua_State *L, void *ud, size_t *size) {
	LoadF *lf = (LoadF *)ud;
	(void)L;
	if (lf->extraline) {
		lf->extraline = 0;
		*size = 1;
		return "\n";
	}
	//if (feof(lf->f)) return NULL;
	*size = lf->stream->read(lf->buff, sizeof(lf->buff));
	return (*size > 0) ? lf->buff : NULL;
}
//
//static int errfile (lua_State *L, const char *what, int fnameindex) {
//	const char *serr = strerror(errno);
//	const char *filename = lua_tostring(L, fnameindex) + 1;
//	lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
//	lua_remove(L, fnameindex);
//	return LUA_ERRFILE;
//}

static int luaL_loadfile(lua_State *L, const char *filename)
{
	LoadF lf;
	//int status, readstatus;
	//int c;
	int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
	lf.extraline = 0;
	//if (filename == NULL) {
	//	lua_pushliteral(L, "=stdin");
	//	lf.f = stdin;
	//}
	//else {
	lua_pushfstring(L, "@%s", filename);
	//	lf.f = fopen(filename, "r");
	//	if (lf.f == NULL) return errfile(L, "open", fnameindex);
	//}
	//c = getc(lf.f);
	//if (c == '#') {  /* Unix exec. file? */
	//	lf.extraline = 1;
	//	while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
	//	if (c == '\n') c = getc(lf.f);
	//}
	//if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
	//	lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
	//	if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
	//	/* skip eventual `#!...' */
	//	while ((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
	//	lf.extraline = 0;
	//}
	//ungetc(c, lf.f);
	try
	{
		lf.stream = theApp.OpenIStream(filename);
	}
	catch (const my::Exception & e)
	{
		lua_pushfstring(L, e.what().c_str());
		lua_remove(L, fnameindex);
		return LUA_ERRFILE;
	}
	int status = lua_load(L, getF, &lf, lua_tostring(L, -1));
	//readstatus = ferror(lf.f);
	//if (filename) fclose(lf.f);  /* close file (even in case of errors) */
	//if (readstatus) {
	//	lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
	//	return errfile(L, "read", fnameindex);
	//}
	lua_remove(L, fnameindex);
	return status;
}

static int load_aux(lua_State *L, int status) {
	if (status == 0)  /* OK? */
		return 1;
	else {
		lua_pushnil(L);
		lua_insert(L, -2);  /* put before error message */
		return 2;  /* return nil plus error message */
	}
}

static int luaB_loadfile(lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	return load_aux(L, luaL_loadfile(L, fname));
}

static int luaB_dofile(lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	int n = lua_gettop(L);
	if (luaL_loadfile(L, fname) != 0) lua_error(L);
	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L) - n;
}

static void loaderror(lua_State *L, const char *filename) {
	luaL_error(L, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s",
		lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

static int loader_Lua(lua_State *L) {
	//const char *filename;
	const char *name = luaL_checkstring(L, 1);
	//filename = findfile(L, name, "path");
	//if (filename == NULL) return 1;  /* library not found in this path */
	if (luaL_loadfile(L, name) != 0)
		loaderror(L, name);
	return 1;  /* library loaded successfully */
}

static int os_exit(lua_State * L)
{
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
	return 0;
}

// CMainApp

BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMainApp::OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CMainApp construction

CMainApp::CMainApp()
{
	default_font_height = 13;
	default_font_face_index = 0;
	default_pass_mask = 0;
	technique_RenderSceneColor = NULL;
	handle_MeshColor = NULL;
	m_bNeedDraw = FALSE;
	m_bHiColorIcons = TRUE;

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
	m_DeviceSettings.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
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
		if(references > 0)
		{
			CString msg;
			msg.Format(_T("no zero reference count: %u"), references);
			AfxMessageBox(msg);
		}
		m_DeviceObjectsCreated = false;
	}
}

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
		("path", boost::program_options::value(&path_list), "Path")
		("default_font", boost::program_options::value(&default_font)->default_value("font/wqy-microhei.ttc"), "Default font")
		("default_font_height", boost::program_options::value(&default_font_height)->default_value(13), "Default font height")
		("default_font_face_index", boost::program_options::value(&default_font_face_index)->default_value(0), "Default font face index")
		("default_texture", boost::program_options::value(&default_texture)->default_value("texture/Checker.bmp"), "Default texture")
		("default_normal_texture", boost::program_options::value(&default_normal_texture)->default_value("texture/Normal.dds"), "Default normal texture")
		("default_specular_texture", boost::program_options::value(&default_specular_texture)->default_value("texture/White.dds"), "Default specular texture")
		("default_shader", boost::program_options::value(&default_shader)->default_value("shader/mtl_lambert1.fx"), "Default shader")
		("default_water_shader", boost::program_options::value(&default_water_shader)->default_value("shader/mtl_water1.fx"), "Default water shader")
		("default_sky_front_texture", boost::program_options::value(&default_sky_texture[4])->default_value("texture/cloudy_noon_FR.jpg"), "Default sky front texture")
		("default_sky_back_texture", boost::program_options::value(&default_sky_texture[5])->default_value("texture/cloudy_noon_BK.jpg"), "Default sky back texture")
		("default_sky_left_texture", boost::program_options::value(&default_sky_texture[1])->default_value("texture/cloudy_noon_LF.jpg"), "Default sky left texture")
		("default_sky_right_texture", boost::program_options::value(&default_sky_texture[0])->default_value("texture/cloudy_noon_RT.jpg"), "Default sky right texture")
		("default_sky_up_texture", boost::program_options::value(&default_sky_texture[2])->default_value("texture/cloudy_noon_UP.jpg"), "Default sky up texture")
		("default_sky_down_texture", boost::program_options::value(&default_sky_texture[3])->default_value("texture/cloudy_noon_DN.jpg"), "Default sky down texture")
		("default_pass_mask", boost::program_options::value(&default_pass_mask)->default_value(RenderPipeline::PassMaskShadowNormalOpaque), "Default pass mask")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_config_file<char>((cfg_file + ".cfg").c_str(), desc, true), vm);
	boost::program_options::notify(vm);
	if (path_list.empty())
	{
		path_list.push_back("Media");
		path_list.push_back("..\\demo2_3\\Media");
	}
	std::vector<std::string>::const_iterator path_iter = path_list.begin();
	for (; path_iter != path_list.end(); path_iter++)
	{
		ResourceMgr::RegisterFileDir(*path_iter);
		ResourceMgr::RegisterZipDir(*path_iter + ".zip");
	}

	_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

	if (!PhysxContext::Init())
	{
		return FALSE;
	}

	LuaContext::Init();
	lua_pushcfunction(m_State, lua_print);
	lua_setglobal(m_State, "print");
	lua_pushcfunction(m_State, luaB_loadfile);
	lua_setglobal(m_State, "loadfile");
	lua_pushcfunction(m_State, luaB_dofile);
	lua_setglobal(m_State, "dofile");
	lua_getglobal(m_State, "package");
	lua_getfield(m_State, -1, "loaders");
	lua_pushcfunction(m_State, loader_Lua);
	lua_rawseti(m_State, -2, 2);
	lua_getglobal(m_State, "os");
	lua_pushcclosure(m_State, os_exit, 0);
	lua_setfield(m_State, -2, "exit");
	lua_settop(m_State, 0);
	luabind::module(m_State)
	[
		luabind::class_<CMainApp, luabind::bases<my::ResourceMgr> >("MainApp")
	];
	luabind::globals(m_State)["app"] = this;

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
	if(FAILED(hr = my::ResourceMgr::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	my::ResourceMgr::StartIORequestProc(4);

	if (FAILED(hr = RenderPipeline::OnCreateDevice(m_d3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	BOOST_VERIFY(technique_RenderSceneColor = m_SimpleSample->GetTechniqueByName("RenderSceneColor"));
	BOOST_VERIFY(handle_MeshColor = m_SimpleSample->GetParameterByName(NULL, "g_MeshColor"));

	if (FAILED(hr = m_UIRender->OnCreateDevice(pd3dDevice, &m_BackBufferSurfaceDesc)))
	{
		TRACE(my::D3DException::Translate(hr));
		return hr;
	}

	if (!(m_Font = LoadFont(default_font.c_str(), default_font_height, default_font_face_index)))
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
	D3DContext::m_EventDeviceReset();

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

	D3DContext::m_EventDeviceLost();

	ResourceMgr::OnLostDevice();

	RenderPipeline::OnLostDevice();
}

void CMainApp::OnDestroyDevice(void)
{
	m_UIRender->OnDestroyDevice();

	D3DContext::m_EventDeviceDestroy();

	ResourceMgr::OnDestroyDevice();

	RenderPipeline::OnDestroyDevice();

	m_UIRender.reset();
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

	m_d3dDeviceSec.Leave();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	ASSERT_VALID(pView);
	if (!m_IORequestList.empty())
	{
		m_bNeedDraw = TRUE;
	}

	BOOL bContinue = FALSE;
	if (my::ResourceMgr::CheckIORequests(0))
	{
		bContinue = TRUE;
	}

	if (CWinAppEx::OnIdle(lCount))
	{
		bContinue = TRUE;
	}

	m_d3dDeviceSec.Enter();

	float fElapsedTime = my::Min(0.016f, m_fElapsedTime);
	if (!pFrame->m_selactors.empty())
	{
		pFrame->OnFrameTick(fElapsedTime);

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
	LuaContext::Shutdown();

	PhysxContext::Shutdown();

	return CWinAppEx::ExitInstance();
}

extern BOOL g_bRemoveFromMRU;

CDocument* CMainApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: Add your specialized code here and/or call the base class
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (pFrame->DoOpen(lpszFileName))
	{
		g_bRemoveFromMRU = FALSE;
	}
	return NULL;
}

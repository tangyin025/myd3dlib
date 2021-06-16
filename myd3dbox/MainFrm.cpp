
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainFrm.h"
#include "ChildView.h"
#include "TerrainDlg.h"
#include "Terrain.h"
#include "Material.h"
#include "Character.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/set.hpp>
#include <fstream>
#include "NavigationDlg.h"
#include "SimplifyMeshDlg.h"
#include "Animation.h"
#include "Win32InputBox.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include "LuaExtension.inl"
#include "myException.h"

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
			pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_REPLACESEL, 0, (LPARAM)_T("\t"));
		pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_REPLACESEL, 0, (LPARAM)u8tots(s).c_str());
		lua_pop(L, 1);  /* pop result */
	}
	pFrame->m_wndOutput.m_wndOutputDebug.SendMessage(EM_REPLACESEL, 0, (LPARAM)_T("\n"));
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

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_FILE_NEW, &CMainFrame::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, &CMainFrame::OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CMainFrame::OnFileSaveAs)
	ON_COMMAND(ID_CREATE_ACTOR, &CMainFrame::OnCreateActor)
	ON_COMMAND(ID_COMPONENT_CHARACTER, &CMainFrame::OnCreateCharacter)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_CHARACTER, &CMainFrame::OnUpdateCreateCharacter)
	ON_COMMAND(ID_COMPONENT_MESH, &CMainFrame::OnComponentMesh)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_MESH, &CMainFrame::OnUpdateComponentMesh)
	ON_COMMAND(ID_COMPONENT_CLOTH, &CMainFrame::OnComponentCloth)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_CLOTH, &CMainFrame::OnUpdateComponentCloth)
	ON_COMMAND(ID_COMPONENT_STATICEMITTER, &CMainFrame::OnComponentStaticEmitter)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_STATICEMITTER, &CMainFrame::OnUpdateComponentStaticEmitter)
	ON_COMMAND(ID_COMPONENT_SPHERICALEMITTER, &CMainFrame::OnComponentSphericalemitter)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_SPHERICALEMITTER, &CMainFrame::OnUpdateComponentSphericalemitter)
	ON_COMMAND(ID_COMPONENT_TERRAIN, &CMainFrame::OnComponentTerrain)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_TERRAIN, &CMainFrame::OnUpdateComponentTerrain)
	ON_COMMAND(ID_EDIT_DELETE, &CMainFrame::OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CMainFrame::OnUpdateEditDelete)
	ON_COMMAND(ID_PIVOT_MOVE, &CMainFrame::OnPivotMove)
	ON_UPDATE_COMMAND_UI(ID_PIVOT_MOVE, &CMainFrame::OnUpdatePivotMove)
	ON_COMMAND(ID_PIVOT_ROTATE, &CMainFrame::OnPivotRotate)
	ON_UPDATE_COMMAND_UI(ID_PIVOT_ROTATE, &CMainFrame::OnUpdatePivotRotate)
	ON_COMMAND(ID_TOOLS_CLEARSHADER, &CMainFrame::OnViewClearshader)
	ON_COMMAND(ID_TOOLS_BUILDNAVIGATION, &CMainFrame::OnToolsBuildnavigation)
	ON_COMMAND(ID_PAINT_TERRAINHEIGHTFIELD, &CMainFrame::OnPaintTerrainHeightField)
	ON_UPDATE_COMMAND_UI(ID_PAINT_TERRAINHEIGHTFIELD, &CMainFrame::OnUpdatePaintTerrainHeightField)
	ON_COMMAND(ID_PAINT_TERRAINCOLOR, &CMainFrame::OnPaintTerrainColor)
	ON_UPDATE_COMMAND_UI(ID_PAINT_TERRAINCOLOR, &CMainFrame::OnUpdatePaintTerrainColor)
	ON_COMMAND(ID_PAINT_EMITTERINSTANCE, &CMainFrame::OnPaintEmitterinstance)
	ON_UPDATE_COMMAND_UI(ID_PAINT_EMITTERINSTANCE, &CMainFrame::OnUpdatePaintEmitterinstance)
	ON_COMMAND(ID_COMPONENT_ANIMATOR, &CMainFrame::OnComponentAnimator)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_ANIMATOR, &CMainFrame::OnUpdateComponentAnimator)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	: OctRoot(-4096, 4096)
	, m_bEatAltUp(FALSE)
	, m_selchunkid(0, 0)
	, m_selbox(-1, 1)
	, m_PaintType(PaintTypeNone)
	, m_PaintShape(PaintShapeCircle)
	, m_PaintMode(PaintModeGreater)
	, m_PaintRadius(5.0f)
	, m_PaintHeight(5.0f)
	, m_PaintColor(1.0f, 1.0f, 1.0f, 1.0f)
	, m_PaintDensity(1)
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2005);
	m_PaintSpline.AddNode(0, 0, 1, 1);
	m_PaintSpline.AddNode(1, 1, 1, 1);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!theApp.CreateD3DDevice(m_hWnd))
		return -1;

	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!PhysxScene::Init(theApp.m_sdk.get(), theApp.m_CpuDispatcher.get()))
		return -1;

	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, 1);

	BOOL bNameValid;
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	if (!m_wndProperties.Create(_T("Properties"), this, CRect(0, 0, 200, 200), TRUE, 3002,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	if (!m_wndEnvironment.Create(_T("Environment"), this, CRect(0, 0, 200, 200), TRUE, 3003,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Environment window\n");
		return FALSE; // failed to create
	}

	if (!m_wndOutput.Create(_T("Output"), this, CRect(0, 0, 200, 200), TRUE, 3001,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE))
	{
		TRACE0("Failed to create Output window\n");
		return -1;
	}

	if (!m_wndScript.Create(_T("Script"), this, CRect(0, 0, 200, 200), TRUE, 3004,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE))
	{
		TRACE0("Failed to create Script window\n");
		return -1;
	}

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	m_wndEnvironment.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	m_wndScript.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndProperties);
	DockPane(&m_wndEnvironment);
	DockPane(&m_wndOutput);
	DockPane(&m_wndScript);
	CDockablePane* pTabbedBar = NULL;
	//m_wndProperties.AttachToTabWnd(&m_wndOutput, DM_SHOW, FALSE, &pTabbedBar);
	//m_wndEnvironment.AttachToTabWnd(&m_wndProperties, DM_SHOW, FALSE, &pTabbedBar);
	m_wndScript.AttachToTabWnd(&m_wndOutput, DM_SHOW, FALSE, &pTabbedBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	//if (CMFCToolBar::GetUserImages() == NULL)
	//{
	//	// load user-defined toolbar images
	//	if (m_UserImages.Load(_T(".\\UserImages.bmp")))
	//	{
	//		m_UserImages.SetImageSize(CSize(16, 16), FALSE);
	//		CMFCToolBar::SetUserImages(&m_UserImages);
	//	}
	//}

	//// enable menu personalization (most-recently used commands)
	//// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	//CList<UINT, UINT> lstBasicCommands;

	//lstBasicCommands.AddTail(ID_FILE_NEW);
	//lstBasicCommands.AddTail(ID_FILE_OPEN);
	//lstBasicCommands.AddTail(ID_FILE_SAVE);
	//lstBasicCommands.AddTail(ID_FILE_PRINT);
	//lstBasicCommands.AddTail(ID_APP_EXIT);
	//lstBasicCommands.AddTail(ID_EDIT_CUT);
	//lstBasicCommands.AddTail(ID_EDIT_PASTE);
	//lstBasicCommands.AddTail(ID_EDIT_UNDO);
	//lstBasicCommands.AddTail(ID_APP_ABOUT);
	//lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	//lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	//lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	//lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	//lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	//lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	//lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	//lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);

	//CMFCToolBar::SetBasicCommands(lstBasicCommands);

	//m_emitter.reset(new EmitterComponent(my::AABB(FLT_MAX, -FLT_MAX), my::Matrix4::Identity()));
	//m_emitter->m_Emitter.reset(new my::Emitter());
	//m_emitter->m_Material.reset(new Material());
	//m_emitter->m_Material->m_Shader = theApp.default_shader;
	//m_emitter->m_Material->ParseShaderParameters();
	//m_emitter->RequestResource();
	////m_emitter->Spawn(my::Vector3(0,0,0), my::Vector3(0,0,0), D3DCOLOR_ARGB(255,255,255,255), my::Vector2(1,1), 0);
	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\atlmfc\src\mfc\winsplit.cpp
	CCreateContext param;
	param.m_pNewViewClass = RUNTIME_CLASS(CChildView);
	return m_wndSplitter.Create(this,
		2, 2,               // TODO: adjust the number of rows, columns
		CSize(10, 10),      // TODO: adjust the minimum pane size
		&param, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT, AFX_IDW_PANE_FIRST);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

void CMainFrame::UpdateSelBox(void)
{
	if (!m_selactors.empty())
	{
		m_selbox = my::AABB(FLT_MAX, -FLT_MAX);
		SelActorList::const_iterator actor_iter = m_selactors.begin();
		for (; actor_iter != m_selactors.end(); actor_iter++)
		{
			m_selbox.unionSelf((*actor_iter)->m_aabb.transform((*actor_iter)->m_World));
		}
	}
}

void CMainFrame::UpdatePivotTransform(void)
{
	if (m_selactors.size() == 1)
	{
		m_Pivot.m_Pos = (*m_selactors.begin())->m_Position;
		m_Pivot.m_Rot = (m_Pivot.m_Mode == Pivot::PivotModeMove ? my::Quaternion::Identity() : (*m_selactors.begin())->m_Rotation);
	}
	else if (!m_selactors.empty())
	{
		m_Pivot.m_Pos = m_selbox.Center();
		m_Pivot.m_Rot = my::Quaternion::Identity();
	}
}

void CMainFrame::OnFrameTick(float fElapsedTime)
{
	SelActorList::iterator actor_iter = m_selactors.begin();
	for (; actor_iter != m_selactors.end(); actor_iter++)
	{
		(*actor_iter)->Update(fElapsedTime);
	}

	PhysxScene::AdvanceSync(fElapsedTime);

	bool haveSelActors = false;
	physx::PxU32 nbActiveTransforms;
	const physx::PxActiveTransform* activeTransforms = m_PxScene->getActiveTransforms(nbActiveTransforms);
	for (physx::PxU32 i = 0; i < nbActiveTransforms; ++i)
	{
		if (activeTransforms[i].userData)
		{
			Actor* actor = (Actor*)activeTransforms[i].userData;
			actor->OnPxTransformChanged(activeTransforms[i].actor2World);
			if (!haveSelActors && m_selactors.end() != std::find(m_selactors.begin(), m_selactors.end(), actor))
			{
				haveSelActors = true;
			}
		}
	}

	if (haveSelActors)
	{
		UpdateSelBox();
		UpdatePivotTransform();
	}
}

void CMainFrame::OnSelChanged()
{
	if (m_PaintType != PaintTypeNone)
	{
		m_PaintType = PaintTypeNone;
	}

	UpdateSelBox();
	UpdatePivotTransform();
	my::EventArg arg;
	m_EventSelectionChanged(&arg);
}

void CMainFrame::InitFileContext()
{
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
		luabind::class_<CWnd> ("CWnd")

		, luabind::class_<CMainFrame, luabind::bases<CWnd, PhysxScene> >("MainFrame")
			.def("AddEntity", &CMainFrame::AddEntity)
			.def("RemoveEntity", &CMainFrame::RemoveEntity)
			.def("ClearAllEntity", &CMainFrame::ClearAllEntity)
			.def_readonly("selactors", &CMainFrame::m_selactors, luabind::return_stl_iterator)

		, luabind::class_<CMainApp, luabind::bases<my::D3DContext, my::ResourceMgr> >("MainApp")
			.def_readonly("MainWnd", &CMainApp::m_pMainWnd)
			.def_readonly("SkyLightCam", &CMainApp::m_SkyLightCam)
			.def_readwrite("SkyLightColor", &CMainApp::m_SkyLightColor)
			.def_readwrite("AmbientColor", &CMainApp::m_AmbientColor)
			.def_readonly("Font", &CMainApp::m_Font)
	];
	luabind::globals(m_State)["theApp"] = &theApp;
}

void CMainFrame::ClearFileContext()
{
	OctRoot::ClearAllEntity();
	theApp.ReleaseResource();
	m_ActorList.clear();
	m_selactors.clear();
	LuaContext::Shutdown();
	_ASSERT(theApp.m_NamedObjects.empty());
	my::NamedObject::ResetUniqueNameIndex();
}

BOOL CMainFrame::OpenFileContext(LPCTSTR lpszFileName)
{
	my::IStreamBuff buff(my::FileIStream::Open(lpszFileName));
	std::istream ifs(&buff);
	LPCTSTR Ext = PathFindExtension(lpszFileName);
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia;
	if (_tcsicmp(Ext, _T(".xml")) == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::xml_iarchive>
			, public PhysxSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags = 0)
				: polymorphic_iarchive_route(is, flags)
			{
			}
		};
		ia.reset(new Archive(ifs));
	}
	else if (_tcsicmp(Ext, _T(".txt")) == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::text_iarchive>
			, public PhysxSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags = 0)
				: polymorphic_iarchive_route(is, flags)
			{
			}
		};
		ia.reset(new Archive(ifs));
	}
	else
	{
		class Archive
			: public boost::archive::detail::polymorphic_iarchive_route<boost::archive::binary_iarchive>
			, public PhysxSerializationContext
		{
		public:
			Archive(std::istream& is, unsigned int flags = 0)
				: polymorphic_iarchive_route(is, flags)
			{
			}
		};
		ia.reset(new Archive(ifs));
	}
	*ia >> boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)theApp);
	*ia >> boost::serialization::make_nvp("OctRoot", (OctRoot &)*this);
	*ia >> boost::serialization::make_nvp("ActorList", m_ActorList);

	ActorPtrSet::const_iterator actor_iter = m_ActorList.begin();
	for (; actor_iter != m_ActorList.end(); actor_iter++)
	{
		AddEntity(actor_iter->get(), (*actor_iter)->m_aabb.transform((*actor_iter)->m_World), Actor::MinBlock, Actor::Threshold);
	}
	return TRUE;
}

BOOL CMainFrame::SaveFileContext(LPCTSTR lpszPathName)
{
	std::ofstream ofs(lpszPathName, std::ios::binary);
	LPCTSTR Ext = PathFindExtension(lpszPathName);
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa;
	if (_tcsicmp(Ext, _T(".xml")) == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_oarchive_route<boost::archive::xml_oarchive>
			, public PhysxSerializationContext
		{
		public:
			Archive(std::ostream& os, unsigned int flags = 0)
				: polymorphic_oarchive_route(os, flags)
			{
			}
		};
		oa.reset(new Archive(ofs));
	}
	else if (_tcsicmp(Ext, _T(".txt")) == 0)
	{
		class Archive
			: public boost::archive::detail::polymorphic_oarchive_route<boost::archive::text_oarchive>
			, public PhysxSerializationContext
		{
		public:
			Archive(std::ostream& os, unsigned int flags = 0)
				: polymorphic_oarchive_route(os, flags)
			{
			}
		};
		oa.reset(new Archive(ofs));
	}
	else
	{
		class Archive
			: public boost::archive::detail::polymorphic_oarchive_route<boost::archive::binary_oarchive>
			, public PhysxSerializationContext
		{
		public:
			Archive(std::ostream& os, unsigned int flags = 0)
				: polymorphic_oarchive_route(os, flags)
			{
			}
		};
		oa.reset(new Archive(ofs));
	}
	*oa << boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)theApp);
	*oa << boost::serialization::make_nvp("OctRoot", (OctRoot &)*this);

	struct Callback : public my::OctNode::QueryCallback
	{
		CMainFrame * pFrame;
		CMainFrame::ActorPtrSet m_ActorList;
		Callback(CMainFrame * _pFrame)
			: pFrame(_pFrame)
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			ASSERT(dynamic_cast<Actor *>(oct_entity));
			Actor * actor = static_cast<Actor *>(oct_entity);
			m_ActorList.insert(actor->shared_from_this());
		}
	} cb(this);
	QueryEntityAll(&cb);

	// ! save all actor in the scene, including lua context actor
	*oa << boost::serialization::make_nvp("ActorList", cb.m_ActorList);

	return TRUE;
}

bool CMainFrame::ExecuteCode(const char * code)
{
	CWaitCursor wait;
	if (dostring(code, "CMainFrame::ExecuteCode") && !lua_isnil(m_State, -1))
	{
		std::string msg = lua_tostring(m_State, -1);
		if (msg.empty())
			msg = "error object is not a string";
		lua_pop(m_State, 1);

		theApp.m_EventLog(msg.c_str());

		return false;
	}
	return false;
}

void CMainFrame::AddEntity(my::OctEntity * entity, const my::AABB & aabb, float minblock, float threshold)
{
	OctNode::AddEntity(entity, aabb, minblock, threshold);
}

bool CMainFrame::RemoveEntity(my::OctEntity * entity)
{
	Actor * actor = dynamic_cast<Actor *>(entity);

	actor->StopAllAction();

	actor->ClearAllAttacher();

	if (actor->m_Base)
	{
		actor->m_Base->Detach(actor);
	}

	if (actor->IsRequested())
	{
		actor->LeavePhysxScene(this);

		actor->ReleaseResource();
	}

	SelActorList::iterator actor_iter = std::find(m_selactors.begin(), m_selactors.end(), actor);
	if (actor_iter != m_selactors.end())
	{
		m_selactors.erase(actor_iter);
	}

	return OctNode::RemoveEntity(entity);
}

void CMainFrame::OnMeshComponentReady(my::DeviceResourceBasePtr res, boost::weak_ptr<MeshComponent> mesh_cmp_weak_ptr)
{
	MeshComponentPtr mesh_cmp = mesh_cmp_weak_ptr.lock();
	if (mesh_cmp)
	{
		ASSERT(mesh_cmp->m_Actor);
		if (mesh_cmp->m_Actor->m_aabb == my::AABB(-1, 1))
		{
			mesh_cmp->m_Actor->UpdateAABB();
		}
		else
		{
			my::OgreMeshPtr mesh = boost::dynamic_pointer_cast<my::OgreMesh>(res);
			ASSERT(mesh);
			mesh_cmp->m_Actor->m_aabb.unionSelf(mesh->CalculateAABB(mesh_cmp->m_MeshSubMeshId));
		}
		mesh_cmp->m_Actor->UpdateOctNode();
		if (std::find(m_selactors.begin(), m_selactors.end(), mesh_cmp->m_Actor) != m_selactors.end())
		{
			UpdateSelBox();
		}
	}
}

Component* CMainFrame::GetSelComponent(Component::ComponentType Type)
{
	if (!m_selactors.empty())
	{
		Actor::ComponentPtrList::iterator cmp_iter = m_selactors.front()->m_Cmps.begin();
		for (; cmp_iter != m_selactors.front()->m_Cmps.end(); cmp_iter++)
		{
			if ((*cmp_iter)->m_Type == Type)
			{
				return cmp_iter->get();
			}
		}
	}

	return NULL;
}

void CMainFrame::OnDestroy()
{
	CFrameWndEx::OnDestroy();

	// TODO: Add your message handler code here
	//m_emitter.reset();
	ClearFileContext();
	theApp.DestroyD3DDevice();
	PhysxScene::Shutdown();
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_MENU && m_bEatAltUp)
	{
		m_bEatAltUp = FALSE;
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	__super::OnActivate(nState, pWndOther, bMinimized);

	// TODO: Add your message handler code here
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
	{
		m_wndScript.m_editScript.CheckFileChange();
	}
}

void CMainFrame::OnFileNew()
{
	// TODO: Add your command handler code here
	m_strPathName.Empty();
	ClearFileContext();
	InitFileContext();
	InitialUpdateFrame(NULL, TRUE);
	theApp.m_BgColor = my::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	theApp.m_SkyLightCam.m_Euler = my::Vector3(D3DXToRadian(-45), 0, 0);
	theApp.m_SkyLightColor = my::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	theApp.m_AmbientColor = my::Vector4(0.3f, 0.3f, 0.3f, 0.3f);
	for (int i = 0; i < _countof(theApp.m_SkyBoxTextures); i++)
	{
		theApp.m_SkyBoxTextures[i].m_TexturePath = theApp.default_sky_texture[i];
	}
	theApp.RequestResource();
	OnSelChanged();
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CEnvironmentWnd::CameraPropEventArgs arg(pView);
	m_EventCameraPropChanged(&arg);

	// TODO:
	//MaterialPtr mtl(new Material());
	//mtl->m_Shader = "shader/mtl_water2.fx";
	//mtl->ParseShaderParameters();

	////MeshComponentPtr mesh_cmp(new MeshComponent());
	////mesh_cmp->m_MeshPath = "mesh/Teapot.mesh.xml";
	////mesh_cmp->m_MeshSubMeshName = "Teapot001";
	////mesh_cmp->AddMaterial(mtl);

	//TerrainPtr terrain(new Terrain(my::NamedObject::MakeUniqueName("editor_terrain").c_str(), 2, 2, 32, 1.0f));
	//terrain->AddMaterial(mtl);

	//ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("editor_actor").c_str(), my::Vector3(-terrain->m_RowChunks*terrain->m_ChunkSize/2, 0, -terrain->m_ColChunks*terrain->m_ChunkSize/2), my::Quaternion::Identity(), my::Vector3(1, 1, 1), my::AABB(-1, 1)));
	//actor->AddComponent(terrain);
	//actor->UpdateAABB();
	//actor->UpdateWorld();
	//AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World));
	//m_ActorList.insert(actor);

	//m_selactors.clear();
	//m_selactors.insert(actor.get());
	//m_selchunkid.SetPoint(0, 0);
	//OnSelChanged();
}

void CMainFrame::OnFileOpen()
{
	// TODO: Add your command handler code here
	CString strPathName;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
	dlg.m_ofn.lpstrFile = strPathName.GetBuffer(_MAX_PATH);
	INT_PTR nResult = dlg.DoModal();
	strPathName.ReleaseBuffer();
	if (nResult != IDOK)
	{
		return;
	}

	m_strPathName = strPathName;

	theApp.AddToRecentFileList(strPathName);

	CWaitCursor wait;
	ClearFileContext();
	InitFileContext();
	OpenFileContext(strPathName);

	theApp.RequestResource();

	OnSelChanged();

	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CEnvironmentWnd::CameraPropEventArgs arg(pView);
	m_EventCameraPropChanged(&arg);
}

void CMainFrame::OnFileSave()
{
	// TODO: Add your command handler code here
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if (dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		OnFileSaveAs();
	}
	else
	{
		SaveFileContext(m_strPathName);
	}
}

void CMainFrame::OnFileSaveAs()
{
	// TODO: Add your command handler code here
	CString strPathName;
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
	dlg.m_ofn.lpstrFile = strPathName.GetBuffer(_MAX_PATH);
	INT_PTR nResult = dlg.DoModal();
	strPathName.ReleaseBuffer();
	if (nResult != IDOK)
	{
		return;
	}

	m_strPathName = strPathName;

	CWaitCursor wait;
	SaveFileContext(m_strPathName);

	theApp.AddToRecentFileList(m_strPathName);
}

void CMainFrame::OnCreateActor()
{
	// TODO: Add your command handler code here
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	my::Vector3 Pos(0,0,0);
	if (pView)
	{
		Pos = boost::dynamic_pointer_cast<my::ModelViewerCamera>(pView->m_Camera)->m_LookAt;
	}
	ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("editor_actor").c_str(), Pos, my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1)));
	actor->UpdateWorld();
	AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World), Actor::MinBlock, Actor::Threshold);
	m_ActorList.insert(actor);

	m_selactors.clear();
	m_selactors.push_back(actor.get());
	OnSelChanged();
}

void CMainFrame::OnCreateCharacter()
{
	//// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CharacterPtr character_cmp(new Character(my::NamedObject::MakeUniqueName("editor_character_cmp").c_str(), 1.0f, 1.0f, 0.1f, 1));
	(*actor_iter)->AddComponent(character_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateCreateCharacter(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnComponentMesh()
{
	// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	std::string path = theApp.GetRelativePath(ts2ms((LPCTSTR)dlg.GetPathName()).c_str());
	if (path.empty())
	{
		MessageBox(str_printf(_T("cannot relative path: %s"), (LPCTSTR)dlg.GetPathName()).c_str());
		return;
	}

	my::CachePtr cache = my::FileIStream::Open(dlg.GetPathName())->GetWholeCache();
	cache->push_back(0);
	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>((char*)&(*cache)[0]);
	}
	catch (rapidxml::parse_error& e)
	{
		theApp.m_EventLog(e.what());
		return;
	}

	const rapidxml::xml_node<char>* node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	rapidxml::xml_node<char>* node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	if (node_sharedgeometry)
	{
		int submesh_i = 0;
		for (; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			MeshComponentPtr mesh_cmp(new MeshComponent(my::NamedObject::MakeUniqueName("editor_mesh_cmp").c_str()));
			mesh_cmp->m_MeshPath = path;
			mesh_cmp->m_MeshSubMeshId = submesh_i;
			MaterialPtr mtl(new Material());
			mtl->m_Shader = theApp.default_shader;
			mtl->ParseShaderParameters();
			mesh_cmp->SetMaterial(mtl);
			(*actor_iter)->AddComponent(mesh_cmp);

			theApp.LoadMeshAsync(path.c_str(), "", boost::bind(&CMainFrame::OnMeshComponentReady, this, boost::placeholders::_1, boost::weak_ptr<MeshComponent>(mesh_cmp)));
		}
	}
	else
	{
		DEFINE_XML_NODE_SIMPLE(submeshnames, mesh);
		DEFINE_XML_NODE_SIMPLE(submeshname, submeshnames);
		for (; node_submesh != NULL && node_submeshname != NULL; node_submesh = node_submesh->next_sibling(), node_submeshname = node_submeshname->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_SIMPLE(name, submeshname);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(index, submeshname);
			MeshComponentPtr mesh_cmp(new MeshComponent(my::NamedObject::MakeUniqueName("editor_mesh_cmp").c_str()));
			mesh_cmp->m_MeshPath = path;
			mesh_cmp->m_MeshSubMeshName = attr_name->value();
			MaterialPtr mtl(new Material());
			mtl->m_Shader = theApp.default_shader;
			mtl->ParseShaderParameters();
			mesh_cmp->SetMaterial(mtl);
			(*actor_iter)->AddComponent(mesh_cmp);

			theApp.LoadMeshAsync(path.c_str(), attr_name->value(), boost::bind(&CMainFrame::OnMeshComponentReady, this, boost::placeholders::_1, boost::weak_ptr<MeshComponent>(mesh_cmp)));
		}
	}

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentMesh(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnComponentCloth()
{
	// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	my::CachePtr cache = my::FileIStream::Open(dlg.GetPathName())->GetWholeCache();
	cache->push_back(0);
	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>((char*)&(*cache)[0]);
	}
	catch (rapidxml::parse_error& e)
	{
		theApp.m_EventLog(e.what());
		return;
	}

	const rapidxml::xml_node<char>* node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	rapidxml::xml_node<char>* node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	if (node_sharedgeometry)
	{
		rapidxml::xml_node<char>* node_boneassignments = node_mesh->first_node("boneassignments");
		my::OgreMeshPtr mesh(new my::OgreMesh());
		mesh->CreateMeshFromOgreXmlNodes(node_sharedgeometry, node_boneassignments, node_submesh, true);
		std::vector<D3DXATTRIBUTERANGE>::iterator att_iter = mesh->m_AttribTable.begin();
		for (; att_iter != mesh->m_AttribTable.end(); att_iter++)
		{
			ClothComponentPtr cloth_cmp(new ClothComponent(my::NamedObject::MakeUniqueName("editor_cloth_cmp").c_str()));
			cloth_cmp->CreateClothFromMesh(mesh, std::distance(mesh->m_AttribTable.begin(), att_iter));
			MaterialPtr mtl(new Material());
			mtl->m_Shader = theApp.default_shader;
			mtl->ParseShaderParameters();
			cloth_cmp->SetMaterial(mtl);
			(*actor_iter)->AddComponent(cloth_cmp);
		}
	}
	else
	{
		DEFINE_XML_NODE_SIMPLE(submeshnames, mesh);
		DEFINE_XML_NODE_SIMPLE(submeshname, submeshnames);
		for (; node_submesh != NULL && node_submeshname != NULL; node_submesh = node_submesh->next_sibling(), node_submeshname = node_submeshname->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_SIMPLE(name, submeshname);
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(index, submeshname);
			DEFINE_XML_NODE_SIMPLE(geometry, submesh);
			rapidxml::xml_node<char>* node_boneassignments = node_submesh->first_node("boneassignments");
			my::OgreMeshPtr mesh(new my::OgreMesh());
			mesh->CreateMeshFromOgreXmlNodes(node_geometry, node_boneassignments, node_submesh, false);
			ClothComponentPtr cloth_cmp(new ClothComponent(my::NamedObject::MakeUniqueName("editor_cloth_cmp").c_str()));
			cloth_cmp->CreateClothFromMesh(mesh, 0);
			MaterialPtr mtl(new Material());
			mtl->m_Shader = theApp.default_shader;
			mtl->ParseShaderParameters();
			cloth_cmp->SetMaterial(mtl);
			(*actor_iter)->AddComponent(cloth_cmp);
		}
	}

	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentCloth(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnComponentStaticEmitter()
{
	// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	StaticEmitterComponentPtr emit_cmp(new StaticEmitterComponent(my::NamedObject::MakeUniqueName("editor_emit_cmp").c_str(), 1, EmitterComponent::FaceTypeCamera, EmitterComponent::SpaceTypeLocal));
	MaterialPtr mtl(new Material());
	mtl->m_Shader = theApp.default_shader;
	mtl->ParseShaderParameters();
	emit_cmp->SetMaterial(mtl);
	(*actor_iter)->AddComponent(emit_cmp);
	emit_cmp->Spawn(my::Vector3(0, 0, 0), my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0.0f, 0.0f);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentStaticEmitter(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnComponentSphericalemitter()
{
	//// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	SphericalEmitterComponentPtr sphe_emit_cmp(new SphericalEmitterComponent(my::NamedObject::MakeUniqueName("editor_sphe_emit_cmp").c_str(), 4096, EmitterComponent::FaceTypeCamera, EmitterComponent::SpaceTypeLocal));
	sphe_emit_cmp->m_ParticleLifeTime=10.0f;
	sphe_emit_cmp->m_SpawnInterval=1/100.0f;
	sphe_emit_cmp->m_SpawnSpeed=5;
	sphe_emit_cmp->m_SpawnInclination.AddNode(0,D3DXToRadian(45),0,0);
	float Azimuth=D3DXToRadian(360)*8;
	sphe_emit_cmp->m_SpawnAzimuth.AddNode(0,0,Azimuth/10,Azimuth/10);
	sphe_emit_cmp->m_SpawnAzimuth.AddNode(10,Azimuth,Azimuth/10,Azimuth/10);
	sphe_emit_cmp->m_SpawnColorA.AddNode(0,1,0,0);
	sphe_emit_cmp->m_SpawnColorA.AddNode(10,0,0,0);
	sphe_emit_cmp->m_SpawnColorR.AddNode(0,1,0,0);
	sphe_emit_cmp->m_SpawnColorR.AddNode(10,0,0,0);
	sphe_emit_cmp->m_SpawnColorG.AddNode(0,1,0,0);
	sphe_emit_cmp->m_SpawnColorG.AddNode(10,0,0,0);
	sphe_emit_cmp->m_SpawnColorB.AddNode(0,1,0,0);
	sphe_emit_cmp->m_SpawnColorB.AddNode(10,0,0,0);
	sphe_emit_cmp->m_SpawnSizeX.AddNode(0,1,0,0);
	sphe_emit_cmp->m_SpawnSizeX.AddNode(10,10,0,0);
	sphe_emit_cmp->m_SpawnSizeY.AddNode(0,1,0,0);
	sphe_emit_cmp->m_SpawnSizeY.AddNode(10,10,0,0);
	MaterialPtr mtl(new Material());
	mtl->m_Shader = theApp.default_shader;
	mtl->ParseShaderParameters();
	sphe_emit_cmp->SetMaterial(mtl);
	(*actor_iter)->AddComponent(sphe_emit_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentSphericalemitter(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnComponentTerrain()
{
	// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CTerrainDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	(*actor_iter)->AddComponent(dlg.m_terrain);
	if (dlg.m_AlignToCenter)
	{
		my::Vector3 center = dlg.m_terrain->Center();
		(*actor_iter)->m_Position.x -= center.x;
		(*actor_iter)->m_Position.z -= center.z;
		(*actor_iter)->UpdateWorld();
		// TODO: update pxactor, ref Actor::SetPose
	}
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();
	UpdatePivotTransform();
	m_selchunkid.SetPoint(0, 0);

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentTerrain(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnEditDelete()
{
	// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	for (; actor_iter != m_selactors.end(); actor_iter = m_selactors.begin())
	{
		ActorPtr actor = (*actor_iter)->shared_from_this();

		RemoveEntity(actor.get());

		m_ActorList.erase(actor);
	}

	OnSelChanged();
}

void CMainFrame::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnPivotMove()
{
	// TODO: Add your command handler code here
	m_PaintType = PaintTypeNone;
	m_Pivot.m_Mode = Pivot::PivotModeMove;
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePivotMove(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_PaintType == PaintTypeNone && m_Pivot.m_Mode == Pivot::PivotModeMove);
}

void CMainFrame::OnPivotRotate()
{
	// TODO: Add your command handler code here
	m_PaintType = PaintTypeNone;
	m_Pivot.m_Mode = Pivot::PivotModeRot;
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePivotRotate(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_PaintType == PaintTypeNone && m_Pivot.m_Mode == Pivot::PivotModeRot);
}

void CMainFrame::OnViewClearshader()
{
	// TODO: Add your command handler code here
	theApp.m_ShaderCache.clear();
	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnToolsBuildnavigation()
{
	//// TODO: Add your command handler code here
	CNavigationDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
}

void CMainFrame::OnPaintTerrainHeightField()
{
	// TODO: Add your command handler code here
	if (m_PaintType == PaintTypeTerrainHeightField)
	{
		m_PaintType = PaintTypeNone;
	}
	else
	{
		m_PaintType = PaintTypeTerrainHeightField;
	}
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePaintTerrainHeightField(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	Terrain* terrain = dynamic_cast<Terrain*>(GetSelComponent(Component::ComponentTypeTerrain));
	if (terrain)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_PaintType == PaintTypeTerrainHeightField);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnPaintTerrainColor()
{
	// TODO: Add your command handler code here
	if (m_PaintType == PaintTypeTerrainColor)
	{
		m_PaintType = PaintTypeNone;
	}
	else
	{
		m_PaintType = PaintTypeTerrainColor;
	}
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePaintTerrainColor(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	Terrain* terrain = dynamic_cast<Terrain*>(GetSelComponent(Component::ComponentTypeTerrain));
	if (terrain)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_PaintType == PaintTypeTerrainColor);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnPaintEmitterinstance()
{
	// TODO: Add your command handler code here
	if (m_PaintType == PaintTypeEmitterInstance)
	{
		m_PaintType = PaintTypeNone;
	}
	else
	{
		m_PaintType = PaintTypeEmitterInstance;
	}
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePaintEmitterinstance(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	StaticEmitterComponent* emit = dynamic_cast<StaticEmitterComponent*>(GetSelComponent(Component::ComponentTypeStaticEmitter));
	if (emit)
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_PaintType == PaintTypeEmitterInstance);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnComponentAnimator()
{
	// TODO: Add your command handler code here
	SelActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	std::string path = theApp.GetRelativePath(ts2ms((LPCTSTR)dlg.GetPathName()).c_str());
	if (path.empty())
	{
		MessageBox(str_printf(_T("cannot relative path: %s"), (LPCTSTR)dlg.GetPathName()).c_str());
		return;
	}

	my::CachePtr cache = my::FileIStream::Open(dlg.GetPathName())->GetWholeCache();
	cache->push_back(0);
	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>((char*)&(*cache)[0]);
	}
	catch (rapidxml::parse_error& e)
	{
		theApp.m_EventLog(e.what());
		return;
	}

	const rapidxml::xml_node<char>* node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(skeleton, root);
	rapidxml::xml_node<char>* node_animations = node_skeleton->first_node("animations");
	if (node_animations != NULL)
	{
		DEFINE_XML_NODE_SIMPLE(animation, animations);
		DEFINE_XML_ATTRIBUTE_SIMPLE(name, animation);
		AnimationNodeSequencePtr seq(new AnimationNodeSequence());
		seq->m_Name = attr_name->value();
		AnimatorPtr animator(new Animator(my::NamedObject::MakeUniqueName("editor_animator_cmp").c_str()));
		animator->SetChild<0>(seq);
		animator->ReloadSequenceGroup();
		animator->m_SkeletonPath = path;
		(*actor_iter)->AddComponent(animator);
	}

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentAnimator(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

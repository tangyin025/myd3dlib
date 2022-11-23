
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainFrm.h"
#include "ChildView.h"
#include "TerrainDlg.h"
#include "StaticEmitterDlg.h"
#include "Material.h"
#include "Controller.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/string.hpp>
#include <fstream>
#include "NavigationSerialization.h"
#include "NavigationDlg.h"
#include "SimplifyMeshDlg.h"
#include "Animator.h"
#include "Win32InputBox.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/tag_function.hpp>
#include "LuaExtension.inl"
#include "myException.h"
#include <boost/scope_exit.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/shared_container_iterator.hpp>
#include <boost/algorithm/string.hpp>
#include "DeleteCmpsDlg.h"
#include "SnapshotDlg.h"
#include "PlayerAgent.h"
#include "Steering.h"
#include "ActionTrack.h"
#include "rapidxml.hpp"
#include <lualib.h>
#include "TerrainToObjDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int lua_print(lua_State * L)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_wndOutput.m_wndOutputDebug.SetSel(-1, -1);

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
			pFrame->m_wndOutput.m_wndOutputDebug.ReplaceSel(_T("\t"));
		pFrame->m_wndOutput.m_wndOutputDebug.ReplaceSel(u8tots(s).c_str());
		lua_pop(L, 1);  /* pop result */
	}
	pFrame->m_wndOutput.m_wndOutputDebug.ReplaceSel(_T("\n"));
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
		CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT_VALID(pFrame);
		lf.stream = theApp.OpenIStream((pFrame->m_ToolScriptDir + "\\" + filename).c_str());
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
//
//static int io_open(lua_State* L) {
//	const char* filename = luaL_checkstring(L, 1);
//	const char* mode = luaL_optstring(L, 2, "r");
//	FILE** pf = (FILE**)lua_newuserdata(L, sizeof(FILE*));
//	*pf = NULL;  /* file handle is currently `closed' */
//	luaL_getmetatable(L, LUA_FILEHANDLE);
//	lua_setmetatable(L, -2);
//	*pf = _tfopen(u8tots(filename).c_str(), ms2ts(mode).c_str());
//	if (*pf == NULL)
//	{
//		int en = errno;  /* calls to Lua API may change this value */
//		lua_pushnil(L);
//		if (filename)
//			lua_pushfstring(L, "%s: %s", filename, strerror(en));
//		else
//			lua_pushfstring(L, "%s", strerror(en));
//		lua_pushinteger(L, en);
//		return 3;
//	}
//	return 1;
//}

static CPoint cchildview_get_cursor_pos(const CChildView* self)
{
	CPoint point;
	GetCursorPos(&point);
	self->ScreenToClient(&point);
	return point;
}

typedef boost::shared_container_iterator<CMainFrame::ActorList> shared_actor_list_iter;

static boost::iterator_range<shared_actor_list_iter> cmainframe_get_all_acts(const CMainFrame* self)
{
	boost::shared_ptr<CMainFrame::ActorList> acts(new CMainFrame::ActorList());
	struct Callback : public my::OctNode::QueryCallback
	{
		boost::shared_ptr<CMainFrame::ActorList> acts;
		Callback(boost::shared_ptr<CMainFrame::ActorList> _acts)
			: acts(_acts)
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			acts->push_back(dynamic_cast<Actor*>(oct_entity));
			return true;
		}
	} cb(acts);
	self->QueryAllEntity(&cb);
	return boost::make_iterator_range(shared_actor_list_iter(acts->begin(), acts), shared_actor_list_iter(acts->end(), acts));
}

static bool cmainapp_query_shader(RenderPipeline* self, RenderPipeline::MeshType mesh_type, const luabind::object& macro, const char* path, unsigned int PassID)
{
	std::vector<D3DXMACRO> macs;
	luabind::iterator iter(macro), end;
	for (; iter != end; iter++)
	{
		std::string key = boost::lexical_cast<std::string>(iter.key());
		std::string value = boost::lexical_cast<std::string>(*iter);
		D3DXMACRO m = { key.c_str(),value.c_str() };
		macs.push_back(m);
	}
	D3DXMACRO m = { 0 };
	macs.push_back(m);
	return self->QueryShader(mesh_type, macs.data(), path, PassID);
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
	ON_COMMAND(ID_FILE_CLOSE, &CMainFrame::OnFileClose)
	ON_COMMAND(ID_FILE_SAVE, &CMainFrame::OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CMainFrame::OnFileSaveAs)
	ON_COMMAND(ID_CREATE_ACTOR, &CMainFrame::OnCreateActor)
	ON_COMMAND(ID_COMPONENT_CONTROLLER, &CMainFrame::OnCreateController)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_CONTROLLER, &CMainFrame::OnUpdateCreateController)
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
	ON_COMMAND(ID_PAINT_TERRAINHEIGHTFIELD, &CMainFrame::OnPaintTerrainHeightField)
	ON_UPDATE_COMMAND_UI(ID_PAINT_TERRAINHEIGHTFIELD, &CMainFrame::OnUpdatePaintTerrainHeightField)
	ON_COMMAND(ID_PAINT_TERRAINCOLOR, &CMainFrame::OnPaintTerrainColor)
	ON_UPDATE_COMMAND_UI(ID_PAINT_TERRAINCOLOR, &CMainFrame::OnUpdatePaintTerrainColor)
	ON_COMMAND(ID_PAINT_EMITTERINSTANCE, &CMainFrame::OnPaintEmitterinstance)
	ON_UPDATE_COMMAND_UI(ID_PAINT_EMITTERINSTANCE, &CMainFrame::OnUpdatePaintEmitterinstance)
	ON_COMMAND(ID_TOOLS_OFFMESHCON, &CMainFrame::OnToolsOffmeshconnections)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_OFFMESHCON, &CMainFrame::OnUpdateToolsOffmeshconnections)
	ON_COMMAND(ID_COMPONENT_ANIMATOR, &CMainFrame::OnComponentAnimator)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_ANIMATOR, &CMainFrame::OnUpdateComponentAnimator)
	ON_COMMAND(ID_COMPONENT_NAVIGATION, &CMainFrame::OnCreateNavigation)
	ON_UPDATE_COMMAND_UI(ID_COMPONENT_NAVIGATION, &CMainFrame::OnUpdateCreateNavigation)
	ON_COMMAND(ID_CREATE_DIALOG, &CMainFrame::OnCreateDialog)
	ON_COMMAND(ID_CONTROL_STATIC, &CMainFrame::OnControlStatic)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_STATIC, &CMainFrame::OnUpdateControlStatic)
	ON_COMMAND(ID_CONTROL_PROGRESSBAR, &CMainFrame::OnControlProgressbar)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_PROGRESSBAR, &CMainFrame::OnUpdateControlProgressbar)
	ON_COMMAND(ID_CONTROL_BUTTON, &CMainFrame::OnControlButton)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_BUTTON, &CMainFrame::OnUpdateControlButton)
	ON_COMMAND(ID_CONTROL_IMEEDITBOX, &CMainFrame::OnControlImeeditbox)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_IMEEDITBOX, &CMainFrame::OnUpdateControlImeeditbox)
	ON_COMMAND(ID_CONTROL_CHECKBOX, &CMainFrame::OnControlCheckbox)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_CHECKBOX, &CMainFrame::OnUpdateControlCheckbox)
	ON_COMMAND(ID_CONTROL_COMBOBOX, &CMainFrame::OnControlCombobox)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_COMBOBOX, &CMainFrame::OnUpdateControlCombobox)
	ON_COMMAND(ID_CONTROL_LISTBOX, &CMainFrame::OnControlListbox)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_LISTBOX, &CMainFrame::OnUpdateControlListbox)
	ON_COMMAND_RANGE(ID_TOOLS_SCRIPT1, ID_TOOLS_SCRIPT_LAST, &CMainFrame::OnToolsScript1)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TOOLS_SCRIPT1, ID_TOOLS_SCRIPT_LAST, &CMainFrame::OnUpdateToolsScript1)
	ON_COMMAND(ID_TOOLS_SNAPSHOT, &CMainFrame::OnToolsSnapshot)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_COORD, &CMainFrame::OnUpdateIndicatorCoord)
	ON_COMMAND(ID_TOOLS_PLAYING, &CMainFrame::OnToolsPlaying)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_PLAYING, &CMainFrame::OnUpdateToolsPlaying)
	ON_COMMAND(ID_TOOLS_SNAP_TO_GRID, &CMainFrame::OnToolsSnapToGrid)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_SNAP_TO_GRID, &CMainFrame::OnUpdateToolsSnapToGrid)
	ON_COMMAND(ID_TOOLS_TERRAINTOOBJ, &CMainFrame::OnToolsTerraintoobj)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TERRAINTOOBJ, &CMainFrame::OnUpdateToolsTerraintoobj)
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
	, m_hitPosSet(false)
	, m_offMeshConCount(0)
	, m_selcmp(NULL)
	, m_selchunkid(0, 0)
	, m_selinstid(0)
	, m_selbox(-1, 1)
	, m_ctlcaptured(false)
	, m_ctlhandle(ControlHandleNone)
	, m_ctlhandleoff(0, 0)
	, m_ctlhandlesz(0, 0)
	, m_PaintType(PaintTypeNone)
	, m_PaintShape(PaintShapeCircle)
	, m_PaintMode(PaintModeGreater)
	, m_PaintRadius(5.0f)
	, m_PaintHeight(5.0f)
	, m_PaintColor(1.0f, 0.0f, 0.0f, 0.0f)
	, m_PaintParticleMinDist(1.0f)
	, m_RenderingView(NULL)
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2005);
	m_PaintSpline.AddNode(0, 0, 1, 1);
	m_PaintSpline.AddNode(1, 1, 1, 1);
	m_ToolScripts.resize(ID_TOOLS_SCRIPT_LAST - ID_TOOLS_SCRIPT1);
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

	if (!PhysxScene::Init(theApp.m_sdk.get(), theApp.m_CpuDispatcher.get(), physx::PxSceneFlags(theApp.default_physx_scene_flags), theApp.default_physx_scene_gravity))
		return -1;

	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1);
	//m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_AXES, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1);

	m_Player.reset(new Actor(NULL, my::Vector3(0, 0, 0), my::Quaternion::Identity(), my::Vector3(theApp.default_player_scale), my::AABB(-1, 1)));
	m_Player->InsertComponent(ComponentPtr(new Controller(NULL, theApp.default_player_height, theApp.default_player_radius, 0.1f, 0.5f, 0.0f)));
	m_Player->InsertComponent(ComponentPtr(new Steering(NULL, theApp.default_player_max_speed, theApp.default_player_breaking_speed, 0.0f, NULL)));
	m_Player->InsertComponent(ComponentPtr(new PlayerAgent(NULL)));
	m_Player->InsertComponent(ComponentPtr(new Animator(NULL)));
	if (!theApp.default_player_anim_list.empty())
	{
		m_Player->GetFirstComponent<Animator>()->m_SkeletonPath = theApp.default_player_anim_list.front();
	}

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
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_COORD, SBPS_NORMAL, 200);

	if (!m_wndProperties.Create(_T("Properties"), this, CRect(0, 0, 200, 200), TRUE, 3001,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	if (!m_wndEnvironment.Create(_T("Environment"), this, CRect(0, 0, 200, 200), TRUE, 3002,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE))
	{
		TRACE0("Failed to create Environment window\n");
		return FALSE; // failed to create
	}

	if (!m_wndOutput.Create(_T("Output"), this, CRect(0, 0, 200, 200), TRUE, 3003,
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

	if (!m_wndOutliner.Create(_T("Outliner"), this, CRect(0, 0, 200, 200), TRUE, 3005,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE))
	{
		TRACE0("Failed to create Outliner window\n");
		return -1;
	}

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	m_wndEnvironment.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	m_wndScript.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutliner.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndOutliner);
	DockPane(&m_wndProperties);
	DockPane(&m_wndEnvironment);
	DockPane(&m_wndOutput);
	DockPane(&m_wndScript);
	CDockablePane* pTabbedBar = NULL;
	m_wndEnvironment.AttachToTabWnd(&m_wndProperties, DM_SHOW, FALSE, &pTabbedBar);
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
	////m_emitter->Spawn(my::Vector3(0,0,0,1), my::Vector3(0,0,0,1), D3DCOLOR_ARGB(255,255,255,255), my::Vector2(1,1), 0);

	m_wndOutliner.OnInitItemList();
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
		ActorList::const_iterator actor_iter = m_selactors.begin();
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
	ActorList::iterator actor_iter = m_selactors.begin();
	for (; actor_iter != m_selactors.end(); actor_iter++)
	{
		if ((*actor_iter)->IsRequested())
		{
			(*actor_iter)->Update(fElapsedTime);

			Animator* animator = (*actor_iter)->GetFirstComponent<Animator>();
			if (animator)
			{
				animator->Tick(fElapsedTime, 1.0f);
			}
		}

		Actor::ActorList::iterator attach_iter = (*actor_iter)->m_Attaches.begin();
		for (; attach_iter != (*actor_iter)->m_Attaches.end(); attach_iter++)
		{
			if ((*attach_iter)->IsRequested())
			{
				(*attach_iter)->Update(fElapsedTime);

				Animator* animator = (*attach_iter)->GetFirstComponent<Animator>();
				if (animator)
				{
					animator->Tick(fElapsedTime, 1.0f);
				}
			}
		}
	}

	if (m_Player->IsRequested())
	{
		m_Player->Update(fElapsedTime);
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
			if (!actor->m_Base || !actor->GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC)) // ! Actor::Update, m_Base->GetAttachPose
			{
				actor->SetPose((my::Vector3&)activeTransforms[i].actor2World.p, (my::Quaternion&)activeTransforms[i].actor2World.q);
			}

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
	//lua_getglobal(m_State, "io");
	//lua_pushcclosure(m_State, io_open, 0); // ! lost file:__close
	//lua_setfield(m_State, -2, "open");
	lua_settop(m_State, 0);
	luabind::module(m_State)
	[
		luabind::class_<CWnd> ("CWnd")

		, luabind::class_<CView, CWnd>("CView")

		, luabind::class_<CChildView, luabind::bases<CView, my::DrawHelper> >("ChildView")
			.def_readonly("Camera", &CChildView::m_Camera)
			.property("CursorPos", &cchildview_get_cursor_pos)
			.def_readonly("SwapChainBufferDesc", &CChildView::m_SwapChainBufferDesc)

		, luabind::class_<COutlinerWnd, luabind::bases<CWnd> >("COutlinerWnd")
			.def("OnInitItemList", &COutlinerWnd::OnInitItemList)
			.def("OnDestroyItemList", &COutlinerWnd::OnDestroyItemList)

		, luabind::class_<CMainFrame, luabind::bases<CWnd, PhysxScene> >("MainFrame")
			.def_readonly("wndOutliner", &CMainFrame::m_wndOutliner)
			.def("InsertDlg", &CMainFrame::InsertDlg)
			.def("RemoveDlg", &CMainFrame::RemoveDlg)
			.def("RemoveAllDlg", &CMainFrame::RemoveAllDlg)
			.def("AddEntity", &CMainFrame::AddEntity)
			.def("AddEntity", luabind::tag_function<void(CMainFrame*, Actor*)>(
				boost::bind(boost::mem_fn(&CMainFrame::AddEntity), boost::placeholders::_1, boost::placeholders::_2, boost::bind(&AABB::transform, boost::bind(&Actor::m_aabb, boost::placeholders::_2), boost::bind(&Actor::m_World, boost::placeholders::_2)), Actor::MinBlock, Actor::Threshold)))
			.def("RemoveEntity", &CMainFrame::RemoveEntity)
			.def("ClearAllEntity", &CMainFrame::ClearAllEntity)
			.def("PushToActorList", luabind::tag_function<void(CMainFrame*,ActorPtr)>(
				boost::bind((void(ActorPtrList::*)(ActorPtr const&)) & ActorPtrList::push_back, boost::bind<ActorPtrList&>(&CMainFrame::m_ActorList, boost::placeholders::_1), boost::placeholders::_2)))
			.property("allactors", cmainframe_get_all_acts, luabind::return_stl_iterator)
			.def_readonly("selactors", &CMainFrame::m_selactors, luabind::return_stl_iterator)
			.def_readonly("selctls", &CMainFrame::m_selctls, luabind::return_stl_iterator)
			.property("ActiveView", &CMainFrame::GetActiveView)
			.def_readonly("RenderingView", &CMainFrame::m_RenderingView)

		, luabind::class_<CMainApp, luabind::bases<my::D3DContext, my::ResourceMgr> >("MainApp")
			.def_readonly("MainWnd", &CMainApp::m_pMainWnd)
			.def_readonly("SkyLightCam", &CMainApp::m_SkyLightCam)
			.def_readwrite("SkyLightColor", &CMainApp::m_SkyLightColor)
			.def_readwrite("AmbientColor", &CMainApp::m_AmbientColor)
			.def("QueryShader", &cmainapp_query_shader)
			.def_readonly("UIRender", &CMainApp::m_UIRender)
			.def_readonly("Font", &CMainApp::m_Font)
			.def_readonly("keyboard", &CMainApp::m_keyboard)
			.def_readonly("mouse", &CMainApp::m_mouse)
	];
	luabind::globals(m_State)["theApp"] = &theApp;
}

void CMainFrame::ClearFileContext()
{
	OctRoot::ClearAllEntity();
	DialogMgr::RemoveAllDlg();
	m_ActorList.clear();
	m_DialogList.clear();
	m_hitPosSet = false;
	m_offMeshConCount = 0;
	ASSERT(m_selactors.empty());
	m_selcmp = NULL;
	m_selchunkid.SetPoint(0, 0);
	m_selinstid = 0;
	m_selctls.clear();
	ASSERT(m_ViewedActors.empty());
	LuaContext::Shutdown();
	theApp.m_CollectionObjs.clear();
	_ASSERT(theApp.m_NamedObjects.empty());
	m_wndOutput.m_wndOutputDebug.SetSel(0, -1);
	m_wndOutput.m_wndOutputDebug.ReplaceSel(_T(""));
}

BOOL CMainFrame::OpenFileContext(LPCTSTR lpszFileName)
{
	SceneContextRequest request(ts2ms(lpszFileName).c_str(), "", INT_MAX);
	request.LoadResource();
	SceneContextPtr scene = boost::dynamic_pointer_cast<SceneContext>(request.m_res);

	theApp.m_SkyLightCam->m_Euler = scene->m_SkyLightCamEuler;
	theApp.m_SkyLightColor = scene->m_SkyLightColor;
	theApp.m_AmbientColor = scene->m_AmbientColor;
	theApp.m_DofParams = scene->m_DofParams;
	theApp.m_LuminanceThreshold = scene->m_LuminanceThreshold;
	theApp.m_BloomColor = scene->m_BloomColor;
	theApp.m_BloomFactor = scene->m_BloomFactor;
	theApp.m_SsaoBias = scene->m_SsaoBias;
	theApp.m_SsaoIntensity = scene->m_SsaoIntensity;
	theApp.m_SsaoRadius = scene->m_SsaoRadius;
	theApp.m_SsaoScale = scene->m_SsaoScale;
	theApp.m_FogColor = scene->m_FogColor;
	theApp.m_FogStartDistance = scene->m_FogStartDistance;
	theApp.m_FogHeight = scene->m_FogHeight;
	theApp.m_FogFalloff = scene->m_FogFalloff;

	SceneContext::ActorPtrList::iterator act_iter = scene->m_ActorList.begin();
	for (; act_iter != scene->m_ActorList.end(); act_iter++)
	{
		m_ActorList.push_back(*act_iter);
		AddEntity(act_iter->get(), (*act_iter)->m_aabb.transform((*act_iter)->m_World), Actor::MinBlock, Actor::Threshold);
	}

	SceneContext::DialogPtrList::iterator dlg_iter = scene->m_DialogList.begin();
	for (; dlg_iter != scene->m_DialogList.end(); dlg_iter++)
	{
		m_DialogList.push_back(*dlg_iter);
		InsertDlg(dlg_iter->get());
	}

	theApp.m_EventLog(str_printf("CMainFrame::OpenFileContext: %Iu actors, %Iu dialogs", m_ActorList.size(), m_DialogList.size()).c_str());

	return TRUE;
}

BOOL CMainFrame::SaveFileContext(LPCTSTR lpszPathName)
{
	std::ofstream ofs(lpszPathName, std::ios::binary, _SH_DENYRW);
	LPCTSTR Ext = PathFindExtension(lpszPathName);
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(ofs, ts2ms(Ext).c_str());
	*oa << boost::serialization::make_nvp("SkyLightCam.m_Euler", theApp.m_SkyLightCam->m_Euler);
	*oa << boost::serialization::make_nvp("SkyLightColor", theApp.m_SkyLightColor);
	*oa << boost::serialization::make_nvp("AmbientColor", theApp.m_AmbientColor);
	*oa << boost::serialization::make_nvp("DofParams", theApp.m_DofParams);
	*oa << boost::serialization::make_nvp("LuminanceThreshold", theApp.m_LuminanceThreshold);
	*oa << boost::serialization::make_nvp("BloomColor", theApp.m_BloomColor);
	*oa << boost::serialization::make_nvp("BloomFactor", theApp.m_BloomFactor);
	*oa << boost::serialization::make_nvp("SsaoBias", theApp.m_SsaoBias);
	*oa << boost::serialization::make_nvp("SsaoIntensity", theApp.m_SsaoIntensity);
	*oa << boost::serialization::make_nvp("SsaoRadius", theApp.m_SsaoRadius);
	*oa << boost::serialization::make_nvp("SsaoScale", theApp.m_SsaoScale);
	*oa << boost::serialization::make_nvp("FogColor", theApp.m_FogColor);
	*oa << boost::serialization::make_nvp("FogStartDistance", theApp.m_FogStartDistance);
	*oa << boost::serialization::make_nvp("FogHeight", theApp.m_FogHeight);
	*oa << boost::serialization::make_nvp("FogFalloff", theApp.m_FogFalloff);

	// ! save all actor in the scene, including lua context actor
	LONG ActorListSize = GetAllEntityNum();
	*oa << BOOST_SERIALIZATION_NVP(ActorListSize);
	struct Callback : public my::OctNode::QueryCallback
	{
		boost::shared_ptr<boost::archive::polymorphic_oarchive> & oa;
		int i;
		Callback(boost::shared_ptr<boost::archive::polymorphic_oarchive> & _oa)
			: oa(_oa)
			, i(0)
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			ActorPtr actor_ptr = dynamic_cast<Actor*>(oct_entity)->shared_from_this();
			if (Component * cmp = actor_ptr->GetFirstComponent(Component::ComponentTypeScript))
			{
				CString msg;
				msg.Format(_T("invalid serialization: %S.%S"), actor_ptr->GetName(), cmp->GetName());
				AfxMessageBox(msg);
				return false;
			}
			*oa << boost::serialization::make_nvp(str_printf("Actor%d", i++).c_str(), actor_ptr);
			return true;
		}
	} cb(oa);
	QueryAllEntity(&cb);
	_ASSERT(ActorListSize == cb.i);

	LONG DialogListSize = m_DlgList.size();
	*oa << BOOST_SERIALIZATION_NVP(DialogListSize);
	int i = 0;
	DialogList::iterator dlg_iter = m_DlgList.begin();
	for (; dlg_iter != m_DlgList.end(); dlg_iter++)
	{
		my::DialogPtr dlg_ptr = boost::dynamic_pointer_cast<my::Dialog>((*dlg_iter)->shared_from_this());
		*oa << boost::serialization::make_nvp(str_printf("Dialog%d", i++).c_str(), dlg_ptr);
	}
	_ASSERT(DialogListSize == i);

	theApp.m_EventLog(str_printf("CMainFrame::SaveFileContext: %d actors, %d dialogs", cb.i, i).c_str());

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

void CMainFrame::RemoveEntity(my::OctEntity * entity)
{
	Actor * actor = dynamic_cast<Actor *>(entity);

	if (actor->IsRequested())
	{
		actor->ReleaseResource();
	}

	actor->StopAllAction();

	actor->ClearAllAttach();

	if (actor->m_Base)
	{
		actor->m_Base->Detach(actor);
	}

	if (actor->is_linked())
	{
		m_ViewedActors.erase(m_ViewedActors.iterator_to(*actor));
	}

	ActorList::iterator actor_iter = std::find(m_selactors.begin(), m_selactors.end(), actor);
	if (actor_iter != m_selactors.end())
	{
		m_selactors.erase(actor_iter);
	}

	ASSERT(HaveNode(entity->m_Node));

	OctNode::RemoveEntity(entity);
}

Component* CMainFrame::GetSelComponent(DWORD Type)
{
	if (!m_selactors.empty())
	{
		return m_selactors.front()->GetFirstComponent(Type);
	}
	return NULL;
}

void CMainFrame::OnDestroy()
{
	m_wndOutliner.OnDestroyItemList();
	ClearFileContext();
	CFrameWndEx::OnDestroy();

	// TODO: Add your message handler code here
	//m_emitter.reset();
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
	m_wndOutliner.OnDestroyItemList();
	ClearFileContext();
	InitFileContext();
	m_wndOutliner.OnInitItemList();
	InitialUpdateFrame(NULL, TRUE);
	theApp.m_SkyLightCam->m_Euler = my::Vector3(D3DXToRadian(-45), 0, 0);
	theApp.m_SkyLightColor = my::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	theApp.m_AmbientColor = my::Vector4(0.3f, 0.3f, 0.3f, 0.3f);
	OnSelChanged();
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CEnvironmentWnd::CameraPropEventArgs arg(pView);
	m_EventCameraPropChanged(&arg);

	//// TODO:
	//MaterialPtr mtl(new Material());
	//mtl->m_Shader = "shader/mtl_BlinnPhong.fx";
	//mtl->ParseShaderParameters();

	//MeshComponentPtr mesh_cmp(new MeshComponent("mesh_222323121"));
	//mesh_cmp->m_MeshPath = "mesh/Cloth.mesh.xml";
	//mesh_cmp->m_Material = mtl;

	//AnimationNodeSequencePtr seq(new AnimationNodeSequence());
	//seq->m_Name = "clip1";

	//AnimatorPtr animator(new Animator("anim_996004422"));
	//animator->m_SkeletonPath = "mesh/Cloth.skeleton.xml";
	//animator->SetChild<0>(seq);
	//animator->UpdateSequenceGroup();

	//ActorPtr actor(new Actor("actor_123349993", my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1), AABB(-10,10)));
	//actor->InsertComponent(mesh_cmp);
	//actor->InsertComponent(animator);
	//actor->UpdateWorld();
	//AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World), Actor::MinBlock, Actor::Threshold);
	//m_ActorList.push_back(actor);

	//my::OgreSkeletonAnimationPtr skel = theApp.LoadSkeleton(animator->m_SkeletonPath.c_str());
	//animator->AddDynamicBone(skel->GetBoneIndex("joint2"), skel->m_boneHierarchy, 0.1, 0.001, -10);

	////TerrainPtr terrain(new Terrain(my::NamedObject::MakeUniqueName("terrain").c_str(), 2, 2, 32, 1.0f));
	////terrain->AddMaterial(mtl);

	////ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("actor").c_str(), my::Vector3(-terrain->m_RowChunks*terrain->m_ChunkSize/2, 0, -terrain->m_ColChunks*terrain->m_ChunkSize/2), my::Quaternion::Identity(), my::Vector3(1, 1, 1), my::AABB(-1, 1)));
	////actor->InsertComponent(terrain);
	////actor->UpdateAABB();
	////actor->UpdateWorld();
	////AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World));
	////m_ActorList.push_back(actor);

	//m_selactors.clear();
	//m_selactors.push_back(actor.get());
	//m_selcmp = NULL;
	//m_selchunkid.SetPoint(0, 0);
	//m_selinstid = 0;
	//m_selctls.clear();
	//OnSelChanged();

	////for (int i = 0; i < 300000; i++)
	////{
	////	ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("actor").c_str(), my::Vector3(my::Random(-3000.0f, 3000.0f), 0, my::Random(-3000.0f, 3000.0f)), my::Quaternion::Identity(), my::Vector3(1, 1, 1), my::AABB(-1, 1)));
	////	actor->UpdateAABB();
	////	actor->UpdateWorld();
	////	AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World), 0.1f, 0.1f);
	////	m_ActorList.push_back(actor);
	////}
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
	m_wndOutliner.OnDestroyItemList();
	ClearFileContext();
	InitFileContext();
	OpenFileContext(strPathName);
	m_wndOutliner.OnInitItemList();
	OnUpdateFrameTitle(TRUE);

	OnSelChanged();

	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CEnvironmentWnd::CameraPropEventArgs arg(pView);
	m_EventCameraPropChanged(&arg);
}

void CMainFrame::OnFileClose()
{
	// TODO: Add your command handler code here
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
	CString strPathName(m_strPathName);
	if (strPathName.IsEmpty())
	{
		strPathName.LoadString(AFX_IDS_UNTITLED);
	}
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
		CRect rc;
		pView->GetClientRect(&rc);
		rc.SetRect(rc.Width() / 2, rc.Height() / 2, rc.Width() / 2 + 1, rc.Height() / 2 + 1);
		D3DLOCKED_RECT lrc = pView->m_OffscreenPositionRT->LockRect(&rc, D3DLOCK_READONLY);
		my::Vector3 posVS = (*(my::Vector4*)lrc.pBits).xyz;
		pView->m_OffscreenPositionRT->UnlockRect();
		if (posVS.z < 0 && posVS.z > -boost::dynamic_pointer_cast<my::ModelViewerCamera>(pView->m_Camera)->m_Distance)
		{
			Pos = posVS.transformCoord(pView->m_Camera->m_View.inverse());
		}
		else
		{
			Pos = boost::dynamic_pointer_cast<my::ModelViewerCamera>(pView->m_Camera)->m_LookAt;
		}
	}
	ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("actor").c_str(), Pos, my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1)));
	actor->UpdateWorld();
	AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World), Actor::MinBlock, Actor::Threshold);
	m_ActorList.push_back(actor);

	m_selactors.clear();
	m_selactors.push_back(actor.get());
	m_selcmp = NULL;
	m_selchunkid.SetPoint(0, 0);
	m_selinstid = 0;
	m_selctls.clear();
	OnSelChanged();
}

void CMainFrame::OnCreateController()
{
	//// TODO: Add your command handler code here
	ActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	ControllerPtr controller_cmp(new Controller(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_controller").c_str()).c_str(), 1.0f, 1.0f, 0.1f, 0.5f, 0.0f));
	(*actor_iter)->InsertComponent(controller_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateCreateController(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}

void CMainFrame::OnComponentMesh()
{
	// TODO: Add your command handler code here
	ActorList::iterator actor_iter = m_selactors.begin();
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

	my::AABB bound = (*actor_iter)->m_Cmps.empty() ? my::AABB::Invalid() : (*actor_iter)->m_aabb;

	const rapidxml::xml_node<char>* node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	DEFINE_XML_NODE_SIMPLE(submeshnames, mesh);
	DEFINE_XML_NODE_SIMPLE(submeshname, submeshnames);
	for (; node_submesh != NULL && node_submeshname != NULL; node_submesh = node_submesh->next_sibling(), node_submeshname = node_submeshname->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(name, submeshname);
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(index, submeshname);
		MeshComponentPtr mesh_cmp(new MeshComponent(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_" + attr_name->value()).c_str()).c_str()));
		mesh_cmp->m_MeshPath = path;
		mesh_cmp->m_MeshSubMeshId = index;
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
		mtl->ParseShaderParameters();
		mesh_cmp->SetMaterial(mtl);
		(*actor_iter)->InsertComponent(mesh_cmp);
	}

	rapidxml::xml_node<char>* node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	if (node_sharedgeometry)
	{
		DEFINE_XML_NODE_SIMPLE(vertexbuffer, sharedgeometry);
		DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
		for (; node_vertex != NULL; node_vertex = node_vertex->next_sibling())
		{
			DEFINE_XML_NODE_SIMPLE(position, vertex);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(x, position);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(y, position);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(z, position);
			bound.unionSelf(my::Vector3(x, y, z));
		}
	}
	else
	{
		DEFINE_XML_NODE(node_submesh, node_submeshes, submesh);
		for (; node_submesh != NULL; node_submesh = node_submesh->next_sibling())
		{
			DEFINE_XML_NODE_SIMPLE(geometry, submesh);
			DEFINE_XML_NODE_SIMPLE(vertexbuffer, geometry);
			DEFINE_XML_NODE_SIMPLE(vertex, vertexbuffer);
			for (; node_vertex != NULL; node_vertex = node_vertex->next_sibling())
			{
				DEFINE_XML_NODE_SIMPLE(position, vertex);
				DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(x, position);
				DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(y, position);
				DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(z, position);
				bound.unionSelf(my::Vector3(x, y, z));
			}
		}
	}
	(*actor_iter)->m_aabb = bound;
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

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
	ActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	std::string MeshPath = theApp.GetRelativePath(ts2ms((LPCTSTR)dlg.GetPathName()).c_str());
	if (MeshPath.empty())
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
	my::OgreMeshPtr mesh(new my::OgreMesh());
	mesh->CreateMeshFromOgreXml(node_root, true, D3DXMESH_MANAGED);

	DEFINE_XML_NODE_SIMPLE(mesh, root);
	DEFINE_XML_NODE_SIMPLE(submeshes, mesh);
	DEFINE_XML_NODE_SIMPLE(submesh, submeshes);
	for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
	{
		std::string ClothFabricPath = MeshPath + ".pxclothfabric_" + boost::lexical_cast<std::string>(submesh_i);
		ClothComponentPtr cloth_cmp(new ClothComponent(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_cloth").c_str()).c_str()));
		cloth_cmp->CreateClothFromMesh(ClothFabricPath.c_str(), mesh, submesh_i, GetGravity());
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
		mtl->ParseShaderParameters();
		cloth_cmp->SetMaterial(mtl);
		(*actor_iter)->InsertComponent(cloth_cmp);
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
	ActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CStaticEmitterDlg dlg(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_static_emit").c_str()).c_str());
	dlg.m_BoundingBox = (*actor_iter)->m_aabb;
	dlg.m_ChunkWidth = theApp.default_emitter_chunk_width / (*actor_iter)->m_Scale.x;
	dlg.m_ChunkLodScale = 1 / (*actor_iter)->m_Scale.x;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	(*actor_iter)->InsertComponent(dlg.m_emit_cmp);
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
	// TODO: Add your command handler code here
	ActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	SphericalEmitterPtr sphe_emit_cmp(new SphericalEmitter(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_sphe_emit").c_str()).c_str(), 4096, EmitterComponent::FaceTypeCamera, EmitterComponent::SpaceTypeLocal, EmitterComponent::VelocityTypeVel, EmitterComponent::PrimitiveTypeQuad));
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
	(*actor_iter)->InsertComponent(sphe_emit_cmp);
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
	ActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CTerrainDlg dlg(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_terrain").c_str()).c_str());
	dlg.m_ActorScale = (*actor_iter)->m_Scale;
	dlg.m_ChunkLodScale = 1 / (*actor_iter)->m_Scale.x;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	(*actor_iter)->InsertComponent(dlg.m_terrain);
	if (dlg.m_AlignToCenter)
	{
		const my::Vector3 center = dlg.m_terrain->Center() * dlg.m_ActorScale;
		(*actor_iter)->m_Position.x = -center.x;
		(*actor_iter)->m_Position.z = -center.z;
		(*actor_iter)->m_Scale = dlg.m_ActorScale;
		(*actor_iter)->m_CullingDistSq = center.magnitudeSq();
		(*actor_iter)->UpdateWorld();
		// TODO: update pxactor, ref Actor::SetPose
	}
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();
	if (dlg.m_AlignToCenter)
	{
		UpdatePivotTransform();
	}

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
	if (m_selactors.size() == 1 && m_selactors.front()->m_Cmps.size() > 1)
	{
		CDeleteCmpsDlg dlg;
		if (dlg.DoModal() != IDOK)
		{
			return;
		}

		if (m_selactors.front()->m_Cmps.size() > 0)
		{
			if (m_selactors.front()->m_Cmps.end() == boost::find_if(m_selactors.front()->m_Cmps,
				boost::bind(std::equal_to<Component*>(), m_selcmp, boost::bind(&ComponentPtr::get, boost::placeholders::_1))))
			{
				m_selcmp = NULL;
				m_selchunkid.SetPoint(0, 0);
				m_selinstid = 0;
			}
			OnSelChanged();
			return;
		}
	}

	typedef std::list<my::Control*> ControlLink;
	ControlLink delctls(m_selctls.begin(), m_selctls.end());
	ControlLink::iterator first_iter = boost::find_if(delctls,
		boost::bind(std::not_equal_to<DWORD>(), my::Control::ControlTypeScrollBar, boost::bind(&my::Control::GetControlType, boost::placeholders::_1)));

	for (; first_iter != delctls.end(); first_iter = boost::find_if(delctls,
		boost::bind(std::not_equal_to<DWORD>(), my::Control::ControlTypeScrollBar, boost::bind(&my::Control::GetControlType, boost::placeholders::_1))))
	{
		ControlLink::iterator other_iter = delctls.begin();
		for (; other_iter != delctls.end(); )
		{
			if (other_iter != first_iter && (*first_iter)->ContainsControl(*other_iter))
			{
				other_iter = delctls.erase(other_iter);
			}
			else
			{
				other_iter++;
			}
		}

		if ((*first_iter)->GetControlType() == my::Control::ControlTypeDialog)
		{
			my::DialogPtr dlg = boost::dynamic_pointer_cast<my::Dialog>((*first_iter)->shared_from_this());
			ASSERT(dlg);

			RemoveDlg(dlg.get());

			DialogPtrList::iterator dlg_iter = std::find(m_DialogList.begin(), m_DialogList.end(), dlg);
			if (dlg_iter != m_DialogList.end())
			{
				m_DialogList.erase(dlg_iter);
			}

			delctls.erase(first_iter);
		}
		else
		{
			my::Control * parent = (*first_iter)->m_Parent;

			parent->RemoveControl((*first_iter)->GetSiblingId());

			if (delctls.size() == 1)
			{
				m_selctls = boost::assign::list_of(parent);
				OnSelChanged();
				return;
			}

			delctls.erase(first_iter);
		}
	}
	m_selctls.assign(delctls.begin(), delctls.end());

	ActorList::iterator actor_iter = m_selactors.begin();
	for (; actor_iter != m_selactors.end(); actor_iter = m_selactors.begin())
	{
		ActorPtr actor = (*actor_iter)->shared_from_this();

		RemoveEntity(actor.get());

		ActorPtrList::iterator actor_iter = std::find(m_ActorList.begin(), m_ActorList.end(), actor);
		if (actor_iter != m_ActorList.end())
		{
			m_ActorList.erase(actor_iter);
		}
	}

	m_selcmp = NULL;
	m_selchunkid.SetPoint(0, 0);
	m_selinstid = 0;
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
	theApp.ClearShaderCache();
	my::EventArg arg;
	m_EventAttributeChanged(&arg);
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
	if (GetSelComponent<Terrain>())
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
	if (GetSelComponent<Terrain>())
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
	if (GetSelComponent<Terrain>() && GetSelComponent<StaticEmitter>())
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(m_PaintType == PaintTypeEmitterInstance);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CMainFrame::OnToolsOffmeshconnections()
{
	// TODO: Add your command handler code here
	if (m_PaintType == PaintTypeOffmeshConnections)
	{
		m_PaintType = PaintTypeNone;
	}
	else
	{
		m_PaintType = PaintTypeOffmeshConnections;
		m_hitPosSet = false;
	}
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdateToolsOffmeshconnections(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_PaintType == PaintTypeOffmeshConnections);
}

void CMainFrame::OnComponentAnimator()
{
	// TODO: Add your command handler code here
	ActorList::iterator actor_iter = m_selactors.begin();
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

	AnimatorPtr animator(new Animator(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_animator").c_str()).c_str()));
	animator->m_SkeletonPath = path;
	(*actor_iter)->InsertComponent(animator);

	const rapidxml::xml_node<char>* node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(skeleton, root);
	rapidxml::xml_node<char>* node_animations = node_skeleton->first_node("animations");
	if (node_animations != NULL)
	{
		DEFINE_XML_NODE_SIMPLE(animation, animations);
		DEFINE_XML_ATTRIBUTE_SIMPLE(name, animation);
		animator->SetChild(0, AnimationNodeSequencePtr(new AnimationNodeSequence(attr_name->value())));
		animator->ReloadSequenceGroup();
	}
	else
	{
		animator->SetChild(0, AnimationNodeSequencePtr(new AnimationNodeSequence("unknown")));
	}

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}

void CMainFrame::OnUpdateComponentAnimator(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}


void CMainFrame::OnCreateNavigation()
{
	// TODO: Add your command handler code here
	ActorList::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CNavigationDlg dlg;
	dlg.m_bindingBox = *(*actor_iter)->m_OctAabb;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	NavigationPtr navi_cmp(new Navigation(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_navigation").c_str()).c_str()));
	navi_cmp->m_navMesh = dlg.m_navMesh;
	navi_cmp->BuildQueryAndChunks(2048);
	(*actor_iter)->InsertComponent(navi_cmp);
	UpdateSelBox();

	my::EventArg arg;
	m_EventAttributeChanged(&arg);
}


void CMainFrame::OnUpdateCreateNavigation(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}


void CMainFrame::OnCreateDialog()
{
	// TODO: Add your command handler code here
	my::DialogSkinPtr skin(new my::DialogSkin());
	skin->m_Color = theApp.default_dialog_color;
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_dialog_img;
	skin->m_Image->m_Rect = theApp.default_dialog_img_rect;
	skin->m_Image->m_Border = theApp.default_dialog_img_border;

	my::DialogPtr dlg(new my::Dialog(my::NamedObject::MakeUniqueName("dialog").c_str()));
	dlg->m_Skin = skin;
	if (!m_selctls.empty() && m_selctls.front()->GetControlType() == my::Control::ControlTypeDialog)
	{
		dlg->m_x.scale = m_selctls.front()->m_x.scale;
		dlg->m_x.offset = m_selctls.front()->m_x.offset + 10;
		dlg->m_y.scale = m_selctls.front()->m_y.scale;
		dlg->m_y.offset = m_selctls.front()->m_y.offset + 10;
	}

	InsertDlg(dlg.get());
	m_DialogList.push_back(dlg);

	m_selactors.clear();
	m_selcmp = NULL;
	m_selchunkid.SetPoint(0, 0);
	m_selinstid = 0;
	m_selctls = boost::assign::list_of(dlg.get());
	OnSelChanged();
}


void CMainFrame::OnControlStatic()
{
	// TODO: Add your command handler code here
	_ASSERT(!m_selctls.empty());

	my::StaticSkinPtr skin(new my::StaticSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_static_text_color;
	skin->m_TextAlign = theApp.default_static_text_align;

	my::StaticPtr static_ctl(new my::Static(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_static").c_str()).c_str()));
	static_ctl->m_Skin = skin;
	std::string text(static_ctl->GetName());
	static_ctl->m_Text = ms2ws(&text[text.find_last_of("_") + 1]);
	static_ctl->m_x.offset = 10;
	static_ctl->m_y.offset = 10;

	m_selctls.front()->InsertControl(static_ctl);
	m_selctls = boost::assign::list_of(static_ctl.get());
	OnSelChanged();
}


void CMainFrame::OnUpdateControlStatic(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


void CMainFrame::OnControlProgressbar()
{
	// TODO: Add your command handler code here
	_ASSERT(!m_selctls.empty());

	my::ProgressBarSkinPtr skin(new my::ProgressBarSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_progressbar_img;
	skin->m_Image->m_Rect = theApp.default_progressbar_img_rect;
	skin->m_Image->m_Border = theApp.default_progressbar_img_border;
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_progressbar_text_color;
	skin->m_TextAlign = theApp.default_progressbar_text_align;
	skin->m_ForegroundImage.reset(new my::ControlImage());
	skin->m_ForegroundImage->m_TexturePath = theApp.default_progressbar_foregroundimg;
	skin->m_ForegroundImage->m_Rect = theApp.default_progressbar_foregroundimg_rect;
	skin->m_ForegroundImage->m_Border = theApp.default_progressbar_foregroundimg_border;

	my::ProgressBarPtr pgs(new my::ProgressBar(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_progressbar").c_str()).c_str()));
	pgs->m_Skin = skin;
	pgs->m_BlendProgress = pgs->m_Progress = 0.6f;
	pgs->m_x.offset = 10;
	pgs->m_y.offset = 10;

	m_selctls.front()->InsertControl(pgs);
	m_selctls.front() = pgs.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlProgressbar(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


void CMainFrame::OnControlButton()
{
	// TODO: Add your command handler code here
	my::ButtonSkinPtr skin(new my::ButtonSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_button_img;
	skin->m_Image->m_Rect = theApp.default_button_img_rect;
	skin->m_Image->m_Border = theApp.default_button_img_border;
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_button_text_color;
	skin->m_TextAlign = theApp.default_button_text_align;
	skin->m_PressedOffset = theApp.default_button_pressed_offset;
	skin->m_DisabledImage.reset(new my::ControlImage());
	skin->m_DisabledImage->m_TexturePath = theApp.default_button_disabledimg;
	skin->m_DisabledImage->m_Rect = theApp.default_button_disabledimg_rect;
	skin->m_DisabledImage->m_Border = theApp.default_button_disabledimg_border;
	skin->m_PressedImage.reset(new my::ControlImage());
	skin->m_PressedImage->m_TexturePath = theApp.default_button_pressedimg;
	skin->m_PressedImage->m_Rect = theApp.default_button_pressedimg_rect;
	skin->m_PressedImage->m_Border = theApp.default_button_pressedimg_border;
	skin->m_MouseOverImage.reset(new my::ControlImage());
	skin->m_MouseOverImage->m_TexturePath = theApp.default_button_mouseoverimg;
	skin->m_MouseOverImage->m_Rect = theApp.default_button_mouseoverimg_rect;
	skin->m_MouseOverImage->m_Border = theApp.default_button_mouseoverimg_border;

	my::ButtonPtr btn(new my::Button(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_button").c_str()).c_str()));
	btn->m_Skin = skin;
	std::string text(btn->GetName());
	btn->m_Text = ms2ws(&text[text.find_last_of("_") + 1]);
	btn->m_x.offset = 10;
	btn->m_y.offset = 10;

	m_selctls.front()->InsertControl(btn);
	m_selctls.front() = btn.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlButton(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


void CMainFrame::OnControlImeeditbox()
{
	// TODO: Add your command handler code here
	my::EditBoxSkinPtr skin(new my::EditBoxSkin());
	skin->m_Color = theApp.default_editbox_color;
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_editbox_text_color;
	skin->m_TextAlign = theApp.default_editbox_text_align;
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_editbox_img;
	skin->m_Image->m_Rect = theApp.default_editbox_img_rect;
	skin->m_Image->m_Border = theApp.default_editbox_img_border;
	skin->m_DisabledImage.reset(new my::ControlImage());
	skin->m_DisabledImage->m_TexturePath = theApp.default_editbox_disabledimg;
	skin->m_DisabledImage->m_Rect = theApp.default_editbox_disabledimg_rect;
	skin->m_DisabledImage->m_Border = theApp.default_editbox_disabledimg_border;
	skin->m_FocusedImage.reset(new my::ControlImage());
	skin->m_FocusedImage->m_TexturePath = theApp.default_editbox_focusedimg;
	skin->m_FocusedImage->m_Rect = theApp.default_editbox_focusedimg_rect;
	skin->m_FocusedImage->m_Border = theApp.default_editbox_focusedimg_border;
	skin->m_SelBkColor = theApp.default_editbox_sel_bk_color;
	skin->m_CaretColor = theApp.default_editbox_caret_color;
	skin->m_CaretImage.reset(new my::ControlImage());
	skin->m_CaretImage->m_TexturePath = theApp.default_editbox_caretimg;
	skin->m_CaretImage->m_Rect = theApp.default_editbox_caretimg_rect;
	skin->m_CaretImage->m_Border = theApp.default_editbox_caretimg_border;

	my::ImeEditBoxPtr edit(new my::ImeEditBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_imeeditbox").c_str()).c_str()));
	edit->m_Skin = skin;
	edit->m_x.offset = 10;
	edit->m_y.offset = 10;

	m_selctls.front()->InsertControl(edit);
	m_selctls.front() = edit.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlImeeditbox(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


void CMainFrame::OnControlCheckbox()
{
	// TODO: Add your command handler code here
	my::ButtonSkinPtr skin(new my::ButtonSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_checkbox_img;
	skin->m_Image->m_Rect = theApp.default_checkbox_img_rect;
	skin->m_Image->m_Border = theApp.default_checkbox_img_border;
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_checkbox_text_color;
	skin->m_TextAlign = theApp.default_checkbox_text_align;
	skin->m_PressedOffset = theApp.default_checkbox_pressed_offset;
	skin->m_DisabledImage.reset(new my::ControlImage());
	skin->m_DisabledImage->m_TexturePath = theApp.default_checkbox_disabledimg;
	skin->m_DisabledImage->m_Rect = theApp.default_checkbox_disabledimg_rect;
	skin->m_DisabledImage->m_Border = theApp.default_checkbox_disabledimg_border;
	skin->m_PressedImage.reset(new my::ControlImage());
	skin->m_PressedImage->m_TexturePath = theApp.default_checkbox_pressedimg;
	skin->m_PressedImage->m_Rect = theApp.default_checkbox_pressedimg_rect;
	skin->m_PressedImage->m_Border = theApp.default_checkbox_pressedimg_border;
	skin->m_MouseOverImage.reset(new my::ControlImage());
	skin->m_MouseOverImage->m_TexturePath = theApp.default_checkbox_mouseoverimg;
	skin->m_MouseOverImage->m_Rect = theApp.default_checkbox_mouseoverimg_rect;
	skin->m_MouseOverImage->m_Border = theApp.default_checkbox_mouseoverimg_border;

	my::CheckBoxPtr checkbox(new my::CheckBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_checkbox").c_str()).c_str()));
	checkbox->m_Skin = skin;
	std::string text(checkbox->GetName());
	checkbox->m_Text = ms2ws(&text[text.find_last_of("_") + 1]);
	checkbox->m_x.offset = 10;
	checkbox->m_y.offset = 10;

	m_selctls.front()->InsertControl(checkbox);
	m_selctls.front() = checkbox.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlCheckbox(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


void CMainFrame::OnControlCombobox()
{
	// TODO: Add your command handler code here
	my::ComboBoxSkinPtr skin(new my::ComboBoxSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_combobox_img;
	skin->m_Image->m_Rect = theApp.default_combobox_img_rect;
	skin->m_Image->m_Border = theApp.default_combobox_img_border;
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_combobox_text_color;
	skin->m_TextAlign = theApp.default_combobox_text_align;
	skin->m_PressedOffset = theApp.default_combobox_pressed_offset;
	skin->m_DisabledImage.reset(new my::ControlImage());
	skin->m_DisabledImage->m_TexturePath = theApp.default_combobox_disabledimg;
	skin->m_DisabledImage->m_Rect = theApp.default_combobox_disabledimg_rect;
	skin->m_DisabledImage->m_Border = theApp.default_combobox_disabledimg_border;
	skin->m_PressedImage.reset(new my::ControlImage());
	skin->m_PressedImage->m_TexturePath = theApp.default_combobox_pressedimg;
	skin->m_PressedImage->m_Rect = theApp.default_combobox_pressedimg_rect;
	skin->m_PressedImage->m_Border = theApp.default_combobox_pressedimg_border;
	skin->m_MouseOverImage.reset(new my::ControlImage());
	skin->m_MouseOverImage->m_TexturePath = theApp.default_combobox_mouseoverimg;
	skin->m_MouseOverImage->m_Rect = theApp.default_combobox_mouseoverimg_rect;
	skin->m_MouseOverImage->m_Border = theApp.default_combobox_mouseoverimg_border;
	skin->m_DropdownImage.reset(new my::ControlImage());
	skin->m_DropdownImage->m_TexturePath = theApp.default_combobox_dropdownimg;
	skin->m_DropdownImage->m_Rect = theApp.default_combobox_dropdownimg_rect;
	skin->m_DropdownImage->m_Border = theApp.default_combobox_dropdownimg_border;
	skin->m_DropdownItemTextColor = theApp.default_combobox_dropdownitem_text_color;
	skin->m_DropdownItemTextAlign = theApp.default_combobox_dropdownitem_text_align;
	skin->m_DropdownItemMouseOverImage.reset(new my::ControlImage());
	skin->m_DropdownItemMouseOverImage->m_TexturePath = theApp.default_combobox_dropdownitem_mouseoverimg;
	skin->m_DropdownItemMouseOverImage->m_Rect = theApp.default_combobox_dropdownitem_mouseoverimg_rect;
	skin->m_DropdownItemMouseOverImage->m_Border = theApp.default_combobox_dropdownitem_mouseoverimg_border;

	my::ScrollBarSkinPtr scroll_skin(new my::ScrollBarSkin());
	scroll_skin->m_Image.reset(new my::ControlImage());
	scroll_skin->m_Image->m_TexturePath = theApp.default_combobox_scrollbar_img;
	scroll_skin->m_Image->m_Rect = theApp.default_combobox_scrollbar_img_rect;
	scroll_skin->m_Image->m_Border = theApp.default_combobox_scrollbar_img_border;
	scroll_skin->m_UpBtnNormalImage.reset(new my::ControlImage());
	scroll_skin->m_UpBtnNormalImage->m_TexturePath = theApp.default_combobox_scrollbarupbtn_normalimg;
	scroll_skin->m_UpBtnNormalImage->m_Rect = theApp.default_combobox_scrollbarupbtn_normalimg_rect;
	scroll_skin->m_UpBtnNormalImage->m_Border = theApp.default_combobox_scrollbarupbtn_normalimg_border;
	scroll_skin->m_UpBtnDisabledImage.reset(new my::ControlImage());
	scroll_skin->m_UpBtnDisabledImage->m_TexturePath = theApp.default_combobox_scrollbarupbtn_disabledimg;
	scroll_skin->m_UpBtnDisabledImage->m_Rect = theApp.default_combobox_scrollbarupbtn_disabledimg_rect;
	scroll_skin->m_UpBtnDisabledImage->m_Border = theApp.default_combobox_scrollbarupbtn_disabledimg_border;
	scroll_skin->m_DownBtnNormalImage.reset(new my::ControlImage());
	scroll_skin->m_DownBtnNormalImage->m_TexturePath = theApp.default_combobox_scrollbardownbtn_normalimg;
	scroll_skin->m_DownBtnNormalImage->m_Rect = theApp.default_combobox_scrollbardownbtn_normalimg_rect;
	scroll_skin->m_DownBtnNormalImage->m_Border = theApp.default_combobox_scrollbardownbtn_normalimg_border;
	scroll_skin->m_DownBtnDisabledImage.reset(new my::ControlImage());
	scroll_skin->m_DownBtnDisabledImage->m_TexturePath = theApp.default_combobox_scrollbardownbtn_disabledimg;
	scroll_skin->m_DownBtnDisabledImage->m_Rect = theApp.default_combobox_scrollbardownbtn_disabledimg_rect;
	scroll_skin->m_DownBtnDisabledImage->m_Border = theApp.default_combobox_scrollbardownbtn_disabledimg_border;
	scroll_skin->m_ThumbBtnNormalImage.reset(new my::ControlImage());
	scroll_skin->m_ThumbBtnNormalImage->m_TexturePath = theApp.default_combobox_scrollbarthumbbtn_normalimg;
	scroll_skin->m_ThumbBtnNormalImage->m_Rect = theApp.default_combobox_scrollbarthumbbtn_normalimg_rect;
	scroll_skin->m_ThumbBtnNormalImage->m_Border = theApp.default_combobox_scrollbarthumbbtn_normalimg_border;

	my::ComboBoxPtr combobox(new my::ComboBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_combobox").c_str()).c_str()));
	combobox->m_Skin = skin;
	combobox->m_ScrollBar->m_Skin = scroll_skin;
	std::string text(combobox->GetName());
	combobox->m_Text = ms2ws(&text[text.find_last_of("_") + 1]);
	combobox->m_x.offset = 10;
	combobox->m_y.offset = 10;
	for (int i = 0; i < 3; i++)
	{
		combobox->AddItem(str_printf(L"item%d", i));
	}
	combobox->OnLayout();

	m_selctls.front()->InsertControl(combobox);
	m_selctls.front() = combobox.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlCombobox(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


void CMainFrame::OnControlListbox()
{
	// TODO: Add your command handler code here
	my::ListBoxSkinPtr skin(new my::ListBoxSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_listbox_img;
	skin->m_Image->m_Rect = theApp.default_listbox_img_rect;
	skin->m_Image->m_Border = theApp.default_listbox_img_border;

	my::ScrollBarSkinPtr scroll_skin(new my::ScrollBarSkin());
	scroll_skin->m_Image.reset(new my::ControlImage());
	scroll_skin->m_Image->m_TexturePath = theApp.default_listbox_scrollbar_img;
	scroll_skin->m_Image->m_Rect = theApp.default_listbox_scrollbar_img_rect;
	scroll_skin->m_Image->m_Border = theApp.default_listbox_scrollbar_img_border;
	scroll_skin->m_UpBtnNormalImage.reset(new my::ControlImage());
	scroll_skin->m_UpBtnNormalImage->m_TexturePath = theApp.default_listbox_scrollbarupbtn_normalimg;
	scroll_skin->m_UpBtnNormalImage->m_Rect = theApp.default_listbox_scrollbarupbtn_normalimg_rect;
	scroll_skin->m_UpBtnNormalImage->m_Border = theApp.default_listbox_scrollbarupbtn_normalimg_border;
	scroll_skin->m_UpBtnDisabledImage.reset(new my::ControlImage());
	scroll_skin->m_UpBtnDisabledImage->m_TexturePath = theApp.default_listbox_scrollbarupbtn_disabledimg;
	scroll_skin->m_UpBtnDisabledImage->m_Rect = theApp.default_listbox_scrollbarupbtn_disabledimg_rect;
	scroll_skin->m_UpBtnDisabledImage->m_Border = theApp.default_listbox_scrollbarupbtn_disabledimg_border;
	scroll_skin->m_DownBtnNormalImage.reset(new my::ControlImage());
	scroll_skin->m_DownBtnNormalImage->m_TexturePath = theApp.default_listbox_scrollbardownbtn_normalimg;
	scroll_skin->m_DownBtnNormalImage->m_Rect = theApp.default_listbox_scrollbardownbtn_normalimg_rect;
	scroll_skin->m_DownBtnNormalImage->m_Border = theApp.default_listbox_scrollbardownbtn_normalimg_border;
	scroll_skin->m_DownBtnDisabledImage.reset(new my::ControlImage());
	scroll_skin->m_DownBtnDisabledImage->m_TexturePath = theApp.default_listbox_scrollbardownbtn_disabledimg;
	scroll_skin->m_DownBtnDisabledImage->m_Rect = theApp.default_listbox_scrollbardownbtn_disabledimg_rect;
	scroll_skin->m_DownBtnDisabledImage->m_Border = theApp.default_listbox_scrollbardownbtn_disabledimg_border;
	scroll_skin->m_ThumbBtnNormalImage.reset(new my::ControlImage());
	scroll_skin->m_ThumbBtnNormalImage->m_TexturePath = theApp.default_listbox_scrollbarthumbbtn_normalimg;
	scroll_skin->m_ThumbBtnNormalImage->m_Rect = theApp.default_listbox_scrollbarthumbbtn_normalimg_rect;
	scroll_skin->m_ThumbBtnNormalImage->m_Border = theApp.default_listbox_scrollbarthumbbtn_normalimg_border;

	my::ListBoxPtr listBox(new my::ListBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_listbox").c_str()).c_str()));
	listBox->m_Skin = skin;
	listBox->m_ScrollBar->m_Skin = scroll_skin;
	listBox->m_x.offset = 10;
	listBox->m_y.offset = 10;
	for (int i = 0; i < 3; i++)
	{
		my::ButtonSkinPtr skin(new my::ButtonSkin());
		skin->m_Image.reset(new my::ControlImage());
		skin->m_Image->m_TexturePath = theApp.default_button_img;
		skin->m_Image->m_Rect = theApp.default_button_img_rect;
		skin->m_Image->m_Border = theApp.default_button_img_border;
		skin->m_FontPath = theApp.default_font_path;
		skin->m_FontHeight = theApp.default_font_height;
		skin->m_FontFaceIndex = theApp.default_font_face_index;
		skin->m_TextColor = theApp.default_button_text_color;
		skin->m_TextAlign = theApp.default_button_text_align;
		skin->m_PressedOffset = theApp.default_button_pressed_offset;
		skin->m_DisabledImage.reset(new my::ControlImage());
		skin->m_DisabledImage->m_TexturePath = theApp.default_button_disabledimg;
		skin->m_DisabledImage->m_Rect = theApp.default_button_disabledimg_rect;
		skin->m_DisabledImage->m_Border = theApp.default_button_disabledimg_border;
		skin->m_PressedImage.reset(new my::ControlImage());
		skin->m_PressedImage->m_TexturePath = theApp.default_button_pressedimg;
		skin->m_PressedImage->m_Rect = theApp.default_button_pressedimg_rect;
		skin->m_PressedImage->m_Border = theApp.default_button_pressedimg_border;
		skin->m_MouseOverImage.reset(new my::ControlImage());
		skin->m_MouseOverImage->m_TexturePath = theApp.default_button_mouseoverimg;
		skin->m_MouseOverImage->m_Rect = theApp.default_button_mouseoverimg_rect;
		skin->m_MouseOverImage->m_Border = theApp.default_button_mouseoverimg_border;

		my::ButtonPtr btn(new my::Button(my::NamedObject::MakeUniqueName((std::string(listBox->GetName()) + "_item").c_str()).c_str()));
		btn->m_Skin = skin;
		std::string text(btn->GetName());
		btn->m_Text = str_printf(L"item%d", i);

		listBox->InsertControl(listBox->GetChildNum(), btn);
	}
	listBox->OnLayout();

	m_selctls.front()->InsertControl(listBox);
	m_selctls.front() = listBox.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlListbox(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selctls.empty());
}


BOOL CMainFrame::OnShowPopupMenu(CMFCPopupMenu* pMenuPopup)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMenuPopup)
	{
		CMFCPopupMenuBar* pMenuBar = pMenuPopup->GetMenuBar();
		ASSERT_VALID(pMenuBar);
		for (int i = 0; i < pMenuBar->GetCount(); i++)
		{
			if (pMenuBar->GetItemID(i) == ID_TOOLS_SCRIPT1)
			{
				const int first_script_index = i;
				WIN32_FIND_DATAA ffd;
				HANDLE hFind = INVALID_HANDLE_VALUE;
				hFind = FindFirstFileA(theApp.default_tool_scrpit_pattern.c_str(), &ffd);
				if (hFind == INVALID_HANDLE_VALUE)
				{
					break;
				}
				m_ToolScriptDir = theApp.default_tool_scrpit_pattern.c_str();
				// ! c++11 contains a null-terminated, https://cplusplus.com/reference/string/string/data/
				PathRemoveFileSpecA(const_cast<char*>(m_ToolScriptDir.data()));
				m_ToolScriptDir.resize(strlen(m_ToolScriptDir.c_str()));
				do
				{
					if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						m_ToolScripts[i - first_script_index] = ffd.cFileName;
						CString strText;
						const TCHAR* pref[] = { _T("&1 "), _T("&2 "), _T("&3 "), _T("&4 "), _T("&5 "), _T("&6 "), _T("&7 "), _T("&8 "), _T("&9 "), _T("1&0 ") };
						strText.Format(_T("%s%s"), i - first_script_index < _countof(pref) ? pref[i - first_script_index] : _T(""), ms2ts(m_ToolScripts[i - first_script_index]).c_str());
						if (i < pMenuBar->GetCount() && pMenuBar->GetButtonStyle(i) != TBBS_SEPARATOR && pMenuBar->GetItemID(i) < ID_TOOLS_SCRIPT_LAST)
						{
							pMenuBar->SetButtonText(i, strText);
							i++;
						}
						else
						{
							pMenuBar->InsertButton(CMFCToolBarMenuButton(ID_TOOLS_SCRIPT1 + i - first_script_index, NULL, -1, strText), i);
							i++;
						}
					}
				}
				while (FindNextFileA(hFind, &ffd) && i - first_script_index < ID_TOOLS_SCRIPT_LAST - ID_TOOLS_SCRIPT1);
				FindClose(hFind);
				break;
			}
		}
	}

	return __super::OnShowPopupMenu(pMenuPopup);
}


void CMainFrame::OnToolsScript1(UINT id)
{
	// TODO: Add your command handler code here
	if (luaL_loadfile(m_State, m_ToolScripts[id - ID_TOOLS_SCRIPT1].c_str()) || docall(0, 1))
	{
		if (!lua_isnil(m_State, -1))
		{
			std::string msg = lua_tostring(m_State, -1);
			if (msg.empty())
				msg = "error object is not a string";
			lua_pop(m_State, 1);

			theApp.m_EventLog(msg.c_str());
		}
	}
}


void CMainFrame::OnUpdateToolsScript1(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_ToolScripts[pCmdUI->m_nID - ID_TOOLS_SCRIPT1].empty());
}


void CMainFrame::OnToolsSnapshot()
{
	// TODO: Add your command handler code here
	CSnapshotDlg dlg;
	dlg.DoModal();
}


void CMainFrame::OnUpdateIndicatorCoord(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CPoint pt;
	GetCursorPos(&pt);
	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, WindowFromPoint(pt));
	if (pView)
	{
		pView->ScreenToClient(&pt);
		CRect rc(pt, CSize(1,1));
		D3DLOCKED_RECT lrc = pView->m_OffscreenPositionRT->LockRect(&rc, D3DLOCK_READONLY);
		my::Vector3 pos = (*(my::Vector4*)lrc.pBits).xyz;
		pView->m_OffscreenPositionRT->UnlockRect();

		CString text;
		if (pos != my::Vector3::zero)
		{
			pos = pos.transformCoord(pView->m_Camera->m_View.inverse());
			text.Format(_T("%f, %f, %f"), pos.x, pos.y, pos.z);
		}
		else
		{
			text = _T("Infinite pos");
		}
		pCmdUI->SetText(text);
	}
}


void CMainFrame::OnToolsPlaying()
{
	// TODO: Add your command handler code here
	if (!m_Player->m_Node)
	{
		CChildView* pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
		ASSERT_VALID(pView);
		pView->SetFocus();
		m_Player->SetPose(boost::dynamic_pointer_cast<my::ModelViewerCamera>(pView->m_Camera)->m_LookAt);
		AddEntity(m_Player.get(), m_Player->m_aabb.transform(m_Player->m_World), Actor::MinBlock, Actor::Threshold);

		Controller* controller = m_Player->GetFirstComponent<Controller>();
		ASSERT(controller);
		physx::PxRaycastBuffer hit;
		physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
			physx::PxFilterData(0x01, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC /*| physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eANY_HIT*/);
		if (m_PxScene->raycast((physx::PxVec3&)(m_Player->m_Position - controller->GetFootOffset() + my::Vector3(0, theApp.default_player_collision_height, 0)),
			physx::PxVec3(0, -1, 0), theApp.default_player_collision_height, hit, physx::PxHitFlag::eDEFAULT, filterData, NULL, NULL))
		{
			boost::dynamic_pointer_cast<ActionTrackPose>(ActionTbl::getSingleton().Climb->m_TrackList[0])->m_ParamPose =
				my::Bone((my::Vector3&)hit.block.position + controller->GetFootOffset(), m_Player->m_Rotation);
			m_Player->PlayAction(ActionTbl::getSingleton().Climb.get(), 0.5f);
		}
	}
	else
	{
		RemoveEntity(m_Player.get());
	}
}


void CMainFrame::OnUpdateToolsPlaying(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_Player->m_Node != NULL);
}


void CMainFrame::OnToolsSnapToGrid()
{
	// TODO: Add your command handler code here
	theApp.default_tool_snap_to_grid = !theApp.default_tool_snap_to_grid;
}


void CMainFrame::OnUpdateToolsSnapToGrid(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck((GetKeyState('X') & 0x8000) ? !theApp.default_tool_snap_to_grid : theApp.default_tool_snap_to_grid);
}


void CMainFrame::OnToolsTerraintoobj()
{
	// TODO: Add your command handler code here
	CTerrainToObjDlg dlg;
	if (m_selcmp && m_selcmp->GetComponentType() == Component::ComponentTypeTerrain)
	{
		dlg.m_terrain = dynamic_cast<Terrain*>(m_selcmp);
		dlg.m_i1 = (dlg.m_i0 = m_selchunkid.x * dlg.m_terrain->m_ChunkSize) + dlg.m_terrain->m_ChunkSize;
		dlg.m_j1 = (dlg.m_j0 = m_selchunkid.y * dlg.m_terrain->m_ChunkSize) + dlg.m_terrain->m_ChunkSize;
	}
	else if (dlg.m_terrain = m_selactors.front()->GetFirstComponent<Terrain>())
	{
		dlg.m_i0 = 0;
		dlg.m_j0 = 0;
		dlg.m_i1 = dlg.m_terrain->m_RowChunks * dlg.m_terrain->m_ChunkSize;
		dlg.m_j1 = dlg.m_terrain->m_ColChunks * dlg.m_terrain->m_ChunkSize;
	}

	if (dlg.m_terrain)
	{
		dlg.DoModal();
	}
}


void CMainFrame::OnUpdateToolsTerraintoobj(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty() && m_selactors.front()->GetFirstComponent<Terrain>());
}


void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: Add your specialized code here and/or call the base class
	if (bAddToTitle)
	{
		if (m_strPathName.IsEmpty())
		{
			CString strTitle;
			strTitle.LoadString(AFX_IDS_UNTITLED);
			UpdateFrameTitleForDocument(strTitle);
		}
		else
		{
			UpdateFrameTitleForDocument(m_strPathName);
		}
	}
	else
	{
		UpdateFrameTitleForDocument(NULL);
	}

	//__super::OnUpdateFrameTitle(bAddToTitle);
}

// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "MainApp.h"

#include "MainFrm.h"
#include "ChildView.h"
#include "TerrainDlg.h"
#include "StaticEmitterDlg.h"
#include "Material.h"
#include "Controller.h"
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/string.hpp>
#include <fstream>
#include "NavigationSerialization.h"
#include "NavigationDlg.h"
#include "SimplifyMeshDlg.h"
#include "Animator.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/out_value_policy.hpp>
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
#include "PlayerBehavior.h"
#include "Steering.h"
#include "ActionTrack.h"
#include "rapidxml.hpp"
#include <lualib.h>
#include "TerrainToObjDlg.h"
//#include "../FastNoiseLite/Cpp/FastNoiseLite.h"
//#include "clipper.hpp"
#include "../HdriToCubemap/HdriToCubemap/HdriToCubemap.hpp"

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
		std::string dir = theApp.default_script_pattern.c_str();
		// ! c++11 contains a null-terminated, https://cplusplus.com/reference/string/string/data/
		PathRemoveFileSpecA(const_cast<char*>(dir.data()));
		dir.resize(strlen(dir.c_str()));
		lf.stream = theApp.OpenIStream((dir + "\\" + filename).c_str());
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

static boost::iterator_range<shared_actor_list_iter> cmainframe_query_entity(const CMainFrame* self, const my::AABB& aabb)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		boost::shared_ptr<CMainFrame::ActorList> acts;

		Callback(void)
			: acts(new CMainFrame::ActorList())
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			acts->push_back(dynamic_cast<Actor*>(oct_entity));
			return true;
		}
	};

	Callback cb;
	self->QueryEntity(aabb, &cb);
	return boost::make_iterator_range(shared_actor_list_iter(cb.acts->begin(), cb.acts), shared_actor_list_iter(cb.acts->end(), cb.acts));
}

static boost::iterator_range<shared_actor_list_iter> cmainframe_get_all_acts(const CMainFrame* self)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		boost::shared_ptr<CMainFrame::ActorList> acts;

		Callback(void)
			: acts(new CMainFrame::ActorList())
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			acts->push_back(dynamic_cast<Actor*>(oct_entity));
			return true;
		}
	};

	Callback cb;
	self->QueryAllEntity(&cb);
	return boost::make_iterator_range(shared_actor_list_iter(cb.acts->begin(), cb.acts), shared_actor_list_iter(cb.acts->end(), cb.acts));
}

static void cmainapp_load_dictionary(CMainApp* self, const std::wstring& path)
{
	self->m_Dicts.LoadFromFile(ws2ms(path.c_str()).c_str());
}
//
//static void spawn_terrain_pos_2_emitter(TerrainStream* tstr, StaticEmitterStream* estr, float terrain_local_x, float terrain_local_z, const my::Matrix4 & trans)
//{
//	my::Vector3 pos(terrain_local_x, tstr->RayTest2D(terrain_local_x, terrain_local_z), terrain_local_z);
//	pos = pos.transformCoord(trans);
//	estr->Spawn(my::Vector4(pos, 1), my::Vector4(0, 0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), 0, 0);
//}

static bool CopyFile_8536547B_AB27_41F9_84A9_6ABDF7B47887(const char* u8_src_path, const char* u8_dst_path)
{
	std::basic_string<TCHAR> src_path = u8tots(u8_src_path), dst_path = u8tots(u8_dst_path);
	WIN32_FIND_DATA srcffd;
	HANDLE hFind = FindFirstFile(src_path.c_str(), &srcffd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		WIN32_FIND_DATA dstffd;
		hFind = FindFirstFile(dst_path.c_str(), &dstffd);
		if (INVALID_HANDLE_VALUE == hFind || CompareFileTime(&srcffd.ftLastWriteTime, &dstffd.ftLastWriteTime) > 0)
		{
			return CopyFile(src_path.c_str(), dst_path.c_str(), FALSE);
		}
	}
	return false;
}

class FindFileIterator : public std::iterator<std::forward_iterator_tag, std::basic_string<TCHAR> >
{
protected:
	HANDLE hFind;

	WIN32_FIND_DATA ffd;

public:
	explicit FindFileIterator(
		LPCTSTR dir)
		: hFind(INVALID_HANDLE_VALUE)
	{
		if (dir)
		{
			hFind = FindFirstFile(dir, &ffd);
		}
	}
	// Assignment operator
	FindFileIterator& operator=(const FindFileIterator& src)
	{
		hFind = src.hFind;
		ffd = src.ffd;
	}
	// Dereference an iterator
	std::basic_string<TCHAR> operator*()
	{
		// When the value is one step more than the last, it's an end iterator
		ASSERT(hFind != INVALID_HANDLE_VALUE);
		return ffd.cFileName;
	}
	// Prefix increment operator
	FindFileIterator& operator++()
	{
		// When the value is one step more than the last, it's an end iterator
		while (FindNextFile(hFind, &ffd))
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				return *this;
			}
		}
		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
		return *this;
	}
	// Postfix increment operator
	FindFileIterator operator++(int)
	{
		FindFileIterator temp = *this;
		temp++;                                      // Increment the value by the range step
		return temp;                                 // The iterator before it's incremented
	}
	// Comparisons
	bool operator==(const FindFileIterator& iter) const
	{
		//return obj_iter == iter.obj_iter;
		if (hFind != iter.hFind)
		{
			return false;
		}
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return true;
		}
		return _tcscmp(ffd.cFileName, iter.ffd.cFileName) == 0;
	}
	bool operator!=(const FindFileIterator& iter) const
	{
		return !operator ==(iter);
	}
};

static boost::iterator_range<FindFileIterator> FindFiles(const char* u8_dir)
{
	return boost::make_iterator_range(FindFileIterator(u8tots(u8_dir).c_str()), FindFileIterator(NULL));
}

static bool GetImageInfoFromFile(const char* u8_path, D3DXIMAGE_INFO* pSrcInfo)
{
	HRESULT hr = D3DXGetImageInfoFromFile(u8tots(u8_path).c_str(), pSrcInfo);
	return SUCCEEDED(hr);
}

static void RectAssignmentNodeLoadFromFile(my::RectAssignmentNode * ass, const char* u8_path)
{
	std::ifstream ifs(u8tots(u8_path).c_str());
	boost::archive::text_iarchive ia(ifs);
	ia >> boost::serialization::make_nvp("ass", *ass);
}

static void RectAssignmentNodeSave(my::RectAssignmentNode * ass, const char* u8_path)
{
	std::ofstream ofs(u8tots(u8_path).c_str());
	boost::archive::text_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("ass", *ass);
}

static void SnapshotDlgSetTexPath(CSnapshotDlg* dlg, std::string u8_path)
{
	dlg->m_TexPath = u8tots(u8_path.c_str()).c_str();
}

static std::string SnapshotDlgGetTexPath(CSnapshotDlg* dlg)
{
	return tstou8((LPCTSTR)dlg->m_TexPath);
}

static void SnapshotDlgSetComponentType(CSnapshotDlg* dlg, Component::ComponentType type, bool enable)
{
	unsigned int idx = type - Component::ComponentTypeMesh;
	if (idx < _countof(dlg->m_ComponentTypes))
	{
		dlg->m_ComponentTypes[idx] = enable;
	}
}

static bool SnapshotDlgGetComponentType(CSnapshotDlg* dlg, Component::ComponentType type)
{
	unsigned int idx = type - Component::ComponentTypeMesh;
	if (idx < _countof(dlg->m_ComponentTypes))
	{
		return dlg->m_ComponentTypes[idx];
	}
	return false;
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
	ON_COMMAND(ID_CONTROL_SCROLLBAR, &CMainFrame::OnControlScrollbar)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_SCROLLBAR, &CMainFrame::OnUpdateControlScrollbar)
	ON_COMMAND_RANGE(ID_TOOLS_SCRIPT1, ID_TOOLS_SCRIPT_LAST, &CMainFrame::OnToolsScript1)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TOOLS_SCRIPT1, ID_TOOLS_SCRIPT_LAST, &CMainFrame::OnUpdateToolsScript1)
	ON_COMMAND(ID_TOOLS_SNAPSHOT, &CMainFrame::OnToolsSnapshot)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_COORD, &CMainFrame::OnUpdateIndicatorCoord)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_POSTFIX, &CMainFrame::OnUpdateIndicatorPostfix)
	ON_COMMAND(ID_TOOLS_PLAYING, &CMainFrame::OnToolsPlaying)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_PLAYING, &CMainFrame::OnUpdateToolsPlaying)
	ON_COMMAND(ID_TOOLS_SNAP_TO_GRID, &CMainFrame::OnToolsSnapToGrid)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_SNAP_TO_GRID, &CMainFrame::OnUpdateToolsSnapToGrid)
	ON_COMMAND(ID_TOOLS_TERRAINTOOBJ, &CMainFrame::OnToolsTerraintoobj)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TERRAINTOOBJ, &CMainFrame::OnUpdateToolsTerraintoobj)
	ON_COMMAND(ID_ALIGN_LEFTS, &CMainFrame::OnAlignLefts)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_LEFTS, &CMainFrame::OnUpdateAlignLefts)
	ON_COMMAND(ID_ALIGN_RIGHTS, &CMainFrame::OnAlignRights)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_RIGHTS, &CMainFrame::OnUpdateAlignRights)
	ON_COMMAND(ID_ALIGN_TOPS, &CMainFrame::OnAlignTops)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_TOPS, &CMainFrame::OnUpdateAlignTops)
	ON_COMMAND(ID_ALIGN_BOTTOMS, &CMainFrame::OnAlignBottoms)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_BOTTOMS, &CMainFrame::OnUpdateAlignBottoms)
	ON_COMMAND(ID_ALIGN_VERTICAL, &CMainFrame::OnAlignVertical)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_VERTICAL, &CMainFrame::OnUpdateAlignVertical)
	ON_COMMAND(ID_ALIGN_HORIZONTAL, &CMainFrame::OnAlignHorizontal)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_HORIZONTAL, &CMainFrame::OnUpdateAlignHorizontal)
	ON_COMMAND(ID_ALIGN_ACROSS, &CMainFrame::OnAlignAcross)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_ACROSS, &CMainFrame::OnUpdateAlignAcross)
	ON_COMMAND(ID_ALIGN_DOWN, &CMainFrame::OnAlignDown)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_DOWN, &CMainFrame::OnUpdateAlignDown)
	ON_COMMAND(ID_ALIGN_WIDTH, &CMainFrame::OnAlignWidth)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_WIDTH, &CMainFrame::OnUpdateAlignWidth)
	ON_COMMAND(ID_ALIGN_HEIGHT, &CMainFrame::OnAlignHeight)
	ON_UPDATE_COMMAND_UI(ID_ALIGN_HEIGHT, &CMainFrame::OnUpdateAlignHeight)
	ON_COMMAND(ID_EDIT_COPY, &CMainFrame::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CMainFrame::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CMainFrame::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CMainFrame::OnUpdateEditPaste)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_COORD,
	ID_INDICATOR_POSTFIX,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	: OctRoot(-4096, 4096)
	, m_bEatAltUp(FALSE)
	, m_hitPosSet(false)
	, m_offMeshConRoot(-4096, 4096)
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
	, m_PaintMode(PaintModeAssign)
	, m_PaintRadius(5.0f)
	, m_PaintHeight(5.0f)
	, m_PaintColor(1.0f, 1.0f, 1.0f, 1.0f)
	, m_PaintEmitterSiblingId(0)
	, m_PaintParticleMinDist(1.0f)
	, m_PaintParticleAngle(0.0f, 0.0f)
	, m_RenderingView(NULL)
	, m_IndicatorCoord(0, 0, 0)
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

	//// ! Example how to improve cooking speed if needed
	//physx::PxTolerancesScale scale;
	//physx::PxCookingParams params(scale);
	//// disable mesh cleaning - perform mesh validation on development configurations
	//params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	//// disable edge precompute, edges are set for each triangle, slows contact generation
	//params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	//// lower hierarchy for internal mesh
	//params.meshCookingHint = physx::PxMeshCookingHint::eCOOKING_PERFORMANCE;
	//PhysxSdk::getSingleton().m_Cooking->setParams(params);

	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1);
	//m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_AXES, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, 1);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, theApp.default_physx_joint_localframe);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, theApp.default_physx_joint_limits);

	m_Player.reset(new Actor(NULL, my::Vector3(0, 0, 0), my::Quaternion::Identity(), my::Vector3(theApp.default_player_scale), my::AABB(-1, 1)));
	m_Player->InsertComponent(ComponentPtr(new Controller(NULL,
		theApp.default_player_height,
		theApp.default_player_radius,
		theApp.default_player_contact_offset,
		theApp.default_player_step_offset,
		theApp.default_player_slope_limit)));
	m_Player->InsertComponent(ComponentPtr(new Steering(NULL,
		theApp.default_player_run_speed,
		theApp.default_player_breaking_speed, 0.0f, NULL)));
	m_Player->InsertComponent(ComponentPtr(new PlayerBehavior(NULL)));
	AnimatorPtr animator(new Animator(NULL));
	std::vector<std::string> skels;
	boost::algorithm::split(skels, theApp.default_player_skel, boost::is_any_of(",;"), boost::algorithm::token_compress_off);
	skels.resize(4);
	animator->m_SkeletonPath = skels[0];
	AnimationNodePtr node_run_blend_list(new NodeRunBlendList("node_run_blend_list"));
	node_run_blend_list->SetChild(0, AnimationNodePtr(new AnimationNodeSequence(skels[1].c_str(), 1.0f, true, "idle")));
	node_run_blend_list->SetChild(1, AnimationNodePtr(new AnimationNodeSequence(skels[2].c_str(), 1.0f, true, "move")));
	node_run_blend_list->SetChild(2, AnimationNodePtr(new AnimationNodeSequence(skels[3].c_str(), 1.0f, true, "move")));
	AnimationNodeSlotPtr node_run_blend_list_slot(new AnimationNodeSlot("node_run_blend_list_slot"));
	node_run_blend_list_slot->SetChild(0, node_run_blend_list);
	animator->SetChild(0, node_run_blend_list_slot);
	animator->ReloadSequenceGroup();
	m_Player->InsertComponent(animator);

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
	if (!m_selactors.empty())
	{
		m_Pivot.m_Pos = m_selactors.front()->m_Base ? m_selactors.front()->m_World.getRow<3>().xyz : m_selactors.front()->m_Position;
		m_Pivot.m_Rot = m_selactors.size() > 1 || m_selactors.front()->m_Base || m_Pivot.m_Mode == Pivot::PivotModeMove ? my::Quaternion::Identity() : m_selactors.front()->m_Rotation;
	}
}

void CMainFrame::OnFrameTick(float fElapsedTime)
{
	if (m_Player->IsRequested())
	{
		ViewedActorSet::iterator actor_iter = m_ViewedActors.begin();
		for (; actor_iter != m_ViewedActors.end(); actor_iter++)
		{
			if (actor_iter->IsRequested())
			{
				actor_iter->Update(fElapsedTime);
			}
		}
	}
	else
	{
		ActorList::iterator actor_iter = m_selactors.begin();
		for (; actor_iter != m_selactors.end(); actor_iter++)
		{
			if ((*actor_iter)->IsRequested())
			{
				(*actor_iter)->Update(fElapsedTime);
			}
		}
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

	theApp.DoAllParallelTasks();

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
			.property("IsCaptured", luabind::tag_function<bool(CWnd*)>(
				boost::bind(std::equal_to<CWnd*>(), boost::placeholders::_1, boost::bind<CWnd*>(&CWnd::GetCapture))))

		, luabind::class_<CView, CWnd>("CView")

		, luabind::class_<CChildView, CView>("ChildView")
			.def_readonly("Camera", &CChildView::m_Camera)
			.def("PushLineVertex", &CChildView::PushLineVertex)
			.def("PushLine", &CChildView::PushLine)
			.def("PushLineAABB", &CChildView::PushLineAABB)
			.def("PushLineBox", &CChildView::PushLineBox)
			.def("PushTriangleVertex", &CChildView::PushTriangleVertex)
			.def("PushTriangle", &CChildView::PushTriangle)
			.def("PushCircle", luabind::tag_function<void(CChildView*, const my::Vector3&, float, D3DCOLOR)>(
				boost::bind(&duDebugDrawCircle, boost::placeholders::_1, boost::bind(&my::Vector3::x, boost::placeholders::_2), boost::bind(&my::Vector3::y, boost::placeholders::_2), boost::bind(&my::Vector3::z, boost::placeholders::_2), boost::placeholders::_3, boost::placeholders::_4, 1.0f)))
			.property("CursorPos", &cchildview_get_cursor_pos)
			.def_readonly("SwapChainBufferDesc", &CChildView::m_SwapChainBufferDesc)

		, luabind::class_<COutlinerWnd, luabind::bases<CWnd> >("COutlinerWnd")
			.def("OnInitItemList", &COutlinerWnd::OnInitItemList)
			.def("OnDestroyItemList", &COutlinerWnd::OnDestroyItemList)

		, luabind::class_<CMainFrame, luabind::bases<CWnd, PhysxScene> >("MainFrame")
			.def_readwrite("DlgView", &CMainFrame::m_View)
			.def_readwrite("DlgProj", &CMainFrame::m_Proj)
			.def_readwrite("DlgViewProj", &CMainFrame::m_ViewProj)
			.def_readonly("wndOutliner", &CMainFrame::m_wndOutliner)
			.def("InsertDlg", &CMainFrame::InsertDlg)
			.def("RemoveDlg", &CMainFrame::RemoveDlg)
			.def("RemoveAllDlg", &CMainFrame::RemoveAllDlg)
			.def("AddEntity", &CMainFrame::AddEntity)
			.def("AddEntity", luabind::tag_function<void(CMainFrame*, Actor*)>(
				boost::bind(boost::mem_fn(&CMainFrame::AddEntity), boost::placeholders::_1, boost::placeholders::_2, boost::bind(&AABB::transform, boost::bind(&Actor::m_aabb, boost::placeholders::_2), boost::bind(&Actor::m_World, boost::placeholders::_2)), Actor::MinBlock, Actor::Threshold)))
			.def("RemoveEntity", &CMainFrame::RemoveEntity)
			.def("ClearAllEntity", &CMainFrame::ClearAllEntity)
			.property("AllEntityNum", &CMainFrame::GetAllEntityNum)
			.property("AllEntityAABB", luabind::tag_function<AABB(CMainFrame*)>(
				boost::bind(&CMainFrame::GetAllEntityAABB, boost::placeholders::_1, AABB::Invalid())))
			.def("QueryEntity", &cmainframe_query_entity, luabind::return_stl_iterator)
			.def("PushToActorList", luabind::tag_function<void(CMainFrame*,ActorPtr)>(
				boost::bind((void(ActorPtrList::*)(ActorPtr const&)) & ActorPtrList::push_back, boost::bind<ActorPtrList&>(&CMainFrame::m_ActorList, boost::placeholders::_1), boost::placeholders::_2)))
			.def("PushToDialogList", luabind::tag_function<void(CMainFrame*,my::DialogPtr)>(
				boost::bind((void(DialogPtrList::*)(my::DialogPtr const&)) & DialogPtrList::push_back, boost::bind<DialogPtrList&>(&CMainFrame::m_DialogList, boost::placeholders::_1), boost::placeholders::_2)))
			.property("allactors", cmainframe_get_all_acts, luabind::return_stl_iterator)
			.def_readonly("selactors", &CMainFrame::m_selactors, luabind::return_stl_iterator)
			.def_readonly("selcmp", &CMainFrame::m_selcmp)
			.def_readonly("selctls", &CMainFrame::m_selctls, luabind::return_stl_iterator)
			.property("ActiveView", &CMainFrame::GetActiveView)
			.def_readonly("RenderingView", &CMainFrame::m_RenderingView)
			.def_readonly("IndicatorCoord", &CMainFrame::m_IndicatorCoord)
			.def_readonly("Player", &CMainFrame::m_Player)
			.def("addOffMeshConnection", &CMainFrame::addOffMeshConnection)

		, luabind::class_<CMainApp, luabind::bases<my::D3DContext, my::ResourceMgr, RenderPipeline> >("MainApp")
			.def_readonly("MainWnd", &CMainApp::m_pMainWnd)
			.def_readonly("UIRender", &CMainApp::m_UIRender)
			.def_readonly("Font", &CMainApp::m_Font)
			.def_readonly("keyboard", &CMainApp::m_keyboard)
			.def_readonly("mouse", &CMainApp::m_mouse)
			.def("LoadDictionary", &cmainapp_load_dictionary)

		//, luabind::def("spawn_terrain_pos_2_emitter", &spawn_terrain_pos_2_emitter)

		, luabind::def("CopyFile", &CopyFile_8536547B_AB27_41F9_84A9_6ABDF7B47887)

		, luabind::def("FindFiles", &FindFiles, luabind::return_stl_iterator)

		, luabind::def("GetKeyState", &GetAsyncKeyState)

		, luabind::class_<D3DXIMAGE_INFO>("D3DXIMAGE_INFO")
			.def_readonly("Width", &D3DXIMAGE_INFO::Width)
			.def_readonly("Height", &D3DXIMAGE_INFO::Height)
			.def_readonly("Depth", &D3DXIMAGE_INFO::Depth)
			.def_readonly("MipLevels", &D3DXIMAGE_INFO::MipLevels)
			.def_readonly("Format", &D3DXIMAGE_INFO::Format)
			.def_readonly("ResourceType", &D3DXIMAGE_INFO::ResourceType)
			.def_readonly("ImageFileFormat", &D3DXIMAGE_INFO::ImageFileFormat)

		, luabind::def("GetImageInfoFromFile", &GetImageInfoFromFile, luabind::pure_out_value(boost::placeholders::_2))

		, luabind::class_<my::RectAssignmentNode, CRect>("RectAssignmentNode")
			.def(luabind::constructor<int, int, int, int>())
			.def("AssignRect", &my::RectAssignmentNode::AssignRect, luabind::pure_out_value(boost::placeholders::_3))
			.def("LoadFromFile", &RectAssignmentNodeLoadFromFile)
			.def("Save", &RectAssignmentNodeSave)

		, luabind::class_<CSnapshotDlg>("SnapshotDlg")
			.def(luabind::constructor<CWnd*>())
			.property("TexPath", &SnapshotDlgGetTexPath, &SnapshotDlgSetTexPath)
			.def_readwrite("TexWidth", &CSnapshotDlg::m_TexWidth)
			.def_readwrite("TexHeight", &CSnapshotDlg::m_TexHeight)
			.def_readwrite("SnapArea", &CSnapshotDlg::m_SnapArea)
			.def_readwrite("SnapEye", &CSnapshotDlg::m_SnapEye)
			.def_readwrite("SnapEular", &CSnapshotDlg::m_SnapEular)
			.def("GetComponentType", &SnapshotDlgGetComponentType)
			.def("SetComponentType", &SnapshotDlgSetComponentType)
			.def_readwrite("RTType", &CSnapshotDlg::m_RTType)
			.def_readwrite("UseOrthoCamera", &CSnapshotDlg::m_UseOrthoCamera)
			.def("DoSnapshot", &CSnapshotDlg::DoSnapshot)

		//, luabind::class_<FastNoiseLite>("FastNoiseLite")
		//	.def(luabind::constructor<int>())
		//	.enum_("NoiseType")
		//	[
		//		luabind::value("NoiseType_OpenSimplex2", FastNoiseLite::NoiseType_OpenSimplex2),
		//		luabind::value("NoiseType_OpenSimplex2S", FastNoiseLite::NoiseType_OpenSimplex2S),
		//		luabind::value("NoiseType_Cellular", FastNoiseLite::NoiseType_Cellular),
		//		luabind::value("NoiseType_Perlin", FastNoiseLite::NoiseType_Perlin),
		//		luabind::value("NoiseType_ValueCubic", FastNoiseLite::NoiseType_ValueCubic),
		//		luabind::value("NoiseType_Value", FastNoiseLite::NoiseType_Value)
		//	]
		//	.enum_("RotationType3D")
		//	[
		//		luabind::value("RotationType3D_None", FastNoiseLite::RotationType3D_None),
		//		luabind::value("RotationType3D_ImproveXYPlanes", FastNoiseLite::RotationType3D_ImproveXYPlanes),
		//		luabind::value("RotationType3D_ImproveXZPlanes", FastNoiseLite::RotationType3D_ImproveXZPlanes)
		//	]
		//	.enum_("FractalType")
		//	[
		//		luabind::value("FractalType_None", FastNoiseLite::FractalType_None),
		//		luabind::value("FractalType_FBm", FastNoiseLite::FractalType_FBm),
		//		luabind::value("FractalType_Ridged", FastNoiseLite::FractalType_Ridged),
		//		luabind::value("FractalType_PingPong", FastNoiseLite::FractalType_PingPong),
		//		luabind::value("FractalType_DomainWarpProgressive", FastNoiseLite::FractalType_DomainWarpProgressive),
		//		luabind::value("FractalType_DomainWarpIndependent", FastNoiseLite::FractalType_DomainWarpIndependent)
		//	]
		//	.enum_("CellularDistanceFunction")
		//	[
		//		luabind::value("CellularDistanceFunction_Euclidean", FastNoiseLite::CellularDistanceFunction_Euclidean),
		//		luabind::value("CellularDistanceFunction_EuclideanSq", FastNoiseLite::CellularDistanceFunction_EuclideanSq),
		//		luabind::value("CellularDistanceFunction_Manhattan", FastNoiseLite::CellularDistanceFunction_Manhattan),
		//		luabind::value("CellularDistanceFunction_Hybrid", FastNoiseLite::CellularDistanceFunction_Hybrid)
		//	]
		//	.enum_("CellularReturnType")
		//	[
		//		luabind::value("CellularReturnType_CellValue", FastNoiseLite::CellularReturnType_CellValue),
		//		luabind::value("CellularReturnType_Distance", FastNoiseLite::CellularReturnType_Distance),
		//		luabind::value("CellularReturnType_Distance2", FastNoiseLite::CellularReturnType_Distance2),
		//		luabind::value("CellularReturnType_Distance2Add", FastNoiseLite::CellularReturnType_Distance2Add),
		//		luabind::value("CellularReturnType_Distance2Sub", FastNoiseLite::CellularReturnType_Distance2Sub),
		//		luabind::value("CellularReturnType_Distance2Mul", FastNoiseLite::CellularReturnType_Distance2Mul),
		//		luabind::value("CellularReturnType_Distance2Div", FastNoiseLite::CellularReturnType_Distance2Div)
		//	]
		//	.enum_("DomainWarpType")
		//	[
		//		luabind::value("DomainWarpType_OpenSimplex2", FastNoiseLite::DomainWarpType_OpenSimplex2),
		//		luabind::value("DomainWarpType_OpenSimplex2Reduced", FastNoiseLite::DomainWarpType_OpenSimplex2Reduced),
		//		luabind::value("DomainWarpType_BasicGrid", FastNoiseLite::DomainWarpType_BasicGrid)
		//	]
		//	.def("SetSeed", &FastNoiseLite::SetSeed)
		//	.def("SetFrequency", &FastNoiseLite::SetFrequency)
		//	.def("SetNoiseType", &FastNoiseLite::SetNoiseType)
		//	.def("SetRotationType3D", &FastNoiseLite::SetRotationType3D)
		//	.def("SetFractalType", &FastNoiseLite::SetFractalType)
		//	.def("SetFractalOctaves", &FastNoiseLite::SetFractalOctaves)
		//	.def("SetFractalLacunarity", &FastNoiseLite::SetFractalLacunarity)
		//	.def("SetFractalGain", &FastNoiseLite::SetFractalGain)
		//	.def("SetFractalWeightedStrength", &FastNoiseLite::SetFractalWeightedStrength)
		//	.def("SetFractalPingPongStrength", &FastNoiseLite::SetFractalPingPongStrength)
		//	.def("SetCellularDistanceFunction", &FastNoiseLite::SetCellularDistanceFunction)
		//	.def("SetCellularReturnType", &FastNoiseLite::SetCellularReturnType)
		//	.def("SetCellularJitter", &FastNoiseLite::SetCellularJitter)
		//	.def("SetDomainWarpType", &FastNoiseLite::SetDomainWarpType)
		//	.def("SetDomainWarpAmp", &FastNoiseLite::SetDomainWarpAmp)
		//	.def("GetNoise", (float (FastNoiseLite::*)(LUA_NUMBER, LUA_NUMBER)const)& FastNoiseLite::GetNoise)
		//	.def("GetNoise", (float (FastNoiseLite::*)(LUA_NUMBER, LUA_NUMBER, LUA_NUMBER)const)& FastNoiseLite::GetNoise)
		//	.def("DomainWarp", (void (FastNoiseLite::*)(LUA_NUMBER&, LUA_NUMBER&)const)& FastNoiseLite::DomainWarp)
		//	.def("DomainWarp", (void (FastNoiseLite::*)(LUA_NUMBER&, LUA_NUMBER&, LUA_NUMBER&)const)& FastNoiseLite::DomainWarp)

		//, luabind::class_<ClipperLib::IntPoint>("IntPoint")
		//	.def(luabind::constructor<int, int>())
		//	.def_readwrite("x", &ClipperLib::IntPoint::X)
		//	.def_readwrite("y", &ClipperLib::IntPoint::Y)
		//	.def(luabind::const_self == luabind::other<const ClipperLib::IntPoint&>())

		//, luabind::class_<ClipperLib::Path>("Path")
		//	.def(luabind::constructor<>())
		//	.def("AddPoint", (void (ClipperLib::Path::*)(const ClipperLib::IntPoint&))& ClipperLib::Path::push_back)
		//	.property("PointNum", &ClipperLib::Path::size)
		//	.def("GetPoint", (ClipperLib::IntPoint& (ClipperLib::Path::*)(size_t))& ClipperLib::Path::at)
		//	.property("Orientation", &ClipperLib::Orientation)
		//	.property("Area", &ClipperLib::Area)
		//	.def("PointInPolygon", luabind::tag_function<int(const ClipperLib::Path&, const ClipperLib::IntPoint&)>(
		//		boost::bind(ClipperLib::PointInPolygon, boost::placeholders::_2, boost::placeholders::_1)))
		//	.def("ReversePath", &ClipperLib::ReversePath)

		//, luabind::class_<ClipperLib::Paths>("Paths")
		//	.def(luabind::constructor<>())
		//	.def("AddPath", (void (ClipperLib::Paths::*)(const ClipperLib::Path&))& ClipperLib::Paths::push_back)
		//	.property("PathNum", &ClipperLib::Paths::size)
		//	.def("GetPath", (ClipperLib::Path& (ClipperLib::Paths::*)(size_t))& ClipperLib::Paths::at)

		//, luabind::class_<ClipperLib::Clipper>("Clipper")
		//	.enum_("InitOptions")
		//	[
		//		luabind::value("ioReverseSolution", ClipperLib::InitOptions::ioReverseSolution),
		//		luabind::value("ioStrictlySimple", ClipperLib::InitOptions::ioStrictlySimple),
		//		luabind::value("ioPreserveCollinear", ClipperLib::InitOptions::ioPreserveCollinear)
		//	]
		//	.enum_("ClipType")
		//	[
		//		luabind::value("ctIntersection", ClipperLib::ClipType::ctIntersection),
		//		luabind::value("ctUnion", ClipperLib::ClipType::ctUnion),
		//		luabind::value("ctDifference", ClipperLib::ClipType::ctDifference),
		//		luabind::value("ctXor", ClipperLib::ClipType::ctXor)
		//	]
		//	.enum_("PolyType")
		//	[
		//		luabind::value("ptSubject", ClipperLib::PolyType::ptSubject),
		//		luabind::value("ptClip", ClipperLib::PolyType::ptClip)
		//	]
		//	.enum_("PolyFillType")
		//	[
		//		luabind::value("pftEvenOdd", ClipperLib::PolyFillType::pftEvenOdd),
		//		luabind::value("pftNonZero", ClipperLib::PolyFillType::pftNonZero),
		//		luabind::value("pftPositive", ClipperLib::PolyFillType::pftPositive),
		//		luabind::value("pftNegative", ClipperLib::PolyFillType::pftNegative)
		//	]
		//	.def(luabind::constructor<int>())
		//	.def("AddPath", &ClipperLib::Clipper::AddPath)
		//	.def("Execute", (bool (ClipperLib::Clipper::*)(ClipperLib::ClipType, ClipperLib::Paths&, ClipperLib::PolyFillType))&ClipperLib::Clipper::Execute, luabind::pure_out_value(boost::placeholders::_3))

		//, luabind::class_<ClipperLib::ClipperOffset>("ClipperOffset")
		//	.enum_("JoinType")
		//	[
		//		luabind::value("jtSquare", ClipperLib::JoinType::jtSquare),
		//		luabind::value("jtRound", ClipperLib::JoinType::jtRound),
		//		luabind::value("jtMiter", ClipperLib::JoinType::jtMiter)
		//	]
		//	.enum_("EndType")
		//	[
		//		luabind::value("etClosedPolygon", ClipperLib::EndType::etClosedPolygon),
		//		luabind::value("etClosedLine", ClipperLib::EndType::etClosedLine),
		//		luabind::value("etOpenButt", ClipperLib::EndType::etOpenButt),
		//		luabind::value("etOpenSquare", ClipperLib::EndType::etOpenSquare),
		//		luabind::value("etOpenRound", ClipperLib::EndType::etOpenRound)
		//	]
		//	.def(luabind::constructor<double,double>())
		//	.def("AddPath", &ClipperLib::ClipperOffset::AddPath)
		//	.def("AddPaths", &ClipperLib::ClipperOffset::AddPaths)
		//	.def("Execute", (void (ClipperLib::ClipperOffset::*)(ClipperLib::Paths&, double))& ClipperLib::ClipperOffset::Execute, luabind::pure_out_value(boost::placeholders::_2))

		, luabind::class_<HdriToCubemap<float> >("HdriToCubemapFloat")
			.def(luabind::constructor<const std::string&, int, bool>())
			.property("CubemapResolution", &HdriToCubemap<float>::getCubemapResolution)
			.property("NumChannels", &HdriToCubemap<float>::getNumChannels)
			.def("writeCubemap", &HdriToCubemap<float>::writeCubemap)
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
	m_offMeshConRoot.ClearAllEntity();
	m_offMeshConChunks.clear();
	ASSERT(m_selactors.empty());
	m_selcmp = NULL;
	m_selchunkid.SetPoint(0, 0);
	m_selinstid = 0;
	m_selctls.clear();
	ASSERT(m_ViewedActors.empty());
	LuaContext::Shutdown();
	theApp.m_CollectionObjs.clear();
	_ASSERT(theApp.m_NamedObjects.empty());
	my::NamedObject::postfix_i = 0;
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
	theApp.m_FogColor = scene->m_FogColor;
	theApp.m_BkColor = scene->m_BkColor;
	theApp.m_CascadeLayerBias = scene->m_ShadowBias;
	theApp.m_DofParams = scene->m_DofParams;
	theApp.m_LuminanceThreshold = scene->m_LuminanceThreshold;
	theApp.m_BloomColor = scene->m_BloomColor;
	theApp.m_BloomFactor = scene->m_BloomFactor;
	theApp.m_SsaoBias = scene->m_SsaoBias;
	theApp.m_SsaoIntensity = scene->m_SsaoIntensity;
	theApp.m_SsaoRadius = scene->m_SsaoRadius;
	theApp.m_SsaoScale = scene->m_SsaoScale;

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
	// ! save all actor in the scene, excluding ComponentTypeScript
	struct Callback : public my::OctNode::QueryCallback
	{
		typedef std::map<std::string, Actor*> NamedActorMap;

		NamedActorMap acts;

		Callback(void)
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);
			if (Component* cmp = actor->GetFirstComponent(Component::ComponentTypeScript, 0))
			{
				CString msg;
				msg.Format(_T("invalid serialization: %S.%S"), actor->GetName(), cmp->GetName());
				AfxMessageBox(msg);
				return false;
			}
			VERIFY(acts.insert(std::make_pair(actor->GetName(), actor)).second);
			return true;
		}
	};

	Callback cb;
	if (!QueryAllEntity(&cb))
	{
		return FALSE;
	}

	std::ofstream ofs(lpszPathName, std::ios::binary, _SH_DENYRW);
	LPCTSTR Ext = PathFindExtension(lpszPathName);
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(ofs, ts2ms(Ext).c_str());
	*oa << boost::serialization::make_nvp("SkyLightCam.m_Euler", theApp.m_SkyLightCam->m_Euler);
	*oa << boost::serialization::make_nvp("SkyLightColor", theApp.m_SkyLightColor);
	*oa << boost::serialization::make_nvp("AmbientColor", theApp.m_AmbientColor);
	*oa << boost::serialization::make_nvp("FogColor", theApp.m_FogColor);
	*oa << boost::serialization::make_nvp("BkColor", theApp.m_BkColor);
	*oa << boost::serialization::make_nvp("ShadowBias", theApp.m_CascadeLayerBias);
	*oa << boost::serialization::make_nvp("DofParams", theApp.m_DofParams);
	*oa << boost::serialization::make_nvp("LuminanceThreshold", theApp.m_LuminanceThreshold);
	*oa << boost::serialization::make_nvp("BloomColor", theApp.m_BloomColor);
	*oa << boost::serialization::make_nvp("BloomFactor", theApp.m_BloomFactor);
	*oa << boost::serialization::make_nvp("SsaoBias", theApp.m_SsaoBias);
	*oa << boost::serialization::make_nvp("SsaoIntensity", theApp.m_SsaoIntensity);
	*oa << boost::serialization::make_nvp("SsaoRadius", theApp.m_SsaoRadius);
	*oa << boost::serialization::make_nvp("SsaoScale", theApp.m_SsaoScale);

	LONG ActorListSize = cb.acts.size();
	*oa << BOOST_SERIALIZATION_NVP(ActorListSize);
	Callback::NamedActorMap::iterator act_iter = cb.acts.begin();
	for (int i = 0; act_iter != cb.acts.end(); act_iter++, i++)
	{
		ActorPtr actor_ptr = act_iter->second->shared_from_this();
		*oa << boost::serialization::make_nvp(str_printf("Actor%d", i).c_str(), actor_ptr);
	}

	LONG DialogListSize = m_DlgList.size();
	*oa << BOOST_SERIALIZATION_NVP(DialogListSize);
	DialogList::iterator dlg_iter = m_DlgList.begin();
	for (int i = 0; dlg_iter != m_DlgList.end(); dlg_iter++, i++)
	{
		my::DialogPtr dlg_ptr = boost::dynamic_pointer_cast<my::Dialog>((*dlg_iter)->shared_from_this());
		*oa << boost::serialization::make_nvp(str_printf("Dialog%d", i).c_str(), dlg_ptr);
	}

	theApp.m_EventLog(str_printf("CMainFrame::SaveFileContext: %d actors, %d dialogs", ActorListSize, DialogListSize).c_str());

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

	actor->StopAllActionInst();

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

void CMainFrame::addOffMeshConnection(const my::Vector3& v0, const my::Vector3& v1, float rad, unsigned char bidir, unsigned char area, unsigned short flags)
{
	OffmeshConnectionChunkPtr chunk(new OffmeshConnectionChunk());
	float* v = &chunk->m_Verts[0];
	chunk->m_Rad = rad;
	chunk->m_Dir = bidir;
	chunk->m_Area = area;
	chunk->m_Flag = flags;
	chunk->m_Id = 1000 + m_offMeshConChunks.size();
	rcVcopy(&v[0], &v0.x);
	rcVcopy(&v[3], &v1.x);
	m_offMeshConChunks.insert(m_offMeshConChunks.end(), chunk);

	my::AABB box(
		my::Min(v0.x, v1.x), my::Min(v0.y, v1.y), my::Min(v0.z, v1.z),
		my::Max(v0.x, v1.x), my::Max(v0.y, v1.y), my::Max(v0.z, v1.z));
	m_offMeshConRoot.AddEntity(chunk.get(), box, 1.0f, 0.01f);
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
	else
	{
		CChildView* pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
		ASSERT_VALID(pView);

		my::ModelViewerCamera* model_view_camera = dynamic_cast<my::ModelViewerCamera*>(pView->m_Camera.get());
		if (model_view_camera->m_DragMode != my::ModelViewerCamera::DragModeNone)
		{
			model_view_camera->m_DragMode = my::ModelViewerCamera::DragModeNone;
		}
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
	theApp.m_BkColor = D3DCOLOR_ARGB(0, 66, 75, 121);
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

	////TerrainPtr terrain(new Terrain(my::NamedObject::MakeUniqueName("terrain0").c_str(), 2, 2, 32, 1.0f));
	////terrain->AddMaterial(mtl);

	////ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("actor0").c_str(), my::Vector3(-terrain->m_RowChunks*terrain->m_ChunkSize/2, 0, -terrain->m_ColChunks*terrain->m_ChunkSize/2), my::Quaternion::Identity(), my::Vector3(1, 1, 1), my::AABB(-1, 1)));
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
	////	ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("actor0").c_str(), my::Vector3(my::Random(-3000.0f, 3000.0f), 0, my::Random(-3000.0f, 3000.0f)), my::Quaternion::Identity(), my::Vector3(1, 1, 1), my::AABB(-1, 1)));
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
	CFileDialog dlg(FALSE, _T("xml"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("xml|*.xml|txt|*.txt|binary|*.*||"), NULL, 0);
	dlg.m_ofn.lpstrFile = strPathName.GetBuffer(_MAX_PATH);
	INT_PTR nResult = dlg.DoModal();
	strPathName.ReleaseBuffer();
	if (nResult != IDOK)
	{
		return;
	}

	CWaitCursor wait;
	if (SaveFileContext(strPathName))
	{
		m_strPathName = strPathName;

		theApp.AddToRecentFileList(m_strPathName);
		OnUpdateFrameTitle(TRUE);
	}
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
	ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("actor0").c_str(), Pos, my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1)));
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

	ControllerPtr controller_cmp(new Controller(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_controller0").c_str()).c_str(), 1.0f, 1.0f, 0.1f, 0.5f, 0.0f));
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

	std::string path = theApp.GetRelativePath((LPCTSTR)dlg.GetPathName());
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
	rapidxml::xml_node<char>* node_sharedgeometry = node_mesh->first_node("sharedgeometry");
	if (node_sharedgeometry)
	{
		for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			char buff[64];
			sprintf_s(buff, _countof(buff), "%s_mesh%d", (*actor_iter)->GetName(), submesh_i);
			MeshComponentPtr mesh_cmp(new MeshComponent(my::NamedObject::MakeUniqueName(buff).c_str()));
			mesh_cmp->m_MeshPath = path;
			mesh_cmp->m_MeshSubMeshId = submesh_i;
			MaterialPtr mtl(new Material());
			mtl->m_Shader = theApp.default_shader;
			mtl->ParseShaderParameters();
			mesh_cmp->SetMaterial(mtl);
			(*actor_iter)->InsertComponent(mesh_cmp);
		}

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
		for (int submesh_i = 0; node_submesh != NULL; node_submesh = node_submesh->next_sibling(), submesh_i++)
		{
			char buff[64];
			sprintf_s(buff, _countof(buff), "%s_mesh%d", (*actor_iter)->GetName(), submesh_i);
			MeshComponentPtr mesh_cmp(new MeshComponent(my::NamedObject::MakeUniqueName(buff).c_str()));
			mesh_cmp->m_MeshPath = path;
			mesh_cmp->m_MeshSubMeshId = submesh_i;
			MaterialPtr mtl(new Material());
			mtl->m_Shader = theApp.default_shader;
			mtl->ParseShaderParameters();
			mesh_cmp->SetMaterial(mtl);
			(*actor_iter)->InsertComponent(mesh_cmp);

			my::AABB aabb(my::AABB::Invalid());
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

	std::string MeshPath = theApp.GetRelativePath((LPCTSTR)dlg.GetPathName());
	if (MeshPath.empty())
	{
		MessageBox(str_printf(_T("cannot relative path: %s"), (LPCTSTR)dlg.GetPathName()).c_str());
		return;
	}

	my::CachePtr cache = my::FileIStream::Open(dlg.GetPathName())->GetWholeCache();
	cache->push_back(0);
	rapidxml::xml_document<char> doc;
	my::OgreMeshPtr mesh(new my::OgreMesh());
	try
	{
		doc.parse<0>((char*)&(*cache)[0]);

		mesh->CreateMeshFromOgreXml(&doc, true, D3DXMESH_MANAGED, 0, 0);
	}
	catch (rapidxml::parse_error& e)
	{
		theApp.m_EventLog(e.what());
		return;
	}
	catch (my::Exception& e)
	{
		theApp.m_EventLog(e.what().c_str());
		return;
	}

	std::string ClothFabricPath = MeshPath + ".pxclothfabric";
	std::basic_string<TCHAR> FullPath = theApp.GetFullPath(ClothFabricPath.c_str());
	if (!DeleteFile(FullPath.c_str()))
	{
		DWORD code = GetLastError();
		if (code != ERROR_FILE_NOT_FOUND)
			THROW_WINEXCEPTION(GetLastError());
	}

	ClothComponentPtr cloth_cmp(new ClothComponent(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_cloth0").c_str()).c_str()));
	cloth_cmp->CreateClothFromMesh(ClothFabricPath.c_str(), mesh, GetGravity());

	//// set solver settings
	//cloth_cmp->m_Cloth->setSolverFrequency(240);

	//cloth_cmp->m_Cloth->setStiffnessFrequency(10.0f);

	//// damp global particle velocity to 90% every 0.1 seconds
	//cloth_cmp->m_Cloth->setDampingCoefficient(physx::PxVec3(0.2f)); // damp local particle velocity
	//cloth_cmp->m_Cloth->setLinearDragCoefficient(physx::PxVec3(0.2f)); // transfer frame velocity
	//cloth_cmp->m_Cloth->setAngularDragCoefficient(physx::PxVec3(0.2f)); // transfer frame rotation

	//// reduce impact of frame acceleration
	//// x, z: cloth swings out less when walking in a circle
	//// y: cloth responds less to jump acceleration
	//cloth_cmp->m_Cloth->setLinearInertiaScale(physx::PxVec3(0.8f, 0.6f, 0.8f));

	//// leave impact of frame torque at default
	//cloth_cmp->m_Cloth->setAngularInertiaScale(physx::PxVec3(1.0f));

	//// reduce centrifugal force of rotating frame
	//cloth_cmp->m_Cloth->setCentrifugalInertiaScale(physx::PxVec3(0.3f));

	// virtual particles
	cloth_cmp->CreateVirtualParticles(1);

	// ccd
	cloth_cmp->m_Cloth->setClothFlag(physx::PxClothFlag::eSWEPT_CONTACT, true);

	//// use GPU or not
	//cloth_cmp->m_Cloth->setClothFlag(physx::PxClothFlag::eCUDA, true);

	// custom fiber configuration
	cloth_cmp->m_Cloth->setStretchConfig(physx::PxClothFabricPhaseType::eVERTICAL, physx::PxClothStretchConfig(1.0f));
	cloth_cmp->m_Cloth->setStretchConfig(physx::PxClothFabricPhaseType::eHORIZONTAL, physx::PxClothStretchConfig(1.0f));
	cloth_cmp->m_Cloth->setStretchConfig(physx::PxClothFabricPhaseType::eSHEARING, physx::PxClothStretchConfig(0.75f));
	cloth_cmp->m_Cloth->setStretchConfig(physx::PxClothFabricPhaseType::eBENDING, physx::PxClothStretchConfig(0.5f));
	cloth_cmp->m_Cloth->setTetherConfig(physx::PxClothTetherConfig(1.0f));

	MaterialPtr mtl(new Material());
	mtl->m_Shader = theApp.default_shader;
	mtl->ParseShaderParameters();
	cloth_cmp->SetMaterial(mtl);
	(*actor_iter)->InsertComponent(cloth_cmp);
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

	CStaticEmitterDlg dlg(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_static_emit0").c_str()).c_str());
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

	SphericalEmitterPtr sphe_emit_cmp(new SphericalEmitter(my::NamedObject::MakeUniqueName(
		(std::string((*actor_iter)->GetName()) + "_sphe_emit0").c_str()).c_str(), 1024, EmitterComponent::FaceTypeCamera, EmitterComponent::SpaceTypeLocal));
	sphe_emit_cmp->m_SpawnInterval = 1 / 10.0f;
	sphe_emit_cmp->m_HalfSpawnArea = my::Vector3(0, 0, 0);
	sphe_emit_cmp->m_SpawnInclination = my::Vector2(D3DXToRadian(45), D3DXToRadian(90));
	sphe_emit_cmp->m_SpawnAzimuth = my::Vector2(0, D3DXToRadian(360));
	sphe_emit_cmp->m_SpawnSpeed = 5.0f;
	sphe_emit_cmp->m_SpawnBoneId = -1;
	sphe_emit_cmp->m_SpawnLocalPose = my::Bone(my::Vector3(0));
	sphe_emit_cmp->m_ParticleLifeTime = 1.0f;
	sphe_emit_cmp->m_ParticleGravity = my::Vector3(0, 0, 0);
	sphe_emit_cmp->m_ParticleDamping = 1.0f;
	sphe_emit_cmp->m_ParticleColorR.AddNode(0, 1, 0, 0);
	sphe_emit_cmp->m_ParticleColorG.AddNode(0, 1, 0, 0);
	sphe_emit_cmp->m_ParticleColorB.AddNode(0, 1, 0, 0);
	sphe_emit_cmp->m_ParticleColorA.AddNode(0, 1, 0, 0);
	sphe_emit_cmp->m_ParticleSizeX.AddNode(0, 1, 0, 0);
	sphe_emit_cmp->m_ParticleSizeY.AddNode(0, 1, 0, 0);
	sphe_emit_cmp->m_ParticleAngle.AddNode(0, 0, 0, 0);
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

	CTerrainDlg dlg(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_terrain0").c_str()).c_str());
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
	pCmdUI->Enable(!m_selactors.empty() || !m_selctls.empty());
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
	if (!m_selactors.empty() && m_selactors.front()->GetFirstComponent<Terrain>())
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
	if (!m_selactors.empty() && m_selactors.front()->GetFirstComponent<Terrain>())
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
	if (!m_selactors.empty() && m_selactors.front()->GetFirstComponent<Terrain>() && m_selactors.front()->GetFirstComponent<StaticEmitter>())
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

	std::string path = theApp.GetRelativePath((LPCTSTR)dlg.GetPathName());
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

	AnimatorPtr animator(new Animator(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_animator0").c_str()).c_str()));
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

	NavigationPtr navi_cmp(new Navigation(my::NamedObject::MakeUniqueName((std::string((*actor_iter)->GetName()) + "_navigation0").c_str()).c_str(), GetAllEntityAABB(my::AABB::Invalid())));

	CNavigationDlg dlg;
	dlg.m_bindingBox = *navi_cmp;
	dlg.m_AssetPath.Format(_T("terrain/%s"), ms2ts(navi_cmp->GetName()).c_str());
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	navi_cmp->m_navMeshPath = ts2ms((LPCTSTR)dlg.m_AssetPath);
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

	my::DialogPtr dlg(new my::Dialog(my::NamedObject::MakeUniqueName("dialog0").c_str()));
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
	skin->m_Image->m_TexturePath = theApp.default_static_img;
	skin->m_Image->m_Rect = theApp.default_static_img_rect;
	skin->m_Image->m_Border = theApp.default_static_img_border;
	skin->m_FontPath = theApp.default_font_path;
	skin->m_FontHeight = theApp.default_font_height;
	skin->m_FontFaceIndex = theApp.default_font_face_index;
	skin->m_TextColor = theApp.default_static_text_color;
	skin->m_TextAlign = theApp.default_static_text_align;

	my::StaticPtr static_ctl(new my::Static(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_static0").c_str()).c_str()));
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

	my::ProgressBarPtr pgs(new my::ProgressBar(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_progressbar0").c_str()).c_str()));
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

	my::ButtonPtr btn(new my::Button(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_button0").c_str()).c_str()));
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

	my::ImeEditBoxPtr edit(new my::ImeEditBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_imeeditbox0").c_str()).c_str()));
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

	my::CheckBoxPtr checkbox(new my::CheckBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_checkbox0").c_str()).c_str()));
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
	scroll_skin->m_PressedOffset = theApp.default_button_pressed_offset;
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

	my::ComboBoxPtr combobox(new my::ComboBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_combobox0").c_str()).c_str()));
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
	scroll_skin->m_PressedOffset = theApp.default_button_pressed_offset;
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

	my::ListBoxPtr listBox(new my::ListBox(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_listbox0").c_str()).c_str()));
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

		my::ButtonPtr btn(new my::Button(my::NamedObject::MakeUniqueName(str_printf("%s_item%d", listBox->GetName(), i).c_str()).c_str()));
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


void CMainFrame::OnControlScrollbar()
{
	// TODO: Add your command handler code here
	my::ScrollBarSkinPtr skin(new my::ScrollBarSkin());
	skin->m_Image.reset(new my::ControlImage());
	skin->m_Image->m_TexturePath = theApp.default_listbox_scrollbar_img;
	skin->m_Image->m_Rect = theApp.default_listbox_scrollbar_img_rect;
	skin->m_Image->m_Border = theApp.default_listbox_scrollbar_img_border;
	skin->m_PressedOffset = theApp.default_button_pressed_offset;
	skin->m_UpBtnNormalImage.reset(new my::ControlImage());
	skin->m_UpBtnNormalImage->m_TexturePath = theApp.default_listbox_scrollbarupbtn_normalimg;
	skin->m_UpBtnNormalImage->m_Rect = theApp.default_listbox_scrollbarupbtn_normalimg_rect;
	skin->m_UpBtnNormalImage->m_Border = theApp.default_listbox_scrollbarupbtn_normalimg_border;
	skin->m_UpBtnDisabledImage.reset(new my::ControlImage());
	skin->m_UpBtnDisabledImage->m_TexturePath = theApp.default_listbox_scrollbarupbtn_disabledimg;
	skin->m_UpBtnDisabledImage->m_Rect = theApp.default_listbox_scrollbarupbtn_disabledimg_rect;
	skin->m_UpBtnDisabledImage->m_Border = theApp.default_listbox_scrollbarupbtn_disabledimg_border;
	skin->m_DownBtnNormalImage.reset(new my::ControlImage());
	skin->m_DownBtnNormalImage->m_TexturePath = theApp.default_listbox_scrollbardownbtn_normalimg;
	skin->m_DownBtnNormalImage->m_Rect = theApp.default_listbox_scrollbardownbtn_normalimg_rect;
	skin->m_DownBtnNormalImage->m_Border = theApp.default_listbox_scrollbardownbtn_normalimg_border;
	skin->m_DownBtnDisabledImage.reset(new my::ControlImage());
	skin->m_DownBtnDisabledImage->m_TexturePath = theApp.default_listbox_scrollbardownbtn_disabledimg;
	skin->m_DownBtnDisabledImage->m_Rect = theApp.default_listbox_scrollbardownbtn_disabledimg_rect;
	skin->m_DownBtnDisabledImage->m_Border = theApp.default_listbox_scrollbardownbtn_disabledimg_border;
	skin->m_ThumbBtnNormalImage.reset(new my::ControlImage());
	skin->m_ThumbBtnNormalImage->m_TexturePath = theApp.default_listbox_scrollbarthumbbtn_normalimg;
	skin->m_ThumbBtnNormalImage->m_Rect = theApp.default_listbox_scrollbarthumbbtn_normalimg_rect;
	skin->m_ThumbBtnNormalImage->m_Border = theApp.default_listbox_scrollbarthumbbtn_normalimg_border;

	my::ScrollBarPtr scrollbar(new my::HorizontalScrollBar(my::NamedObject::MakeUniqueName((std::string(m_selctls.front()->GetName()) + "_hscrollbar0").c_str()).c_str()));
	scrollbar->m_Skin = skin;
	scrollbar->m_x.offset = 10;
	scrollbar->m_y.offset = 10;

	m_selctls.front()->InsertControl(scrollbar);
	m_selctls.front() = scrollbar.get();
	OnSelChanged();
}


void CMainFrame::OnUpdateControlScrollbar(CCmdUI* pCmdUI)
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
				hFind = FindFirstFileA(theApp.default_script_pattern.c_str(), &ffd);
				if (hFind == INVALID_HANDLE_VALUE)
				{
					break;
				}
				do
				{
					if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						m_ToolScripts[i - first_script_index] = ffd.cFileName;
						static const TCHAR* pref[] = { _T("&1 "), _T("&2 "), _T("&3 "), _T("&4 "), _T("&5 "), _T("&6 "), _T("&7 "), _T("&8 "), _T("&9 "), _T("1&0 ") };
						std::basic_string<TCHAR> strText(i - first_script_index < _countof(pref) ? pref[i - first_script_index] : _T(""));
						strText.append(ms2ts(m_ToolScripts[i - first_script_index].c_str()));
						if (i < pMenuBar->GetCount() && pMenuBar->GetButtonStyle(i) != TBBS_SEPARATOR && pMenuBar->GetItemID(i) < ID_TOOLS_SCRIPT_LAST)
						{
							pMenuBar->SetButtonText(i, strText.c_str());
							i++;
						}
						else
						{
							pMenuBar->InsertButton(CMFCToolBarMenuButton(ID_TOOLS_SCRIPT1 + i - first_script_index, NULL, -1, strText.c_str()), i);
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
	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CSnapshotDlg dlg;
	dlg.m_SnapEye = pView->m_Camera->m_Eye;
	dlg.m_SnapEular = pView->m_Camera->m_Euler;
	dlg.DoModal();
}


void CMainFrame::OnUpdateIndicatorCoord(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CPoint pt;
	GetCursorPos(&pt);
	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, WindowFromPoint(pt));
	if (pView && pView->m_OffscreenPositionRT->m_ptr) // theApp.m_DeviceObjectsReset
	{
		pView->ScreenToClient(&pt);
		CRect rc(pt, CSize(1,1));
		D3DLOCKED_RECT lrc = pView->m_OffscreenPositionRT->LockRect(&rc, D3DLOCK_READONLY);
		my::Vector3 pos = (*(my::Vector4*)lrc.pBits).xyz;
		pView->m_OffscreenPositionRT->UnlockRect();

		CString text;
		if (pos != my::Vector3::zero)
		{
			m_IndicatorCoord = pos.transformCoord(pView->m_Camera->m_View.inverse());
			text.Format(_T("%f, %f, %f"), m_IndicatorCoord.x, m_IndicatorCoord.y, m_IndicatorCoord.z);
		}
		else
		{
			text = _T("Infinite pos");
		}
		pCmdUI->SetText(text);
	}
}


void CMainFrame::OnUpdateIndicatorPostfix(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CString text;
	text.Format(_T("%d"), my::NamedObject::postfix_i);
	pCmdUI->SetText(text);
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
		if (m_PxScene->raycast((physx::PxVec3&)(m_Player->m_Position - controller->GetFootOffset() + my::Vector3(0, 1000, 0)),
			physx::PxVec3(0, -1, 0), 1000, hit, physx::PxHitFlag::eDEFAULT, filterData, NULL, NULL))
		{
			boost::dynamic_pointer_cast<ActionTrackPose>(ActionTbl::getSingleton().Climb->m_TrackList[0])->m_ParamPose =
				my::Bone((my::Vector3&)hit.block.position + controller->GetFootOffset(), m_Player->m_Rotation);
			m_Player->PlayAction(ActionTbl::getSingleton().Climb.get());
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
	theApp.default_snap_to_grid = !theApp.default_snap_to_grid;
}


void CMainFrame::OnUpdateToolsSnapToGrid(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck((GetKeyState('X') & 0x8000) ? !theApp.default_snap_to_grid : theApp.default_snap_to_grid);
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
	else if (!m_selactors.empty() && (dlg.m_terrain = m_selactors.front()->GetFirstComponent<Terrain>()))
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


void CMainFrame::OnAlignLefts()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_x.offset += (*begin_iter)->m_Rect.l - (*ctrl_iter)->m_Rect.l;
	}
}


void CMainFrame::OnUpdateAlignLefts(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignRights()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_x.offset += (*begin_iter)->m_Rect.r - (*ctrl_iter)->m_Rect.r;
	}
}


void CMainFrame::OnUpdateAlignRights(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignTops()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_y.offset += (*begin_iter)->m_Rect.t - (*ctrl_iter)->m_Rect.t;
	}
}


void CMainFrame::OnUpdateAlignTops(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignBottoms()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_y.offset += (*begin_iter)->m_Rect.b - (*ctrl_iter)->m_Rect.b;
	}
}


void CMainFrame::OnUpdateAlignBottoms(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignVertical()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_x.offset += (*begin_iter)->m_Rect.l - (*ctrl_iter)->m_Rect.l + ((*begin_iter)->m_Rect.r - (*begin_iter)->m_Rect.l - (*ctrl_iter)->m_Rect.r + (*ctrl_iter)->m_Rect.l) * 0.5f;
	}
}


void CMainFrame::OnUpdateAlignVertical(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignHorizontal()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_y.offset += (*begin_iter)->m_Rect.t - (*ctrl_iter)->m_Rect.t + ((*begin_iter)->m_Rect.b - (*begin_iter)->m_Rect.t - (*ctrl_iter)->m_Rect.b + (*ctrl_iter)->m_Rect.t) * 0.5f;
	}
}


void CMainFrame::OnUpdateAlignHorizontal(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignAcross()
{
	// TODO: Add your command handler code here
	struct CtrlCompare
	{
		bool operator() (my::Control* lhs, my::Control* rhs)
		{
			return lhs->m_Rect.l < rhs->m_Rect.l;
		}
	};
	std::sort(m_selctls.begin(), m_selctls.end(), CtrlCompare());
	for (int i = 1; i < m_selctls.size() - 1; i++)
	{
		m_selctls[i]->m_x.offset += m_selctls.front()->m_Rect.l + (m_selctls.back()->m_Rect.l - m_selctls.front()->m_Rect.l) * i / (m_selctls.size() - 1) - m_selctls[i]->m_Rect.l;
	}
}


void CMainFrame::OnUpdateAlignAcross(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 2);
}


void CMainFrame::OnAlignDown()
{
	// TODO: Add your command handler code here
	struct CtrlCompare
	{
		bool operator() (my::Control* lhs, my::Control* rhs)
		{
			return lhs->m_Rect.t < rhs->m_Rect.t;
		}
	};
	std::sort(m_selctls.begin(), m_selctls.end(), CtrlCompare());
	for (int i = 1; i < m_selctls.size() - 1; i++)
	{
		m_selctls[i]->m_y.offset += m_selctls.front()->m_Rect.t + (m_selctls.back()->m_Rect.t - m_selctls.front()->m_Rect.t) * i / (m_selctls.size() - 1) - m_selctls[i]->m_Rect.t;
	}
}


void CMainFrame::OnUpdateAlignDown(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 2);
}


void CMainFrame::OnAlignWidth()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_Width.offset += (*begin_iter)->m_Rect.Width() - (*ctrl_iter)->m_Rect.Width();
	}
}


void CMainFrame::OnUpdateAlignWidth(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnAlignHeight()
{
	// TODO: Add your command handler code here
	ControlList::iterator begin_iter = m_selctls.begin();
	ControlList::iterator ctrl_iter = begin_iter + 1;
	for (; ctrl_iter != m_selctls.end(); ctrl_iter++)
	{
		(*ctrl_iter)->m_Height.offset += (*begin_iter)->m_Rect.Height() - (*ctrl_iter)->m_Rect.Height();
	}
}


void CMainFrame::OnUpdateAlignHeight(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_selctls.size() > 1);
}


void CMainFrame::OnEditCopy()
{
	// TODO: Add your command handler code here
	std::ostringstream osstr;
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa = Actor::GetOArchive(osstr, ".txt");
	*oa << boost::serialization::make_nvp(__FUNCTION__, m_selactors);
	std::stringbuf* buf = osstr.rdbuf();
	std::stringbuf::_Buffer_view view = buf->_Get_buffer_view();

	//https://learn.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
	// Open the clipboard, and empty it. 
	if (!OpenClipboard())
		return;
	EmptyClipboard();

	// Allocate a global memory object for the text. 
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, view._Size + 1);
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return;
	}

	// Lock the handle and copy the text to the buffer. 
	LPSTR lptstrCopy = (LPSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, view._Ptr, view._Size);
	lptstrCopy[view._Size] = 0;    // null character 
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard. 
	SetClipboardData(CF_TEXT, hglbCopy);
}


void CMainFrame::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selactors.empty());
}


void CMainFrame::OnEditPaste()
{
	// TODO: Add your command handler code here
	if (!IsClipboardFormatAvailable(CF_TEXT))
		return;
	if (!OpenClipboard())
		return;

	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if (hglb == NULL)
	{
		CloseClipboard();
		return;
	}

	LPSTR lptstr = (LPSTR)GlobalLock(hglb);
	if (lptstr == NULL)
	{
		CloseClipboard();
		return;
	}
	// Call the application-defined ReplaceSelection 
	// function to insert the text and repaint the 
	// window. 
	std::string s(lptstr);
	GlobalUnlock(hglb);
	CloseClipboard();

	std::istringstream isstr(s);
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia = Actor::GetIArchive(isstr, ".txt");
	boost::dynamic_pointer_cast<my::NamedObjectSerializationContext>(ia)->make_unique = true;
	ActorList selacts;
	*ia >> boost::serialization::make_nvp(__FUNCTION__, selacts);

	ActorList::iterator act_iter = selacts.begin();
	for (; act_iter != selacts.end(); act_iter++)
	{
		m_ActorList.push_back(ActorPtr(*act_iter));
		AddEntity(*act_iter, (*act_iter)->m_aabb.transform((*act_iter)->m_World), Actor::MinBlock, Actor::Threshold);
	}
	m_selactors.swap(selacts);
}


void CMainFrame::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(IsClipboardFormatAvailable(CF_TEXT));
}

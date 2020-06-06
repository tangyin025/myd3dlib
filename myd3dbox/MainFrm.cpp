
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainFrm.h"
#include "ChildView.h"
#include "ShapeDlg.h"
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
#include "TerrainGrassBrashDlg.h"
#include "Animation.h"
#include "Win32InputBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	ON_COMMAND(ID_FILE_NEW, &CMainFrame::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, &CMainFrame::OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CMainFrame::OnFileSaveAs)
	ON_COMMAND(ID_CREATE_ACTOR, &CMainFrame::OnCreateActor)
	ON_COMMAND(ID_CREATE_CHARACTER, &CMainFrame::OnCreateCharacter)
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
	ON_COMMAND(ID_VIEW_CLEARSHADER, &CMainFrame::OnViewClearshader)
	ON_COMMAND(ID_TOOLS_BUILDNAVIGATION, &CMainFrame::OnToolsBuildnavigation)
	ON_COMMAND(ID_TOOLS_SIMPLIFYMESH, &CMainFrame::OnToolsSimplifymesh)
	ON_COMMAND(ID_TOOLS_TERRAINGRASSBRUSH, &CMainFrame::OnToolsTerraingrassbrush)
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
	: OctRoot(my::AABB(-4096, 4096))
	, m_bEatAltUp(FALSE)
	, m_selchunkid(0, 0)
	, m_selbox(-1, 1)
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2005);
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

	if (!PhysxSceneContext::Init(theApp.m_sdk.get(), theApp.m_CpuDispatcher.get()))
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

	//if (!m_wndOutliner.Create(_T("Outliner"), this, CRect(0,0,200,200), TRUE, 3001,
	//	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE))
	//{
	//	TRACE0("Failed to create outliner\n");
	//	return -1;
	//}

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

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndOutliner.EnableDocking(CBRS_ALIGN_ANY);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	m_wndEnvironment.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	//DockPane(&m_wndOutliner);
	DockPane(&m_wndProperties);
	DockPane(&m_wndEnvironment);
	CDockablePane* pTabbedBar = NULL;
	//m_wndProperties.AttachToTabWnd(&m_wndOutliner, DM_SHOW, FALSE, &pTabbedBar);
	//m_wndEnvironment.AttachToTabWnd(&m_wndProperties, DM_SHOW, FALSE, &pTabbedBar);


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
	//m_emitter->m_Material->m_PassMask = theApp.default_pass_mask;
	//m_emitter->m_Material->m_MeshTexture.m_Path = theApp.default_texture;
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
		SelActorList::const_iterator sel_iter = m_selactors.begin();
		for (; sel_iter != m_selactors.end(); sel_iter++)
		{
			m_selbox.unionSelf((*sel_iter)->m_aabb.transform((*sel_iter)->m_World));
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

	PhysxSceneContext::AdvanceSync(fElapsedTime);

	bool haveSelActors = false;
	physx::PxU32 nbActiveTransforms;
	const physx::PxActiveTransform* activeTransforms = m_PxScene->getActiveTransforms(nbActiveTransforms);
	for (physx::PxU32 i = 0; i < nbActiveTransforms; ++i)
	{
		Actor * actor = (Actor *)activeTransforms[i].userData;
		actor->OnPxTransformChanged(activeTransforms[i].actor2World);
		if (!haveSelActors && m_selactors.end() != std::find(m_selactors.begin(), m_selactors.end(), actor))
		{
			haveSelActors = true;
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
	UpdateSelBox();
	UpdatePivotTransform();
	my::EventArg arg;
	m_EventSelectionChanged(&arg);
}

void CMainFrame::ClearFileContext()
{
	OctRoot::ClearAllEntity();
	PhysxSceneContext::ClearSerializedObjs();
	theApp.ReleaseResource();
	ActorPtrSet::const_iterator actor_iter = m_ActorList.begin();
	for (; actor_iter != m_ActorList.end(); actor_iter++)
	{
		(*actor_iter)->StopAllAction();

		(*actor_iter)->ClearAllAttacher();

		if ((*actor_iter)->m_Base)
		{
			(*actor_iter)->m_Base->Detach(actor_iter->get());
		}

		if ((*actor_iter)->IsRequested())
		{
			(*actor_iter)->ReleaseResource();
		}

		if ((*actor_iter)->IsEnteredPhysx())
		{
			(*actor_iter)->LeavePhysxScene(this);
		}
	}
	m_ActorList.clear();
	m_selactors.clear();
}

BOOL CMainFrame::DoSave(LPCTSTR lpszPathName)
{
	if (!lpszPathName)
	{
		CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
		dlg.m_ofn.lpstrFile = m_strPathName.GetBuffer(_MAX_PATH);
		INT_PTR nResult = dlg.DoModal();
		m_strPathName.ReleaseBuffer();
		if (nResult != IDOK)
		{
			return FALSE;
		}
	}
	else
	{
		m_strPathName = lpszPathName;
	}

	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CWaitCursor waiter;
	std::ofstream ofs(m_strPathName, std::ios::binary, _OPENPROT);
	LPCTSTR Ext = PathFindExtension(m_strPathName);
	boost::shared_ptr<boost::archive::polymorphic_oarchive> oa;
	if (_tcsicmp(Ext, _T(".xml")) == 0)
	{
		oa.reset(new boost::archive::polymorphic_xml_oarchive(ofs));
	}
	else if (_tcsicmp(Ext, _T(".txt")) == 0)
	{
		oa.reset(new boost::archive::polymorphic_text_oarchive(ofs));
	}
	else
	{
		oa.reset(new boost::archive::polymorphic_binary_oarchive(ofs));
	}
	*oa << boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)theApp);
	*oa << boost::serialization::make_nvp("PhysxSceneContext", (PhysxSceneContext &)*this);
	*oa << boost::serialization::make_nvp("OctRoot", (OctRoot &)*this);
	*oa << BOOST_SERIALIZATION_NVP(m_ActorList);

	return TRUE;
}

void CMainFrame::OnDestroy()
{
	CFrameWndEx::OnDestroy();

	// TODO: Add your message handler code here
	//m_emitter.reset();
	ClearFileContext();
	theApp.DestroyD3DDevice();
	PhysxSceneContext::Shutdown();
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

void CMainFrame::OnFileNew()
{
	// TODO: Add your command handler code here
	ClearFileContext();
	m_strPathName.Empty();
	InitialUpdateFrame(NULL, TRUE);
	theApp.m_BgColor = my::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	theApp.m_SkyLightCam.m_Eular = my::Vector3(D3DXToRadian(-45), 0, 0);
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
	MaterialPtr mtl(new Material());
	mtl->m_Shader = "shader/mtl_water2.fx";
	mtl->ParseShaderParameters();

	//MeshComponentPtr mesh_cmp(new MeshComponent());
	//mesh_cmp->m_MeshPath = "mesh/Teapot.mesh.xml";
	//mesh_cmp->m_MeshSubMeshName = "Teapot001";
	//mesh_cmp->AddMaterial(mtl);

	TerrainPtr terrain(new Terrain(my::NamedObject::MakeUniqueName("editor_terrain").c_str(), 2, 2, 32, 1.0f));
	terrain->AddMaterial(mtl);

	ActorPtr actor(new Actor(my::NamedObject::MakeUniqueName("editor_actor").c_str(), my::Vector3(-terrain->m_RowChunks*terrain->m_ChunkSize/2, 0, -terrain->m_ColChunks*terrain->m_ChunkSize/2), my::Quaternion::Identity(), my::Vector3(1, 1, 1), my::AABB(-1, 1)));
	actor->AddComponent(terrain);
	actor->UpdateAABB();
	actor->UpdateWorld();
	AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World));
	m_ActorList.insert(actor);

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

	ClearFileContext();

	m_strPathName = strPathName;

	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CWaitCursor waiter;
	my::IStreamBuff buff(my::FileIStream::Open(m_strPathName));
	std::istream ifs(&buff);
	LPCTSTR Ext = PathFindExtension((LPCTSTR)m_strPathName);
	boost::shared_ptr<boost::archive::polymorphic_iarchive> ia;
	if (_tcsicmp(Ext, _T(".xml")) == 0)
	{
		ia.reset(new boost::archive::polymorphic_xml_iarchive(ifs));
	}
	else if (_tcsicmp(Ext, _T(".txt")) == 0)
	{
		ia.reset(new boost::archive::polymorphic_text_iarchive(ifs));
	}
	else
	{
		ia.reset(new boost::archive::polymorphic_binary_iarchive(ifs));
	}
	*ia >> boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)theApp);
	*ia >> boost::serialization::make_nvp("PhysxSceneContext", (PhysxSceneContext &)*this);
	*ia >> boost::serialization::make_nvp("OctRoot", (OctRoot &)*this);
	*ia >> BOOST_SERIALIZATION_NVP(m_ActorList);

	ActorPtrSet::const_iterator actor_iter = m_ActorList.begin();
	for (; actor_iter != m_ActorList.end(); actor_iter++)
	{
		AddEntity(actor_iter->get(), (*actor_iter)->m_aabb.transform((*actor_iter)->m_World));
	}

	theApp.RequestResource();
	OnSelChanged();
	CEnvironmentWnd::CameraPropEventArgs arg(pView);
	m_EventCameraPropChanged(&arg);
}

void CMainFrame::OnFileSave()
{
	// TODO: Add your command handler code here
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if (dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		DoSave(NULL);
	}
	else
	{
		DoSave(m_strPathName);
	}
}

void CMainFrame::OnFileSaveAs()
{
	// TODO: Add your command handler code here
	DoSave(NULL);
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
	AddEntity(actor.get(), actor->m_aabb.transform(actor->m_World));
	m_ActorList.insert(actor);
	actor->RequestResource();
	actor->EnterPhysxScene(this);

	m_selactors.clear();
	m_selactors.push_back(actor.get());
	OnSelChanged();
}

void CMainFrame::OnCreateCharacter()
{
	// TODO: Add your command handler code here
	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	my::Vector3 Pos(0,0,0);
	if (pView)
	{
		Pos = boost::dynamic_pointer_cast<my::ModelViewerCamera>(pView->m_Camera)->m_LookAt;
	}
	CharacterPtr character(new Character(my::NamedObject::MakeUniqueName("editor_character").c_str(), Pos, my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1), 1.0f, 1.0f, 0.1f, 1));
	character->UpdateWorld();
	AddEntity(character.get(), character->m_aabb.transform(character->m_World));
	m_ActorList.insert(character);
	character->RequestResource();
	character->EnterPhysxScene(this);

	m_selactors.clear();
	m_selactors.push_back(character.get());
	OnSelChanged();
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

	TCHAR buff[256] = _T("");
	if (IDOK != CWin32InputBox::InputBox(NULL, _T("sub mesh name"), buff, _countof(buff), false, m_hWnd))
	{
		return;
	}

	std::string path = ts2ms((LPCTSTR)dlg.GetPathName());
	std::string sub_mesh_name = ts2ms(buff);
	my::OgreMeshPtr mesh = theApp.LoadMesh(path.c_str(), sub_mesh_name.c_str());
	if (!mesh)
	{
		return;
	}

	MeshComponentPtr mesh_cmp(new MeshComponent(my::NamedObject::MakeUniqueName("editor_mesh_cmp").c_str()));
	mesh_cmp->m_MeshPath = path;
	mesh_cmp->m_MeshSubMeshName = sub_mesh_name;
	for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
		mtl->ParseShaderParameters();
		mesh_cmp->m_MaterialList.push_back(mtl);
	}
	(*actor_iter)->AddComponent(mesh_cmp);
	(*actor_iter)->UpdateAABB();
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

	TCHAR buff[256] = _T("");
	if (IDOK != CWin32InputBox::InputBox(NULL, _T("sub mesh name"), buff, _countof(buff), false, m_hWnd))
	{
		return;
	}

	std::string path = ts2ms((LPCTSTR)dlg.GetPathName());
	std::string sub_mesh_name = ts2ms(buff);
	my::OgreMeshPtr mesh = theApp.LoadMesh(path.c_str(), sub_mesh_name.c_str());
	if (!mesh)
	{
		return;
	}

	ClothComponentPtr cloth_cmp(new ClothComponent(my::NamedObject::MakeUniqueName("editor_cloth_cmp").c_str()));
	cloth_cmp->CreateClothFromMesh(mesh);
	for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
		mtl->ParseShaderParameters();
		cloth_cmp->m_MaterialList.push_back(mtl);
	}
	(*actor_iter)->AddComponent(cloth_cmp);
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

	StaticEmitterComponentPtr emit_cmp(new StaticEmitterComponent(my::NamedObject::MakeUniqueName("editor_emit_cmp").c_str(), 1));
	MaterialPtr mtl(new Material());
	mtl->m_Shader = theApp.default_shader;
	mtl->ParseShaderParameters();
	emit_cmp->AddMaterial(mtl);
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

	SphericalEmitterComponentPtr sphe_emit_cmp(new SphericalEmitterComponent(my::NamedObject::MakeUniqueName("editor_sphe_emit_cmp").c_str(), 1024));
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
	sphe_emit_cmp->AddMaterial(mtl);
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

	TerrainDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	TerrainPtr terrain(new Terrain(my::NamedObject::MakeUniqueName("editor_terrain").c_str(), dlg.m_RowChunks, dlg.m_ColChunks, dlg.m_ChunkSize, 1.0f));
	MaterialPtr mtl(new Material());
	mtl->m_Shader = theApp.default_shader;
	mtl->ParseShaderParameters();
	terrain->AddMaterial(mtl);
	(*actor_iter)->AddComponent(terrain);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();
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
	for (; actor_iter != m_selactors.end(); actor_iter++)
	{
		(*actor_iter)->StopAllAction();

		(*actor_iter)->ClearAllAttacher();

		if ((*actor_iter)->m_Base)
		{
			(*actor_iter)->m_Base->Detach((*actor_iter));
		}

		if ((*actor_iter)->IsRequested())
		{
			(*actor_iter)->ReleaseResource();
		}

		if ((*actor_iter)->IsEnteredPhysx())
		{
			(*actor_iter)->LeavePhysxScene(this);
		}

		(*actor_iter)->m_Node->GetTopNode()->RemoveEntity((*actor_iter));

		m_ActorList.erase((*actor_iter)->shared_from_this());
	}
	m_selactors.clear();
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
	m_Pivot.m_Mode = Pivot::PivotModeMove;
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePivotMove(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_Pivot.m_Mode == Pivot::PivotModeMove);
}

void CMainFrame::OnPivotRotate()
{
	// TODO: Add your command handler code here
	m_Pivot.m_Mode = Pivot::PivotModeRot;
	my::EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePivotRotate(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_Pivot.m_Mode == Pivot::PivotModeRot);
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

void CMainFrame::OnToolsSimplifymesh()
{
	// TODO: Add your command handler code here
	CSimplifyMeshDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
}

void CMainFrame::OnToolsTerraingrassbrush()
{
	// TODO: Add your command handler code here
	//CTerrainGrassBrashDlg dlg;
	//if (dlg.DoModal() != IDOK)
	//{
	//	return;
	//}

	//SelActorList::iterator actor_iter = m_selactors.begin();
	//if (actor_iter == m_selactors.end())
	//{
	//	return;
	//}

	//Terrain * terrain = NULL;
	//Actor::ComponentPtrList::iterator cmp_iter = (*actor_iter)->m_Cmps.begin();
	//for (; cmp_iter != (*actor_iter)->m_Cmps.end(); cmp_iter++)
	//{
	//	if ((*cmp_iter)->m_Type == Component::ComponentTypeTerrain)
	//	{
	//		terrain = dynamic_cast<Terrain *>(cmp_iter->get());
	//	}
	//}
	//if (!terrain)
	//{
	//	return;
	//}

	//D3DLOCKED_RECT lrc = terrain->m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	//for (float z = 0; z < terrain->m_RowChunks * terrain->m_ChunkSize; z += 1.0f)
	//{
	//	for (float x = 0; x < terrain->m_ColChunks * terrain->m_ChunkSize; x += 1.0f)
	//	{
	//		int i = (int)floor(z / terrain->m_ChunkSize);
	//		int j = (int)floor(x / terrain->m_ChunkSize);
	//		TerrainChunk * chunk = terrain->m_Chunks[i][j];
	//		//chunk->Spawn(my::Vector3(x, terrain->GetPosHeight(lrc.pBits, lrc.Pitch, x, z), z),
	//		//	my::Vector3::zero, my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), boost::hash_value(std::make_pair(x, z)) / (float)SIZE_MAX * D3DX_PI * 2.0f);
	//	}
	//}
	//terrain->m_HeightMap.UnlockRect();
}

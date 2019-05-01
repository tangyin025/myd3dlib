
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
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>
#include "RecastDump.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "NavigationDlg.h"
#include "SimplifyMeshDlg.h"
#include "TerrainGrassBrashDlg.h"

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
	: m_bEatAltUp(FALSE)
	, m_selchunkid(0, 0)
	, m_selbox(-1, 1)
	, m_Root(my::AABB(-1024, 1024))
	, m_solid(NULL)
	//, m_triareas(NULL)
	, m_chf(NULL)
	, m_cset(NULL)
	, m_pmesh(NULL)
	, m_dmesh(NULL)
	, m_navMesh(NULL)
	, m_navQuery(NULL)
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2005);
	m_navQuery = dtAllocNavMeshQuery();
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

	if (!PhysXSceneContext::Init(theApp.m_sdk.get(), theApp.m_CpuDispatcher.get()))
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
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
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
	m_wndEnvironment.AttachToTabWnd(&m_wndProperties, DM_SHOW, FALSE, &pTabbedBar);


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
		ActorSet::const_iterator sel_iter = m_selactors.begin();
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

BOOL CMainFrame::OnFrameTick(float fElapsedTime)
{
	if (m_selactors.empty())
	{
		return FALSE;
	}

	ActorSet::iterator actor_iter = m_selactors.begin();
	for (; actor_iter != m_selactors.end(); actor_iter++)
	{
		(*actor_iter)->Update(fElapsedTime);
	}

	PhysXSceneContext::AdvanceSync(fElapsedTime);

	bool haveSelActors = false;
	physx::PxU32 nbActiveTransforms;
	const physx::PxActiveTransform* activeTransforms = m_PxScene->getActiveTransforms(nbActiveTransforms);
	for (physx::PxU32 i = 0; i < nbActiveTransforms; ++i)
	{
		Actor * actor = (Actor *)activeTransforms[i].userData;
		actor->OnPxTransformChanged(activeTransforms[i].actor2World);
		if (!haveSelActors && m_selactors.end() != m_selactors.find(actor))
		{
			haveSelActors = true;
		}
	}

	if (haveSelActors)
	{
		UpdateSelBox();
		UpdatePivotTransform();
	}

	EventArgs arg;
	m_EventSelectionPlaying(&arg);
	return TRUE;
}

void CMainFrame::OnSelChanged()
{
	UpdateSelBox();
	UpdatePivotTransform();
	EventArgs arg;
	m_EventSelectionChanged(&arg);
}

void CMainFrame::ClearFileContext()
{
	m_Root.ClearAllActor();
	m_selactors.clear();
	m_ViewedActors.clear();
	PhysXSceneContext::ClearSerializedObjs();
	theApp.ReleaseResource();
}

void CMainFrame::OnDestroy()
{
	CFrameWndEx::OnDestroy();

	// TODO: Add your message handler code here
	//m_emitter.reset();
	ClearFileContext();
	PhysXSceneContext::Shutdown();
	theApp.DestroyD3DDevice();
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

	//unsigned int numRows = 5;
	//unsigned int numCols = 5;
	//PxHeightFieldSample* samples = (PxHeightFieldSample*)malloc(sizeof(PxHeightFieldSample)*(numRows*numCols));
	//for (unsigned i = 0; i < numRows; i++)
	//{
	//	for (unsigned int j = 0; j < numCols; j++)
	//	{
	//		PxHeightFieldSample & sample = samples[i * numCols + j];
	//		sample.height = 0;
	//		sample.materialIndex0 = PxBitAndByte(0, false);
	//		sample.materialIndex1 = PxBitAndByte(0, false);
	//	}
	//}

	//PxHeightFieldDesc hfDesc;
	//hfDesc.format             = PxHeightFieldFormat::eS16_TM;
	//hfDesc.nbColumns          = numCols;
	//hfDesc.nbRows             = numRows;
	//hfDesc.samples.data       = samples;
	//hfDesc.samples.stride     = sizeof(PxHeightFieldSample);
	//PxHeightField* aHeightField = PhysXContext::getSingleton().m_sdk->createHeightField(hfDesc);
	//float heightScale = 1.0f;
	//float rowScale = 3.0f;
	//float colScale = 3.0f;
	//PxHeightFieldGeometry hfGeom(aHeightField, PxMeshGeometryFlags(), heightScale, rowScale, colScale);

	//RigidComponentPtr rigid_cmp(new RigidComponent(my::AABB(-5,5), my::Matrix4::Identity()));
	//rigid_cmp->m_RigidActor->createShape(hfGeom, *theApp.m_PxMaterial, PxTransform::createIdentity());
	//rigid_cmp->RequestResource();
	//m_WorldL.GetLevel(m_WorldL.m_LevelId)->AddActor(rigid_cmp.get(), rigid_cmp->m_aabb.transform(Component::GetCmpWorld(rigid_cmp.get())));
	//m_cmps.push_back(rigid_cmp);
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
	std::basic_ifstream<char> ifs(m_strPathName);
	boost::archive::polymorphic_xml_iarchive ia(ifs);
	ia >> boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)theApp);
	ia >> boost::serialization::make_nvp("PhysXSceneContext", (PhysXSceneContext &)*this);
	ia >> boost::serialization::make_nvp("Root", m_Root);

	theApp.RequestResource();
}

void CMainFrame::OnFileSave()
{
	// TODO: Add your command handler code here
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if (dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
		dlg.m_ofn.lpstrFile = m_strPathName.GetBuffer(_MAX_PATH);
		INT_PTR nResult = dlg.DoModal();
		m_strPathName.ReleaseBuffer();
		if (nResult != IDOK)
		{
			return;
		}
	}

	CChildView * pView = DYNAMIC_DOWNCAST(CChildView, GetActiveView());
	ASSERT_VALID(pView);
	CWaitCursor waiter;
	std::basic_ofstream<char> ofs(m_strPathName);
	boost::archive::polymorphic_xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("RenderPipeline", (RenderPipeline &)theApp);
	oa << boost::serialization::make_nvp("PhysXSceneContext", (PhysXSceneContext &)*this);
	oa << boost::serialization::make_nvp("Root", m_Root);
}

void CMainFrame::OnFileSaveAs()
{
	// TODO: Add your command handler code here
	AfxMessageBox(_T("Hello, world!"));
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
	ActorPtr actor(new Actor(Pos, my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1)));
	actor->UpdateWorld();
	m_Root.AddActor(actor, actor->m_aabb.transform(actor->m_World));
	actor->RequestResource();
	actor->OnEnterPxScene(this);

	m_selactors.clear();
	m_selactors.insert(actor.get());
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
	CharacterPtr character(new Character(Pos, my::Quaternion::Identity(), my::Vector3(1,1,1), my::AABB(-1,1), 1.0f, 1.0f));
	character->UpdateWorld();
	m_Root.AddActor(character, character->m_aabb.transform(character->m_World));
	character->RequestResource();
	character->OnEnterPxScene(this);

	m_selactors.clear();
	m_selactors.insert(character.get());
	OnSelChanged();
}

void CMainFrame::OnComponentMesh()
{
	// TODO: Add your command handler code here
	ActorSet::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strPathName = dlg.GetPathName();
	my::OgreMeshPtr mesh = theApp.LoadMesh(ts2ms((LPCTSTR)strPathName).c_str());
	if (!mesh)
	{
		return;
	}

	MeshComponentPtr mesh_cmp(new MeshComponent());
	mesh_cmp->m_MeshPath = ts2ms((LPCTSTR)strPathName);
	for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
	{
		MaterialPtr lambert1(new Material());
		lambert1->m_Shader = theApp.default_shader;
		lambert1->m_PassMask = theApp.default_pass_mask;
		lambert1->ParseShaderParameters();
		lambert1->SetParameterTexture("g_DiffuseTexture", theApp.default_texture);
		lambert1->SetParameterTexture("g_NormalTexture", theApp.default_normal_texture);
		lambert1->SetParameterTexture("g_SpecularTexture", theApp.default_specular_texture);
		mesh_cmp->m_MaterialList.push_back(lambert1);
	}
	mesh_cmp->RequestResource();
	mesh_cmp->OnEnterPxScene(this);
	(*actor_iter)->AddComponent(mesh_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	EventArgs arg;
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
	ActorSet::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}


	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strPathName = dlg.GetPathName();
	my::OgreMeshPtr mesh = theApp.LoadMesh(ts2ms((LPCTSTR)strPathName).c_str());
	if (!mesh)
	{
		return;
	}

	ClothComponentPtr cloth_cmp(new ClothComponent());
	cloth_cmp->CreateClothFromMesh(mesh, 1);
	for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
	{
		MaterialPtr lambert1(new Material());
		lambert1->m_Shader = theApp.default_shader;
		lambert1->m_PassMask = theApp.default_pass_mask;
		lambert1->ParseShaderParameters();
		lambert1->SetParameterTexture("g_DiffuseTexture", theApp.default_texture);
		lambert1->SetParameterTexture("g_NormalTexture", theApp.default_normal_texture);
		lambert1->SetParameterTexture("g_SpecularTexture", theApp.default_specular_texture);
		cloth_cmp->m_MaterialList.push_back(lambert1);
	}
	cloth_cmp->RequestResource();
	cloth_cmp->OnEnterPxScene(this);
	(*actor_iter)->AddComponent(cloth_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	EventArgs arg;
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
	ActorSet::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	StaticEmitterComponentPtr emit_cmp(new StaticEmitterComponent());
	MaterialPtr lambert1(new Material());
	lambert1->m_Shader = theApp.default_shader;
	lambert1->m_PassMask = theApp.default_pass_mask;
	lambert1->ParseShaderParameters();
	lambert1->SetParameterTexture("g_DiffuseTexture", theApp.default_texture);
	lambert1->SetParameterTexture("g_NormalTexture", theApp.default_normal_texture);
	lambert1->SetParameterTexture("g_SpecularTexture", theApp.default_specular_texture);
	emit_cmp->m_Material = lambert1;
	emit_cmp->RequestResource();
	emit_cmp->OnEnterPxScene(this);
	(*actor_iter)->AddComponent(emit_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	emit_cmp->Spawn(my::Vector3(0, 0, 0), my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0.0f);

	EventArgs arg;
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
	ActorSet::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	SphericalEmitterComponentPtr sphe_emit_cmp(new SphericalEmitterComponent());
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
	MaterialPtr lambert1(new Material());
	lambert1->m_Shader = theApp.default_shader;
	lambert1->m_PassMask = theApp.default_pass_mask;
	lambert1->ParseShaderParameters();
	lambert1->SetParameterTexture("g_DiffuseTexture", theApp.default_texture);
	lambert1->SetParameterTexture("g_NormalTexture", theApp.default_normal_texture);
	lambert1->SetParameterTexture("g_SpecularTexture", theApp.default_specular_texture);
	sphe_emit_cmp->m_Material = lambert1;
	sphe_emit_cmp->RequestResource();
	sphe_emit_cmp->OnEnterPxScene(this);
	(*actor_iter)->AddComponent(sphe_emit_cmp);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();

	EventArgs arg;
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
	ActorSet::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	TerrainDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	TerrainPtr terrain(new Terrain(dlg.m_RowChunks, dlg.m_ColChunks, dlg.m_ChunkSize, 1.0f));
	for (unsigned int i = 0; i < terrain->m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < terrain->m_Chunks.shape()[1]; j++)
		{
			TerrainChunk * chunk = terrain->m_Chunks[i][j];
			chunk->m_UvRepeat = dlg.m_UvRepeat;

			MaterialPtr lambert1(new Material());
			lambert1->m_Shader = theApp.default_shader;
			lambert1->m_PassMask = theApp.default_pass_mask;
			lambert1->ParseShaderParameters();
			lambert1->SetParameterTexture("g_DiffuseTexture", ts2ms((LPCTSTR)dlg.m_DiffuseTexture));
			lambert1->SetParameterTexture("g_NormalTexture", ts2ms((LPCTSTR)dlg.m_NormalTexture));
			lambert1->SetParameterTexture("g_SpecularTexture", ts2ms((LPCTSTR)dlg.m_SpecularTexture));
			chunk->m_Material = lambert1;
		}
	}
	MaterialPtr lambert1(new Material());
	lambert1->m_Shader = theApp.default_shader;
	lambert1->m_PassMask = theApp.default_pass_mask;
	lambert1->ParseShaderParameters();
	lambert1->SetParameterTexture("g_DiffuseTexture", theApp.default_texture);
	lambert1->SetParameterTexture("g_NormalTexture", theApp.default_normal_texture);
	lambert1->SetParameterTexture("g_SpecularTexture", theApp.default_specular_texture);
	terrain->m_GrassMaterial = lambert1;
	terrain->RequestResource();
	terrain->OnEnterPxScene(this);
	(*actor_iter)->AddComponent(terrain);
	(*actor_iter)->UpdateAABB();
	(*actor_iter)->UpdateOctNode();
	UpdateSelBox();
	m_selchunkid.SetPoint(0, 0);

	EventArgs arg;
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
	ActorSet::iterator actor_iter = m_selactors.begin();
	for (; actor_iter != m_selactors.end(); actor_iter++)
	{
		(*actor_iter)->OnLeavePxScene(this);
		//m_WorldL.m_ViewedActors.erase(*actor_iter);
		(*actor_iter)->m_Node->GetTopNode()->RemoveActor((*actor_iter)->shared_from_this());
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
	EventArgs arg;
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
	EventArgs arg;
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
	EventArgs arg;
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

	//const float* verts = 0;// m_geom->getMesh()->getVerts();
	//const int nverts = 100;// m_geom->getMesh()->getVertCount();
	//const int* tris = 0;// m_geom->getMesh()->getTris();
	//const int ntris = 10;// m_geom->getMesh()->getTriCount();

	//
	// Step 1. Initialize build config.
	//

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = 0.3f;// m_cellSize;
	m_cfg.ch = 0.2f;// m_cellHeight;
	m_cfg.walkableSlopeAngle = 45.0f;// m_agentMaxSlope;
	m_cfg.walkableHeight = 10;// (int)ceilf(m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = 4;// (int)floorf(m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = 2;// (int)ceilf(m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = 40;// (int)(m_edgeMaxLen / m_cellSize);
	m_cfg.maxSimplificationError = 1.3f;// m_edgeMaxError;
	m_cfg.minRegionArea = 64;// (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = 400;// (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = 6;// (int)m_vertsPerPoly;
	m_cfg.detailSampleDist = 1.8f;// m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	m_cfg.detailSampleMaxError = 0.2f;// m_cellHeight * m_detailSampleMaxError;

	rcVcopy(m_cfg.bmin, &m_Root.m_aabb.m_min.x);
	rcVcopy(m_cfg.bmax, &m_Root.m_aabb.m_max.x);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// Reset build times gathering.
	this->resetTimers();

	// Start the build process.	
	this->startTimer(RC_TIMER_TOTAL);

	this->log(RC_LOG_PROGRESS, "Building navigation:");
	this->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	//this->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

	//
	// Step 2. Rasterize input polygon soup.
	//

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return;
	}

	if (!rcCreateHeightfield(this, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return;
	}

	//// Allocate array that can hold triangle area types.
	//// If you have multiple meshes you need to process, allocate
	//// and array which can hold the max number of triangles you need to process.
	//m_triareas = new unsigned char[ntris];
	//if (!m_triareas)
	//{
	//	this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
	//	return;
	//}

	//// Find triangles which are walkable based on their slope and rasterize them.
	//// If your input data is multiple meshes, you can transform them here, calculate
	//// the are type for each of the meshes and rasterize them.
	//memset(m_triareas, 0, ntris*sizeof(unsigned char));
	//rcMarkWalkableTriangles(this, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	//{
	//	this->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
	//	return;
	//if (!rcRasterizeTriangles(this, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
	//}

	struct Callback : public my::OctNode::QueryCallback
	{
		Callback(void)
		{
		}
		void operator() (my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT_VALID(pFrame);
			Actor * actor = dynamic_cast<Actor *>(oct_actor);
			ASSERT(actor);
			if (!actor->m_PxActor || !actor->m_PxActor->isRigidStatic())
			{
				return;
			}
			const float walkableThr = cosf(pFrame->m_cfg.walkableSlopeAngle / 180.0f*RC_PI);
			Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
			for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
			{
				Component * cmp = cmp_iter->get();
				if (!cmp->m_PxShape)
				{
					continue;
				}
				switch (cmp->m_PxShape->getGeometryType())
				{
				case physx::PxGeometryType::eSPHERE:
				case physx::PxGeometryType::ePLANE:
				case physx::PxGeometryType::eCAPSULE:
				case physx::PxGeometryType::eBOX:
					TRACE("OnToolsBuildnavigation: unsupported collision shape");
					continue;
				case physx::PxGeometryType::eCONVEXMESH:
				{
					physx::PxConvexMeshGeometry geom;
					VERIFY(cmp->m_PxShape->getConvexMeshGeometry(geom));
					boost::const_multi_array_ref<physx::PxVec3, 1> verts(geom.convexMesh->getVertices(), boost::extents[geom.convexMesh->getNbVertices()]);
					const physx::PxU8 * polys = geom.convexMesh->getIndexBuffer();
					for (unsigned int i = 0; i < geom.convexMesh->getNbPolygons(); i++)
					{
						physx::PxHullPolygon hullpoly;
						geom.convexMesh->getPolygonData(i, hullpoly);
						if (hullpoly.mNbVerts < 3)
						{
							TRACE("OnToolsBuildnavigation: invalid polygon");
							continue;
						}
						for (int j = 2; j < hullpoly.mNbVerts; j++)
						{
							my::Vector3 v0 = ((my::Vector3 &)verts[polys[hullpoly.mIndexBase + 0]]).transformCoord(actor->m_World);
							my::Vector3 v1 = ((my::Vector3 &)verts[polys[hullpoly.mIndexBase + j - 1]]).transformCoord(actor->m_World);
							my::Vector3 v2 = ((my::Vector3 &)verts[polys[hullpoly.mIndexBase + j - 0]]).transformCoord(actor->m_World);
							rcRasterizeTriangle(pFrame, &v0.x, &v1.x, &v2.x, hullpoly.mPlane[1] > walkableThr ? RC_WALKABLE_AREA : 0, *pFrame->m_solid, pFrame->m_cfg.walkableClimb);
						}
					}
					break;
				}
				case physx::PxGeometryType::eTRIANGLEMESH:
				{
					physx::PxTriangleMeshGeometry geom;
					VERIFY(cmp->m_PxShape->getTriangleMeshGeometry(geom));
					boost::const_multi_array_ref<physx::PxVec3, 1> verts(geom.triangleMesh->getVertices(), boost::extents[geom.triangleMesh->getNbVertices()]);
					for (unsigned int i = 0; i < geom.triangleMesh->getNbTriangles(); i++)
					{
						my::Vector3 v0, v1, v2;
						if (geom.triangleMesh->getTriangleMeshFlags().isSet(physx::PxTriangleMeshFlag::e16_BIT_INDICES))
						{
							boost::const_multi_array_ref<unsigned short, 1> tris((unsigned short *)geom.triangleMesh->getTriangles(), boost::extents[geom.triangleMesh->getNbTriangles() * 3]);
							v0 = ((my::Vector3 &)verts[tris[i * 3 + 0]]).transformCoord(actor->m_World);
							v1 = ((my::Vector3 &)verts[tris[i * 3 + 1]]).transformCoord(actor->m_World);
							v2 = ((my::Vector3 &)verts[tris[i * 3 + 2]]).transformCoord(actor->m_World);
						}
						else
						{
							boost::const_multi_array_ref<int, 1> tris((int *)geom.triangleMesh->getTriangles(), boost::extents[geom.triangleMesh->getNbTriangles() * 3]);
							v0 = ((my::Vector3 &)verts[tris[i * 3 + 0]]).transformCoord(actor->m_World);
							v1 = ((my::Vector3 &)verts[tris[i * 3 + 1]]).transformCoord(actor->m_World);
							v2 = ((my::Vector3 &)verts[tris[i * 3 + 2]]).transformCoord(actor->m_World);
						}
						my::Vector3 Normal = (v1 - v0).cross(v2 - v0).normalize();
						rcRasterizeTriangle(pFrame, &v0.x, &v1.x, &v2.x, Normal.y > walkableThr ? RC_WALKABLE_AREA : 0, *pFrame->m_solid, pFrame->m_cfg.walkableClimb);
					}
					break;
				}
				case physx::PxGeometryType::eHEIGHTFIELD:
				{
					Terrain * terrain = dynamic_cast<Terrain*>(cmp_iter->get());
					if (!terrain)
					{
						TRACE("OnToolsBuildnavigation: invalid terrain component");
						continue;
					}
					D3DLOCKED_RECT lrc = terrain->m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
					const void * pVertices = terrain->m_vb.Lock(0, 0, D3DLOCK_READONLY);
					for (int i = 0; i < terrain->m_RowChunks; i++)
					{
						for (int j = 0; j < terrain->m_ColChunks; j++)
						{
							const Terrain::Fragment & frag = terrain->GetFragment(0, 0, 0, 0, 0);
							const void * pIndices = const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY);
							for (unsigned int face_i = 0; face_i < frag.PrimitiveCount; face_i++)
							{
								int i0 = *((Terrain::VertexArray2D::element *)pIndices + face_i * 3 + 0);
								int i1 = *((Terrain::VertexArray2D::element *)pIndices + face_i * 3 + 1);
								int i2 = *((Terrain::VertexArray2D::element *)pIndices + face_i * 3 + 2);

								my::Vector3 v0 = terrain->GetPosByVertexIndex(pVertices, i, j, i0, lrc.pBits, lrc.Pitch).transformCoord(actor->m_World);
								my::Vector3 v1 = terrain->GetPosByVertexIndex(pVertices, i, j, i1, lrc.pBits, lrc.Pitch).transformCoord(actor->m_World);
								my::Vector3 v2 = terrain->GetPosByVertexIndex(pVertices, i, j, i2, lrc.pBits, lrc.Pitch).transformCoord(actor->m_World);

								my::Vector3 Normal = (v1 - v0).cross(v2 - v0).normalize();

								rcRasterizeTriangle(pFrame, &v0.x, &v1.x, &v2.x, Normal.y > walkableThr ? RC_WALKABLE_AREA : 0, *pFrame->m_solid, pFrame->m_cfg.walkableClimb);
							}
							const_cast<my::IndexBuffer&>(frag.ib).Unlock();
						}
					}
					terrain->m_vb.Unlock();
					terrain->m_HeightMap.UnlockRect(0);
					break;
				}
				}
			}
		}
	};
	m_Root.QueryActorAll(&Callback());

	//if (!m_keepInterResults)
	//{
	//	delete[] m_triareas;
	//	m_triareas = 0;
	//}

	bool m_filterLowHangingObstacles = true;
	bool m_filterLedgeSpans = true;
	bool m_filterWalkableLowHeightSpans = true;

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(this, m_cfg.walkableClimb, *m_solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(this, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(this, m_cfg.walkableHeight, *m_solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return;
	}
	if (!rcBuildCompactHeightfield(this, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return;
	}

	//if (!m_keepInterResults)
	//{
	//	rcFreeHeightField(m_solid);
	//	m_solid = 0;
	//}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(this, m_cfg.walkableRadius, *m_chf))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return;
	}

	//// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(this, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles

	enum SamplePartitionType
	{
		SAMPLE_PARTITION_WATERSHED,
		SAMPLE_PARTITION_MONOTONE,
		SAMPLE_PARTITION_LAYERS,
	};

	int m_partitionType = SAMPLE_PARTITION_WATERSHED;

	if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(this, *m_chf))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(this, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return;
		}
	}
	else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(this, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
			return;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(this, *m_chf, 0, m_cfg.minRegionArea))
		{
			this->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
			return;
		}
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return;
	}
	if (!rcBuildContours(this, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return;
	}
	if (!rcBuildPolyMesh(this, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return;
	}

	if (!rcBuildPolyMeshDetail(this, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		this->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return;
	}

	//if (!m_keepInterResults)
	//{
	//	rcFreeCompactHeightfield(m_chf);
	//	m_chf = 0;
	//	rcFreeContourSet(m_cset);
	//	m_cset = 0;
	//}

	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//

	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i)
		{
			if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
				m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

			if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
				m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
				m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
			}
			else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			}
			else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}


		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		//params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		//params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		//params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		//params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		//params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		//params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		//params.offMeshConCount = m_geom->getOffMeshConnectionCount();
		params.walkableHeight = 2.0f;// m_agentHeight;
		params.walkableRadius = 0.6f;// m_agentRadius;
		params.walkableClimb = 0.9f;// m_agentMaxClimb;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			this->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return;
		}

		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			this->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return;
		}

		dtStatus status;

		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			this->log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return;
		}

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			this->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return;
		}
	}

	this->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	duLogBuildTimes(*this, this->getAccumulatedTime(RC_TIMER_TOTAL));
	this->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);
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
	CTerrainGrassBrashDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	ActorSet::iterator actor_iter = m_selactors.begin();
	if (actor_iter == m_selactors.end())
	{
		return;
	}

	Terrain * terrain = NULL;
	Actor::ComponentPtrList::iterator cmp_iter = (*actor_iter)->m_Cmps.begin();
	for (; cmp_iter != (*actor_iter)->m_Cmps.end(); cmp_iter++)
	{
		if ((*cmp_iter)->m_Type == Component::ComponentTypeTerrain)
		{
			terrain = dynamic_cast<Terrain *>(cmp_iter->get());
		}
	}
	if (!terrain)
	{
		return;
	}

	D3DLOCKED_RECT lrc = terrain->m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	for (float z = 0; z < terrain->m_RowChunks * terrain->m_ChunkSize; z += 1.0f)
	{
		for (float x = 0; x < terrain->m_ColChunks * terrain->m_ChunkSize; x += 1.0f)
		{
			int i = (int)floor(z / terrain->m_ChunkSize);
			int j = (int)floor(x / terrain->m_ChunkSize);
			TerrainChunk * chunk = terrain->m_Chunks[i][j];
			chunk->Spawn(my::Vector3(x, terrain->GetPosHeight(lrc.pBits, lrc.Pitch, x, z), z),
				my::Vector3::zero, my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), boost::hash_value(std::make_pair(x, z)) / (float)SIZE_MAX * D3DX_PI * 2.0f);
		}
	}
	terrain->m_HeightMap.UnlockRect();
}

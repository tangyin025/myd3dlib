
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainApp.h"

#include "MainFrm.h"
#include "ChildView.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

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
	ON_COMMAND(ID_COMPONENT_MESH, &CMainFrame::OnComponentMesh)
	ON_COMMAND(ID_COMPONENT_EMITTER, &CMainFrame::OnComponentEmitter)
	ON_COMMAND(ID_COMPONENT_SPHERICALEMITTER, &CMainFrame::OnComponentSphericalemitter)
	ON_COMMAND(ID_EDIT_DELETE, &CMainFrame::OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CMainFrame::OnUpdateEditDelete)
	ON_COMMAND(ID_PIVOT_MOVE, &CMainFrame::OnPivotMove)
	ON_UPDATE_COMMAND_UI(ID_PIVOT_MOVE, &CMainFrame::OnUpdatePivotMove)
	ON_COMMAND(ID_PIVOT_ROTATE, &CMainFrame::OnPivotRotate)
	ON_UPDATE_COMMAND_UI(ID_PIVOT_ROTATE, &CMainFrame::OnUpdatePivotRotate)
	ON_WM_TIMER()
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
	: m_Root(my::Vector3(-1000), my::Vector3(1000), 1.0f)
	, m_selbox(-FLT_MAX, FLT_MAX)
	, m_bEatAltUp(FALSE)
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

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndOutliner.EnableDocking(CBRS_ALIGN_ANY);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	//DockPane(&m_wndOutliner);
	DockPane(&m_wndProperties);
	//CDockablePane* pTabbedBar = NULL;
	//m_wndProperties.AttachToTabWnd(&m_wndOutliner, DM_SHOW, FALSE, &pTabbedBar);


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

	SetTimer(1, 33, NULL);

	m_emitter.reset(new EmitterComponent(my::AABB(FLT_MAX, -FLT_MAX), my::Matrix4::Identity()));
	m_emitter->m_Emitter.reset(new my::Emitter());
	m_emitter->m_Material.reset(new Material());
	m_emitter->m_Material->m_Shader = "lambert1.fx";
	m_emitter->m_Material->m_PassMask = RenderPipeline::PassMaskOpaque;
	m_emitter->m_Material->m_MeshTexture.m_Path = "texture/Checker.bmp";
	m_emitter->RequestResource();
	//m_emitter->m_Emitter->Spawn(my::Vector3(0,0,0), my::Vector3(0,0,0), D3DCOLOR_ARGB(255,255,255,255), my::Vector2(1,1), 0);
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
	m_selbox = my::AABB(FLT_MAX, -FLT_MAX);
	ComponentSet::const_iterator sel_iter = m_selcmps.begin();
	for (; sel_iter != m_selcmps.end(); sel_iter++)
	{
		m_selbox.unionSelf((*sel_iter)->GetComponentAABB());
	}
}

void CMainFrame::UpdatePivotTransform(void)
{
	if (m_selcmps.size() == 1)
	{
		my::Vector3 Pos, Scale; my::Quaternion Rot;
		(*m_selcmps.begin())->m_World.Decompose(Scale, Rot, Pos);
		m_Pivot.m_Pos = Pos;
		m_Pivot.m_Rot = (m_Pivot.m_Mode == Pivot::PivotModeMove ? my::Quaternion::Identity() : Rot);
	}
	else if (!m_selcmps.empty())
	{
		m_Pivot.m_Pos = m_selbox.Center();
		m_Pivot.m_Rot = my::Quaternion::Identity();
	}
}

void CMainFrame::ClearAllComponents()
{
	m_Root.ClearAllComponents();
	m_cmps.clear();
	m_selcmps.clear();
}

void CMainFrame::OnDestroy()
{
	CFrameWndEx::OnDestroy();

	// TODO: Add your message handler code here
	ClearAllComponents();
	m_emitter->ReleaseResource();
	m_emitter.reset();
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
	ClearAllComponents();
	m_strPathName.Empty();
	InitialUpdateFrame(NULL, TRUE);
}

void CMainFrame::OnFileOpen()
{
	// TODO: Add your command handler code here
	CString strPathName;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);
	dlg.m_ofn.lpstrFile = strPathName.GetBuffer(_MAX_PATH);
	INT_PTR nResult = dlg.DoModal();
	strPathName.ReleaseBuffer();
	if (nResult == IDCANCEL)
	{
		return;
	}

	ClearAllComponents();
	m_strPathName = strPathName;
	std::basic_ifstream<char> ifs(m_strPathName);
	boost::archive::xml_iarchive ia(ifs);
	ia >> boost::serialization::make_nvp("level", m_cmps);
	ComponentPtrList::iterator cmp_iter = m_cmps.begin();
	for (; cmp_iter != m_cmps.end(); cmp_iter++)
	{
		m_Root.AddComponent(cmp_iter->get(), (*cmp_iter)->m_aabb.transform((*cmp_iter)->m_World), 0.1f);
	}
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
		if (nResult == IDCANCEL)
		{
			return;
		}
	}

	std::basic_ofstream<char> ofs(m_strPathName);
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("level", m_cmps);
}

void CMainFrame::OnComponentMesh()
{
	// TODO: Add your command handler code here
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (IDOK == dlg.DoModal())
	{
		CString strPathName = dlg.GetPathName();
		my::OgreMeshPtr mesh = theApp.LoadMesh(ts2ms((LPCTSTR)strPathName));
		if (mesh)
		{
			MeshComponentPtr mesh_cmp(new MeshComponent(mesh->m_aabb, my::Matrix4::Identity(), false));
			mesh_cmp->m_MeshRes.m_Path = ts2ms((LPCTSTR)strPathName);
			mesh_cmp->m_MeshRes.OnReady(mesh);
			for (unsigned int i = 0; i < mesh->m_MaterialNameList.size(); i++)
			{
				MaterialPtr lambert1(new Material());
				lambert1->m_Shader = "lambert1.fx";
				lambert1->m_PassMask = RenderPipeline::PassMaskOpaque;
				lambert1->m_MeshTexture.m_Path = "texture/Checker.bmp";
				lambert1->m_NormalTexture.m_Path = "texture/Normal.dds";
				lambert1->m_SpecularTexture.m_Path = "texture/White.dds";
				mesh_cmp->m_MaterialList.push_back(lambert1);
			}
			mesh_cmp->RequestResource();
			m_Root.AddComponent(mesh_cmp.get(), mesh_cmp->m_aabb.transform(mesh_cmp->m_World), 0.1f);
			m_cmps.push_back(mesh_cmp);
		}
		else
		{
			MessageBox(strPathName, _T("Load Mesh Error"), MB_OK | MB_ICONERROR);
		}
	}
}

void CMainFrame::OnComponentEmitter()
{
	// TODO: Add your command handler code here
	EmitterComponentPtr emit_cmp(new EmitterComponent(my::AABB(-10,10), my::Matrix4::Identity()));
	my::EmitterPtr emit(new my::Emitter());
	emit->Spawn(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector4(1,1,1,1), my::Vector2(10,10), 0.0f);
	emit_cmp->m_Emitter = emit;
	MaterialPtr particle1(new Material());
	particle1->m_Shader = "particle1.fx";
	particle1->m_PassMask = RenderPipeline::PassMaskTransparent;
	particle1->m_MeshTexture.m_Path = "texture/flare.dds";
	emit_cmp->m_Material = particle1;
	emit_cmp->RequestResource();
	m_Root.AddComponent(emit_cmp.get(), emit_cmp->m_aabb.transform(emit_cmp->m_World), 0.1f);
	m_cmps.push_back(emit_cmp);
}

void CMainFrame::OnComponentSphericalemitter()
{
	// TODO: Add your command handler code here
	EmitterComponentPtr emit_cmp(new EmitterComponent(my::AABB(-10,10), my::Matrix4::Identity()));
	my::SphericalEmitterPtr emit(new my::SphericalEmitter());
	emit->m_ParticleLifeTime=10.0f;
	emit->m_SpawnInterval=1/100.0f;
	emit->m_SpawnSpeed=5;
	emit->m_SpawnInclination.AddNode(0,D3DXToRadian(45),0,0);
	float Azimuth=D3DXToRadian(360)*8;
	emit->m_SpawnAzimuth.AddNode(0,0,Azimuth/10,Azimuth/10);
	emit->m_SpawnAzimuth.AddNode(10,Azimuth,Azimuth/10,Azimuth/10);
	emit->m_SpawnColorA.AddNode(0,1,0,0);
	emit->m_SpawnColorA.AddNode(10,0,0,0);
	emit->m_SpawnColorR.AddNode(0,1,0,0);
	emit->m_SpawnColorR.AddNode(10,0,0,0);
	emit->m_SpawnColorG.AddNode(0,1,0,0);
	emit->m_SpawnColorG.AddNode(10,0,0,0);
	emit->m_SpawnColorB.AddNode(0,1,0,0);
	emit->m_SpawnColorB.AddNode(10,0,0,0);
	emit->m_SpawnSizeX.AddNode(0,1,0,0);
	emit->m_SpawnSizeX.AddNode(10,10,0,0);
	emit->m_SpawnSizeY.AddNode(0,1,0,0);
	emit->m_SpawnSizeY.AddNode(10,10,0,0);
	emit_cmp->m_Emitter = emit;
	MaterialPtr particle1(new Material());
	particle1->m_Shader = "particle1.fx";
	particle1->m_PassMask = RenderPipeline::PassMaskTransparent;
	particle1->m_MeshTexture.m_Path = "texture/flare.dds";
	emit_cmp->m_Material = particle1;
	emit_cmp->RequestResource();
	m_Root.AddComponent(emit_cmp.get(), emit_cmp->m_aabb.transform(emit_cmp->m_World), 0.1f);
	m_cmps.push_back(emit_cmp);
}

void CMainFrame::OnEditDelete()
{
	// TODO: Add your command handler code here
	ComponentSet::iterator cmp_iter = m_selcmps.begin();
	for (; cmp_iter != m_selcmps.end(); cmp_iter++)
	{
		m_Root.RemoveComponent(*cmp_iter);
		ComponentPtrList::iterator cmp_ptr_iter = m_cmps.begin();
		for (; cmp_ptr_iter != m_cmps.end(); cmp_ptr_iter++)
		{
			if (cmp_ptr_iter->get() == *cmp_iter)
			{
				m_cmps.erase(cmp_ptr_iter);
				break;
			}
		}
	}
	m_selcmps.clear();
	EventArg arg;
	m_EventSelectionChanged(&arg);
}

void CMainFrame::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_selcmps.empty());
}

void CMainFrame::OnPivotMove()
{
	// TODO: Add your command handler code here
	m_Pivot.m_Mode = Pivot::PivotModeMove;
	UpdatePivotTransform();
	EventArg arg;
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
	UpdatePivotTransform();
	EventArg arg;
	m_EventPivotModeChanged(&arg);
}

void CMainFrame::OnUpdatePivotRotate(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_Pivot.m_Mode == Pivot::PivotModeRot);
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (!m_selcmps.empty())
	{
		ComponentSet::iterator cmp_iter = m_selcmps.begin();
		for (; cmp_iter != m_selcmps.end(); cmp_iter++)
		{
			(*cmp_iter)->Update(0.033f);
		}
		EventArg arg;
		m_EventSelectionPlaying(&arg);
	}

	__super::OnTimer(nIDEvent);
}

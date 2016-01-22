
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "CtrlProps.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	//ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	//ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	//ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	//ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	//ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	//ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	//ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	//m_wndObjectCombo.GetWindowRect(&rectCombo);

	int cyCmb = 0;//rectCombo.Size().cy;
	int cyTlb = 0;//m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CPropertiesWnd::OnSelectionChanged(EventArg * arg)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (!pFrame->m_selcmps.empty())
	{
		UpdateProperties(*pFrame->m_selcmps.begin());
	}
	else
	{
	}
}

void CPropertiesWnd::UpdateProperties(Component * cmp)
{
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		UpdatePropertiesMesh(dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeEmitter:
		break;
	case Component::ComponentTypeTerrain:
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesMesh(MeshComponent * cmp)
{
}

void CPropertiesWnd::CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName)
{
	CMFCPropertyGridProperty * pSpline = new CSimpleProp(lpszName, 0, TRUE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)0, NULL, 0);
	pSpline->AddSubItem(pProp);
	CreatePropertiesSplineNode(pSpline);
	CreatePropertiesSplineNode(pSpline);
	CreatePropertiesSplineNode(pSpline);
	pParentProp->AddSubItem(pSpline);
}

void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline)
{
	CMFCPropertyGridProperty * pNode = new CSimpleProp(_T("Node"), 0, TRUE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k0"), (_variant_t)0.0f, NULL, 0);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k"), (_variant_t)0.0f, NULL, 0);
	pNode->AddSubItem(pProp);
	pSpline->AddSubItem(pNode);
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName)
{
	CMFCPropertyGridProperty * pMaterial = new CSimpleProp(lpszName, 0, TRUE);
	pParentProp->AddSubItem(pMaterial);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	//{
	//	TRACE0("Failed to create Properties Combo \n");
	//	return -1;      // fail to create
	//}

	//m_wndObjectCombo.AddString(_T("Application"));
	//m_wndObjectCombo.AddString(_T("Properties Window"));
	//m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
	//m_wndObjectCombo.SetCurSel(0);

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	SetPropListFont();

	InitPropList();

	//m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	//m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	//m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	//m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	//m_wndToolBar.SetOwner(this);

	//// All commands will be routed via this control , not via the parent frame:
	//m_wndToolBar.SetRouteCommandsViaFrame(FALSE);
	CMainFrame::getSingleton().m_EventSelectionChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	// TODO: Add your message handler code here
	CMainFrame::getSingleton().m_EventSelectionChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}
//
//void CPropertiesWnd::OnExpandAllProperties()
//{
//	m_wndPropList.ExpandAll();
//}
//
//void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
//{
//}
//
//void CPropertiesWnd::OnSortProperties()
//{
//	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
//}
//
//void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
//{
//	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
//}
//
//void CPropertiesWnd::OnProperties1()
//{
//	// TODO: Add your command handler code here
//}
//
//void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
//{
//	// TODO: Add your command update UI handler code here
//}
//
//void CPropertiesWnd::OnProperties2()
//{
//	// TODO: Add your command handler code here
//}
//
//void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
//{
//	// TODO: Add your command update UI handler code here
//}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	CMFCPropertyGridProperty * pComponent = new CMFCPropertyGridProperty(_T("Component"));
	CMFCPropertyGridProperty * pAABB = new CSimpleProp(_T("AABB"), 0, TRUE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("min.x"), (_variant_t)0.0f, NULL, 0);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("miny"), (_variant_t)0.0f, NULL, 0);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("minz"), (_variant_t)0.0f, NULL, 0);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxx"), (_variant_t)0.0f, NULL, 0);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxy"), (_variant_t)0.0f, NULL, 0);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxz"), (_variant_t)0.0f, NULL, 0);
	pAABB->AddSubItem(pProp);
	pComponent->AddSubItem(pAABB);

	CMFCPropertyGridProperty * pPosition = new CSimpleProp(_T("Position"), 0, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, 0);
	pPosition->AddSubItem(pProp);
	pComponent->AddSubItem(pPosition);

	CMFCPropertyGridProperty * pRotate = new CSimpleProp(_T("Rotate"), 0, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, 0);
	pRotate->AddSubItem(pProp);
	pComponent->AddSubItem(pRotate);

	CMFCPropertyGridProperty * pScale = new CSimpleProp(_T("Scale"), 0, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, 0);
	pScale->AddSubItem(pProp);
	pComponent->AddSubItem(pScale);
	m_wndPropList.AddProperty(pComponent, TRUE, TRUE);

	CMFCPropertyGridProperty * pMesh = new CMFCPropertyGridProperty(_T("Mesh"));
	pProp = new CSimpleProp(_T("ResPath"), (_variant_t)"", NULL, 0);
	pMesh->AddSubItem(pProp);
	m_wndPropList.AddProperty(pMesh, TRUE, TRUE);

	CMFCPropertyGridProperty * pEmitter = new CMFCPropertyGridProperty(_T("Emitter"));
	pProp = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)0.0f, NULL, 0);
	pEmitter->AddSubItem(pProp);
	m_wndPropList.AddProperty(pEmitter, TRUE, TRUE);

	CMFCPropertyGridProperty * pSphericalEmitter = new CMFCPropertyGridProperty(_T("SphericalEmitter"));
	pProp = new CSimpleProp(_T("SpawnInterval"), (_variant_t)0.0f, NULL, 0);
	pSphericalEmitter->AddSubItem(pProp);
	CMFCPropertyGridProperty * pHalfSpawnArea = new CSimpleProp(_T("HalfSpawnArea"), 0, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, 0);
	pHalfSpawnArea->AddSubItem(pProp);
	pSphericalEmitter->AddSubItem(pHalfSpawnArea);
	pProp = new CSimpleProp(_T("SpawnSpeed"), (_variant_t)0.0f, NULL, 0);
	pSphericalEmitter->AddSubItem(pProp);
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnInclination"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnAzimuth"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnColorA"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnColorR"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnColorG"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnColorB"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnSizeX"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnSizeY"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnAngle"));
	CreatePropertiesSpline(pSphericalEmitter, _T("SpawnLoopTime"));
	m_wndPropList.AddProperty(pSphericalEmitter, TRUE, TRUE);

	CMFCPropertyGridProperty * pTerrain = new CMFCPropertyGridProperty(_T("Terrain"));
	CMFCPropertyGridProperty * pTexStart = new CSimpleProp(_T("TexStart"), 0, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pTexStart->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pTexStart->AddSubItem(pProp);
	pTerrain->AddSubItem(pTexStart);
	CMFCPropertyGridProperty * pTexEnd = new CSimpleProp(_T("TexEnd"), 0, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, 0);
	pTexEnd->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, 0);
	pTexEnd->AddSubItem(pProp);
	pTerrain->AddSubItem(pTexEnd);
	pProp = new CSimpleProp(_T("XDivision"), (_variant_t)0.0f, NULL, 0);
	pTerrain->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ZDivision"), (_variant_t)0.0f, NULL, 0);
	pTerrain->AddSubItem(pProp);
	m_wndPropList.AddProperty(pTerrain, TRUE, TRUE);

	CMFCPropertyGridProperty * pMaterial = new CMFCPropertyGridProperty(_T("Material"));
	CreatePropertiesMaterial(pMaterial, _T("mat0"));
	CreatePropertiesMaterial(pMaterial, _T("mat1"));
	CreatePropertiesMaterial(pMaterial, _T("mat2"));
	m_wndPropList.AddProperty(pMaterial, TRUE, TRUE);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

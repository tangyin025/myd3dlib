
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
	memset(&m_pProp, 0, sizeof(m_pProp));
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
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
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
	m_pProp[PropertyComponent]->Show(TRUE, FALSE);
	m_pProp[PropertyComponentMinX]->SetValue((_variant_t)cmp->m_aabb.m_min.x);
	m_pProp[PropertyComponentMinY]->SetValue((_variant_t)cmp->m_aabb.m_min.y);
	m_pProp[PropertyComponentMinZ]->SetValue((_variant_t)cmp->m_aabb.m_min.z);
	m_pProp[PropertyComponentMaxX]->SetValue((_variant_t)cmp->m_aabb.m_max.x);
	m_pProp[PropertyComponentMaxY]->SetValue((_variant_t)cmp->m_aabb.m_max.y);
	m_pProp[PropertyComponentMaxZ]->SetValue((_variant_t)cmp->m_aabb.m_max.z);
	my::Vector3 trans, scale; my::Quaternion rot;
	cmp->m_World.Decompose(scale, rot, trans);
	m_pProp[PropertyComponentPosX]->SetValue((_variant_t)trans.x);
	m_pProp[PropertyComponentPosY]->SetValue((_variant_t)trans.y);
	m_pProp[PropertyComponentPosZ]->SetValue((_variant_t)trans.z);
	my::Vector3 euler = rot.ToEulerAngles();
	m_pProp[PropertyComponentRotX]->SetValue((_variant_t)D3DXToDegree(euler.x));
	m_pProp[PropertyComponentRotY]->SetValue((_variant_t)D3DXToDegree(euler.y));
	m_pProp[PropertyComponentRotZ]->SetValue((_variant_t)D3DXToDegree(euler.z));
	m_pProp[PropertyComponentScaleX]->SetValue((_variant_t)scale.x);
	m_pProp[PropertyComponentScaleY]->SetValue((_variant_t)scale.y);
	m_pProp[PropertyComponentScaleZ]->SetValue((_variant_t)scale.z);

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
	m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::UpdatePropertiesMesh(MeshComponent * cmp)
{
	m_pProp[PropertyMesh]->Show(TRUE, FALSE);
	m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
	m_pProp[PropertySphericalEmitter]->Show(FALSE, FALSE);
	m_pProp[PropertyTerrain]->Show(FALSE, FALSE);

	m_pProp[PropertyMeshResourcePath]->SetValue((_variant_t)cmp->m_MeshRes.m_ResPath.c_str());

	for (unsigned int i = 0; i < (PropertyMaterialEnd - PropertyMaterial0); i++)
	{
		if (i < cmp->m_MaterialList.size() && cmp->m_MaterialList[i])
		{
			UpdatePropertiesMaterial(cmp->m_MaterialList[i].get(), (Property)(PropertyMaterial0 + i));
		}
		else
		{
			m_pProp[PropertyMaterial0 + i]->Show(FALSE, FALSE);
		}
	}
}

void CPropertiesWnd::UpdatePropertiesMaterial(Material * mat, Property PropertyId)
{
	_ASSERT(PropertyId >= PropertyMaterial0 && PropertyId < PropertyMaterialEnd);
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialShader - PropertyMaterialShader)->SetValue((_variant_t)mat->m_Shader.c_str());
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialPassMask - PropertyMaterialShader)->SetValue((_variant_t)mat->m_PassMask);
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialMeshTexture - PropertyMaterialShader)->SetValue((_variant_t)mat->m_MeshTexture.m_ResPath.c_str());
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialNormalTexture - PropertyMaterialShader)->SetValue((_variant_t)mat->m_NormalTexture.m_ResPath.c_str());
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialSpecularTexture - PropertyMaterialShader)->SetValue((_variant_t)mat->m_SpecularTexture.m_ResPath.c_str());
}

void CPropertiesWnd::CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId)
{
	_ASSERT(PropertyId >= PropertySphericalEmitterSpawnInclination && PropertyId <= PropertySphericalEmitterSpawnLoopTime);
	m_pProp[PropertyId] = new CSimpleProp(lpszName, PropertyId, TRUE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)0, NULL, PropertySplineNodeCount);
	m_pProp[PropertyId]->AddSubItem(pProp);
	CreatePropertiesSplineNode(m_pProp[PropertyId], 0);
	CreatePropertiesSplineNode(m_pProp[PropertyId], 1);
	CreatePropertiesSplineNode(m_pProp[PropertyId], 2);
	pParentProp->AddSubItem(m_pProp[PropertyId]);
}

void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId)
{
	CMFCPropertyGridProperty * pNode = new CSimpleProp(_T("Node"), NodeId, TRUE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertySplineNodeX);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertySplineNodeY);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k0"), (_variant_t)0.0f, NULL, PropertySplineNodeK0);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k"), (_variant_t)0.0f, NULL, PropertySplineNodeK);
	pNode->AddSubItem(pProp);
	pSpline->AddSubItem(pNode);
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridCtrl * pParentCtrl, LPCTSTR lpszName, Property PropertyId)
{
	_ASSERT(PropertyId >= PropertyMaterial0 && PropertyId < PropertyMaterialEnd);
	m_pProp[PropertyId] = new CMFCPropertyGridProperty(lpszName, PropertyId, FALSE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Shader"), (_variant_t)_T(""), NULL, PropertyMaterialShader);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("PassMask"), (_variant_t)0u, NULL, PropertyMaterialPassMask);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("MeshTexture"), (_variant_t)_T(""), NULL, PropertyMaterialMeshTexture);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("NormalTexture"), (_variant_t)_T(""), NULL, PropertyMaterialNormalTexture);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("SpecularTexture"), (_variant_t)_T(""), NULL, PropertyMaterialSpecularTexture);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pParentCtrl->AddProperty(m_pProp[PropertyId], TRUE, TRUE);
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

	m_pProp[PropertyComponent] = new CMFCPropertyGridProperty(_T("Component"), PropertyComponent, FALSE);
	CMFCPropertyGridProperty * pAABB = new CSimpleProp(_T("AABB"), 0, TRUE);
	m_pProp[PropertyComponentMinX] = new CSimpleProp(_T("minx"), (_variant_t)0.0f, NULL, PropertyComponentMinX);
	pAABB->AddSubItem(m_pProp[PropertyComponentMinX]);
	m_pProp[PropertyComponentMinY] = new CSimpleProp(_T("miny"), (_variant_t)0.0f, NULL, PropertyComponentMinY);
	pAABB->AddSubItem(m_pProp[PropertyComponentMinY]);
	m_pProp[PropertyComponentMinZ] = new CSimpleProp(_T("minz"), (_variant_t)0.0f, NULL, PropertyComponentMinZ);
	pAABB->AddSubItem(m_pProp[PropertyComponentMinZ]);
	m_pProp[PropertyComponentMaxX] = new CSimpleProp(_T("maxx"), (_variant_t)0.0f, NULL, PropertyComponentMaxX);
	pAABB->AddSubItem(m_pProp[PropertyComponentMaxX]);
	m_pProp[PropertyComponentMaxY] = new CSimpleProp(_T("maxy"), (_variant_t)0.0f, NULL, PropertyComponentMaxY);
	pAABB->AddSubItem(m_pProp[PropertyComponentMaxY]);
	m_pProp[PropertyComponentMaxZ] = new CSimpleProp(_T("maxz"), (_variant_t)0.0f, NULL, PropertyComponentMaxZ);
	pAABB->AddSubItem(m_pProp[PropertyComponentMaxZ]);
	m_pProp[PropertyComponent]->AddSubItem(pAABB);

	CMFCPropertyGridProperty * pPosition = new CSimpleProp(_T("Position"), 0, TRUE);
	m_pProp[PropertyComponentPosX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyComponentPosX);
	pPosition->AddSubItem(m_pProp[PropertyComponentPosX]);
	m_pProp[PropertyComponentPosY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyComponentPosY);
	pPosition->AddSubItem(m_pProp[PropertyComponentPosY]);
	m_pProp[PropertyComponentPosZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyComponentPosZ);
	pPosition->AddSubItem(m_pProp[PropertyComponentPosZ]);
	m_pProp[PropertyComponent]->AddSubItem(pPosition);

	CMFCPropertyGridProperty * pRotate = new CSimpleProp(_T("Rotate"), 0, TRUE);
	m_pProp[PropertyComponentRotX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyComponentRotX);
	pRotate->AddSubItem(m_pProp[PropertyComponentRotX]);
	m_pProp[PropertyComponentRotY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyComponentRotY);
	pRotate->AddSubItem(m_pProp[PropertyComponentRotY]);
	m_pProp[PropertyComponentRotZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyComponentRotZ);
	pRotate->AddSubItem(m_pProp[PropertyComponentRotZ]);
	m_pProp[PropertyComponent]->AddSubItem(pRotate);

	CMFCPropertyGridProperty * pScale = new CSimpleProp(_T("Scale"), 0, TRUE);
	m_pProp[PropertyComponentScaleX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyComponentScaleX);
	pScale->AddSubItem(m_pProp[PropertyComponentScaleX]);
	m_pProp[PropertyComponentScaleY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyComponentScaleY);
	pScale->AddSubItem(m_pProp[PropertyComponentScaleY]);
	m_pProp[PropertyComponentScaleZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyComponentScaleZ);
	pScale->AddSubItem(m_pProp[PropertyComponentScaleZ]);
	m_pProp[PropertyComponent]->AddSubItem(pScale);
	m_wndPropList.AddProperty(m_pProp[PropertyComponent], TRUE, TRUE);

	m_pProp[PropertyMesh] = new CMFCPropertyGridProperty(_T("Mesh"), PropertyMesh, FALSE);
	m_pProp[PropertyMeshResourcePath] = new CSimpleProp(_T("ResourcePath"), (_variant_t)"", NULL, 0);
	m_pProp[PropertyMesh]->AddSubItem(m_pProp[PropertyMeshResourcePath]);
	m_wndPropList.AddProperty(m_pProp[PropertyMesh], TRUE, TRUE);

	m_pProp[PropertyEmitter] = new CMFCPropertyGridProperty(_T("Emitter"), PropertyEmitter, FALSE);
	m_pProp[PropertyEmitterParticleLifeTime] = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)0.0f, NULL, PropertyEmitterParticleLifeTime);
	m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertyEmitterParticleLifeTime]);
	m_wndPropList.AddProperty(m_pProp[PropertyEmitter], TRUE, TRUE);

	m_pProp[PropertySphericalEmitter] = new CMFCPropertyGridProperty(_T("SphericalEmitter"), PropertySphericalEmitter, FALSE);
	m_pProp[PropertySphericalEmitterSpawnInterval] = new CSimpleProp(_T("SpawnInterval"), (_variant_t)0.0f, NULL, PropertySphericalEmitterSpawnInterval);
	m_pProp[PropertySphericalEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterSpawnInterval]);
	CMFCPropertyGridProperty * pHalfSpawnArea = new CSimpleProp(_T("HalfSpawnArea"), 0, TRUE);
	m_pProp[PropertySphericalEmitterHalfSpawnAreaX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertySphericalEmitterHalfSpawnAreaX);
	pHalfSpawnArea->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnAreaX]);
	m_pProp[PropertySphericalEmitterHalfSpawnAreaY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertySphericalEmitterHalfSpawnAreaY);
	pHalfSpawnArea->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnAreaY]);
	m_pProp[PropertySphericalEmitterHalfSpawnAreaZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertySphericalEmitterHalfSpawnAreaZ);
	pHalfSpawnArea->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnAreaZ]);
	m_pProp[PropertySphericalEmitter]->AddSubItem(pHalfSpawnArea);
	m_pProp[PropertySphericalEmitterSpawnSpeed] = new CSimpleProp(_T("SpawnSpeed"), (_variant_t)0.0f, NULL, PropertySphericalEmitterSpawnSpeed);
	m_pProp[PropertySphericalEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterSpawnSpeed]);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnInclination"), PropertySphericalEmitterSpawnInclination);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnAzimuth"), PropertySphericalEmitterSpawnAzimuth);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnColorA"), PropertySphericalEmitterSpawnColorA);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnColorR"), PropertySphericalEmitterSpawnColorR);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnColorG"), PropertySphericalEmitterSpawnColorG);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnColorB"), PropertySphericalEmitterSpawnColorB);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnSizeX"), PropertySphericalEmitterSpawnSizeX);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnSizeY"), PropertySphericalEmitterSpawnSizeY);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnAngle"), PropertySphericalEmitterSpawnAngle);
	CreatePropertiesSpline(m_pProp[PropertySphericalEmitter], _T("SpawnLoopTime"), PropertySphericalEmitterSpawnLoopTime);
	m_wndPropList.AddProperty(m_pProp[PropertySphericalEmitter], TRUE, TRUE);

	m_pProp[PropertyTerrain] = new CMFCPropertyGridProperty(_T("Terrain"), PropertyTerrain, FALSE);
	CMFCPropertyGridProperty * pTexStart = new CSimpleProp(_T("TexStart"), 0, TRUE);
	m_pProp[PropertyTerrainTexStartX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyTerrainTexStartX);
	pTexStart->AddSubItem(m_pProp[PropertyTerrainTexStartX]);
	m_pProp[PropertyTerrainTexStartY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyTerrainTexStartY);
	pTexStart->AddSubItem(m_pProp[PropertyTerrainTexStartY]);
	m_pProp[PropertyTerrain]->AddSubItem(pTexStart);
	CMFCPropertyGridProperty * pTexEnd = new CSimpleProp(_T("TexEnd"), 0, TRUE);
	m_pProp[PropertyTerrainTexEndX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyTerrainTexEndX);
	pTexEnd->AddSubItem(m_pProp[PropertyTerrainTexEndX]);
	m_pProp[PropertyTerrainTexEndY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyTerrainTexEndY);
	pTexEnd->AddSubItem(m_pProp[PropertyTerrainTexEndY]);
	m_pProp[PropertyTerrain]->AddSubItem(pTexEnd);
	m_pProp[PropertyTerrainXDivision] = new CSimpleProp(_T("XDivision"), (_variant_t)0.0f, NULL, PropertyTerrainXDivision);
	m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainXDivision]);
	m_pProp[PropertyTerrainZDivision] = new CSimpleProp(_T("ZDivision"), (_variant_t)0.0f, NULL, PropertyTerrainZDivision);
	m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainZDivision]);
	m_wndPropList.AddProperty(m_pProp[PropertyTerrain], TRUE, TRUE);

	for (unsigned int i = 0; i < (PropertyMaterialEnd - PropertyMaterial0); i++)
	{
		TCHAR buff[128];
		_stprintf_s(buff, _countof(buff), _T("Material%u"), i);
		CreatePropertiesMaterial(&m_wndPropList, buff, (Property)(PropertyMaterial0 + i));
	}
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

afx_msg LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	return 0;
}


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

struct PassMaskDesc
{
	LPCTSTR desc;
	DWORD mask;
};

static const PassMaskDesc g_PassMaskDesc[4] =
{
	{_T("None"), RenderPipeline::PassMaskNone},
	{_T("Light"), RenderPipeline::PassMaskLight},
	{_T("Opaque"), RenderPipeline::PassMaskOpaque},
	{_T("Transparent"), RenderPipeline::PassMaskTransparent},
};

static LPCTSTR GetPassMaskDesc(DWORD mask)
{
	for (unsigned int i = 0; i < _countof(g_PassMaskDesc); i++)
	{
		if (g_PassMaskDesc[i].mask == mask)
		{
			return g_PassMaskDesc[i].desc;
		}
	}
	return g_PassMaskDesc[0].desc;
}

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
		//HideAllProperties();
	}
}

void CPropertiesWnd::OnCmpAttriChanged(EventArg * arg)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (!pFrame->m_selcmps.empty())
	{
		UpdateProperties(*pFrame->m_selcmps.begin());
	}
}

void CPropertiesWnd::HideAllProperties(void)
{
	m_pProp[PropertyComponent]->Show(FALSE, FALSE);
	m_pProp[PropertyMesh]->Show(FALSE, FALSE);
	m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
	m_pProp[PropertySphericalEmitter]->Show(FALSE, FALSE);
	for (unsigned int i = 0; i < (PropertyMaterialEnd - PropertyMaterial0); i++)
	{
		m_pProp[PropertyMaterial0 + i]->Show(FALSE, FALSE);
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
		UpdatePropertiesEmitter(dynamic_cast<EmitterComponent *>(cmp));
		break;
	}
	m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::UpdatePropertiesMesh(MeshComponent * cmp)
{
	m_pProp[PropertyMesh]->Show(TRUE, FALSE);
	m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
	m_pProp[PropertySphericalEmitter]->Show(FALSE, FALSE);

	m_pProp[PropertyMeshPath]->SetValue((_variant_t)cmp->m_MeshRes.m_Path.c_str());

	for (unsigned int i = 0; i < (PropertyMaterialEnd - PropertyMaterial0); i++)
	{
		if (i < cmp->m_MaterialList.size() && cmp->m_MaterialList[i])
		{
			UpdatePropertiesMaterial((Property)(PropertyMaterial0 + i), cmp->m_MaterialList[i].get());
		}
		else
		{
			m_pProp[PropertyMaterial0 + i]->Show(FALSE, FALSE);
		}
	}
}

void CPropertiesWnd::UpdatePropertiesEmitter(EmitterComponent * cmp)
{
	m_pProp[PropertyMesh]->Show(FALSE, FALSE);
	m_pProp[PropertyEmitter]->Show(TRUE, FALSE);

	m_pProp[PropertyEmitterParticleLifeTime]->SetValue((_variant_t)cmp->m_Emitter->m_ParticleLifeTime);

	my::SphericalEmitterPtr spherical_emit = boost::dynamic_pointer_cast<my::SphericalEmitter>(cmp->m_Emitter);
	if (spherical_emit)
	{
		m_pProp[PropertySphericalEmitter]->Show(TRUE, FALSE);
		m_pProp[PropertySphericalEmitterSpawnInterval]->SetValue((_variant_t)spherical_emit->m_SpawnInterval);
		m_pProp[PropertySphericalEmitterHalfSpawnAreaX]->SetValue((_variant_t)spherical_emit->m_HalfSpawnArea.x);
		m_pProp[PropertySphericalEmitterHalfSpawnAreaY]->SetValue((_variant_t)spherical_emit->m_HalfSpawnArea.y);
		m_pProp[PropertySphericalEmitterHalfSpawnAreaZ]->SetValue((_variant_t)spherical_emit->m_HalfSpawnArea.z);
		m_pProp[PropertySphericalEmitterSpawnSpeed]->SetValue((_variant_t)spherical_emit->m_SpawnSpeed);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnInclination, &spherical_emit->m_SpawnInclination);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnAzimuth, &spherical_emit->m_SpawnAzimuth);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorA, &spherical_emit->m_SpawnColorA);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorR, &spherical_emit->m_SpawnColorR);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorG, &spherical_emit->m_SpawnColorG);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorB, &spherical_emit->m_SpawnColorB);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnSizeX, &spherical_emit->m_SpawnSizeX);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnSizeY, &spherical_emit->m_SpawnSizeY);
		UpdatePropertiesSpline(PropertySphericalEmitterSpawnAngle, &spherical_emit->m_SpawnAngle);
		m_pProp[PropertySphericalEmitterSpawnLoopTime]->SetValue((_variant_t)spherical_emit->m_SpawnLoopTime);
	}
	else
	{
		m_pProp[PropertySphericalEmitter]->Show(FALSE, FALSE);
	}

	for (unsigned int i = 0; i < (PropertyMaterialEnd - PropertyMaterial0); i++)
	{
		if (i == 0)
		{
			UpdatePropertiesMaterial(PropertyMaterial0, cmp->m_Material.get());
		}
		else
		{
			m_pProp[PropertyMaterial0 + i]->Show(FALSE, FALSE);
		}
	}
}

void CPropertiesWnd::UpdatePropertiesMaterial(Property PropertyId, Material * mat)
{
	_ASSERT(PropertyId >= PropertyMaterial0 && PropertyId < PropertyMaterialEnd);
	m_pProp[PropertyId]->Show(TRUE, FALSE);
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialShader - PropertyMaterialShader)->SetValue((_variant_t)mat->m_Shader.c_str());
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialPassMask - PropertyMaterialShader)->SetValue((_variant_t)GetPassMaskDesc(mat->m_PassMask));
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialMeshTexture - PropertyMaterialShader)->SetValue((_variant_t)mat->m_MeshTexture.m_Path.c_str());
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialNormalTexture - PropertyMaterialShader)->SetValue((_variant_t)mat->m_NormalTexture.m_Path.c_str());
	m_pProp[PropertyId]->GetSubItem(PropertyMaterialSpecularTexture - PropertyMaterialShader)->SetValue((_variant_t)mat->m_SpecularTexture.m_Path.c_str());
}

void CPropertiesWnd::UpdatePropertiesSpline(Property PropertyId, my::Spline * spline)
{
	_ASSERT(PropertyId >= PropertySphericalEmitterSpawnInclination && PropertyId <= PropertySphericalEmitterSpawnAngle);
	m_pProp[PropertyId]->GetSubItem(0)->SetValue((_variant_t)spline->size());
	unsigned int i = 0;
	for (; i < spline->size(); i++)
	{
		if ((unsigned int)m_pProp[PropertyId]->GetSubItemsCount() <= i + 1)
		{
			CreatePropertiesSplineNode(m_pProp[PropertyId], i);
		}
		UpdatePropertiesSplineNode(m_pProp[PropertyId], i, &(*spline)[i]);
	}
	while ((unsigned int)m_pProp[PropertyId]->GetSubItemsCount() > i + 1)
	{
		CMFCPropertyGridProperty * pProp = m_pProp[PropertyId]->GetSubItem(i + 1);
		m_pProp[PropertyId]->RemoveSubItem(pProp, TRUE);
	}
}

void CPropertiesWnd::UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId, const my::SplineNode * node)
{
	CMFCPropertyGridProperty * pProp = pSpline->GetSubItem(NodeId + 1);
	_ASSERT(pProp);
	pProp->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->SetValue((_variant_t)node->x);
	pProp->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->SetValue((_variant_t)node->y);
	pProp->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->SetValue((_variant_t)node->k0);
	pProp->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->SetValue((_variant_t)node->k);
}

void CPropertiesWnd::CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId)
{
	_ASSERT(PropertyId >= PropertySphericalEmitterSpawnInclination && PropertyId <= PropertySphericalEmitterSpawnAngle);
	m_pProp[PropertyId] = new CSimpleProp(lpszName, PropertyId, TRUE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)(size_t)0, NULL, PropertySplineNodeCount);
	m_pProp[PropertyId]->AddSubItem(pProp);
	CreatePropertiesSplineNode(m_pProp[PropertyId], 0);
	CreatePropertiesSplineNode(m_pProp[PropertyId], 1);
	CreatePropertiesSplineNode(m_pProp[PropertyId], 2);
	pParentProp->AddSubItem(m_pProp[PropertyId]);
}

void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId)
{
	CMFCPropertyGridProperty * pNode = new CSimpleProp(_T("Node"), NodeId, TRUE);
	pSpline->AddSubItem(pNode);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertySplineNodeX);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertySplineNodeY);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k0"), (_variant_t)0.0f, NULL, PropertySplineNodeK0);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k"), (_variant_t)0.0f, NULL, PropertySplineNodeK);
	pNode->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridCtrl * pParentCtrl, LPCTSTR lpszName, Property PropertyId)
{
	_ASSERT(PropertyId >= PropertyMaterial0 && PropertyId < PropertyMaterialEnd);
	m_pProp[PropertyId] = new CMFCPropertyGridProperty(lpszName, PropertyId, FALSE);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Shader"), (_variant_t)_T(""), NULL, PropertyMaterialShader);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CComboProp(_T("PassMask"), (_variant_t)g_PassMaskDesc[0].desc, NULL, PropertyMaterialPassMask);
	for (unsigned int i = 0; i < _countof(g_PassMaskDesc); i++)
	{
		pProp->AddOption(g_PassMaskDesc[i].desc, FALSE);
	}
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CFileProp(_T("MeshTexture"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialMeshTexture);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CFileProp(_T("NormalTexture"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialNormalTexture);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pProp = new CFileProp(_T("SpecularTexture"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialSpecularTexture);
	m_pProp[PropertyId]->AddSubItem(pProp);
	pParentCtrl->AddProperty(m_pProp[PropertyId], FALSE, FALSE);
}

Material * CPropertiesWnd::GetComponentMaterial(Component * cmp, unsigned int id)
{
	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
			if (id < mesh_cmp->m_MaterialList.size())
			{
				return mesh_cmp->m_MaterialList[id].get();
			}
		}
		break;
	case Component::ComponentTypeEmitter:
		{
			EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
			if (id == 0)
			{
				return emit_cmp->m_Material.get();
			}
		}
		break;
	}
	return NULL;
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
	CMainFrame::getSingleton().m_EventCmpAttriChanged.connect(boost::bind(&CPropertiesWnd::OnCmpAttriChanged, this, _1));

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	// TODO: Add your message handler code here
	CMainFrame::getSingleton().m_EventSelectionChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
	CMainFrame::getSingleton().m_EventCmpAttriChanged.disconnect(boost::bind(&CPropertiesWnd::OnCmpAttriChanged, this, _1));
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
	m_wndPropList.AddProperty(m_pProp[PropertyComponent], FALSE, FALSE);

	m_pProp[PropertyMesh] = new CMFCPropertyGridProperty(_T("Mesh"), PropertyMesh, FALSE);
	m_pProp[PropertyMeshPath] = new CFileProp(_T("Path"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshPath);
	m_pProp[PropertyMesh]->AddSubItem(m_pProp[PropertyMeshPath]);
	m_wndPropList.AddProperty(m_pProp[PropertyMesh], FALSE, FALSE);

	m_pProp[PropertyEmitter] = new CMFCPropertyGridProperty(_T("Emitter"), PropertyEmitter, FALSE);
	m_pProp[PropertyEmitterParticleLifeTime] = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)0.0f, NULL, PropertyEmitterParticleLifeTime);
	m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertyEmitterParticleLifeTime]);
	m_wndPropList.AddProperty(m_pProp[PropertyEmitter], FALSE, FALSE);

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
	m_pProp[PropertySphericalEmitterSpawnLoopTime] = new CSimpleProp(_T("SpawnLoopTime"), (_variant_t)0.0f, NULL, PropertySphericalEmitterSpawnLoopTime);
	m_pProp[PropertySphericalEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterSpawnLoopTime]);
	m_wndPropList.AddProperty(m_pProp[PropertySphericalEmitter], FALSE, FALSE);

	for (unsigned int i = 0; i < (PropertyMaterialEnd - PropertyMaterial0); i++)
	{
		TCHAR buff[128];
		_stprintf_s(buff, _countof(buff), _T("Material%u"), i);
		CreatePropertiesMaterial(&m_wndPropList, buff, (Property)(PropertyMaterial0 + i));
	}

	//HideAllProperties();
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
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (pFrame->m_selcmps.empty())
	{
		return 0;
	}
	Component * cmp = *pFrame->m_selcmps.begin();

	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	DWORD PropertyId = pProp->GetData();
	switch (PropertyId)
	{
	case PropertyComponentMinX:
	case PropertyComponentMinY:
	case PropertyComponentMinZ:
	case PropertyComponentMaxX:
	case PropertyComponentMaxY:
	case PropertyComponentMaxZ:
	case PropertyComponentPosX:
	case PropertyComponentPosY:
	case PropertyComponentPosZ:
	case PropertyComponentRotX:
	case PropertyComponentRotY:
	case PropertyComponentRotZ:
	case PropertyComponentScaleX:
	case PropertyComponentScaleY:
	case PropertyComponentScaleZ:
		{
			cmp->m_aabb.m_min.x = m_pProp[PropertyComponentMinX]->GetValue().fltVal;
			cmp->m_aabb.m_min.y = m_pProp[PropertyComponentMinY]->GetValue().fltVal;
			cmp->m_aabb.m_min.z = m_pProp[PropertyComponentMinZ]->GetValue().fltVal;
			cmp->m_aabb.m_max.x = m_pProp[PropertyComponentMaxX]->GetValue().fltVal;
			cmp->m_aabb.m_max.y = m_pProp[PropertyComponentMaxY]->GetValue().fltVal;
			cmp->m_aabb.m_max.z = m_pProp[PropertyComponentMaxZ]->GetValue().fltVal;
			my::Vector3 trans(
				m_pProp[PropertyComponentPosX]->GetValue().fltVal,
				m_pProp[PropertyComponentPosY]->GetValue().fltVal,
				m_pProp[PropertyComponentPosZ]->GetValue().fltVal);
			my::Quaternion rot = my::Quaternion::RotationEulerAngles(my::Vector3(
				D3DXToRadian(m_pProp[PropertyComponentRotX]->GetValue().fltVal),
				D3DXToRadian(m_pProp[PropertyComponentRotY]->GetValue().fltVal),
				D3DXToRadian(m_pProp[PropertyComponentRotZ]->GetValue().fltVal)));
			my::Vector3 scale(
				m_pProp[PropertyComponentScaleX]->GetValue().fltVal,
				m_pProp[PropertyComponentScaleY]->GetValue().fltVal,
				m_pProp[PropertyComponentScaleZ]->GetValue().fltVal);
			cmp->m_World = my::Matrix4::Compose(scale, rot, trans);
			VERIFY(pFrame->m_Root.RemoveComponent(cmp));
			pFrame->m_Root.AddComponent(cmp, cmp->m_aabb.transform(cmp->m_World), 0.1f);
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertyMaterialShader:
		{
			Material * material = GetComponentMaterial(cmp, pProp->GetParent()->GetData() - PropertyMaterial0);
			material->m_Shader = ts2ms(pProp->GetValue().bstrVal);
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertyMaterialPassMask:
		{
			Material * material = GetComponentMaterial(cmp, pProp->GetParent()->GetData() - PropertyMaterial0);
			material->m_PassMask = g_PassMaskDesc[(DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex].mask;
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertyMaterialMeshTexture:
		{
			Material * material = GetComponentMaterial(cmp, pProp->GetParent()->GetData() - PropertyMaterial0);
			material->m_MeshTexture.ReleaseResource();
			material->m_MeshTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_MeshTexture.RequestResource();
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertyMaterialNormalTexture:
		{
			Material * material = GetComponentMaterial(cmp, pProp->GetParent()->GetData() - PropertyMaterial0);
			material->m_NormalTexture.ReleaseResource();
			material->m_NormalTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_NormalTexture.RequestResource();
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertyMaterialSpecularTexture:
		{
			Material * material = GetComponentMaterial(cmp, pProp->GetParent()->GetData() - PropertyMaterial0);
			material->m_SpecularTexture.ReleaseResource();
			material->m_SpecularTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_SpecularTexture.RequestResource();
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertyEmitterParticleLifeTime:
		{
			EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
			emit_cmp->m_Emitter->m_ParticleLifeTime = pProp->GetValue().fltVal;
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertySphericalEmitterSpawnInterval:
	case PropertySphericalEmitterHalfSpawnAreaX:
	case PropertySphericalEmitterHalfSpawnAreaY:
	case PropertySphericalEmitterHalfSpawnAreaZ:
	case PropertySphericalEmitterSpawnSpeed:
	case PropertySphericalEmitterSpawnLoopTime:
		{
			EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
			my::SphericalEmitter * spherical_emit = dynamic_cast<my::SphericalEmitter *>(emit_cmp->m_Emitter.get());
			spherical_emit->m_SpawnInterval = m_pProp[PropertySphericalEmitterSpawnInterval]->GetValue().fltVal;
			spherical_emit->m_HalfSpawnArea.x = m_pProp[PropertySphericalEmitterHalfSpawnAreaX]->GetValue().fltVal;
			spherical_emit->m_HalfSpawnArea.y = m_pProp[PropertySphericalEmitterHalfSpawnAreaY]->GetValue().fltVal;
			spherical_emit->m_HalfSpawnArea.z = m_pProp[PropertySphericalEmitterHalfSpawnAreaZ]->GetValue().fltVal;
			spherical_emit->m_SpawnSpeed = m_pProp[PropertySphericalEmitterSpawnSpeed]->GetValue().fltVal;
			spherical_emit->m_SpawnLoopTime = m_pProp[PropertySphericalEmitterSpawnLoopTime]->GetValue().fltVal;
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	case PropertySplineNodeCount:
	case PropertySplineNodeX:
	case PropertySplineNodeY:
	case PropertySplineNodeK0:
	case PropertySplineNodeK:
		{
			EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
			my::SphericalEmitter * spherical_emit = dynamic_cast<my::SphericalEmitter *>(emit_cmp->m_Emitter.get());
			CMFCPropertyGridProperty * pSpline = NULL;
			switch (PropertyId)
			{
			case PropertySplineNodeCount:
				pSpline = pProp->GetParent();
				break;
			case PropertySplineNodeX:
			case PropertySplineNodeY:
			case PropertySplineNodeK0:
			case PropertySplineNodeK:
				pSpline = pProp->GetParent()->GetParent();
				break;
			}
			my::Spline * spline = NULL;
			switch (pSpline->GetData())
			{
			case PropertySphericalEmitterSpawnInclination:
				spline = &spherical_emit->m_SpawnInclination;
				break;
			case PropertySphericalEmitterSpawnAzimuth:
				spline = &spherical_emit->m_SpawnAzimuth;
				break;
			case PropertySphericalEmitterSpawnColorA:
				spline = &spherical_emit->m_SpawnColorA;
				break;
			case PropertySphericalEmitterSpawnColorR:
				spline = &spherical_emit->m_SpawnColorR;
				break;
			case PropertySphericalEmitterSpawnColorG:
				spline = &spherical_emit->m_SpawnColorG;
				break;
			case PropertySphericalEmitterSpawnColorB:
				spline = &spherical_emit->m_SpawnColorB;
				break;
			case PropertySphericalEmitterSpawnSizeX:
				spline = &spherical_emit->m_SpawnSizeX;
				break;
			case PropertySphericalEmitterSpawnSizeY:
				spline = &spherical_emit->m_SpawnSizeY;
				break;
			case PropertySphericalEmitterSpawnAngle:
				spline = &spherical_emit->m_SpawnAngle;
				break;
			}
			switch (PropertyId)
			{
			case PropertySplineNodeCount:
				spline->resize(pProp->GetValue().uintVal, my::SplineNode(0, 0, 0, 0));
				UpdatePropertiesSpline((Property)pSpline->GetData(), spline);
				break;
			case PropertySplineNodeX:
			case PropertySplineNodeY:
			case PropertySplineNodeK0:
			case PropertySplineNodeK:
				{
					CMFCPropertyGridProperty * pNode = pProp->GetParent();
					DWORD id = pNode->GetData();
					_ASSERT(id < spline->size());
					my::SplineNode & node = (*spline)[id];
					node.x = pNode->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->GetValue().fltVal;
					node.y = pNode->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->GetValue().fltVal;
					node.k0 = pNode->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->GetValue().fltVal;
					node.k = pNode->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->GetValue().fltVal;
				}
				break;
			}
			EventArg arg;
			pFrame->m_EventCmpAttriChanged(&arg);
		}
		break;
	}
	return 0;
}

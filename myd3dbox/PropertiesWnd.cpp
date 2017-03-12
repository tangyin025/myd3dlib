
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "CtrlProps.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"

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
//
//struct ShapeTypeDesc
//{
//	LPCTSTR desc;
//	DWORD type;
//};
//
//static const ShapeTypeDesc g_ShapeTypeDesc[PxGeometryType::eGEOMETRY_COUNT] =
//{
//	{_T("Sphere"), PxGeometryType::eSPHERE},
//	{_T("Plane"), PxGeometryType::ePLANE},
//	{_T("Capsule"), PxGeometryType::eCAPSULE},
//	{_T("Box"), PxGeometryType::eBOX},
//	{_T("ConvexMesh"), PxGeometryType::eCONVEXMESH},
//	{_T("TriangleMesh"), PxGeometryType::eTRIANGLEMESH},
//	{_T("HeightField"), PxGeometryType::eHEIGHTFIELD},
//};

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
		UpdateProperties(NULL, 0, *pFrame->m_selcmps.begin());
		m_wndPropList.AdjustLayout();
	}
	else
	{
		m_wndPropList.RemoveAll();
		m_wndPropList.AdjustLayout();
	}
}

void CPropertiesWnd::OnCmpAttriChanged(EventArg * arg)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMainFrame::ComponentSet::iterator cmp_iter = pFrame->m_selcmps.begin();
	if (cmp_iter != pFrame->m_selcmps.end())
	{
		UpdateProperties(NULL, 0, *cmp_iter);
		m_wndPropList.AdjustLayout();
	}
}

void CPropertiesWnd::HideAllProperties(void)
{
	//m_pProp[PropertyComponent]->Show(FALSE, FALSE);
	//m_pProp[PropertyMesh]->Show(FALSE, FALSE);
	//m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
	//m_pProp[PropertyMaterialList]->Show(FALSE, FALSE);
	//m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, DWORD i)
{
	if (pParentCtrl)
	{
		while ((unsigned int)pParentCtrl->GetSubItemsCount() > i)
		{
			CMFCPropertyGridProperty * pProp = pParentCtrl->GetSubItem(i);
			static_cast<CMFCPropertyGridPropertyReader *>(pParentCtrl)->RemoveSubItem(pProp, TRUE);
		}
	}
	else
	{
		while ((unsigned int)m_wndPropList.GetPropertyCount() > i)
		{
			CMFCPropertyGridProperty * pProp = m_wndPropList.GetProperty(i);
			m_wndPropList.DeleteProperty(pProp, FALSE, FALSE);
		}
	}
}

void CPropertiesWnd::UpdateProperties(CMFCPropertyGridProperty * pParentCtrl, DWORD i, Component * cmp)
{
	CMFCPropertyGridProperty * pComponent = NULL;
	if (pParentCtrl)
	{
		if (i < pParentCtrl->GetSubItemsCount())
		{
			pComponent = pParentCtrl->GetSubItem(i);
		}
	}
	else
	{
		if (i < m_wndPropList.GetPropertyCount())
		{
			pComponent = m_wndPropList.GetProperty(i);
		}
	}

	if (!pComponent || pComponent->GetData() != PropertyComponent || pComponent->GetValue().ulVal != (DWORD_PTR)cmp)
	{
		RemovePropertiesFrom(pParentCtrl, i);
		CreateProperties(pParentCtrl, i, cmp);
		return;
	}

	pComponent->GetSubItem(0)->GetSubItem(0)->SetValue((_variant_t)cmp->m_aabb.m_min.x);
	pComponent->GetSubItem(0)->GetSubItem(1)->SetValue((_variant_t)cmp->m_aabb.m_min.y);
	pComponent->GetSubItem(0)->GetSubItem(2)->SetValue((_variant_t)cmp->m_aabb.m_min.z);
	pComponent->GetSubItem(0)->GetSubItem(3)->SetValue((_variant_t)cmp->m_aabb.m_max.x);
	pComponent->GetSubItem(0)->GetSubItem(4)->SetValue((_variant_t)cmp->m_aabb.m_max.y);
	pComponent->GetSubItem(0)->GetSubItem(5)->SetValue((_variant_t)cmp->m_aabb.m_max.z);

	pComponent->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)cmp->m_Position.x);
	pComponent->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)cmp->m_Position.y);
	pComponent->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)cmp->m_Position.z);

	my::Vector3 angle = cmp->m_Rotation.ToEulerAngles();
	pComponent->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)angle.x);
	pComponent->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)angle.y);
	pComponent->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)angle.z);

	pComponent->GetSubItem(3)->GetSubItem(0)->SetValue((_variant_t)cmp->m_Scale.x);
	pComponent->GetSubItem(3)->GetSubItem(1)->SetValue((_variant_t)cmp->m_Scale.y);
	pComponent->GetSubItem(3)->GetSubItem(2)->SetValue((_variant_t)cmp->m_Scale.z);

	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
//		m_pProp[PropertyMesh]->Show(TRUE, FALSE);
//		m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
//		m_pProp[PropertyMaterialList]->Show(TRUE, FALSE);
//		//m_pProp[PropertyRigidShapeList]->Show(FALSE, FALSE);
//		m_pProp[PropertyTerrain]->Show(FALSE, FALSE);
//		UpdatePropertiesMesh(dynamic_cast<MeshComponent *>(cmp));
		UpdatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
//	case Component::ComponentTypeEmitter:
//		m_pProp[PropertyMesh]->Show(FALSE, FALSE);
//		m_pProp[PropertyEmitter]->Show(TRUE, FALSE);
//		m_pProp[PropertyMaterialList]->Show(TRUE, FALSE);
//		//m_pProp[PropertyRigidShapeList]->Show(FALSE, FALSE);
//		m_pProp[PropertyTerrain]->Show(FALSE, FALSE);
//		UpdatePropertiesEmitter(dynamic_cast<EmitterComponent *>(cmp));
//		break;
//	//case Component::ComponentTypeRigid:
//	//	m_pProp[PropertyMesh]->Show(FALSE, FALSE);
//	//	m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
//	//	m_pProp[PropertyMaterialList]->Show(FALSE, FALSE);
//	//	m_pProp[PropertyRigidShapeList]->Show(TRUE, FALSE);
//	//	m_pProp[PropertyTerrain]->Show(FALSE, FALSE);
//	//	UpdatePropertiesRigid(dynamic_cast<RigidComponent *>(cmp));
//	//	break;
//	case Component::ComponentTypeTerrain:
//		m_pProp[PropertyMesh]->Show(FALSE, FALSE);
//		m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
//		m_pProp[PropertyMaterialList]->Show(TRUE, FALSE);
//		//m_pProp[PropertyRigidShapeList]->Show(FALSE, FALSE);
//		m_pProp[PropertyTerrain]->Show(TRUE, FALSE);
//		UpdatePropertiesTerrain(dynamic_cast<Terrain *>(cmp));
//		break;
	}
//	m_wndPropList.AdjustLayout();

	if (!cmp->m_Cmps.empty())
	{
		Component::ComponentPtrList::iterator m_selcmps = cmp->m_Cmps.begin();
		for (; m_selcmps != cmp->m_Cmps.end(); m_selcmps++)
		{
			UpdateProperties(pComponent, GetComponentAttrCount(cmp->m_Type) + std::distance(cmp->m_Cmps.begin(), m_selcmps), m_selcmps->get());
		}
	}
}

void CPropertiesWnd::UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeActor);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyMeshResPath)
	{
		CreatePropertiesMesh(pComponent, mesh_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(mesh_cmp->m_MeshRes.m_Path).c_str());
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)mesh_cmp->m_bAnimation);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)mesh_cmp->m_bInstance);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)mesh_cmp->m_StaticCollision);
	CMFCPropertyGridProperty * pMaterialList = pComponent->GetSubItem(PropId + 4);
	for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
	{
		if ((unsigned int)pMaterialList->GetSubItemsCount() <= i)
		{
			CreatePropertiesMaterial(pMaterialList, i, mesh_cmp->m_MaterialList[i].get());
			continue;
		}
		UpdatePropertiesMaterial(pMaterialList, i, mesh_cmp->m_MaterialList[i].get());
	}
}

void CPropertiesWnd::UpdatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, Material * mat)
{
	CMFCPropertyGridProperty * pMaterial = pParentCtrl->GetSubItem(NodeId);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mat);
	pMaterial->GetSubItem(0)->SetValue((_variant_t)mat->m_Shader.c_str());
	pMaterial->GetSubItem(1)->SetValue((_variant_t)GetPassMaskDesc(mat->m_PassMask));
	pMaterial->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)mat->m_MeshColor.x);
	pMaterial->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)mat->m_MeshColor.y);
	pMaterial->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)mat->m_MeshColor.z);
	pMaterial->GetSubItem(2)->GetSubItem(3)->SetValue((_variant_t)mat->m_MeshColor.w);
	pMaterial->GetSubItem(3)->SetValue((_variant_t)mat->m_MeshTexture.m_Path.c_str());
	pMaterial->GetSubItem(4)->SetValue((_variant_t)mat->m_NormalTexture.m_Path.c_str());
	pMaterial->GetSubItem(5)->SetValue((_variant_t)mat->m_SpecularTexture.m_Path.c_str());
}
//
//void CPropertiesWnd::UpdatePropertiesMeshLodList(CMFCPropertyGridProperty * pLodList, MeshComponent * cmp)
//{
//	pLodList->GetSubItem(0)->SetValue((_variant_t)cmp->m_lods.size());
//	unsigned int i = 0;
//	for (; i < cmp->m_lods.size(); i++)
//	{
//		if ((unsigned int)pLodList->GetSubItemsCount() <= i + 1)
//		{
//			CreatePropertiesMeshLod(pLodList, i);
//		}
//		UpdatePropertiesMeshLod(pLodList, i, cmp->m_lods[i]);
//	}
//	RemovePropertiesFrom(pLodList, i + 1);
//}
//
//void CPropertiesWnd::UpdatePropertiesMeshLod(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, MeshComponent::LOD & lod)
//{
//	CMFCPropertyGridProperty * pNode = pParentCtrl->GetSubItem(NodeId + 1);
//	_ASSERT(pNode->GetData() == NodeId);
//	pNode->GetSubItem(0)->SetValue((_variant_t)lod.m_MeshRes.m_Path.c_str());
//	pNode->GetSubItem(1)->SetValue((_variant_t)lod.m_bInstance);
//	pNode->GetSubItem(2)->SetValue((_variant_t)lod.m_MaxDistance);
//}
//
//void CPropertiesWnd::UpdatePropertiesEmitter(EmitterComponent * cmp)
//{
//	my::DynamicEmitterPtr dynamic_emit = boost::dynamic_pointer_cast<my::DynamicEmitter>(cmp->m_Emitter);
//	if (dynamic_emit)
//	{
//		m_pProp[PropertyEmitterParticleList]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterParticleLifeTime]->Show(TRUE, FALSE);
//		m_pProp[PropertySphericalEmitterParticleLifeTime]->SetValue((_variant_t)dynamic_emit->m_ParticleLifeTime);
//	}
//	else
//	{
//		m_pProp[PropertyEmitterParticleList]->Show(TRUE, FALSE);
//		m_pProp[PropertySphericalEmitterParticleLifeTime]->Show(FALSE, FALSE);
//		UpdatePropertiesEmitterParticleList(m_pProp[PropertyEmitterParticleList], cmp->m_Emitter->m_ParticleList);
//	}
//
//	my::SphericalEmitterPtr spherical_emit = boost::dynamic_pointer_cast<my::SphericalEmitter>(cmp->m_Emitter);
//	if (spherical_emit)
//	{
//		m_pProp[PropertySphericalEmitterSpawnInterval]->SetValue((_variant_t)spherical_emit->m_SpawnInterval);
//		m_pProp[PropertySphericalEmitterHalfSpawnAreaX]->SetValue((_variant_t)spherical_emit->m_HalfSpawnArea.x);
//		m_pProp[PropertySphericalEmitterHalfSpawnAreaY]->SetValue((_variant_t)spherical_emit->m_HalfSpawnArea.y);
//		m_pProp[PropertySphericalEmitterHalfSpawnAreaZ]->SetValue((_variant_t)spherical_emit->m_HalfSpawnArea.z);
//		m_pProp[PropertySphericalEmitterSpawnSpeed]->SetValue((_variant_t)spherical_emit->m_SpawnSpeed);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnInclination, &spherical_emit->m_SpawnInclination);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnAzimuth, &spherical_emit->m_SpawnAzimuth);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorR, &spherical_emit->m_SpawnColorR);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorG, &spherical_emit->m_SpawnColorG);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorB, &spherical_emit->m_SpawnColorB);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnColorA, &spherical_emit->m_SpawnColorA);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnSizeX, &spherical_emit->m_SpawnSizeX);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnSizeY, &spherical_emit->m_SpawnSizeY);
//		UpdatePropertiesSpline(PropertySphericalEmitterSpawnAngle, &spherical_emit->m_SpawnAngle);
//		m_pProp[PropertySphericalEmitterSpawnLoopTime]->SetValue((_variant_t)spherical_emit->m_SpawnLoopTime);
//	}
//	else
//	{
//		m_pProp[PropertySphericalEmitterSpawnInterval]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterHalfSpawnArea]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnSpeed]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnInclination]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnAzimuth]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnColorR]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnColorG]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnColorB]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnColorA]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnSizeX]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnSizeY]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnAngle]->Show(FALSE, FALSE);
//		m_pProp[PropertySphericalEmitterSpawnLoopTime]->Show(FALSE, FALSE);
//	}
//
//	if ((unsigned int)m_pProp[PropertyMaterialList]->GetSubItemsCount() <= 0)
//	{
//		CreatePropertiesMaterial(m_pProp[PropertyMaterialList], 0);
//	}
//	UpdatePropertiesMaterial(m_pProp[PropertyMaterialList], 0, cmp->m_Material.get());
//	RemovePropertiesFrom(m_pProp[PropertyMaterialList], 1);
//}
////
////void CPropertiesWnd::UpdatePropertiesRigid(RigidComponent * cmp)
////{
////	unsigned int NbShapes = cmp->m_RigidActor->getNbShapes();
////	std::vector<PxShape *> shapes(NbShapes);
////	NbShapes = cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
////	unsigned int i = 0;
////	for (; i < NbShapes; i++)
////	{
////		if ((unsigned int)m_pProp[PropertyRigidShapeList]->GetSubItemsCount() > i + 1
////			&& HIWORD(m_pProp[PropertyRigidShapeList]->GetSubItem(i + 1)->GetData()) != shapes[i]->getGeometryType())
////		{
////			RemovePropertiesFrom(m_pProp[PropertyRigidShapeList], i + 1);
////		}
////		if ((unsigned int)m_pProp[PropertyRigidShapeList]->GetSubItemsCount() <= i + 1)
////		{
////			CreatePropertiesShape(m_pProp[PropertyRigidShapeList], i, shapes[i]->getGeometryType());
////		}
////		UpdatePropertiesShape(m_pProp[PropertyRigidShapeList], i, shapes[i]);
////	}
////	RemovePropertiesFrom(m_pProp[PropertyRigidShapeList], i + 1);
////}
//
//void CPropertiesWnd::UpdatePropertiesTerrain(Terrain * terrain)
//{
//	if ((unsigned int)m_pProp[PropertyMaterialList]->GetSubItemsCount() <= 0)
//	{
//		CreatePropertiesMaterial(m_pProp[PropertyMaterialList], 0);
//	}
//	UpdatePropertiesMaterial(m_pProp[PropertyMaterialList], 0, terrain->m_Material.get());
//	RemovePropertiesFrom(m_pProp[PropertyMaterialList], 1);
//	m_pProp[PropertyTerrainRowChunks]->SetValue((_variant_t)terrain->m_RowChunks);
//	m_pProp[PropertyTerrainChunkRows]->SetValue((_variant_t)terrain->m_ChunkRows);
//	m_pProp[PropertyTerrainHeightScale]->SetValue((_variant_t)terrain->m_HeightScale);
//	m_pProp[PropertyTerrainWrappedU]->SetValue((_variant_t)terrain->m_WrappedU);
//	m_pProp[PropertyTerrainWrappedV]->SetValue((_variant_t)terrain->m_WrappedV);
//	m_pProp[PropertyTerrainStaticCollision]->SetValue((_variant_t)(VARIANT_BOOL)terrain->m_StaticCollision);
//}
//
//void CPropertiesWnd::UpdatePropertiesEmitterParticleList(CMFCPropertyGridProperty * pParticleList, const my::Emitter::ParticleList & particle_list)
//{
//	m_pProp[PropertyEmitterParticleCount]->SetValue((_variant_t)particle_list.size());
//	unsigned int i = 0;
//	for (; i < particle_list.size(); i++)
//	{
//		if ((unsigned int)m_pProp[PropertyEmitterParticleList]->GetSubItemsCount() <= i + 1)
//		{
//			CreatePropertiesEmitterParticle(m_pProp[PropertyEmitterParticleList], i);
//		}
//		UpdatePropertiesEmitterParticle(m_pProp[PropertyEmitterParticleList], i, particle_list[i]);
//	}
//	RemovePropertiesFrom(m_pProp[PropertyEmitterParticleList], i + 1);
//}
//
//void CPropertiesWnd::UpdatePropertiesEmitterParticle(CMFCPropertyGridProperty * pParentProp, DWORD NodeId, const my::Emitter::Particle & particle)
//{
//	CMFCPropertyGridProperty * pParticle = pParentProp->GetSubItem(NodeId + 1);
//	_ASSERT(pParticle);
//	CMFCPropertyGridProperty * pProp = pParticle->GetSubItem(0)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionX); pProp->SetValue((_variant_t)particle.m_Position.x);
//	pProp = pParticle->GetSubItem(0)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionY); pProp->SetValue((_variant_t)particle.m_Position.y);
//	pProp = pParticle->GetSubItem(0)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionZ); pProp->SetValue((_variant_t)particle.m_Position.z);
//	pProp = pParticle->GetSubItem(1)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityX); pProp->SetValue((_variant_t)particle.m_Velocity.x);
//	pProp = pParticle->GetSubItem(1)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityY); pProp->SetValue((_variant_t)particle.m_Velocity.y);
//	pProp = pParticle->GetSubItem(1)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityZ); pProp->SetValue((_variant_t)particle.m_Velocity.z);
//	pProp = pParticle->GetSubItem(2)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorR); pProp->SetValue((_variant_t)particle.m_Color.x);
//	pProp = pParticle->GetSubItem(2)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorG); pProp->SetValue((_variant_t)particle.m_Color.y);
//	pProp = pParticle->GetSubItem(2)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorB); pProp->SetValue((_variant_t)particle.m_Color.z);
//	pProp = pParticle->GetSubItem(2)->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorA); pProp->SetValue((_variant_t)particle.m_Color.w);
//	pProp = pParticle->GetSubItem(3)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeX); pProp->SetValue((_variant_t)particle.m_Size.x);
//	pProp = pParticle->GetSubItem(3)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeY); pProp->SetValue((_variant_t)particle.m_Size.y);
//	pProp = pParticle->GetSubItem(4); _ASSERT(pProp->GetData() == PropertyEmitterParticleAngle); pProp->SetValue((_variant_t)particle.m_Angle);
//}
//
//void CPropertiesWnd::UpdatePropertiesSpline(Property PropertyId, my::Spline * spline)
//{
//	_ASSERT(PropertyId >= PropertySphericalEmitterSpawnInclination && PropertyId <= PropertySphericalEmitterSpawnAngle);
//	m_pProp[PropertyId]->GetSubItem(0)->SetValue((_variant_t)spline->size());
//	unsigned int i = 0;
//	for (; i < spline->size(); i++)
//	{
//		if ((unsigned int)m_pProp[PropertyId]->GetSubItemsCount() <= i + 1)
//		{
//			CreatePropertiesSplineNode(m_pProp[PropertyId], i);
//		}
//		UpdatePropertiesSplineNode(m_pProp[PropertyId], i, &(*spline)[i]);
//	}
//	RemovePropertiesFrom(m_pProp[PropertyId], i + 1);
//}
//
//void CPropertiesWnd::UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId, const my::SplineNode * node)
//{
//	CMFCPropertyGridProperty * pProp = pSpline->GetSubItem(NodeId + 1);
//	_ASSERT(pProp);
//	pProp->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->SetValue((_variant_t)node->x);
//	pProp->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->SetValue((_variant_t)node->y);
//	pProp->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->SetValue((_variant_t)node->k0);
//	pProp->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->SetValue((_variant_t)node->k);
//}
//
//void CPropertiesWnd::UpdatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, PxShape * shape)
//{
//	CMFCPropertyGridProperty * pShape = pParentCtrl->GetSubItem(NodeId + 1);
//	_ASSERT(LOWORD(pShape->GetData()) == NodeId && HIWORD(pShape->GetData()) == shape->getGeometryType());
//	PxTransform pose = shape->getLocalPose();
//	my::Vector3 euler = ((my::Quaternion&)pose.q).ToEulerAngles();
//	pShape->GetSubItem(0)->GetSubItem(0)->SetValue((_variant_t)pose.p.x);
//	pShape->GetSubItem(0)->GetSubItem(1)->SetValue((_variant_t)pose.p.y);
//	pShape->GetSubItem(0)->GetSubItem(2)->SetValue((_variant_t)pose.p.z);
//	pShape->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(euler.x));
//	pShape->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(euler.y));
//	pShape->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(euler.z));
//	int type = shape->getGeometryType();
//	switch (type)
//	{
//	case PxGeometryType::eSPHERE:
//		UpdatePropertiesShapeSphere(pShape, shape->getGeometry().sphere());
//		break;
//	case PxGeometryType::ePLANE:
//		break;
//	case PxGeometryType::eCAPSULE:
//		UpdatePropertiesShapeCapsule(pShape, shape->getGeometry().capsule());
//		break;
//	case PxGeometryType::eBOX:
//		UpdatePropertiesShapeBox(pShape, shape->getGeometry().box());
//		break;
//	}
//}
//
//void CPropertiesWnd::UpdatePropertiesShapeBox(CMFCPropertyGridProperty * pShape, PxBoxGeometry & box)
//{
//	pShape->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)box.halfExtents.x);
//	pShape->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)box.halfExtents.y);
//	pShape->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)box.halfExtents.z);
//}
//
//void CPropertiesWnd::UpdatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape, PxSphereGeometry & sphere)
//{
//	pShape->GetSubItem(2)->SetValue((_variant_t)sphere.radius);
//}
//
//void CPropertiesWnd::UpdatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape, PxCapsuleGeometry & capsule)
//{
//	pShape->GetSubItem(2)->SetValue((_variant_t)capsule.radius);
//	pShape->GetSubItem(3)->SetValue((_variant_t)capsule.halfHeight);
//}

void CPropertiesWnd::CreateProperties(CMFCPropertyGridProperty * pParentCtrl, DWORD i, Component * cmp)
{
	CMFCPropertyGridProperty * pComponent = NULL;
	if (pParentCtrl)
	{
		while (i >= pParentCtrl->GetSubItemsCount())
		{
			pComponent = new CSimpleProp(_T("Component"), PropertyComponent, FALSE);
			pParentCtrl->AddSubItem(pComponent);
		}
	}
	else
	{
		while (i >= m_wndPropList.GetPropertyCount())
		{
			pComponent = new CSimpleProp(_T("Component"), PropertyComponent, FALSE);
			m_wndPropList.AddProperty(pComponent);
		}
	}
	ASSERT(pComponent);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp); // ! only worked on 32bit system

	CMFCPropertyGridProperty * pAABB = new CSimpleProp(_T("AABB"), PropertyComponentAABB, TRUE);
	pComponent->AddSubItem(pAABB);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("minx"), (_variant_t)cmp->m_aabb.m_min.x, NULL, PropertyComponentMinX);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("miny"), (_variant_t)cmp->m_aabb.m_min.y, NULL, PropertyComponentMinY);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("minz"), (_variant_t)cmp->m_aabb.m_min.z, NULL, PropertyComponentMinZ);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxx"), (_variant_t)cmp->m_aabb.m_max.x, NULL, PropertyComponentMaxX);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxy"), (_variant_t)cmp->m_aabb.m_max.y, NULL, PropertyComponentMaxY);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxz"), (_variant_t)cmp->m_aabb.m_max.z, NULL, PropertyComponentMaxZ);
	pAABB->AddSubItem(pProp);

	CMFCPropertyGridProperty * pPosition = new CMFCPropertyGridProperty(_T("Position"), PropertyComponentPos, TRUE);
	pComponent->AddSubItem(pPosition);
	pProp = new CSimpleProp(_T("x"), (_variant_t)cmp->m_Position.x, NULL, PropertyComponentPosX);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)cmp->m_Position.y, NULL, PropertyComponentPosY);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)cmp->m_Position.z, NULL, PropertyComponentPosZ);
	pPosition->AddSubItem(pProp);

	my::Vector3 engle = cmp->m_Rotation.ToEulerAngles();
	CMFCPropertyGridProperty * pRotate = new CSimpleProp(_T("Rotate"), PropertyComponentRot, TRUE);
	pComponent->AddSubItem(pRotate);
	pProp = new CSimpleProp(_T("x"), (_variant_t)engle.x, NULL, PropertyComponentRotX);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)engle.y, NULL, PropertyComponentRotY);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)engle.z, NULL, PropertyComponentRotZ);
	pRotate->AddSubItem(pProp);

	CMFCPropertyGridProperty * pScale = new CSimpleProp(_T("Scale"), PropertyComponentScale, TRUE);
	pComponent->AddSubItem(pScale);
	pProp = new CSimpleProp(_T("x"), (_variant_t)cmp->m_Scale.x, NULL, PropertyComponentScaleX);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)cmp->m_Scale.y, NULL, PropertyComponentScaleY);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)cmp->m_Scale.z, NULL, PropertyComponentScaleZ);
	pScale->AddSubItem(pProp);

	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		CreatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	}

	if (!cmp->m_Cmps.empty())
	{
		Component::ComponentPtrList::iterator m_selcmps = cmp->m_Cmps.begin();
		for (; m_selcmps != cmp->m_Cmps.end(); m_selcmps++)
		{
			CreateProperties(pComponent, GetComponentAttrCount(cmp->m_Type) + std::distance(cmp->m_Cmps.begin(), m_selcmps), m_selcmps->get());
		}
	}
}

void CPropertiesWnd::CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeActor);
	while ((unsigned int)pComponent->GetSubItemsCount() > PropId)
	{
		CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
		static_cast<CMFCPropertyGridPropertyReader *>(pComponent)->RemoveSubItem(pProp, TRUE);
	}

	CMFCPropertyGridProperty * pProp = new CFileProp(_T("ResPath"), TRUE, (_variant_t)ms2ts(mesh_cmp->m_MeshRes.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshResPath);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Animation"), (_variant_t)mesh_cmp->m_bAnimation, NULL, PropertyMeshAnimation);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Instance"), (_variant_t)mesh_cmp->m_bInstance, NULL, PropertyMeshInstance);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("StaticCollision"), (_variant_t)mesh_cmp->m_StaticCollision, NULL, PropertyMeshStaticCollision);
	pComponent->AddSubItem(pProp);
	pProp = new CMFCPropertyGridProperty(_T("MaterialList"), PropertyMaterialList, FALSE);
	pComponent->AddSubItem(pProp);
	for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
	{
		CreatePropertiesMaterial(pProp, i, mesh_cmp->m_MaterialList[i].get());
	}
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, Material * mat)
{
	TCHAR buff[128];
	_stprintf_s(buff, _countof(buff), _T("Material%u"), NodeId);
	CMFCPropertyGridProperty * pMaterial = new CSimpleProp(buff, NodeId, FALSE);
	pParentCtrl->AddSubItem(pMaterial);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mat);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Shader"), (_variant_t)ms2ts(mat->m_Shader).c_str(), NULL, PropertyMaterialShader);
	pMaterial->AddSubItem(pProp);
	pProp = new CComboProp(_T("PassMask"), (_variant_t)GetPassMaskDesc(mat->m_PassMask), NULL, PropertyMaterialPassMask);
	for (unsigned int i = 0; i < _countof(g_PassMaskDesc); i++)
	{
		pProp->AddOption(g_PassMaskDesc[i].desc, TRUE);
	}
	pMaterial->AddSubItem(pProp);
	CMFCPropertyGridProperty * pColor = new CSimpleProp(_T("MeshColor"), PropertyMaterialMeshColor, TRUE);
	pMaterial->AddSubItem(pColor);
	pProp = new CSimpleProp(_T("r"), (_variant_t)mat->m_MeshColor.x, NULL, PropertyMaterialMeshColorR);
	pColor->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("g"), (_variant_t)mat->m_MeshColor.y, NULL, PropertyMaterialMeshColorG);
	pColor->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("b"), (_variant_t)mat->m_MeshColor.z, NULL, PropertyMaterialMeshColorB);
	pColor->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("a"), (_variant_t)mat->m_MeshColor.w, NULL, PropertyMaterialMeshColorA);
	pColor->AddSubItem(pProp);
	pProp = new CFileProp(_T("MeshTexture"), TRUE, (_variant_t)ms2ts(mat->m_MeshTexture.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialMeshTexture);
	pMaterial->AddSubItem(pProp);
	pProp = new CFileProp(_T("NormalTexture"), TRUE, (_variant_t)ms2ts(mat->m_NormalTexture.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialNormalTexture);
	pMaterial->AddSubItem(pProp);
	pProp = new CFileProp(_T("SpecularTexture"), TRUE, (_variant_t)ms2ts(mat->m_SpecularTexture.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialSpecularTexture);
	pMaterial->AddSubItem(pProp);
}
//
//void CPropertiesWnd::CreatePropertiesMeshLod(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId)
//{
//	TCHAR buff[128];
//	_stprintf_s(buff, _countof(buff), _T("Lod%u"), NodeId);
//	CMFCPropertyGridProperty * pNode = new CMFCPropertyGridProperty(buff, NodeId, FALSE);
//	pParentCtrl->AddSubItem(pNode);
//	CMFCPropertyGridProperty * pProp = new CFileProp(_T("ResPath"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshResPath);
//	pNode->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("Instance"), (_variant_t)false, NULL, PropertyMeshInstance);
//	pNode->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("MaxDistance"), (_variant_t)0.0f, NULL, PropertyMeshLodMaxDistance);
//	pNode->AddSubItem(pProp);
//}
//
//void CPropertiesWnd::CreatePropertiesEmitterParticle(CMFCPropertyGridProperty * pParentProp, DWORD NodeId)
//{
//	TCHAR buff[128];
//	_stprintf_s(buff, _countof(buff), _T("Particle%u"), NodeId);
//	CMFCPropertyGridProperty * pParticle = new CMFCPropertyGridProperty(buff, NodeId, FALSE);
//	pParentProp->AddSubItem(pParticle);
//	CMFCPropertyGridProperty * pPosition = new CSimpleProp(_T("Position"), PropertyEmitterParticlePosition, TRUE);
//	pParticle->AddSubItem(pPosition);
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyEmitterParticlePositionX);
//	pPosition->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyEmitterParticlePositionY);
//	pPosition->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyEmitterParticlePositionZ);
//	pPosition->AddSubItem(pProp);
//
//	CMFCPropertyGridProperty * pVelocity = new CSimpleProp(_T("Velocity"), PropertyEmitterParticleVelocity, TRUE);
//	pParticle->AddSubItem(pVelocity);
//	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyEmitterParticleVelocityX);
//	pVelocity->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyEmitterParticleVelocityY);
//	pVelocity->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyEmitterParticleVelocityZ);
//	pVelocity->AddSubItem(pProp);
//
//	CMFCPropertyGridProperty * pColor = new CSimpleProp(_T("Color"), PropertyEmitterParticleColor, TRUE);
//	pParticle->AddSubItem(pColor);
//	pProp = new CSimpleProp(_T("r"), (_variant_t)0.0f, NULL, PropertyEmitterParticleColorR);
//	pColor->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("g"), (_variant_t)0.0f, NULL, PropertyEmitterParticleColorG);
//	pColor->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("b"), (_variant_t)0.0f, NULL, PropertyEmitterParticleColorB);
//	pColor->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("a"), (_variant_t)0.0f, NULL, PropertyEmitterParticleColorA);
//	pColor->AddSubItem(pProp);
//
//	CMFCPropertyGridProperty * pSize = new CSimpleProp(_T("Size"), PropertyEmitterParticleSize, TRUE);
//	pParticle->AddSubItem(pSize);
//	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyEmitterParticleSizeX);
//	pSize->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyEmitterParticleSizeY);
//	pSize->AddSubItem(pProp);
//
//	pProp = new CSimpleProp(_T("Angle"), (_variant_t)0.0f, NULL, PropertyEmitterParticleAngle);
//	pParticle->AddSubItem(pProp);
//}
//
//void CPropertiesWnd::CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId)
//{
//	_ASSERT(PropertyId >= PropertySphericalEmitterSpawnInclination && PropertyId <= PropertySphericalEmitterSpawnAngle);
//	m_pProp[PropertyId] = new CSimpleProp(lpszName, PropertyId, TRUE);
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)(size_t)0, NULL, PropertySplineNodeCount);
//	m_pProp[PropertyId]->AddSubItem(pProp);
//	CreatePropertiesSplineNode(m_pProp[PropertyId], 0);
//	CreatePropertiesSplineNode(m_pProp[PropertyId], 1);
//	CreatePropertiesSplineNode(m_pProp[PropertyId], 2);
//	pParentProp->AddSubItem(m_pProp[PropertyId]);
//}
//
//void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId)
//{
//	CMFCPropertyGridProperty * pNode = new CSimpleProp(_T("Node"), NodeId, TRUE);
//	pSpline->AddSubItem(pNode);
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertySplineNodeX);
//	pNode->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertySplineNodeY);
//	pNode->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("k0"), (_variant_t)0.0f, NULL, PropertySplineNodeK0);
//	pNode->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("k"), (_variant_t)0.0f, NULL, PropertySplineNodeK);
//	pNode->AddSubItem(pProp);
//}
////
////void CPropertiesWnd::CreatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, PxGeometryType::Enum type)
////{
////	TCHAR buff[128];
////	_stprintf_s(buff, _countof(buff), _T("%u.%s"), NodeId, g_ShapeTypeDesc[type].desc);
////	CMFCPropertyGridProperty * pShape = new CMFCPropertyGridProperty(buff, MAKELONG(NodeId, type), FALSE);
////	pParentCtrl->AddSubItem(pShape);
////	CMFCPropertyGridProperty * pPos = new CSimpleProp(_T("Pos"), PropertyRigidShapePos, TRUE);
////	pShape->AddSubItem(pPos);
////	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyRigidShapePosX);
////	pPos->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyRigidShapePosY);
////	pPos->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyRigidShapePosZ);
////	pPos->AddSubItem(pProp);
////	CMFCPropertyGridProperty * pRot = new CSimpleProp(_T("Rot"), PropertyRigidShapeRot, TRUE);
////	pShape->AddSubItem(pRot);
////	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyRigidShapeRotX);
////	pRot->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyRigidShapeRotY);
////	pRot->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyRigidShapeRotZ);
////	pRot->AddSubItem(pProp);
////	switch (type)
////	{
////	case PxGeometryType::eSPHERE:
////		CreatePropertiesShapeSphere(pShape);
////		break;
////	case PxGeometryType::ePLANE:
////		break;
////	case PxGeometryType::eCAPSULE:
////		CreatePropertiesShapeCapsule(pShape);
////		break;
////	case PxGeometryType::eBOX:
////		CreatePropertiesShapeBox(pShape);
////		break;
////	}
////}
////
////void CPropertiesWnd::CreatePropertiesShapeBox(CMFCPropertyGridProperty * pShape)
////{
////	CMFCPropertyGridProperty * pHalfExtents = new CSimpleProp(_T("HalfExtents"), PropertyRigidShapeBoxHalfExtents, TRUE);
////	pShape->AddSubItem(pHalfExtents);
////	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyRigidShapeBoxHalfExtentsX);
////	pHalfExtents->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyRigidShapeBoxHalfExtentsY);
////	pHalfExtents->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyRigidShapeBoxHalfExtentsZ);
////	pHalfExtents->AddSubItem(pProp);
////}
////
////void CPropertiesWnd::CreatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape)
////{
////	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Radius"), (_variant_t)0.0f, NULL, PropertyRigidShapeSphereRadius);
////	pShape->AddSubItem(pProp);
////}
////
////void CPropertiesWnd::CreatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape)
////{
////	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Radius"), (_variant_t)0.0f, NULL, PropertyRigidShapeCapsuleRadius);
////	pShape->AddSubItem(pProp);
////	pProp = new CSimpleProp(_T("HalfHeight"), (_variant_t)0.0f, NULL, PropertyRigidShapeCapsuleHalfHeight);
////	pShape->AddSubItem(pProp);
////}
//
//Material * CPropertiesWnd::GetComponentMaterial(Component * cmp, unsigned int id)
//{
//	switch (cmp->m_Type)
//	{
//	case Component::ComponentTypeMesh:
//		{
//			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
//			if (id < mesh_cmp->m_MaterialList.size())
//			{
//				return mesh_cmp->m_MaterialList[id].get();
//			}
//		}
//		break;
//	case Component::ComponentTypeEmitter:
//		{
//			EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
//			if (id == 0)
//			{
//				return emit_cmp->m_Material.get();
//			}
//		}
//		break;
//	case Component::ComponentTypeTerrain:
//		{
//			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
//			if (id == 0)
//			{
//				return terrain->m_Material.get();
//			}
//		}
//		break;
//	}
//	return NULL;
//}

unsigned int CPropertiesWnd::GetComponentAttrCount(Component::ComponentType type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return 4;
	case Component::ComponentTypeMesh:
		return GetComponentAttrCount(Component::ComponentTypeActor) + 5;
	case Component::ComponentTypeEmitter:
		return GetComponentAttrCount(Component::ComponentTypeActor);
	case Component::ComponentTypeTerrain:
		return GetComponentAttrCount(Component::ComponentTypeActor);
	}
	return 0;
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
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.connect(boost::bind(&CPropertiesWnd::OnCmpAttriChanged, this, _1));

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	//// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.disconnect(boost::bind(&CPropertiesWnd::OnCmpAttriChanged, this, _1));
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

	//m_pProp[PropertyComponent] = new CMFCPropertyGridProperty(_T("Component"), PropertyComponent, FALSE);
	//CMFCPropertyGridProperty * pAABB = new CSimpleProp(_T("AABB"), PropertyComponentAABB, TRUE);
	//m_pProp[PropertyComponentMinX] = new CSimpleProp(_T("minx"), (_variant_t)0.0f, NULL, PropertyComponentMinX);
	//pAABB->AddSubItem(m_pProp[PropertyComponentMinX]);
	//m_pProp[PropertyComponentMinY] = new CSimpleProp(_T("miny"), (_variant_t)0.0f, NULL, PropertyComponentMinY);
	//pAABB->AddSubItem(m_pProp[PropertyComponentMinY]);
	//m_pProp[PropertyComponentMinZ] = new CSimpleProp(_T("minz"), (_variant_t)0.0f, NULL, PropertyComponentMinZ);
	//pAABB->AddSubItem(m_pProp[PropertyComponentMinZ]);
	//m_pProp[PropertyComponentMaxX] = new CSimpleProp(_T("maxx"), (_variant_t)0.0f, NULL, PropertyComponentMaxX);
	//pAABB->AddSubItem(m_pProp[PropertyComponentMaxX]);
	//m_pProp[PropertyComponentMaxY] = new CSimpleProp(_T("maxy"), (_variant_t)0.0f, NULL, PropertyComponentMaxY);
	//pAABB->AddSubItem(m_pProp[PropertyComponentMaxY]);
	//m_pProp[PropertyComponentMaxZ] = new CSimpleProp(_T("maxz"), (_variant_t)0.0f, NULL, PropertyComponentMaxZ);
	//pAABB->AddSubItem(m_pProp[PropertyComponentMaxZ]);
	//m_pProp[PropertyComponent]->AddSubItem(pAABB);

	//CMFCPropertyGridProperty * pPosition = new CSimpleProp(_T("Position"), PropertyComponentPos, TRUE);
	//m_pProp[PropertyComponentPosX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyComponentPosX);
	//pPosition->AddSubItem(m_pProp[PropertyComponentPosX]);
	//m_pProp[PropertyComponentPosY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyComponentPosY);
	//pPosition->AddSubItem(m_pProp[PropertyComponentPosY]);
	//m_pProp[PropertyComponentPosZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyComponentPosZ);
	//pPosition->AddSubItem(m_pProp[PropertyComponentPosZ]);
	//m_pProp[PropertyComponent]->AddSubItem(pPosition);

	//CMFCPropertyGridProperty * pRotate = new CSimpleProp(_T("Rotate"), PropertyComponentRot, TRUE);
	//m_pProp[PropertyComponentRotX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyComponentRotX);
	//pRotate->AddSubItem(m_pProp[PropertyComponentRotX]);
	//m_pProp[PropertyComponentRotY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyComponentRotY);
	//pRotate->AddSubItem(m_pProp[PropertyComponentRotY]);
	//m_pProp[PropertyComponentRotZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyComponentRotZ);
	//pRotate->AddSubItem(m_pProp[PropertyComponentRotZ]);
	//m_pProp[PropertyComponent]->AddSubItem(pRotate);

	//CMFCPropertyGridProperty * pScale = new CSimpleProp(_T("Scale"), PropertyComponentScale, TRUE);
	//m_pProp[PropertyComponentScaleX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyComponentScaleX);
	//pScale->AddSubItem(m_pProp[PropertyComponentScaleX]);
	//m_pProp[PropertyComponentScaleY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyComponentScaleY);
	//pScale->AddSubItem(m_pProp[PropertyComponentScaleY]);
	//m_pProp[PropertyComponentScaleZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyComponentScaleZ);
	//pScale->AddSubItem(m_pProp[PropertyComponentScaleZ]);
	//m_pProp[PropertyComponent]->AddSubItem(pScale);
	//m_wndPropList.AddProperty(m_pProp[PropertyComponent], FALSE, FALSE);

	//m_pProp[PropertyMesh] = new CMFCPropertyGridProperty(_T("Mesh"), PropertyMesh, FALSE);
	//m_wndPropList.AddProperty(m_pProp[PropertyMesh], FALSE, FALSE);
	//m_pProp[PropertyMeshLodList] = new CMFCPropertyGridProperty(_T("Lods"), PropertyMeshLodList, FALSE);
	//m_pProp[PropertyMesh]->AddSubItem(m_pProp[PropertyMeshLodList]);
	//m_pProp[PropertyMeshLodCount] = new CSimpleProp(_T("LodCount"), (_variant_t)(size_t)0, NULL, PropertyMeshLodCount);
	//m_pProp[PropertyMeshLodList]->AddSubItem(m_pProp[PropertyMeshLodCount]);
	//for (unsigned int i = 0; i < 3; i++)
	//{
	//	CreatePropertiesMeshLod(m_pProp[PropertyMeshLodList], i);
	//}
	//m_pProp[PropertyMeshLodBand] = new CSimpleProp(_T("LodBand"), (_variant_t)0.0f, NULL, PropertyMeshLodBand);
	//m_pProp[PropertyMesh]->AddSubItem(m_pProp[PropertyMeshLodBand]);
	//m_pProp[PropertyMeshStaticCollision] = new CCheckBoxProp(_T("StaticCollision"), FALSE, NULL, PropertyMeshStaticCollision);
	//m_pProp[PropertyMesh]->AddSubItem(m_pProp[PropertyMeshStaticCollision]);

	//m_pProp[PropertyEmitter] = new CMFCPropertyGridProperty(_T("Emitter"), PropertyEmitter, FALSE);
	//m_pProp[PropertyEmitterParticleList] = new CMFCPropertyGridProperty(_T("ParticleList"), PropertyEmitterParticleList, FALSE);
	//m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertyEmitterParticleList]);
	//m_pProp[PropertyEmitterParticleCount] = new CSimpleProp(_T("ParticleCount"), (_variant_t)(size_t)0, NULL, PropertyEmitterParticleCount);
	//m_pProp[PropertyEmitterParticleList]->AddSubItem(m_pProp[PropertyEmitterParticleCount]);
	//m_pProp[PropertySphericalEmitterParticleLifeTime] = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)0.0f, NULL, PropertySphericalEmitterParticleLifeTime);
	//m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterParticleLifeTime]);
	//m_pProp[PropertySphericalEmitterSpawnInterval] = new CSimpleProp(_T("SpawnInterval"), (_variant_t)0.0f, NULL, PropertySphericalEmitterSpawnInterval);
	//m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterSpawnInterval]);
	//m_pProp[PropertySphericalEmitterHalfSpawnArea] = new CSimpleProp(_T("HalfSpawnArea"), 0, TRUE);
	//m_pProp[PropertySphericalEmitterHalfSpawnAreaX] = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertySphericalEmitterHalfSpawnAreaX);
	//m_pProp[PropertySphericalEmitterHalfSpawnArea]->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnAreaX]);
	//m_pProp[PropertySphericalEmitterHalfSpawnAreaY] = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertySphericalEmitterHalfSpawnAreaY);
	//m_pProp[PropertySphericalEmitterHalfSpawnArea]->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnAreaY]);
	//m_pProp[PropertySphericalEmitterHalfSpawnAreaZ] = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertySphericalEmitterHalfSpawnAreaZ);
	//m_pProp[PropertySphericalEmitterHalfSpawnArea]->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnAreaZ]);
	//m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterHalfSpawnArea]);
	//m_pProp[PropertySphericalEmitterSpawnSpeed] = new CSimpleProp(_T("SpawnSpeed"), (_variant_t)0.0f, NULL, PropertySphericalEmitterSpawnSpeed);
	//m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterSpawnSpeed]);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnInclination"), PropertySphericalEmitterSpawnInclination);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnAzimuth"), PropertySphericalEmitterSpawnAzimuth);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnColorR"), PropertySphericalEmitterSpawnColorR);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnColorG"), PropertySphericalEmitterSpawnColorG);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnColorB"), PropertySphericalEmitterSpawnColorB);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnColorA"), PropertySphericalEmitterSpawnColorA);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnSizeX"), PropertySphericalEmitterSpawnSizeX);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnSizeY"), PropertySphericalEmitterSpawnSizeY);
	//CreatePropertiesSpline(m_pProp[PropertyEmitter], _T("SpawnAngle"), PropertySphericalEmitterSpawnAngle);
	//m_pProp[PropertySphericalEmitterSpawnLoopTime] = new CSimpleProp(_T("SpawnLoopTime"), (_variant_t)0.0f, NULL, PropertySphericalEmitterSpawnLoopTime);
	//m_pProp[PropertyEmitter]->AddSubItem(m_pProp[PropertySphericalEmitterSpawnLoopTime]);
	//m_wndPropList.AddProperty(m_pProp[PropertyEmitter], FALSE, FALSE);

	//m_pProp[PropertyMaterialList] = new CMFCPropertyGridProperty(_T("MaterialList"), PropertyMaterialList, FALSE);
	//m_wndPropList.AddProperty(m_pProp[PropertyMaterialList], FALSE, FALSE);
	//for (unsigned int i = 0; i < 3; i++)
	//{
	//	CreatePropertiesMaterial(m_pProp[PropertyMaterialList], i);
	//}

	////m_pProp[PropertyRigidShapeList] = new CMFCPropertyGridProperty(_T("ShapeList"), PropertyRigidShapeList, FALSE);
	////m_wndPropList.AddProperty(m_pProp[PropertyRigidShapeList], FALSE, FALSE);
	////m_pProp[PropertyRigidShapeAdd] = new CComboProp(_T("Add..."), (_variant_t)_T(""), NULL, PropertyRigidShapeAdd);
	////for (unsigned int i = 0; i < _countof(g_ShapeTypeDesc); i++)
	////{
	////	m_pProp[PropertyRigidShapeAdd]->AddOption(g_ShapeTypeDesc[i].desc, TRUE);
	////}
	////m_pProp[PropertyRigidShapeList]->AddSubItem(m_pProp[PropertyRigidShapeAdd]);
	////for (unsigned int i = 0; i < 3; i++)
	////{
	////	CreatePropertiesShape(m_pProp[PropertyRigidShapeList], i, PxGeometryType::eBOX);
	////}

	//m_pProp[PropertyTerrain] = new CMFCPropertyGridProperty(_T("Terrain"), PropertyTerrain, FALSE);
	//m_wndPropList.AddProperty(m_pProp[PropertyTerrain], FALSE, FALSE);
	//m_pProp[PropertyTerrainRowChunks] = new CSimpleProp(_T("RowChunks"), (_variant_t)(DWORD)1, NULL, PropertyTerrainRowChunks);
	//m_pProp[PropertyTerrainRowChunks]->Enable(FALSE);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainRowChunks]);
	//m_pProp[PropertyTerrainChunkRows] = new CSimpleProp(_T("ChunkRows"), (_variant_t)(DWORD)1, NULL, PropertyTerrainChunkRows);
	//m_pProp[PropertyTerrainChunkRows]->Enable(FALSE);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainChunkRows]);
	//m_pProp[PropertyTerrainHeightScale] = new CSimpleProp(_T("HeightScale"), (_variant_t)1.0f, NULL, PropertyTerrainHeightScale);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainHeightScale]);
	//m_pProp[PropertyTerrainWrappedU] = new CSimpleProp(_T("WrappedU"), (_variant_t)1.0f, NULL, PropertyTerrainWrappedU);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainWrappedU]);
	//m_pProp[PropertyTerrainWrappedV] = new CSimpleProp(_T("WrappedV"), (_variant_t)1.0f, NULL, PropertyTerrainWrappedV);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainWrappedV]);
	//m_pProp[PropertyTerrainHeightMap] = new CFileProp(_T("HeightMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainHeightMap);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainHeightMap]);
	//m_pProp[PropertyTerrainStaticCollision] = new CCheckBoxProp(_T("StaticCollision"), FALSE, NULL, PropertyTerrainStaticCollision);
	//m_pProp[PropertyTerrain]->AddSubItem(m_pProp[PropertyTerrainStaticCollision]);

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
	//if (pFrame->m_selcmps.empty())
	//{
	//	return 0;
	//}
	//Component * cmp = *pFrame->m_selcmps.begin();

	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	DWORD PropertyId = pProp->GetData();
	switch (PropertyId)
	{
	case PropertyComponentAABB:
	case PropertyComponentMinX:
	case PropertyComponentMinY:
	case PropertyComponentMinZ:
	case PropertyComponentMaxX:
	case PropertyComponentMaxY:
	case PropertyComponentMaxZ:
	case PropertyComponentPos:
	case PropertyComponentPosX:
	case PropertyComponentPosY:
	case PropertyComponentPosZ:
	case PropertyComponentRot:
	case PropertyComponentRotX:
	case PropertyComponentRotY:
	case PropertyComponentRotZ:
	case PropertyComponentScale:
	case PropertyComponentScaleX:
	case PropertyComponentScaleY:
	case PropertyComponentScaleZ:
		{
			Component * cmp = NULL;
			CMFCPropertyGridProperty * pAABB = NULL, * pPosition = NULL, * pRotation = NULL, * pScale = NULL;
			switch (PropertyId)
			{
			case PropertyComponentAABB:
			case PropertyComponentPos:
			case PropertyComponentRot:
			case PropertyComponentScale:
				cmp = (Component *)pProp->GetParent()->GetValue().ulVal;
				pAABB = pProp->GetParent()->GetSubItem(0);
				pPosition = pProp->GetParent()->GetSubItem(1);
				pRotation = pProp->GetParent()->GetSubItem(2);
				pScale = pProp->GetParent()->GetSubItem(3);
				break;
			default:
				cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
				pAABB = pProp->GetParent()->GetParent()->GetSubItem(0);
				pPosition = pProp->GetParent()->GetParent()->GetSubItem(1);
				pRotation = pProp->GetParent()->GetParent()->GetSubItem(2);
				pScale = pProp->GetParent()->GetParent()->GetSubItem(3);
				break;
			}
			cmp->m_aabb.m_min.x = pAABB->GetSubItem(0)->GetValue().fltVal;
			cmp->m_aabb.m_min.y = pAABB->GetSubItem(1)->GetValue().fltVal;
			cmp->m_aabb.m_min.z = pAABB->GetSubItem(2)->GetValue().fltVal;
			cmp->m_aabb.m_max.x = pAABB->GetSubItem(3)->GetValue().fltVal;
			cmp->m_aabb.m_max.y = pAABB->GetSubItem(4)->GetValue().fltVal;
			cmp->m_aabb.m_max.z = pAABB->GetSubItem(5)->GetValue().fltVal;
			cmp->m_Position.x = pPosition->GetSubItem(0)->GetValue().fltVal;
			cmp->m_Position.y = pPosition->GetSubItem(1)->GetValue().fltVal;
			cmp->m_Position.z = pPosition->GetSubItem(2)->GetValue().fltVal;
			my::Quaternion rot = my::Quaternion::RotationEulerAngles(my::Vector3(
				pRotation->GetSubItem(0)->GetValue().fltVal,
				pRotation->GetSubItem(1)->GetValue().fltVal,
				pRotation->GetSubItem(2)->GetValue().fltVal));
			cmp->m_Rotation = rot;
			cmp->m_Scale.x = pScale->GetSubItem(0)->GetValue().fltVal;
			cmp->m_Scale.y = pScale->GetSubItem(1)->GetValue().fltVal;
			cmp->m_Scale.z = pScale->GetSubItem(2)->GetValue().fltVal;
			Actor * actor = dynamic_cast<Actor *>(cmp->GetTopParent());
			actor->Update(0);
			pFrame->OnActorPosChanged(actor);
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMeshResPath:
		{
			//MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			//mesh_cmp->m_MeshRes.ReleaseResource();
			//mesh_cmp->m_MeshRes.m_Path = ts2ms(pProp->GetValue().bstrVal);
			//mesh_cmp->m_MeshRes.RequestResource();
			//EventArg arg;
			//pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMeshAnimation:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			mesh_cmp->m_bAnimation = pProp->GetValue().boolVal != 0;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMeshInstance:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			mesh_cmp->m_bInstance = pProp->GetValue().boolVal != 0;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMeshStaticCollision:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			mesh_cmp->m_StaticCollision = pProp->GetValue().boolVal != 0;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	//case PropertyEmitterParticleCount:
	//	{
	//		EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
	//		emit_cmp->m_Emitter->m_ParticleList.resize(pProp->GetValue().uintVal,
	//			my::Emitter::Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector4(1,1,1,1), my::Vector2(10,10), 0, 0));
	//		UpdatePropertiesEmitterParticleList(m_pProp[PropertyEmitterParticleList], emit_cmp->m_Emitter->m_ParticleList);
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyEmitterParticlePosition:
	//case PropertyEmitterParticlePositionX:
	//case PropertyEmitterParticlePositionY:
	//case PropertyEmitterParticlePositionZ:
	//case PropertyEmitterParticleVelocity:
	//case PropertyEmitterParticleVelocityX:
	//case PropertyEmitterParticleVelocityY:
	//case PropertyEmitterParticleVelocityZ:
	//case PropertyEmitterParticleColor:
	//case PropertyEmitterParticleColorA:
	//case PropertyEmitterParticleColorR:
	//case PropertyEmitterParticleColorG:
	//case PropertyEmitterParticleColorB:
	//case PropertyEmitterParticleSize:
	//case PropertyEmitterParticleSizeX:
	//case PropertyEmitterParticleSizeY:
	//case PropertyEmitterParticleAngle:
	//	{
	//		CMFCPropertyGridProperty * pParticle = NULL;
	//		switch (PropertyId)
	//		{
	//		case PropertyEmitterParticlePositionX:
	//		case PropertyEmitterParticlePositionY:
	//		case PropertyEmitterParticlePositionZ:
	//		case PropertyEmitterParticleVelocityX:
	//		case PropertyEmitterParticleVelocityY:
	//		case PropertyEmitterParticleVelocityZ:
	//		case PropertyEmitterParticleColorA:
	//		case PropertyEmitterParticleColorR:
	//		case PropertyEmitterParticleColorG:
	//		case PropertyEmitterParticleColorB:
	//		case PropertyEmitterParticleSizeX:
	//		case PropertyEmitterParticleSizeY:
	//			pParticle = pProp->GetParent()->GetParent();
	//			break;
	//		case PropertyEmitterParticlePosition:
	//		case PropertyEmitterParticleVelocity:
	//		case PropertyEmitterParticleColor:
	//		case PropertyEmitterParticleSize:
	//		case PropertyEmitterParticleAngle:
	//			pParticle = pProp->GetParent();
	//			break;
	//		}
	//		DWORD NodeId = pParticle->GetData();
	//		EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
	//		my::Emitter::Particle & particle = emit_cmp->m_Emitter->m_ParticleList[NodeId];
	//		particle.m_Position.x = pParticle->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal;
	//		particle.m_Position.y = pParticle->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal;
	//		particle.m_Position.z = pParticle->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal;
	//		particle.m_Velocity.x = pParticle->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
	//		particle.m_Velocity.y = pParticle->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
	//		particle.m_Velocity.z = pParticle->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
	//		particle.m_Color.x = pParticle->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal;
	//		particle.m_Color.y = pParticle->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal;
	//		particle.m_Color.z = pParticle->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal;
	//		particle.m_Color.w = pParticle->GetSubItem(2)->GetSubItem(3)->GetValue().fltVal;
	//		particle.m_Size.x = pParticle->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal;
	//		particle.m_Size.y = pParticle->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal;
	//		particle.m_Angle = pParticle->GetSubItem(4)->GetValue().fltVal;
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertySphericalEmitterParticleLifeTime:
	//	{
	//		EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
	//		my::DynamicEmitter * dynamic_emit = dynamic_cast<my::DynamicEmitter *>(emit_cmp->m_Emitter.get());
	//		dynamic_emit->m_ParticleLifeTime = pProp->GetValue().fltVal;
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertySphericalEmitterSpawnInterval:
	//case PropertySphericalEmitterHalfSpawnAreaX:
	//case PropertySphericalEmitterHalfSpawnAreaY:
	//case PropertySphericalEmitterHalfSpawnAreaZ:
	//case PropertySphericalEmitterSpawnSpeed:
	//case PropertySphericalEmitterSpawnLoopTime:
	//	{
	//		EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
	//		my::SphericalEmitter * spherical_emit = dynamic_cast<my::SphericalEmitter *>(emit_cmp->m_Emitter.get());
	//		spherical_emit->m_SpawnInterval = m_pProp[PropertySphericalEmitterSpawnInterval]->GetValue().fltVal;
	//		spherical_emit->m_HalfSpawnArea.x = m_pProp[PropertySphericalEmitterHalfSpawnAreaX]->GetValue().fltVal;
	//		spherical_emit->m_HalfSpawnArea.y = m_pProp[PropertySphericalEmitterHalfSpawnAreaY]->GetValue().fltVal;
	//		spherical_emit->m_HalfSpawnArea.z = m_pProp[PropertySphericalEmitterHalfSpawnAreaZ]->GetValue().fltVal;
	//		spherical_emit->m_SpawnSpeed = m_pProp[PropertySphericalEmitterSpawnSpeed]->GetValue().fltVal;
	//		spherical_emit->m_SpawnLoopTime = m_pProp[PropertySphericalEmitterSpawnLoopTime]->GetValue().fltVal;
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertySplineNodeCount:
	//case PropertySplineNodeX:
	//case PropertySplineNodeY:
	//case PropertySplineNodeK0:
	//case PropertySplineNodeK:
	//	{
	//		EmitterComponent * emit_cmp = dynamic_cast<EmitterComponent *>(cmp);
	//		my::SphericalEmitter * spherical_emit = dynamic_cast<my::SphericalEmitter *>(emit_cmp->m_Emitter.get());
	//		CMFCPropertyGridProperty * pSpline = NULL;
	//		switch (PropertyId)
	//		{
	//		case PropertySplineNodeCount:
	//			pSpline = pProp->GetParent();
	//			break;
	//		case PropertySplineNodeX:
	//		case PropertySplineNodeY:
	//		case PropertySplineNodeK0:
	//		case PropertySplineNodeK:
	//			pSpline = pProp->GetParent()->GetParent();
	//			break;
	//		}
	//		my::Spline * spline = NULL;
	//		switch (pSpline->GetData())
	//		{
	//		case PropertySphericalEmitterSpawnInclination:
	//			spline = &spherical_emit->m_SpawnInclination;
	//			break;
	//		case PropertySphericalEmitterSpawnAzimuth:
	//			spline = &spherical_emit->m_SpawnAzimuth;
	//			break;
	//		case PropertySphericalEmitterSpawnColorR:
	//			spline = &spherical_emit->m_SpawnColorR;
	//			break;
	//		case PropertySphericalEmitterSpawnColorG:
	//			spline = &spherical_emit->m_SpawnColorG;
	//			break;
	//		case PropertySphericalEmitterSpawnColorB:
	//			spline = &spherical_emit->m_SpawnColorB;
	//			break;
	//		case PropertySphericalEmitterSpawnColorA:
	//			spline = &spherical_emit->m_SpawnColorA;
	//			break;
	//		case PropertySphericalEmitterSpawnSizeX:
	//			spline = &spherical_emit->m_SpawnSizeX;
	//			break;
	//		case PropertySphericalEmitterSpawnSizeY:
	//			spline = &spherical_emit->m_SpawnSizeY;
	//			break;
	//		case PropertySphericalEmitterSpawnAngle:
	//			spline = &spherical_emit->m_SpawnAngle;
	//			break;
	//		}
	//		switch (PropertyId)
	//		{
	//		case PropertySplineNodeCount:
	//			spline->resize(pProp->GetValue().uintVal, my::SplineNode(0, 0, 0, 0));
	//			UpdatePropertiesSpline((Property)pSpline->GetData(), spline);
	//			break;
	//		case PropertySplineNodeX:
	//		case PropertySplineNodeY:
	//		case PropertySplineNodeK0:
	//		case PropertySplineNodeK:
	//			{
	//				CMFCPropertyGridProperty * pNode = pProp->GetParent();
	//				DWORD id = pNode->GetData();
	//				_ASSERT(id < spline->size());
	//				my::SplineNode & node = (*spline)[id];
	//				node.x = pNode->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->GetValue().fltVal;
	//				node.y = pNode->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->GetValue().fltVal;
	//				node.k0 = pNode->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->GetValue().fltVal;
	//				node.k = pNode->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->GetValue().fltVal;
	//			}
	//			break;
	//		}
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	case PropertyMaterialShader:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_Shader = ts2ms(pProp->GetValue().bstrVal);
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialPassMask:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
			if (i < 0 || i >= _countof(g_PassMaskDesc))
			{
				TRACE("Invalid PropertyMaterialPassMask index: %d\n", i);
				break;
			}
			material->m_PassMask = g_PassMaskDesc[i].mask;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialMeshColor:
	case PropertyMaterialMeshColorA:
	case PropertyMaterialMeshColorR:
	case PropertyMaterialMeshColorG:
	case PropertyMaterialMeshColorB:
		{
			CMFCPropertyGridProperty * pColor = NULL;
			switch (PropertyId)
			{
			case PropertyMaterialMeshColor:
				pColor = pProp;
				break;
			case PropertyMaterialMeshColorA:
			case PropertyMaterialMeshColorR:
			case PropertyMaterialMeshColorG:
			case PropertyMaterialMeshColorB:
				pColor = pProp->GetParent();
				break;
			}
			Material * material = (Material *)pColor->GetParent()->GetValue().ulVal;
			material->m_MeshColor.x = pColor->GetSubItem(0)->GetValue().fltVal;
			material->m_MeshColor.y = pColor->GetSubItem(1)->GetValue().fltVal;
			material->m_MeshColor.z = pColor->GetSubItem(2)->GetValue().fltVal;
			material->m_MeshColor.w = pColor->GetSubItem(3)->GetValue().fltVal;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialMeshTexture:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_MeshTexture.ReleaseResource();
			material->m_MeshTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_MeshTexture.RequestResource();
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialNormalTexture:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_NormalTexture.ReleaseResource();
			material->m_NormalTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_NormalTexture.RequestResource();
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialSpecularTexture:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_SpecularTexture.ReleaseResource();
			material->m_SpecularTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_SpecularTexture.RequestResource();
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	////case PropertyRigidShapeAdd:
	////	{
	////		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	////		DWORD NodeId = rigid_cmp->m_RigidActor->getNbShapes();
	////		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
	////		PxShape * shape = NULL;
	////		switch (i)
	////		{
	////		case PxGeometryType::eSPHERE:
	////			shape = rigid_cmp->m_RigidActor->createShape(PxSphereGeometry(1.0f),
	////				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	////			break;
	////		case PxGeometryType::ePLANE:
	////			shape = rigid_cmp->m_RigidActor->createShape(PxPlaneGeometry(),
	////				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	////			break;
	////		case PxGeometryType::eCAPSULE:
	////			shape = rigid_cmp->m_RigidActor->createShape(PxCapsuleGeometry(1.0f, 1.0f),
	////				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	////			break;
	////		case PxGeometryType::eBOX:
	////			shape = rigid_cmp->m_RigidActor->createShape(PxBoxGeometry(PxVec3(1,1,1)),
	////				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	////			break;
	////		case PxGeometryType::eCONVEXMESH:
	////			break;
	////		case PxGeometryType::eTRIANGLEMESH:
	////			break;
	////		case PxGeometryType::eHEIGHTFIELD:
	////			break;
	////		}
	////		if (shape)
	////		{
	////			CreatePropertiesShape(m_pProp[PropertyRigidShapeList], NodeId, (PxGeometryType::Enum)i);
	////			UpdatePropertiesShape(m_pProp[PropertyRigidShapeList], NodeId, shape);
	////		}
	////		EventArg arg;
	////		pFrame->m_EventAttributeChanged(&arg);
	////	}
	////	break;
	////case PropertyRigidShapePos:
	////case PropertyRigidShapePosX:
	////case PropertyRigidShapePosY:
	////case PropertyRigidShapePosZ:
	////case PropertyRigidShapeRot:
	////case PropertyRigidShapeRotX:
	////case PropertyRigidShapeRotY:
	////case PropertyRigidShapeRotZ:
	////	{
	////		CMFCPropertyGridProperty * pShape = NULL;
	////		switch (PropertyId)
	////		{
	////		case PropertyRigidShapePos:
	////		case PropertyRigidShapeRot:
	////			pShape = pProp->GetParent();
	////			break;
	////		case PropertyRigidShapePosX:
	////		case PropertyRigidShapePosY:
	////		case PropertyRigidShapePosZ:
	////		case PropertyRigidShapeRotX:
	////		case PropertyRigidShapeRotY:
	////		case PropertyRigidShapeRotZ:
	////			pShape = pProp->GetParent()->GetParent();
	////			break;
	////		}
	////		unsigned int NodeId = LOWORD(pShape->GetData());
	////		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	////		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	////		std::vector<PxShape *> shapes(NbShapes);
	////		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	////		my::Quaternion rot = my::Quaternion::RotationEulerAngles(my::Vector3(
	////			D3DXToRadian(pShape->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal),
	////			D3DXToRadian(pShape->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal),
	////			D3DXToRadian(pShape->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal)));
	////		shapes[NodeId]->setLocalPose(PxTransform(PxVec3(
	////			pShape->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal,
	////			pShape->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal,
	////			pShape->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal), (PxQuat&)rot));
	////		EventArg arg;
	////		pFrame->m_EventAttributeChanged(&arg);
	////	}
	////	break;
	////case PropertyRigidShapeCapsuleRadius:
	////case PropertyRigidShapeCapsuleHalfHeight:
	////	{
	////		CMFCPropertyGridProperty * pShape = pProp->GetParent();
	////		unsigned int NodeId = LOWORD(pShape->GetData());
	////		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	////		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	////		std::vector<PxShape *> shapes(NbShapes);
	////		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	////		shapes[NodeId]->setGeometry(PxCapsuleGeometry(
	////			pShape->GetSubItem(2)->GetValue().fltVal,
	////			pShape->GetSubItem(3)->GetValue().fltVal));
	////		EventArg arg;
	////		pFrame->m_EventAttributeChanged(&arg);
	////	}
	////	break;
	////case PropertyRigidShapeBoxHalfExtents:
	////case PropertyRigidShapeBoxHalfExtentsX:
	////case PropertyRigidShapeBoxHalfExtentsY:
	////case PropertyRigidShapeBoxHalfExtentsZ:
	////	{
	////		CMFCPropertyGridProperty * pShape = NULL;
	////		switch (PropertyId)
	////		{
	////		case PropertyRigidShapeBoxHalfExtents:
	////			pShape = pProp->GetParent();
	////			break;
	////		case PropertyRigidShapeBoxHalfExtentsX:
	////		case PropertyRigidShapeBoxHalfExtentsY:
	////		case PropertyRigidShapeBoxHalfExtentsZ:
	////			pShape = pProp->GetParent()->GetParent();
	////			break;
	////		}
	////		unsigned int NodeId = LOWORD(pShape->GetData());
	////		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	////		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	////		std::vector<PxShape *> shapes(NbShapes);
	////		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	////		shapes[NodeId]->setGeometry(PxBoxGeometry(PxVec3(
	////			pShape->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal,
	////			pShape->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal,
	////			pShape->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal)));
	////		EventArg arg;
	////		pFrame->m_EventAttributeChanged(&arg);
	////	}
	////	break;
	////case PropertyRigidShapeSphereRadius:
	////	{
	////		CMFCPropertyGridProperty * pShape = pProp->GetParent();
	////		unsigned int NodeId = LOWORD(pShape->GetData());
	////		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	////		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	////		std::vector<PxShape *> shapes(NbShapes);
	////		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	////		shapes[NodeId]->setGeometry(PxSphereGeometry(
	////			pShape->GetSubItem(2)->GetValue().fltVal));
	////		EventArg arg;
	////		pFrame->m_EventAttributeChanged(&arg);
	////	}
	////	break;
	//case PropertyTerrainHeightScale:
	//	{
	//		Terrain * terrain = dynamic_cast<Terrain *>(cmp);
	//		terrain->m_HeightScale = m_pProp[PropertyTerrainHeightScale]->GetValue().fltVal;
	//		terrain->UpdateHeightMapNormal();
	//		terrain->UpdateChunks();
	//		pFrame->OnActorPosChanged(cmp);
	//		pFrame->OnSelActorsChanged();
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyTerrainWrappedU:
	//case PropertyTerrainWrappedV:
	//	{
	//		Terrain * terrain = dynamic_cast<Terrain *>(cmp);
	//		terrain->m_WrappedU = m_pProp[PropertyTerrainWrappedU]->GetValue().fltVal;
	//		terrain->m_WrappedV = m_pProp[PropertyTerrainWrappedV]->GetValue().fltVal;
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyTerrainHeightMap:
	//	{
	//		std::string path = ts2ms(pProp->GetValue().bstrVal);
	//		my::Texture2DPtr res = boost::dynamic_pointer_cast<my::Texture2D>(theApp.LoadTexture(path));
	//		if (res)
	//		{
	//			Terrain * terrain = dynamic_cast<Terrain *>(cmp);
	//			terrain->UpdateHeightMap(res);
	//			pFrame->OnActorPosChanged(cmp);
	//			EventArg arg;
	//			pFrame->m_EventAttributeChanged(&arg);
	//		}
	//	}
	//	break;
	//case PropertyTerrainStaticCollision:
	//	{
	//		Terrain * terrain = dynamic_cast<Terrain *>(cmp);
	//		terrain->m_StaticCollision = m_pProp[PropertyTerrainStaticCollision]->GetValue().boolVal;
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	}
	return 0;
}


#include "stdafx.h"

#include "PropertiesWnd.h"
#include "CtrlProps.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "ShapeDlg.h"

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
	{_T("None"), Material::PassMaskNone},
	{_T("Light"), Material::PassMaskLight},
	{_T("Opaque"), Material::PassMaskOpaque},
	{_T("Transparent"), Material::PassMaskTransparent},
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

static LPCTSTR g_ShapeTypeDesc[physx::PxGeometryType::eGEOMETRY_COUNT + 1] =
{
	_T("Sphere"),
	_T("Plane"),
	_T("Capsule"),
	_T("Box"),
	_T("ConvexMesh"),
	_T("TriangleMesh"),
	_T("HeightField"),
	_T("None")
};

static LPCTSTR g_ActorTypeDesc[physx::PxActorType::eACTOR_COUNT + 1] =
{
	_T("RigidStatic"),
	_T("RigidDynamic"),
	_T("ParticleSystem"),
	_T("ParticleFluid"),
	_T("ArticulationLink"),
	_T("Cloth"),
	_T("None")
};

static LPCTSTR g_CullModeDesc[] =
{
	_T("D3DCULL_NONE"),
	_T("D3DCULL_CW"),
	_T("D3DCULL_CCW")
};

static LPCTSTR g_BlendModeDesc[] =
{
	_T("BlendModeNone"),
	_T("BlendModeAlpha"),
	_T("BlendModeAdditive")
};

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

void CPropertiesWnd::OnSelectionChanged(EventArgs * arg)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	if (!pFrame->m_selactors.empty())
	{
		UpdatePropertiesActor(*pFrame->m_selactors.begin());
		m_wndPropList.AdjustLayout();
	}
	else
	{
		m_wndPropList.RemoveAll();
		m_wndPropList.AdjustLayout();
	}
}

void CPropertiesWnd::OnCmpAttriChanged(EventArgs * arg)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMainFrame::ActorSet::iterator actor_iter = pFrame->m_selactors.begin();
	if (actor_iter != pFrame->m_selactors.end())
	{
		UpdatePropertiesActor(*actor_iter);
		m_wndPropList.AdjustLayout();
	}
}

void CPropertiesWnd::RemovePropertiesFrom(CMFCPropertyGridProperty * pParentCtrl, int i)
{
	if (pParentCtrl)
	{
		while (pParentCtrl->GetSubItemsCount() > i)
		{
			CMFCPropertyGridProperty * pProp = pParentCtrl->GetSubItem(i);
			static_cast<CMFCPropertyGridPropertyReader *>(pParentCtrl)->RemoveSubItem(pProp, TRUE);
		}
	}
	else
	{
		while (m_wndPropList.GetPropertyCount() > i)
		{
			CMFCPropertyGridProperty * pProp = m_wndPropList.GetProperty(i);
			static_cast<CMFCPropertyGridCtrlReader &>(m_wndPropList).DeleteProperty(pProp, FALSE, FALSE);
		}
	}
}

void CPropertiesWnd::UpdatePropertiesActor(Actor * actor)
{
	CMFCPropertyGridProperty * pComponent = NULL;
	if (m_wndPropList.GetPropertyCount() >= 1)
	{
		pComponent = m_wndPropList.GetProperty(0);
	}
	if (!pComponent || pComponent->GetData() != PropertyActor)
	{
		m_wndPropList.RemoveAll();
		CreatePropertiesActor(actor);
		return;
	}
	pComponent->SetName(GetComponentTypeName(Component::ComponentTypeActor), FALSE);
	pComponent->SetValue((_variant_t)(DWORD_PTR)actor);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pComponent->GetSubItem(0)->GetSubItem(0)->SetValue((_variant_t)actor->m_aabb.m_min.x);
	pComponent->GetSubItem(0)->GetSubItem(1)->SetValue((_variant_t)actor->m_aabb.m_min.y);
	pComponent->GetSubItem(0)->GetSubItem(2)->SetValue((_variant_t)actor->m_aabb.m_min.z);
	pComponent->GetSubItem(0)->GetSubItem(3)->SetValue((_variant_t)actor->m_aabb.m_max.x);
	pComponent->GetSubItem(0)->GetSubItem(4)->SetValue((_variant_t)actor->m_aabb.m_max.y);
	pComponent->GetSubItem(0)->GetSubItem(5)->SetValue((_variant_t)actor->m_aabb.m_max.z);
	pComponent->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)actor->m_Position.x);
	pComponent->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)actor->m_Position.y);
	pComponent->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)actor->m_Position.z);
	my::Vector3 angle = actor->m_Rotation.ToEulerAngles();
	pComponent->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pComponent->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pComponent->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pComponent->GetSubItem(3)->GetSubItem(0)->SetValue((_variant_t)actor->m_Scale.x);
	pComponent->GetSubItem(3)->GetSubItem(1)->SetValue((_variant_t)actor->m_Scale.y);
	pComponent->GetSubItem(3)->GetSubItem(2)->SetValue((_variant_t)actor->m_Scale.z);
	pComponent->GetSubItem(4)->SetValue((_variant_t)g_ActorTypeDesc[actor->m_PxActor ? actor->m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT]);
	if (!actor->m_Cmps.empty())
	{
		Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
		for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
		{
			UpdateProperties(pComponent, GetComponentPropCount(Component::ComponentTypeActor) + std::distance(actor->m_Cmps.begin(), cmp_iter), cmp_iter->get());
		}
	}
	RemovePropertiesFrom(pComponent, GetComponentPropCount(Component::ComponentTypeActor) + actor->m_Cmps.size());
	//m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::UpdateProperties(CMFCPropertyGridProperty * pParentCtrl, int i, Component * cmp)
{
	CMFCPropertyGridProperty * pComponent = NULL;
	if (i < pParentCtrl->GetSubItemsCount())
	{
		pComponent = pParentCtrl->GetSubItem(i);
	}
	if (!pComponent || pComponent->GetData() != PropertyComponent)
	{
		RemovePropertiesFrom(pParentCtrl, i);
		CreateProperties(pParentCtrl, i, cmp);
		return;
	}
	pComponent->SetName(GetComponentTypeName(cmp->m_Type), FALSE);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp);

	pComponent->GetSubItem(0)->SetValue((_variant_t)g_ShapeTypeDesc[cmp->m_PxShape ? cmp->m_PxShape->getGeometryType() : physx::PxGeometryType::eGEOMETRY_COUNT]);

	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		UpdatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeCloth:
		UpdatePropertiesCloth(pComponent, dynamic_cast<ClothComponent *>(cmp));
		break;
	case Component::ComponentTypeStaticEmitter:
		UpdatePropertiesStaticEmitter(pComponent, dynamic_cast<EmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		UpdatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		UpdatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyMeshResPath)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesMesh(pComponent, mesh_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(mesh_cmp->m_MeshRes.m_Path).c_str());
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)(VARIANT_BOOL)mesh_cmp->m_bInstance);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)(VARIANT_BOOL)mesh_cmp->m_bUseAnimation);
	CMFCPropertyGridProperty * pMaterialList = pComponent->GetSubItem(PropId + 3);
	for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
	{
		if ((unsigned int)pMaterialList->GetSubItemsCount() <= i)
		{
			CreatePropertiesMaterial(pMaterialList, i, mesh_cmp->m_MaterialList[i].get());
			continue;
		}
		UpdatePropertiesMaterial(pMaterialList, i, mesh_cmp->m_MaterialList[i].get());
	}
	RemovePropertiesFrom(pMaterialList, mesh_cmp->m_MaterialList.size());
}

void CPropertiesWnd::UpdatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, int NodeId, Material * mat)
{
	CMFCPropertyGridProperty * pMaterial = pParentCtrl->GetSubItem(NodeId);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mat);
	pMaterial->GetSubItem(0)->SetValue((_variant_t)mat->m_Shader.c_str());
	pMaterial->GetSubItem(1)->SetValue((_variant_t)GetPassMaskDesc(mat->m_PassMask));
	pMaterial->GetSubItem(2)->SetValue((_variant_t)g_CullModeDesc[mat->m_CullMode - 1]);
	pMaterial->GetSubItem(3)->SetValue((_variant_t)(VARIANT_BOOL)mat->m_ZEnable);
	pMaterial->GetSubItem(4)->SetValue((_variant_t)(VARIANT_BOOL)mat->m_ZWriteEnable);
	pMaterial->GetSubItem(5)->SetValue((_variant_t)g_BlendModeDesc[mat->m_BlendMode]);
	COLORREF color = RGB(mat->m_MeshColor.x * 255, mat->m_MeshColor.y * 255, mat->m_MeshColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pMaterial->GetSubItem(6)))->SetColor(color);
	pMaterial->GetSubItem(7)->SetValue((_variant_t)mat->m_MeshColor.w);
	pMaterial->GetSubItem(8)->SetValue((_variant_t)mat->m_MeshTexture.m_Path.c_str());
	pMaterial->GetSubItem(9)->SetValue((_variant_t)mat->m_NormalTexture.m_Path.c_str());
	pMaterial->GetSubItem(10)->SetValue((_variant_t)mat->m_SpecularTexture.m_Path.c_str());
}

void CPropertiesWnd::UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyClothSceneCollision)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesCloth(pComponent, cloth_cmp);
	}
	physx::PxClothFlags flags = cloth_cmp->m_Cloth->getClothFlags();
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)(VARIANT_BOOL)flags.isSet(physx::PxClothFlag::eSCENE_COLLISION));
	CMFCPropertyGridProperty * pMaterialList = pComponent->GetSubItem(PropId + 1);
	for (unsigned int i = 0; i < cloth_cmp->m_MaterialList.size(); i++)
	{
		if ((unsigned int)pMaterialList->GetSubItemsCount() <= i)
		{
			CreatePropertiesMaterial(pMaterialList, i, cloth_cmp->m_MaterialList[i].get());
			continue;
		}
		UpdatePropertiesMaterial(pMaterialList, i, cloth_cmp->m_MaterialList[i].get());
	}
	RemovePropertiesFrom(pMaterialList, cloth_cmp->m_MaterialList.size());
}

void CPropertiesWnd::UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId + 0);
	if (!pProp || pProp->GetData() != PropertyEmitterParticleList)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesStaticEmitter(pComponent, emit_cmp);
		return;
	}
	pProp->GetSubItem(0)->SetValue((_variant_t)emit_cmp->m_Emitter->m_ParticleList.size());
	for (unsigned int i = 0; i < emit_cmp->m_Emitter->m_ParticleList.size(); i++)
	{
		if ((unsigned int)pProp->GetSubItemsCount() <= i + 1)
		{
			CreatePropertiesStaticEmitterParticle(pProp, i, emit_cmp);
			continue;
		}
		UpdatePropertiesStaticEmitterParticle(pProp, i, emit_cmp);
	}
	RemovePropertiesFrom(pProp, 1 + emit_cmp->m_Emitter->m_ParticleList.size());
	UpdatePropertiesMaterial(pComponent, PropId + 1, emit_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp)
{
	CMFCPropertyGridProperty * pParticle = pParentProp->GetSubItem(NodeId + 1);
	_ASSERT(pParticle);
	my::Emitter::Particle & particle = emit_cmp->m_Emitter->m_ParticleList[NodeId];
	CMFCPropertyGridProperty * pProp = pParticle->GetSubItem(0)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionX); pProp->SetValue((_variant_t)particle.m_Position.x);
	pProp = pParticle->GetSubItem(0)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionY); pProp->SetValue((_variant_t)particle.m_Position.y);
	pProp = pParticle->GetSubItem(0)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionZ); pProp->SetValue((_variant_t)particle.m_Position.z);
	pProp = pParticle->GetSubItem(1)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityX); pProp->SetValue((_variant_t)particle.m_Velocity.x);
	pProp = pParticle->GetSubItem(1)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityY); pProp->SetValue((_variant_t)particle.m_Velocity.y);
	pProp = pParticle->GetSubItem(1)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityZ); pProp->SetValue((_variant_t)particle.m_Velocity.z);
	COLORREF color = RGB(particle.m_Color.x * 255, particle.m_Color.y * 255, particle.m_Color.z * 255);
	pProp = pParticle->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleColor); (DYNAMIC_DOWNCAST(CColorProp, pProp))->SetColor(color);
	pProp = pParticle->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorAlpha); pProp->SetValue((_variant_t)particle.m_Color.w);
	pProp = pParticle->GetSubItem(4)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeX); pProp->SetValue((_variant_t)particle.m_Size.x);
	pProp = pParticle->GetSubItem(4)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeY); pProp->SetValue((_variant_t)particle.m_Size.y);
	pProp = pParticle->GetSubItem(5); _ASSERT(pProp->GetData() == PropertyEmitterParticleAngle); pProp->SetValue((_variant_t)particle.m_Angle);
}

void CPropertiesWnd::UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertySphericalEmitterParticleLifeTime)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesSphericalEmitter(pComponent, sphe_emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleLifeTime);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnInterval);
	pComponent->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.x);
	pComponent->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.y);
	pComponent->GetSubItem(PropId + 2)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.z);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnSpeed);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 4), &sphe_emit_cmp->m_SpawnInclination);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 5), &sphe_emit_cmp->m_SpawnAzimuth);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 6), &sphe_emit_cmp->m_SpawnColorR);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 7), &sphe_emit_cmp->m_SpawnColorG);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 8), &sphe_emit_cmp->m_SpawnColorB);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 9), &sphe_emit_cmp->m_SpawnColorA);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 10), &sphe_emit_cmp->m_SpawnSizeX);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 11), &sphe_emit_cmp->m_SpawnSizeY);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 12), &sphe_emit_cmp->m_SpawnAngle);
	pComponent->GetSubItem(PropId + 13)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnLoopTime);
	UpdatePropertiesMaterial(pComponent, PropId + 14, sphe_emit_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline)
{
	_ASSERT(pSpline->GetData() >= PropertySphericalEmitterSpawnInclination && pSpline->GetData() <= PropertySphericalEmitterSpawnAngle);
	pSpline->SetValue((_variant_t)(DWORD_PTR)spline);
	pSpline->GetSubItem(0)->SetValue((_variant_t)spline->size());
	unsigned int i = 0;
	for (; i < spline->size(); i++)
	{
		if ((unsigned int)pSpline->GetSubItemsCount() <= i + 1)
		{
			CreatePropertiesSplineNode(pSpline, i, &(*spline)[i]);
		}
		UpdatePropertiesSplineNode(pSpline, i, &(*spline)[i]);
	}
	RemovePropertiesFrom(pSpline, i + 1);
}

void CPropertiesWnd::UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::SplineNode * node)
{
	CMFCPropertyGridProperty * pProp = pSpline->GetSubItem(NodeId + 1);
	_ASSERT(pProp);
	pProp->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->SetValue((_variant_t)node->x);
	pProp->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->SetValue((_variant_t)node->y);
	pProp->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->SetValue((_variant_t)node->k0);
	pProp->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->SetValue((_variant_t)node->k);
}

void CPropertiesWnd::UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyTerrainRowChunks)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesTerrain(pComponent, terrain);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)terrain->ROW_CHUNKS);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)terrain->COL_CHUNKS);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)terrain->CHUNK_SIZE);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)terrain->m_HeightScale);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)terrain->m_WrappedU);
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)terrain->m_WrappedV);
	pComponent->GetSubItem(PropId + 6);
	UpdatePropertiesMaterial(pComponent, PropId + 7, terrain->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesActor(Actor * actor)
{
	CMFCPropertyGridProperty * pComponent = new CSimpleProp(GetComponentTypeName(Component::ComponentTypeActor), PropertyActor, FALSE);
	m_wndPropList.AddProperty(pComponent, FALSE, FALSE);
	pComponent->SetValue((_variant_t)(DWORD_PTR)actor); // ! only worked on 32bit system

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMFCPropertyGridProperty * pAABB = new CSimpleProp(_T("AABB"), PropertyActorAABB, TRUE);
	pComponent->AddSubItem(pAABB);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("minx"), (_variant_t)actor->m_aabb.m_min.x, NULL, PropertyActorMinX);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("miny"), (_variant_t)actor->m_aabb.m_min.y, NULL, PropertyActorMinY);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("minz"), (_variant_t)actor->m_aabb.m_min.z, NULL, PropertyActorMinZ);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxx"), (_variant_t)actor->m_aabb.m_max.x, NULL, PropertyActorMaxX);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxy"), (_variant_t)actor->m_aabb.m_max.y, NULL, PropertyActorMaxY);
	pAABB->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("maxz"), (_variant_t)actor->m_aabb.m_max.z, NULL, PropertyActorMaxZ);
	pAABB->AddSubItem(pProp);

	CMFCPropertyGridProperty * pPosition = new CMFCPropertyGridProperty(_T("Position"), PropertyActorPos, TRUE);
	pComponent->AddSubItem(pPosition);
	pProp = new CSimpleProp(_T("x"), (_variant_t)actor->m_Position.x, NULL, PropertyActorPosX);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)actor->m_Position.y, NULL, PropertyActorPosY);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)actor->m_Position.z, NULL, PropertyActorPosZ);
	pPosition->AddSubItem(pProp);

	my::Vector3 angle = actor->m_Rotation.ToEulerAngles();
	CMFCPropertyGridProperty * pRotate = new CSimpleProp(_T("Rotate"), PropertyActorRot, TRUE);
	pComponent->AddSubItem(pRotate);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertyActorRotX);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertyActorRotY);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertyActorRotZ);
	pRotate->AddSubItem(pProp);

	CMFCPropertyGridProperty * pScale = new CSimpleProp(_T("Scale"), PropertyActorScale, TRUE);
	pComponent->AddSubItem(pScale);
	pProp = new CSimpleProp(_T("x"), (_variant_t)actor->m_Scale.x, NULL, PropertyActorScaleX);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)actor->m_Scale.y, NULL, PropertyActorScaleY);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)actor->m_Scale.z, NULL, PropertyActorScaleZ);
	pScale->AddSubItem(pProp);

	CMFCPropertyGridProperty * pRigidActor = new CComboProp(_T("RigidActor"), g_ActorTypeDesc[actor->m_PxActor ? actor->m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT], NULL, PropertyActorRigidActor);
	for (unsigned int i = 0; i < _countof(g_ActorTypeDesc); i++)
	{
		pRigidActor->AddOption(g_ActorTypeDesc[i], TRUE);
	}
	pComponent->AddSubItem(pRigidActor);

	if (!actor->m_Cmps.empty())
	{
		Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
		for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
		{
			CreateProperties(pComponent, GetComponentPropCount(Component::ComponentTypeActor) + std::distance(actor->m_Cmps.begin(), cmp_iter), cmp_iter->get());
		}
	}
}

void CPropertiesWnd::CreateProperties(CMFCPropertyGridProperty * pParentCtrl, int i, Component * cmp)
{
	CMFCPropertyGridProperty * pComponent = NULL;
	while (i >= pParentCtrl->GetSubItemsCount())
	{
		pComponent = new CSimpleProp(GetComponentTypeName(cmp->m_Type), PropertyComponent, FALSE);
		pParentCtrl->AddSubItem(pComponent);
	}
	ASSERT(pComponent);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp); // ! only worked on 32bit system

	CMFCPropertyGridProperty * pShape = new CComboProp(_T("Shape"), g_ShapeTypeDesc[cmp->m_PxShape ? cmp->m_PxShape->getGeometryType() : physx::PxGeometryType::eGEOMETRY_COUNT], NULL, PropertyComponentShape);
	for (unsigned int i = 0; i < _countof(g_ShapeTypeDesc); i++)
	{
		pShape->AddOption(g_ShapeTypeDesc[i], TRUE);
	}
	pComponent->AddSubItem(pShape);

	switch (cmp->m_Type)
	{
	case Component::ComponentTypeMesh:
		CreatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeCloth:
		CreatePropertiesCloth(pComponent, dynamic_cast<ClothComponent *>(cmp));
		break;
	case Component::ComponentTypeStaticEmitter:
		CreatePropertiesStaticEmitter(pComponent, dynamic_cast<EmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		CreatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		CreatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	}
}

void CPropertiesWnd::CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("ResPath"), TRUE, (_variant_t)ms2ts(mesh_cmp->m_MeshRes.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshResPath);
	pComponent->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("Instance"), (_variant_t)mesh_cmp->m_bInstance, NULL, PropertyMeshInstance);
	pComponent->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("UseAnimation"), (_variant_t)mesh_cmp->m_bUseAnimation, NULL, PropertyMeshUseAnimation);
	pComponent->AddSubItem(pProp);
	pProp = new CMFCPropertyGridProperty(_T("MaterialList"), PropertyMaterialList, FALSE);
	pComponent->AddSubItem(pProp);
	for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
	{
		CreatePropertiesMaterial(pProp, i, mesh_cmp->m_MaterialList[i].get());
	}
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, int NodeId, Material * mat)
{
	TCHAR buff[128];
	_stprintf_s(buff, _countof(buff), _T("Material%d"), NodeId);
	CMFCPropertyGridProperty * pMaterial = new CSimpleProp(buff, NodeId, FALSE);
	pParentCtrl->AddSubItem(pMaterial);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mat);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("Shader"), TRUE, (_variant_t)ms2ts(mat->m_Shader).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialShader);
	pMaterial->AddSubItem(pProp);
	CComboProp * pPassMask = new CComboProp(_T("PassMask"), (_variant_t)GetPassMaskDesc(mat->m_PassMask), NULL, PropertyMaterialPassMask);
	for (unsigned int i = 0; i < _countof(g_PassMaskDesc); i++)
	{
		pPassMask->AddOption(g_PassMaskDesc[i].desc, TRUE);
	}
	pMaterial->AddSubItem(pPassMask);
	CComboProp * pCullMode = new CComboProp(_T("CullMode"), (_variant_t)g_CullModeDesc[mat->m_CullMode - 1], NULL, PropertyMaterialCullMode);
	for (unsigned int i = 0; i < _countof(g_CullModeDesc); i++)
	{
		pCullMode->AddOption(g_CullModeDesc[i], TRUE);
	}
	pMaterial->AddSubItem(pCullMode);
	CCheckBoxProp * pZEnable = new CCheckBoxProp(_T("ZEnable"), mat->m_ZEnable, NULL, PropertyMaterialZEnable);
	pMaterial->AddSubItem(pZEnable);
	CCheckBoxProp * pZWriteEnable = new CCheckBoxProp(_T("ZWriteEnable"), mat->m_ZWriteEnable, NULL, PropertyMaterialZWriteEnable);
	pMaterial->AddSubItem(pZWriteEnable);
	CComboProp * pBlendMode = new CComboProp(_T("BlendMode"), (_variant_t)g_BlendModeDesc[mat->m_BlendMode], NULL, PropertyMaterialBlendMode);
	for (unsigned int i = 0; i < _countof(g_BlendModeDesc); i++)
	{
		pBlendMode->AddOption(g_BlendModeDesc[i], TRUE);
	}
	pMaterial->AddSubItem(pBlendMode);
	COLORREF color = RGB(mat->m_MeshColor.x * 255, mat->m_MeshColor.y * 255, mat->m_MeshColor.z * 255);
	CColorProp * pColor = new CColorProp(_T("MeshColor"), color, NULL, NULL, PropertyMaterialMeshColor);
	pColor->EnableOtherButton(_T("Other..."));
	pMaterial->AddSubItem(pColor);
	CMFCPropertyGridProperty * pColorAlpha = new CSimpleProp(_T("MeshColorAlpha"), (_variant_t)mat->m_MeshColor.w, NULL, PropertyMaterialMeshColorAlpha);
	pMaterial->AddSubItem(pColorAlpha);
	pProp = new CFileProp(_T("MeshTexture"), TRUE, (_variant_t)ms2ts(mat->m_MeshTexture.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialMeshTexture);
	pMaterial->AddSubItem(pProp);
	pProp = new CFileProp(_T("NormalTexture"), TRUE, (_variant_t)ms2ts(mat->m_NormalTexture.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialNormalTexture);
	pMaterial->AddSubItem(pProp);
	pProp = new CFileProp(_T("SpecularTexture"), TRUE, (_variant_t)ms2ts(mat->m_SpecularTexture.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialSpecularTexture);
	pMaterial->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	physx::PxClothFlags flags = cloth_cmp->m_Cloth->getClothFlags();
	CMFCPropertyGridProperty * pProp = new CCheckBoxProp(_T("SceneCollision"), (_variant_t)flags.isSet(physx::PxClothFlag::eSCENE_COLLISION), NULL, PropertyClothSceneCollision);
	pComponent->AddSubItem(pProp);
	pProp = new CMFCPropertyGridProperty(_T("MaterialList"), PropertyMaterialList, FALSE);
	pComponent->AddSubItem(pProp);
	for (unsigned int i = 0; i < cloth_cmp->m_MaterialList.size(); i++)
	{
		CreatePropertiesMaterial(pProp, i, cloth_cmp->m_MaterialList[i].get());
	}
}

void CPropertiesWnd::CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pParticleList = new CSimpleProp(_T("ParticleList"), PropertyEmitterParticleList, FALSE);
	pComponent->AddSubItem(pParticleList);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("ParticleCount"), (_variant_t)emit_cmp->m_Emitter->m_ParticleList.size(), NULL, PropertyEmitterParticleCount);
	pParticleList->AddSubItem(pProp);
	for (unsigned int i = 0; i < emit_cmp->m_Emitter->m_ParticleList.size(); i++)
	{
		CreatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
	}
	CreatePropertiesMaterial(pComponent, 0, emit_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp)
{
	TCHAR buff[128];
	_stprintf_s(buff, _countof(buff), _T("Particle%d"), NodeId);
	CMFCPropertyGridProperty * pParticle = new CSimpleProp(buff, NodeId, FALSE);
	pParentProp->AddSubItem(pParticle);
	CMFCPropertyGridProperty * pPosition = new CMFCPropertyGridProperty(_T("Position"), PropertyEmitterParticlePosition, TRUE);
	pParticle->AddSubItem(pPosition);
	my::Emitter::Particle & particle = emit_cmp->m_Emitter->m_ParticleList[NodeId];
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)particle.m_Position.x, NULL, PropertyEmitterParticlePositionX);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)particle.m_Position.y, NULL, PropertyEmitterParticlePositionY);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)particle.m_Position.z, NULL, PropertyEmitterParticlePositionZ);
	pPosition->AddSubItem(pProp);

	CMFCPropertyGridProperty * pVelocity = new CMFCPropertyGridProperty(_T("Velocity"), PropertyEmitterParticleVelocity, TRUE);
	pParticle->AddSubItem(pVelocity);
	pProp = new CSimpleProp(_T("x"), (_variant_t)particle.m_Velocity.x, NULL, PropertyEmitterParticleVelocityX);
	pVelocity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)particle.m_Velocity.y, NULL, PropertyEmitterParticleVelocityY);
	pVelocity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)particle.m_Velocity.z, NULL, PropertyEmitterParticleVelocityZ);
	pVelocity->AddSubItem(pProp);

	COLORREF color = RGB(particle.m_Color.x * 255, particle.m_Color.y * 255, particle.m_Color.z * 255);
	CColorProp * pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyEmitterParticleColor);
	pColor->EnableOtherButton(_T("Other..."));
	pParticle->AddSubItem(pColor);

	CMFCPropertyGridProperty * pAlpha = new CSimpleProp(_T("Alpha"), particle.m_Color.w, NULL, PropertyEmitterParticleColorAlpha);
	pParticle->AddSubItem(pAlpha);

	CMFCPropertyGridProperty * pSize = new CMFCPropertyGridProperty(_T("Size"), PropertyEmitterParticleSize, TRUE);
	pParticle->AddSubItem(pSize);
	pProp = new CSimpleProp(_T("x"), (_variant_t)particle.m_Size.x, NULL, PropertyEmitterParticleSizeX);
	pSize->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)particle.m_Size.y, NULL, PropertyEmitterParticleSizeY);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("Angle"), (_variant_t)particle.m_Angle, NULL, PropertyEmitterParticleAngle);
	pParticle->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)sphe_emit_cmp->m_ParticleLifeTime, NULL, PropertySphericalEmitterParticleLifeTime);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("SpawnInterval"), (_variant_t)sphe_emit_cmp->m_SpawnInterval, NULL, PropertySphericalEmitterSpawnInterval);
	pComponent->AddSubItem(pProp);
	CMFCPropertyGridProperty * pHalfSpawnArea = new CSimpleProp(_T("HalfSpawnArea"), PropertySphericalEmitterHalfSpawnArea, TRUE);
	pComponent->AddSubItem(pHalfSpawnArea);
	pProp = new CSimpleProp(_T("x"), (_variant_t)sphe_emit_cmp->m_HalfSpawnArea.x, NULL, PropertySphericalEmitterHalfSpawnAreaX);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)sphe_emit_cmp->m_HalfSpawnArea.y, NULL, PropertySphericalEmitterHalfSpawnAreaY);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)sphe_emit_cmp->m_HalfSpawnArea.z, NULL, PropertySphericalEmitterHalfSpawnAreaZ);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("SpawnSpeed"), (_variant_t)sphe_emit_cmp->m_SpawnSpeed, NULL, PropertySphericalEmitterSpawnSpeed);
	pComponent->AddSubItem(pProp);
	CreatePropertiesSpline(pComponent, _T("SpawnInclination"), PropertySphericalEmitterSpawnInclination, &sphe_emit_cmp->m_SpawnInclination);
	CreatePropertiesSpline(pComponent, _T("SpawnAzimuth"), PropertySphericalEmitterSpawnAzimuth, &sphe_emit_cmp->m_SpawnAzimuth);
	CreatePropertiesSpline(pComponent, _T("SpawnColorR"), PropertySphericalEmitterSpawnColorR, &sphe_emit_cmp->m_SpawnColorR);
	CreatePropertiesSpline(pComponent, _T("SpawnColorG"), PropertySphericalEmitterSpawnColorG, &sphe_emit_cmp->m_SpawnColorG);
	CreatePropertiesSpline(pComponent, _T("SpawnColorB"), PropertySphericalEmitterSpawnColorB, &sphe_emit_cmp->m_SpawnColorB);
	CreatePropertiesSpline(pComponent, _T("SpawnColorA"), PropertySphericalEmitterSpawnColorA, &sphe_emit_cmp->m_SpawnColorA);
	CreatePropertiesSpline(pComponent, _T("SpawnSizeX"), PropertySphericalEmitterSpawnSizeX, &sphe_emit_cmp->m_SpawnSizeX);
	CreatePropertiesSpline(pComponent, _T("SpawnSizeY"), PropertySphericalEmitterSpawnSizeY, &sphe_emit_cmp->m_SpawnSizeY);
	CreatePropertiesSpline(pComponent, _T("SpawnAngle"), PropertySphericalEmitterSpawnAngle, &sphe_emit_cmp->m_SpawnAngle);
	pProp = new CSimpleProp(_T("SpawnLoopTime"), (_variant_t)sphe_emit_cmp->m_SpawnLoopTime, NULL, PropertySphericalEmitterSpawnLoopTime);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, 0, sphe_emit_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline)
{
	_ASSERT(PropertyId >= PropertySphericalEmitterSpawnInclination && PropertyId <= PropertySphericalEmitterSpawnAngle);
	CMFCPropertyGridProperty * pSpline = new CSimpleProp(lpszName, PropertyId, TRUE);
	pParentProp->AddSubItem(pSpline);
	pSpline->SetValue((_variant_t)(DWORD_PTR)spline);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)spline->size(), NULL, PropertySplineNodeCount);
	pSpline->AddSubItem(pProp);
	for (unsigned int i = 0; i < spline->size(); i++)
	{
		CreatePropertiesSplineNode(pSpline, i, &(*spline)[i]);
	}
}

void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::SplineNode * node)
{
	CMFCPropertyGridProperty * pNode = new CSimpleProp(_T("Node"), NodeId, TRUE);
	pSpline->AddSubItem(pNode);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)node->x, NULL, PropertySplineNodeX);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)node->y, NULL, PropertySplineNodeY);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k0"), (_variant_t)node->k0, NULL, PropertySplineNodeK0);
	pNode->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("k"), (_variant_t)node->k, NULL, PropertySplineNodeK);
	pNode->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("RowChunks"), (_variant_t)terrain->ROW_CHUNKS, NULL, PropertyTerrainRowChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ColChunks"), (_variant_t)terrain->COL_CHUNKS, NULL, PropertyTerrainColChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ChunkSize"), (_variant_t)terrain->CHUNK_SIZE, NULL, PropertyTerrainChunkSize);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("HeightScale"), (_variant_t)terrain->m_HeightScale, NULL, PropertyTerrainHeightScale);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("WrappedU"), (_variant_t)terrain->m_WrappedU, NULL, PropertyTerrainWrappedU);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("WrappedV"), (_variant_t)terrain->m_WrappedV, NULL, PropertyTerrainWrappedV);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("HeightMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainHeightMap);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, 0, terrain->m_Material.get());
}

unsigned int CPropertiesWnd::GetComponentPropCount(Component::ComponentType type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return 5;
	case Component::ComponentTypeCharacter:
		return GetComponentPropCount(Component::ComponentTypeActor);
	case Component::ComponentTypeMesh:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 4;
	case Component::ComponentTypeCloth:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 2;
	case Component::ComponentTypeStaticEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 2;
	case Component::ComponentTypeSphericalEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 15;
	case Component::ComponentTypeTerrain:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 8;
	}
	return 1;
}

LPCTSTR CPropertiesWnd::GetComponentTypeName(Component::ComponentType type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return _T("Actor");
	case Component::ComponentTypeCharacter:
		return _T("Character");
	case Component::ComponentTypeMesh:
		return _T("Mesh");
	case Component::ComponentTypeCloth:
		return _T("Cloth");
	case Component::ComponentTypeStaticEmitter:
		return _T("Emitter");
	case Component::ComponentTypeSphericalEmitter:
		return _T("SphericalEmitter");
	case Component::ComponentTypeTerrain:
		return _T("Terrain");
	}
	return _T("Unknown");
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
	//if (pFrame->m_selactors.empty())
	//{
	//	return 0;
	//}
	//Component * cmp = *pFrame->m_selactors.begin();

	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	DWORD PropertyId = pProp->GetData();
	switch (PropertyId)
	{
	case PropertyActorAABB:
	case PropertyActorMinX:
	case PropertyActorMinY:
	case PropertyActorMinZ:
	case PropertyActorMaxX:
	case PropertyActorMaxY:
	case PropertyActorMaxZ:
		{
			CMFCPropertyGridProperty * pAABB = NULL;
			switch (PropertyId)
			{
			case PropertyActorAABB:
				pAABB = pProp;
				break;
			case PropertyActorMinX:
			case PropertyActorMinY:
			case PropertyActorMinZ:
			case PropertyActorMaxX:
			case PropertyActorMaxY:
			case PropertyActorMaxZ:
				pAABB = pProp->GetParent();
				break;
			}
			Actor * actor = (Actor *)pAABB->GetParent()->GetValue().ulVal;
			actor->m_aabb.m_min.x = pAABB->GetSubItem(0)->GetValue().fltVal;
			actor->m_aabb.m_min.y = pAABB->GetSubItem(1)->GetValue().fltVal;
			actor->m_aabb.m_min.z = pAABB->GetSubItem(2)->GetValue().fltVal;
			actor->m_aabb.m_max.x = pAABB->GetSubItem(3)->GetValue().fltVal;
			actor->m_aabb.m_max.y = pAABB->GetSubItem(4)->GetValue().fltVal;
			actor->m_aabb.m_max.z = pAABB->GetSubItem(5)->GetValue().fltVal;
			actor->OnWorldChanged();
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyActorPos:
	case PropertyActorPosX:
	case PropertyActorPosY:
	case PropertyActorPosZ:
	case PropertyActorRot:
	case PropertyActorRotX:
	case PropertyActorRotY:
	case PropertyActorRotZ:
	case PropertyActorScale:
	case PropertyActorScaleX:
	case PropertyActorScaleY:
	case PropertyActorScaleZ:
		{
			CMFCPropertyGridProperty * pComponent = NULL;
			switch (PropertyId)
			{
			case PropertyActorPos:
			case PropertyActorRot:
			case PropertyActorScale:
				pComponent = pProp->GetParent();
				break;
			default:
				pComponent = pProp->GetParent()->GetParent();
				break;
			}
			Actor * actor = (Actor *)pComponent->GetValue().ulVal;
			actor->m_Position.x = pComponent->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
			actor->m_Position.y = pComponent->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
			actor->m_Position.z = pComponent->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
			actor->m_Rotation = my::Quaternion::RotationEulerAngles(my::Vector3(
				D3DXToRadian(pComponent->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal),
				D3DXToRadian(pComponent->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal),
				D3DXToRadian(pComponent->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal)));
			actor->m_Scale.x = pComponent->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal;
			actor->m_Scale.y = pComponent->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal;
			actor->m_Scale.z = pComponent->GetSubItem(3)->GetSubItem(2)->GetValue().fltVal;
			actor->UpdateAABB();
			actor->UpdateWorld();
			actor->OnWorldChanged();
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyActorRigidActor:
		{
			Actor * actor = (Actor *)pProp->GetParent()->GetValue().ulVal;
			int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
			ASSERT(i >= 0 && i < _countof(g_ActorTypeDesc));
			actor->ClearRigidActor();
			actor->CreateRigidActor((physx::PxActorType::Enum)i);
			actor->OnEnterPxScene(pFrame);
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyComponentShape:
		{
			Component * cmp = (Component *)pProp->GetParent()->GetValue().ulVal;
			int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
			ASSERT(i >= 0 && i < _countof(g_ShapeTypeDesc));
			CShapeDlg dlg(pFrame, cmp, i);
			switch (i)
			{
			case physx::PxGeometryType::eSPHERE:
			case physx::PxGeometryType::ePLANE:
			case physx::PxGeometryType::eCAPSULE:
			case physx::PxGeometryType::eBOX:
			case physx::PxGeometryType::eCONVEXMESH:
			case physx::PxGeometryType::eTRIANGLEMESH:
			case physx::PxGeometryType::eHEIGHTFIELD:
				if (dlg.DoModal() == IDOK)
				{
					EventArgs arg;
					pFrame->m_EventAttributeChanged(&arg);
				}
				break;
			default:
				cmp->ClearShape();
				break;
			}
		}
		break;
	case PropertyMeshResPath:
		{
			//MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			//mesh_cmp->m_MeshRes.ReleaseResource();
			//mesh_cmp->m_MeshRes.m_Path = ts2ms(pProp->GetValue().bstrVal);
			//mesh_cmp->m_MeshRes.RequestResource();
			//EventArgs arg;
			//pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMeshInstance:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			mesh_cmp->m_bInstance = pProp->GetValue().boolVal != 0;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMeshUseAnimation:
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
			mesh_cmp->m_bUseAnimation = pProp->GetValue().boolVal != 0;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialShader:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_Shader = ts2ms(pProp->GetValue().bstrVal);
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialPassMask:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
			ASSERT(i >= 0 && i < _countof(g_PassMaskDesc));
			material->m_PassMask = g_PassMaskDesc[i].mask;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialCullMode:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
			ASSERT(i >= 0 && i < _countof(g_CullModeDesc));
			material->m_CullMode = i + 1;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialZEnable:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_ZEnable = pProp->GetValue().boolVal != 0;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialZWriteEnable:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_ZWriteEnable = pProp->GetValue().boolVal != 0;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialBlendMode:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
			ASSERT(i >= 0 && i < _countof(g_BlendModeDesc));
			material->m_BlendMode = i;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialMeshColor:
		{
			CColorProp * pColor = DYNAMIC_DOWNCAST(CColorProp, pProp);
			ASSERT(pColor);
			COLORREF color = pColor->GetColor();
			Material * material = (Material *)pColor->GetParent()->GetValue().ulVal;
			material->m_MeshColor.x = GetRValue(color) / 255.0f;
			material->m_MeshColor.y = GetGValue(color) / 255.0f;
			material->m_MeshColor.z = GetBValue(color) / 255.0f;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialMeshColorAlpha:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_MeshColor.w = pProp->GetValue().fltVal;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialMeshTexture:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_MeshTexture.ReleaseResource();
			material->m_MeshTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_MeshTexture.RequestResource();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialNormalTexture:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_NormalTexture.ReleaseResource();
			material->m_NormalTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_NormalTexture.RequestResource();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyMaterialSpecularTexture:
		{
			Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
			material->m_SpecularTexture.ReleaseResource();
			material->m_SpecularTexture.m_Path = ts2ms(pProp->GetValue().bstrVal);
			material->m_SpecularTexture.RequestResource();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyClothSceneCollision:
		{
			ClothComponent * cloth_cmp = (ClothComponent *)pProp->GetParent()->GetValue().ulVal;
			cloth_cmp->m_Cloth->setClothFlag(physx::PxClothFlag::eSCENE_COLLISION, pProp->GetValue().boolVal != 0);
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyEmitterParticleCount:
		{
			EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetParent()->GetValue().ulVal;
			emit_cmp->m_Emitter->m_ParticleList.resize(pProp->GetValue().uintVal,
				my::Emitter::Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector4(1,1,1,1), my::Vector2(10,10), 0, 0));
			UpdatePropertiesStaticEmitter(pProp->GetParent()->GetParent(), emit_cmp);
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyEmitterParticlePosition:
	case PropertyEmitterParticlePositionX:
	case PropertyEmitterParticlePositionY:
	case PropertyEmitterParticlePositionZ:
	case PropertyEmitterParticleVelocity:
	case PropertyEmitterParticleVelocityX:
	case PropertyEmitterParticleVelocityY:
	case PropertyEmitterParticleVelocityZ:
	case PropertyEmitterParticleColor:
	case PropertyEmitterParticleColorAlpha:
	case PropertyEmitterParticleSize:
	case PropertyEmitterParticleSizeX:
	case PropertyEmitterParticleSizeY:
	case PropertyEmitterParticleAngle:
		{
			CMFCPropertyGridProperty * pParticle = NULL;
			switch (PropertyId)
			{
			case PropertyEmitterParticlePositionX:
			case PropertyEmitterParticlePositionY:
			case PropertyEmitterParticlePositionZ:
			case PropertyEmitterParticleVelocityX:
			case PropertyEmitterParticleVelocityY:
			case PropertyEmitterParticleVelocityZ:
			case PropertyEmitterParticleSizeX:
			case PropertyEmitterParticleSizeY:
				pParticle = pProp->GetParent()->GetParent();
				break;
			case PropertyEmitterParticlePosition:
			case PropertyEmitterParticleVelocity:
			case PropertyEmitterParticleColor:
			case PropertyEmitterParticleColorAlpha:
			case PropertyEmitterParticleSize:
			case PropertyEmitterParticleAngle:
				pParticle = pProp->GetParent();
				break;
			}
			int NodeId = pParticle->GetData();
			EmitterComponent * emit_cmp = (EmitterComponent *)pParticle->GetParent()->GetParent()->GetValue().ulVal;
			my::Emitter::Particle & particle = emit_cmp->m_Emitter->m_ParticleList[NodeId];
			particle.m_Position.x = pParticle->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Position.y = pParticle->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Position.z = pParticle->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal;
			particle.m_Velocity.x = pParticle->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Velocity.y = pParticle->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Velocity.z = pParticle->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
			COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pParticle->GetSubItem(2)))->GetColor();
			particle.m_Color.x = GetRValue(color) / 255.0f;
			particle.m_Color.y = GetGValue(color) / 255.0f;
			particle.m_Color.z = GetBValue(color) / 255.0f;
			particle.m_Color.w = pParticle->GetSubItem(3)->GetValue().fltVal;
			particle.m_Size.x = pParticle->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Size.y = pParticle->GetSubItem(4)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Angle = pParticle->GetSubItem(5)->GetValue().fltVal;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertySphericalEmitterParticleLifeTime:
	case PropertySphericalEmitterSpawnInterval:
	case PropertySphericalEmitterHalfSpawnArea:
	case PropertySphericalEmitterHalfSpawnAreaX:
	case PropertySphericalEmitterHalfSpawnAreaY:
	case PropertySphericalEmitterHalfSpawnAreaZ:
	case PropertySphericalEmitterSpawnSpeed:
	case PropertySphericalEmitterSpawnLoopTime:
		{
			CMFCPropertyGridProperty * pComponent = NULL;
			switch (PropertyId)
			{
			case PropertySphericalEmitterHalfSpawnAreaX:
			case PropertySphericalEmitterHalfSpawnAreaY:
			case PropertySphericalEmitterHalfSpawnAreaZ:
				pComponent = pProp->GetParent()->GetParent();
				break;
			default:
				pComponent = pProp->GetParent();
				break;
			}
			SphericalEmitterComponent * sphe_emit_cmp = (SphericalEmitterComponent *)pComponent->GetValue().ulVal;
			unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
			sphe_emit_cmp->m_ParticleLifeTime = pComponent->GetSubItem(PropId + 0)->GetValue().fltVal;
			sphe_emit_cmp->m_SpawnInterval = pComponent->GetSubItem(PropId + 1)->GetValue().fltVal;
			sphe_emit_cmp->m_HalfSpawnArea.x = pComponent->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().fltVal;
			sphe_emit_cmp->m_HalfSpawnArea.y = pComponent->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().fltVal;
			sphe_emit_cmp->m_HalfSpawnArea.z = pComponent->GetSubItem(PropId + 2)->GetSubItem(2)->GetValue().fltVal;
			sphe_emit_cmp->m_SpawnSpeed = pComponent->GetSubItem(PropId + 3)->GetValue().fltVal;
			sphe_emit_cmp->m_SpawnLoopTime = pComponent->GetSubItem(PropId + 13)->GetValue().fltVal;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertySplineNodeCount:
	case PropertySplineNodeX:
	case PropertySplineNodeY:
	case PropertySplineNodeK0:
	case PropertySplineNodeK:
		{
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
			SphericalEmitterComponent * sphe_emit_cmp = (SphericalEmitterComponent *)pSpline->GetParent()->GetValue().ulVal;
			my::Spline * spline = (my::Spline *)pSpline->GetValue().ulVal;
			switch (PropertyId)
			{
			case PropertySplineNodeCount:
				spline->resize(pProp->GetValue().uintVal, my::SplineNode(0, 0, 0, 0));
				UpdatePropertiesSpline(pSpline, spline);
				break;
			case PropertySplineNodeX:
			case PropertySplineNodeY:
			case PropertySplineNodeK0:
			case PropertySplineNodeK:
				{
					CMFCPropertyGridProperty * pNode = pProp->GetParent();
					int id = pNode->GetData();
					_ASSERT(id < (int)spline->size());
					my::SplineNode & node = (*spline)[id];
					node.x = pNode->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->GetValue().fltVal;
					node.y = pNode->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->GetValue().fltVal;
					node.k0 = pNode->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->GetValue().fltVal;
					node.k = pNode->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->GetValue().fltVal;
				}
				break;
			}
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyTerrainHeightScale:
		{
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->m_HeightScale = pProp->GetValue().fltVal;
			terrain->UpdateHeightMapNormal();
			terrain->UpdateChunks();
			Actor * actor = terrain->m_Actor;
			actor->UpdateAABB();
			actor->OnWorldChanged();
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyTerrainWrappedU:
	case PropertyTerrainWrappedV:
		{
			unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->m_WrappedU = pProp->GetParent()->GetSubItem(PropId + 4)->GetValue().fltVal;
			terrain->m_WrappedV = pProp->GetParent()->GetSubItem(PropId + 5)->GetValue().fltVal;
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyTerrainHeightMap:
		{
			std::string path = ts2ms(pProp->GetValue().bstrVal);
			my::Texture2DPtr res = boost::dynamic_pointer_cast<my::Texture2D>(theApp.LoadTexture(path.c_str()));
			if (res)
			{
				Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
				terrain->UpdateHeightMap(res);
				Actor * actor = terrain->m_Actor;
				actor->UpdateAABB();
				actor->OnWorldChanged();
				pFrame->UpdateSelBox();
				pFrame->UpdatePivotTransform();
				EventArgs arg;
				pFrame->m_EventAttributeChanged(&arg);
			}
		}
		break;
	}
	return 0;
}

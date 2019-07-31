
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "CtrlProps.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "ShapeDlg.h"
#include "Material.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPropertiesWnd::PassMaskDesc CPropertiesWnd::g_PassMaskDesc[] =
{
	{ _T("None"), RenderPipeline::PassMaskNone },
	{ _T("Light"), RenderPipeline::PassMaskLight },
	{ _T("Opaque"), RenderPipeline::PassMaskOpaque },
	{ _T("NormalOpaque"), RenderPipeline::PassMaskNormalOpaque },
	{ _T("ShadowNormalOpaque"), RenderPipeline::PassMaskShadowNormalOpaque },
	{ _T("Transparent"), RenderPipeline::PassMaskTransparent },
};

LPCTSTR CPropertiesWnd::GetPassMaskDesc(DWORD mask)
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

LPCTSTR CPropertiesWnd::g_CullModeDesc[] =
{
	_T("D3DCULL_NONE"),
	_T("D3DCULL_CW"),
	_T("D3DCULL_CCW")
};

LPCTSTR CPropertiesWnd::g_BlendModeDesc[] =
{
	_T("BlendModeNone"),
	_T("BlendModeAlpha"),
	_T("BlendModeAdditive")
};

static const CPropertiesWnd::PassMaskDesc g_LodMaskDesc[] =
{
	{ _T("LOD0"), Component::LOD0 },
	{ _T("LOD1"), Component::LOD1 },
	{ _T("LOD2"), Component::LOD2 },
	{ _T("LOD0_1"), Component::LOD0_1 },
	{ _T("LOD1_2"), Component::LOD1_2 },
	{ _T("LOD0_1_2"), Component::LOD0_1_2 },
};

static LPCTSTR GetLodMaskDesc(DWORD mask)
{
	for (unsigned int i = 0; i < _countof(g_LodMaskDesc); i++)
	{
		if (g_LodMaskDesc[i].mask == mask)
		{
			return g_LodMaskDesc[i].desc;
		}
	}
	return g_LodMaskDesc[0].desc;
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
	ASSERT_VALID(pParentCtrl);
	while (pParentCtrl->GetSubItemsCount() > i)
	{
		CMFCPropertyGridProperty * pProp = pParentCtrl->GetSubItem(i);
		static_cast<CMFCPropertyGridPropertyReader *>(pParentCtrl)->RemoveSubItem(pProp, TRUE);
	}
}

void CPropertiesWnd::UpdatePropertiesActor(Actor * actor)
{
	CMFCPropertyGridProperty * pActor = NULL;
	if (m_wndPropList.GetPropertyCount() >= 1)
	{
		pActor = m_wndPropList.GetProperty(0);
	}
	if (!pActor || pActor->GetData() != PropertyActor)
	{
		m_wndPropList.RemoveAll();
		CreatePropertiesActor(actor);
		return;
	}
	pActor->SetName(GetComponentTypeName(Component::ComponentTypeActor), FALSE);
	pActor->SetValue((_variant_t)(DWORD_PTR)actor);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pActor->GetSubItem(0)->GetSubItem(0)->SetValue((_variant_t)actor->m_aabb.m_min.x);
	pActor->GetSubItem(0)->GetSubItem(1)->SetValue((_variant_t)actor->m_aabb.m_min.y);
	pActor->GetSubItem(0)->GetSubItem(2)->SetValue((_variant_t)actor->m_aabb.m_min.z);
	pActor->GetSubItem(0)->GetSubItem(3)->SetValue((_variant_t)actor->m_aabb.m_max.x);
	pActor->GetSubItem(0)->GetSubItem(4)->SetValue((_variant_t)actor->m_aabb.m_max.y);
	pActor->GetSubItem(0)->GetSubItem(5)->SetValue((_variant_t)actor->m_aabb.m_max.z);
	pActor->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)actor->m_Position.x);
	pActor->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)actor->m_Position.y);
	pActor->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)actor->m_Position.z);
	my::Vector3 angle = actor->m_Rotation.ToEulerAngles();
	pActor->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pActor->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pActor->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pActor->GetSubItem(3)->GetSubItem(0)->SetValue((_variant_t)actor->m_Scale.x);
	pActor->GetSubItem(3)->GetSubItem(1)->SetValue((_variant_t)actor->m_Scale.y);
	pActor->GetSubItem(3)->GetSubItem(2)->SetValue((_variant_t)actor->m_Scale.z);
	pActor->GetSubItem(4)->SetValue((_variant_t)actor->m_LodRatio);
	pActor->GetSubItem(5)->SetValue((_variant_t)g_ActorTypeDesc[actor->m_PxActor ? actor->m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT]);
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeActor);
	for (unsigned int i = 0; i < actor->m_Cmps.size(); i++)
	{
		if ((unsigned int)pActor->GetSubItemsCount() <= PropId + i)
		{
			CreateProperties(pActor, actor->m_Cmps[i].get());
			continue;
		}
		if (pActor->GetSubItem(PropId + i)->GetData() != GetComponentProp(actor->m_Cmps[i]->m_Type))
		{
			RemovePropertiesFrom(pActor, PropId + i);
			CreateProperties(pActor, actor->m_Cmps[i].get());
			continue;
		}
		UpdateProperties(pActor->GetSubItem(PropId + i), i, actor->m_Cmps[i].get());
	}
	RemovePropertiesFrom(pActor, GetComponentPropCount(Component::ComponentTypeActor) + actor->m_Cmps.size());
	//m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::UpdateProperties(CMFCPropertyGridProperty * pComponent, int i, Component * cmp)
{
	//pComponent->SetName(GetComponentTypeName(cmp->m_Type), FALSE);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp);
	pComponent->GetSubItem(0)->SetValue((_variant_t)GetLodMaskDesc(cmp->m_LodMask));

	UpdatePropertiesShape(pComponent->GetSubItem(1), cmp);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
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
		UpdatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp), pFrame->m_selchunkid);
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesShape(CMFCPropertyGridProperty * pShape, Component * cmp)
{
	pShape->GetSubItem(0)->SetValue((_variant_t)g_ShapeTypeDesc[cmp->m_PxShape ? cmp->m_PxShape->getGeometryType() : physx::PxGeometryType::eGEOMETRY_COUNT]);
	physx::PxTransform localPose;
	physx::PxFilterData filterData;
	physx::PxShapeFlags shapeFlags;
	if (cmp->m_PxShape)
	{
		localPose = cmp->m_PxShape->getLocalPose();
		filterData = cmp->m_PxShape->getQueryFilterData();
		shapeFlags = cmp->m_PxShape->getFlags();
	}
	pShape->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)localPose.p.x);
	pShape->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)localPose.p.y);
	pShape->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)localPose.p.z);
	my::Vector3 angle = ((my::Quaternion &)localPose.q).ToEulerAngles();
	pShape->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pShape->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pShape->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pShape->GetSubItem(3)->SetValue((_variant_t)filterData.word0);
	pShape->GetSubItem(4)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE));
	pShape->GetSubItem(5)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE));
	pShape->GetSubItem(6)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE));
	pShape->GetSubItem(7)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eVISUALIZATION));
	UpdatePropertiesShapeShow(pShape, cmp->m_PxShape != NULL);
}

void CPropertiesWnd::UpdatePropertiesShapeShow(CMFCPropertyGridProperty * pShape, BOOL bShow)
{
	for (int i = 1; i < pShape->GetSubItemsCount(); i++)
	{
		pShape->GetSubItem(i)->Show(bShow, FALSE);
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
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(mesh_cmp->m_MeshPath).c_str());
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)(VARIANT_BOOL)mesh_cmp->m_bInstance);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)(VARIANT_BOOL)mesh_cmp->m_bUseAnimation);
	CMFCPropertyGridProperty * pMaterialList = pComponent->GetSubItem(PropId + 3);
	for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
	{
		if ((unsigned int)pMaterialList->GetSubItemsCount() <= i)
		{
			TCHAR buff[128];
			_stprintf_s(buff, _countof(buff), _T("Material%d"), i);
			CreatePropertiesMaterial(pMaterialList, buff, mesh_cmp->m_MaterialList[i].get());
			continue;
		}
		UpdatePropertiesMaterial(pMaterialList->GetSubItem(i), mesh_cmp->m_MaterialList[i].get());
	}
	RemovePropertiesFrom(pMaterialList, mesh_cmp->m_MaterialList.size());
}

void CPropertiesWnd::UpdatePropertiesMaterial(CMFCPropertyGridProperty * pMaterial, Material * mtl)
{
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mtl);
	pMaterial->GetSubItem(0)->SetValue((_variant_t)mtl->m_Shader.c_str());
	pMaterial->GetSubItem(1)->SetValue((_variant_t)GetPassMaskDesc(mtl->m_PassMask));
	pMaterial->GetSubItem(2)->SetValue((_variant_t)g_CullModeDesc[mtl->m_CullMode - 1]);
	pMaterial->GetSubItem(3)->SetValue((_variant_t)(VARIANT_BOOL)mtl->m_ZEnable);
	pMaterial->GetSubItem(4)->SetValue((_variant_t)(VARIANT_BOOL)mtl->m_ZWriteEnable);
	pMaterial->GetSubItem(5)->SetValue((_variant_t)g_BlendModeDesc[mtl->m_BlendMode]);

	CMFCPropertyGridProperty * pParameterList = pMaterial->GetSubItem(6);
	for (unsigned int i = 0; i < mtl->m_ParameterList.size(); i++)
	{
		if ((unsigned int)pParameterList->GetSubItemsCount() <= i)
		{
			CreatePropertiesMaterialParameter(pParameterList, i, mtl->m_ParameterList[i].get());
			continue;
		}
		if (pParameterList->GetSubItem(i)->GetData() != GetMaterialParameterTypeProp(mtl->m_ParameterList[i]->m_Type))
		{
			RemovePropertiesFrom(pParameterList, i);
			CreatePropertiesMaterialParameter(pParameterList, i, mtl->m_ParameterList[i].get());
			continue;
		}
		UpdatePropertiesMaterialParameter(pParameterList, i, mtl->m_ParameterList[i].get());
	}
	RemovePropertiesFrom(pParameterList, mtl->m_ParameterList.size());
}

void CPropertiesWnd::UpdatePropertiesMaterialParameter(CMFCPropertyGridProperty * pParentCtrl, int NodeId, MaterialParameter * mtl_param)
{
	switch (mtl_param->m_Type)
	{
	case MaterialParameter::ParameterTypeFloat:
		pParentCtrl->GetSubItem(NodeId)->SetValue((_variant_t)
			dynamic_cast<MaterialParameterFloat *>(mtl_param)->m_Value);
		break;
	case MaterialParameter::ParameterTypeFloat2:
	{
		const my::Vector2 & Value = dynamic_cast<MaterialParameterFloat2 *>(mtl_param)->m_Value;
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(0)->SetValue((_variant_t)Value.x);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(1)->SetValue((_variant_t)Value.y);
		break;
	}
	case MaterialParameter::ParameterTypeFloat3:
	{
		const my::Vector3 & Value = dynamic_cast<MaterialParameterFloat3 *>(mtl_param)->m_Value;
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(0)->SetValue((_variant_t)Value.x);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(1)->SetValue((_variant_t)Value.y);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(2)->SetValue((_variant_t)Value.z);
		break;
	}
	case MaterialParameter::ParameterTypeFloat4:
	{
		const my::Vector4 & Value = dynamic_cast<MaterialParameterFloat4 *>(mtl_param)->m_Value;
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(0)->SetValue((_variant_t)Value.x);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(1)->SetValue((_variant_t)Value.y);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(2)->SetValue((_variant_t)Value.z);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(3)->SetValue((_variant_t)Value.w);
		break;
	}
	case MaterialParameter::ParameterTypeTexture:
		pParentCtrl->GetSubItem(NodeId)->SetValue((_variant_t)
			dynamic_cast<MaterialParameterTexture *>(mtl_param)->m_TexturePath.c_str());
		break;
	}
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
			TCHAR buff[128];
			_stprintf_s(buff, _countof(buff), _T("Material%d"), i);
			CreatePropertiesMaterial(pMaterialList, buff, cloth_cmp->m_MaterialList[i].get());
			continue;
		}
		UpdatePropertiesMaterial(pMaterialList->GetSubItem(i), cloth_cmp->m_MaterialList[i].get());
	}
	RemovePropertiesFrom(pMaterialList, cloth_cmp->m_MaterialList.size());
}

void CPropertiesWnd::UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)(VARIANT_BOOL)emit_cmp->m_EmitterToWorld);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(0)->SetValue((_variant_t)emit_cmp->m_ParticleOffset.x);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(1)->SetValue((_variant_t)emit_cmp->m_ParticleOffset.y);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(2)->SetValue((_variant_t)emit_cmp->m_ParticleOffset.z);
	CMFCPropertyGridProperty * pParticleList = pComponent->GetSubItem(PropId + 2);
	if (!pParticleList || pParticleList->GetData() != PropertyEmitterParticleList)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesStaticEmitter(pComponent, emit_cmp);
		return;
	}
	pParticleList->GetSubItem(0)->SetValue((_variant_t)emit_cmp->m_ParticleList.size());
	for (unsigned int i = 0; i < emit_cmp->m_ParticleList.size(); i++)
	{
		if ((unsigned int)pParticleList->GetSubItemsCount() <= i + 1)
		{
			CreatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
			continue;
		}
		UpdatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
	}
	RemovePropertiesFrom(pParticleList, 1 + emit_cmp->m_ParticleList.size());
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 3), emit_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp)
{
	CMFCPropertyGridProperty * pParticle = pParentProp->GetSubItem(NodeId + 1);
	_ASSERT(pParticle);
	my::Emitter::Particle & particle = emit_cmp->m_ParticleList[NodeId];
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
	pProp = pParticle->GetSubItem(5); _ASSERT(pProp->GetData() == PropertyEmitterParticleAngle); pProp->SetValue((_variant_t)D3DXToDegree(particle.m_Angle));
}

void CPropertiesWnd::UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pParticleLifeTime = pComponent->GetSubItem(PropId + 2);
	if (!pParticleLifeTime || pParticleLifeTime->GetData() != PropertySphericalEmitterParticleCapacity)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesSphericalEmitter(pComponent, sphe_emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)(VARIANT_BOOL)sphe_emit_cmp->m_EmitterToWorld);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleOffset.x);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleOffset.y);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleOffset.z);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleList.capacity());
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleLifeTime);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnInterval);
	pComponent->GetSubItem(PropId + 5)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.x);
	pComponent->GetSubItem(PropId + 5)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.y);
	pComponent->GetSubItem(PropId + 5)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.z);
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnSpeed);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 7), &sphe_emit_cmp->m_SpawnInclination);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 8), &sphe_emit_cmp->m_SpawnAzimuth);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 9), &sphe_emit_cmp->m_SpawnColorR);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 10), &sphe_emit_cmp->m_SpawnColorG);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 11), &sphe_emit_cmp->m_SpawnColorB);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 12), &sphe_emit_cmp->m_SpawnColorA);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 13), &sphe_emit_cmp->m_SpawnSizeX);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 14), &sphe_emit_cmp->m_SpawnSizeY);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 15), &sphe_emit_cmp->m_SpawnAngle);
	pComponent->GetSubItem(PropId + 16)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnLoopTime);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 17), sphe_emit_cmp->m_Material.get());
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

void CPropertiesWnd::UpdatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain, const CPoint & chunkid)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyTerrainRowChunks)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesTerrain(pComponent, terrain, chunkid);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)terrain->m_RowChunks);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)terrain->m_ColChunks);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)terrain->m_ChunkSize);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)terrain->m_HeightScale);
	pComponent->GetSubItem(PropId + 4);
	pComponent->GetSubItem(PropId + 5);
	TerrainChunk * chunk = GetTerrainChunkSafe(terrain, chunkid);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 6), chunk->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesActor(Actor * actor)
{
	CMFCPropertyGridProperty * pActor = new CSimpleProp(GetComponentTypeName(Component::ComponentTypeActor), PropertyActor, FALSE);
	m_wndPropList.AddProperty(pActor, FALSE, FALSE);
	pActor->SetValue((_variant_t)(DWORD_PTR)actor); // ! only worked on 32bit system

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMFCPropertyGridProperty * pAABB = new CSimpleProp(_T("AABB"), PropertyActorAABB, TRUE);
	pActor->AddSubItem(pAABB);
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
	pActor->AddSubItem(pPosition);
	pProp = new CSimpleProp(_T("x"), (_variant_t)actor->m_Position.x, NULL, PropertyActorPosX);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)actor->m_Position.y, NULL, PropertyActorPosY);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)actor->m_Position.z, NULL, PropertyActorPosZ);
	pPosition->AddSubItem(pProp);

	my::Vector3 angle = actor->m_Rotation.ToEulerAngles();
	CMFCPropertyGridProperty * pRotate = new CSimpleProp(_T("Rotate"), PropertyActorRot, TRUE);
	pActor->AddSubItem(pRotate);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertyActorRotX);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertyActorRotY);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertyActorRotZ);
	pRotate->AddSubItem(pProp);

	CMFCPropertyGridProperty * pScale = new CSimpleProp(_T("Scale"), PropertyActorScale, TRUE);
	pActor->AddSubItem(pScale);
	pProp = new CSimpleProp(_T("x"), (_variant_t)actor->m_Scale.x, NULL, PropertyActorScaleX);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)actor->m_Scale.y, NULL, PropertyActorScaleY);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)actor->m_Scale.z, NULL, PropertyActorScaleZ);
	pScale->AddSubItem(pProp);

	CMFCPropertyGridProperty * pLodRatio = new CSimpleProp(_T("LodRatio"), (_variant_t)actor->m_LodRatio, NULL, PropertyActorLodRatio);
	pActor->AddSubItem(pLodRatio);

	CMFCPropertyGridProperty * pRigidActor = new CComboProp(_T("RigidActor"), g_ActorTypeDesc[actor->m_PxActor ? actor->m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT], NULL, PropertyActorRigidActor);
	for (unsigned int i = 0; i < _countof(g_ActorTypeDesc); i++)
	{
		pRigidActor->AddOption(g_ActorTypeDesc[i], TRUE);
	}
	pActor->AddSubItem(pRigidActor);

	if (!actor->m_Cmps.empty())
	{
		Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
		for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
		{
			CreateProperties(pActor, cmp_iter->get());
		}
	}
}

void CPropertiesWnd::CreateProperties(CMFCPropertyGridProperty * pParentCtrl, Component * cmp)
{
	CMFCPropertyGridProperty * pComponent = new CSimpleProp(GetComponentTypeName(cmp->m_Type), GetComponentProp(cmp->m_Type), FALSE);
	pParentCtrl->AddSubItem(pComponent);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp); // ! only worked on 32bit system

	CMFCPropertyGridProperty * pLodMask = new CComboProp(_T("LodMask"), (_variant_t)GetLodMaskDesc(cmp->m_LodMask), NULL, PropertyComponentLODMask);
	for (unsigned int i = 0; i < _countof(g_LodMaskDesc); i++)
	{
		pLodMask->AddOption(g_LodMaskDesc[i].desc, TRUE);
	}
	pComponent->AddSubItem(pLodMask);

	CreatePropertiesShape(pComponent, cmp);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
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
		CreatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp), pFrame->m_selchunkid);
		break;
	}
}

void CPropertiesWnd::CreatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, Component * cmp)
{
	CMFCPropertyGridProperty * pShape = new CSimpleProp(_T("Shape"), PropertyShape, FALSE);
	pParentCtrl->AddSubItem(pShape);
	CMFCPropertyGridProperty * pType = new CComboProp(_T("Type"), g_ShapeTypeDesc[cmp->m_PxShape ? cmp->m_PxShape->getGeometryType() : physx::PxGeometryType::eGEOMETRY_COUNT], NULL, PropertyShapeType);
	for (unsigned int i = 0; i < _countof(g_ShapeTypeDesc); i++)
	{
		pType->AddOption(g_ShapeTypeDesc[i], TRUE);
	}
	pShape->AddSubItem(pType);

	physx::PxTransform localPose;
	physx::PxFilterData filterData;
	physx::PxShapeFlags shapeFlags;
	if (cmp->m_PxShape)
	{
		localPose = cmp->m_PxShape->getLocalPose();
		filterData = cmp->m_PxShape->getQueryFilterData();
		shapeFlags = cmp->m_PxShape->getFlags();
	}

	CMFCPropertyGridProperty * pLocalPos = new CSimpleProp(_T("LocalPos"), PropertyShapeLocalPos, TRUE);
	pShape->AddSubItem(pLocalPos);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)localPose.p.x, NULL, PropertyShapeLocalPosX);
	pLocalPos->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)localPose.p.y, NULL, PropertyShapeLocalPosY);
	pLocalPos->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)localPose.p.z, NULL, PropertyShapeLocalPosZ);
	pLocalPos->AddSubItem(pProp);

	my::Vector3 angle = ((my::Quaternion &)localPose.q).ToEulerAngles();
	CMFCPropertyGridProperty * pLocalRot = new CSimpleProp(_T("LocalRot"), PropertyShapeLocalRot, TRUE);
	pShape->AddSubItem(pLocalRot);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertyShapeLocalRotX);
	pLocalRot->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertyShapeLocalRotY);
	pLocalRot->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertyShapeLocalRotZ);
	pLocalRot->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("FilterData"), (_variant_t)filterData.word0, NULL, PropertyShapeFilterData);
	pShape->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("Simulation"), shapeFlags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE), NULL, PropertyShapeSimulation);
	pShape->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("SceneQuery"), shapeFlags.isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE), NULL, PropertyShapeSceneQuery);
	pShape->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("Trigger"), shapeFlags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE), NULL, PropertyShapeTrigger);
	pShape->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("Visualization"), shapeFlags.isSet(physx::PxShapeFlag::eVISUALIZATION), NULL, PropertyShapeVisualization);
	pShape->AddSubItem(pProp);
	UpdatePropertiesShapeShow(pShape, cmp->m_PxShape != NULL);
}

void CPropertiesWnd::CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("MeshPath"), TRUE, (_variant_t)ms2ts(mesh_cmp->m_MeshPath).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshResPath);
	pComponent->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("Instance"), (_variant_t)mesh_cmp->m_bInstance, NULL, PropertyMeshInstance);
	pComponent->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("UseAnimation"), (_variant_t)mesh_cmp->m_bUseAnimation, NULL, PropertyMeshUseAnimation);
	pComponent->AddSubItem(pProp);
	pProp = new CMFCPropertyGridProperty(_T("MaterialList"), PropertyMaterialList, FALSE);
	pComponent->AddSubItem(pProp);
	for (unsigned int i = 0; i < mesh_cmp->m_MaterialList.size(); i++)
	{
		TCHAR buff[128];
		_stprintf_s(buff, _countof(buff), _T("Material%d"), i);
		CreatePropertiesMaterial(pProp, buff, mesh_cmp->m_MaterialList[i].get());
	}
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, LPCTSTR lpszName, Material * mtl)
{
	CMFCPropertyGridProperty * pMaterial = new CSimpleProp(lpszName, PropertyMaterial, FALSE);
	pParentCtrl->AddSubItem(pMaterial);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mtl);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("Shader"), TRUE, (_variant_t)ms2ts(mtl->m_Shader).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialShader);
	pMaterial->AddSubItem(pProp);
	CComboProp * pPassMask = new CComboProp(_T("PassMask"), (_variant_t)GetPassMaskDesc(mtl->m_PassMask), NULL, PropertyMaterialPassMask);
	for (unsigned int i = 0; i < _countof(g_PassMaskDesc); i++)
	{
		pPassMask->AddOption(g_PassMaskDesc[i].desc, TRUE);
	}
	pMaterial->AddSubItem(pPassMask);
	CComboProp * pCullMode = new CComboProp(_T("CullMode"), (_variant_t)g_CullModeDesc[mtl->m_CullMode - 1], NULL, PropertyMaterialCullMode);
	for (unsigned int i = 0; i < _countof(g_CullModeDesc); i++)
	{
		pCullMode->AddOption(g_CullModeDesc[i], TRUE);
	}
	pMaterial->AddSubItem(pCullMode);
	CCheckBoxProp * pZEnable = new CCheckBoxProp(_T("ZEnable"), mtl->m_ZEnable, NULL, PropertyMaterialZEnable);
	pMaterial->AddSubItem(pZEnable);
	CCheckBoxProp * pZWriteEnable = new CCheckBoxProp(_T("ZWriteEnable"), mtl->m_ZWriteEnable, NULL, PropertyMaterialZWriteEnable);
	pMaterial->AddSubItem(pZWriteEnable);
	CComboProp * pBlendMode = new CComboProp(_T("BlendMode"), (_variant_t)g_BlendModeDesc[mtl->m_BlendMode], NULL, PropertyMaterialBlendMode);
	for (unsigned int i = 0; i < _countof(g_BlendModeDesc); i++)
	{
		pBlendMode->AddOption(g_BlendModeDesc[i], TRUE);
	}
	pMaterial->AddSubItem(pBlendMode);

	CMFCPropertyGridProperty * pParameterList = new CSimpleProp(_T("Parameters"), PropertyMaterialParameterList, FALSE);
	pMaterial->AddSubItem(pParameterList);
	for (unsigned int i = 0; i < mtl->m_ParameterList.size(); i++)
	{
		CreatePropertiesMaterialParameter(pParameterList, i, mtl->m_ParameterList[i].get());
	}
}

void CPropertiesWnd::CreatePropertiesMaterialParameter(CMFCPropertyGridProperty * pParentCtrl, int NodeId, MaterialParameter * mtl_param)
{
	CMFCPropertyGridProperty * pProp = NULL;
	switch (mtl_param->m_Type)
	{
	case MaterialParameter::ParameterTypeFloat:
		pProp = new CSimpleProp(ms2ts(mtl_param->m_Name).c_str(), (_variant_t)
			dynamic_cast<MaterialParameterFloat *>(mtl_param)->m_Value, NULL, PropertyMaterialParameterFloat);
		pParentCtrl->AddSubItem(pProp);
		break;
	case MaterialParameter::ParameterTypeFloat2:
	{
		const my::Vector2 & Value = dynamic_cast<MaterialParameterFloat2 *>(mtl_param)->m_Value;
		CMFCPropertyGridProperty * pParameter = new CSimpleProp(ms2ts(mtl_param->m_Name).c_str(), PropertyMaterialParameterFloat2, TRUE);
		pParentCtrl->AddSubItem(pParameter);
		pProp = new CSimpleProp(_T("x"), (_variant_t)Value.x, NULL, PropertyMaterialParameterFloatValueX);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("y"), (_variant_t)Value.y, NULL, PropertyMaterialParameterFloatValueY);
		pParameter->AddSubItem(pProp);
		break;
	}
	case MaterialParameter::ParameterTypeFloat3:
	{
		const my::Vector3 & Value = dynamic_cast<MaterialParameterFloat3 *>(mtl_param)->m_Value;
		CMFCPropertyGridProperty * pParameter = new CSimpleProp(ms2ts(mtl_param->m_Name).c_str(), PropertyMaterialParameterFloat3, TRUE);
		pParentCtrl->AddSubItem(pParameter);
		pProp = new CSimpleProp(_T("x"), (_variant_t)Value.x, NULL, PropertyMaterialParameterFloatValueX);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("y"), (_variant_t)Value.y, NULL, PropertyMaterialParameterFloatValueY);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("z"), (_variant_t)Value.z, NULL, PropertyMaterialParameterFloatValueZ);
		pParameter->AddSubItem(pProp);
		break;
	}
	case MaterialParameter::ParameterTypeFloat4:
	{
		const my::Vector4 & Value = dynamic_cast<MaterialParameterFloat4 *>(mtl_param)->m_Value;
		CMFCPropertyGridProperty * pParameter = new CSimpleProp(ms2ts(mtl_param->m_Name).c_str(), PropertyMaterialParameterFloat4, TRUE);
		pParentCtrl->AddSubItem(pParameter);
		pProp = new CSimpleProp(_T("x"), (_variant_t)Value.x, NULL, PropertyMaterialParameterFloatValueX);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("y"), (_variant_t)Value.y, NULL, PropertyMaterialParameterFloatValueY);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("z"), (_variant_t)Value.z, NULL, PropertyMaterialParameterFloatValueZ);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("w"), (_variant_t)Value.w, NULL, PropertyMaterialParameterFloatValueW);
		pParameter->AddSubItem(pProp);
		break;
	}
	case MaterialParameter::ParameterTypeTexture:
		pProp = new CFileProp(ms2ts(mtl_param->m_Name).c_str(), TRUE, (_variant_t)
			ms2ts(dynamic_cast<MaterialParameterTexture *>(mtl_param)->m_TexturePath).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialParameterTexture);
		pParentCtrl->AddSubItem(pProp);
		break;
	}
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
		TCHAR buff[128];
		_stprintf_s(buff, _countof(buff), _T("Material%d"), i);
		CreatePropertiesMaterial(pProp, buff, cloth_cmp->m_MaterialList[i].get());
	}
}

void CPropertiesWnd::CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pEmitterToWorld = new CCheckBoxProp(_T("EmitterToWorld"), (_variant_t)emit_cmp->m_EmitterToWorld, NULL, PropertyEmitterToWorld);
	pComponent->AddSubItem(pEmitterToWorld);
	CMFCPropertyGridProperty * pEmitterOffset = new CSimpleProp(_T("ParticleOffset"), PropertyEmitterOffset, TRUE);
	pComponent->AddSubItem(pEmitterOffset);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)emit_cmp->m_ParticleOffset.x, NULL, PropertyEmitterOffsetX);
	pEmitterOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)emit_cmp->m_ParticleOffset.y, NULL, PropertyEmitterOffsetY);
	pEmitterOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)emit_cmp->m_ParticleOffset.z, NULL, PropertyEmitterOffsetZ);
	pEmitterOffset->AddSubItem(pProp);
	CMFCPropertyGridProperty * pParticleList = new CSimpleProp(_T("ParticleList"), PropertyEmitterParticleList, FALSE);
	pComponent->AddSubItem(pParticleList);
	pProp = new CSimpleProp(_T("ParticleCount"), (_variant_t)emit_cmp->m_ParticleList.size(), NULL, PropertyEmitterParticleCount);
	pParticleList->AddSubItem(pProp);
	for (unsigned int i = 0; i < emit_cmp->m_ParticleList.size(); i++)
	{
		CreatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
	}
	CreatePropertiesMaterial(pComponent, _T("Material"), emit_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, EmitterComponent * emit_cmp)
{
	TCHAR buff[128];
	_stprintf_s(buff, _countof(buff), _T("Particle%d"), NodeId);
	CMFCPropertyGridProperty * pParticle = new CSimpleProp(buff, NodeId, FALSE);
	pParentProp->AddSubItem(pParticle);
	CMFCPropertyGridProperty * pPosition = new CMFCPropertyGridProperty(_T("Position"), PropertyEmitterParticlePosition, TRUE);
	pParticle->AddSubItem(pPosition);
	my::Emitter::Particle & particle = emit_cmp->m_ParticleList[NodeId];
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

	pProp = new CMFCPropertyGridProperty(_T("Angle"), (_variant_t)D3DXToDegree(particle.m_Angle), NULL, PropertyEmitterParticleAngle);
	pParticle->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pEmitterToWorld = new CCheckBoxProp(_T("EmitterToWorld"), (_variant_t)sphe_emit_cmp->m_EmitterToWorld, NULL, PropertyEmitterToWorld);
	pComponent->AddSubItem(pEmitterToWorld);
	CMFCPropertyGridProperty * pEmitterOffset = new CSimpleProp(_T("ParticleOffset"), PropertyEmitterOffset, TRUE);
	pComponent->AddSubItem(pEmitterOffset);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)sphe_emit_cmp->m_ParticleOffset.x, NULL, PropertyEmitterOffsetX);
	pEmitterOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)sphe_emit_cmp->m_ParticleOffset.y, NULL, PropertyEmitterOffsetY);
	pEmitterOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)sphe_emit_cmp->m_ParticleOffset.z, NULL, PropertyEmitterOffsetZ);
	pEmitterOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ParticleCapacity"), (_variant_t)sphe_emit_cmp->m_ParticleList.capacity(), NULL, PropertySphericalEmitterParticleCapacity);
	pComponent->AddSubItem(pProp);
	CMFCPropertyGridProperty * pParticleLifeTime = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)sphe_emit_cmp->m_ParticleLifeTime, NULL, PropertySphericalEmitterParticleLifeTime);
	pComponent->AddSubItem(pParticleLifeTime);
	CMFCPropertyGridProperty * pSpawnInterval = new CSimpleProp(_T("SpawnInterval"), (_variant_t)sphe_emit_cmp->m_SpawnInterval, NULL, PropertySphericalEmitterSpawnInterval);
	pComponent->AddSubItem(pSpawnInterval);
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
	CreatePropertiesMaterial(pComponent, _T("Material"), sphe_emit_cmp->m_Material.get());
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

void CPropertiesWnd::CreatePropertiesTerrain(CMFCPropertyGridProperty * pComponent, Terrain * terrain, const CPoint & chunkid)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("RowChunks"), (_variant_t)terrain->m_RowChunks, NULL, PropertyTerrainRowChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ColChunks"), (_variant_t)terrain->m_ColChunks, NULL, PropertyTerrainColChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ChunkSize"), (_variant_t)terrain->m_ChunkSize, NULL, PropertyTerrainChunkSize);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("HeightScale"), (_variant_t)terrain->m_HeightScale, NULL, PropertyTerrainHeightScale);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("HeightMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainHeightMap);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("SplatMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainSplatMap);
	pComponent->AddSubItem(pProp);
	TerrainChunk * chunk = GetTerrainChunkSafe(terrain, chunkid);
	CreatePropertiesMaterial(pComponent, _T("ChunkMaterial"), chunk->m_Material.get());
}

CPropertiesWnd::Property CPropertiesWnd::GetComponentProp(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return PropertyActor;
	case Component::ComponentTypeCharacter:
		return PropertyCharacter;
	case Component::ComponentTypeMesh:
		return PropertyMesh;
	case Component::ComponentTypeCloth:
		return PropertyCloth;
	case Component::ComponentTypeStaticEmitter:
		return PropertyStaticEmitter;
	case Component::ComponentTypeSphericalEmitter:
		return PropertySphericalEmitter;
	case Component::ComponentTypeTerrain:
		return PropertyTerrain;
	}
	return PropertyUnknown;
}

unsigned int CPropertiesWnd::GetComponentPropCount(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return 6;
	case Component::ComponentTypeCharacter:
		return GetComponentPropCount(Component::ComponentTypeActor);
	case Component::ComponentTypeMesh:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 4;
	case Component::ComponentTypeCloth:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 2;
	case Component::ComponentTypeStaticEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 4;
	case Component::ComponentTypeSphericalEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 18;
	case Component::ComponentTypeTerrain:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 7;
	}

	ASSERT(Component::ComponentTypeComponent == type);
	return 2;
}

LPCTSTR CPropertiesWnd::GetComponentTypeName(DWORD type)
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

TerrainChunk * CPropertiesWnd::GetTerrainChunkSafe(Terrain * terrain, const CPoint & chunkid)
{
	if (chunkid.x >= 0 && chunkid.x < (int)terrain->m_Chunks.shape()[0]
		&& chunkid.y >= 0 && chunkid.y < (int)terrain->m_Chunks.shape()[1])
	{
		return terrain->m_Chunks[chunkid.x][chunkid.y];
	}
	return terrain->m_Chunks[0][0];
}

CPropertiesWnd::Property CPropertiesWnd::GetMaterialParameterTypeProp(DWORD type)
{
	switch (type)
	{
	case MaterialParameter::ParameterTypeFloat:
		return PropertyMaterialParameterFloat;
	case MaterialParameter::ParameterTypeFloat2:
		return PropertyMaterialParameterFloat2;
	case MaterialParameter::ParameterTypeFloat3:
		return PropertyMaterialParameterFloat3;
	case MaterialParameter::ParameterTypeFloat4:
		return PropertyMaterialParameterFloat4;
	case MaterialParameter::ParameterTypeTexture:
		return PropertyMaterialParameterTexture;
	}
	ASSERT(FALSE);
	return PropertyUnknown;
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
		actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
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
		CMFCPropertyGridProperty * pActor = NULL;
		switch (PropertyId)
		{
		case PropertyActorPos:
		case PropertyActorRot:
		case PropertyActorScale:
			pActor = pProp->GetParent();
			break;
		default:
			pActor = pProp->GetParent()->GetParent();
			break;
		}
		Actor * actor = (Actor *)pActor->GetValue().ulVal;
		actor->m_Position.x = pActor->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
		actor->m_Position.y = pActor->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
		actor->m_Position.z = pActor->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
		actor->m_Rotation = my::Quaternion::RotationEulerAngles(my::Vector3(
			D3DXToRadian(pActor->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal),
			D3DXToRadian(pActor->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal),
			D3DXToRadian(pActor->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal)));
		actor->m_Scale.x = pActor->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal;
		actor->m_Scale.y = pActor->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal;
		actor->m_Scale.z = pActor->GetSubItem(3)->GetSubItem(2)->GetValue().fltVal;
		actor->UpdateAABB();
		actor->UpdateWorld();
		actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();

		if (actor->m_PxActor)
		{
			actor->m_PxActor->setGlobalPose(physx::PxTransform(
				(physx::PxVec3&)actor->m_Position, (physx::PxQuat&)actor->m_Rotation));
		}
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyActorLodRatio:
	{
		Actor * actor = (Actor *)pProp->GetParent()->GetValue().ulVal;
		actor->m_LodRatio = pProp->GetValue().fltVal;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
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
		break;
	}
	case PropertyComponentLODMask:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_LodMaskDesc));
		cmp->m_LodMask = (Component::LODMask)g_LodMaskDesc[i].mask;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeType:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
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
		{
			cmp->ClearShape();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
			break;
		}
		}
		break;
	}
	case PropertyShapeLocalPos:
	case PropertyShapeLocalPosX:
	case PropertyShapeLocalPosY:
	case PropertyShapeLocalPosZ:
	case PropertyShapeLocalRot:
	case PropertyShapeLocalRotX:
	case PropertyShapeLocalRotY:
	case PropertyShapeLocalRotZ:
	{
		CMFCPropertyGridProperty * pShape = NULL;
		switch (PropertyId)
		{
		case PropertyShapeLocalPos:
		case PropertyShapeLocalRot:
			pShape = pProp->GetParent();
			break;
		case PropertyShapeLocalPosX:
		case PropertyShapeLocalPosY:
		case PropertyShapeLocalPosZ:
		case PropertyShapeLocalRotX:
		case PropertyShapeLocalRotY:
		case PropertyShapeLocalRotZ:
			pShape = pProp->GetParent()->GetParent();
			break;
		}
		Component * cmp = (Component *)pShape->GetParent()->GetValue().ulVal;
		ASSERT(cmp->m_PxShape);
		physx::PxVec3 localPos(
			pShape->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal,
			pShape->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal,
			pShape->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal);
		physx::PxQuat localRot = (physx::PxQuat &)my::Quaternion::RotationEulerAngles(my::Vector3(
			D3DXToRadian(pShape->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal),
			D3DXToRadian(pShape->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal),
			D3DXToRadian(pShape->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal)));
		cmp->m_Actor->m_PxActor->detachShape(*cmp->m_PxShape, false);
		cmp->m_PxShape->setLocalPose(physx::PxTransform(localPos, localRot));
		cmp->m_Actor->m_PxActor->attachShape(*cmp->m_PxShape);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeFilterData:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_Actor->m_PxActor->detachShape(*cmp->m_PxShape, false);
		physx::PxFilterData filterData = cmp->m_PxShape->getQueryFilterData();
		filterData.word0 = pProp->GetValue().uintVal;
		cmp->m_PxShape->setQueryFilterData(filterData);
		cmp->m_Actor->m_PxActor->attachShape(*cmp->m_PxShape);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeSimulation:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_Actor->m_PxActor->detachShape(*cmp->m_PxShape, false);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, pProp->GetValue().boolVal != 0);
		cmp->m_Actor->m_PxActor->attachShape(*cmp->m_PxShape);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeSceneQuery:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_Actor->m_PxActor->detachShape(*cmp->m_PxShape, false);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, pProp->GetValue().boolVal != 0);
		cmp->m_Actor->m_PxActor->attachShape(*cmp->m_PxShape);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeTrigger:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_Actor->m_PxActor->detachShape(*cmp->m_PxShape, false);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, pProp->GetValue().boolVal != 0);
		cmp->m_Actor->m_PxActor->attachShape(*cmp->m_PxShape);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeVisualization:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_Actor->m_PxActor->detachShape(*cmp->m_PxShape, false);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, pProp->GetValue().boolVal != 0);
		cmp->m_Actor->m_PxActor->attachShape(*cmp->m_PxShape);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshResPath:
	{
		//MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
		//mesh_cmp->m_MeshRes.ReleaseResource();
		//mesh_cmp->m_MeshRes.m_Path = ts2ms(pProp->GetValue().bstrVal);
		//mesh_cmp->m_MeshRes.RequestResource();
		//EventArgs arg;
		//pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshInstance:
	{
		MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
		mesh_cmp->m_bInstance = pProp->GetValue().boolVal != 0;
		// ! update whole actor shader cache
		CMainFrame::ActorSet::const_iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			(*sel_iter)->ReleaseResource();
			(*sel_iter)->OnShaderChanged();
			(*sel_iter)->RequestResource();
		}
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshUseAnimation:
	{
		MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
		mesh_cmp->m_bUseAnimation = pProp->GetValue().boolVal != 0;
		// ! update whole actor shader cache
		CMainFrame::ActorSet::const_iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			(*sel_iter)->ReleaseResource();
			(*sel_iter)->OnShaderChanged();
			(*sel_iter)->RequestResource();
		}
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialShader:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		material->m_Shader = ts2ms(pProp->GetValue().bstrVal);
		// ! update whole actor shader cache
		CMainFrame::ActorSet::const_iterator sel_iter = pFrame->m_selactors.begin();
		for (; sel_iter != pFrame->m_selactors.end(); sel_iter++)
		{
			(*sel_iter)->ReleaseResource();
			(*sel_iter)->OnShaderChanged();
			(*sel_iter)->RequestResource();
		}
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialPassMask:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_PassMaskDesc));
		material->m_PassMask = g_PassMaskDesc[i].mask;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialCullMode:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_CullModeDesc));
		material->m_CullMode = i + 1;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialZEnable:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		material->m_ZEnable = pProp->GetValue().boolVal != 0;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialZWriteEnable:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		material->m_ZWriteEnable = pProp->GetValue().boolVal != 0;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialBlendMode:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().ulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_BlendModeDesc));
		material->m_BlendMode = i;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterFloat:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat);
		boost::dynamic_pointer_cast<MaterialParameterFloat>(mtl->m_ParameterList[i])->m_Value = pProp->GetValue().fltVal;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterFloat2:
	case PropertyMaterialParameterFloat3:
	case PropertyMaterialParameterFloat4:
	case PropertyMaterialParameterFloatValueX:
	case PropertyMaterialParameterFloatValueY:
	case PropertyMaterialParameterFloatValueZ:
	case PropertyMaterialParameterFloatValueW:
	{
		CMFCPropertyGridProperty * pParameter = NULL;
		switch (PropertyId)
		{
		case PropertyMaterialParameterFloat2:
		case PropertyMaterialParameterFloat3:
		case PropertyMaterialParameterFloat4:
			pParameter = pProp;
			break;
		case PropertyMaterialParameterFloatValueX:
		case PropertyMaterialParameterFloatValueY:
		case PropertyMaterialParameterFloatValueZ:
		case PropertyMaterialParameterFloatValueW:
			pParameter = pProp->GetParent();
			break;
		}
		ASSERT(pParameter);
		Material * mtl = (Material *)pParameter->GetParent()->GetParent()->GetValue().ulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pParameter);
		switch (pParameter->GetData())
		{
		case PropertyMaterialParameterFloat2:
			ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat2);
			boost::dynamic_pointer_cast<MaterialParameterFloat2>(mtl->m_ParameterList[i])->m_Value = my::Vector2(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal);
			break;
		case PropertyMaterialParameterFloat3:
			ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat3);
			boost::dynamic_pointer_cast<MaterialParameterFloat3>(mtl->m_ParameterList[i])->m_Value = my::Vector3(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal, pParameter->GetSubItem(2)->GetValue().fltVal);
			break;
		case PropertyMaterialParameterFloat4:
			ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat4);
			boost::dynamic_pointer_cast<MaterialParameterFloat4>(mtl->m_ParameterList[i])->m_Value = my::Vector4(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal, pParameter->GetSubItem(2)->GetValue().fltVal, pParameter->GetSubItem(3)->GetValue().fltVal);
			break;
		}
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterTexture:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeTexture);
		mtl->m_ParameterList[i]->ReleaseResource();
		boost::dynamic_pointer_cast<MaterialParameterTexture>(mtl->m_ParameterList[i])->m_TexturePath = ts2ms(pProp->GetValue().bstrVal);
		mtl->m_ParameterList[i]->RequestResource();
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyClothSceneCollision:
	{
		ClothComponent * cloth_cmp = (ClothComponent *)pProp->GetParent()->GetValue().ulVal;
		cloth_cmp->m_Cloth->setClothFlag(physx::PxClothFlag::eSCENE_COLLISION, pProp->GetValue().boolVal != 0);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterToWorld:
	{
		EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetValue().ulVal;
		emit_cmp->m_EmitterToWorld = pProp->GetValue().boolVal != 0;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterOffset:
	case PropertyEmitterOffsetX:
	case PropertyEmitterOffsetY:
	case PropertyEmitterOffsetZ:
	{
		CMFCPropertyGridProperty * pComponent = NULL;
		switch (PropertyId)
		{
		case PropertyEmitterOffsetX:
		case PropertyEmitterOffsetY:
		case PropertyEmitterOffsetZ:
			pComponent = pProp->GetParent()->GetParent();
			break;
		case PropertyEmitterOffset:
			pComponent = pProp->GetParent();
			break;
		}
		EmitterComponent * emit_cmp = (EmitterComponent *)pComponent->GetValue().ulVal;
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		emit_cmp->m_ParticleOffset = pComponent->GetSubItem(PropId + 1)->GetSubItem(0)->GetValue().fltVal;
		emit_cmp->m_ParticleOffset = pComponent->GetSubItem(PropId + 1)->GetSubItem(1)->GetValue().fltVal;
		emit_cmp->m_ParticleOffset = pComponent->GetSubItem(PropId + 1)->GetSubItem(2)->GetValue().fltVal;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterParticleCount:
	{
		EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetParent()->GetValue().ulVal;
		unsigned int new_size = pProp->GetValue().uintVal;
		if (new_size < emit_cmp->m_ParticleList.size())
		{
			emit_cmp->m_ParticleList.set_capacity(new_size);
		}
		else
		{
			emit_cmp->m_ParticleList.resize(new_size, my::Emitter::Particle(my::Vector3(0, 0, 0), my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0, 0));
		}
		UpdatePropertiesStaticEmitter(pProp->GetParent()->GetParent(), emit_cmp);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
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
		my::Emitter::Particle & particle = emit_cmp->m_ParticleList[NodeId];
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
		particle.m_Angle = D3DXToRadian(pParticle->GetSubItem(5)->GetValue().fltVal);
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertySphericalEmitterParticleCapacity:
	{
		EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetValue().ulVal;
		unsigned int new_size = pProp->GetValue().uintVal;
		emit_cmp->m_ParticleList.set_capacity(new_size);
		UpdatePropertiesSphericalEmitter(pProp->GetParent(), dynamic_cast<SphericalEmitterComponent *>(emit_cmp));
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
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
		sphe_emit_cmp->m_ParticleLifeTime = pComponent->GetSubItem(PropId + 3)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnInterval = pComponent->GetSubItem(PropId + 4)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.x = pComponent->GetSubItem(PropId + 5)->GetSubItem(0)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.y = pComponent->GetSubItem(PropId + 5)->GetSubItem(1)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.z = pComponent->GetSubItem(PropId + 5)->GetSubItem(2)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnSpeed = pComponent->GetSubItem(PropId + 6)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnLoopTime = pComponent->GetSubItem(PropId + 16)->GetValue().fltVal;
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
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
		break;
	}
	case PropertyTerrainHeightScale:
	{
		Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
		terrain->m_HeightScale = pProp->GetValue().fltVal;
		//terrain->UpdateHeightMapNormal();
		//terrain->UpdateChunks();
		//Actor * actor = terrain->m_Actor;
		//actor->UpdateAABB();
		//actor->UpdateOctNode();
		//pFrame->UpdateSelBox();
		//pFrame->UpdatePivotTransform();
		EventArgs arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyTerrainHeightMap:
	{
		std::string path = ts2ms(pProp->GetValue().bstrVal);
		my::Texture2DPtr res = boost::dynamic_pointer_cast<my::Texture2D>(theApp.LoadTexture(path.c_str()));
		if (res)
		{
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->UpdateHeightMap(res.get());
			Actor * actor = terrain->m_Actor;
			actor->UpdateAABB();
			actor->UpdateOctNode();
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	}
	case PropertyTerrainSplatMap:
	{
		std::string path = ts2ms(pProp->GetValue().bstrVal);
		my::Texture2DPtr res = boost::dynamic_pointer_cast<my::Texture2D>(theApp.LoadTexture(path.c_str()));
		if (res)
		{
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->UpdateSplatmap(res.get());
			EventArgs arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	}
	}
	return 0;
}

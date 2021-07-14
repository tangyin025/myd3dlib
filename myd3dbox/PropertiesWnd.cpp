
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "CtrlProps.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "ShapeDlg.h"
#include "Material.h"
#include "Terrain.h"
#include "Animation.h"
#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "ImportHeightDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CPropertiesWnd::PassMaskDesc g_PassMaskDesc[] =
{
	{ _T("None"), RenderPipeline::PassMaskNone },
	{ _T("Light"), RenderPipeline::PassMaskLight },
	{ _T("Background"), RenderPipeline::PassMaskBackground },
	{ _T("Opaque"), RenderPipeline::PassMaskOpaque },
	{ _T("NormalOpaque"), RenderPipeline::PassMaskNormalOpaque },
	{ _T("ShadowNormalOpaque"), RenderPipeline::PassMaskShadowNormalOpaque },
	{ _T("Transparent"), RenderPipeline::PassMaskTransparent },
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

static const LPCTSTR g_CullModeDesc[] =
{
	_T("NONE"),
	_T("CW"),
	_T("CCW")
};

static const LPCTSTR g_BlendModeDesc[] =
{
	_T("None"),
	_T("Alpha"),
	_T("Additive")
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

static LPCTSTR g_EmitterFaceType[EmitterComponent::FaceTypeAngleCamera + 1] =
{
	_T("X"),
	_T("Y"),
	_T("Z"),
	_T("Camera"),
	_T("Angle"),
	_T("AngleCamera"),
};

static LPCTSTR g_EmitterSpaceType[EmitterComponent::SpaceTypeLocal + 1] =
{
	_T("World"),
	_T("Local"),
};

static LPCTSTR g_EmitterVelType[EmitterComponent::VelocityTypeVel + 1] =
{
	_T("None"),
	_T("Velocity"),
};

static LPCTSTR g_EmitterPrimitiveType[EmitterComponent::PrimitiveTypeQuad + 1] =
{
	_T("Triangle"),
	_T("Quad"),
};

static LPCTSTR g_AnimationNodeType[] =
{
	_T("Sequence"),
	_T("None")
};

static LPCTSTR g_PaintType[CMainFrame::PaintTypeEmitterInstance + 1] =
{
	_T("PaintNone"),
	_T("PaintTerrainHeightField"),
	_T("PaintTerrainColor"),
	_T("PaintEmitterInstance"),
};

static LPCTSTR g_PaintShape[CMainFrame::PaintShapeCircle + 1] =
{
	_T("Circle"),
};

static LPCTSTR g_PaintMode[CMainFrame::PaintModeGreater + 1] =
{
	_T("Greater"),
};

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
	: m_IsOnPropertyChanged(FALSE)
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

void CPropertiesWnd::OnSelectionChanged(my::EventArg * arg)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMainFrame::SelActorList::iterator actor_iter = pFrame->m_selactors.begin();
	if (actor_iter != pFrame->m_selactors.end())
	{
		if (!m_IsOnPropertyChanged)
		{
			if (pFrame->m_PaintType == CMainFrame::PaintTypeTerrainHeightField
				|| pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor
				|| pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
			{
				UpdatePropertiesPaintTool();
				m_wndPropList.AdjustLayout();
			}
			else
			{
				UpdatePropertiesActor(*actor_iter);
				m_wndPropList.AdjustLayout();
			}
		}
	}
	else
	{
		m_wndPropList.RemoveAll();
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
	pActor->GetSubItem(0)->SetValue((_variant_t)ms2ts(actor->GetName()).c_str());
	pActor->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)actor->m_aabb.m_min.x);
	pActor->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)actor->m_aabb.m_min.y);
	pActor->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)actor->m_aabb.m_min.z);
	pActor->GetSubItem(1)->GetSubItem(3)->SetValue((_variant_t)actor->m_aabb.m_max.x);
	pActor->GetSubItem(1)->GetSubItem(4)->SetValue((_variant_t)actor->m_aabb.m_max.y);
	pActor->GetSubItem(1)->GetSubItem(5)->SetValue((_variant_t)actor->m_aabb.m_max.z);
	pActor->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)actor->m_Position.x);
	pActor->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)actor->m_Position.y);
	pActor->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)actor->m_Position.z);
	my::Vector3 angle = actor->m_Rotation.toEulerAngles();
	pActor->GetSubItem(3)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pActor->GetSubItem(3)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pActor->GetSubItem(3)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pActor->GetSubItem(4)->GetSubItem(0)->SetValue((_variant_t)actor->m_Scale.x);
	pActor->GetSubItem(4)->GetSubItem(1)->SetValue((_variant_t)actor->m_Scale.y);
	pActor->GetSubItem(4)->GetSubItem(2)->SetValue((_variant_t)actor->m_Scale.z);
	pActor->GetSubItem(5)->SetValue((_variant_t)actor->m_LodDist);
	pActor->GetSubItem(6)->SetValue((_variant_t)actor->m_LodFactor);
	pActor->GetSubItem(7)->SetValue((_variant_t)actor->m_CullingDist);
	UpdatePropertiesRigidActor(pActor->GetSubItem(8), actor);
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
	RemovePropertiesFrom(pActor, GetComponentPropCount(Component::ComponentTypeActor) + (int)actor->m_Cmps.size());
	//m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::UpdatePropertiesRigidActor(CMFCPropertyGridProperty * pRigidActor, Actor * actor)
{
	if (!actor->m_PxActor)
	{
		pRigidActor->GetSubItem(0)->SetValue((_variant_t)g_ActorTypeDesc[physx::PxActorType::eACTOR_COUNT]);
		pRigidActor->GetSubItem(1)->Show(FALSE, FALSE);
		return;
	}
	pRigidActor->GetSubItem(0)->SetValue((_variant_t)g_ActorTypeDesc[actor->m_PxActor ? actor->m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT]);
	physx::PxRigidBodyFlags bodyFlags;
	if (actor->m_PxActor)
	{
		physx::PxRigidBody * body = actor->m_PxActor->is<physx::PxRigidBody>();
		if (body)
		{
			bodyFlags = body->getRigidBodyFlags();
		}
	}
	pRigidActor->GetSubItem(1)->SetValue((_variant_t)(VARIANT_BOOL)bodyFlags.isSet(physx::PxRigidBodyFlag::eKINEMATIC));
	pRigidActor->GetSubItem(1)->Show(actor->m_PxActor && actor->m_PxActor->is<physx::PxRigidBody>(), FALSE);
}

void CPropertiesWnd::UpdateProperties(CMFCPropertyGridProperty * pComponent, int i, Component * cmp)
{
	//pComponent->SetName(GetComponentTypeName(cmp->m_Type), FALSE);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp);
	pComponent->GetSubItem(0)->SetValue((_variant_t)ms2ts(cmp->GetName()).c_str());
	pComponent->GetSubItem(1)->SetValue((_variant_t)GetLodMaskDesc(cmp->m_LodMask));
	UpdatePropertiesShape(pComponent->GetSubItem(2), cmp);

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
		UpdatePropertiesStaticEmitter(pComponent, dynamic_cast<StaticEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		UpdatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		UpdatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	case Component::ComponentTypeAnimator:
		UpdatePropertiesAnimator(pComponent, dynamic_cast<Animator *>(cmp));
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesShape(CMFCPropertyGridProperty * pShape, Component * cmp)
{
	pShape->GetSubItem(0)->SetValue((_variant_t)g_ShapeTypeDesc[cmp->m_PxShape ? cmp->m_PxShape->getGeometryType() : physx::PxGeometryType::eGEOMETRY_COUNT]);
	physx::PxTransform localPose;
	physx::PxFilterData simulFilterData;
	physx::PxFilterData queryFilterData;
	physx::PxShapeFlags shapeFlags;
	if (cmp->m_PxShape)
	{
		localPose = cmp->m_PxShape->getLocalPose();
		simulFilterData = cmp->m_PxShape->getSimulationFilterData();
		queryFilterData = cmp->m_PxShape->getQueryFilterData();
		shapeFlags = cmp->m_PxShape->getFlags();
	}
	pShape->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)localPose.p.x);
	pShape->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)localPose.p.y);
	pShape->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)localPose.p.z);
	my::Vector3 angle = ((my::Quaternion &)localPose.q).toEulerAngles();
	pShape->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pShape->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pShape->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pShape->GetSubItem(3)->SetValue((_variant_t)simulFilterData.word0);
	pShape->GetSubItem(4)->SetValue((_variant_t)queryFilterData.word0);
	pShape->GetSubItem(5)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE));
	pShape->GetSubItem(6)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE));
	pShape->GetSubItem(7)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE));
	pShape->GetSubItem(8)->SetValue((_variant_t)(VARIANT_BOOL)shapeFlags.isSet(physx::PxShapeFlag::eVISUALIZATION));
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
	if (!pProp || pProp->GetData() != PropertyMeshPath)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesMesh(pComponent, mesh_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(theApp.GetFullPath(mesh_cmp->m_MeshPath.c_str()).c_str()).c_str());
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)ms2ts(mesh_cmp->m_MeshSubMeshName.c_str()).c_str());
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)mesh_cmp->m_MeshSubMeshId);
	COLORREF color = RGB(mesh_cmp->m_MeshColor.x * 255, mesh_cmp->m_MeshColor.y * 255, mesh_cmp->m_MeshColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pComponent->GetSubItem(PropId + 3)))->SetColor(color);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)(long)(mesh_cmp->m_MeshColor.w * 255));
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)(VARIANT_BOOL)mesh_cmp->m_bInstance);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 6), mesh_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesMaterial(CMFCPropertyGridProperty * pMaterial, Material * mtl)
{
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mtl);
	pMaterial->GetSubItem(0)->SetValue((_variant_t)ms2ts(theApp.GetFullPath(mtl->m_Shader.c_str()).c_str()).c_str());
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
	RemovePropertiesFrom(pParameterList, (int)mtl->m_ParameterList.size());
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
			ms2ts(theApp.GetFullPath(dynamic_cast<MaterialParameterTexture *>(mtl_param)->m_TexturePath.c_str()).c_str()).c_str());
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyClothColor)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesCloth(pComponent, cloth_cmp);
	}
	COLORREF color = RGB(cloth_cmp->m_MeshColor.x * 255, cloth_cmp->m_MeshColor.y * 255, cloth_cmp->m_MeshColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pComponent->GetSubItem(PropId + 0)))->SetColor(color);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)(long)(cloth_cmp->m_MeshColor.w * 255));
	physx::PxClothFlags flags = cloth_cmp->m_Cloth->getClothFlags();
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)(VARIANT_BOOL)flags.isSet(physx::PxClothFlag::eSCENE_COLLISION));
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 3), cloth_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pChunkStep = pComponent->GetSubItem(PropId + 4);
	if (!pChunkStep || pChunkStep->GetData() != PropertyStaticEmitterChunkStep)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesStaticEmitter(pComponent, emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)g_EmitterFaceType[emit_cmp->m_EmitterFaceType]);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)g_EmitterSpaceType[emit_cmp->m_EmitterSpaceType]);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)g_EmitterVelType[emit_cmp->m_EmitterVelType]);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)g_EmitterPrimitiveType[emit_cmp->m_EmitterPrimitiveType]);
	pChunkStep->SetValue((_variant_t)emit_cmp->m_ChunkStep);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 5), emit_cmp->m_Material.get());
	CMFCPropertyGridProperty * pParticleList = pComponent->GetSubItem(PropId + 6);
	//pParticleList->GetSubItem(0)->SetValue((_variant_t)(unsigned int)emit_cmp->m_ParticleList.size());
	//int NumParticles = my::Min(theApp.max_editable_particle_count, (int)emit_cmp->m_ParticleList.size());
	//for (int i = 0; i < NumParticles; i++)
	//{
	//	if (pParticleList->GetSubItemsCount() <= i + 1)
	//	{
	//		CreatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
	//		continue;
	//	}
	//	UpdatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
	//}
	//RemovePropertiesFrom(pParticleList, 1 + NumParticles);
}

void CPropertiesWnd::UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, StaticEmitterComponent* emit_cmp)
{
	//CMFCPropertyGridProperty * pParticle = pParentProp->GetSubItem(NodeId + 1);
	//_ASSERT(pParticle);
	//my::Emitter::Particle & particle = emit_cmp->m_ParticleList[NodeId];
	//CMFCPropertyGridProperty * pProp = pParticle->GetSubItem(0)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionX); pProp->SetValue((_variant_t)particle.m_Position.x);
	//pProp = pParticle->GetSubItem(0)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionY); pProp->SetValue((_variant_t)particle.m_Position.y);
	//pProp = pParticle->GetSubItem(0)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionZ); pProp->SetValue((_variant_t)particle.m_Position.z);
	//pProp = pParticle->GetSubItem(1)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityX); pProp->SetValue((_variant_t)particle.m_Velocity.x);
	//pProp = pParticle->GetSubItem(1)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityY); pProp->SetValue((_variant_t)particle.m_Velocity.y);
	//pProp = pParticle->GetSubItem(1)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityZ); pProp->SetValue((_variant_t)particle.m_Velocity.z);
	//COLORREF color = RGB(particle.m_Color.x * 255, particle.m_Color.y * 255, particle.m_Color.z * 255);
	//pProp = pParticle->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleColor); (DYNAMIC_DOWNCAST(CColorProp, pProp))->SetColor(color);
	//pProp = pParticle->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorAlpha); pProp->SetValue((_variant_t)(long)(particle.m_Color.w * 255));
	//pProp = pParticle->GetSubItem(4)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeX); pProp->SetValue((_variant_t)particle.m_Size.x);
	//pProp = pParticle->GetSubItem(4)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeY); pProp->SetValue((_variant_t)particle.m_Size.y);
	//pProp = pParticle->GetSubItem(5); _ASSERT(pProp->GetData() == PropertyEmitterParticleAngle); pProp->SetValue((_variant_t)D3DXToDegree(particle.m_Angle));
}

void CPropertiesWnd::UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pParticleCapacity = pComponent->GetSubItem(PropId + 4);
	if (!pParticleCapacity || pParticleCapacity->GetData() != PropertySphericalEmitterParticleCapacity)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesSphericalEmitter(pComponent, sphe_emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)g_EmitterFaceType[sphe_emit_cmp->m_EmitterFaceType]);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)g_EmitterSpaceType[sphe_emit_cmp->m_EmitterSpaceType]);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)g_EmitterVelType[sphe_emit_cmp->m_EmitterVelType]);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)g_EmitterPrimitiveType[sphe_emit_cmp->m_EmitterPrimitiveType]);
	pParticleCapacity->SetValue((_variant_t)(unsigned int)sphe_emit_cmp->m_ParticleList.capacity());
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleLifeTime);
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnInterval);
	pComponent->GetSubItem(PropId + 7)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.x);
	pComponent->GetSubItem(PropId + 7)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.y);
	pComponent->GetSubItem(PropId + 7)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.z);
	pComponent->GetSubItem(PropId + 8)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnSpeed);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 9), &sphe_emit_cmp->m_SpawnInclination);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 10), &sphe_emit_cmp->m_SpawnAzimuth);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 11), &sphe_emit_cmp->m_SpawnColorR);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 12), &sphe_emit_cmp->m_SpawnColorG);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 13), &sphe_emit_cmp->m_SpawnColorB);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 14), &sphe_emit_cmp->m_SpawnColorA);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 15), &sphe_emit_cmp->m_SpawnSizeX);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 16), &sphe_emit_cmp->m_SpawnSizeY);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 17), &sphe_emit_cmp->m_SpawnAngle);
	pComponent->GetSubItem(PropId + 18)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnCycle);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 19), sphe_emit_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesSpline(CMFCPropertyGridProperty * pSpline, my::Spline * spline)
{
	pSpline->SetValue((_variant_t)(DWORD_PTR)spline);
	pSpline->GetSubItem(0)->SetValue((_variant_t)(unsigned int)spline->size());
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
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)terrain->m_RowChunks);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)terrain->m_ColChunks);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)terrain->m_ChunkSize);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)ms2ts(terrain->m_ChunkPath.c_str()).c_str());
	pComponent->GetSubItem(PropId + 4);
	pComponent->GetSubItem(PropId + 5);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 6), terrain->m_Material.get());

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	MaterialPtr mtl = terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material;
	pComponent->GetSubItem(PropId + 7)->SetValue((_variant_t)(VARIANT_BOOL)(mtl != NULL));

	CString strTitle;
	strTitle.Format(_T("Chunk_%d_%d Material"), pFrame->m_selchunkid.x, pFrame->m_selchunkid.y);
	if (mtl)
	{
		if (pComponent->GetSubItem(PropId + 8)->GetSubItemsCount() <= 0)
		{
			RemovePropertiesFrom(pComponent, PropId + 8);
			CreatePropertiesMaterial(pComponent, strTitle, mtl.get());
		}
		else
		{
			pComponent->GetSubItem(PropId + 8)->SetName(strTitle, FALSE);
			UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 8), mtl.get());
		}
	}
	else
	{
		if (pComponent->GetSubItem(PropId + 8)->GetSubItemsCount() <= 0)
		{
			pComponent->GetSubItem(PropId + 8)->SetName(strTitle, FALSE);
		}
		else
		{
			RemovePropertiesFrom(pComponent, PropId + 8);
			CMFCPropertyGridProperty* pMaterial = new CSimpleProp(strTitle, PropertyMaterial, FALSE);
			pComponent->AddSubItem(pMaterial);
		}
	}
}

void CPropertiesWnd::UpdatePropertiesAnimator(CMFCPropertyGridProperty* pComponent, Animator* animator)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty* pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyAnimatorSkeletonPath)
	{
		RemovePropertiesFrom(pComponent, PropId);
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(theApp.GetFullPath(animator->m_SkeletonPath.c_str()).c_str()).c_str());

	UpdatePropertiesAnimationNode(pComponent->GetSubItem(PropId + 1), animator->m_Childs[0].get());
}

void CPropertiesWnd::UpdatePropertiesAnimationNode(CMFCPropertyGridProperty* pAnimationNode, AnimationNode* node)
{
	pAnimationNode->SetValue((_variant_t)(DWORD_PTR)node);

	if (AnimationNodeSequence* seq = dynamic_cast<AnimationNodeSequence*>(node))
	{
		pAnimationNode->GetSubItem(0)->SetValue((_variant_t)g_AnimationNodeType[0]);

		if (pAnimationNode->GetSubItemsCount() <= 1 || pAnimationNode->GetSubItem(1)->GetData() != PropertyAnimationNodeSequenceName)
		{
			RemovePropertiesFrom(pAnimationNode, 1);
			CreatePropertiesAnimationNodeSequence(pAnimationNode, seq);
		}

		UpdatePropertiesAnimationNodeSequence(pAnimationNode, seq);
	}
}

void CPropertiesWnd::UpdatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty* pAnimationNode, AnimationNodeSequence* seq)
{
	pAnimationNode->GetSubItem(1)->SetValue((_variant_t)ms2ts(seq->m_Name.c_str()).c_str());

	pAnimationNode->GetSubItem(1)->RemoveAllOptions();
	Animator* animator = dynamic_cast<Animator*>(seq->GetTopNode());
	if (animator->m_Skeleton)
	{
		my::OgreSkeletonAnimation::OgreAnimationMap::const_iterator anim_iter = animator->m_Skeleton->m_animationMap.begin();
		for (; anim_iter != animator->m_Skeleton->m_animationMap.end(); anim_iter++)
		{
			pAnimationNode->GetSubItem(1)->AddOption(ms2ts(anim_iter->first.c_str()).c_str(), TRUE);
		}
	}
}

void CPropertiesWnd::CreatePropertiesActor(Actor * actor)
{
	CMFCPropertyGridProperty * pActor = new CSimpleProp(GetComponentTypeName(Component::ComponentTypeActor), PropertyActor, FALSE);
	m_wndPropList.AddProperty(pActor, FALSE, FALSE);
	pActor->SetValue((_variant_t)(DWORD_PTR)actor); // ! only worked on 32bit system

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMFCPropertyGridProperty * pName = new CSimpleProp(_T("Name"), (_variant_t)ms2ts(actor->GetName()).c_str(), NULL, PropertyActorName);
	pActor->AddSubItem(pName);
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

	my::Vector3 angle = actor->m_Rotation.toEulerAngles();
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

	CMFCPropertyGridProperty * pLodDist = new CSimpleProp(_T("LodDist"), (_variant_t)actor->m_LodDist, NULL, PropertyActorLodDist);
	pActor->AddSubItem(pLodDist);
	CMFCPropertyGridProperty * pLodFactor = new CSimpleProp(_T("LodFactor"), (_variant_t)actor->m_LodFactor, NULL, PropertyActorLodFactor);
	pActor->AddSubItem(pLodFactor);
	CMFCPropertyGridProperty* pCullingDist = new CSimpleProp(_T("CullingDist"), (_variant_t)actor->m_CullingDist, NULL, PropertyActorCullingDist);
	pActor->AddSubItem(pCullingDist);

	CreatePropertiesRigidActor(pActor, actor);

	if (!actor->m_Cmps.empty())
	{
		Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
		for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
		{
			CreateProperties(pActor, cmp_iter->get());
		}
	}
}

void CPropertiesWnd::CreatePropertiesRigidActor(CMFCPropertyGridProperty * pParentCtrl, Actor * actor)
{
	CMFCPropertyGridProperty * pRigidActor = new CSimpleProp(_T("RigidActor"), PropertyActorRigidActor, FALSE);
	pParentCtrl->AddSubItem(pRigidActor);
	CMFCPropertyGridProperty * pProp = new CComboProp(_T("RigidActor"), g_ActorTypeDesc[actor->m_PxActor ? actor->m_PxActor->getType() : physx::PxActorType::eACTOR_COUNT], NULL, PropertyActorRigidActorType);
	for (unsigned int i = 0; i < _countof(g_ActorTypeDesc); i++)
	{
		pProp->AddOption(g_ActorTypeDesc[i], TRUE);
	}
	pRigidActor->AddSubItem(pProp);
	physx::PxRigidBodyFlags bodyFlags;
	if (actor->m_PxActor)
	{
		physx::PxRigidBody * body = actor->m_PxActor->is<physx::PxRigidBody>();
		if (body)
		{
			bodyFlags = body->getRigidBodyFlags();
		}
	}
	const TCHAR * szDesc = _T("Enables kinematic mode for the actor.\n\nKinematic actors are special dynamic actors that are not influenced by forces(such as gravity), and have no momentum. They are considered to have infinite mass and can be moved around the world using the setKinematicTarget() method. They will push regular dynamic actors out of the way. Kinematics will not collide with static or other kinematic objects.\n\nKinematic actors are great for moving platforms or characters, where direct motion control is desired.\n\nYou can not connect Reduced joints to kinematic actors. Lagrange joints work ok if the platform is moving with a relatively low, uniform velocity.");
	pProp = new CCheckBoxProp(_T("eKINEMATIC"), bodyFlags.isSet(physx::PxRigidBodyFlag::eKINEMATIC), szDesc, PropertyActorRigidActorKinematic);
	pRigidActor->AddSubItem(pProp);
	pRigidActor->GetSubItem(1)->Show(actor->m_PxActor && actor->m_PxActor->is<physx::PxRigidBody>(), FALSE);
}

void CPropertiesWnd::CreateProperties(CMFCPropertyGridProperty * pParentCtrl, Component * cmp)
{
	CMFCPropertyGridProperty * pComponent = new CSimpleProp(GetComponentTypeName(cmp->m_Type), GetComponentProp(cmp->m_Type), FALSE);
	pParentCtrl->AddSubItem(pComponent);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp); // ! only worked on 32bit system

	CMFCPropertyGridProperty * pName = new CSimpleProp(_T("Name"), (_variant_t)ms2ts(cmp->GetName()).c_str(), NULL, PropertyComponentName);
	pComponent->AddSubItem(pName);

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
		CreatePropertiesStaticEmitter(pComponent, dynamic_cast<StaticEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		CreatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		CreatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	case Component::ComponentTypeAnimator:
		CreatePropertiesAnimator(pComponent, dynamic_cast<Animator *>(cmp));
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
	physx::PxFilterData simulFilterData;
	physx::PxFilterData queryFilterData;
	physx::PxShapeFlags shapeFlags;
	if (cmp->m_PxShape)
	{
		localPose = cmp->m_PxShape->getLocalPose();
		simulFilterData = cmp->m_PxShape->getSimulationFilterData();
		queryFilterData = cmp->m_PxShape->getQueryFilterData();
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

	my::Vector3 angle = ((my::Quaternion &)localPose.q).toEulerAngles();
	CMFCPropertyGridProperty * pLocalRot = new CSimpleProp(_T("LocalRot"), PropertyShapeLocalRot, TRUE);
	pShape->AddSubItem(pLocalRot);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertyShapeLocalRotX);
	pLocalRot->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertyShapeLocalRotY);
	pLocalRot->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertyShapeLocalRotZ);
	pLocalRot->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("SimulationFilterData"), (_variant_t)simulFilterData.word0, NULL, PropertyShapeSimulationFilterData);
	pShape->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("QueryFilterData"), (_variant_t)queryFilterData.word0, NULL, PropertyShapeQueryFilterData);
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
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CMFCPropertyGridProperty * pProp = new CFileProp(_T("MeshPath"), TRUE, (_variant_t)ms2ts(theApp.GetFullPath(mesh_cmp->m_MeshPath.c_str()).c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("MeshSubMeshName"), (_variant_t)ms2ts(mesh_cmp->m_MeshSubMeshName.c_str()).c_str(), NULL, PropertyMeshSubMeshName);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("MeshSubMeshId"), (_variant_t)mesh_cmp->m_MeshSubMeshId, NULL, PropertyMeshSubMeshId);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	COLORREF color = RGB(mesh_cmp->m_MeshColor.x * 255, mesh_cmp->m_MeshColor.y * 255, mesh_cmp->m_MeshColor.z * 255);
	CColorProp* pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyMeshColor);
	pColor->EnableOtherButton(_T("Other..."));
	pComponent->AddSubItem(pColor);

	CMFCPropertyGridProperty* pAlpha = new CSliderProp(_T("Alpha"), (long)(mesh_cmp->m_MeshColor.w * 255), NULL, PropertyMeshAlpha);
	pComponent->AddSubItem(pAlpha);

	pProp = new CCheckBoxProp(_T("Instance"), (_variant_t)mesh_cmp->m_bInstance, NULL, PropertyMeshInstance);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, _T("Material"), mesh_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, LPCTSTR lpszName, Material * mtl)
{
	CMFCPropertyGridProperty * pMaterial = new CSimpleProp(lpszName, PropertyMaterial, FALSE);
	pParentCtrl->AddSubItem(pMaterial);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mtl);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("Shader"), TRUE, (_variant_t)ms2ts(theApp.GetFullPath(mtl->m_Shader.c_str()).c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialShader);
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
	std::basic_string<TCHAR> name = ms2ts(mtl_param->m_Name.c_str());
	boost::trim_left_if(name, boost::is_any_of(_T("g_")));
	switch (mtl_param->m_Type)
	{
	case MaterialParameter::ParameterTypeFloat:
		pProp = new CSimpleProp(name.c_str(), (_variant_t)dynamic_cast<MaterialParameterFloat *>(mtl_param)->m_Value, NULL, PropertyMaterialParameterFloat);
		pParentCtrl->AddSubItem(pProp);
		break;
	case MaterialParameter::ParameterTypeFloat2:
	{
		const my::Vector2 & Value = dynamic_cast<MaterialParameterFloat2 *>(mtl_param)->m_Value;
		CMFCPropertyGridProperty * pParameter = new CSimpleProp(name.c_str(), PropertyMaterialParameterFloat2, TRUE);
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
		CMFCPropertyGridProperty * pParameter = new CSimpleProp(name.c_str(), PropertyMaterialParameterFloat3, TRUE);
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
		CMFCPropertyGridProperty * pParameter = new CSimpleProp(name.c_str(), PropertyMaterialParameterFloat4, TRUE);
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
		pProp = new CFileProp(name.c_str(), TRUE, (_variant_t)ms2ts(theApp.GetFullPath(dynamic_cast<MaterialParameterTexture *>(mtl_param)->m_TexturePath.c_str()).c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialParameterTexture);
		pParentCtrl->AddSubItem(pProp);
		break;
	}
}

void CPropertiesWnd::CreatePropertiesCloth(CMFCPropertyGridProperty * pComponent, ClothComponent * cloth_cmp)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	COLORREF color = RGB(cloth_cmp->m_MeshColor.x * 255, cloth_cmp->m_MeshColor.y * 255, cloth_cmp->m_MeshColor.z * 255);
	CColorProp* pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyClothColor);
	pColor->EnableOtherButton(_T("Other..."));
	pComponent->AddSubItem(pColor);

	CMFCPropertyGridProperty* pAlpha = new CSliderProp(_T("Alpha"), (long)(cloth_cmp->m_MeshColor.w * 255), NULL, PropertyClothAlpha);
	pComponent->AddSubItem(pAlpha);

	physx::PxClothFlags flags = cloth_cmp->m_Cloth->getClothFlags();
	CMFCPropertyGridProperty * pProp = new CCheckBoxProp(_T("SceneCollision"), (_variant_t)flags.isSet(physx::PxClothFlag::eSCENE_COLLISION), NULL, PropertyClothSceneCollision);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, _T("Material"), cloth_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitterComponent * emit_cmp)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CComboProp * pEmitterFaceType = new CComboProp(_T("FaceType"), (_variant_t)g_EmitterFaceType[emit_cmp->m_EmitterFaceType], NULL, PropertyEmitterFaceType);
	for (unsigned int i = 0; i < _countof(g_EmitterFaceType); i++)
	{
		pEmitterFaceType->AddOption(g_EmitterFaceType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterFaceType);
	CComboProp * pEmitterSpaceType = new CComboProp(_T("SpaceType"), (_variant_t)g_EmitterSpaceType[emit_cmp->m_EmitterSpaceType], NULL, PropertyEmitterSpaceType);
	for (unsigned int i = 0; i < _countof(g_EmitterSpaceType); i++)
	{
		pEmitterSpaceType->AddOption(g_EmitterSpaceType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterSpaceType);
	CComboProp * pEmitterVelType = new CComboProp(_T("VelocityType"), (_variant_t)g_EmitterVelType[emit_cmp->m_EmitterVelType], NULL, PropertyEmitterVelType);
	for (unsigned int i = 0; i < _countof(g_EmitterVelType); i++)
	{
		pEmitterVelType->AddOption(g_EmitterVelType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterVelType);
	CComboProp * pEmitterPrimitiveType = new CComboProp(_T("PrimitiveType"), (_variant_t)g_EmitterPrimitiveType[emit_cmp->m_EmitterPrimitiveType], NULL, PropertyEmitterPrimitiveType);
	for (unsigned int i = 0; i < _countof(g_EmitterPrimitiveType); i++)
	{
		pEmitterPrimitiveType->AddOption(g_EmitterPrimitiveType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterPrimitiveType);
	CMFCPropertyGridProperty * pChunkStep = new CSimpleProp(_T("ChunkStep"), (_variant_t)emit_cmp->m_ChunkStep, NULL, PropertyStaticEmitterChunkStep);
	pComponent->AddSubItem(pChunkStep);
	CreatePropertiesMaterial(pComponent, _T("Material"), emit_cmp->m_Material.get());
	CMFCPropertyGridProperty * pParticleList = new CSimpleProp(_T("ParticleList"), PropertyEmitterParticleList, FALSE);
	pComponent->AddSubItem(pParticleList);
	//CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("ParticleCount"), (_variant_t)(unsigned int)emit_cmp->m_ParticleList.size(), NULL, PropertyEmitterParticleCount);
	//pParticleList->AddSubItem(pProp);
	//int NumParticles = my::Min(theApp.max_editable_particle_count, (int)emit_cmp->m_ParticleList.size());
	//for (int i = 0; i < NumParticles; i++)
	//{
	//	CreatePropertiesStaticEmitterParticle(pParticleList, i, emit_cmp);
	//}
}

void CPropertiesWnd::CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, int NodeId, StaticEmitterComponent* emit_cmp)
{
	//TCHAR buff[128];
	//_stprintf_s(buff, _countof(buff), _T("Particle%d"), NodeId);
	//CMFCPropertyGridProperty * pParticle = new CSimpleProp(buff, NodeId, FALSE);
	//pParentProp->AddSubItem(pParticle);
	//CMFCPropertyGridProperty * pPosition = new CMFCPropertyGridProperty(_T("Position"), PropertyEmitterParticlePosition, TRUE);
	//pParticle->AddSubItem(pPosition);
	//my::Emitter::Particle & particle = emit_cmp->m_ParticleList[NodeId];
	//CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)particle.m_Position.x, NULL, PropertyEmitterParticlePositionX);
	//pPosition->AddSubItem(pProp);
	//pProp = new CSimpleProp(_T("y"), (_variant_t)particle.m_Position.y, NULL, PropertyEmitterParticlePositionY);
	//pPosition->AddSubItem(pProp);
	//pProp = new CSimpleProp(_T("z"), (_variant_t)particle.m_Position.z, NULL, PropertyEmitterParticlePositionZ);
	//pPosition->AddSubItem(pProp);

	//CMFCPropertyGridProperty * pVelocity = new CMFCPropertyGridProperty(_T("Velocity"), PropertyEmitterParticleVelocity, TRUE);
	//pParticle->AddSubItem(pVelocity);
	//pProp = new CSimpleProp(_T("x"), (_variant_t)particle.m_Velocity.x, NULL, PropertyEmitterParticleVelocityX);
	//pVelocity->AddSubItem(pProp);
	//pProp = new CSimpleProp(_T("y"), (_variant_t)particle.m_Velocity.y, NULL, PropertyEmitterParticleVelocityY);
	//pVelocity->AddSubItem(pProp);
	//pProp = new CSimpleProp(_T("z"), (_variant_t)particle.m_Velocity.z, NULL, PropertyEmitterParticleVelocityZ);
	//pVelocity->AddSubItem(pProp);

	//COLORREF color = RGB(particle.m_Color.x * 255, particle.m_Color.y * 255, particle.m_Color.z * 255);
	//CColorProp * pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyEmitterParticleColor);
	//pColor->EnableOtherButton(_T("Other..."));
	//pParticle->AddSubItem(pColor);

	//CMFCPropertyGridProperty * pAlpha = new CSliderProp(_T("Alpha"), (long)(particle.m_Color.w * 255), NULL, PropertyEmitterParticleColorAlpha);
	//pParticle->AddSubItem(pAlpha);

	//CMFCPropertyGridProperty * pSize = new CMFCPropertyGridProperty(_T("Size"), PropertyEmitterParticleSize, TRUE);
	//pParticle->AddSubItem(pSize);
	//pProp = new CSimpleProp(_T("x"), (_variant_t)particle.m_Size.x, NULL, PropertyEmitterParticleSizeX);
	//pSize->AddSubItem(pProp);
	//pProp = new CSimpleProp(_T("y"), (_variant_t)particle.m_Size.y, NULL, PropertyEmitterParticleSizeY);
	//pSize->AddSubItem(pProp);

	//pProp = new CMFCPropertyGridProperty(_T("Angle"), (_variant_t)D3DXToDegree(particle.m_Angle), NULL, PropertyEmitterParticleAngle);
	//pParticle->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CComboProp * pEmitterFaceType = new CComboProp(_T("FaceType"), (_variant_t)g_EmitterFaceType[sphe_emit_cmp->m_EmitterFaceType], NULL, PropertyEmitterFaceType);
	for (unsigned int i = 0; i < _countof(g_EmitterFaceType); i++)
	{
		pEmitterFaceType->AddOption(g_EmitterFaceType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterFaceType);
	CComboProp * pEmitterSpaceType = new CComboProp(_T("SpaceType"), (_variant_t)g_EmitterSpaceType[sphe_emit_cmp->m_EmitterSpaceType], NULL, PropertyEmitterSpaceType);
	for (unsigned int i = 0; i < _countof(g_EmitterSpaceType); i++)
	{
		pEmitterSpaceType->AddOption(g_EmitterSpaceType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterSpaceType);
	CComboProp* pEmitterVelType = new CComboProp(_T("VelocityType"), (_variant_t)g_EmitterVelType[sphe_emit_cmp->m_EmitterVelType], NULL, PropertyEmitterVelType);
	for (unsigned int i = 0; i < _countof(g_EmitterVelType); i++)
	{
		pEmitterVelType->AddOption(g_EmitterVelType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterVelType);
	CComboProp* pEmitterPrimitiveType = new CComboProp(_T("PrimitiveType"), (_variant_t)g_EmitterPrimitiveType[sphe_emit_cmp->m_EmitterPrimitiveType], NULL, PropertyEmitterPrimitiveType);
	for (unsigned int i = 0; i < _countof(g_EmitterPrimitiveType); i++)
	{
		pEmitterPrimitiveType->AddOption(g_EmitterPrimitiveType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterPrimitiveType);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("ParticleCapacity"), (_variant_t)(unsigned int)sphe_emit_cmp->m_ParticleList.capacity(), NULL, PropertySphericalEmitterParticleCapacity);
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
	pProp = new CSimpleProp(_T("SpawnCycle"), (_variant_t)sphe_emit_cmp->m_SpawnCycle, NULL, PropertySphericalEmitterSpawnCycle);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, _T("Material"), sphe_emit_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesSpline(CMFCPropertyGridProperty * pParentProp, LPCTSTR lpszName, Property PropertyId, my::Spline * spline)
{
	CMFCPropertyGridProperty * pSpline = new CSimpleProp(lpszName, PropertyId, TRUE);
	pParentProp->AddSubItem(pSpline);
	pSpline->SetValue((_variant_t)(DWORD_PTR)spline);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)(unsigned int)spline->size(), NULL, PropertySplineNodeCount);
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
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("RowChunks"), (_variant_t)terrain->m_RowChunks, NULL, PropertyTerrainRowChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ColChunks"), (_variant_t)terrain->m_ColChunks, NULL, PropertyTerrainColChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ChunkSize"), (_variant_t)terrain->m_ChunkSize, NULL, PropertyTerrainChunkSize);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ChunkPath"), (_variant_t)ms2ts(terrain->m_ChunkPath.c_str()).c_str(), NULL, PropertyTerrainChunkPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("HeightMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainHeightMap);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("SplatMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainSplatMap);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, _T("Material"), terrain->m_Material.get());

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CString strTitle;
	strTitle.Format(_T("Enable Chunk_%d_%d Material"), pFrame->m_selchunkid.x, pFrame->m_selchunkid.y);
	MaterialPtr mtl = terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material;
	pProp = new CCheckBoxProp(strTitle, mtl != NULL, NULL, PropertyTerrainChunkMaterial);
	pComponent->AddSubItem(pProp);

	strTitle.Format(_T("Chunk_%d_%d Material"), pFrame->m_selchunkid.x, pFrame->m_selchunkid.y);
	if (mtl)
	{
		CreatePropertiesMaterial(pComponent, strTitle, mtl.get());
	}
	else
	{
		CMFCPropertyGridProperty* pMaterial = new CSimpleProp(strTitle, PropertyMaterial, FALSE);
		pComponent->AddSubItem(pMaterial);
	}
}

void CPropertiesWnd::CreatePropertiesAnimator(CMFCPropertyGridProperty* pComponent, Animator* animator)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CMFCPropertyGridProperty * pProp = new CFileProp(_T("SkeletonPath"), TRUE, (_variant_t)ms2ts(theApp.GetFullPath(animator->m_SkeletonPath.c_str()).c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyAnimatorSkeletonPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	CreatePropertiesAnimationNode(pComponent, animator->m_Childs[0].get());
}

void CPropertiesWnd::CreatePropertiesAnimationNode(CMFCPropertyGridProperty* pParentCtrl, AnimationNode* node)
{
	CMFCPropertyGridProperty* pAnimationNode = new CSimpleProp(_T("AnimationNode"), PropertyAnimationNode, FALSE);
	pAnimationNode->SetValue((_variant_t)(DWORD_PTR)node);
	pParentCtrl->AddSubItem(pAnimationNode);

	CMFCPropertyGridProperty* pProp = new CComboProp(_T("Type"), (_variant_t)_T("None"), NULL, PropertyAnimationNodeType);
	for (unsigned int i = 0; i < _countof(g_AnimationNodeType); i++)
	{
		pProp->AddOption(g_AnimationNodeType[i], TRUE);
	}
	pAnimationNode->AddSubItem(pProp);

	if (AnimationNodeSequence * seq = dynamic_cast<AnimationNodeSequence *>(node))
	{
		pProp->SetValue((_variant_t)g_AnimationNodeType[0]);

		CreatePropertiesAnimationNodeSequence(pAnimationNode, seq);
	}
}

void CPropertiesWnd::CreatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty* pAnimationNode, AnimationNodeSequence* seq)
{
	ASSERT(pAnimationNode->GetSubItemsCount() == 1);

	CMFCPropertyGridProperty* pProp = new CComboProp(_T("Name"), (_variant_t)ms2ts(seq->m_Name.c_str()).c_str(), NULL, PropertyAnimationNodeSequenceName);
	pAnimationNode->AddSubItem(pProp);

	Animator* animator = dynamic_cast<Animator*>(seq->GetTopNode());
	if (animator->m_Skeleton)
	{
		my::OgreSkeletonAnimation::OgreAnimationMap::const_iterator anim_iter = animator->m_Skeleton->m_animationMap.begin();
		for (; anim_iter != animator->m_Skeleton->m_animationMap.end(); anim_iter++)
		{
			pProp->AddOption(ms2ts(anim_iter->first.c_str()).c_str(), TRUE);
		}
	}
}

CPropertiesWnd::Property CPropertiesWnd::GetComponentProp(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return PropertyActor;
	case Component::ComponentTypeController:
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
	case Component::ComponentTypeAnimator:
		return PropertyAnimator;
	}
	return PropertyUnknown;
}

unsigned int CPropertiesWnd::GetComponentPropCount(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return 9;
	case Component::ComponentTypeController:
		return GetComponentPropCount(Component::ComponentTypeComponent);
	case Component::ComponentTypeMesh:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 7;
	case Component::ComponentTypeCloth:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 4;
	case Component::ComponentTypeStaticEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 7;
	case Component::ComponentTypeSphericalEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 20;
	case Component::ComponentTypeTerrain:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 9;
	case Component::ComponentTypeAnimator:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 2;
	}

	ASSERT(Component::ComponentTypeComponent == type);
	return 3;
}

LPCTSTR CPropertiesWnd::GetComponentTypeName(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeActor:
		return _T("Actor");
	case Component::ComponentTypeController:
		return _T("Controller");
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
	case Component::ComponentTypeScript:
		return _T("Script");
	case Component::ComponentTypeAnimator:
		return _T("Animator");
	}
	return _T("Unknown");
}

TerrainChunk * CPropertiesWnd::GetTerrainChunkSafe(Terrain * terrain, const CPoint & chunkid)
{
	if (chunkid.x >= 0 && chunkid.x < (int)terrain->m_Chunks.shape()[0]
		&& chunkid.y >= 0 && chunkid.y < (int)terrain->m_Chunks.shape()[1])
	{
		return terrain->GetChunk(chunkid.x, chunkid.y);
	}
	return terrain->GetChunk(0, 0);
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

void CPropertiesWnd::UpdatePropertiesPaintTool(void)
{
	CMFCPropertyGridProperty* pPaint = NULL;
	if (m_wndPropList.GetPropertyCount() >= 1)
	{
		pPaint = m_wndPropList.GetProperty(0);
	}
	if (!pPaint || pPaint->GetData() != PropertyPaint)
	{
		m_wndPropList.RemoveAll();
		CreatePropertiesPaintTool();
		return;
	}

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pPaint->SetName(g_PaintType[pFrame->m_PaintType]);
	pPaint->GetSubItem(0)->SetValue((_variant_t)g_PaintShape[pFrame->m_PaintShape]);
	pPaint->GetSubItem(1)->SetValue((_variant_t)g_PaintMode[pFrame->m_PaintMode]);
	pPaint->GetSubItem(2)->SetValue((_variant_t)pFrame->m_PaintRadius);
	pPaint->GetSubItem(3)->SetValue((_variant_t)pFrame->m_PaintHeight);
	COLORREF color = RGB(pFrame->m_PaintColor.r * 255, pFrame->m_PaintColor.g * 255, pFrame->m_PaintColor.b * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pPaint->GetSubItem(4)))->SetColor(color);
	UpdatePropertiesSpline(pPaint->GetSubItem(5), &pFrame->m_PaintSpline);
	pPaint->GetSubItem(6)->SetValue((_variant_t)pFrame->m_PaintDensity);
}

void CPropertiesWnd::CreatePropertiesPaintTool(void)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMFCPropertyGridProperty* pPaint = new CSimpleProp(g_PaintType[pFrame->m_PaintType], PropertyPaint, FALSE);
	m_wndPropList.AddProperty(pPaint, FALSE, FALSE);

	CMFCPropertyGridProperty* pProp = new CComboProp(_T("PaintShape"), g_PaintShape[pFrame->m_PaintShape], NULL, PropertyPaintShape);
	for (unsigned int i = 0; i < _countof(g_PaintShape); i++)
	{
		pProp->AddOption(g_PaintShape[i], TRUE);
	}
	pPaint->AddSubItem(pProp);

	pProp = new CComboProp(_T("PaintMode"), g_PaintMode[pFrame->m_PaintMode], NULL, PropertyPaintMode);
	for (unsigned int i = 0; i < _countof(g_PaintMode); i++)
	{
		pProp->AddOption(g_PaintMode[i], TRUE);
	}
	pPaint->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("PaintRadius"), pFrame->m_PaintRadius, NULL, PropertyPaintRadius);
	pPaint->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("PaintHeight"), pFrame->m_PaintHeight, NULL, PropertyPaintHeight);
	pPaint->AddSubItem(pProp);

	COLORREF color = RGB(pFrame->m_PaintColor.r * 255, pFrame->m_PaintColor.g * 255, pFrame->m_PaintColor.b * 255);
	CColorProp* pPaintColor = new CColorProp(_T("PaintColor"), color, NULL, NULL, PropertyPaintColor);
	pPaintColor->EnableOtherButton(_T("Other..."));
	pPaint->AddSubItem(pPaintColor);

	CreatePropertiesSpline(pPaint, _T("PaintSpline"), PropertyPaintSpline, &pFrame->m_PaintSpline);
	pProp = new CSimpleProp(_T("PaintDensity"), (_variant_t)pFrame->m_PaintDensity, NULL, PropertyPaintDensity);
	pPaint->AddSubItem(pProp);
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
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	//// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, _1));
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
	m_IsOnPropertyChanged = TRUE;
	BOOST_SCOPE_EXIT(&m_IsOnPropertyChanged)
	{
		m_IsOnPropertyChanged = FALSE;
	}
	BOOST_SCOPE_EXIT_END

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	//if (pFrame->m_selactors.empty())
	//{
	//	return 0;
	//}
	//Component * cmp = *pFrame->m_selactors.begin();

	CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
	ASSERT(pProp);
	DWORD PropertyId = (DWORD)pProp->GetData();
	switch (PropertyId)
	{
	case PropertyActorName:
	{
		Actor * actor = (Actor *)pProp->GetParent()->GetValue().pulVal;
		std::string Name = ts2ms(pProp->GetValue().bstrVal);
		if (theApp.GetNamedObject(Name.c_str()))
		{
			MessageBox(str_printf(_T("%s already existed"), pProp->GetValue().bstrVal).c_str());
			pProp->SetValue((_variant_t)ms2ts(actor->GetName()).c_str());
			return 0;
		}
		actor->SetName(Name.c_str());
		break;
	}
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
		Actor * actor = (Actor *)pAABB->GetParent()->GetValue().pulVal;
		actor->m_aabb.m_min.x = pAABB->GetSubItem(0)->GetValue().fltVal;
		actor->m_aabb.m_min.y = pAABB->GetSubItem(1)->GetValue().fltVal;
		actor->m_aabb.m_min.z = pAABB->GetSubItem(2)->GetValue().fltVal;
		actor->m_aabb.m_max.x = pAABB->GetSubItem(3)->GetValue().fltVal;
		actor->m_aabb.m_max.y = pAABB->GetSubItem(4)->GetValue().fltVal;
		actor->m_aabb.m_max.z = pAABB->GetSubItem(5)->GetValue().fltVal;
		actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
		my::EventArg arg;
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
		Actor * actor = (Actor *)pActor->GetValue().pulVal;
		my::Vector3 pos(
			pActor->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal,
			pActor->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal,
			pActor->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal);
		actor->m_Position = pos;
		my::Quaternion rot = my::Quaternion::RotationEulerAngles(
			D3DXToRadian(pActor->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal),
			D3DXToRadian(pActor->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal),
			D3DXToRadian(pActor->GetSubItem(3)->GetSubItem(2)->GetValue().fltVal));
		actor->m_Rotation = rot;
		actor->m_Scale.x = pActor->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
		actor->m_Scale.y = pActor->GetSubItem(4)->GetSubItem(1)->GetValue().fltVal;
		actor->m_Scale.z = pActor->GetSubItem(4)->GetSubItem(2)->GetValue().fltVal;
		actor->UpdateWorld();
		actor->UpdateOctNode();
		actor->SetPxPoseOrbyPxThread(physx::PxTransform((physx::PxVec3&)actor->m_Position, (physx::PxQuat&)actor->m_Rotation));
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();

		//Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
		//for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
		//{
		//	if ((*cmp_iter)->m_Type == Component::ComponentTypeStaticEmitter
		//		&& dynamic_cast<StaticEmitterComponent*>(cmp_iter->get())->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld)
		//	{
		//		dynamic_cast<StaticEmitterComponent *>(cmp_iter->get())->BuildChunks();
		//	}
		//}

		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyActorLodDist:
	{
		Actor * actor = (Actor *)pProp->GetParent()->GetValue().pulVal;
		actor->m_LodDist = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyActorLodFactor:
	{
		Actor * actor = (Actor *)pProp->GetParent()->GetValue().pulVal;
		actor->m_LodFactor = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyActorCullingDist:
	{
		Actor* actor = (Actor*)pProp->GetParent()->GetValue().pulVal;
		actor->m_CullingDist = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyActorRigidActorType:
	{
		Actor * actor = (Actor *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_ActorTypeDesc));
		actor->ClearRigidActor();
		actor->CreateRigidActor((physx::PxActorType::Enum)i);
		if (actor->m_PxActor)
		{
			physx::PxRigidBody* body = actor->m_PxActor->is<physx::PxRigidBody>();
			if (body)
			{
				body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			}

			if (actor->IsRequested())
			{
				pFrame->m_PxScene->addActor(*actor->m_PxActor);
			}
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyActorRigidActorKinematic:
	{
		Actor * actor = (Actor *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(actor->m_PxActor && actor->m_PxActor->is<physx::PxRigidBody>());
		actor->m_PxActor->is<physx::PxRigidBody>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, pProp->GetValue().boolVal != 0);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComponentName:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetValue().pulVal;
		std::string Name = ts2ms(pProp->GetValue().bstrVal);
		if (theApp.GetNamedObject(Name.c_str()))
		{
			MessageBox(str_printf(_T("%s already existed"), pProp->GetValue().bstrVal).c_str());
			pProp->SetValue((_variant_t)ms2ts(cmp->GetName()).c_str());
			return 0;
		}
		cmp->SetName(Name.c_str());
	}
	case PropertyComponentLODMask:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_LodMaskDesc));
		cmp->m_LodMask = (Component::LODMask)g_LodMaskDesc[i].mask;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeType:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
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
				UpdatePropertiesShape(pProp->GetParent(), cmp);
				m_wndPropList.AdjustLayout();
				my::EventArg arg;
				pFrame->m_EventAttributeChanged(&arg);
			}
			break;
		default:
		{
			cmp->ClearShape();
			UpdatePropertiesShape(pProp->GetParent(), cmp);
			m_wndPropList.AdjustLayout();
			my::EventArg arg;
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
		Component * cmp = (Component *)pShape->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		physx::PxVec3 localPos(
			pShape->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal,
			pShape->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal,
			pShape->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal);
		physx::PxQuat localRot = (physx::PxQuat &)my::Quaternion::RotationEulerAngles(
			D3DXToRadian(pShape->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal),
			D3DXToRadian(pShape->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal),
			D3DXToRadian(pShape->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal));
		cmp->m_PxShape->setLocalPose(physx::PxTransform(localPos, localRot));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeSimulationFilterData:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		cmp->SetSimulationFilterWord0(pProp->GetValue().uintVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeQueryFilterData:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		cmp->SetQueryFilterWord0(pProp->GetValue().uintVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeSimulation:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, pProp->GetValue().boolVal != 0);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeSceneQuery:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, pProp->GetValue().boolVal != 0);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeTrigger:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, pProp->GetValue().boolVal != 0);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeVisualization:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		ASSERT(cmp->m_PxShape);
		cmp->m_PxShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, pProp->GetValue().boolVal != 0);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshPath:
	case PropertyMeshSubMeshName:
	case PropertyMeshSubMeshId:
	{
		//MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
		//std::string path = theApp.GetRelativePath(ts2ms(pProp->GetValue().bstrVal).c_str());
		//if (path.empty())
		//{
		//	MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
		//	UpdatePropertiesMesh(pProp->GetParent(), mesh_cmp);
		//	return 0;
		//}
		//mesh_cmp->ReleaseResource();
		//mesh_cmp->m_MeshPath = path;
		//mesh_cmp->RequestResource();
		//my::EventArg arg;
		//pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshColor:
	case PropertyMeshAlpha:
	{
		MeshComponent* mesh_cmp = dynamic_cast<MeshComponent*>((Component*)pProp->GetParent()->GetValue().pulVal);
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 3)))->GetColor();
		mesh_cmp->m_MeshColor.x = GetRValue(color) / 255.0f;
		mesh_cmp->m_MeshColor.y = GetGValue(color) / 255.0f;
		mesh_cmp->m_MeshColor.z = GetBValue(color) / 255.0f;
		mesh_cmp->m_MeshColor.w = pProp->GetParent()->GetSubItem(PropId + 4)->GetValue().lVal / 255.0f;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshInstance:
	{
		MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().pulVal);
		mesh_cmp->m_bInstance = pProp->GetValue().boolVal != 0;
		// ! reset shader handles
		mesh_cmp->handle_World = NULL;
		Material::MaterialParameterPtrList::iterator param_iter = mesh_cmp->m_Material->m_ParameterList.begin();
		for (; param_iter != mesh_cmp->m_Material->m_ParameterList.end(); param_iter++)
		{
			(*param_iter)->m_Handle = NULL;
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialShader:
	{
		Material* material = (Material*)pProp->GetParent()->GetValue().pulVal;
		std::string path = theApp.GetRelativePath(ts2ms(pProp->GetValue().bstrVal).c_str());
		if (path.empty())
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesMaterial(pProp->GetParent(), material);
			return 0;
		}
		material->ReleaseResource();
		material->m_Shader = path;
		material->ParseShaderParameters();
		material->RequestResource();
		UpdatePropertiesMaterial(pProp->GetParent(), material);
		m_wndPropList.AdjustLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialPassMask:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_PassMaskDesc));
		material->m_PassMask = g_PassMaskDesc[i].mask;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialCullMode:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_CullModeDesc));
		material->m_CullMode = i + 1;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialZEnable:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().pulVal;
		material->m_ZEnable = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialZWriteEnable:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().pulVal;
		material->m_ZWriteEnable = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialBlendMode:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_BlendModeDesc));
		material->m_BlendMode = i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterFloat:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeFloat);
		boost::dynamic_pointer_cast<MaterialParameterFloat>(mtl->m_ParameterList[i])->m_Value = pProp->GetValue().fltVal;
		my::EventArg arg;
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
		Material * mtl = (Material *)pParameter->GetParent()->GetParent()->GetValue().pulVal;
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
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterTexture:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->m_Type == MaterialParameter::ParameterTypeTexture);
		std::string path = theApp.GetRelativePath(ts2ms(pProp->GetValue().bstrVal).c_str());
		if (path.empty())
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesMaterialParameter(pProp->GetParent(), i, mtl->m_ParameterList[i].get());
			return 0;
		}
		mtl->m_ParameterList[i]->ReleaseResource();
		boost::dynamic_pointer_cast<MaterialParameterTexture>(mtl->m_ParameterList[i])->m_TexturePath = path;
		mtl->m_ParameterList[i]->RequestResource();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyClothColor:
	case PropertyClothAlpha:
	{
		ClothComponent* cloth_cmp = dynamic_cast<ClothComponent*>((Component*)pProp->GetParent()->GetValue().pulVal);
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 0)))->GetColor();
		cloth_cmp->m_MeshColor.x = GetRValue(color) / 255.0f;
		cloth_cmp->m_MeshColor.y = GetGValue(color) / 255.0f;
		cloth_cmp->m_MeshColor.z = GetBValue(color) / 255.0f;
		cloth_cmp->m_MeshColor.w = pProp->GetParent()->GetSubItem(PropId + 1)->GetValue().lVal / 255.0f;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyClothSceneCollision:
	{
		ClothComponent * cloth_cmp = (ClothComponent *)pProp->GetParent()->GetValue().pulVal;
		cloth_cmp->m_Cloth->setClothFlag(physx::PxClothFlag::eSCENE_COLLISION, pProp->GetValue().boolVal != 0);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterFaceType:
	{
		EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_EmitterFaceType));
		emit_cmp->m_EmitterFaceType = (EmitterComponent::FaceType)i;
		// ! reset shader handles
		emit_cmp->handle_World = NULL;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterSpaceType:
	{
		EmitterComponent* emit_cmp = (EmitterComponent*)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_EmitterSpaceType));
		EmitterComponent::SpaceType old_space_type = emit_cmp->m_EmitterSpaceType;
		emit_cmp->m_EmitterSpaceType = (EmitterComponent::SpaceType)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterVelType:
	{
		EmitterComponent* emit_cmp = (EmitterComponent*)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_EmitterVelType));
		emit_cmp->m_EmitterVelType = (EmitterComponent::VelocityType)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterPrimitiveType:
	{
		EmitterComponent* emit_cmp = (EmitterComponent*)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_EmitterPrimitiveType));
		emit_cmp->m_EmitterPrimitiveType = (EmitterComponent::PrimitiveType)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterParticleCount:
	{
		//EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		//Actor * actor = emit_cmp->m_Actor;
		//unsigned int new_size = pProp->GetValue().uintVal;
		//if (new_size < emit_cmp->m_ParticleList.size())
		//{
		//	emit_cmp->m_ParticleList.set_capacity(new_size);
		//}
		//else
		//{
		//	emit_cmp->m_ParticleList.resize(new_size, my::Emitter::Particle(actor->m_Position, my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0, 0));
		//}
		//dynamic_cast<StaticEmitterComponent*>(emit_cmp)->BuildChunks();
		//actor->UpdateAABB();
		//actor->UpdateOctNode();
		//pFrame->UpdateSelBox();
		//pFrame->UpdatePivotTransform();
		//UpdatePropertiesStaticEmitter(pProp->GetParent()->GetParent(), dynamic_cast<StaticEmitterComponent*>(emit_cmp));
		//m_wndPropList.AdjustLayout();
		my::EventArg arg;
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
		//CMFCPropertyGridProperty * pParticle = NULL;
		//switch (PropertyId)
		//{
		//case PropertyEmitterParticlePositionX:
		//case PropertyEmitterParticlePositionY:
		//case PropertyEmitterParticlePositionZ:
		//case PropertyEmitterParticleVelocityX:
		//case PropertyEmitterParticleVelocityY:
		//case PropertyEmitterParticleVelocityZ:
		//case PropertyEmitterParticleSizeX:
		//case PropertyEmitterParticleSizeY:
		//	pParticle = pProp->GetParent()->GetParent();
		//	break;
		//case PropertyEmitterParticlePosition:
		//case PropertyEmitterParticleVelocity:
		//case PropertyEmitterParticleColor:
		//case PropertyEmitterParticleColorAlpha:
		//case PropertyEmitterParticleSize:
		//case PropertyEmitterParticleAngle:
		//	pParticle = pProp->GetParent();
		//	break;
		//}
		//int NodeId = (int)pParticle->GetData();
		//EmitterComponent * emit_cmp = (EmitterComponent *)pParticle->GetParent()->GetParent()->GetValue().pulVal;
		//my::Emitter::Particle & particle = emit_cmp->m_ParticleList[NodeId];
		//particle.m_Position.x = pParticle->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal;
		//particle.m_Position.y = pParticle->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal;
		//particle.m_Position.z = pParticle->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal;
		//particle.m_Velocity.x = pParticle->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
		//particle.m_Velocity.y = pParticle->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
		//particle.m_Velocity.z = pParticle->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
		//COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pParticle->GetSubItem(2)))->GetColor();
		//particle.m_Color.x = GetRValue(color) / 255.0f;
		//particle.m_Color.y = GetGValue(color) / 255.0f;
		//particle.m_Color.z = GetBValue(color) / 255.0f;
		//particle.m_Color.w = pParticle->GetSubItem(3)->GetValue().lVal / 255.0f;
		//particle.m_Size.x = pParticle->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
		//particle.m_Size.y = pParticle->GetSubItem(4)->GetSubItem(1)->GetValue().fltVal;
		//particle.m_Angle = D3DXToRadian(pParticle->GetSubItem(5)->GetValue().fltVal);
		//dynamic_cast<StaticEmitterComponent*>(emit_cmp)->BuildChunks();
		//Actor * actor = emit_cmp->m_Actor;
		//actor->UpdateAABB();
		//actor->UpdateOctNode();
		//pFrame->UpdateSelBox();
		//pFrame->UpdatePivotTransform();
		//UpdatePropertiesStaticEmitter(pParticle->GetParent()->GetParent(), dynamic_cast<StaticEmitterComponent*>(emit_cmp));
		//m_wndPropList.AdjustLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticEmitterChunkStep:
	{
		//StaticEmitterComponent * emit_cmp = (StaticEmitterComponent*)pProp->GetParent()->GetValue().pulVal;
		//emit_cmp->m_ChunkStep = my::Max((float)EPSILON_E3, pProp->GetValue().fltVal);
		//if (!emit_cmp->m_Chunks.empty())
		//{
		//	emit_cmp->BuildChunks();
		//}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertySphericalEmitterParticleCapacity:
	{
		SphericalEmitterComponent * sphe_emit_cmp = (SphericalEmitterComponent *)pProp->GetParent()->GetValue().pulVal;
		unsigned int new_size = pProp->GetValue().uintVal;
		sphe_emit_cmp->m_ParticleList.set_capacity(new_size);
		UpdatePropertiesSphericalEmitter(pProp->GetParent(), sphe_emit_cmp);
		my::EventArg arg;
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
	case PropertySphericalEmitterSpawnCycle:
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
		SphericalEmitterComponent * sphe_emit_cmp = (SphericalEmitterComponent *)pComponent->GetValue().pulVal;
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		sphe_emit_cmp->m_ParticleLifeTime = pComponent->GetSubItem(PropId + 5)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnInterval = pComponent->GetSubItem(PropId + 6)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.x = pComponent->GetSubItem(PropId + 7)->GetSubItem(0)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.y = pComponent->GetSubItem(PropId + 7)->GetSubItem(1)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.z = pComponent->GetSubItem(PropId + 7)->GetSubItem(2)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnSpeed = pComponent->GetSubItem(PropId + 8)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnCycle = pComponent->GetSubItem(PropId + 18)->GetValue().fltVal;
		my::EventArg arg;
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
		my::Spline * spline = (my::Spline *)pSpline->GetValue().pulVal;
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
			int NodeId = (int)pNode->GetData();
			_ASSERT(NodeId < (int)spline->size());
			my::SplineNode & node = (*spline)[NodeId];
			node.x = pNode->GetSubItem(PropertySplineNodeX - PropertySplineNodeX)->GetValue().fltVal;
			node.y = pNode->GetSubItem(PropertySplineNodeY - PropertySplineNodeX)->GetValue().fltVal;
			node.k0 = pNode->GetSubItem(PropertySplineNodeK0 - PropertySplineNodeX)->GetValue().fltVal;
			node.k = pNode->GetSubItem(PropertySplineNodeK - PropertySplineNodeX)->GetValue().fltVal;
		}
		break;
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyTerrainHeightMap:
	{
		CMFCPropertyGridProperty * pComponent = pProp->GetParent();
		Terrain * terrain = (Terrain *)pComponent->GetValue().pulVal;
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		ImportHeightDlg dlg;
		dlg.m_AssetPath = pComponent->GetSubItem(PropId + 4)->GetValue().bstrVal;
		if (!dlg.m_AssetPath.IsEmpty())
		{
			TerrainStream tstr(terrain);
			my::IStreamPtr istr = my::FileIStream::Open(dlg.m_AssetPath);
			dlg.m_TerrainSize = (int)sqrt(istr->GetSize() / sizeof(unsigned short));
			if (dlg.DoModal() != IDOK)
			{
				return 0;
			}
			for (int i = 0; i < my::Min(terrain->m_RowChunks * terrain->m_ChunkSize + 1, dlg.m_TerrainSize); i++)
			{
				istr->seek(i* dlg.m_TerrainSize * sizeof(unsigned short));
				for (int j = 0; j < my::Min(terrain->m_ColChunks * terrain->m_ChunkSize + 1, dlg.m_TerrainSize); j++)
				{
					unsigned short r16;
					istr->read(&r16, sizeof(r16));
					tstr.SetPos(my::Vector3(j, (float)r16 / USHRT_MAX * dlg.m_MaxHeight - dlg.m_WaterLevel, i), i, j, false);
				}
			}
			for (int i = 0; i < my::Min(terrain->m_RowChunks * terrain->m_ChunkSize + 1, dlg.m_TerrainSize); i++)
			{
				for (int j = 0; j < my::Min(terrain->m_ColChunks * terrain->m_ChunkSize + 1, dlg.m_TerrainSize); j++)
				{
					const my::Vector3 pos = tstr.GetPos(i, j);
					const my::Vector3 Dirs[4] = {
						tstr.GetPos(i - 1, j) - pos,
						tstr.GetPos(i, j - 1) - pos,
						tstr.GetPos(i + 1, j) - pos,
						tstr.GetPos(i, j + 1) - pos
					};
					const my::Vector3 Nors[4] = {
						Dirs[0].cross(Dirs[1]).normalize(),
						Dirs[1].cross(Dirs[2]).normalize(),
						Dirs[2].cross(Dirs[3]).normalize(),
						Dirs[3].cross(Dirs[0]).normalize()
					};
					const my::Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();
					tstr.SetNormal(Normal, i, j);
				}
			}
			istr.reset();
			tstr.Release();
			Actor * actor = terrain->m_Actor;
			actor->UpdateAABB();
			actor->UpdateOctNode();
			pFrame->UpdateSelBox();
			pFrame->UpdatePivotTransform();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyTerrainSplatMap:
	{
		Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().pulVal;
		CString strPath = pProp->GetValue().bstrVal;
		if (!strPath.IsEmpty())
		{
			TerrainStream tstr(terrain);
			my::Texture2D tex;
			tex.CreateTextureFromFile(strPath, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL);
			D3DSURFACE_DESC desc = tex.GetLevelDesc();
			if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8)
			{
				MessageBox(_T("unsupported splatmap format"));
				return 0;
			}
			D3DLOCKED_RECT lrc = tex.LockRect(NULL, D3DLOCK_READONLY, 0);
			boost::multi_array_ref<DWORD, 2> pixel((DWORD*)lrc.pBits, boost::extents[desc.Height][lrc.Pitch / sizeof(DWORD)]);
			for (int i = 0; i < my::Min<int>(terrain->m_RowChunks * terrain->m_ChunkSize + 1, desc.Height); i++)
			{
				for (int j = 0; j < my::Min<int>(terrain->m_ColChunks * terrain->m_ChunkSize + 1, desc.Width); j++)
				{
					tstr.SetColor(pixel[i][j], i, j);
				}
			}
			tex.UnlockRect(0);
			tstr.Release();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyTerrainChunkMaterial:
	{
		CMFCPropertyGridProperty* pComponent = pProp->GetParent();
		Terrain* terrain = (Terrain*)pComponent->GetValue().pulVal;
		CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT_VALID(pFrame);
		if (pProp->GetValue().boolVal)
		{
			if (!terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material)
			{
				terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material = terrain->m_Material->Clone();
				if (terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->IsRequested())
				{
					terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material->RequestResource();
				}
			}
		}
		else
		{
			if (terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material)
			{
				if (terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->IsRequested())
				{
					terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material->ReleaseResource();
				}
				terrain->m_Chunks[pFrame->m_selchunkid.x][pFrame->m_selchunkid.y]->m_Material.reset();
			}
		}
		UpdatePropertiesTerrain(pComponent, terrain);
		m_wndPropList.AdjustLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintShape:
	{
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_PaintShape));
		pFrame->m_PaintShape = (CMainFrame::PaintShape)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintMode:
	{
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_PaintMode));
		pFrame->m_PaintMode = (CMainFrame::PaintMode)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintRadius:
	{
		pFrame->m_PaintRadius = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintHeight:
	{
		pFrame->m_PaintHeight = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintColor:
	{
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp))->GetColor();
		pFrame->m_PaintColor.r = GetRValue(color) / 255.0f;
		pFrame->m_PaintColor.g = GetGValue(color) / 255.0f;
		pFrame->m_PaintColor.b = GetBValue(color) / 255.0f;
		pFrame->m_PaintColor.a = my::Clamp(1.0f - pFrame->m_PaintColor.r - pFrame->m_PaintColor.g - pFrame->m_PaintColor.b, 0.0f, 1.0f);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintDensity:
	{
		pFrame->m_PaintDensity = pProp->GetValue().intVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyAnimatorSkeletonPath:
	{
		break;
	}
	case PropertyAnimationNodeSequenceName:
	{
		CMFCPropertyGridProperty * pAnimatioNode = pProp->GetParent();
		ASSERT(pAnimatioNode->GetData() == PropertyAnimationNode);
		AnimationNodeSequence* node = dynamic_cast<AnimationNodeSequence*>((AnimationNode*)pAnimatioNode->GetValue().pulVal);
		ASSERT(node);
		node->m_Name = ts2ms(pProp->GetValue().bstrVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	}
	return 0;
}

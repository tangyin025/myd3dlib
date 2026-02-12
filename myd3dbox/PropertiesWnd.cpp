// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"

#include "PropertiesWnd.h"
#include "CtrlProps.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "ShapeDlg.h"
#include "Material.h"
#include "Terrain.h"
#include "StaticEmitter.h"
#include "StaticMesh.h"
#include "Animator.h"
#include "NavigationSerialization.h"
#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "ImportHeightDlg.h"
#include "DetourNavMesh.h"
#include <ft2build.h>
#include <freetype/freetype.h>
#include <boost/regex.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CPropertiesWnd::PassMaskDesc g_PassMaskDesc[] =
{
	{ _T("None"), Material::PassMaskNone },
	{ _T("Shadow"), Material::PassMaskShadow },
	{ _T("Light"), Material::PassMaskLight },
	{ _T("Background"), Material::PassMaskBackground },
	{ _T("Opaque"), Material::PassMaskOpaque },
	{ _T("NormalOpaque"), Material::PassMaskNormalOpaque },
	{ _T("ShadowNormalOpaque"), Material::PassMaskShadowNormalOpaque },
	{ _T("Transparent"), Material::PassMaskTransparent },
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

static const LPCTSTR g_InstanceTypeDesc[] =
{
	_T("None"),
	_T("Instance"),
	_T("Batch")
};

static const LPCTSTR g_CullModeDesc[] =
{
	_T("NONE"),
	_T("CW"),
	_T("CCW")
};

static const LPCTSTR g_ZFuncDesc[] =
{
	_T("NEVER"),
	_T("LESS"),
	_T("EQUAL"),
	_T("LESSEQUAL"),
	_T("GREATER"),
	_T("NOTEQUAL"),
	_T("GREATEREQUAL"),
	_T("ALWAYS")
};

static const LPCTSTR g_BlendModeDesc[] =
{
	_T("None"),
	_T("Alpha"),
	_T("Additive")
};

static const LPCTSTR g_VirtualParticleLevel[] =
{
	_T("None"),
	_T("1: e0, t1, q1"),
	_T("2: e1, t0, q0"),
	_T("3: e1, t1, q1"),
	_T("4: e0, t3, q4"),
	_T("5: e1, t3, q4")
};

static const boost::regex g_rcolor("color|colour", boost::regex::icase);

static const CPropertiesWnd::PassMaskDesc g_LodMaskDesc[] =
{
	{ _T("LOD0"), Component::LOD0 },
	{ _T("LOD1"), Component::LOD1 },
	{ _T("LOD2"), Component::LOD2 },
	{ _T("LOD0_1"), Component::LOD0_1 },
	{ _T("LOD1_2"), Component::LOD1_2 },
	{ _T("LOD0_1_2"), Component::LOD0_1_2 },
	{ _T("INVISIBLE"), 0 },
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

static LPCTSTR g_EmitterFaceType[EmitterComponent::FaceTypeStretchedCamera + 1] =
{
	_T("X"),
	_T("Y"),
	_T("Z"),
	_T("Camera"),
	_T("Angle"),
	_T("AngleCamera"),
	_T("StretchedCamera"),
};

static LPCTSTR g_EmitterSpaceType[EmitterComponent::SpaceTypeLocal + 1] =
{
	_T("World"),
	_T("Local"),
};

static LPCTSTR g_EmitterPrimitiveType[StaticEmitter::PrimitiveTypeMesh + 1] =
{
	_T("Triangle"),
	_T("Quad"),
	_T("Mesh...")
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

static LPCTSTR g_PaintMode[CMainFrame::PaintModeAssign + 1] =
{
	_T("Assign"),
};

static CPropertiesWnd::PassMaskDesc g_FontAlignDesc[] =
{
	{ _T("AlignLeftTop"), my::Font::AlignLeftTop },
	{ _T("AlignLeftTopMultiLine"), my::Font::AlignLeftTop | my::Font::AlignMultiLine },
	{ _T("AlignLeftTopVertical"), my::Font::AlignLeftTop | my::Font::AlignVertical },
	{ _T("AlignLeftTopVerticalMultiLine"), my::Font::AlignLeftTop | my::Font::AlignVertical | my::Font::AlignMultiLine },
	{ _T("AlignCenterTop"), my::Font::AlignCenterTop },
	{ _T("AlignRightTop"), my::Font::AlignRightTop },
	{ _T("AlignLeftMiddle"), my::Font::AlignLeftMiddle },
	{ _T("AlignCenterMiddle"), my::Font::AlignCenterMiddle },
	{ _T("AlignRightMiddle"), my::Font::AlignRightMiddle },
	{ _T("AlignLeftBottom"), my::Font::AlignLeftBottom },
	{ _T("AlignCenterBottom"), my::Font::AlignCenterBottom },
	{ _T("AlignRightBottom"), my::Font::AlignRightBottom },
};

static LPCTSTR GetFontAlignDesc(DWORD mask)
{
	for (unsigned int i = 0; i < _countof(g_FontAlignDesc); i++)
	{
		if (g_FontAlignDesc[i].mask == mask)
		{
			return g_FontAlignDesc[i].desc;
		}
	}
	return g_FontAlignDesc[0].desc;
}

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
	: m_OnPropertyChangeMuted(FALSE)
{
	memset(&m_pProp, 0, sizeof(m_pProp));
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
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
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CPropertiesWnd::OnSelectionChanged(my::EventArg * arg)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMainFrame::ActorList::iterator actor_iter = pFrame->m_selactors.begin();
	if (actor_iter != pFrame->m_selactors.end())
	{
		if (!m_OnPropertyChangeMuted)
		{
			if (pFrame->m_PaintType == CMainFrame::PaintTypeTerrainHeightField
				|| pFrame->m_PaintType == CMainFrame::PaintTypeTerrainColor
				|| pFrame->m_PaintType == CMainFrame::PaintTypeEmitterInstance)
			{
				UpdatePropertiesPaintTool(*actor_iter);
				m_wndPropList.AdjustLayout();
			}
			else
			{
				UpdatePropertiesActor(*actor_iter);
				m_wndPropList.AdjustLayout();
			}
		}
	}
	else if (!pFrame->m_selctls.empty())
	{
		if (!m_OnPropertyChangeMuted)
		{
			UpdatePropertiesControl(pFrame->m_selctls.front());
			m_wndPropList.AdjustLayout();
		}
	}
	else
	{
		m_wndPropList.EndEditItem(FALSE);
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
	pActor->SetName(_T("Actor"), FALSE);
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
	pActor->GetSubItem(7)->SetValue((_variant_t)actor->m_CullingDistSq);
	UpdatePropertiesRigidActor(pActor->GetSubItem(8), actor);
	unsigned int PropId = 9;
	Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
	for (unsigned int i = 0; cmp_iter != actor->m_Cmps.end(); cmp_iter++, i++)
	{
		if ((unsigned int)pActor->GetSubItemsCount() <= PropId + i)
		{
			CreateProperties(pActor, cmp_iter->get());
			continue;
		}
		UpdateProperties(pActor->GetSubItem(PropId + i), i, cmp_iter->get());
	}
	RemovePropertiesFrom(pActor, PropId + (int)actor->m_Cmps.size());
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
	CString strTitle;
	strTitle.Format(_T("%s: %S"), GetComponentTypeName(cmp->GetComponentType()), cmp->GetName());
	pComponent->SetName(strTitle, FALSE);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp);
	pComponent->GetSubItem(0)->SetValue((_variant_t)ms2ts(cmp->GetName()).c_str());
	pComponent->GetSubItem(1)->SetValue((_variant_t)GetLodMaskDesc(cmp->m_LodMask));
	pComponent->GetSubItem(2)->SetValue((_variant_t)cmp->GetSiblingId());
	UpdatePropertiesShape(pComponent->GetSubItem(3), cmp);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (cmp->GetComponentType())
	{
	case Component::ComponentTypeMesh:
		UpdatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeStaticMesh:
		UpdatePropertiesStaticMesh(pComponent, dynamic_cast<StaticMesh *>(cmp));
		break;
	case Component::ComponentTypeCloth:
		UpdatePropertiesCloth(pComponent, dynamic_cast<ClothComponent *>(cmp));
		break;
	case Component::ComponentTypeStaticEmitter:
	case Component::ComponentTypeSphericalEmitter:
		UpdatePropertiesEmitter(pComponent, dynamic_cast<EmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		UpdatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	case Component::ComponentTypeAnimator:
		UpdatePropertiesAnimator(pComponent, dynamic_cast<Animator *>(cmp));
		break;
	case Component::ComponentTypeNavigation:
		UpdatePropertiesNavigation(pComponent, dynamic_cast<Navigation *>(cmp));
		break;
	default:
		RemovePropertiesFrom(pComponent, GetComponentPropCount(Component::ComponentTypeComponent));
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
	CMFCPropertyGridProperty* pProp = pComponent->GetSubItem(PropId + 3);
	if (!pProp || pProp->GetData() != PropertyMeshSubMeshId)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesMesh(pComponent, mesh_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(mesh_cmp->m_MeshPath.c_str()).c_str());
	COLORREF color = RGB(mesh_cmp->m_MeshColor.x * 255, mesh_cmp->m_MeshColor.y * 255, mesh_cmp->m_MeshColor.z * 255);
	(DYNAMIC_DOWNCAST(CColorProp, pComponent->GetSubItem(PropId + 1)))->SetColor(color);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)(long)(mesh_cmp->m_MeshColor.w * 255));
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)mesh_cmp->m_MeshSubMeshId);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)(mesh_cmp->m_Mesh ? mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].VertexCount : 0));
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)(mesh_cmp->m_Mesh ? mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceCount : 0));
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)g_InstanceTypeDesc[mesh_cmp->m_InstanceType]);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 7), mesh_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesMaterial(CMFCPropertyGridProperty * pMaterial, Material * mtl)
{
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mtl);
	pMaterial->GetSubItem(0)->SetValue((_variant_t)theApp.GetFullPath(mtl->m_Shader.c_str()).c_str());
	pMaterial->GetSubItem(1)->SetValue((_variant_t)GetPassMaskDesc(mtl->m_PassMask));
	pMaterial->GetSubItem(2)->SetValue((_variant_t)g_CullModeDesc[mtl->m_CullMode - 1]);
	pMaterial->GetSubItem(3)->SetValue((_variant_t)(VARIANT_BOOL)mtl->m_ZEnable);
	pMaterial->GetSubItem(4)->SetValue((_variant_t)(VARIANT_BOOL)mtl->m_ZWriteEnable);
	pMaterial->GetSubItem(5)->SetValue((_variant_t)g_ZFuncDesc[mtl->m_ZFunc - 1]);
	pMaterial->GetSubItem(6)->SetValue((_variant_t)(VARIANT_BOOL)mtl->m_AlphaTestEnable);
	pMaterial->GetSubItem(7)->SetValue((_variant_t)mtl->m_AlphaRef);
	pMaterial->GetSubItem(8)->SetValue((_variant_t)g_ZFuncDesc[mtl->m_AlphaFunc - 1]);
	pMaterial->GetSubItem(9)->SetValue((_variant_t)g_BlendModeDesc[mtl->m_BlendMode]);

	CMFCPropertyGridProperty * pParameterList = pMaterial->GetSubItem(10);
	for (unsigned int i = 0; i < mtl->m_ParameterList.size(); i++)
	{
		if ((unsigned int)pParameterList->GetSubItemsCount() <= i)
		{
			CreatePropertiesMaterialParameter(pParameterList, i, mtl->m_ParameterList[i].get());
			continue;
		}
		if (pParameterList->GetSubItem(i)->GetData() != GetMaterialParameterTypeProp(mtl->m_ParameterList[i].get()))
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
	switch (mtl_param->GetParameterType())
	{
	case MaterialParameter::ParameterTypeInt2:
	{
		const CPoint& Value = dynamic_cast<MaterialParameterInt2*>(mtl_param)->m_Value;
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(0)->SetValue((_variant_t)Value.x);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(1)->SetValue((_variant_t)Value.y);
		break;
	}
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
		if (boost::regex_search(mtl_param->m_Name, g_rcolor, boost::match_default))
		{
			COLORREF color = RGB(Value.x * 255, Value.y * 255, Value.z * 255);
			(DYNAMIC_DOWNCAST(CColorProp, pParentCtrl->GetSubItem(NodeId)))->SetColor(color);
		}
		else
		{
			pParentCtrl->GetSubItem(NodeId)->GetSubItem(0)->SetValue((_variant_t)Value.x);
			pParentCtrl->GetSubItem(NodeId)->GetSubItem(1)->SetValue((_variant_t)Value.y);
			pParentCtrl->GetSubItem(NodeId)->GetSubItem(2)->SetValue((_variant_t)Value.z);
		}
		break;
	}
	case MaterialParameter::ParameterTypeFloat4:
	{
		const my::Vector4 & Value = dynamic_cast<MaterialParameterFloat4*>(mtl_param)->m_Value;
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(0)->SetValue((_variant_t)Value.x);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(1)->SetValue((_variant_t)Value.y);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(2)->SetValue((_variant_t)Value.z);
		pParentCtrl->GetSubItem(NodeId)->GetSubItem(3)->SetValue((_variant_t)Value.w);
		break;
	}
	case MaterialParameter::ParameterTypeTexture:
		pParentCtrl->GetSubItem(NodeId)->SetValue((_variant_t)
			theApp.GetFullPath(dynamic_cast<MaterialParameterTexture *>(mtl_param)->m_TexturePath.c_str()).c_str());
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesStaticMesh(CMFCPropertyGridProperty * pComponent, StaticMesh * static_mesh_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty* pProp = pComponent->GetSubItem(PropId);
	CMFCPropertyGridProperty* pChunkWidth = pComponent->GetSubItem(PropId);
	if (!pChunkWidth || pChunkWidth->GetData() != PropertyStaticEmitterChunkWidth)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesStaticMesh(pComponent, static_mesh_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)static_mesh_cmp->m_ChunkWidth);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)ms2ts(static_mesh_cmp->m_ChunkPath.c_str()).c_str());
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)static_mesh_cmp->m_ChunkLodScale);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 3), static_mesh_cmp->m_Material.get());
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
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)(VARIANT_BOOL)flags.isSet(physx::PxClothFlag::eSWEPT_CONTACT));
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)(VARIANT_BOOL)flags.isSet(physx::PxClothFlag::eSCENE_COLLISION));
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)cloth_cmp->m_Cloth->getSolverFrequency());
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)cloth_cmp->m_Cloth->getStiffnessFrequency());
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)cloth_cmp->m_Cloth->getNbCollisionSpheres());
	pComponent->GetSubItem(PropId + 7)->SetValue((_variant_t)cloth_cmp->m_Cloth->getNbCollisionCapsules());
	pComponent->GetSubItem(PropId + 9)->SetValue((_variant_t)cloth_cmp->m_Cloth->getNbVirtualParticles());
	physx::PxClothStretchConfig stretchConfig = cloth_cmp->m_Cloth->getStretchConfig(physx::PxClothFabricPhaseType::eVERTICAL);
	pComponent->GetSubItem(PropId + 10)->GetSubItem(0)->SetValue((_variant_t)stretchConfig.stiffness);
	pComponent->GetSubItem(PropId + 10)->GetSubItem(1)->SetValue((_variant_t)stretchConfig.stiffnessMultiplier);
	pComponent->GetSubItem(PropId + 10)->GetSubItem(2)->SetValue((_variant_t)stretchConfig.compressionLimit);
	pComponent->GetSubItem(PropId + 10)->GetSubItem(3)->SetValue((_variant_t)stretchConfig.stretchLimit);
	physx::PxClothTetherConfig tetherConfig = cloth_cmp->m_Cloth->getTetherConfig();
	pComponent->GetSubItem(PropId + 11)->GetSubItem(0)->SetValue((_variant_t)tetherConfig.stiffness);
	pComponent->GetSubItem(PropId + 11)->GetSubItem(1)->SetValue((_variant_t)tetherConfig.stretchLimit);
	my::Vector3 acceleration = cloth_cmp->GetExternalAcceleration();
	pComponent->GetSubItem(PropId + 12)->GetSubItem(0)->SetValue((_variant_t)acceleration.x);
	pComponent->GetSubItem(PropId + 12)->GetSubItem(1)->SetValue((_variant_t)acceleration.y);
	pComponent->GetSubItem(PropId + 12)->GetSubItem(2)->SetValue((_variant_t)acceleration.z);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 13), cloth_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty* pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyEmitterFaceType)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesEmitter(pComponent, emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)g_EmitterFaceType[emit_cmp->m_EmitterFaceType]);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)g_EmitterSpaceType[emit_cmp->m_EmitterSpaceType]);
	pComponent->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)emit_cmp->m_Tiles.x);
	pComponent->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)emit_cmp->m_Tiles.y);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)g_EmitterPrimitiveType[emit_cmp->m_ParticlePrimitiveType]);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)ms2ts(emit_cmp->m_MeshPath.c_str()).c_str());
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)emit_cmp->m_MeshSubMeshId);
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)(emit_cmp->m_Mesh ? emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].VertexCount : 0));
	pComponent->GetSubItem(PropId + 7)->SetValue((_variant_t)(emit_cmp->m_Mesh ? emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].FaceCount : 0));

	switch (emit_cmp->GetComponentType())
	{
	case Component::ComponentTypeStaticEmitter:
		UpdatePropertiesStaticEmitter(pComponent, dynamic_cast<StaticEmitter*>(emit_cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		UpdatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitter*>(emit_cmp));
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitter * emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeEmitter);
	CMFCPropertyGridProperty* pChunkWidth = pComponent->GetSubItem(PropId);
	if (!pChunkWidth || pChunkWidth->GetData() != PropertyStaticEmitterChunkWidth)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesStaticEmitter(pComponent, emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)emit_cmp->m_ChunkWidth);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)ms2ts(emit_cmp->m_ChunkPath.c_str()).c_str());
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)emit_cmp->m_ChunkLodScale);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 3), emit_cmp->m_Material.get());
	CMFCPropertyGridProperty * pParticle = pComponent->GetSubItem(PropId + 4);
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	StaticEmitter::ChunkMap::const_iterator chunk_iter = (pFrame->m_selcmp == emit_cmp ? emit_cmp->m_Chunks.find(std::make_pair(pFrame->m_selchunkid.x, pFrame->m_selchunkid.y)) : emit_cmp->m_Chunks.begin());
	if (chunk_iter != emit_cmp->m_Chunks.end() && chunk_iter->second.m_buff && pFrame->m_selinstid < chunk_iter->second.m_buff->size())
	{
		if (pParticle->GetSubItemsCount() > 0)
		{
			UpdatePropertiesStaticEmitterParticle(pComponent->GetSubItem(PropId + 4), CPoint(chunk_iter->first.first, chunk_iter->first.second), pFrame->m_selinstid, &(*chunk_iter->second.m_buff)[pFrame->m_selinstid]);
		}
		else
		{
			RemovePropertiesFrom(pComponent, PropId + 4);
			CreatePropertiesStaticEmitterParticle(pComponent, CPoint(chunk_iter->first.first, chunk_iter->first.second), pFrame->m_selinstid, &(*chunk_iter->second.m_buff)[pFrame->m_selinstid]);
		}
	}
	else
	{
		RemovePropertiesFrom(pComponent->GetSubItem(PropId + 4), 0);
	}
}

void CPropertiesWnd::UpdatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParticle, const CPoint & chunkid, int instid, my::Emitter::Particle * particle)
{
	CString strTitle;
	strTitle.Format(_T("Particle_%d_%d_%d"), chunkid.x, chunkid.y, instid);
	pParticle->SetName(strTitle, FALSE);
	pParticle->SetData(MAKELONG(chunkid.x, chunkid.y));
	pParticle->SetValue((_variant_t)instid);
	CMFCPropertyGridProperty * pProp = pParticle->GetSubItem(0)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionX); pProp->SetValue((_variant_t)particle->m_Position.x);
	pProp = pParticle->GetSubItem(0)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionY); pProp->SetValue((_variant_t)particle->m_Position.y);
	pProp = pParticle->GetSubItem(0)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionZ); pProp->SetValue((_variant_t)particle->m_Position.z);
	pProp = pParticle->GetSubItem(0)->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticlePositionW); pProp->SetValue((_variant_t)particle->m_Position.w);
	pProp = pParticle->GetSubItem(1)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityX); pProp->SetValue((_variant_t)particle->m_Velocity.x);
	pProp = pParticle->GetSubItem(1)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityY); pProp->SetValue((_variant_t)particle->m_Velocity.y);
	pProp = pParticle->GetSubItem(1)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityZ); pProp->SetValue((_variant_t)particle->m_Velocity.z);
	pProp = pParticle->GetSubItem(1)->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticleVelocityW); pProp->SetValue((_variant_t)particle->m_Velocity.w);
	COLORREF color = RGB(particle->m_Color.x * 255, particle->m_Color.y * 255, particle->m_Color.z * 255);
	pProp = pParticle->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleColor); (DYNAMIC_DOWNCAST(CColorProp, pProp))->SetColor(color);
	pProp = pParticle->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorAlpha); pProp->SetValue((_variant_t)(long)(particle->m_Color.w * 255));
	pProp = pParticle->GetSubItem(4)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeX); pProp->SetValue((_variant_t)particle->m_Size.x);
	pProp = pParticle->GetSubItem(4)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeY); pProp->SetValue((_variant_t)particle->m_Size.y);
	pProp = pParticle->GetSubItem(5); _ASSERT(pProp->GetData() == PropertyEmitterParticleAngle); pProp->SetValue((_variant_t)D3DXToDegree(particle->m_Angle));
}

void CPropertiesWnd::UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitter * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeEmitter);
	CMFCPropertyGridProperty * pParticleCapacity = pComponent->GetSubItem(PropId);
	if (!pParticleCapacity || pParticleCapacity->GetData() != PropertySphericalEmitterParticleCapacity)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesSphericalEmitter(pComponent, sphe_emit_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)(unsigned int)sphe_emit_cmp->m_ParticleList.capacity());
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnInterval);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnCount);
	pComponent->GetSubItem(PropId + 3)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.x);
	pComponent->GetSubItem(PropId + 3)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.y);
	pComponent->GetSubItem(PropId + 3)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_HalfSpawnArea.z);
	pComponent->GetSubItem(PropId + 4)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnInclination.x));
	pComponent->GetSubItem(PropId + 4)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnInclination.y));
	pComponent->GetSubItem(PropId + 5)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnAzimuth.x));
	pComponent->GetSubItem(PropId + 5)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnAzimuth.y));
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnSpeed);
	pComponent->GetSubItem(PropId + 7)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnBoneId);
	pComponent->GetSubItem(PropId + 8)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnLocalPose.m_position.x);
	pComponent->GetSubItem(PropId + 8)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnLocalPose.m_position.y);
	pComponent->GetSubItem(PropId + 8)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_SpawnLocalPose.m_position.z);
	my::Vector3 angle = sphe_emit_cmp->m_SpawnLocalPose.m_rotation.toEulerAngles();
	pComponent->GetSubItem(PropId + 9)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pComponent->GetSubItem(PropId + 9)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pComponent->GetSubItem(PropId + 9)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pComponent->GetSubItem(PropId + 10)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleLifeTime);
	pComponent->GetSubItem(PropId + 11)->GetSubItem(0)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleGravity.x);
	pComponent->GetSubItem(PropId + 11)->GetSubItem(1)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleGravity.y);
	pComponent->GetSubItem(PropId + 11)->GetSubItem(2)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleGravity.z);
	pComponent->GetSubItem(PropId + 12)->SetValue((_variant_t)sphe_emit_cmp->m_ParticleDamping);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 13), &sphe_emit_cmp->m_ParticleColorR);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 14), &sphe_emit_cmp->m_ParticleColorG);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 15), &sphe_emit_cmp->m_ParticleColorB);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 16), &sphe_emit_cmp->m_ParticleColorA);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 17), &sphe_emit_cmp->m_ParticleSizeX);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 18), &sphe_emit_cmp->m_ParticleSizeY);
	UpdatePropertiesSpline(pComponent->GetSubItem(PropId + 19), &sphe_emit_cmp->m_ParticleAngle);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 20), sphe_emit_cmp->m_Material.get());
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
			continue;
		}
		UpdatePropertiesSplineNode(pSpline, i, &(*spline)[i]);
	}
	RemovePropertiesFrom(pSpline, i + 1);
}

void CPropertiesWnd::UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, const my::Spline::value_type* node)
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
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)terrain->m_ChunkLodScale);
	pComponent->GetSubItem(PropId + 5);
	pComponent->GetSubItem(PropId + 6);
	UpdatePropertiesMaterial(pComponent->GetSubItem(PropId + 7), terrain->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesAnimator(CMFCPropertyGridProperty* pComponent, Animator* animator)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty* pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyAnimatorSkeletonPath)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesAnimator(pComponent, animator);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)theApp.GetFullPath(animator->m_SkeletonPath.c_str()).c_str());
	pComponent->GetSubItem(PropId + 1)->GetSubItem(0)->SetValue((_variant_t)animator->m_RootBone.m_position.x);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(1)->SetValue((_variant_t)animator->m_RootBone.m_position.y);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(2)->SetValue((_variant_t)animator->m_RootBone.m_position.z);
	my::Vector3 angle = animator->m_RootBone.m_rotation.toEulerAngles();
	pComponent->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pComponent->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pComponent->GetSubItem(PropId + 2)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	UpdatePropertiesAnimationNode(pComponent->GetSubItem(PropId + 3), animator->m_Childs[0].get());
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
			return;
		}

		UpdatePropertiesAnimationNodeSequence(pAnimationNode, seq);
	}
}

void CPropertiesWnd::UpdatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty * pAnimationNode, AnimationNodeSequence * seq)
{
	pAnimationNode->GetSubItem(1)->SetValue((_variant_t)ms2ts(seq->m_Name.c_str()).c_str());

	pAnimationNode->GetSubItem(1)->RemoveAllOptions();
	Animator* animator = dynamic_cast<Animator*>(seq->GetTopNode());
	if (animator->m_Skeleton)
	{
		std::set<std::string> dummy_map;
		std::transform(animator->m_Skeleton->m_animationMap.begin(), animator->m_Skeleton->m_animationMap.end(),
			std::inserter(dummy_map, dummy_map.end()), boost::bind(&my::OgreSkeletonAnimation::OgreAnimationMap::value_type::first, boost::placeholders::_1));
		std::set<std::string>::const_iterator anim_iter = dummy_map.begin();
		for (; anim_iter != dummy_map.end(); anim_iter++)
		{
			pAnimationNode->GetSubItem(1)->AddOption(ms2ts(anim_iter->c_str()).c_str(), TRUE);
		}
	}

	pAnimationNode->GetSubItem(2)->SetValue((_variant_t)seq->m_Rate);
}

void CPropertiesWnd::UpdatePropertiesNavigation(CMFCPropertyGridProperty * pComponent, Navigation * navigation)
{
	unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty* pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyNavigationNavMeshPath)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesNavigation(pComponent, navigation);
		return;
	}

	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(navigation->m_navMeshPath.c_str()).c_str());
	const dtNavMeshParams* params = navigation->m_navMesh->getParams();
	pComponent->GetSubItem(PropId + 1)->GetSubItem(0)->SetValue((_variant_t)params->orig[0]);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(1)->SetValue((_variant_t)params->orig[1]);
	pComponent->GetSubItem(PropId + 1)->GetSubItem(2)->SetValue((_variant_t)params->orig[2]);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)params->tileWidth);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)params->tileHeight);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)params->maxTiles);
	pComponent->GetSubItem(PropId + 5)->SetValue((_variant_t)params->maxPolys);
}

void CPropertiesWnd::UpdatePropertiesControl(my::Control * control)
{
	CMFCPropertyGridProperty * pControl = NULL;
	if (m_wndPropList.GetPropertyCount() >= 1)
	{
		pControl = m_wndPropList.GetProperty(0);
	}
	if (!pControl || pControl->GetData() != PropertyControl)
	{
		m_wndPropList.RemoveAll();
		CreatePropertiesControl(control);
		return;
	}

	pControl->SetName(GetControlTypeName(control->GetControlType()), FALSE);
	pControl->SetValue((_variant_t)(DWORD_PTR)control);
	pControl->GetSubItem(0)->SetValue((_variant_t)ms2ts(control->GetName()).c_str());
	pControl->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)control->m_x.scale);
	pControl->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)control->m_x.offset);
	pControl->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)control->m_y.scale);
	pControl->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)control->m_y.offset);
	pControl->GetSubItem(3)->GetSubItem(0)->SetValue((_variant_t)control->m_Width.scale);
	pControl->GetSubItem(3)->GetSubItem(1)->SetValue((_variant_t)control->m_Width.offset);
	pControl->GetSubItem(4)->GetSubItem(0)->SetValue((_variant_t)control->m_Height.scale);
	pControl->GetSubItem(4)->GetSubItem(1)->SetValue((_variant_t)control->m_Height.offset);
	pControl->GetSubItem(5)->SetValue((_variant_t)(VARIANT_BOOL)control->GetEnabled());
	pControl->GetSubItem(6)->SetValue((_variant_t)(VARIANT_BOOL)control->GetVisible());
	pControl->GetSubItem(7)->SetValue((_variant_t)(VARIANT_BOOL)(my::Control::GetFocusControl() == control));
	pControl->GetSubItem(8)->SetValue((_variant_t)control->GetSiblingId());

	COLORREF color = RGB(LOBYTE(control->m_Skin->m_Color >> 16), LOBYTE(control->m_Skin->m_Color >> 8), LOBYTE(control->m_Skin->m_Color));
	(DYNAMIC_DOWNCAST(CColorProp, pControl->GetSubItem(9)))->SetColor(color);
	pControl->GetSubItem(10)->SetValue((_variant_t)(long)LOBYTE(control->m_Skin->m_Color >> 24));
	pControl->GetSubItem(11)->SetValue((_variant_t)theApp.GetFullPath(control->m_Skin->m_Image->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(12)->GetSubItem(0)->SetValue((_variant_t)control->m_Skin->m_Image->m_Rect.left);
	pControl->GetSubItem(12)->GetSubItem(1)->SetValue((_variant_t)control->m_Skin->m_Image->m_Rect.top);
	pControl->GetSubItem(12)->GetSubItem(2)->SetValue((_variant_t)(long)control->m_Skin->m_Image->m_Rect.Width());
	pControl->GetSubItem(12)->GetSubItem(3)->SetValue((_variant_t)(long)control->m_Skin->m_Image->m_Rect.Height());
	pControl->GetSubItem(13)->GetSubItem(0)->SetValue((_variant_t)control->m_Skin->m_Image->m_Border.left);
	pControl->GetSubItem(13)->GetSubItem(1)->SetValue((_variant_t)control->m_Skin->m_Image->m_Border.top);
	pControl->GetSubItem(13)->GetSubItem(2)->SetValue((_variant_t)control->m_Skin->m_Image->m_Border.right);
	pControl->GetSubItem(13)->GetSubItem(3)->SetValue((_variant_t)control->m_Skin->m_Image->m_Border.bottom);

	pControl->GetSubItem(14)->SetValue((_variant_t)theApp.GetFullPath(control->m_Skin->m_VisibleShowSoundPath.c_str()).c_str());
	pControl->GetSubItem(15)->SetValue((_variant_t)theApp.GetFullPath(control->m_Skin->m_VisibleHideSoundPath.c_str()).c_str());
	pControl->GetSubItem(16)->SetValue((_variant_t)theApp.GetFullPath(control->m_Skin->m_MouseEnterSoundPath.c_str()).c_str());
	pControl->GetSubItem(17)->SetValue((_variant_t)theApp.GetFullPath(control->m_Skin->m_MouseLeaveSoundPath.c_str()).c_str());
	pControl->GetSubItem(18)->SetValue((_variant_t)theApp.GetFullPath(control->m_Skin->m_MouseClickSoundPath.c_str()).c_str());

	switch (control->GetControlType())
	{
	case my::Control::ControlTypeStatic:
	case my::Control::ControlTypeProgressBar:
	case my::Control::ControlTypeButton:
	//case my::Control::ControlTypeEditBox:
	case my::Control::ControlTypeImeEditBox:
		UpdatePropertiesStatic(pControl, dynamic_cast<my::Static*>(control));
		break;
	case my::Control::ControlTypeScrollBar:
	case my::Control::ControlTypeHorizontalScrollBar:
		UpdatePropertiesScrollBar(pControl, dynamic_cast<my::ScrollBar*>(control));
		break;
	case my::Control::ControlTypeCheckBox:
	case my::Control::ControlTypeComboBox:
		UpdatePropertiesStatic(pControl, dynamic_cast<my::Static*>(control));
		break;
	case my::Control::ControlTypeListBox:
		UpdatePropertiesListBox(pControl, dynamic_cast<my::ListBox*>(control));
		break;
	case my::Control::ControlTypeDialog:
		UpdatePropertiesDialog(pControl, dynamic_cast<my::Dialog*>(control));
		break;
	default:
		RemovePropertiesFrom(pControl, GetControlPropCount(my::Control::ControlTypeControl));
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesStatic(CMFCPropertyGridProperty * pControl, my::Static * static_ctl)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyStaticText)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesStatic(pControl, static_ctl);
		return;
	}
	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)ws2ts(boost::algorithm::replace_all_copy(static_ctl->m_Text, L"\n", L"\\n")).c_str());

	my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(static_ctl->m_Skin);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)theApp.GetFullPath(skin->m_FontPath.c_str()).c_str());
	pControl->GetSubItem(PropId + 2)->SetValue((_variant_t)(long)skin->m_FontHeight);

	TCHAR buff[65];
	CMFCPropertyGridProperty* pFontFaceIndex = pControl->GetSubItem(PropId + 3);
	pFontFaceIndex->SetValue((_variant_t)_itot(skin->m_FontFaceIndex, buff, 10));
	pFontFaceIndex->RemoveAllOptions();
	for (unsigned int i = 0; skin->m_Font && i < skin->m_Font->m_face->num_faces; i++)
	{
		pFontFaceIndex->AddOption(_itot(i, buff, 10), TRUE);
	}

	COLORREF color = RGB(LOBYTE(skin->m_TextColor >> 16), LOBYTE(skin->m_TextColor >> 8), LOBYTE(skin->m_TextColor));
	(DYNAMIC_DOWNCAST(CColorProp, pControl->GetSubItem(PropId + 4)))->SetColor(color);
	pControl->GetSubItem(PropId + 5)->SetValue((_variant_t)(long)LOBYTE(skin->m_TextColor >> 24));
	pControl->GetSubItem(PropId + 6)->SetValue(GetFontAlignDesc(skin->m_TextAlign));

	color = RGB(LOBYTE(skin->m_TextOutlineColor >> 16), LOBYTE(skin->m_TextOutlineColor >> 8), LOBYTE(skin->m_TextOutlineColor));
	(DYNAMIC_DOWNCAST(CColorProp, pControl->GetSubItem(PropId + 7)))->SetColor(color);
	pControl->GetSubItem(PropId + 8)->SetValue((_variant_t)(long)LOBYTE(skin->m_TextOutlineColor >> 24));
	pControl->GetSubItem(PropId + 9)->SetValue((_variant_t)skin->m_TextOutlineWidth);

	switch (static_ctl->GetControlType())
	{
	case my::Control::ControlTypeProgressBar:
		UpdatePropertiesProgressBar(pControl, dynamic_cast<my::ProgressBar*>(static_ctl));
		break;
	case my::Control::ControlTypeButton:
		UpdatePropertiesButton(pControl, dynamic_cast<my::Button*>(static_ctl));
		break;
	//case my::Control::ControlTypeEditBox:
	case my::Control::ControlTypeImeEditBox:
		UpdatePropertiesEditBox(pControl, dynamic_cast<my::EditBox*>(static_ctl));
		break;
	case my::Control::ControlTypeCheckBox:
	case my::Control::ControlTypeComboBox:
		UpdatePropertiesButton(pControl, dynamic_cast<my::Button*>(static_ctl));
		break;
	default:
		RemovePropertiesFrom(pControl, GetControlPropCount(my::Control::ControlTypeStatic));
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesProgressBar(CMFCPropertyGridProperty * pControl, my::ProgressBar * progressbar)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyProgressBarProgress)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesProgressBar(pControl, progressbar);
		return;
	}
	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)progressbar->m_Progress);
	my::ProgressBarSkinPtr skin = boost::dynamic_pointer_cast<my::ProgressBarSkin>(progressbar->m_Skin);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)theApp.GetFullPath(skin->m_ForegroundImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)skin->m_ForegroundImage->m_Rect.left);
	pControl->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)skin->m_ForegroundImage->m_Rect.top);
	pControl->GetSubItem(PropId + 2)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_ForegroundImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 2)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_ForegroundImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 3)->GetSubItem(0)->SetValue((_variant_t)skin->m_ForegroundImage->m_Border.left);
	pControl->GetSubItem(PropId + 3)->GetSubItem(1)->SetValue((_variant_t)skin->m_ForegroundImage->m_Border.top);
	pControl->GetSubItem(PropId + 3)->GetSubItem(2)->SetValue((_variant_t)skin->m_ForegroundImage->m_Border.right);
	pControl->GetSubItem(PropId + 3)->GetSubItem(3)->SetValue((_variant_t)skin->m_ForegroundImage->m_Border.bottom);
}

void CPropertiesWnd::UpdatePropertiesButton(CMFCPropertyGridProperty * pControl, my::Button * button)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyButtonPressed)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesButton(pControl, button);
		return;
	}
	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)(VARIANT_BOOL)button->m_bPressed);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)(VARIANT_BOOL)button->GetMouseOver());

	my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
	pControl->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)skin->m_PressedOffset.x);
	pControl->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)skin->m_PressedOffset.y);

	pControl->GetSubItem(PropId + 3)->SetValue((_variant_t)theApp.GetFullPath(skin->m_DisabledImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 4)->GetSubItem(0)->SetValue((_variant_t)skin->m_DisabledImage->m_Rect.left);
	pControl->GetSubItem(PropId + 4)->GetSubItem(1)->SetValue((_variant_t)skin->m_DisabledImage->m_Rect.top);
	pControl->GetSubItem(PropId + 4)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_DisabledImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 4)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_DisabledImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 5)->GetSubItem(0)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.left);
	pControl->GetSubItem(PropId + 5)->GetSubItem(1)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.top);
	pControl->GetSubItem(PropId + 5)->GetSubItem(2)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.right);
	pControl->GetSubItem(PropId + 5)->GetSubItem(3)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.bottom);

	pControl->GetSubItem(PropId + 6)->SetValue((_variant_t)theApp.GetFullPath(skin->m_PressedImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 7)->GetSubItem(0)->SetValue((_variant_t)skin->m_PressedImage->m_Rect.left);
	pControl->GetSubItem(PropId + 7)->GetSubItem(1)->SetValue((_variant_t)skin->m_PressedImage->m_Rect.top);
	pControl->GetSubItem(PropId + 7)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_PressedImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 7)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_PressedImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 8)->GetSubItem(0)->SetValue((_variant_t)skin->m_PressedImage->m_Border.left);
	pControl->GetSubItem(PropId + 8)->GetSubItem(1)->SetValue((_variant_t)skin->m_PressedImage->m_Border.top);
	pControl->GetSubItem(PropId + 8)->GetSubItem(2)->SetValue((_variant_t)skin->m_PressedImage->m_Border.right);
	pControl->GetSubItem(PropId + 8)->GetSubItem(3)->SetValue((_variant_t)skin->m_PressedImage->m_Border.bottom);

	pControl->GetSubItem(PropId + 9)->SetValue((_variant_t)theApp.GetFullPath(skin->m_MouseOverImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 10)->GetSubItem(0)->SetValue((_variant_t)skin->m_MouseOverImage->m_Rect.left);
	pControl->GetSubItem(PropId + 10)->GetSubItem(1)->SetValue((_variant_t)skin->m_MouseOverImage->m_Rect.top);
	pControl->GetSubItem(PropId + 10)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_MouseOverImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 10)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_MouseOverImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 11)->GetSubItem(0)->SetValue((_variant_t)skin->m_MouseOverImage->m_Border.left);
	pControl->GetSubItem(PropId + 11)->GetSubItem(1)->SetValue((_variant_t)skin->m_MouseOverImage->m_Border.top);
	pControl->GetSubItem(PropId + 11)->GetSubItem(2)->SetValue((_variant_t)skin->m_MouseOverImage->m_Border.right);
	pControl->GetSubItem(PropId + 11)->GetSubItem(3)->SetValue((_variant_t)skin->m_MouseOverImage->m_Border.bottom);

	switch (button->GetControlType())
	{
	case my::Control::ControlTypeCheckBox:
		UpdatePropertiesCheckBox(pControl, dynamic_cast<my::CheckBox*>(button));
		break;
	case my::Control::ControlTypeComboBox:
		UpdatePropertiesComboBox(pControl, dynamic_cast<my::ComboBox*>(button));
		break;
	default:
		RemovePropertiesFrom(pControl, GetControlPropCount(my::Control::ControlTypeButton));
		break;
	}
}

void CPropertiesWnd::UpdatePropertiesEditBox(CMFCPropertyGridProperty * pControl, my::EditBox * editbox)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyEditBoxBorder)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesEditBox(pControl, editbox);
		return;
	}
	pControl->GetSubItem(PropId + 0)->GetSubItem(0)->SetValue((_variant_t)editbox->m_Border.x);
	pControl->GetSubItem(PropId + 0)->GetSubItem(1)->SetValue((_variant_t)editbox->m_Border.y);
	pControl->GetSubItem(PropId + 0)->GetSubItem(2)->SetValue((_variant_t)editbox->m_Border.z);
	pControl->GetSubItem(PropId + 0)->GetSubItem(3)->SetValue((_variant_t)editbox->m_Border.w);

	my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(editbox->m_Skin);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)theApp.GetFullPath(skin->m_DisabledImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)skin->m_DisabledImage->m_Rect.left);
	pControl->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)skin->m_DisabledImage->m_Rect.top);
	pControl->GetSubItem(PropId + 2)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_DisabledImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 2)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_DisabledImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 3)->GetSubItem(0)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.left);
	pControl->GetSubItem(PropId + 3)->GetSubItem(1)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.top);
	pControl->GetSubItem(PropId + 3)->GetSubItem(2)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.right);
	pControl->GetSubItem(PropId + 3)->GetSubItem(3)->SetValue((_variant_t)skin->m_DisabledImage->m_Border.bottom);

	pControl->GetSubItem(PropId + 4)->SetValue((_variant_t)theApp.GetFullPath(skin->m_FocusedImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 5)->GetSubItem(0)->SetValue((_variant_t)skin->m_FocusedImage->m_Rect.left);
	pControl->GetSubItem(PropId + 5)->GetSubItem(1)->SetValue((_variant_t)skin->m_FocusedImage->m_Rect.top);
	pControl->GetSubItem(PropId + 5)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_FocusedImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 5)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_FocusedImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 6)->GetSubItem(0)->SetValue((_variant_t)skin->m_FocusedImage->m_Border.left);
	pControl->GetSubItem(PropId + 6)->GetSubItem(1)->SetValue((_variant_t)skin->m_FocusedImage->m_Border.top);
	pControl->GetSubItem(PropId + 6)->GetSubItem(2)->SetValue((_variant_t)skin->m_FocusedImage->m_Border.right);
	pControl->GetSubItem(PropId + 6)->GetSubItem(3)->SetValue((_variant_t)skin->m_FocusedImage->m_Border.bottom);

	COLORREF color = RGB(LOBYTE(skin->m_SelBkColor >> 16), LOBYTE(skin->m_SelBkColor >> 8), LOBYTE(skin->m_SelBkColor));
	(DYNAMIC_DOWNCAST(CColorProp, pControl->GetSubItem(PropId + 7)))->SetColor(color);
	pControl->GetSubItem(PropId + 8)->SetValue((_variant_t)(long)LOBYTE(skin->m_SelBkColor >> 24));

	color = RGB(LOBYTE(skin->m_CaretColor >> 16), LOBYTE(skin->m_CaretColor >> 8), LOBYTE(skin->m_CaretColor));
	(DYNAMIC_DOWNCAST(CColorProp, pControl->GetSubItem(PropId + 9)))->SetColor(color);
	pControl->GetSubItem(PropId + 10)->SetValue((_variant_t)(long)LOBYTE(skin->m_CaretColor >> 24));

	pControl->GetSubItem(PropId + 11)->SetValue((_variant_t)theApp.GetFullPath(skin->m_CaretImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 12)->GetSubItem(0)->SetValue((_variant_t)skin->m_CaretImage->m_Rect.left);
	pControl->GetSubItem(PropId + 12)->GetSubItem(1)->SetValue((_variant_t)skin->m_CaretImage->m_Rect.top);
	pControl->GetSubItem(PropId + 12)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_CaretImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 12)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_CaretImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 13)->GetSubItem(0)->SetValue((_variant_t)skin->m_CaretImage->m_Border.left);
	pControl->GetSubItem(PropId + 13)->GetSubItem(1)->SetValue((_variant_t)skin->m_CaretImage->m_Border.top);
	pControl->GetSubItem(PropId + 13)->GetSubItem(2)->SetValue((_variant_t)skin->m_CaretImage->m_Border.right);
	pControl->GetSubItem(PropId + 13)->GetSubItem(3)->SetValue((_variant_t)skin->m_CaretImage->m_Border.bottom);
}

void CPropertiesWnd::UpdatePropertiesScrollBar(CMFCPropertyGridProperty * pControl, my::ScrollBar * scrollbar)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyScrollBarUpDownButtonHeight)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesScrollBar(pControl, scrollbar);
		return;
	}

	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)scrollbar->m_UpDownButtonHeight);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)(long)scrollbar->m_nPosition);
	pControl->GetSubItem(PropId + 2)->SetValue((_variant_t)(long)scrollbar->m_nPageSize);
	pControl->GetSubItem(PropId + 3)->SetValue((_variant_t)(long)scrollbar->m_nStart);
	pControl->GetSubItem(PropId + 4)->SetValue((_variant_t)(long)scrollbar->m_nEnd);

	my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
	pControl->GetSubItem(PropId + 5)->GetSubItem(0)->SetValue((_variant_t)skin->m_PressedOffset.x);
	pControl->GetSubItem(PropId + 5)->GetSubItem(1)->SetValue((_variant_t)skin->m_PressedOffset.y);

	pControl->GetSubItem(PropId + 6)->SetValue((_variant_t)theApp.GetFullPath(skin->m_UpBtnNormalImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 7)->GetSubItem(0)->SetValue((_variant_t)skin->m_UpBtnNormalImage->m_Rect.left);
	pControl->GetSubItem(PropId + 7)->GetSubItem(1)->SetValue((_variant_t)skin->m_UpBtnNormalImage->m_Rect.top);
	pControl->GetSubItem(PropId + 7)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_UpBtnNormalImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 7)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_UpBtnNormalImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 8)->GetSubItem(0)->SetValue((_variant_t)skin->m_UpBtnNormalImage->m_Border.left);
	pControl->GetSubItem(PropId + 8)->GetSubItem(1)->SetValue((_variant_t)skin->m_UpBtnNormalImage->m_Border.top);
	pControl->GetSubItem(PropId + 8)->GetSubItem(2)->SetValue((_variant_t)skin->m_UpBtnNormalImage->m_Border.right);
	pControl->GetSubItem(PropId + 8)->GetSubItem(3)->SetValue((_variant_t)skin->m_UpBtnNormalImage->m_Border.bottom);

	pControl->GetSubItem(PropId + 9)->SetValue((_variant_t)theApp.GetFullPath(skin->m_UpBtnDisabledImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 10)->GetSubItem(0)->SetValue((_variant_t)skin->m_UpBtnDisabledImage->m_Rect.left);
	pControl->GetSubItem(PropId + 10)->GetSubItem(1)->SetValue((_variant_t)skin->m_UpBtnDisabledImage->m_Rect.top);
	pControl->GetSubItem(PropId + 10)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_UpBtnDisabledImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 10)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_UpBtnDisabledImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 11)->GetSubItem(1)->SetValue((_variant_t)skin->m_UpBtnDisabledImage->m_Border.top);
	pControl->GetSubItem(PropId + 11)->GetSubItem(2)->SetValue((_variant_t)skin->m_UpBtnDisabledImage->m_Border.right);
	pControl->GetSubItem(PropId + 11)->GetSubItem(3)->SetValue((_variant_t)skin->m_UpBtnDisabledImage->m_Border.bottom);
	pControl->GetSubItem(PropId + 11)->GetSubItem(0)->SetValue((_variant_t)skin->m_UpBtnDisabledImage->m_Border.left);

	pControl->GetSubItem(PropId + 12)->SetValue((_variant_t)theApp.GetFullPath(skin->m_DownBtnNormalImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 13)->GetSubItem(0)->SetValue((_variant_t)skin->m_DownBtnNormalImage->m_Rect.left);
	pControl->GetSubItem(PropId + 13)->GetSubItem(1)->SetValue((_variant_t)skin->m_DownBtnNormalImage->m_Rect.top);
	pControl->GetSubItem(PropId + 13)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_DownBtnNormalImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 13)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_DownBtnNormalImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 14)->GetSubItem(0)->SetValue((_variant_t)skin->m_DownBtnNormalImage->m_Border.left);
	pControl->GetSubItem(PropId + 14)->GetSubItem(1)->SetValue((_variant_t)skin->m_DownBtnNormalImage->m_Border.top);
	pControl->GetSubItem(PropId + 14)->GetSubItem(2)->SetValue((_variant_t)skin->m_DownBtnNormalImage->m_Border.right);
	pControl->GetSubItem(PropId + 14)->GetSubItem(3)->SetValue((_variant_t)skin->m_DownBtnNormalImage->m_Border.bottom);

	pControl->GetSubItem(PropId + 15)->SetValue((_variant_t)theApp.GetFullPath(skin->m_DownBtnDisabledImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 16)->GetSubItem(0)->SetValue((_variant_t)skin->m_DownBtnDisabledImage->m_Rect.left);
	pControl->GetSubItem(PropId + 16)->GetSubItem(1)->SetValue((_variant_t)skin->m_DownBtnDisabledImage->m_Rect.top);
	pControl->GetSubItem(PropId + 16)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_DownBtnDisabledImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 16)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_DownBtnDisabledImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 17)->GetSubItem(0)->SetValue((_variant_t)skin->m_DownBtnDisabledImage->m_Border.left);
	pControl->GetSubItem(PropId + 17)->GetSubItem(1)->SetValue((_variant_t)skin->m_DownBtnDisabledImage->m_Border.top);
	pControl->GetSubItem(PropId + 17)->GetSubItem(2)->SetValue((_variant_t)skin->m_DownBtnDisabledImage->m_Border.right);
	pControl->GetSubItem(PropId + 17)->GetSubItem(3)->SetValue((_variant_t)skin->m_DownBtnDisabledImage->m_Border.bottom);

	pControl->GetSubItem(PropId + 18)->SetValue((_variant_t)theApp.GetFullPath(skin->m_ThumbBtnNormalImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 19)->GetSubItem(0)->SetValue((_variant_t)skin->m_ThumbBtnNormalImage->m_Rect.left);
	pControl->GetSubItem(PropId + 19)->GetSubItem(1)->SetValue((_variant_t)skin->m_ThumbBtnNormalImage->m_Rect.top);
	pControl->GetSubItem(PropId + 19)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_ThumbBtnNormalImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 19)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_ThumbBtnNormalImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 20)->GetSubItem(1)->SetValue((_variant_t)skin->m_ThumbBtnNormalImage->m_Border.top);
	pControl->GetSubItem(PropId + 20)->GetSubItem(0)->SetValue((_variant_t)skin->m_ThumbBtnNormalImage->m_Border.left);
	pControl->GetSubItem(PropId + 20)->GetSubItem(2)->SetValue((_variant_t)skin->m_ThumbBtnNormalImage->m_Border.right);
	pControl->GetSubItem(PropId + 20)->GetSubItem(3)->SetValue((_variant_t)skin->m_ThumbBtnNormalImage->m_Border.bottom);
}

void CPropertiesWnd::UpdatePropertiesCheckBox(CMFCPropertyGridProperty * pControl, my::CheckBox * checkbox)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeButton);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyCheckBoxChecked)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesCheckBox(pControl, checkbox);
		return;
	}
	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)(VARIANT_BOOL)checkbox->m_Checked);
}

void CPropertiesWnd::UpdatePropertiesComboBox(CMFCPropertyGridProperty * pControl, my::ComboBox * combobox)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeButton);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyComboBoxDropdownSize)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesComboBox(pControl, combobox);
		return;
	}
	pControl->GetSubItem(PropId + 0)->GetSubItem(0)->SetValue((_variant_t)combobox->m_DropdownSize.x);
	pControl->GetSubItem(PropId + 0)->GetSubItem(1)->SetValue((_variant_t)combobox->m_DropdownSize.y);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)combobox->m_ScrollbarWidth);
	pControl->GetSubItem(PropId + 2)->SetValue((_variant_t)combobox->m_ScrollbarUpDownBtnHeight);
	pControl->GetSubItem(PropId + 3)->GetSubItem(0)->SetValue((_variant_t)combobox->m_Border.x);
	pControl->GetSubItem(PropId + 3)->GetSubItem(1)->SetValue((_variant_t)combobox->m_Border.y);
	pControl->GetSubItem(PropId + 3)->GetSubItem(2)->SetValue((_variant_t)combobox->m_Border.z);
	pControl->GetSubItem(PropId + 3)->GetSubItem(3)->SetValue((_variant_t)combobox->m_Border.w);
	pControl->GetSubItem(PropId + 4)->SetValue((_variant_t)combobox->m_ItemHeight);
	pControl->GetSubItem(PropId + 5)->SetValue((_variant_t)(long)combobox->m_Items.size());

	my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
	pControl->GetSubItem(PropId + 6)->SetValue((_variant_t)theApp.GetFullPath(skin->m_DropdownImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 7)->GetSubItem(0)->SetValue((_variant_t)skin->m_DropdownImage->m_Rect.left);
	pControl->GetSubItem(PropId + 7)->GetSubItem(1)->SetValue((_variant_t)skin->m_DropdownImage->m_Rect.top);
	pControl->GetSubItem(PropId + 7)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_DropdownImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 7)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_DropdownImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 8)->GetSubItem(0)->SetValue((_variant_t)skin->m_DropdownImage->m_Border.left);
	pControl->GetSubItem(PropId + 8)->GetSubItem(1)->SetValue((_variant_t)skin->m_DropdownImage->m_Border.top);
	pControl->GetSubItem(PropId + 8)->GetSubItem(2)->SetValue((_variant_t)skin->m_DropdownImage->m_Border.right);
	pControl->GetSubItem(PropId + 8)->GetSubItem(3)->SetValue((_variant_t)skin->m_DropdownImage->m_Border.bottom);

	COLORREF color = RGB(LOBYTE(skin->m_DropdownItemTextColor >> 16), LOBYTE(skin->m_DropdownItemTextColor >> 8), LOBYTE(skin->m_DropdownItemTextColor));
	(DYNAMIC_DOWNCAST(CColorProp, pControl->GetSubItem(PropId + 9)))->SetColor(color);
	pControl->GetSubItem(PropId + 10)->SetValue((_variant_t)(long)LOBYTE(skin->m_DropdownItemTextColor >> 24));
	pControl->GetSubItem(PropId + 11)->SetValue(GetFontAlignDesc(skin->m_DropdownItemTextAlign));

	pControl->GetSubItem(PropId + 12)->SetValue((_variant_t)theApp.GetFullPath(skin->m_DropdownItemMouseOverImage->m_TexturePath.c_str()).c_str());
	pControl->GetSubItem(PropId + 13)->GetSubItem(0)->SetValue((_variant_t)skin->m_DropdownItemMouseOverImage->m_Rect.left);
	pControl->GetSubItem(PropId + 13)->GetSubItem(1)->SetValue((_variant_t)skin->m_DropdownItemMouseOverImage->m_Rect.top);
	pControl->GetSubItem(PropId + 13)->GetSubItem(2)->SetValue((_variant_t)(long)skin->m_DropdownItemMouseOverImage->m_Rect.Width());
	pControl->GetSubItem(PropId + 13)->GetSubItem(3)->SetValue((_variant_t)(long)skin->m_DropdownItemMouseOverImage->m_Rect.Height());
	pControl->GetSubItem(PropId + 14)->GetSubItem(0)->SetValue((_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.left);
	pControl->GetSubItem(PropId + 14)->GetSubItem(1)->SetValue((_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.top);
	pControl->GetSubItem(PropId + 14)->GetSubItem(2)->SetValue((_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.right);
	pControl->GetSubItem(PropId + 14)->GetSubItem(3)->SetValue((_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.bottom);
}

void CPropertiesWnd::UpdatePropertiesListBox(CMFCPropertyGridProperty * pControl, my::ListBox * listbox)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyListBoxScrollbarWidth)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesListBox(pControl, listbox);
		return;
	}

	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)listbox->m_ScrollbarWidth);
	pControl->GetSubItem(PropId + 1)->SetValue((_variant_t)listbox->m_ScrollbarUpDownBtnHeight);
	pControl->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)listbox->m_ItemSize.x);
	pControl->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)listbox->m_ItemSize.y);
	pControl->GetSubItem(PropId + 3)->SetValue((_variant_t)(long)listbox->m_Childs.size());

	my::ListBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ListBoxSkin>(listbox->m_Skin);
}

void CPropertiesWnd::UpdatePropertiesDialog(CMFCPropertyGridProperty * pControl, my::Dialog * dialog)
{
	unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
	if (pControl->GetSubItemsCount() <= PropId || pControl->GetSubItem(PropId)->GetData() != PropertyDialogEnableDrag)
	{
		RemovePropertiesFrom(pControl, PropId);
		CreatePropertiesDialog(pControl, dialog);
		return;
	}

	my::Vector3 Pos, Scale; my::Quaternion Rot;
	dialog->m_World.Decompose(Scale, Rot, Pos);
	pControl->GetSubItem(PropId + 0)->SetValue((_variant_t)(VARIANT_BOOL)dialog->m_EnableDrag);
	pControl->GetSubItem(PropId + 1)->GetSubItem(0)->SetValue((_variant_t)Pos.x);
	pControl->GetSubItem(PropId + 1)->GetSubItem(1)->SetValue((_variant_t)Pos.y);
	pControl->GetSubItem(PropId + 1)->GetSubItem(2)->SetValue((_variant_t)Pos.z);
	my::Vector3 angle = Rot.toEulerAngles();
	pControl->GetSubItem(PropId + 2)->GetSubItem(0)->SetValue((_variant_t)D3DXToDegree(angle.x));
	pControl->GetSubItem(PropId + 2)->GetSubItem(1)->SetValue((_variant_t)D3DXToDegree(angle.y));
	pControl->GetSubItem(PropId + 2)->GetSubItem(2)->SetValue((_variant_t)D3DXToDegree(angle.z));
	pControl->GetSubItem(PropId + 3)->GetSubItem(0)->SetValue((_variant_t)Scale.x);
	pControl->GetSubItem(PropId + 3)->GetSubItem(1)->SetValue((_variant_t)Scale.y);
	pControl->GetSubItem(PropId + 3)->GetSubItem(2)->SetValue((_variant_t)Scale.z);
}

void CPropertiesWnd::CreatePropertiesActor(Actor * actor)
{
	CMFCPropertyGridProperty * pActor = new CSimpleProp(_T("Actor"), PropertyActor, FALSE);
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

	CMFCPropertyGridProperty * pPosition = new CSimpleProp(_T("Position"), PropertyActorPos, TRUE);
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
	pProp = new CSimpleProp(_T("x, y, z"), (_variant_t)actor->m_Scale.x, NULL, PropertyActorScaleX);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)actor->m_Scale.y, NULL, PropertyActorScaleY);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)actor->m_Scale.z, NULL, PropertyActorScaleZ);
	pScale->AddSubItem(pProp);

	CMFCPropertyGridProperty * pLodDist = new CSimpleProp(_T("LodDist"), (_variant_t)actor->m_LodDist, NULL, PropertyActorLodDist);
	pActor->AddSubItem(pLodDist);
	CMFCPropertyGridProperty * pLodFactor = new CSimpleProp(_T("LodFactor"), (_variant_t)actor->m_LodFactor, NULL, PropertyActorLodFactor);
	pActor->AddSubItem(pLodFactor);
	CMFCPropertyGridProperty * pCullingDistSq = new CSimpleProp(_T("CullingDistSq"), (_variant_t)actor->m_CullingDistSq, NULL, PropertyActorCullingDistSq);
	pActor->AddSubItem(pCullingDistSq);

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
	CString strTitle;
	strTitle.Format(_T("%s: %S"), GetComponentTypeName(cmp->GetComponentType()), cmp->GetName());
	CMFCPropertyGridProperty * pComponent = new CSimpleProp(strTitle, PropertyComponent, FALSE);
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

	CMFCPropertyGridProperty* pSiblingId = new CSimpleProp(_T("SiblingId"), (_variant_t)cmp->GetSiblingId(), NULL, PropertyComponentSiblingId);
	pComponent->AddSubItem(pSiblingId);

	CreatePropertiesShape(pComponent, cmp);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	switch (cmp->GetComponentType())
	{
	case Component::ComponentTypeMesh:
		CreatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeStaticMesh:
		CreatePropertiesStaticMesh(pComponent, dynamic_cast<StaticMesh *>(cmp));
		break;
	case Component::ComponentTypeCloth:
		CreatePropertiesCloth(pComponent, dynamic_cast<ClothComponent *>(cmp));
		break;
	case Component::ComponentTypeStaticEmitter:
	case Component::ComponentTypeSphericalEmitter:
		CreatePropertiesEmitter(pComponent, dynamic_cast<EmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		CreatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	case Component::ComponentTypeAnimator:
		CreatePropertiesAnimator(pComponent, dynamic_cast<Animator *>(cmp));
		break;
	case Component::ComponentTypeNavigation:
		CreatePropertiesNavigation(pComponent, dynamic_cast<Navigation *>(cmp));
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

	CMFCPropertyGridProperty * pProp = new CFileProp(_T("MeshPath"), TRUE, (_variant_t)ms2ts(mesh_cmp->m_MeshPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	COLORREF color = RGB(mesh_cmp->m_MeshColor.x * 255, mesh_cmp->m_MeshColor.y * 255, mesh_cmp->m_MeshColor.z * 255);
	CColorProp* pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyMeshColor);
	pColor->EnableOtherButton(_T("Other..."));
	pComponent->AddSubItem(pColor);

	CMFCPropertyGridProperty* pAlpha = new CSliderProp(_T("Alpha"), (long)(mesh_cmp->m_MeshColor.w * 255), NULL, PropertyMeshAlpha);
	pComponent->AddSubItem(pAlpha);

	pProp = new CSimpleProp(_T("MeshSubMeshId"), (_variant_t)mesh_cmp->m_MeshSubMeshId, NULL, PropertyMeshSubMeshId);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("NumVert"), (_variant_t)(mesh_cmp->m_Mesh ? mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].VertexCount : 0), NULL, PropertyMeshNumVert);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("NumFace"), (_variant_t)(mesh_cmp->m_Mesh ? mesh_cmp->m_Mesh->m_AttribTable[mesh_cmp->m_MeshSubMeshId].FaceCount : 0), NULL, PropertyMeshNumFace);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	CComboProp * pInstance = new CComboProp(_T("InstanceType"), (_variant_t)g_InstanceTypeDesc[mesh_cmp->m_InstanceType], NULL, PropertyMeshInstanceType);
	for (unsigned int i = 0; i < _countof(g_InstanceTypeDesc); i++)
	{
		pInstance->AddOption(g_InstanceTypeDesc[i], TRUE);
	}
	pComponent->AddSubItem(pInstance);
	CreatePropertiesMaterial(pComponent, _T("Material"), mesh_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesMaterial(CMFCPropertyGridProperty * pParentCtrl, LPCTSTR lpszName, Material * mtl)
{
	CMFCPropertyGridProperty * pMaterial = new CSimpleProp(lpszName, PropertyMaterial, FALSE);
	pParentCtrl->AddSubItem(pMaterial);
	pMaterial->SetValue((_variant_t)(DWORD_PTR)mtl);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("Shader"), TRUE, (_variant_t)theApp.GetFullPath(mtl->m_Shader.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialShader);
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
	CComboProp* pZFunc = new CComboProp(_T("ZFunc"), (_variant_t)g_ZFuncDesc[mtl->m_ZFunc - 1], NULL, PropertyMaterialZFunc);
	for (unsigned int i = 0; i < _countof(g_ZFuncDesc); i++)
	{
		pZFunc->AddOption(g_ZFuncDesc[i], TRUE);
	}
	pMaterial->AddSubItem(pZFunc);
	CCheckBoxProp* pAlphaTestEnable = new CCheckBoxProp(_T("AlphaTestEnable"), mtl->m_AlphaTestEnable, NULL, PropertyMaterialAlphaTestEnable);
	pMaterial->AddSubItem(pAlphaTestEnable);
	CMFCPropertyGridProperty* pAlphaRef = new CSimpleProp(_T("AlphaRef"), (_variant_t)mtl->m_AlphaRef, NULL, PropertyMaterialAlphaRef);
	pMaterial->AddSubItem(pAlphaRef);
	CComboProp* pAlphaFunc = new CComboProp(_T("AlphaFunc"), (_variant_t)g_ZFuncDesc[mtl->m_AlphaFunc - 1], NULL, PropertyMaterialAlphaFunc);
	for (unsigned int i = 0; i < _countof(g_ZFuncDesc); i++)
	{
		pAlphaFunc->AddOption(g_ZFuncDesc[i], TRUE);
	}
	pMaterial->AddSubItem(pAlphaFunc);
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
	switch (mtl_param->GetParameterType())
	{
	case MaterialParameter::ParameterTypeInt2:
	{
		const CPoint& Value = dynamic_cast<MaterialParameterInt2*>(mtl_param)->m_Value;
		CMFCPropertyGridProperty* pParameter = new CSimpleProp(name.c_str(), PropertyMaterialParameterInt2, TRUE);
		pParentCtrl->AddSubItem(pParameter);
		pProp = new CSimpleProp(_T("x"), (_variant_t)Value.x, NULL, PropertyMaterialParameterIntValueX);
		pParameter->AddSubItem(pProp);
		pProp = new CSimpleProp(_T("y"), (_variant_t)Value.y, NULL, PropertyMaterialParameterIntValueY);
		pParameter->AddSubItem(pProp);
		break;
	}
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
		if (boost::regex_search(mtl_param->m_Name, g_rcolor, boost::match_default))
		{
			COLORREF color = RGB(Value.x * 255, Value.y * 255, Value.z * 255);
			CColorProp* pColor = new CColorProp(name.c_str(), color, NULL, NULL, PropertyMaterialParameterColor);
			pColor->EnableOtherButton(_T("Other..."));
			pParentCtrl->AddSubItem(pColor);
		}
		else
		{
			CMFCPropertyGridProperty* pParameter = new CSimpleProp(name.c_str(), PropertyMaterialParameterFloat3, TRUE);
			pParentCtrl->AddSubItem(pParameter);
			pProp = new CSimpleProp(_T("x"), (_variant_t)Value.x, NULL, PropertyMaterialParameterFloatValueX);
			pParameter->AddSubItem(pProp);
			pProp = new CSimpleProp(_T("y"), (_variant_t)Value.y, NULL, PropertyMaterialParameterFloatValueY);
			pParameter->AddSubItem(pProp);
			pProp = new CSimpleProp(_T("z"), (_variant_t)Value.z, NULL, PropertyMaterialParameterFloatValueZ);
			pParameter->AddSubItem(pProp);
		}
		break;
	}
	case MaterialParameter::ParameterTypeFloat4:
	{
		const my::Vector4 & Value = dynamic_cast<MaterialParameterFloat4*>(mtl_param)->m_Value;
		CMFCPropertyGridProperty* pParameter = new CSimpleProp(name.c_str(), PropertyMaterialParameterFloat4, TRUE);
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
		pProp = new CFileProp(name.c_str(), TRUE, (_variant_t)theApp.GetFullPath(dynamic_cast<MaterialParameterTexture *>(mtl_param)->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMaterialParameterTexture);
		pParentCtrl->AddSubItem(pProp);
		break;
	}
}

void CPropertiesWnd::CreatePropertiesStaticMesh(CMFCPropertyGridProperty* pComponent, StaticMesh* static_mesh_cmp)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CMFCPropertyGridProperty* pChunkWidth = new CSimpleProp(_T("ChunkWidth"), (_variant_t)static_mesh_cmp->m_ChunkWidth, NULL, PropertyStaticMeshChunkWidth);
	pChunkWidth->Enable(FALSE);
	pComponent->AddSubItem(pChunkWidth);
	CMFCPropertyGridProperty* pChunkPath = new CSimpleProp(_T("ChunkPath"), (_variant_t)ms2ts(static_mesh_cmp->m_ChunkPath.c_str()).c_str(), NULL, PropertyStaticMeshChunkPath);
	pChunkPath->Enable(FALSE);
	pComponent->AddSubItem(pChunkPath);
	CMFCPropertyGridProperty* pChunkLodScale = new CSimpleProp(_T("ChunkLodScale"), (_variant_t)static_mesh_cmp->m_ChunkLodScale, NULL, PropertyStaticMeshChunkLodScale);
	pComponent->AddSubItem(pChunkLodScale);
	CreatePropertiesMaterial(pComponent, _T("Material"), static_mesh_cmp->m_Material.get());
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
	CMFCPropertyGridProperty* pProp = new CCheckBoxProp(_T("SweptContact"), (_variant_t)flags.isSet(physx::PxClothFlag::eSWEPT_CONTACT), NULL, PropertyClothSweptContact);
	pComponent->AddSubItem(pProp);

	pProp = new CCheckBoxProp(_T("SceneCollision"), (_variant_t)flags.isSet(physx::PxClothFlag::eSCENE_COLLISION), NULL, PropertyClothSceneCollision);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("SolverFrequency"), (_variant_t)cloth_cmp->m_Cloth->getSolverFrequency(), NULL, PropertyClothSolverFrequency);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("StiffnessFrequency"), (_variant_t)cloth_cmp->m_Cloth->getStiffnessFrequency(), NULL, PropertyClothStiffnessFrequency);
	pComponent->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("CollisionSpheresNum"), (_variant_t)cloth_cmp->m_Cloth->getNbCollisionSpheres(), NULL, PropertyClothCollisionSpheresNum);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("CollisionCapsulesNum"), (_variant_t)cloth_cmp->m_Cloth->getNbCollisionCapsules(), NULL, PropertyClothCollisionCapsulesNum);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	CComboProp* pVirtualParticleLevel = new CComboProp(_T("VirtualParticleLevel"), _T(""), NULL, PropertyClothVirtualParticleLevel);
	for (unsigned int i = 0; i < 6; i++)
	{
		pVirtualParticleLevel->AddOption(g_VirtualParticleLevel[i], TRUE);
	}
	pComponent->AddSubItem(pVirtualParticleLevel);
	pProp = new CSimpleProp(_T("VirtualParticleNum"), (_variant_t)cloth_cmp->m_Cloth->getNbVirtualParticles(), NULL, PropertyClothVirtualParticleNum);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	physx::PxClothStretchConfig stretchConfig = cloth_cmp->m_Cloth->getStretchConfig(physx::PxClothFabricPhaseType::eVERTICAL);
	CMFCPropertyGridProperty* pStretchVertical = new CSimpleProp(_T("StretchVertical"), PropertyClothStretchVertical, TRUE);
	pComponent->AddSubItem(pStretchVertical);
	pProp = new CSimpleProp(_T("Stiffness"), (_variant_t)stretchConfig.stiffness, NULL, PropertyClothStretchVerticalStiffness);
	pStretchVertical->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("StiffnessMultiplier"), (_variant_t)stretchConfig.stiffnessMultiplier, NULL, PropertyClothStretchVerticalStiffnessMultiplier);
	pStretchVertical->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("CompressionLimit"), (_variant_t)stretchConfig.compressionLimit, NULL, PropertyClothStretchVerticalCompressionLimit);
	pStretchVertical->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("StretchLimit"), (_variant_t)stretchConfig.stretchLimit, NULL, PropertyClothStretchVerticalStretchLimit);
	pStretchVertical->AddSubItem(pProp);

	physx::PxClothTetherConfig tetherConfig = cloth_cmp->m_Cloth->getTetherConfig();
	CMFCPropertyGridProperty* pTether = new CSimpleProp(_T("Tether"), PropertyClothTether, TRUE);
	pComponent->AddSubItem(pTether);
	pProp = new CSimpleProp(_T("Stiffness"), (_variant_t)tetherConfig.stiffness, NULL, PropertyClothTetherStiffness);
	pTether->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("StretchLimit"), (_variant_t)tetherConfig.stretchLimit, NULL, PropertyClothTetherStretchLimit);
	pTether->AddSubItem(pProp);

	my::Vector3 acceleration = cloth_cmp->GetExternalAcceleration();
	CMFCPropertyGridProperty* pExternalAcceleration = new CSimpleProp(_T("ExternalAcceleration"), PropertyClothExternalAcceleration, TRUE);
	pComponent->AddSubItem(pExternalAcceleration);
	pProp = new CSimpleProp(_T("x"), (_variant_t)acceleration.x, NULL, PropertyClothExternalAccelerationX);
	pExternalAcceleration->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)acceleration.y, NULL, PropertyClothExternalAccelerationY);
	pExternalAcceleration->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)acceleration.z, NULL, PropertyClothExternalAccelerationZ);
	pExternalAcceleration->AddSubItem(pProp);

	CreatePropertiesMaterial(pComponent, _T("Material"), cloth_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
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
	CMFCPropertyGridProperty* pTiles = new CSimpleProp(_T("Tiles"), PropertyEmitterTiles, TRUE);
	pComponent->AddSubItem(pTiles);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)emit_cmp->m_Tiles.x, NULL, PropertyEmitterTilesX);
	pTiles->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)emit_cmp->m_Tiles.y, NULL, PropertyEmitterTilesY);
	pTiles->AddSubItem(pProp);
	CComboProp * pEmitterPrimitiveType = new CComboProp(_T("PrimitiveType"), (_variant_t)g_EmitterPrimitiveType[emit_cmp->m_ParticlePrimitiveType], NULL, PropertyEmitterPrimitiveType);
	for (unsigned int i = 0; i < _countof(g_EmitterPrimitiveType); i++)
	{
		pEmitterPrimitiveType->AddOption(g_EmitterPrimitiveType[i], TRUE);
	}
	pComponent->AddSubItem(pEmitterPrimitiveType);
	pProp = new CFileProp(_T("MeshPath"), TRUE, (_variant_t)ms2ts(emit_cmp->m_MeshPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyEmitterMeshPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	CMFCPropertyGridProperty* pMeshSubMeshId = new CSimpleProp(_T("MeshSubMeshId"), (_variant_t)emit_cmp->m_MeshSubMeshId, NULL, PropertyEmitterMeshSubMeshId);
	pMeshSubMeshId->Enable(FALSE);
	pComponent->AddSubItem(pMeshSubMeshId);
	pProp = new CSimpleProp(_T("NumVert"), (_variant_t)(emit_cmp->m_Mesh ? emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].VertexCount : 0), NULL, PropertyEmitterMeshNumVert);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("NumFace"), (_variant_t)(emit_cmp->m_Mesh ? emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].FaceCount : 0), NULL, PropertyEmitterMeshNumFace);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	switch (emit_cmp->GetComponentType())
	{
	case Component::ComponentTypeStaticEmitter:
		CreatePropertiesStaticEmitter(pComponent, dynamic_cast<StaticEmitter*>(emit_cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		CreatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitter*>(emit_cmp));
		break;
	}
}

void CPropertiesWnd::CreatePropertiesStaticEmitter(CMFCPropertyGridProperty * pComponent, StaticEmitter * emit_cmp)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeEmitter));

	CMFCPropertyGridProperty * pChunkWidth = new CSimpleProp(_T("ChunkWidth"), (_variant_t)emit_cmp->m_ChunkWidth, NULL, PropertyStaticEmitterChunkWidth);
	pChunkWidth->Enable(FALSE);
	pComponent->AddSubItem(pChunkWidth);
	CMFCPropertyGridProperty * pChunkPath = new CSimpleProp(_T("ChunkPath"), (_variant_t)ms2ts(emit_cmp->m_ChunkPath.c_str()).c_str(), NULL, PropertyStaticEmitterChunkPath);
	pChunkPath->Enable(FALSE);
	pComponent->AddSubItem(pChunkPath);
	CMFCPropertyGridProperty* pChunkLodScale = new CSimpleProp(_T("ChunkLodScale"), (_variant_t)emit_cmp->m_ChunkLodScale, NULL, PropertyStaticEmitterChunkLodScale);
	pComponent->AddSubItem(pChunkLodScale);
	CreatePropertiesMaterial(pComponent, _T("Material"), emit_cmp->m_Material.get());
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	StaticEmitter::ChunkMap::const_iterator chunk_iter = (pFrame->m_selcmp == emit_cmp ? emit_cmp->m_Chunks.find(std::make_pair(pFrame->m_selchunkid.x, pFrame->m_selchunkid.y)) : emit_cmp->m_Chunks.begin());
	if (chunk_iter != emit_cmp->m_Chunks.end() && chunk_iter->second.m_buff && pFrame->m_selinstid < chunk_iter->second.m_buff->size())
	{
		CreatePropertiesStaticEmitterParticle(pComponent, CPoint(chunk_iter->first.first, chunk_iter->first.second), pFrame->m_selinstid, &(*chunk_iter->second.m_buff)[pFrame->m_selinstid]);
	}
	else
	{
		CMFCPropertyGridProperty * pParticle = new CSimpleProp(_T("Particle"), 0, FALSE);
		pComponent->AddSubItem(pParticle);
	}
}

void CPropertiesWnd::CreatePropertiesStaticEmitterParticle(CMFCPropertyGridProperty * pParentProp, const CPoint & chunkid, int instid, my::Emitter::Particle * particle)
{
	CString strTitle;
	strTitle.Format(_T("Particle_%d_%d_%d"), chunkid.x, chunkid.y, instid);
	CMFCPropertyGridProperty * pParticle = new CSimpleProp(strTitle, MAKELONG(chunkid.x, chunkid.y), FALSE);
	pParticle->SetValue((_variant_t)instid);
	pParentProp->AddSubItem(pParticle);
	CMFCPropertyGridProperty * pPosition = new CSimpleProp(_T("Position"), PropertyEmitterParticlePosition, TRUE);
	pParticle->AddSubItem(pPosition);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)particle->m_Position.x, NULL, PropertyEmitterParticlePositionX);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)particle->m_Position.y, NULL, PropertyEmitterParticlePositionY);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)particle->m_Position.z, NULL, PropertyEmitterParticlePositionZ);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)particle->m_Position.w, NULL, PropertyEmitterParticlePositionW);
	pPosition->AddSubItem(pProp);

	CMFCPropertyGridProperty * pVelocity = new CSimpleProp(_T("Velocity"), PropertyEmitterParticleVelocity, TRUE);
	pParticle->AddSubItem(pVelocity);
	pProp = new CSimpleProp(_T("x"), (_variant_t)particle->m_Velocity.x, NULL, PropertyEmitterParticleVelocityX);
	pVelocity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)particle->m_Velocity.y, NULL, PropertyEmitterParticleVelocityY);
	pVelocity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)particle->m_Velocity.z, NULL, PropertyEmitterParticleVelocityZ);
	pVelocity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)particle->m_Velocity.w, NULL, PropertyEmitterParticleVelocityW);
	pVelocity->AddSubItem(pProp);

	COLORREF color = RGB(particle->m_Color.x * 255, particle->m_Color.y * 255, particle->m_Color.z * 255);
	CColorProp * pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyEmitterParticleColor);
	pColor->EnableOtherButton(_T("Other..."));
	pParticle->AddSubItem(pColor);

	CMFCPropertyGridProperty * pAlpha = new CSliderProp(_T("Alpha"), (long)(particle->m_Color.w * 255), NULL, PropertyEmitterParticleColorAlpha);
	pParticle->AddSubItem(pAlpha);

	CMFCPropertyGridProperty * pSize = new CSimpleProp(_T("Size"), PropertyEmitterParticleSize, TRUE);
	pParticle->AddSubItem(pSize);
	pProp = new CSimpleProp(_T("x"), (_variant_t)particle->m_Size.x, NULL, PropertyEmitterParticleSizeX);
	pSize->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)particle->m_Size.y, NULL, PropertyEmitterParticleSizeY);
	pSize->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("Angle"), (_variant_t)D3DXToDegree(particle->m_Angle), NULL, PropertyEmitterParticleAngle);
	pParticle->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitter * sphe_emit_cmp)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeEmitter));

	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("ParticleCapacity"), (_variant_t)(unsigned int)sphe_emit_cmp->m_ParticleList.capacity(), NULL, PropertySphericalEmitterParticleCapacity);
	pComponent->AddSubItem(pProp);
	CMFCPropertyGridProperty * pSpawnInterval = new CSimpleProp(_T("SpawnInterval"), (_variant_t)sphe_emit_cmp->m_SpawnInterval, NULL, PropertySphericalEmitterSpawnInterval);
	pComponent->AddSubItem(pSpawnInterval);
	CMFCPropertyGridProperty* pSpawnCount = new CSimpleProp(_T("SpawnCount"), (_variant_t)sphe_emit_cmp->m_SpawnCount, NULL, PropertySphericalEmitterSpawnCount);
	pComponent->AddSubItem(pSpawnCount);
	CMFCPropertyGridProperty * pHalfSpawnArea = new CSimpleProp(_T("HalfSpawnArea"), PropertySphericalEmitterHalfSpawnArea, TRUE);
	pComponent->AddSubItem(pHalfSpawnArea);
	pProp = new CSimpleProp(_T("x"), (_variant_t)sphe_emit_cmp->m_HalfSpawnArea.x, NULL, PropertySphericalEmitterHalfSpawnAreaX);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)sphe_emit_cmp->m_HalfSpawnArea.y, NULL, PropertySphericalEmitterHalfSpawnAreaY);
	pHalfSpawnArea->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)sphe_emit_cmp->m_HalfSpawnArea.z, NULL, PropertySphericalEmitterHalfSpawnAreaZ);
	pHalfSpawnArea->AddSubItem(pProp);
	CMFCPropertyGridProperty* pSpawnInclination = new CSimpleProp(_T("SpawnInclination"), PropertySphericalEmitterSpawnInclination, TRUE);
	pComponent->AddSubItem(pSpawnInclination);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnInclination.x), NULL, PropertySphericalEmitterSpawnInclinationX);
	pSpawnInclination->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnInclination.y), NULL, PropertySphericalEmitterSpawnInclinationY);
	pSpawnInclination->AddSubItem(pProp);
	CMFCPropertyGridProperty* pSpawnAzimuth = new CSimpleProp(_T("SpawnAzimuth"), PropertySphericalEmitterSpawnAzimuth, TRUE);
	pComponent->AddSubItem(pSpawnAzimuth);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnAzimuth.x), NULL, PropertySphericalEmitterSpawnAzimuthX);
	pSpawnAzimuth->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(sphe_emit_cmp->m_SpawnAzimuth.y), NULL, PropertySphericalEmitterSpawnAzimuthY);
	pSpawnAzimuth->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("SpawnSpeed"), (_variant_t)sphe_emit_cmp->m_SpawnSpeed, NULL, PropertySphericalEmitterSpawnSpeed);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("BoneId"), (_variant_t)sphe_emit_cmp->m_SpawnBoneId, NULL, PropertySphericalEmitterSpawnBoneId);
	pComponent->AddSubItem(pProp);
	CMFCPropertyGridProperty* pSpawnLocalPos = new CSimpleProp(_T("SpawnLocalPos"), PropertySphericalEmitterSpawnLocalPos, TRUE);
	pComponent->AddSubItem(pSpawnLocalPos);
	pProp = new CSimpleProp(_T("x"), (_variant_t)sphe_emit_cmp->m_SpawnLocalPose.m_position.x, NULL, PropertySphericalEmitterSpawnLocalPosX);
	pSpawnLocalPos->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)sphe_emit_cmp->m_SpawnLocalPose.m_position.y, NULL, PropertySphericalEmitterSpawnLocalPosY);
	pSpawnLocalPos->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)sphe_emit_cmp->m_SpawnLocalPose.m_position.z, NULL, PropertySphericalEmitterSpawnLocalPosZ);
	pSpawnLocalPos->AddSubItem(pProp);
	my::Vector3 angle = sphe_emit_cmp->m_SpawnLocalPose.m_rotation.toEulerAngles();
	CMFCPropertyGridProperty* pSpawnLocalRot = new CSimpleProp(_T("SpawnLocalRot"), PropertySphericalEmitterSpawnLocalRot, TRUE);
	pComponent->AddSubItem(pSpawnLocalRot);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertySphericalEmitterSpawnLocalRotX);
	pSpawnLocalRot->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertySphericalEmitterSpawnLocalRotY);
	pSpawnLocalRot->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertySphericalEmitterSpawnLocalRotZ);
	pSpawnLocalRot->AddSubItem(pProp);
	CMFCPropertyGridProperty * pParticleLifeTime = new CSimpleProp(_T("ParticleLifeTime"), (_variant_t)sphe_emit_cmp->m_ParticleLifeTime, NULL, PropertySphericalEmitterParticleLifeTime);
	pComponent->AddSubItem(pParticleLifeTime);
	CMFCPropertyGridProperty* pParticleGravity = new CSimpleProp(_T("ParticleGravity"), PropertySphericalEmitterParticleGravity, TRUE);
	pComponent->AddSubItem(pParticleGravity);
	pProp = new CSimpleProp(_T("x"), (_variant_t)sphe_emit_cmp->m_ParticleGravity.x, NULL, PropertySphericalEmitterParticleGravityX);
	pParticleGravity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)sphe_emit_cmp->m_ParticleGravity.y, NULL, PropertySphericalEmitterParticleGravityY);
	pParticleGravity->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)sphe_emit_cmp->m_ParticleGravity.z, NULL, PropertySphericalEmitterParticleGravityZ);
	pParticleGravity->AddSubItem(pProp);
	CMFCPropertyGridProperty * pParticleDamping = new CSimpleProp(_T("ParticleDamping"), (_variant_t)sphe_emit_cmp->m_ParticleDamping, NULL, PropertySphericalEmitterParticleDamping);
	pComponent->AddSubItem(pParticleDamping);
	CreatePropertiesSpline(pComponent, _T("ParticleColorR"), PropertySphericalEmitterParticleColorR, &sphe_emit_cmp->m_ParticleColorR);
	CreatePropertiesSpline(pComponent, _T("ParticleColorG"), PropertySphericalEmitterParticleColorG, &sphe_emit_cmp->m_ParticleColorG);
	CreatePropertiesSpline(pComponent, _T("ParticleColorB"), PropertySphericalEmitterParticleColorB, &sphe_emit_cmp->m_ParticleColorB);
	CreatePropertiesSpline(pComponent, _T("ParticleColorA"), PropertySphericalEmitterParticleColorA, &sphe_emit_cmp->m_ParticleColorA);
	CreatePropertiesSpline(pComponent, _T("ParticleSizeX"), PropertySphericalEmitterParticleSizeX, &sphe_emit_cmp->m_ParticleSizeX);
	CreatePropertiesSpline(pComponent, _T("ParticleSizeY"), PropertySphericalEmitterParticleSizeY, &sphe_emit_cmp->m_ParticleSizeY);
	CreatePropertiesSpline(pComponent, _T("ParticleAngle"), PropertySphericalEmitterParticleAngle, &sphe_emit_cmp->m_ParticleAngle);
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

void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, int NodeId, my::Spline::value_type* node)
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
	pProp = new CSimpleProp(_T("ChunkLodScale"), (_variant_t)terrain->m_ChunkLodScale, NULL, PropertyTerrainChunkLodScale);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("HeightMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainHeightMap);
	pComponent->AddSubItem(pProp);
	pProp = new CFileProp(_T("SplatMap"), TRUE, (_variant_t)_T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyTerrainSplatMap);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, _T("Material"), terrain->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesAnimator(CMFCPropertyGridProperty* pComponent, Animator* animator)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CMFCPropertyGridProperty * pProp = new CFileProp(_T("SkeletonPath"), TRUE, (_variant_t)theApp.GetFullPath(animator->m_SkeletonPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyAnimatorSkeletonPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	CMFCPropertyGridProperty* pRootPosition = new CSimpleProp(_T("RootPosition"), PropertyAnimatorRootPosition, TRUE);
	pComponent->AddSubItem(pRootPosition);
	pProp = new CSimpleProp(_T("x"), (_variant_t)animator->m_RootBone.m_position.x, NULL, PropertyAnimatorRootPositionX);
	pRootPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)animator->m_RootBone.m_position.y, NULL, PropertyAnimatorRootPositionY);
	pRootPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)animator->m_RootBone.m_position.z, NULL, PropertyAnimatorRootPositionZ);
	pRootPosition->AddSubItem(pProp);
	my::Vector3 angle = animator->m_RootBone.m_rotation.toEulerAngles();
	CMFCPropertyGridProperty* pRootRotation = new CSimpleProp(_T("RootRotation"), PropertyAnimatorRootRotation, TRUE);
	pComponent->AddSubItem(pRootRotation);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertyAnimatorRootRotationX);
	pRootRotation->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertyAnimatorRootRotationY);
	pRootRotation->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertyAnimatorRootRotationZ);
	pRootRotation->AddSubItem(pProp);
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

void CPropertiesWnd::CreatePropertiesAnimationNodeSequence(CMFCPropertyGridProperty * pAnimationNode, AnimationNodeSequence * seq)
{
	ASSERT(pAnimationNode->GetSubItemsCount() == 1);

	CMFCPropertyGridProperty* pProp = new CComboProp(_T("Name"), (_variant_t)ms2ts(seq->m_Name.c_str()).c_str(), NULL, PropertyAnimationNodeSequenceName);
	pAnimationNode->AddSubItem(pProp);

	Animator* animator = dynamic_cast<Animator*>(seq->GetTopNode());
	if (animator->m_Skeleton)
	{
		std::set<std::string> dummy_map;
		std::transform(animator->m_Skeleton->m_animationMap.begin(), animator->m_Skeleton->m_animationMap.end(),
			std::inserter(dummy_map, dummy_map.end()), boost::bind(&my::OgreSkeletonAnimation::OgreAnimationMap::value_type::first, boost::placeholders::_1));
		std::set<std::string>::const_iterator anim_iter = dummy_map.begin();
		for (; anim_iter != dummy_map.end(); anim_iter++)
		{
			pProp->AddOption(ms2ts(anim_iter->c_str()).c_str(), TRUE);
		}
	}

	pProp = new CSimpleProp(_T("Rate"), (_variant_t)seq->m_Rate, NULL, PropertyAnimationNodeSequenceRate);
	pAnimationNode->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesNavigation(CMFCPropertyGridProperty * pComponent, Navigation * navigation)
{
	ASSERT(pComponent->GetSubItemsCount() == GetComponentPropCount(Component::ComponentTypeComponent));

	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("NavMeshPath"), (_variant_t)ms2ts(navigation->m_navMeshPath.c_str()).c_str(), NULL, PropertyNavigationNavMeshPath);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);

	const dtNavMeshParams * params = navigation->m_navMesh->getParams();
	CMFCPropertyGridProperty* pOrigin = new CSimpleProp(_T("Origin"), PropertyNavigationOrigin, TRUE);
	pOrigin->Enable(FALSE);
	pComponent->AddSubItem(pOrigin);
	pProp = new CSimpleProp(_T("x"), (_variant_t)params->orig[0], NULL, PropertyNavigationOriginX);
	pProp->Enable(FALSE);
	pOrigin->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)params->orig[1], NULL, PropertyNavigationOriginY);
	pProp->Enable(FALSE);
	pOrigin->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)params->orig[2], NULL, PropertyNavigationOriginZ);
	pProp->Enable(FALSE);
	pOrigin->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("TileWidth"), (_variant_t)params->tileWidth, NULL, PropertyNavigationTileWidth);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("TileHeight"), (_variant_t)params->tileHeight, NULL, PropertyNavigationTileHeight);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("MaxTiles"), (_variant_t)params->maxTiles, NULL, PropertyNavigationMaxTiles);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("MaxPolys"), (_variant_t)params->maxPolys, NULL, PropertyNavigationMaxPolys);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesControl(my::Control * control)
{
	CMFCPropertyGridProperty* pControl = new CSimpleProp(GetControlTypeName(control->GetControlType()), PropertyControl, FALSE);
	m_wndPropList.AddProperty(pControl, FALSE, FALSE);
	pControl->SetValue((_variant_t)(DWORD_PTR)control);

	CMFCPropertyGridProperty* pName = new CSimpleProp(_T("Name"), (_variant_t)ms2ts(control->GetName()).c_str(), NULL, PropertyControlName);
	pControl->AddSubItem(pName);

	CMFCPropertyGridProperty* pX = new CSimpleProp(_T("x"), PropertyControlX, TRUE);
	pControl->AddSubItem(pX);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("scale"), (_variant_t)control->m_x.scale, NULL, PropertyControlXScale);
	pX->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("offset"), (_variant_t)control->m_x.offset, NULL, PropertyControlXOffset);
	pX->AddSubItem(pProp);

	CMFCPropertyGridProperty* pY = new CSimpleProp(_T("y"), PropertyControlY, TRUE);
	pControl->AddSubItem(pY);
	pProp = new CSimpleProp(_T("scale"), (_variant_t)control->m_y.scale, NULL, PropertyControlYScale);
	pY->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("offset"), (_variant_t)control->m_y.offset, NULL, PropertyControlYOffset);
	pY->AddSubItem(pProp);

	CMFCPropertyGridProperty* pWidth = new CSimpleProp(_T("Width"), PropertyControlWidth, TRUE);
	pControl->AddSubItem(pWidth);
	pProp = new CSimpleProp(_T("scale"), (_variant_t)control->m_Width.scale, NULL, PropertyControlWidthScale);
	pWidth->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("offset"), (_variant_t)control->m_Width.offset, NULL, PropertyControlWidthOffset);
	pWidth->AddSubItem(pProp);

	CMFCPropertyGridProperty* pHeight = new CSimpleProp(_T("Height"), PropertyControlHeight, TRUE);
	pControl->AddSubItem(pHeight);
	pProp = new CSimpleProp(_T("scale"), (_variant_t)control->m_Height.scale, NULL, PropertyControlHeightScale);
	pHeight->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("offset"), (_variant_t)control->m_Height.offset, NULL, PropertyControlHeightOffset);
	pHeight->AddSubItem(pProp);

	CMFCPropertyGridProperty* pEnabled = new CCheckBoxProp(_T("Enabled"), control->GetEnabled(), NULL, PropertyControlEnabled);
	pControl->AddSubItem(pEnabled);

	CMFCPropertyGridProperty* pVisible = new CCheckBoxProp(_T("Visible"), control->GetVisible(), NULL, PropertyControlVisible);
	pControl->AddSubItem(pVisible);

	CMFCPropertyGridProperty* pFocused = new CCheckBoxProp(_T("Focused"), my::Control::GetFocusControl() == control, NULL, PropertyControlFocused);
	pControl->AddSubItem(pFocused);

	CMFCPropertyGridProperty* pSiblingId = new CSimpleProp(_T("SiblingId"), (_variant_t)control->GetSiblingId(), NULL, PropertyControlSiblingId);
	pControl->AddSubItem(pSiblingId);

	COLORREF color = RGB(LOBYTE(control->m_Skin->m_Color >> 16), LOBYTE(control->m_Skin->m_Color >> 8), LOBYTE(control->m_Skin->m_Color));
	CColorProp* pColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyControlColor);
	pColor->EnableOtherButton(_T("Other..."));
	pControl->AddSubItem(pColor);
	CMFCPropertyGridProperty* pAlpha = new CSliderProp(_T("Alpha"), (long)LOBYTE(control->m_Skin->m_Color >> 24), NULL, PropertyControlColorAlpha);
	pControl->AddSubItem(pAlpha);

	CMFCPropertyGridProperty* pImagePath = new CFileProp(_T("Image"), TRUE, (_variant_t)theApp.GetFullPath(control->m_Skin->m_Image->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyControlImagePath);
	pControl->AddSubItem(pImagePath);

	CMFCPropertyGridProperty* pImageRect = new CSimpleProp(_T("ImageRect"), PropertyControlImageRect, TRUE);
	pControl->AddSubItem(pImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)control->m_Skin->m_Image->m_Rect.left, NULL, PropertyControlImageRectLeft);
	pImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)control->m_Skin->m_Image->m_Rect.top, NULL, PropertyControlImageRectTop);
	pImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)control->m_Skin->m_Image->m_Rect.Width(), NULL, PropertyControlImageRectWidth);
	pImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)control->m_Skin->m_Image->m_Rect.Height(), NULL, PropertyControlImageRectHeight);
	pImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pImageBorder = new CSimpleProp(_T("ImageBorder"), PropertyControlImageBorder, TRUE);
	pControl->AddSubItem(pImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)control->m_Skin->m_Image->m_Border.left, NULL, PropertyControlImageBorderX);
	pImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)control->m_Skin->m_Image->m_Border.top, NULL, PropertyControlImageBorderY);
	pImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)control->m_Skin->m_Image->m_Border.right, NULL, PropertyControlImageBorderZ);
	pImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)control->m_Skin->m_Image->m_Border.bottom, NULL, PropertyControlImageBorderW);
	pImageBorder->AddSubItem(pProp);

	pProp = new CFileProp(_T("VisibleShowSound"), TRUE, (_variant_t)theApp.GetFullPath(control->m_Skin->m_VisibleShowSoundPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyControlVisibleShowSoundPath);
	pControl->AddSubItem(pProp);
	pProp = new CFileProp(_T("VisibleHideSound"), TRUE, (_variant_t)theApp.GetFullPath(control->m_Skin->m_VisibleHideSoundPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyControlVisibleHideSoundPath);
	pControl->AddSubItem(pProp);
	pProp = new CFileProp(_T("MouseEnterSound"), TRUE, (_variant_t)theApp.GetFullPath(control->m_Skin->m_MouseEnterSoundPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyControlMouseEnterSoundPath);
	pControl->AddSubItem(pProp);
	pProp = new CFileProp(_T("MouseLeaveSound"), TRUE, (_variant_t)theApp.GetFullPath(control->m_Skin->m_MouseLeaveSoundPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyControlMouseLeaveSoundPath);
	pControl->AddSubItem(pProp);
	pProp = new CFileProp(_T("MouseClickSound"), TRUE, (_variant_t)theApp.GetFullPath(control->m_Skin->m_MouseClickSoundPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyControlMouseClickSoundPath);
	pControl->AddSubItem(pProp);

	switch (control->GetControlType())
	{
	case my::Control::ControlTypeStatic:
	case my::Control::ControlTypeProgressBar:
	case my::Control::ControlTypeButton:
	//case my::Control::ControlTypeEditBox:
	case my::Control::ControlTypeImeEditBox:
		CreatePropertiesStatic(pControl, dynamic_cast<my::Static*>(control));
		break;
	case my::Control::ControlTypeScrollBar:
	case my::Control::ControlTypeHorizontalScrollBar:
		CreatePropertiesScrollBar(pControl, dynamic_cast<my::ScrollBar*>(control));
		break;
	case my::Control::ControlTypeCheckBox:
	case my::Control::ControlTypeComboBox:
		CreatePropertiesStatic(pControl, dynamic_cast<my::Static*>(control));
		break;
	case my::Control::ControlTypeListBox:
		CreatePropertiesListBox(pControl, dynamic_cast<my::ListBox*>(control));
		break;
	case my::Control::ControlTypeDialog:
		CreatePropertiesDialog(pControl, dynamic_cast<my::Dialog*>(control));
		break;
	default:
		break;
	}
}

void CPropertiesWnd::CreatePropertiesStatic(CMFCPropertyGridProperty * pControl, my::Static * static_ctl)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeControl));

	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("Text"), (_variant_t)ws2ts(boost::algorithm::replace_all_copy(static_ctl->m_Text, L"\n", L"\\n")).c_str(), NULL, PropertyStaticText);
	pControl->AddSubItem(pProp);

	my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(static_ctl->m_Skin);

	CMFCPropertyGridProperty* pFontPath = new CFileProp(_T("FontPath"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_FontPath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyStaticFontPath);
	pControl->AddSubItem(pFontPath);
	CMFCPropertyGridProperty* pFontHeight = new CSimpleProp(_T("FontHeight"), (_variant_t)(long)skin->m_FontHeight, NULL, PropertyStaticFontHeight);
	pControl->AddSubItem(pFontHeight);

	TCHAR buff[65];
	CMFCPropertyGridProperty* pFontFaceIndex = new CComboProp(_T("FontFaceIndex"), _itot(skin->m_FontFaceIndex, buff, 10), NULL, PropertyStaticFontFaceIndex);
	for (unsigned int i = 0; skin->m_Font && i < skin->m_Font->m_face->num_faces; i++)
	{
		pFontFaceIndex->AddOption(_itot(i, buff, 10), TRUE);
	}
	pControl->AddSubItem(pFontFaceIndex);

	COLORREF color = RGB(LOBYTE(skin->m_TextColor >> 16), LOBYTE(skin->m_TextColor >> 8), LOBYTE(skin->m_TextColor));
	CColorProp* pTextColor = new CColorProp(_T("TextColor"), color, NULL, NULL, PropertyStaticTextColor);
	pTextColor->EnableOtherButton(_T("Other..."));
	pControl->AddSubItem(pTextColor);
	CMFCPropertyGridProperty* pTextColorAlpha = new CSliderProp(_T("TextAlpha"), (long)LOBYTE(skin->m_TextColor >> 24), NULL, PropertyStaticTextColorAlpha);
	pControl->AddSubItem(pTextColorAlpha);

	CMFCPropertyGridProperty* pTextAlign = new CComboProp(_T("TextAlign"), GetFontAlignDesc(skin->m_TextAlign), NULL, PropertyStaticTextAlign);
	for (unsigned int i = 0; i < _countof(g_FontAlignDesc); i++)
	{
		pTextAlign->AddOption(g_FontAlignDesc[i].desc, TRUE);
	}
	pControl->AddSubItem(pTextAlign);

	color = RGB(LOBYTE(skin->m_TextOutlineColor >> 16), LOBYTE(skin->m_TextOutlineColor >> 8), LOBYTE(skin->m_TextOutlineColor));
	CColorProp* pTextOutlineColor = new CColorProp(_T("TextOutlineColor"), color, NULL, NULL, PropertyStaticTextOutlineColor);
	pTextOutlineColor->EnableOtherButton(_T("Other..."));
	pControl->AddSubItem(pTextOutlineColor);
	CMFCPropertyGridProperty* pTextOutlineAlpha = new CSliderProp(_T("TextOutlineAlpha"), (long)LOBYTE(skin->m_TextOutlineColor >> 24), NULL, PropertyStaticTextOutlineAlpha);
	pControl->AddSubItem(pTextOutlineAlpha);

	CMFCPropertyGridProperty* pTextOutlineWidth = new CSimpleProp(_T("TextOutlineWidth"), (_variant_t)skin->m_TextOutlineWidth, NULL, PropertyStaticTextOutlineWidth);
	pControl->AddSubItem(pTextOutlineWidth);

	switch (static_ctl->GetControlType())
	{
	case my::Control::ControlTypeProgressBar:
		CreatePropertiesProgressBar(pControl, dynamic_cast<my::ProgressBar*>(static_ctl));
		break;
	case my::Control::ControlTypeButton:
		CreatePropertiesButton(pControl, dynamic_cast<my::Button*>(static_ctl));
		break;
	//case my::Control::ControlTypeEditBox:
	case my::Control::ControlTypeImeEditBox:
		CreatePropertiesEditBox(pControl, dynamic_cast<my::EditBox*>(static_ctl));
		break;
	case my::Control::ControlTypeCheckBox:
	case my::Control::ControlTypeComboBox:
		CreatePropertiesButton(pControl, dynamic_cast<my::Button*>(static_ctl));
		break;
	}
}

void CPropertiesWnd::CreatePropertiesProgressBar(CMFCPropertyGridProperty * pControl, my::ProgressBar * progressbar)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeStatic));

	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("Progress"), (_variant_t)progressbar->m_Progress, NULL, PropertyProgressBarProgress);
	pControl->AddSubItem(pProp);

	my::ProgressBarSkinPtr skin = boost::dynamic_pointer_cast<my::ProgressBarSkin>(progressbar->m_Skin);

	CMFCPropertyGridProperty* pForegroundImagePath = new CFileProp(_T("ForegroundImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_ForegroundImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyProgressBarForegroundImagePath);
	pControl->AddSubItem(pForegroundImagePath);

	CMFCPropertyGridProperty* pForegroundImageRect = new CSimpleProp(_T("ForegroundImageRect"), PropertyProgressBarForegroundImageRect, TRUE);
	pControl->AddSubItem(pForegroundImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_ForegroundImage->m_Rect.left, NULL, PropertyProgressBarForegroundImageRectLeft);
	pForegroundImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_ForegroundImage->m_Rect.top, NULL, PropertyProgressBarForegroundImageRectTop);
	pForegroundImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_ForegroundImage->m_Rect.Width(), NULL, PropertyProgressBarForegroundImageRectWidth);
	pForegroundImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_ForegroundImage->m_Rect.Height(), NULL, PropertyProgressBarForegroundImageRectHeight);
	pForegroundImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pForegroundImageBorder = new CSimpleProp(_T("ForegroundImageBorder"), PropertyProgressBarForegroundImageBorder, TRUE);
	pControl->AddSubItem(pForegroundImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_ForegroundImage->m_Border.left, NULL, PropertyProgressBarForegroundImageBorderX);
	pForegroundImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_ForegroundImage->m_Border.top, NULL, PropertyProgressBarForegroundImageBorderY);
	pForegroundImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_ForegroundImage->m_Border.right, NULL, PropertyProgressBarForegroundImageBorderZ);
	pForegroundImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_ForegroundImage->m_Border.bottom, NULL, PropertyProgressBarForegroundImageBorderW);
	pForegroundImageBorder->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesButton(CMFCPropertyGridProperty * pControl, my::Button * button)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeStatic));

	CMFCPropertyGridProperty* pPressed = new CCheckBoxProp(_T("Pressed"), button->m_bPressed, NULL, PropertyButtonPressed);
	pControl->AddSubItem(pPressed);

	CMFCPropertyGridProperty* pMouseOver = new CCheckBoxProp(_T("MouseOver"), button->GetMouseOver(), NULL, PropertyButtonMouseOver);
	pControl->AddSubItem(pMouseOver);

	my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);

	CMFCPropertyGridProperty* pPressedOffset = new CSimpleProp(_T("PressedOffset"), PropertyButtonPressedOffset, TRUE);
	pControl->AddSubItem(pPressedOffset);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_PressedOffset.x, NULL, PropertyButtonPressedOffsetX);
	pPressedOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_PressedOffset.y, NULL, PropertyButtonPressedOffsetY);
	pPressedOffset->AddSubItem(pProp);

	CMFCPropertyGridProperty* pDisabledImagePath = new CFileProp(_T("DisabledImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_DisabledImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyButtonDisabledImagePath);
	pControl->AddSubItem(pDisabledImagePath);

	CMFCPropertyGridProperty* pDisabledImageRect = new CSimpleProp(_T("DisabledImageRect"), PropertyButtonDisabledImageRect, TRUE);
	pControl->AddSubItem(pDisabledImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_DisabledImage->m_Rect.left, NULL, PropertyButtonDisabledImageRectLeft);
	pDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_DisabledImage->m_Rect.top, NULL, PropertyButtonDisabledImageRectTop);
	pDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_DisabledImage->m_Rect.Width(), NULL, PropertyButtonDisabledImageRectWidth);
	pDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_DisabledImage->m_Rect.Height(), NULL, PropertyButtonDisabledImageRectHeight);
	pDisabledImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pDisabledImageBorder = new CSimpleProp(_T("DisabledImageBorder"), PropertyButtonDisabledImageBorder, TRUE);
	pControl->AddSubItem(pDisabledImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_DisabledImage->m_Border.left, NULL, PropertyButtonDisabledImageBorderX);
	pDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_DisabledImage->m_Border.top, NULL, PropertyButtonDisabledImageBorderY);
	pDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_DisabledImage->m_Border.right, NULL, PropertyButtonDisabledImageBorderZ);
	pDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_DisabledImage->m_Border.bottom, NULL, PropertyButtonDisabledImageBorderW);
	pDisabledImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pPressedImagePath = new CFileProp(_T("PressedImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_PressedImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyButtonPressedImagePath);
	pControl->AddSubItem(pPressedImagePath);

	CMFCPropertyGridProperty* pPressedImageRect = new CSimpleProp(_T("PressedImageRect"), PropertyButtonPressedImageRect, TRUE);
	pControl->AddSubItem(pPressedImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_PressedImage->m_Rect.left, NULL, PropertyButtonPressedImageRectLeft);
	pPressedImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_PressedImage->m_Rect.top, NULL, PropertyButtonPressedImageRectTop);
	pPressedImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_PressedImage->m_Rect.Width(), NULL, PropertyButtonPressedImageRectWidth);
	pPressedImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_PressedImage->m_Rect.Height(), NULL, PropertyButtonPressedImageRectHeight);
	pPressedImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pPressedImageBorder = new CSimpleProp(_T("PressedImageBorder"), PropertyButtonPressedImageBorder, TRUE);
	pControl->AddSubItem(pPressedImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_PressedImage->m_Border.left, NULL, PropertyButtonPressedImageBorderX);
	pPressedImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_PressedImage->m_Border.top, NULL, PropertyButtonPressedImageBorderY);
	pPressedImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_PressedImage->m_Border.right, NULL, PropertyButtonPressedImageBorderZ);
	pPressedImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_PressedImage->m_Border.bottom, NULL, PropertyButtonPressedImageBorderW);
	pPressedImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pMouseOverImagePath = new CFileProp(_T("MouseOverImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_MouseOverImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyButtonMouseOverImagePath);
	pControl->AddSubItem(pMouseOverImagePath);

	CMFCPropertyGridProperty* pMouseOverImageRect = new CSimpleProp(_T("MouseOverImageRect"), PropertyButtonMouseOverImageRect, TRUE);
	pControl->AddSubItem(pMouseOverImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_MouseOverImage->m_Rect.left, NULL, PropertyButtonMouseOverImageRectLeft);
	pMouseOverImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_MouseOverImage->m_Rect.top, NULL, PropertyButtonMouseOverImageRectTop);
	pMouseOverImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_MouseOverImage->m_Rect.Width(), NULL, PropertyButtonMouseOverImageRectWidth);
	pMouseOverImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_MouseOverImage->m_Rect.Height(), NULL, PropertyButtonMouseOverImageRectHeight);
	pMouseOverImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pMouseOverImageBorder = new CSimpleProp(_T("MouseOverImageBorder"), PropertyButtonMouseOverImageBorder, TRUE);
	pControl->AddSubItem(pMouseOverImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_MouseOverImage->m_Border.left, NULL, PropertyButtonMouseOverImageBorderX);
	pMouseOverImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_MouseOverImage->m_Border.top, NULL, PropertyButtonMouseOverImageBorderY);
	pMouseOverImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_MouseOverImage->m_Border.right, NULL, PropertyButtonMouseOverImageBorderZ);
	pMouseOverImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_MouseOverImage->m_Border.bottom, NULL, PropertyButtonMouseOverImageBorderW);
	pMouseOverImageBorder->AddSubItem(pProp);

	switch (button->GetControlType())
	{
	case my::Control::ControlTypeCheckBox:
		CreatePropertiesCheckBox(pControl, dynamic_cast<my::CheckBox*>(button));
		break;
	case my::Control::ControlTypeComboBox:
		CreatePropertiesComboBox(pControl, dynamic_cast<my::ComboBox*>(button));
		break;
	}
}

void CPropertiesWnd::CreatePropertiesEditBox(CMFCPropertyGridProperty * pControl, my::EditBox * editbox)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeStatic));

	CMFCPropertyGridProperty* pBorder = new CSimpleProp(_T("Border"), PropertyEditBoxBorder, TRUE);
	pControl->AddSubItem(pBorder);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)editbox->m_Border.x, NULL, PropertyEditBoxBorderX);
	pBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)editbox->m_Border.y, NULL, PropertyEditBoxBorderY);
	pBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)editbox->m_Border.z, NULL, PropertyEditBoxBorderZ);
	pBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)editbox->m_Border.w, NULL, PropertyEditBoxBorderW);
	pBorder->AddSubItem(pProp);

	my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(editbox->m_Skin);

	CMFCPropertyGridProperty* pDisabledImagePath = new CFileProp(_T("DisabledImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_DisabledImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyEditBoxDisabledImagePath);
	pControl->AddSubItem(pDisabledImagePath);

	CMFCPropertyGridProperty* pDisabledImageRect = new CSimpleProp(_T("DisabledImageRect"), PropertyEditBoxDisabledImageRect, TRUE);
	pControl->AddSubItem(pDisabledImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_DisabledImage->m_Rect.left, NULL, PropertyEditBoxDisabledImageRectLeft);
	pDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_DisabledImage->m_Rect.top, NULL, PropertyEditBoxDisabledImageRectTop);
	pDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_DisabledImage->m_Rect.Width(), NULL, PropertyEditBoxDisabledImageRectWidth);
	pDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_DisabledImage->m_Rect.Height(), NULL, PropertyEditBoxDisabledImageRectHeight);
	pDisabledImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pDisabledImageBorder = new CSimpleProp(_T("DisabledImageBorder"), PropertyEditBoxDisabledImageBorder, TRUE);
	pControl->AddSubItem(pDisabledImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_DisabledImage->m_Border.left, NULL, PropertyEditBoxDisabledImageBorderX);
	pDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_DisabledImage->m_Border.top, NULL, PropertyEditBoxDisabledImageBorderY);
	pDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_DisabledImage->m_Border.right, NULL, PropertyEditBoxDisabledImageBorderZ);
	pDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_DisabledImage->m_Border.bottom, NULL, PropertyEditBoxDisabledImageBorderW);
	pDisabledImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pFocusedImagePath = new CFileProp(_T("FocusedImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_FocusedImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyEditBoxFocusedImagePath);
	pControl->AddSubItem(pFocusedImagePath);

	CMFCPropertyGridProperty* pFocusedImageRect = new CSimpleProp(_T("FocusedImageRect"), PropertyEditBoxFocusedImageRect, TRUE);
	pControl->AddSubItem(pFocusedImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_FocusedImage->m_Rect.left, NULL, PropertyEditBoxFocusedImageRectLeft);
	pFocusedImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_FocusedImage->m_Rect.top, NULL, PropertyEditBoxFocusedImageRectTop);
	pFocusedImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_FocusedImage->m_Rect.Width(), NULL, PropertyEditBoxFocusedImageRectWidth);
	pFocusedImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_FocusedImage->m_Rect.Height(), NULL, PropertyEditBoxFocusedImageRectHeight);
	pFocusedImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pFocusedImageBorder = new CSimpleProp(_T("FocusedImageBorder"), PropertyEditBoxFocusedImageBorder, TRUE);
	pControl->AddSubItem(pFocusedImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_FocusedImage->m_Border.left, NULL, PropertyEditBoxFocusedImageBorderX);
	pFocusedImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_FocusedImage->m_Border.top, NULL, PropertyEditBoxFocusedImageBorderY);
	pFocusedImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_FocusedImage->m_Border.right, NULL, PropertyEditBoxFocusedImageBorderZ);
	pFocusedImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_FocusedImage->m_Border.bottom, NULL, PropertyEditBoxFocusedImageBorderW);
	pFocusedImageBorder->AddSubItem(pProp);

	COLORREF color = RGB(LOBYTE(skin->m_SelBkColor >> 16), LOBYTE(skin->m_SelBkColor >> 8), LOBYTE(skin->m_SelBkColor));
	CColorProp* pSelBkColor = new CColorProp(_T("SelBkColor"), color, NULL, NULL, PropertyEditBoxSelBkColor);
	pSelBkColor->EnableOtherButton(_T("Other..."));
	pControl->AddSubItem(pSelBkColor);
	CMFCPropertyGridProperty* pSelBkColorAlpha = new CSliderProp(_T("SelBkAlpha"), (long)LOBYTE(skin->m_SelBkColor >> 24), NULL, PropertyEditBoxSelBkColorAlpha);
	pControl->AddSubItem(pSelBkColorAlpha);

	color = RGB(LOBYTE(skin->m_CaretColor >> 16), LOBYTE(skin->m_CaretColor >> 8), LOBYTE(skin->m_CaretColor));
	CColorProp* pCaretColor = new CColorProp(_T("CaretColor"), color, NULL, NULL, PropertyEditBoxCaretColor);
	pCaretColor->EnableOtherButton(_T("Other..."));
	pControl->AddSubItem(pCaretColor);
	CMFCPropertyGridProperty* pCaretColorAlpha = new CSliderProp(_T("SelBkAlpha"), (long)LOBYTE(skin->m_CaretColor >> 24), NULL, PropertyEditBoxCaretColorAlpha);
	pControl->AddSubItem(pCaretColorAlpha);

	CMFCPropertyGridProperty* pCaretImagePath = new CFileProp(_T("CaretImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_CaretImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyEditBoxCaretImagePath);
	pControl->AddSubItem(pCaretImagePath);

	CMFCPropertyGridProperty* pCaretImageRect = new CSimpleProp(_T("CaretImageRect"), PropertyEditBoxCaretImageRect, TRUE);
	pControl->AddSubItem(pCaretImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_CaretImage->m_Rect.left, NULL, PropertyEditBoxCaretImageRectLeft);
	pCaretImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_CaretImage->m_Rect.top, NULL, PropertyEditBoxCaretImageRectTop);
	pCaretImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_CaretImage->m_Rect.Width(), NULL, PropertyEditBoxCaretImageRectWidth);
	pCaretImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_CaretImage->m_Rect.Height(), NULL, PropertyEditBoxCaretImageRectHeight);
	pCaretImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pCaretImageBorder = new CSimpleProp(_T("CaretImageBorder"), PropertyEditBoxCaretImageBorder, TRUE);
	pControl->AddSubItem(pCaretImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_CaretImage->m_Border.left, NULL, PropertyEditBoxCaretImageBorderX);
	pCaretImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_CaretImage->m_Border.top, NULL, PropertyEditBoxCaretImageBorderY);
	pCaretImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_CaretImage->m_Border.right, NULL, PropertyEditBoxCaretImageBorderZ);
	pCaretImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_CaretImage->m_Border.bottom, NULL, PropertyEditBoxCaretImageBorderW);
	pCaretImageBorder->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesScrollBar(CMFCPropertyGridProperty * pControl, my::ScrollBar * scrollbar)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeControl));

	CMFCPropertyGridProperty* pUpDownButtonHeight = new CSimpleProp(_T("UpDownButtonHeight"), (_variant_t)scrollbar->m_UpDownButtonHeight, NULL, PropertyScrollBarUpDownButtonHeight);
	pControl->AddSubItem(pUpDownButtonHeight);
	CMFCPropertyGridProperty* pPosition = new CSimpleProp(_T("Position"), (_variant_t)(long)scrollbar->m_nPosition, NULL, PropertyScrollBarPosition);
	pControl->AddSubItem(pPosition);
	CMFCPropertyGridProperty* pPageSize = new CSimpleProp(_T("PageSize"), (_variant_t)(long)scrollbar->m_nPageSize, NULL, PropertyScrollBarPageSize);
	pControl->AddSubItem(pPageSize);
	CMFCPropertyGridProperty* pStart = new CSimpleProp(_T("Start"), (_variant_t)(long)scrollbar->m_nStart, NULL, PropertyScrollBarStart);
	pControl->AddSubItem(pStart);
	CMFCPropertyGridProperty* pEnd = new CSimpleProp(_T("End"), (_variant_t)(long)scrollbar->m_nEnd, NULL, PropertyScrollBarEnd);
	pControl->AddSubItem(pEnd);

	my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);

	CMFCPropertyGridProperty* pPressedOffset = new CSimpleProp(_T("PressedOffset"), PropertyScrollBarPressedOffset, TRUE);
	pControl->AddSubItem(pPressedOffset);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_PressedOffset.x, NULL, PropertyScrollBarPressedOffsetX);
	pPressedOffset->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_PressedOffset.y, NULL, PropertyScrollBarPressedOffsetY);
	pPressedOffset->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarUpBtnNormalImagePath = new CFileProp(_T("ScrollBarUpBtnNormalImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_UpBtnNormalImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyScrollBarUpBtnNormalImagePath);
	pControl->AddSubItem(pScrollBarUpBtnNormalImagePath);

	CMFCPropertyGridProperty* pScrollBarUpBtnNormalImageRect = new CSimpleProp(_T("ScrollBarUpBtnNormalImageRect"), PropertyScrollBarUpBtnNormalImageRect, TRUE);
	pControl->AddSubItem(pScrollBarUpBtnNormalImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_UpBtnNormalImage->m_Rect.left, NULL, PropertyScrollBarUpBtnNormalImageRectLeft);
	pScrollBarUpBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_UpBtnNormalImage->m_Rect.top, NULL, PropertyScrollBarUpBtnNormalImageRectTop);
	pScrollBarUpBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_UpBtnNormalImage->m_Rect.Width(), NULL, PropertyScrollBarUpBtnNormalImageRectWidth);
	pScrollBarUpBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_UpBtnNormalImage->m_Rect.Height(), NULL, PropertyScrollBarUpBtnNormalImageRectHeight);
	pScrollBarUpBtnNormalImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarUpBtnNormalImageBorder = new CSimpleProp(_T("ScrollBarUpBtnNormalImageBorder"), PropertyScrollBarUpBtnNormalImageBorder, TRUE);
	pControl->AddSubItem(pScrollBarUpBtnNormalImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_UpBtnNormalImage->m_Border.left, NULL, PropertyScrollBarUpBtnNormalImageBorderX);
	pScrollBarUpBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_UpBtnNormalImage->m_Border.top, NULL, PropertyScrollBarUpBtnNormalImageBorderY);
	pScrollBarUpBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_UpBtnNormalImage->m_Border.right, NULL, PropertyScrollBarUpBtnNormalImageBorderZ);
	pScrollBarUpBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_UpBtnNormalImage->m_Border.bottom, NULL, PropertyScrollBarUpBtnNormalImageBorderW);
	pScrollBarUpBtnNormalImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarUpBtnDisabledImagePath = new CFileProp(_T("ScrollBarUpBtnDisabledImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_UpBtnDisabledImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyScrollBarUpBtnDisabledImagePath);
	pControl->AddSubItem(pScrollBarUpBtnDisabledImagePath);

	CMFCPropertyGridProperty* pScrollBarUpBtnDisabledImageRect = new CSimpleProp(_T("ScrollBarUpBtnDisabledImageRect"), PropertyScrollBarUpBtnDisabledImageRect, TRUE);
	pControl->AddSubItem(pScrollBarUpBtnDisabledImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_UpBtnDisabledImage->m_Rect.left, NULL, PropertyScrollBarUpBtnDisabledImageRectLeft);
	pScrollBarUpBtnDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_UpBtnDisabledImage->m_Rect.top, NULL, PropertyScrollBarUpBtnDisabledImageRectTop);
	pScrollBarUpBtnDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_UpBtnDisabledImage->m_Rect.Width(), NULL, PropertyScrollBarUpBtnDisabledImageRectWidth);
	pScrollBarUpBtnDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_UpBtnDisabledImage->m_Rect.Height(), NULL, PropertyScrollBarUpBtnDisabledImageRectHeight);
	pScrollBarUpBtnDisabledImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarUpBtnDisabledImageBorder = new CSimpleProp(_T("ScrollBarUpBtnDisabledImageBorder"), PropertyScrollBarUpBtnDisabledImageBorder, TRUE);
	pControl->AddSubItem(pScrollBarUpBtnDisabledImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_UpBtnDisabledImage->m_Border.left, NULL, PropertyScrollBarUpBtnDisabledImageBorderX);
	pScrollBarUpBtnDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_UpBtnDisabledImage->m_Border.top, NULL, PropertyScrollBarUpBtnDisabledImageBorderY);
	pScrollBarUpBtnDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_UpBtnDisabledImage->m_Border.right, NULL, PropertyScrollBarUpBtnDisabledImageBorderZ);
	pScrollBarUpBtnDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_UpBtnDisabledImage->m_Border.bottom, NULL, PropertyScrollBarUpBtnDisabledImageBorderW);
	pScrollBarUpBtnDisabledImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarDownBtnNormalImagePath = new CFileProp(_T("ScrollBarDownBtnNormalImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_DownBtnNormalImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyScrollBarDownBtnNormalImagePath);
	pControl->AddSubItem(pScrollBarDownBtnNormalImagePath);

	CMFCPropertyGridProperty* pScrollBarDownBtnNormalImageRect = new CSimpleProp(_T("ScrollBarDownBtnNormalImageRect"), PropertyScrollBarDownBtnNormalImageRect, TRUE);
	pControl->AddSubItem(pScrollBarDownBtnNormalImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_DownBtnNormalImage->m_Rect.left, NULL, PropertyScrollBarDownBtnNormalImageRectLeft);
	pScrollBarDownBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_DownBtnNormalImage->m_Rect.top, NULL, PropertyScrollBarDownBtnNormalImageRectTop);
	pScrollBarDownBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_DownBtnNormalImage->m_Rect.Width(), NULL, PropertyScrollBarDownBtnNormalImageRectWidth);
	pScrollBarDownBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_DownBtnNormalImage->m_Rect.Height(), NULL, PropertyScrollBarDownBtnNormalImageRectHeight);
	pScrollBarDownBtnNormalImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarDownBtnNormalImageBorder = new CSimpleProp(_T("ScrollBarDownBtnNormalImageBorder"), PropertyScrollBarDownBtnNormalImageBorder, TRUE);
	pControl->AddSubItem(pScrollBarDownBtnNormalImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_DownBtnNormalImage->m_Border.left, NULL, PropertyScrollBarDownBtnNormalImageBorderX);
	pScrollBarDownBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_DownBtnNormalImage->m_Border.top, NULL, PropertyScrollBarDownBtnNormalImageBorderY);
	pScrollBarDownBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_DownBtnNormalImage->m_Border.right, NULL, PropertyScrollBarDownBtnNormalImageBorderZ);
	pScrollBarDownBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_DownBtnNormalImage->m_Border.bottom, NULL, PropertyScrollBarDownBtnNormalImageBorderW);
	pScrollBarDownBtnNormalImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarDownBtnDisabledImagePath = new CFileProp(_T("ScrollBarDownBtnDisabledImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_DownBtnDisabledImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyScrollBarDownBtnDisabledImagePath);
	pControl->AddSubItem(pScrollBarDownBtnDisabledImagePath);

	CMFCPropertyGridProperty* pScrollBarDownBtnDisabledImageRect = new CSimpleProp(_T("ScrollBarDownBtnDisabledImageRect"), PropertyScrollBarDownBtnDisabledImageRect, TRUE);
	pControl->AddSubItem(pScrollBarDownBtnDisabledImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_DownBtnDisabledImage->m_Rect.left, NULL, PropertyScrollBarDownBtnDisabledImageRectLeft);
	pScrollBarDownBtnDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_DownBtnDisabledImage->m_Rect.top, NULL, PropertyScrollBarDownBtnDisabledImageRectTop);
	pScrollBarDownBtnDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_DownBtnDisabledImage->m_Rect.Width(), NULL, PropertyScrollBarDownBtnDisabledImageRectWidth);
	pScrollBarDownBtnDisabledImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_DownBtnDisabledImage->m_Rect.Height(), NULL, PropertyScrollBarDownBtnDisabledImageRectHeight);
	pScrollBarDownBtnDisabledImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarDownBtnDisabledImageBorder = new CSimpleProp(_T("ScrollBarDownBtnDisabledImageBorder"), PropertyScrollBarDownBtnDisabledImageBorder, TRUE);
	pControl->AddSubItem(pScrollBarDownBtnDisabledImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_DownBtnDisabledImage->m_Border.left, NULL, PropertyScrollBarDownBtnDisabledImageBorderX);
	pScrollBarDownBtnDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_DownBtnDisabledImage->m_Border.top, NULL, PropertyScrollBarDownBtnDisabledImageBorderY);
	pScrollBarDownBtnDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_DownBtnDisabledImage->m_Border.right, NULL, PropertyScrollBarDownBtnDisabledImageBorderZ);
	pScrollBarDownBtnDisabledImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_DownBtnDisabledImage->m_Border.bottom, NULL, PropertyScrollBarDownBtnDisabledImageBorderW);
	pScrollBarDownBtnDisabledImageBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarThumbBtnNormalImagePath = new CFileProp(_T("ScrollBarThumbBtnNormalImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_ThumbBtnNormalImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyScrollBarThumbBtnNormalImagePath);
	pControl->AddSubItem(pScrollBarThumbBtnNormalImagePath);

	CMFCPropertyGridProperty* pScrollBarThumbBtnNormalImageRect = new CSimpleProp(_T("ScrollBarThumbBtnNormalImageRect"), PropertyScrollBarThumbBtnNormalImageRect, TRUE);
	pControl->AddSubItem(pScrollBarThumbBtnNormalImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_ThumbBtnNormalImage->m_Rect.left, NULL, PropertyScrollBarThumbBtnNormalImageRectLeft);
	pScrollBarThumbBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_ThumbBtnNormalImage->m_Rect.top, NULL, PropertyScrollBarThumbBtnNormalImageRectTop);
	pScrollBarThumbBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_ThumbBtnNormalImage->m_Rect.Width(), NULL, PropertyScrollBarThumbBtnNormalImageRectWidth);
	pScrollBarThumbBtnNormalImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_ThumbBtnNormalImage->m_Rect.Height(), NULL, PropertyScrollBarThumbBtnNormalImageRectHeight);
	pScrollBarThumbBtnNormalImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollBarThumbBtnNormalImageBorder = new CSimpleProp(_T("ScrollBarThumbBtnNormalImageBorder"), PropertyScrollBarThumbBtnNormalImageBorder, TRUE);
	pControl->AddSubItem(pScrollBarThumbBtnNormalImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_ThumbBtnNormalImage->m_Border.left, NULL, PropertyScrollBarThumbBtnNormalImageBorderX);
	pScrollBarThumbBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_ThumbBtnNormalImage->m_Border.top, NULL, PropertyScrollBarThumbBtnNormalImageBorderY);
	pScrollBarThumbBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_ThumbBtnNormalImage->m_Border.right, NULL, PropertyScrollBarThumbBtnNormalImageBorderZ);
	pScrollBarThumbBtnNormalImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_ThumbBtnNormalImage->m_Border.bottom, NULL, PropertyScrollBarThumbBtnNormalImageBorderW);
	pScrollBarThumbBtnNormalImageBorder->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesCheckBox(CMFCPropertyGridProperty * pControl, my::CheckBox * checkbox)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeButton));

	CMFCPropertyGridProperty* pChecked = new CCheckBoxProp(_T("Checked"), checkbox->m_Checked, NULL, PropertyCheckBoxChecked);
	pControl->AddSubItem(pChecked);
}

void CPropertiesWnd::CreatePropertiesComboBox(CMFCPropertyGridProperty * pControl, my::ComboBox * combobox)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeButton));

	CMFCPropertyGridProperty* pDropdownSize = new CSimpleProp(_T("DropdownSize"), PropertyComboBoxDropdownSize, TRUE);
	pControl->AddSubItem(pDropdownSize);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)combobox->m_DropdownSize.x, NULL, PropertyComboBoxDropdownSizeX);
	pDropdownSize->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)combobox->m_DropdownSize.y, NULL, PropertyComboBoxDropdownSizeY);
	pDropdownSize->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScrollbarWidth = new CSimpleProp(_T("ScrollbarWidth"), (_variant_t)combobox->m_ScrollbarWidth, NULL, PropertyComboBoxScrollbarWidth);
	pControl->AddSubItem(pScrollbarWidth);

	CMFCPropertyGridProperty* pScrollbarUpDownBtnHeight = new CSimpleProp(_T("ScrollbarUpDownBtnHeight"), (_variant_t)combobox->m_ScrollbarUpDownBtnHeight, NULL, PropertyComboBoxScrollbarUpDownBtnHeight);
	pControl->AddSubItem(pScrollbarUpDownBtnHeight);

	CMFCPropertyGridProperty* pBorder = new CSimpleProp(_T("Border"), PropertyComboBoxBorder, TRUE);
	pControl->AddSubItem(pBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)combobox->m_Border.x, NULL, PropertyComboBoxBorderX);
	pBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)combobox->m_Border.y, NULL, PropertyComboBoxBorderY);
	pBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)combobox->m_Border.z, NULL, PropertyComboBoxBorderZ);
	pBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)combobox->m_Border.w, NULL, PropertyComboBoxBorderW);
	pBorder->AddSubItem(pProp);

	CMFCPropertyGridProperty* pItemHeight = new CSimpleProp(_T("ItemHeight"), (_variant_t)combobox->m_ItemHeight, NULL, PropertyComboBoxItemHeight);
	pControl->AddSubItem(pItemHeight);

	CMFCPropertyGridProperty* pItemCount = new CSimpleProp(_T("ItemCount"), (_variant_t)(long)combobox->m_Items.size(), NULL, PropertyComboBoxItemCount);
	pControl->AddSubItem(pItemCount);

	my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);

	CMFCPropertyGridProperty* pDropdownImagePath = new CFileProp(_T("DropdownImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_DropdownImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyComboBoxDropdownImagePath);
	pControl->AddSubItem(pDropdownImagePath);

	CMFCPropertyGridProperty* pDropdownImageRect = new CSimpleProp(_T("DropdownImageRect"), PropertyComboBoxDropdownImageRect, TRUE);
	pControl->AddSubItem(pDropdownImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_DropdownImage->m_Rect.left, NULL, PropertyComboBoxDropdownImageRectLeft);
	pDropdownImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_DropdownImage->m_Rect.top, NULL, PropertyComboBoxDropdownImageRectTop);
	pDropdownImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_DropdownImage->m_Rect.Width(), NULL, PropertyComboBoxDropdownImageRectWidth);
	pDropdownImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_DropdownImage->m_Rect.Height(), NULL, PropertyComboBoxDropdownImageRectHeight);
	pDropdownImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pDropdownImageBorder = new CSimpleProp(_T("DropdownImageBorder"), PropertyComboBoxDropdownImageBorder, TRUE);
	pControl->AddSubItem(pDropdownImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_DropdownImage->m_Border.left, NULL, PropertyComboBoxDropdownImageBorderX);
	pDropdownImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_DropdownImage->m_Border.top, NULL, PropertyComboBoxDropdownImageBorderY);
	pDropdownImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_DropdownImage->m_Border.right, NULL, PropertyComboBoxDropdownImageBorderZ);
	pDropdownImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_DropdownImage->m_Border.bottom, NULL, PropertyComboBoxDropdownImageBorderW);
	pDropdownImageBorder->AddSubItem(pProp);

	COLORREF color = RGB(LOBYTE(skin->m_DropdownItemTextColor >> 16), LOBYTE(skin->m_DropdownItemTextColor >> 8), LOBYTE(skin->m_DropdownItemTextColor));
	CColorProp* pDropdownItemTextColor = new CColorProp(_T("DropdownItemTextColor"), color, NULL, NULL, PropertyComboBoxDropdownItemTextColor);
	pDropdownItemTextColor->EnableOtherButton(_T("Other..."));
	pControl->AddSubItem(pDropdownItemTextColor);
	CMFCPropertyGridProperty* pDropdownItemTextColorAlpha = new CSliderProp(_T("DropdownItemTextAlpha"), (long)LOBYTE(skin->m_DropdownItemTextColor >> 24), NULL, PropertyComboBoxDropdownItemTextColorAlpha);
	pControl->AddSubItem(pDropdownItemTextColorAlpha);

	CMFCPropertyGridProperty* pDropdownItemTextAlign = new CComboProp(_T("DropdownItemTextAlign"), GetFontAlignDesc(skin->m_DropdownItemTextAlign), NULL, PropertyComboBoxDropdownItemTextAlign);
	for (unsigned int i = 0; i < _countof(g_FontAlignDesc); i++)
	{
		pDropdownItemTextAlign->AddOption(g_FontAlignDesc[i].desc, TRUE);
	}
	pControl->AddSubItem(pDropdownItemTextAlign);

	CMFCPropertyGridProperty* pDropdownItemMouseOverImagePath = new CFileProp(_T("DropdownItemMouseOverImage"), TRUE, (_variant_t)theApp.GetFullPath(skin->m_DropdownItemMouseOverImage->m_TexturePath.c_str()).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyComboBoxDropdownItemMouseOverImagePath);
	pControl->AddSubItem(pDropdownItemMouseOverImagePath);

	CMFCPropertyGridProperty* pDropdownItemMouseOverImageRect = new CSimpleProp(_T("DropdownItemMouseOverImageRect"), PropertyComboBoxDropdownItemMouseOverImageRect, TRUE);
	pControl->AddSubItem(pDropdownItemMouseOverImageRect);
	pProp = new CSimpleProp(_T("left"), (_variant_t)skin->m_DropdownItemMouseOverImage->m_Rect.left, NULL, PropertyComboBoxDropdownItemMouseOverImageRectLeft);
	pDropdownItemMouseOverImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("top"), (_variant_t)skin->m_DropdownItemMouseOverImage->m_Rect.top, NULL, PropertyComboBoxDropdownItemMouseOverImageRectTop);
	pDropdownItemMouseOverImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Width"), (_variant_t)(long)skin->m_DropdownItemMouseOverImage->m_Rect.Width(), NULL, PropertyComboBoxDropdownItemMouseOverImageRectWidth);
	pDropdownItemMouseOverImageRect->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), (_variant_t)(long)skin->m_DropdownItemMouseOverImage->m_Rect.Height(), NULL, PropertyComboBoxDropdownItemMouseOverImageRectHeight);
	pDropdownItemMouseOverImageRect->AddSubItem(pProp);

	CMFCPropertyGridProperty* pDropdownItemMouseOverImageBorder = new CSimpleProp(_T("DropdownItemMouseOverImageBorder"), PropertyComboBoxDropdownItemMouseOverImageBorder, TRUE);
	pControl->AddSubItem(pDropdownItemMouseOverImageBorder);
	pProp = new CSimpleProp(_T("x"), (_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.left, NULL, PropertyComboBoxDropdownItemMouseOverImageBorderX);
	pDropdownItemMouseOverImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.top, NULL, PropertyComboBoxDropdownItemMouseOverImageBorderY);
	pDropdownItemMouseOverImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.right, NULL, PropertyComboBoxDropdownItemMouseOverImageBorderZ);
	pDropdownItemMouseOverImageBorder->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)skin->m_DropdownItemMouseOverImage->m_Border.bottom, NULL, PropertyComboBoxDropdownItemMouseOverImageBorderW);
	pDropdownItemMouseOverImageBorder->AddSubItem(pProp);
}

void CPropertiesWnd::CreatePropertiesListBox(CMFCPropertyGridProperty * pControl, my::ListBox * listbox)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeControl));

	CMFCPropertyGridProperty* pScrollbarWidth = new CSimpleProp(_T("ScrollbarWidth"), (_variant_t)listbox->m_ScrollbarWidth, NULL, PropertyListBoxScrollbarWidth);
	pControl->AddSubItem(pScrollbarWidth);

	CMFCPropertyGridProperty* pScrollbarUpDownBtnHeight = new CSimpleProp(_T("ScrollbarUpDownBtnHeight"), (_variant_t)listbox->m_ScrollbarUpDownBtnHeight, NULL, PropertyListBoxScrollbarUpDownBtnHeight);
	pControl->AddSubItem(pScrollbarUpDownBtnHeight);

	CMFCPropertyGridProperty* pItemSize = new CSimpleProp(_T("ItemSize"), PropertyListBoxItemSize, TRUE);
	pControl->AddSubItem(pItemSize);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)listbox->m_ItemSize.x, NULL, PropertyListBoxItemSizeX);
	pItemSize->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)listbox->m_ItemSize.y, NULL, PropertyListBoxItemSizeY);
	pItemSize->AddSubItem(pProp);

	CMFCPropertyGridProperty* pItemCount = new CSimpleProp(_T("ItemCount"), (_variant_t)(long)listbox->m_Childs.size(), NULL, PropertyListBoxItemCount);
	pControl->AddSubItem(pItemCount);

	my::ListBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ListBoxSkin>(listbox->m_Skin);
}

void CPropertiesWnd::CreatePropertiesDialog(CMFCPropertyGridProperty * pControl, my::Dialog * dialog)
{
	ASSERT(pControl->GetSubItemsCount() == GetControlPropCount(my::Control::ControlTypeControl));

	CMFCPropertyGridProperty* pEnableDrag = new CCheckBoxProp(_T("EnableDrag"), dialog->m_EnableDrag, NULL, PropertyDialogEnableDrag);
	pControl->AddSubItem(pEnableDrag);

	my::Vector3 Pos, Scale; my::Quaternion Rot;
	dialog->m_World.Decompose(Scale, Rot, Pos);
	CMFCPropertyGridProperty* pPosition = new CSimpleProp(_T("Position"), PropertyDialogPos, TRUE);
	pControl->AddSubItem(pPosition);
	CMFCPropertyGridProperty* pProp = new CSimpleProp(_T("x"), (_variant_t)Pos.x, NULL, PropertyDialogPosX);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)Pos.y, NULL, PropertyDialogPosY);
	pPosition->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)Pos.z, NULL, PropertyDialogPosZ);
	pPosition->AddSubItem(pProp);

	my::Vector3 angle = Rot.toEulerAngles();
	CMFCPropertyGridProperty* pRotate = new CSimpleProp(_T("Rotate"), PropertyDialogRot, TRUE);
	pControl->AddSubItem(pRotate);
	pProp = new CSimpleProp(_T("x"), (_variant_t)D3DXToDegree(angle.x), NULL, PropertyDialogRotX);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)D3DXToDegree(angle.y), NULL, PropertyDialogRotY);
	pRotate->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)D3DXToDegree(angle.z), NULL, PropertyDialogRotZ);
	pRotate->AddSubItem(pProp);

	CMFCPropertyGridProperty* pScale = new CSimpleProp(_T("Scale"), PropertyDialogScale, TRUE);
	pControl->AddSubItem(pScale);
	pProp = new CSimpleProp(_T("x, y, z"), (_variant_t)Scale.x, NULL, PropertyDialogScaleX);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)Scale.y, NULL, PropertyDialogScaleY);
	pScale->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)Scale.z, NULL, PropertyDialogScaleZ);
	pScale->AddSubItem(pProp);
}

unsigned int CPropertiesWnd::GetComponentPropCount(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeController:
		return GetComponentPropCount(Component::ComponentTypeComponent);
	case Component::ComponentTypeMesh:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 8;
	case Component::ComponentTypeStaticMesh:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 4;
	case Component::ComponentTypeCloth:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 14;
	case Component::ComponentTypeEmitter:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 8;
	case Component::ComponentTypeStaticEmitter:
		return GetComponentPropCount(Component::ComponentTypeEmitter) + 4;
	case Component::ComponentTypeSphericalEmitter:
		return GetComponentPropCount(Component::ComponentTypeEmitter) + 21;
	case Component::ComponentTypeTerrain:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 8;
	case Component::ComponentTypeAnimator:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 4;
	case Component::ComponentTypeNavigation:
		return GetComponentPropCount(Component::ComponentTypeComponent) + 6;
	}

	ASSERT(Component::ComponentTypeComponent == type);
	return 4;
}

LPCTSTR CPropertiesWnd::GetComponentTypeName(DWORD type)
{
	switch (type)
	{
	case Component::ComponentTypeController:
		return _T("Controller");
	case Component::ComponentTypeMesh:
		return _T("Mesh");
	case Component::ComponentTypeStaticMesh:
		return _T("StaticMesh");
	case Component::ComponentTypeCloth:
		return _T("Cloth");
	case Component::ComponentTypeStaticEmitter:
		return _T("StaticEmitter");
	case Component::ComponentTypeSphericalEmitter:
		return _T("SphericalEmitter");
	case Component::ComponentTypeTerrain:
		return _T("Terrain");
	case Component::ComponentTypeAnimator:
		return _T("Animator");
	case Component::ComponentTypeNavigation:
		return _T("Navigation");
	}
	return _T("Unknown Component");
}

TerrainChunk * CPropertiesWnd::GetTerrainChunkSafe(Terrain * terrain, const CPoint & chunkid)
{
	if (chunkid.x >= 0 && chunkid.x < (int)terrain->m_Chunks.shape()[0]
		&& chunkid.y >= 0 && chunkid.y < (int)terrain->m_Chunks.shape()[1])
	{
		return &terrain->m_Chunks[chunkid.x][chunkid.y];
	}
	return &terrain->m_Chunks[0][0];
}

CPropertiesWnd::Property CPropertiesWnd::GetMaterialParameterTypeProp(MaterialParameter * mtl_param)
{
	switch (mtl_param->GetParameterType())
	{
	case MaterialParameter::ParameterTypeInt2:
		return PropertyMaterialParameterInt2;
	case MaterialParameter::ParameterTypeFloat:
		return PropertyMaterialParameterFloat;
	case MaterialParameter::ParameterTypeFloat2:
		return PropertyMaterialParameterFloat2;
	case MaterialParameter::ParameterTypeFloat3:
		return PropertyMaterialParameterFloat3;
	case MaterialParameter::ParameterTypeFloat4:
		if (boost::regex_search(mtl_param->m_Name, g_rcolor, boost::match_default))
		{
			return PropertyMaterialParameterColor;
		}
		else
		{
			return PropertyMaterialParameterFloat4;
		}
	case MaterialParameter::ParameterTypeTexture:
		return PropertyMaterialParameterTexture;
	}
	ASSERT(FALSE);
	return PropertyUnknown;
}

unsigned int CPropertiesWnd::GetControlPropCount(DWORD type)
{
	switch (type)
	{
	case my::Control::ControlTypeControl:
		return 19;
	case my::Control::ControlTypeStatic:
		return GetControlPropCount(my::Control::ControlTypeControl) + 10;
	case my::Control::ControlTypeButton:
		return GetControlPropCount(my::Control::ControlTypeStatic) + 12;
	}
	ASSERT(false);
	return UINT_MAX;
}

LPCTSTR CPropertiesWnd::GetControlTypeName(DWORD type)
{
	switch (type)
	{
	case my::Control::ControlTypeStatic:
		return _T("Static");
	case my::Control::ControlTypeProgressBar:
		return _T("ProgressBar");
	case my::Control::ControlTypeButton:
		return _T("Button");
	case my::Control::ControlTypeEditBox:
		return _T("EditBox");
	case my::Control::ControlTypeImeEditBox:
		return _T("ImeEditBox");
	case my::Control::ControlTypeScrollBar:
		return _T("ScrollBar");
	case my::Control::ControlTypeHorizontalScrollBar:
		return _T("HorizontalScrollBar");
	case my::Control::ControlTypeCheckBox:
		return _T("CheckBox");
	case my::Control::ControlTypeComboBox:
		return _T("ComboBox");
	case my::Control::ControlTypeListBox:
		return _T("ListBox");
	case my::Control::ControlTypeDialog:
		return _T("Dialog");
	}
	return _T("Unknown Control");
}

void CPropertiesWnd::UpdatePropertiesPaintTool(Actor* actor)
{
	CMFCPropertyGridProperty* pPaint = NULL;
	if (m_wndPropList.GetPropertyCount() >= 1)
	{
		pPaint = m_wndPropList.GetProperty(0);
	}
	if (!pPaint || pPaint->GetData() != PropertyPaint)
	{
		m_wndPropList.RemoveAll();
		CreatePropertiesPaintTool(actor);
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
	pPaint->GetSubItem(5)->SetValue((_variant_t)(long)(pFrame->m_PaintColor.a * 255));
	UpdatePropertiesSpline(pPaint->GetSubItem(6), &pFrame->m_PaintSpline);
	std::basic_string<TCHAR> emit_name = actor->m_Cmps.size() > pFrame->m_PaintEmitterSiblingId
		&& actor->m_Cmps[pFrame->m_PaintEmitterSiblingId]->GetComponentType() == Component::ComponentTypeStaticEmitter ? ms2ts(actor->m_Cmps[pFrame->m_PaintEmitterSiblingId]->GetName()) : _T("");
	pPaint->GetSubItem(7)->SetValue((_variant_t)emit_name.c_str());
	pPaint->GetSubItem(8)->SetValue((_variant_t)pFrame->m_PaintParticleMinDist);
	pPaint->GetSubItem(9)->GetSubItem(0)->SetValue((_variant_t)pFrame->m_PaintParticleAngle.x);
	pPaint->GetSubItem(9)->GetSubItem(1)->SetValue((_variant_t)pFrame->m_PaintParticleAngle.y);
}

void CPropertiesWnd::CreatePropertiesPaintTool(Actor* actor)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMFCPropertyGridProperty* pPaint = new CSimpleProp(g_PaintType[pFrame->m_PaintType], PropertyPaint, FALSE);
	m_wndPropList.AddProperty(pPaint, FALSE, FALSE);

	CMFCPropertyGridProperty* pProp = new CComboProp(_T("Shape"), g_PaintShape[pFrame->m_PaintShape], NULL, PropertyPaintShape);
	for (unsigned int i = 0; i < _countof(g_PaintShape); i++)
	{
		pProp->AddOption(g_PaintShape[i], TRUE);
	}
	pPaint->AddSubItem(pProp);

	pProp = new CComboProp(_T("Mode"), g_PaintMode[pFrame->m_PaintMode], NULL, PropertyPaintMode);
	for (unsigned int i = 0; i < _countof(g_PaintMode); i++)
	{
		pProp->AddOption(g_PaintMode[i], TRUE);
	}
	pPaint->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("Radius"), pFrame->m_PaintRadius, NULL, PropertyPaintRadius);
	pPaint->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Height"), pFrame->m_PaintHeight, NULL, PropertyPaintHeight);
	pPaint->AddSubItem(pProp);

	COLORREF color = RGB(pFrame->m_PaintColor.r * 255, pFrame->m_PaintColor.g * 255, pFrame->m_PaintColor.b * 255);
	CColorProp* pPaintColor = new CColorProp(_T("Color"), color, NULL, NULL, PropertyPaintColor);
	pPaintColor->EnableOtherButton(_T("Other..."));
	pPaint->AddSubItem(pPaintColor);

	CMFCPropertyGridProperty* pPaintAlpha = new CSliderProp(_T("Alpha"), (long)(pFrame->m_PaintColor.a * 255), NULL, PropertyPaintAlpha);
	pPaint->AddSubItem(pPaintAlpha);

	CreatePropertiesSpline(pPaint, _T("Spline"), PropertyPaintSpline, &pFrame->m_PaintSpline);

	std::basic_string<TCHAR> emit_name = actor->m_Cmps.size() > pFrame->m_PaintEmitterSiblingId
		&& actor->m_Cmps[pFrame->m_PaintEmitterSiblingId]->GetComponentType() == Component::ComponentTypeStaticEmitter ? ms2ts(actor->m_Cmps[pFrame->m_PaintEmitterSiblingId]->GetName()) : _T("");
	pProp = new CComboProp(_T("Emitter"), emit_name.c_str(), NULL, PropertyPaintEmitterSiblingId);
	for (unsigned int i = 0; i < actor->m_Cmps.size(); i++)
	{
		if (actor->m_Cmps[i]->GetComponentType() == Component::ComponentTypeStaticEmitter)
		{
			pProp->AddOption(ms2ts(actor->m_Cmps[i]->GetName()).c_str(), TRUE);
		}
	}
	pPaint->AddSubItem(pProp);

	pProp = new CSimpleProp(_T("ParticleMinDist"), (_variant_t)pFrame->m_PaintParticleMinDist, NULL, PropertyPaintParticleMinDist);
	pPaint->AddSubItem(pProp);

	CMFCPropertyGridProperty* pParticleAngle = new CSimpleProp(_T("ParticleAngle"), PropertyPaintParticleAngle, TRUE);
	pPaint->AddSubItem(pParticleAngle);
	pProp = new CSimpleProp(_T("min"), (_variant_t)pFrame->m_PaintParticleAngle.x, NULL, PropertyPaintParticleAngleMin);
	pParticleAngle->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("max"), (_variant_t)pFrame->m_PaintParticleAngle.y, NULL, PropertyPaintParticleAngleMax);
	pParticleAngle->AddSubItem(pProp);
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

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_TOOLBAR1);
	m_wndToolBar.LoadToolBar(IDR_TOOLBAR1, 0, 0, TRUE /* Is locked */);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.connect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, boost::placeholders::_1));

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	//// TODO: Add your message handler code here
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventSelectionChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventPivotModeChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, boost::placeholders::_1));
	(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_EventAttributeChanged.disconnect(boost::bind(&CPropertiesWnd::OnSelectionChanged, this, boost::placeholders::_1));
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	if (m_wndPropList.IsAlphabeticMode())
	{
		return;
	}

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	CMainFrame::ActorList::iterator actor_iter = pFrame->m_selactors.begin();
	if (actor_iter == pFrame->m_selactors.end())
	{
		return;
	}

	for (int i = 0; i < m_wndPropList.GetPropertyCount(); i++)
	{
		CMFCPropertyGridProperty* pProp = m_wndPropList.GetProperty(i);
		for (int j = 0; j < pProp->GetSubItemsCount(); j++)
		{
			CSimpleProp* pSubItem = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetSubItem(j));
			pSubItem->ExpandDeep(FALSE);
		}
	}

	m_wndPropList.AdjustLayout();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
{
}
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
	m_wndPropList.EnableDescriptionArea(TRUE);
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
	m_OnPropertyChangeMuted = TRUE;
	BOOST_SCOPE_EXIT(&m_OnPropertyChangeMuted)
	{
		m_OnPropertyChangeMuted = FALSE;
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
		Actor* actor = (Actor*)pActor->GetValue().pulVal;
		if (PropertyId == PropertyActorScaleX)
		{
			actor->m_Scale.x = actor->m_Scale.y = actor->m_Scale.z = pActor->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
			pActor->GetSubItem(4)->GetSubItem(1)->SetValue((_variant_t)actor->m_Scale.y);
			pActor->GetSubItem(4)->GetSubItem(2)->SetValue((_variant_t)actor->m_Scale.z);
			m_wndPropList.InvalidateRect(pActor->GetSubItem(4)->GetSubItem(1)->GetRect());
			m_wndPropList.InvalidateRect(pActor->GetSubItem(4)->GetSubItem(2)->GetRect());
		}
		else
		{
			actor->m_Scale.x = pActor->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
			actor->m_Scale.y = pActor->GetSubItem(4)->GetSubItem(1)->GetValue().fltVal;
			actor->m_Scale.z = pActor->GetSubItem(4)->GetSubItem(2)->GetValue().fltVal;
		}
		actor->SetPose(
			my::Vector3(
				pActor->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal,
				pActor->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal,
				pActor->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal),
			my::Quaternion::RotationEulerAngles(
				D3DXToRadian(pActor->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal),
				D3DXToRadian(pActor->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal),
				D3DXToRadian(pActor->GetSubItem(3)->GetSubItem(2)->GetValue().fltVal)));
		actor->SetPxPoseOrbyPxThread(actor->m_Position, actor->m_Rotation, NULL);
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();

		//Actor::ComponentPtrList::iterator cmp_iter = actor->m_Cmps.begin();
		//for (; cmp_iter != actor->m_Cmps.end(); cmp_iter++)
		//{
		//	if ((*cmp_iter)->GetComponentType() == Component::ComponentTypeStaticEmitter
		//		&& dynamic_cast<StaticEmitter*>(cmp_iter->get())->m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld)
		//	{
		//		dynamic_cast<StaticEmitter *>(cmp_iter->get())->BuildChunks();
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
	case PropertyActorCullingDistSq:
	{
		Actor* actor = (Actor*)pProp->GetParent()->GetValue().pulVal;
		actor->m_CullingDistSq = pProp->GetValue().fltVal;
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
		}
		UpdatePropertiesRigidActor(pProp->GetParent(), actor);
		m_wndPropList.AdjustLayout();
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
		break;
	}
	case PropertyComponentLODMask:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_LodMaskDesc));
		cmp->m_LodMask = (Component::LODMask)g_LodMaskDesc[i].mask;
		cmp->m_Actor->m_Lod = Actor::MaxLod;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComponentSiblingId:
	{
		Component* cmp = (Component*)pProp->GetParent()->GetValue().pulVal;
		cmp->SetSiblingId(pProp->GetValue().uintVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyShapeType:
	{
		Component * cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		if (!cmp->m_Actor || !cmp->m_Actor->m_PxActor)
		{
			MessageBox(_T("!cmp->m_Actor || !cmp->m_Actor->m_PxActor"));
			return 0;
		}
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_ShapeTypeDesc));
		CShapeDlg dlg(pFrame, cmp, i);
		switch (i)
		{
		case physx::PxGeometryType::eSPHERE:
		case physx::PxGeometryType::ePLANE:
		case physx::PxGeometryType::eCAPSULE:
		case physx::PxGeometryType::eBOX:
			break;
		case physx::PxGeometryType::eCONVEXMESH:
		{
			MeshComponent* mesh_cmp = dynamic_cast<MeshComponent*>(cmp);
			if (!mesh_cmp || mesh_cmp->m_MeshPath.empty())
			{
				MessageBox(_T("!mesh_cmp || mesh_cmp->m_MeshPath.empty()"));
				return 0;
			}
			dlg.m_AssetPath = ms2ts((mesh_cmp->m_MeshPath + ".pxconvexmesh").c_str()).c_str();
			break;
		}
		case physx::PxGeometryType::eTRIANGLEMESH:
		{
			MeshComponent* mesh_cmp = dynamic_cast<MeshComponent*>(cmp);
			if (!mesh_cmp || mesh_cmp->m_MeshPath.empty())
			{
				MessageBox(_T("!mesh_cmp || mesh_cmp->m_MeshPath.empty()"));
				return 0;
			}
			dlg.m_AssetPath = ms2ts((mesh_cmp->m_MeshPath + ".pxtrianglemesh").c_str()).c_str();
			break;
		}
		case physx::PxGeometryType::eHEIGHTFIELD:
		{
			Terrain* terrain = dynamic_cast<Terrain*>(cmp);
			if (!terrain || terrain->m_ChunkPath.empty())
			{
				MessageBox(_T("!terrain || terrain->m_ChunkPath.empty()"));
				return 0;
			}
			dlg.m_AssetPath = ms2ts((terrain->m_ChunkPath + ".pxheightfield").c_str()).c_str();
			break;
		}
		default:
		{
			cmp->ClearShape();
			UpdatePropertiesShape(pProp->GetParent(), cmp);
			m_wndPropList.AdjustLayout();
			my::EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
			return 0;
		}
		}
		if (dlg.DoModal() != IDOK)
		{
			return 0;
		}
		UpdatePropertiesShape(pProp->GetParent(), cmp);
		m_wndPropList.AdjustLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
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
	{
		//MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().ulVal);
		//std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
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
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 1)))->GetColor();
		mesh_cmp->m_MeshColor.x = GetRValue(color) / 255.0f;
		mesh_cmp->m_MeshColor.y = GetGValue(color) / 255.0f;
		mesh_cmp->m_MeshColor.z = GetBValue(color) / 255.0f;
		mesh_cmp->m_MeshColor.w = pProp->GetParent()->GetSubItem(PropId + 2)->GetValue().lVal / 255.0f;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMeshSubMeshId:
	case PropertyMeshNumVert:
	case PropertyMeshNumFace:
		break;
	case PropertyMeshInstanceType:
	{
		MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>((Component *)pProp->GetParent()->GetValue().pulVal);
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_InstanceTypeDesc));
		mesh_cmp->m_InstanceType = (MeshComponent::InstanceType)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);

		// ! reset shader handles
		mesh_cmp->OnResetShader();
		mesh_cmp->m_Material->OnResetShader();
		break;
	}
	case PropertyMaterialShader:
	{
		Material* material = (Material*)pProp->GetParent()->GetValue().pulVal;
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
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

		// ! reset shader handlers of mesh or terrain
		//switch (pProp->GetParent()->GetParent()->GetData())
		//{
		//case PropertyMesh:
		//case PropertyCloth:
		//case PropertyStaticEmitter:
		//case PropertySphericalEmitter:
		//case PropertyTerrain:
		//{
			Component* cmp = (Component*)pProp->GetParent()->GetParent()->GetValue().pulVal;
			cmp->OnResetShader();
		//	break;
		//}
		//}
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
	case PropertyMaterialZFunc:
	{
		Material * material = (Material *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_ZFuncDesc));
		material->m_ZFunc = i + 1;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialAlphaTestEnable:
	{
		Material* material = (Material*)pProp->GetParent()->GetValue().pulVal;
		material->m_AlphaTestEnable = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialAlphaRef:
	{
		Material* material = (Material*)pProp->GetParent()->GetValue().pulVal;
		material->m_AlphaRef = pProp->GetValue().uintVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialAlphaFunc:
	{
		Material* material = (Material*)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_ZFuncDesc));
		material->m_AlphaFunc = i + 1;
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
	case PropertyMaterialParameterInt2:
	case PropertyMaterialParameterIntValueX:
	case PropertyMaterialParameterIntValueY:
	{
		CMFCPropertyGridProperty* pParameter = NULL;
		switch (PropertyId)
		{
		case PropertyMaterialParameterInt2:
			pParameter = pProp;
			break;
		case PropertyMaterialParameterIntValueX:
		case PropertyMaterialParameterIntValueY:
			pParameter = pProp->GetParent();
			break;
		}
		ASSERT(pParameter);
		Material* mtl = (Material*)pParameter->GetParent()->GetParent()->GetValue().pulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pParameter);
		ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeInt2);
		boost::dynamic_pointer_cast<MaterialParameterInt2>(mtl->m_ParameterList[i])->m_Value.SetPoint(
			pParameter->GetSubItem(0)->GetValue().intVal, pParameter->GetSubItem(1)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterFloat:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeFloat);
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
	case PropertyMaterialParameterColor:
	{
		CMFCPropertyGridProperty * pParameter = NULL;
		switch (PropertyId)
		{
		case PropertyMaterialParameterFloat2:
		case PropertyMaterialParameterFloat3:
		case PropertyMaterialParameterFloat4:
		case PropertyMaterialParameterColor:
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
			ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeFloat2);
			boost::dynamic_pointer_cast<MaterialParameterFloat2>(mtl->m_ParameterList[i])->m_Value = my::Vector2(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal);
			break;
		case PropertyMaterialParameterFloat3:
			ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeFloat3);
			boost::dynamic_pointer_cast<MaterialParameterFloat3>(mtl->m_ParameterList[i])->m_Value = my::Vector3(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal, pParameter->GetSubItem(2)->GetValue().fltVal);
			break;
		case PropertyMaterialParameterFloat4:
			ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeFloat4);
			boost::dynamic_pointer_cast<MaterialParameterFloat4>(mtl->m_ParameterList[i])->m_Value = my::Vector4(
				pParameter->GetSubItem(0)->GetValue().fltVal, pParameter->GetSubItem(1)->GetValue().fltVal, pParameter->GetSubItem(2)->GetValue().fltVal, pParameter->GetSubItem(3)->GetValue().fltVal);
			break;
		case PropertyMaterialParameterColor:
		{
			ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeFloat3);
			COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pParameter))->GetColor();
			boost::dynamic_pointer_cast<MaterialParameterFloat3>(mtl->m_ParameterList[i])->m_Value = my::Vector3(
				GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
			break;
		}
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyMaterialParameterTexture:
	{
		Material * mtl = (Material *)pProp->GetParent()->GetParent()->GetValue().pulVal;
		INT i = CSimpleProp::GetSubIndexInParent(pProp);
		ASSERT(mtl->m_ParameterList[i]->GetParameterType() == MaterialParameter::ParameterTypeTexture);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
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
	case PropertyStaticMeshChunkWidth:
	case PropertyStaticMeshChunkPath:
		break;
	case PropertyStaticMeshChunkLodScale:
	{
		StaticMesh* static_mesh_cmp = (StaticMesh*)pProp->GetParent()->GetValue().pulVal;
		static_mesh_cmp->m_ChunkLodScale = pProp->GetValue().fltVal;
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
	case PropertyClothSweptContact:
	{
		ClothComponent* cloth_cmp = (ClothComponent*)pProp->GetParent()->GetValue().pulVal;
		cloth_cmp->m_Cloth->setClothFlag(physx::PxClothFlag::eSWEPT_CONTACT, pProp->GetValue().boolVal != 0);
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
	case PropertyClothSolverFrequency:
	{
		ClothComponent* cloth_cmp = (ClothComponent*)pProp->GetParent()->GetValue().pulVal;
		cloth_cmp->m_Cloth->setSolverFrequency(pProp->GetValue().fltVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyClothStiffnessFrequency:
	{
		ClothComponent* cloth_cmp = (ClothComponent*)pProp->GetParent()->GetValue().pulVal;
		cloth_cmp->m_Cloth->setStiffnessFrequency(pProp->GetValue().fltVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyClothCollisionSpheresNum:
	case PropertyClothCollisionCapsulesNum:
	case PropertyClothVirtualParticleNum:
		break;
	case PropertyClothVirtualParticleLevel:
	{
		ClothComponent* cloth_cmp = (ClothComponent*)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		cloth_cmp->CreateVirtualParticles(i);
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		pProp->GetParent()->GetSubItem(PropId + 9)->SetValue((_variant_t)cloth_cmp->m_Cloth->getNbVirtualParticles());
		m_wndPropList.InvalidateRect(pProp->GetParent()->GetSubItem(PropId + 9)->GetRect());
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyClothStretchVertical:
	case PropertyClothStretchVerticalStiffness:
	case PropertyClothStretchVerticalStiffnessMultiplier:
	case PropertyClothStretchVerticalCompressionLimit:
	case PropertyClothStretchVerticalStretchLimit:
	{
		CMFCPropertyGridProperty* pStretchVertical = NULL;
		switch (PropertyId)
		{
		case PropertyClothStretchVertical:
			pStretchVertical = pProp;
			break;
		case PropertyClothStretchVerticalStiffness:
		case PropertyClothStretchVerticalStiffnessMultiplier:
		case PropertyClothStretchVerticalCompressionLimit:
		case PropertyClothStretchVerticalStretchLimit:
			pStretchVertical = pProp->GetParent();
			break;
		}
		ASSERT(pStretchVertical);
		ClothComponent* cloth_cmp = (ClothComponent*)pStretchVertical->GetParent()->GetValue().pulVal;
		cloth_cmp->m_Cloth->setStretchConfig(physx::PxClothFabricPhaseType::eVERTICAL, physx::PxClothStretchConfig(
			pStretchVertical->GetSubItem(0)->GetValue().fltVal,
			pStretchVertical->GetSubItem(1)->GetValue().fltVal,
			pStretchVertical->GetSubItem(2)->GetValue().fltVal,
			pStretchVertical->GetSubItem(3)->GetValue().fltVal));
		break;
	}
	case PropertyClothTether:
	case PropertyClothTetherStiffness:
	case PropertyClothTetherStretchLimit:
	{
		CMFCPropertyGridProperty* pTether = NULL;
		switch (PropertyId)
		{
		case PropertyClothTether:
			pTether = pProp;
			break;
		case PropertyClothTetherStiffness:
		case PropertyClothTetherStretchLimit:
			pTether = pProp->GetParent();
			break;
		}
		ASSERT(pTether);
		ClothComponent* cloth_cmp = (ClothComponent*)pTether->GetParent()->GetValue().pulVal;
		cloth_cmp->m_Cloth->setTetherConfig(physx::PxClothTetherConfig(
			pTether->GetSubItem(0)->GetValue().fltVal,
			pTether->GetSubItem(1)->GetValue().fltVal));
		break;
	}
	case PropertyClothExternalAcceleration:
	case PropertyClothExternalAccelerationX:
	case PropertyClothExternalAccelerationY:
	case PropertyClothExternalAccelerationZ:
	{
		CMFCPropertyGridProperty* pExternalAcceleration = NULL;
		switch (PropertyId)
		{
		case PropertyClothExternalAcceleration:
			pExternalAcceleration = pProp;
			break;
		case PropertyClothExternalAccelerationX:
		case PropertyClothExternalAccelerationY:
		case PropertyClothExternalAccelerationZ:
			pExternalAcceleration = pProp->GetParent();
			break;
		}
		ASSERT(pExternalAcceleration);
		ClothComponent* cloth_cmp = (ClothComponent*)pExternalAcceleration->GetParent()->GetValue().pulVal;
		cloth_cmp->SetExternalAcceleration(my::Vector3(
			pExternalAcceleration->GetSubItem(0)->GetValue().fltVal,
			pExternalAcceleration->GetSubItem(1)->GetValue().fltVal,
			pExternalAcceleration->GetSubItem(2)->GetValue().fltVal));
		break;
	}
	case PropertyEmitterFaceType:
	{
		EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_EmitterFaceType));
		emit_cmp->m_EmitterFaceType = (EmitterComponent::FaceType)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);

		// ! reset shader handles
		emit_cmp->OnResetShader();
		emit_cmp->m_Material->OnResetShader();
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
	case PropertyEmitterTiles:
	case PropertyEmitterTilesX:
	case PropertyEmitterTilesY:
	{
		CMFCPropertyGridProperty* pTiles = NULL;
		switch (PropertyId)
		{
		case PropertyEmitterTiles:
			pTiles = pProp;
			break;
		case PropertyEmitterTilesX:
		case PropertyEmitterTilesY:
			pTiles = pProp->GetParent();
			break;
		}
		ASSERT(pTiles);
		EmitterComponent* emit_cmp = (EmitterComponent*)pTiles->GetParent()->GetValue().pulVal;
		emit_cmp->m_Tiles.x = my::Max(1, pTiles->GetSubItem(0)->GetValue().intVal);
		emit_cmp->m_Tiles.y = my::Max(1, pTiles->GetSubItem(1)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);

		// ! reset shader handles
		emit_cmp->OnResetShader();
		emit_cmp->m_Material->OnResetShader();
		break;
	}
	case PropertyEmitterPrimitiveType:
	{
		EmitterComponent* emit_cmp = (EmitterComponent*)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_EmitterPrimitiveType));
		if (i == EmitterComponent::PrimitiveTypeMesh)
		{
			CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
			if (dlg.DoModal() != IDOK)
			{
				return 0;
			}

			bool requested = emit_cmp->IsRequested();
			if (requested)
			{
				emit_cmp->ReleaseResource();
			}
			emit_cmp->m_MeshPath = theApp.GetRelativePath((LPCTSTR)dlg.GetPathName());
			emit_cmp->m_MeshSubMeshId = 0;
			if (requested)
			{
				emit_cmp->RequestResource();
			}
		}
		emit_cmp->m_ParticlePrimitiveType = (EmitterComponent::PrimitiveType)i;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEmitterMeshPath:
	case PropertyEmitterMeshSubMeshId:
	case PropertyEmitterMeshNumVert:
	case PropertyEmitterMeshNumFace:
		break;
	case PropertyEmitterParticlePosition:
	case PropertyEmitterParticlePositionX:
	case PropertyEmitterParticlePositionY:
	case PropertyEmitterParticlePositionZ:
	case PropertyEmitterParticlePositionW:
	case PropertyEmitterParticleVelocity:
	case PropertyEmitterParticleVelocityX:
	case PropertyEmitterParticleVelocityY:
	case PropertyEmitterParticleVelocityZ:
	case PropertyEmitterParticleVelocityW:
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
		case PropertyEmitterParticlePositionW:
		case PropertyEmitterParticleVelocityX:
		case PropertyEmitterParticleVelocityY:
		case PropertyEmitterParticleVelocityZ:
		case PropertyEmitterParticleVelocityW:
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
		CPoint chunkid(LOWORD(pParticle->GetData()), HIWORD(pParticle->GetData()));
		StaticEmitter* emit_cmp = (StaticEmitter*)pParticle->GetParent()->GetValue().pulVal;
		StaticEmitter::ChunkMap::iterator chunk_iter = emit_cmp->m_Chunks.find(std::make_pair(chunkid.x, chunkid.y));
		if (chunk_iter == emit_cmp->m_Chunks.end())
		{
			MessageBox(_T("chunk_iter == emit_cmp->m_Chunks.end()"));
			return 0;
		}
		int instid = pParticle->GetValue().intVal;
		StaticEmitterStream estr(emit_cmp);
		StaticEmitterChunkBuffer * buff = estr.GetBuffer(chunkid.x, chunkid.y);
		if (!buff || instid >= buff->size())
		{
			MessageBox(_T("!buff || instid >= buff->size()"));
			return 0;
		}
		my::Emitter::Particle * particle = &(*buff)[instid];
		particle->m_Position.x = pParticle->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal;
		particle->m_Position.y = pParticle->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal;
		particle->m_Position.z = pParticle->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal;
		particle->m_Position.w = pParticle->GetSubItem(0)->GetSubItem(3)->GetValue().fltVal;
		particle->m_Velocity.x = pParticle->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
		particle->m_Velocity.y = pParticle->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
		particle->m_Velocity.z = pParticle->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
		particle->m_Velocity.w = pParticle->GetSubItem(1)->GetSubItem(3)->GetValue().fltVal;
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pParticle->GetSubItem(2)))->GetColor();
		particle->m_Color.x = GetRValue(color) / 255.0f;
		particle->m_Color.y = GetGValue(color) / 255.0f;
		particle->m_Color.z = GetBValue(color) / 255.0f;
		particle->m_Color.w = pParticle->GetSubItem(3)->GetValue().lVal / 255.0f;
		particle->m_Size.x = pParticle->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
		particle->m_Size.y = pParticle->GetSubItem(4)->GetSubItem(1)->GetValue().fltVal;
		particle->m_Angle = D3DXToRadian(pParticle->GetSubItem(5)->GetValue().fltVal);
		estr.m_dirty[std::make_pair(chunkid.x, chunkid.y)] = true;
		estr.Flush();
		Actor * actor = emit_cmp->m_Actor;
		actor->UpdateAABB();
		actor->UpdateOctNode();
		pFrame->UpdateSelBox();
		pFrame->UpdatePivotTransform();
		m_wndPropList.AdjustLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticEmitterChunkWidth:
	case PropertyStaticEmitterChunkPath:
		break;
	case PropertyStaticEmitterChunkLodScale:
	{
		StaticEmitter* emit_cmp = (StaticEmitter*)pProp->GetParent()->GetValue().pulVal;
		emit_cmp->m_ChunkLodScale = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertySphericalEmitterParticleCapacity:
	{
		SphericalEmitter * sphe_emit_cmp = (SphericalEmitter *)pProp->GetParent()->GetValue().pulVal;
		unsigned int new_size = pProp->GetValue().uintVal;
		sphe_emit_cmp->m_ParticleList.set_capacity(new_size);
		UpdatePropertiesSphericalEmitter(pProp->GetParent(), sphe_emit_cmp);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertySphericalEmitterSpawnInterval:
	case PropertySphericalEmitterSpawnCount:
	case PropertySphericalEmitterHalfSpawnArea:
	case PropertySphericalEmitterHalfSpawnAreaX:
	case PropertySphericalEmitterHalfSpawnAreaY:
	case PropertySphericalEmitterHalfSpawnAreaZ:
	case PropertySphericalEmitterSpawnInclination:
	case PropertySphericalEmitterSpawnInclinationX:
	case PropertySphericalEmitterSpawnInclinationY:
	case PropertySphericalEmitterSpawnAzimuth:
	case PropertySphericalEmitterSpawnAzimuthX:
	case PropertySphericalEmitterSpawnAzimuthY:
	case PropertySphericalEmitterSpawnSpeed:
	case PropertySphericalEmitterSpawnBoneId:
	case PropertySphericalEmitterSpawnLocalPos:
	case PropertySphericalEmitterSpawnLocalPosX:
	case PropertySphericalEmitterSpawnLocalPosY:
	case PropertySphericalEmitterSpawnLocalPosZ:
	case PropertySphericalEmitterSpawnLocalRot:
	case PropertySphericalEmitterSpawnLocalRotX:
	case PropertySphericalEmitterSpawnLocalRotY:
	case PropertySphericalEmitterSpawnLocalRotZ:
	case PropertySphericalEmitterParticleLifeTime:
	case PropertySphericalEmitterParticleGravity:
	case PropertySphericalEmitterParticleGravityX:
	case PropertySphericalEmitterParticleGravityY:
	case PropertySphericalEmitterParticleGravityZ:
	case PropertySphericalEmitterParticleDamping:
	{
		CMFCPropertyGridProperty* pComponent = NULL;
		switch (PropertyId)
		{
		case PropertySphericalEmitterHalfSpawnAreaX:
		case PropertySphericalEmitterHalfSpawnAreaY:
		case PropertySphericalEmitterHalfSpawnAreaZ:
		case PropertySphericalEmitterSpawnInclinationX:
		case PropertySphericalEmitterSpawnInclinationY:
		case PropertySphericalEmitterSpawnAzimuthX:
		case PropertySphericalEmitterSpawnAzimuthY:
		case PropertySphericalEmitterSpawnLocalPosX:
		case PropertySphericalEmitterSpawnLocalPosY:
		case PropertySphericalEmitterSpawnLocalPosZ:
		case PropertySphericalEmitterSpawnLocalRotX:
		case PropertySphericalEmitterSpawnLocalRotY:
		case PropertySphericalEmitterSpawnLocalRotZ:
		case PropertySphericalEmitterParticleGravityX:
		case PropertySphericalEmitterParticleGravityY:
		case PropertySphericalEmitterParticleGravityZ:
			pComponent = pProp->GetParent()->GetParent();
			break;
		default:
			pComponent = pProp->GetParent();
			break;
		}
		SphericalEmitter* sphe_emit_cmp = (SphericalEmitter*)pComponent->GetValue().pulVal;
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeEmitter);
		sphe_emit_cmp->m_SpawnInterval = pComponent->GetSubItem(PropId + 1)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnCount = pComponent->GetSubItem(PropId + 2)->GetValue().intVal;
		sphe_emit_cmp->m_HalfSpawnArea.x = pComponent->GetSubItem(PropId + 3)->GetSubItem(0)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.y = pComponent->GetSubItem(PropId + 3)->GetSubItem(1)->GetValue().fltVal;
		sphe_emit_cmp->m_HalfSpawnArea.z = pComponent->GetSubItem(PropId + 3)->GetSubItem(2)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnInclination.x = D3DXToRadian(pComponent->GetSubItem(PropId + 4)->GetSubItem(0)->GetValue().fltVal);
		sphe_emit_cmp->m_SpawnInclination.y = D3DXToRadian(pComponent->GetSubItem(PropId + 4)->GetSubItem(1)->GetValue().fltVal);
		sphe_emit_cmp->m_SpawnAzimuth.x = D3DXToRadian(pComponent->GetSubItem(PropId + 5)->GetSubItem(0)->GetValue().fltVal);
		sphe_emit_cmp->m_SpawnAzimuth.y = D3DXToRadian(pComponent->GetSubItem(PropId + 5)->GetSubItem(1)->GetValue().fltVal);
		sphe_emit_cmp->m_SpawnSpeed = pComponent->GetSubItem(PropId + 6)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnBoneId = pComponent->GetSubItem(PropId + 7)->GetValue().intVal;
		sphe_emit_cmp->m_SpawnLocalPose.m_position.x = pComponent->GetSubItem(PropId + 8)->GetSubItem(0)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnLocalPose.m_position.y = pComponent->GetSubItem(PropId + 8)->GetSubItem(1)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnLocalPose.m_position.z = pComponent->GetSubItem(PropId + 8)->GetSubItem(2)->GetValue().fltVal;
		sphe_emit_cmp->m_SpawnLocalPose.m_rotation = my::Quaternion::RotationEulerAngles(
			D3DXToRadian(pComponent->GetSubItem(PropId + 9)->GetSubItem(0)->GetValue().fltVal),
			D3DXToRadian(pComponent->GetSubItem(PropId + 9)->GetSubItem(1)->GetValue().fltVal),
			D3DXToRadian(pComponent->GetSubItem(PropId + 9)->GetSubItem(2)->GetValue().fltVal));
		sphe_emit_cmp->m_ParticleLifeTime = pComponent->GetSubItem(PropId + 10)->GetValue().fltVal;
		sphe_emit_cmp->m_ParticleGravity.x = pComponent->GetSubItem(PropId + 11)->GetSubItem(0)->GetValue().fltVal;
		sphe_emit_cmp->m_ParticleGravity.y = pComponent->GetSubItem(PropId + 11)->GetSubItem(1)->GetValue().fltVal;
		sphe_emit_cmp->m_ParticleGravity.z = pComponent->GetSubItem(PropId + 11)->GetSubItem(2)->GetValue().fltVal;
		sphe_emit_cmp->m_ParticleDamping = pComponent->GetSubItem(PropId + 12)->GetValue().fltVal;
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
			spline->resize(pProp->GetValue().uintVal, spline->empty() ? my::Spline::value_type(0.0f, 1.0f, 0.0f, 0.0f) : spline->back());
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
			my::Spline::value_type & node = (*spline)[NodeId];
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
	case PropertyTerrainRowChunks:
	case PropertyTerrainColChunks:
	case PropertyTerrainChunkSize:
	case PropertyTerrainChunkPath:
		break;
	case PropertyTerrainChunkLodScale:
	{
		Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().pulVal;
		terrain->m_ChunkLodScale = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyTerrainHeightMap:
	{
		Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().pulVal;
		ImportHeightDlg dlg;
		dlg.m_AssetPath = pProp->GetValue().bstrVal;
		if (!dlg.m_AssetPath.IsEmpty())
		{
			my::Texture2D tex;
			tex.CreateTextureFromFile(dlg.m_AssetPath);
			D3DSURFACE_DESC desc = tex.GetLevelDesc(0);
			if (desc.Format != D3DFMT_L16)
			{
				MessageBox(_T("desc.Format != D3DFMT_L16"));
				return 0;
			}
			dlg.m_TextureSize.SetSize(desc.Width, desc.Height);
			if (dlg.DoModal() != IDOK)
			{
				return 0;
			}
			D3DLOCKED_RECT lrc = tex.LockRect(NULL, D3DLOCK_READONLY, 0);
			my::BilinearFiltering<unsigned short> sampler((unsigned short*)lrc.pBits, lrc.Pitch, desc.Width, desc.Height);
			TerrainStream tstr(terrain);
			for (int i = 0; i < terrain->m_RowChunks * terrain->m_ChunkSize + 1; i++)
			{
				for (int j = 0; j < terrain->m_ColChunks * terrain->m_ChunkSize + 1; j++)
				{
					unsigned short pixel = sampler.Sample((j + 0.5f) / (terrain->m_ColChunks * terrain->m_ChunkSize + 1), (i + 0.5f) / (terrain->m_RowChunks * terrain->m_ChunkSize + 1));
					tstr.SetPos(i, j, ((float)pixel / USHRT_MAX * theApp.default_terrain_max_height - theApp.default_terrain_water_level) / terrain->m_Actor->m_Scale.y);
				}
			}
			tex.UnlockRect(0);
			tstr.Flush();
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
			D3DSURFACE_DESC desc = tex.GetLevelDesc(0);
			if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8)
			{
				MessageBox(_T("desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8"));
				return 0;
			}
			D3DLOCKED_RECT lrc = tex.LockRect(NULL, D3DLOCK_READONLY, 0);
			my::BilinearFiltering<D3DCOLOR> sampler((D3DCOLOR*)lrc.pBits, lrc.Pitch, desc.Width, desc.Height);
			for (int i = 0; i < terrain->m_RowChunks * terrain->m_ChunkSize + 1; i++)
			{
				for (int j = 0; j < terrain->m_ColChunks * terrain->m_ChunkSize + 1; j++)
				{
					D3DXCOLOR pixel = sampler.Sample((j + 0.5f) / (terrain->m_ColChunks * terrain->m_ChunkSize + 1), (i + 0.5f) / (terrain->m_RowChunks * terrain->m_ChunkSize + 1));
					switch (desc.Format)
					{
					case D3DFMT_X8R8G8B8:
						tstr.SetColor(i, j, D3DCOLOR_ARGB(255 - LOBYTE(pixel >> 16) - LOBYTE(pixel >> 8) - LOBYTE(pixel), LOBYTE(pixel >> 16), LOBYTE(pixel >> 8), LOBYTE(pixel)));
						break;
					case D3DFMT_A8R8G8B8:
						tstr.SetColor(i, j, pixel);
						break;
					}
				}
			}
			tex.UnlockRect(0);
			tstr.Flush();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyAnimatorSkeletonPath:
	{
		break;
	}
	case PropertyAnimatorRootPosition:
	case PropertyAnimatorRootPositionX:
	case PropertyAnimatorRootPositionY:
	case PropertyAnimatorRootPositionZ:
	case PropertyAnimatorRootRotation:
	case PropertyAnimatorRootRotationX:
	case PropertyAnimatorRootRotationY:
	case PropertyAnimatorRootRotationZ:
	{
		CMFCPropertyGridProperty* pComponent = NULL;
		switch (PropertyId)
		{
		case PropertyAnimatorRootPositionX:
		case PropertyAnimatorRootPositionY:
		case PropertyAnimatorRootPositionZ:
		case PropertyAnimatorRootRotationX:
		case PropertyAnimatorRootRotationY:
		case PropertyAnimatorRootRotationZ:
			pComponent = pProp->GetParent()->GetParent();
			break;
		default:
			pComponent = pProp->GetParent();
			break;
		}
		Animator* animator = (Animator*)pComponent->GetValue().pulVal;
		unsigned int PropId = GetComponentPropCount(Component::ComponentTypeComponent);
		animator->m_RootBone.m_position.x = pComponent->GetSubItem(PropId + 1)->GetSubItem(0)->GetValue().fltVal;
		animator->m_RootBone.m_position.y = pComponent->GetSubItem(PropId + 1)->GetSubItem(1)->GetValue().fltVal;
		animator->m_RootBone.m_position.z = pComponent->GetSubItem(PropId + 1)->GetSubItem(2)->GetValue().fltVal;
		animator->m_RootBone.m_rotation = my::Quaternion::RotationEulerAngles(
			D3DXToRadian(pComponent->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().fltVal),
			D3DXToRadian(pComponent->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().fltVal),
			D3DXToRadian(pComponent->GetSubItem(PropId + 2)->GetSubItem(2)->GetValue().fltVal));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyAnimationNodeSequenceName:
	{
		CMFCPropertyGridProperty* pAnimatioNode = pProp->GetParent();
		ASSERT(pAnimatioNode->GetData() == PropertyAnimationNode);
		AnimationNodeSequence* node = dynamic_cast<AnimationNodeSequence*>((AnimationNode*)pAnimatioNode->GetValue().pulVal);
		ASSERT(node);
		node->m_Name = ts2ms(pProp->GetValue().bstrVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyAnimationNodeSequenceRate:
	{
		AnimationNodeSequence* node = dynamic_cast<AnimationNodeSequence*>((AnimationNode*)pProp->GetParent()->GetValue().pulVal);
		node->m_Rate = pProp->GetValue().fltVal;
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
	case PropertyPaintAlpha:
	{
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(4)))->GetColor();
		pFrame->m_PaintColor.r = GetRValue(color) / 255.0f;
		pFrame->m_PaintColor.g = GetGValue(color) / 255.0f;
		pFrame->m_PaintColor.b = GetBValue(color) / 255.0f;
		pFrame->m_PaintColor.a = pProp->GetParent()->GetSubItem(5)->GetValue().lVal / 255.0f;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintEmitterSiblingId:
	{
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		StaticEmitter* emit = dynamic_cast<StaticEmitter*>(theApp.GetNamedObject(ts2ms((DYNAMIC_DOWNCAST(CComboProp, pProp))->GetOption(i)).c_str()));
		ASSERT(emit);
		pFrame->m_PaintEmitterSiblingId = emit->GetSiblingId();
		break;
	}
	case PropertyPaintParticleMinDist:
	{
		pFrame->m_PaintParticleMinDist = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyPaintParticleAngle:
	case PropertyPaintParticleAngleMin:
	case PropertyPaintParticleAngleMax:
	{
		CMFCPropertyGridProperty* pParticleAngle = NULL;
		switch (PropertyId)
		{
		case PropertyPaintParticleAngle:
			pParticleAngle = pProp;
			break;
		default:
			pParticleAngle = pProp->GetParent();
			break;
		}
		pFrame->m_PaintParticleAngle.x = pParticleAngle->GetSubItem(0)->GetValue().fltVal;
		pFrame->m_PaintParticleAngle.y = pParticleAngle->GetSubItem(1)->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlName:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		std::string Name = ts2ms(pProp->GetValue().bstrVal);
		if (theApp.GetNamedObject(Name.c_str()))
		{
			MessageBox(str_printf(_T("%s already existed"), pProp->GetValue().bstrVal).c_str());
			pProp->SetValue((_variant_t)ms2ts(control->GetName()).c_str());
			return 0;
		}
		control->SetName(Name.c_str());
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlX:
	case PropertyControlXScale:
	case PropertyControlXOffset:
	case PropertyControlY:
	case PropertyControlYScale:
	case PropertyControlYOffset:
	case PropertyControlWidth:
	case PropertyControlWidthScale:
	case PropertyControlWidthOffset:
	case PropertyControlHeight:
	case PropertyControlHeightScale:
	case PropertyControlHeightOffset:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyControlXScale:
		case PropertyControlXOffset:
		case PropertyControlYScale:
		case PropertyControlYOffset:
		case PropertyControlWidthScale:
		case PropertyControlWidthOffset:
		case PropertyControlHeightScale:
		case PropertyControlHeightOffset:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyControlX:
		case PropertyControlY:
		case PropertyControlWidth:
		case PropertyControlHeight:
			pControl = pProp->GetParent();
			break;
		}
		my::Control* control = (my::Control*)pControl->GetValue().pulVal;
		control->m_x.scale = pControl->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
		control->m_x.offset = pControl->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
		control->m_y.scale = pControl->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal;
		control->m_y.offset = pControl->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal;
		control->m_Width.scale = pControl->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal;
		control->m_Width.offset = pControl->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal;
		control->m_Height.scale = pControl->GetSubItem(4)->GetSubItem(0)->GetValue().fltVal;
		control->m_Height.offset = pControl->GetSubItem(4)->GetSubItem(1)->GetValue().fltVal;
		control->OnLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlEnabled:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		control->SetEnabled(pProp->GetValue().boolVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlVisible:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		control->m_bVisible = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlFocused:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		if (pProp->GetValue().boolVal)
		{
			my::Control::SetFocusControl(control);
		}
		else
		{
			my::Control::SetFocusControl(NULL);
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlSiblingId:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		control->SetSiblingId(pProp->GetValue().uintVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlColor:
	case PropertyControlColorAlpha:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(9)))->GetColor();
		BYTE alpha = pProp->GetParent()->GetSubItem(10)->GetValue().lVal;
		control->m_Skin->m_Color = D3DCOLOR_ARGB(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlImagePath:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(control);
			return 0;
		}
		control->m_Skin->m_Image->ReleaseResource();
		control->m_Skin->m_Image->m_TexturePath = path;
		if (control->m_Skin->IsRequested())
		{
			control->m_Skin->m_Image->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlImageRect:
	case PropertyControlImageRectLeft:
	case PropertyControlImageRectTop:
	case PropertyControlImageRectWidth:
	case PropertyControlImageRectHeight:
	case PropertyControlImageBorder:
	case PropertyControlImageBorderX:
	case PropertyControlImageBorderY:
	case PropertyControlImageBorderZ:
	case PropertyControlImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyControlImageRectLeft:
		case PropertyControlImageRectTop:
		case PropertyControlImageRectWidth:
		case PropertyControlImageRectHeight:
		case PropertyControlImageBorderX:
		case PropertyControlImageBorderY:
		case PropertyControlImageBorderZ:
		case PropertyControlImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyControlImageRect:
		case PropertyControlImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::Control* control = (my::Control *)pControl->GetValue().pulVal;
		control->m_Skin->m_Image->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(12)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(12)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(12)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(12)->GetSubItem(3)->GetValue().intVal));
		control->m_Skin->m_Image->m_Border.SetRect(
			pControl->GetSubItem(13)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(13)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(13)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(13)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyControlVisibleShowSoundPath:
	case PropertyControlVisibleHideSoundPath:
	case PropertyControlMouseEnterSoundPath:
	case PropertyControlMouseLeaveSoundPath:
	case PropertyControlMouseClickSoundPath:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(control);
			return 0;
		}
		control->m_Skin->ReleaseResource();
		switch (PropertyId)
		{
		case PropertyControlVisibleShowSoundPath:
			control->m_Skin->m_VisibleShowSoundPath = path;
			break;
		case PropertyControlVisibleHideSoundPath:
			control->m_Skin->m_VisibleHideSoundPath = path;
			break;
		case PropertyControlMouseEnterSoundPath:
			control->m_Skin->m_MouseEnterSoundPath = path;
			break;
		case PropertyControlMouseLeaveSoundPath:
			control->m_Skin->m_MouseLeaveSoundPath = path;
			break;
		case PropertyControlMouseClickSoundPath:
			control->m_Skin->m_MouseClickSoundPath = path;
			break;
		}
		if (control->IsRequested())
		{
			control->m_Skin->RequestResource();
		}
		break;
	}
	case PropertyStaticText:
	{
		my::Static* static_ctl = dynamic_cast<my::Static*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		static_ctl->m_Text = boost::algorithm::replace_all_copy(ts2ws(std::basic_string<TCHAR>(pProp->GetValue().bstrVal)), L"\\n", L"\n");
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticFontPath:
	case PropertyStaticFontHeight:
	case PropertyStaticFontFaceIndex:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyStaticFontPath:
			pControl = pProp->GetParent();
			break;
		case PropertyStaticFontHeight:
			pControl = pProp->GetParent();
			break;
		case PropertyStaticFontFaceIndex:
			pControl = pProp->GetParent();
			break;
		}
		my::Control* control = (my::Control*)pControl->GetValue().pulVal;
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		std::string path = theApp.GetRelativePath(pControl->GetSubItem(PropId + 1)->GetValue().bstrVal);
		if (path.empty())
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pControl->GetSubItem(PropId + 1)->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(control);
			return 0;
		}
		my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(control->m_Skin);
		skin->ReleaseResource();
		skin->m_FontPath = path;
		skin->m_FontHeight = pControl->GetSubItem(PropId + 2)->GetValue().lVal;
		skin->m_FontFaceIndex = _ttoi(pControl->GetSubItem(PropId + 3)->GetValue().bstrVal);
		if (control->IsRequested())
		{
			control->m_Skin->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticTextColor:
	case PropertyStaticTextColorAlpha:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 4)))->GetColor();
		BYTE alpha = pProp->GetParent()->GetSubItem(PropId + 5)->GetValue().lVal;
		my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(control->m_Skin);
		skin->m_TextColor = D3DCOLOR_ARGB(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticTextAlign:
	{
		my::Control* control = (my::Control *)pProp->GetParent()->GetValue().pulVal;
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_FontAlignDesc));
		my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(control->m_Skin);
		skin->m_TextAlign = (my::Font::Align)g_FontAlignDesc[i].mask;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticTextOutlineColor:
	case PropertyStaticTextOutlineAlpha:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 7)))->GetColor();
		BYTE alpha = pProp->GetParent()->GetSubItem(PropId + 8)->GetValue().lVal;
		my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(control->m_Skin);
		skin->m_TextOutlineColor = D3DCOLOR_ARGB(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyStaticTextOutlineWidth:
	{
		my::Control* control = (my::Control*)pProp->GetParent()->GetValue().pulVal;
		my::StaticSkinPtr skin = boost::dynamic_pointer_cast<my::StaticSkin>(control->m_Skin);
		skin->m_TextOutlineWidth = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyProgressBarProgress:
	{
		my::ProgressBar* progressbar = dynamic_cast<my::ProgressBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		progressbar->m_Progress = pProp->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyProgressBarForegroundImagePath:
	{
		my::ProgressBar* progressbar = dynamic_cast<my::ProgressBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(progressbar);
			return 0;
		}
		my::ProgressBarSkinPtr skin = boost::dynamic_pointer_cast<my::ProgressBarSkin>(progressbar->m_Skin);
		skin->m_ForegroundImage->ReleaseResource();
		skin->m_ForegroundImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_ForegroundImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyProgressBarForegroundImageRect:
	case PropertyProgressBarForegroundImageRectLeft:
	case PropertyProgressBarForegroundImageRectTop:
	case PropertyProgressBarForegroundImageRectWidth:
	case PropertyProgressBarForegroundImageRectHeight:
	case PropertyProgressBarForegroundImageBorder:
	case PropertyProgressBarForegroundImageBorderX:
	case PropertyProgressBarForegroundImageBorderY:
	case PropertyProgressBarForegroundImageBorderZ:
	case PropertyProgressBarForegroundImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyProgressBarForegroundImageRectLeft:
		case PropertyProgressBarForegroundImageRectTop:
		case PropertyProgressBarForegroundImageRectWidth:
		case PropertyProgressBarForegroundImageRectHeight:
		case PropertyProgressBarForegroundImageBorderX:
		case PropertyProgressBarForegroundImageBorderY:
		case PropertyProgressBarForegroundImageBorderZ:
		case PropertyProgressBarForegroundImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyProgressBarForegroundImageRect:
		case PropertyProgressBarForegroundImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ProgressBar* progressbar = dynamic_cast<my::ProgressBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ProgressBarSkinPtr skin = boost::dynamic_pointer_cast<my::ProgressBarSkin>(progressbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_ForegroundImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 2)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 2)->GetSubItem(3)->GetValue().intVal));
		skin->m_ForegroundImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 3)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 3)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 3)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 3)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonPressed:
	{
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		button->m_bPressed = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonMouseOver:
	{
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		button->SetMouseOver(pProp->GetValue().boolVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonPressedOffset:
	case PropertyButtonPressedOffsetX:
	case PropertyButtonPressedOffsetY:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyButtonPressedOffset:
			pControl = pProp->GetParent();
			break;
		case PropertyButtonPressedOffsetX:
		case PropertyButtonPressedOffsetY:
			pControl = pProp->GetParent()->GetParent();
			break;
		}
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pControl->GetValue().pulVal);
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_PressedOffset.x = pControl->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().fltVal;
		skin->m_PressedOffset.y = pControl->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonDisabledImagePath:
	{
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(button);
			return 0;
		}
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		skin->m_DisabledImage->ReleaseResource();
		skin->m_DisabledImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_DisabledImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonDisabledImageRect:
	case PropertyButtonDisabledImageRectLeft:
	case PropertyButtonDisabledImageRectTop:
	case PropertyButtonDisabledImageRectWidth:
	case PropertyButtonDisabledImageRectHeight:
	case PropertyButtonDisabledImageBorder:
	case PropertyButtonDisabledImageBorderX:
	case PropertyButtonDisabledImageBorderY:
	case PropertyButtonDisabledImageBorderZ:
	case PropertyButtonDisabledImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyButtonDisabledImageRectLeft:
		case PropertyButtonDisabledImageRectTop:
		case PropertyButtonDisabledImageRectWidth:
		case PropertyButtonDisabledImageRectHeight:
		case PropertyButtonDisabledImageBorderX:
		case PropertyButtonDisabledImageBorderY:
		case PropertyButtonDisabledImageBorderZ:
		case PropertyButtonDisabledImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyButtonDisabledImageRect:
		case PropertyButtonDisabledImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pControl->GetValue().pulVal);
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_DisabledImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 4)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 4)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 4)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 4)->GetSubItem(3)->GetValue().intVal));
		skin->m_DisabledImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 5)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 5)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 5)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 5)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonPressedImagePath:
	{
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(button);
			return 0;
		}
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		skin->m_PressedImage->ReleaseResource();
		skin->m_PressedImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_PressedImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonPressedImageRect:
	case PropertyButtonPressedImageRectLeft:
	case PropertyButtonPressedImageRectTop:
	case PropertyButtonPressedImageRectWidth:
	case PropertyButtonPressedImageRectHeight:
	case PropertyButtonPressedImageBorder:
	case PropertyButtonPressedImageBorderX:
	case PropertyButtonPressedImageBorderY:
	case PropertyButtonPressedImageBorderZ:
	case PropertyButtonPressedImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyButtonPressedImageRectLeft:
		case PropertyButtonPressedImageRectTop:
		case PropertyButtonPressedImageRectWidth:
		case PropertyButtonPressedImageRectHeight:
		case PropertyButtonPressedImageBorderX:
		case PropertyButtonPressedImageBorderY:
		case PropertyButtonPressedImageBorderZ:
		case PropertyButtonPressedImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyButtonPressedImageRect:
		case PropertyButtonPressedImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pControl->GetValue().pulVal);
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_PressedImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 7)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 7)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 7)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 7)->GetSubItem(3)->GetValue().intVal));
		skin->m_PressedImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 8)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonMouseOverImagePath:
	{
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(button);
			return 0;
		}
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		skin->m_MouseOverImage->ReleaseResource();
		skin->m_MouseOverImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_MouseOverImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyButtonMouseOverImageRect:
	case PropertyButtonMouseOverImageRectLeft:
	case PropertyButtonMouseOverImageRectTop:
	case PropertyButtonMouseOverImageRectWidth:
	case PropertyButtonMouseOverImageRectHeight:
	case PropertyButtonMouseOverImageBorder:
	case PropertyButtonMouseOverImageBorderX:
	case PropertyButtonMouseOverImageBorderY:
	case PropertyButtonMouseOverImageBorderZ:
	case PropertyButtonMouseOverImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyButtonMouseOverImageRectLeft:
		case PropertyButtonMouseOverImageRectTop:
		case PropertyButtonMouseOverImageRectWidth:
		case PropertyButtonMouseOverImageRectHeight:
		case PropertyButtonMouseOverImageBorderX:
		case PropertyButtonMouseOverImageBorderY:
		case PropertyButtonMouseOverImageBorderZ:
		case PropertyButtonMouseOverImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyButtonMouseOverImageRect:
		case PropertyButtonMouseOverImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::Button* button = dynamic_cast<my::Button*>((my::Control*)pControl->GetValue().pulVal);
		my::ButtonSkinPtr skin = boost::dynamic_pointer_cast<my::ButtonSkin>(button->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_MouseOverImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 10)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 10)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 10)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 10)->GetSubItem(3)->GetValue().intVal));
		skin->m_MouseOverImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 11)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 11)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 11)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 11)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxBorder:
	case PropertyEditBoxBorderX:
	case PropertyEditBoxBorderY:
	case PropertyEditBoxBorderZ:
	case PropertyEditBoxBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyEditBoxBorder:
			pControl = pProp->GetParent();
			break;
		case PropertyEditBoxBorderX:
		case PropertyEditBoxBorderY:
		case PropertyEditBoxBorderZ:
		case PropertyEditBoxBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		}
		my::EditBox* editbox = dynamic_cast<my::EditBox*>((my::Control*)pControl->GetValue().pulVal);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		editbox->m_Border.x = pControl->GetSubItem(PropId + 0)->GetSubItem(0)->GetValue().fltVal;
		editbox->m_Border.y = pControl->GetSubItem(PropId + 0)->GetSubItem(1)->GetValue().fltVal;
		editbox->m_Border.z = pControl->GetSubItem(PropId + 0)->GetSubItem(2)->GetValue().fltVal;
		editbox->m_Border.w = pControl->GetSubItem(PropId + 0)->GetSubItem(3)->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxDisabledImagePath:
	{
		my::EditBox* button = dynamic_cast<my::EditBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(button);
			return 0;
		}
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(button->m_Skin);
		skin->m_DisabledImage->ReleaseResource();
		skin->m_DisabledImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_DisabledImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxDisabledImageRect:
	case PropertyEditBoxDisabledImageRectLeft:
	case PropertyEditBoxDisabledImageRectTop:
	case PropertyEditBoxDisabledImageRectWidth:
	case PropertyEditBoxDisabledImageRectHeight:
	case PropertyEditBoxDisabledImageBorder:
	case PropertyEditBoxDisabledImageBorderX:
	case PropertyEditBoxDisabledImageBorderY:
	case PropertyEditBoxDisabledImageBorderZ:
	case PropertyEditBoxDisabledImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyEditBoxDisabledImageRectLeft:
		case PropertyEditBoxDisabledImageRectTop:
		case PropertyEditBoxDisabledImageRectWidth:
		case PropertyEditBoxDisabledImageRectHeight:
		case PropertyEditBoxDisabledImageBorderX:
		case PropertyEditBoxDisabledImageBorderY:
		case PropertyEditBoxDisabledImageBorderZ:
		case PropertyEditBoxDisabledImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyEditBoxDisabledImageRect:
		case PropertyEditBoxDisabledImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::EditBox* button = dynamic_cast<my::EditBox*>((my::Control*)pControl->GetValue().pulVal);
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(button->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_DisabledImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 2)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 2)->GetSubItem(3)->GetValue().intVal));
		skin->m_DisabledImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 3)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 3)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 3)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 3)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxFocusedImagePath:
	{
		my::EditBox* editbox = dynamic_cast<my::EditBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(editbox);
			return 0;
		}
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(editbox->m_Skin);
		skin->m_FocusedImage->ReleaseResource();
		skin->m_FocusedImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_FocusedImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxFocusedImageRect:
	case PropertyEditBoxFocusedImageRectLeft:
	case PropertyEditBoxFocusedImageRectTop:
	case PropertyEditBoxFocusedImageRectWidth:
	case PropertyEditBoxFocusedImageRectHeight:
	case PropertyEditBoxFocusedImageBorder:
	case PropertyEditBoxFocusedImageBorderX:
	case PropertyEditBoxFocusedImageBorderY:
	case PropertyEditBoxFocusedImageBorderZ:
	case PropertyEditBoxFocusedImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyEditBoxFocusedImageRectLeft:
		case PropertyEditBoxFocusedImageRectTop:
		case PropertyEditBoxFocusedImageRectWidth:
		case PropertyEditBoxFocusedImageRectHeight:
		case PropertyEditBoxFocusedImageBorderX:
		case PropertyEditBoxFocusedImageBorderY:
		case PropertyEditBoxFocusedImageBorderZ:
		case PropertyEditBoxFocusedImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyEditBoxFocusedImageRect:
		case PropertyEditBoxFocusedImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::EditBox* editbox = dynamic_cast<my::EditBox*>((my::Control*)pControl->GetValue().pulVal);
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(editbox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_FocusedImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 5)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 5)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 5)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 5)->GetSubItem(3)->GetValue().intVal));
		skin->m_FocusedImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 6)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 6)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 6)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 6)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxSelBkColor:
	case PropertyEditBoxSelBkColorAlpha:
	{
		my::EditBox* editbox = dynamic_cast<my::EditBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(editbox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 7)))->GetColor();
		BYTE alpha = pProp->GetParent()->GetSubItem(PropId + 8)->GetValue().lVal;
		skin->m_SelBkColor = D3DCOLOR_ARGB(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxCaretColor:
	case PropertyEditBoxCaretColorAlpha:
	{
		my::EditBox* editbox = dynamic_cast<my::EditBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(editbox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 9)))->GetColor();
		BYTE alpha = pProp->GetParent()->GetSubItem(PropId + 10)->GetValue().lVal;
		skin->m_CaretColor = D3DCOLOR_ARGB(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxCaretImagePath:
	{
		my::EditBox* button = dynamic_cast<my::EditBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(button);
			return 0;
		}
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(button->m_Skin);
		skin->m_CaretImage->ReleaseResource();
		skin->m_CaretImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_CaretImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyEditBoxCaretImageRect:
	case PropertyEditBoxCaretImageRectLeft:
	case PropertyEditBoxCaretImageRectTop:
	case PropertyEditBoxCaretImageRectWidth:
	case PropertyEditBoxCaretImageRectHeight:
	case PropertyEditBoxCaretImageBorder:
	case PropertyEditBoxCaretImageBorderX:
	case PropertyEditBoxCaretImageBorderY:
	case PropertyEditBoxCaretImageBorderZ:
	case PropertyEditBoxCaretImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyEditBoxCaretImageRectLeft:
		case PropertyEditBoxCaretImageRectTop:
		case PropertyEditBoxCaretImageRectWidth:
		case PropertyEditBoxCaretImageRectHeight:
		case PropertyEditBoxCaretImageBorderX:
		case PropertyEditBoxCaretImageBorderY:
		case PropertyEditBoxCaretImageBorderZ:
		case PropertyEditBoxCaretImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyEditBoxCaretImageRect:
		case PropertyEditBoxCaretImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::EditBox* button = dynamic_cast<my::EditBox*>((my::Control*)pControl->GetValue().pulVal);
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(button->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeStatic);
		skin->m_CaretImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 12)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 12)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 12)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 12)->GetSubItem(3)->GetValue().intVal));
		skin->m_CaretImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 13)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 13)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 13)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 13)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarUpDownButtonHeight:
	case PropertyScrollBarPosition:
	case PropertyScrollBarPageSize:
	case PropertyScrollBarStart:
	case PropertyScrollBarEnd:
	{
		CMFCPropertyGridProperty* pControl = pProp->GetParent();
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		scrollbar->m_UpDownButtonHeight = pControl->GetSubItem(PropId + 0)->GetValue().fltVal;
		scrollbar->m_nPosition = pControl->GetSubItem(PropId + 1)->GetValue().intVal;
		scrollbar->m_nPageSize = pControl->GetSubItem(PropId + 2)->GetValue().intVal;
		scrollbar->m_nStart = pControl->GetSubItem(PropId + 3)->GetValue().intVal;
		scrollbar->m_nEnd = pControl->GetSubItem(PropId + 4)->GetValue().intVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarPressedOffset:
	case PropertyScrollBarPressedOffsetX:
	case PropertyScrollBarPressedOffsetY:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyScrollBarPressedOffset:
			pControl = pProp->GetParent();
			break;
		case PropertyScrollBarPressedOffsetX:
		case PropertyScrollBarPressedOffsetY:
			pControl = pProp->GetParent()->GetParent();
			break;
		}
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		skin->m_PressedOffset.x = pControl->GetSubItem(PropId + 5)->GetSubItem(0)->GetValue().fltVal;
		skin->m_PressedOffset.y = pControl->GetSubItem(PropId + 5)->GetSubItem(1)->GetValue().fltVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarUpBtnNormalImagePath:
	{
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(scrollbar);
			return 0;
		}
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		skin->m_UpBtnNormalImage->ReleaseResource();
		skin->m_UpBtnNormalImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_UpBtnNormalImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarUpBtnNormalImageRect:
	case PropertyScrollBarUpBtnNormalImageRectLeft:
	case PropertyScrollBarUpBtnNormalImageRectTop:
	case PropertyScrollBarUpBtnNormalImageRectWidth:
	case PropertyScrollBarUpBtnNormalImageRectHeight:
	case PropertyScrollBarUpBtnNormalImageBorder:
	case PropertyScrollBarUpBtnNormalImageBorderX:
	case PropertyScrollBarUpBtnNormalImageBorderY:
	case PropertyScrollBarUpBtnNormalImageBorderZ:
	case PropertyScrollBarUpBtnNormalImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyScrollBarUpBtnNormalImageRectLeft:
		case PropertyScrollBarUpBtnNormalImageRectTop:
		case PropertyScrollBarUpBtnNormalImageRectWidth:
		case PropertyScrollBarUpBtnNormalImageRectHeight:
		case PropertyScrollBarUpBtnNormalImageBorderX:
		case PropertyScrollBarUpBtnNormalImageBorderY:
		case PropertyScrollBarUpBtnNormalImageBorderZ:
		case PropertyScrollBarUpBtnNormalImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyScrollBarUpBtnNormalImageRect:
		case PropertyScrollBarUpBtnNormalImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		skin->m_UpBtnNormalImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 7)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 7)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 7)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 7)->GetSubItem(3)->GetValue().intVal));
		skin->m_UpBtnNormalImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 8)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarUpBtnDisabledImagePath:
	{
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(scrollbar);
			return 0;
		}
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		skin->m_UpBtnDisabledImage->ReleaseResource();
		skin->m_UpBtnDisabledImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_UpBtnDisabledImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarUpBtnDisabledImageRect:
	case PropertyScrollBarUpBtnDisabledImageRectLeft:
	case PropertyScrollBarUpBtnDisabledImageRectTop:
	case PropertyScrollBarUpBtnDisabledImageRectWidth:
	case PropertyScrollBarUpBtnDisabledImageRectHeight:
	case PropertyScrollBarUpBtnDisabledImageBorder:
	case PropertyScrollBarUpBtnDisabledImageBorderX:
	case PropertyScrollBarUpBtnDisabledImageBorderY:
	case PropertyScrollBarUpBtnDisabledImageBorderZ:
	case PropertyScrollBarUpBtnDisabledImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyScrollBarUpBtnDisabledImageRectLeft:
		case PropertyScrollBarUpBtnDisabledImageRectTop:
		case PropertyScrollBarUpBtnDisabledImageRectWidth:
		case PropertyScrollBarUpBtnDisabledImageRectHeight:
		case PropertyScrollBarUpBtnDisabledImageBorderX:
		case PropertyScrollBarUpBtnDisabledImageBorderY:
		case PropertyScrollBarUpBtnDisabledImageBorderZ:
		case PropertyScrollBarUpBtnDisabledImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyScrollBarUpBtnDisabledImageRect:
		case PropertyScrollBarUpBtnDisabledImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		skin->m_UpBtnDisabledImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 10)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 10)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 10)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 10)->GetSubItem(3)->GetValue().intVal));
		skin->m_UpBtnDisabledImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 11)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 11)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 11)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 11)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarDownBtnNormalImagePath:
	{
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(scrollbar);
			return 0;
		}
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		skin->m_DownBtnNormalImage->ReleaseResource();
		skin->m_DownBtnNormalImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_DownBtnNormalImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarDownBtnNormalImageRect:
	case PropertyScrollBarDownBtnNormalImageRectLeft:
	case PropertyScrollBarDownBtnNormalImageRectTop:
	case PropertyScrollBarDownBtnNormalImageRectWidth:
	case PropertyScrollBarDownBtnNormalImageRectHeight:
	case PropertyScrollBarDownBtnNormalImageBorder:
	case PropertyScrollBarDownBtnNormalImageBorderX:
	case PropertyScrollBarDownBtnNormalImageBorderY:
	case PropertyScrollBarDownBtnNormalImageBorderZ:
	case PropertyScrollBarDownBtnNormalImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyScrollBarDownBtnNormalImageRectLeft:
		case PropertyScrollBarDownBtnNormalImageRectTop:
		case PropertyScrollBarDownBtnNormalImageRectWidth:
		case PropertyScrollBarDownBtnNormalImageRectHeight:
		case PropertyScrollBarDownBtnNormalImageBorderX:
		case PropertyScrollBarDownBtnNormalImageBorderY:
		case PropertyScrollBarDownBtnNormalImageBorderZ:
		case PropertyScrollBarDownBtnNormalImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyScrollBarDownBtnNormalImageRect:
		case PropertyScrollBarDownBtnNormalImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		skin->m_DownBtnNormalImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 13)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 13)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 13)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 13)->GetSubItem(3)->GetValue().intVal));
		skin->m_DownBtnNormalImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 14)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 14)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 14)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 14)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarDownBtnDisabledImagePath:
	{
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(scrollbar);
			return 0;
		}
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		skin->m_DownBtnDisabledImage->ReleaseResource();
		skin->m_DownBtnDisabledImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_DownBtnDisabledImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarDownBtnDisabledImageRect:
	case PropertyScrollBarDownBtnDisabledImageRectLeft:
	case PropertyScrollBarDownBtnDisabledImageRectTop:
	case PropertyScrollBarDownBtnDisabledImageRectWidth:
	case PropertyScrollBarDownBtnDisabledImageRectHeight:
	case PropertyScrollBarDownBtnDisabledImageBorder:
	case PropertyScrollBarDownBtnDisabledImageBorderX:
	case PropertyScrollBarDownBtnDisabledImageBorderY:
	case PropertyScrollBarDownBtnDisabledImageBorderZ:
	case PropertyScrollBarDownBtnDisabledImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyScrollBarDownBtnDisabledImageRectLeft:
		case PropertyScrollBarDownBtnDisabledImageRectTop:
		case PropertyScrollBarDownBtnDisabledImageRectWidth:
		case PropertyScrollBarDownBtnDisabledImageRectHeight:
		case PropertyScrollBarDownBtnDisabledImageBorderX:
		case PropertyScrollBarDownBtnDisabledImageBorderY:
		case PropertyScrollBarDownBtnDisabledImageBorderZ:
		case PropertyScrollBarDownBtnDisabledImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyScrollBarDownBtnDisabledImageRect:
		case PropertyScrollBarDownBtnDisabledImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		skin->m_DownBtnDisabledImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 16)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 16)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 16)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 16)->GetSubItem(3)->GetValue().intVal));
		skin->m_DownBtnDisabledImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 17)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 17)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 17)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 17)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarThumbBtnNormalImagePath:
	{
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(scrollbar);
			return 0;
		}
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		skin->m_ThumbBtnNormalImage->ReleaseResource();
		skin->m_ThumbBtnNormalImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_ThumbBtnNormalImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyScrollBarThumbBtnNormalImageRect:
	case PropertyScrollBarThumbBtnNormalImageRectLeft:
	case PropertyScrollBarThumbBtnNormalImageRectTop:
	case PropertyScrollBarThumbBtnNormalImageRectWidth:
	case PropertyScrollBarThumbBtnNormalImageRectHeight:
	case PropertyScrollBarThumbBtnNormalImageBorder:
	case PropertyScrollBarThumbBtnNormalImageBorderX:
	case PropertyScrollBarThumbBtnNormalImageBorderY:
	case PropertyScrollBarThumbBtnNormalImageBorderZ:
	case PropertyScrollBarThumbBtnNormalImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyScrollBarThumbBtnNormalImageRectLeft:
		case PropertyScrollBarThumbBtnNormalImageRectTop:
		case PropertyScrollBarThumbBtnNormalImageRectWidth:
		case PropertyScrollBarThumbBtnNormalImageRectHeight:
		case PropertyScrollBarThumbBtnNormalImageBorderX:
		case PropertyScrollBarThumbBtnNormalImageBorderY:
		case PropertyScrollBarThumbBtnNormalImageBorderZ:
		case PropertyScrollBarThumbBtnNormalImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyScrollBarThumbBtnNormalImageRect:
		case PropertyScrollBarThumbBtnNormalImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ScrollBar* scrollbar = dynamic_cast<my::ScrollBar*>((my::Control*)pControl->GetValue().pulVal);
		my::ScrollBarSkinPtr skin = boost::dynamic_pointer_cast<my::ScrollBarSkin>(scrollbar->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		skin->m_ThumbBtnNormalImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 19)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 19)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 19)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 19)->GetSubItem(3)->GetValue().intVal));
		skin->m_ThumbBtnNormalImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 20)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 20)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 20)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 20)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyCheckBoxChecked:
	{
		my::CheckBox* checkbox = dynamic_cast<my::CheckBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		checkbox->m_Checked = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownSize:
	case PropertyComboBoxDropdownSizeX:
	case PropertyComboBoxDropdownSizeY:
	case PropertyComboBoxScrollbarWidth:
	case PropertyComboBoxScrollbarUpDownBtnHeight:
	case PropertyComboBoxBorder:
	case PropertyComboBoxBorderX:
	case PropertyComboBoxBorderY:
	case PropertyComboBoxBorderZ:
	case PropertyComboBoxBorderW:
	case PropertyComboBoxItemHeight:
	case PropertyComboBoxItemCount:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyComboBoxDropdownSizeX:
		case PropertyComboBoxDropdownSizeY:
		case PropertyComboBoxBorderX:
		case PropertyComboBoxBorderY:
		case PropertyComboBoxBorderZ:
		case PropertyComboBoxBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyComboBoxDropdownSize:
		case PropertyComboBoxScrollbarWidth:
		case PropertyComboBoxScrollbarUpDownBtnHeight:
		case PropertyComboBoxBorder:
		case PropertyComboBoxItemHeight:
		case PropertyComboBoxItemCount:
			pControl = pProp->GetParent();
			break;
		}
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pControl->GetValue().pulVal);
		my::EditBoxSkinPtr skin = boost::dynamic_pointer_cast<my::EditBoxSkin>(combobox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeButton);
		combobox->m_DropdownSize.x = pControl->GetSubItem(PropId + 0)->GetSubItem(0)->GetValue().fltVal;
		combobox->m_DropdownSize.y = pControl->GetSubItem(PropId + 0)->GetSubItem(1)->GetValue().fltVal;
		combobox->m_ScrollbarWidth = pControl->GetSubItem(PropId + 1)->GetValue().fltVal;
		combobox->m_ScrollbarUpDownBtnHeight = pControl->GetSubItem(PropId + 2)->GetValue().fltVal;
		combobox->m_Border.x = pControl->GetSubItem(PropId + 3)->GetSubItem(0)->GetValue().fltVal;
		combobox->m_Border.y = pControl->GetSubItem(PropId + 3)->GetSubItem(1)->GetValue().fltVal;
		combobox->m_Border.z = pControl->GetSubItem(PropId + 3)->GetSubItem(2)->GetValue().fltVal;
		combobox->m_Border.w = pControl->GetSubItem(PropId + 3)->GetSubItem(3)->GetValue().fltVal;
		combobox->m_ItemHeight = pControl->GetSubItem(PropId + 4)->GetValue().fltVal;
		int ItemCount = pControl->GetSubItem(PropId + 5)->GetValue().lVal;
		for (int i = combobox->m_Items.size(); i < ItemCount; i++)
		{
			combobox->AddItem(str_printf(L"item%d", i));
		}
		combobox->m_Items.resize(ItemCount, my::ComboBoxItem(L""));
		combobox->OnLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownImagePath:
	{
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(combobox);
			return 0;
		}
		my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
		skin->m_DropdownImage->ReleaseResource();
		skin->m_DropdownImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_DropdownImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownImageRect:
	case PropertyComboBoxDropdownImageRectLeft:
	case PropertyComboBoxDropdownImageRectTop:
	case PropertyComboBoxDropdownImageRectWidth:
	case PropertyComboBoxDropdownImageRectHeight:
	case PropertyComboBoxDropdownImageBorder:
	case PropertyComboBoxDropdownImageBorderX:
	case PropertyComboBoxDropdownImageBorderY:
	case PropertyComboBoxDropdownImageBorderZ:
	case PropertyComboBoxDropdownImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyComboBoxDropdownImageRectLeft:
		case PropertyComboBoxDropdownImageRectTop:
		case PropertyComboBoxDropdownImageRectWidth:
		case PropertyComboBoxDropdownImageRectHeight:
		case PropertyComboBoxDropdownImageBorderX:
		case PropertyComboBoxDropdownImageBorderY:
		case PropertyComboBoxDropdownImageBorderZ:
		case PropertyComboBoxDropdownImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyComboBoxDropdownImageRect:
		case PropertyComboBoxDropdownImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pControl->GetValue().pulVal);
		my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeButton);
		skin->m_DropdownImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 7)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 7)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 7)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 7)->GetSubItem(3)->GetValue().intVal));
		skin->m_DropdownImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 8)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 8)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownItemTextColor:
	case PropertyComboBoxDropdownItemTextColorAlpha:
	{
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeButton);
		COLORREF color = (DYNAMIC_DOWNCAST(CColorProp, pProp->GetParent()->GetSubItem(PropId + 9)))->GetColor();
		BYTE alpha = pProp->GetParent()->GetSubItem(PropId + 10)->GetValue().lVal;
		skin->m_DropdownItemTextColor = D3DCOLOR_ARGB(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownItemTextAlign:
	{
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
		ASSERT(i >= 0 && i < _countof(g_FontAlignDesc));
		skin->m_DropdownItemTextAlign = (my::Font::Align)g_FontAlignDesc[i].mask;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownItemMouseOverImagePath:
	{
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		std::string path = theApp.GetRelativePath(pProp->GetValue().bstrVal);
		if (path.empty() && _tcslen(pProp->GetValue().bstrVal) > 0)
		{
			MessageBox(str_printf(_T("cannot relative path: %s"), pProp->GetValue().bstrVal).c_str());
			UpdatePropertiesControl(combobox);
			return 0;
		}
		my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
		skin->m_DropdownItemMouseOverImage->ReleaseResource();
		skin->m_DropdownItemMouseOverImage->m_TexturePath = path;
		if (skin->IsRequested())
		{
			skin->m_DropdownItemMouseOverImage->RequestResource();
		}
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyComboBoxDropdownItemMouseOverImageRect:
	case PropertyComboBoxDropdownItemMouseOverImageRectLeft:
	case PropertyComboBoxDropdownItemMouseOverImageRectTop:
	case PropertyComboBoxDropdownItemMouseOverImageRectWidth:
	case PropertyComboBoxDropdownItemMouseOverImageRectHeight:
	case PropertyComboBoxDropdownItemMouseOverImageBorder:
	case PropertyComboBoxDropdownItemMouseOverImageBorderX:
	case PropertyComboBoxDropdownItemMouseOverImageBorderY:
	case PropertyComboBoxDropdownItemMouseOverImageBorderZ:
	case PropertyComboBoxDropdownItemMouseOverImageBorderW:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyComboBoxDropdownItemMouseOverImageRectLeft:
		case PropertyComboBoxDropdownItemMouseOverImageRectTop:
		case PropertyComboBoxDropdownItemMouseOverImageRectWidth:
		case PropertyComboBoxDropdownItemMouseOverImageRectHeight:
		case PropertyComboBoxDropdownItemMouseOverImageBorderX:
		case PropertyComboBoxDropdownItemMouseOverImageBorderY:
		case PropertyComboBoxDropdownItemMouseOverImageBorderZ:
		case PropertyComboBoxDropdownItemMouseOverImageBorderW:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyComboBoxDropdownItemMouseOverImageRect:
		case PropertyComboBoxDropdownItemMouseOverImageBorder:
			pControl = pProp->GetParent();
			break;
		}
		my::ComboBox* combobox = dynamic_cast<my::ComboBox*>((my::Control*)pControl->GetValue().pulVal);
		my::ComboBoxSkinPtr skin = boost::dynamic_pointer_cast<my::ComboBoxSkin>(combobox->m_Skin);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeButton);
		skin->m_DropdownItemMouseOverImage->m_Rect = CRect(
			CPoint(
				pControl->GetSubItem(PropId + 13)->GetSubItem(0)->GetValue().intVal,
				pControl->GetSubItem(PropId + 13)->GetSubItem(1)->GetValue().intVal),
			CSize(
				pControl->GetSubItem(PropId + 13)->GetSubItem(2)->GetValue().intVal,
				pControl->GetSubItem(PropId + 13)->GetSubItem(3)->GetValue().intVal));
		skin->m_DropdownItemMouseOverImage->m_Border.SetRect(
			pControl->GetSubItem(PropId + 14)->GetSubItem(0)->GetValue().intVal,
			pControl->GetSubItem(PropId + 14)->GetSubItem(1)->GetValue().intVal,
			pControl->GetSubItem(PropId + 14)->GetSubItem(2)->GetValue().intVal,
			pControl->GetSubItem(PropId + 14)->GetSubItem(3)->GetValue().intVal);
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyListBoxScrollbarWidth:
	case PropertyListBoxScrollbarUpDownBtnHeight:
	case PropertyListBoxItemSize:
	case PropertyListBoxItemSizeX:
	case PropertyListBoxItemSizeY:
	case PropertyListBoxItemCount:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyListBoxItemSizeX:
		case PropertyListBoxItemSizeY:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyListBoxScrollbarWidth:
		case PropertyListBoxScrollbarUpDownBtnHeight:
		case PropertyListBoxItemSize:
		case PropertyListBoxItemCount:
			pControl = pProp->GetParent();
			break;
		}
		my::ListBox* listbox = dynamic_cast<my::ListBox*>((my::Control*)pControl->GetValue().pulVal);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		listbox->m_ScrollbarWidth = pControl->GetSubItem(PropId + 0)->GetValue().fltVal;
		listbox->m_ScrollbarUpDownBtnHeight = pControl->GetSubItem(PropId + 1)->GetValue().fltVal;
		listbox->m_ItemSize.x = pControl->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().fltVal;
		listbox->m_ItemSize.y = pControl->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().fltVal;
		int ItemCount = pControl->GetSubItem(PropId + 3)->GetValue().lVal;
		int i = listbox->m_Childs.size();
		for (; i < ItemCount; i++)
		{
			my::StaticPtr item;
			if (listbox->m_Childs.empty())
			{
				my::ButtonSkinPtr skin(new my::ButtonSkin());
				skin->m_Image.reset(new my::ControlImage());
				skin->m_Image->m_TexturePath = theApp.default_button_img;
				skin->m_Image->m_Rect = theApp.default_button_img_rect;
				skin->m_Image->m_Border = theApp.default_button_img_border;
				skin->m_FontPath = theApp.default_font_path;
				skin->m_FontHeight = theApp.default_font_height;
				skin->m_FontFaceIndex = theApp.default_font_face_index;
				skin->m_TextColor = theApp.default_button_text_color;
				skin->m_TextAlign = theApp.default_button_text_align;
				skin->m_PressedOffset = theApp.default_button_pressed_offset;
				skin->m_DisabledImage.reset(new my::ControlImage());
				skin->m_DisabledImage->m_TexturePath = theApp.default_button_disabledimg;
				skin->m_DisabledImage->m_Rect = theApp.default_button_disabledimg_rect;
				skin->m_DisabledImage->m_Border = theApp.default_button_disabledimg_border;
				skin->m_PressedImage.reset(new my::ControlImage());
				skin->m_PressedImage->m_TexturePath = theApp.default_button_pressedimg;
				skin->m_PressedImage->m_Rect = theApp.default_button_pressedimg_rect;
				skin->m_PressedImage->m_Border = theApp.default_button_pressedimg_border;
				skin->m_MouseOverImage.reset(new my::ControlImage());
				skin->m_MouseOverImage->m_TexturePath = theApp.default_button_mouseoverimg;
				skin->m_MouseOverImage->m_Rect = theApp.default_button_mouseoverimg_rect;
				skin->m_MouseOverImage->m_Border = theApp.default_button_mouseoverimg_border;

				item.reset(new my::Button(my::NamedObject::MakeUniqueName(str_printf("%s_item%d", listbox->GetName(), i).c_str()).c_str()));
				item->m_Skin = skin;
			}
			else
			{
				item = boost::dynamic_pointer_cast<my::Static>(listbox->m_Childs.back()->Clone());
			}
			item->m_Text = str_printf(L"item%d", i);

			listbox->InsertControl(listbox->GetChildNum(), item);
		}
		for (; i > ItemCount; i--)
		{
			listbox->RemoveControl(i - 1);
		}
		listbox->OnLayout();
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyDialogEnableDrag:
	{
		my::Dialog* dialog = dynamic_cast<my::Dialog*>((my::Control*)pProp->GetParent()->GetValue().pulVal);
		dialog->m_EnableDrag = pProp->GetValue().boolVal;
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	case PropertyDialogPos:
	case PropertyDialogPosX:
	case PropertyDialogPosY:
	case PropertyDialogPosZ:
	case PropertyDialogRot:
	case PropertyDialogRotX:
	case PropertyDialogRotY:
	case PropertyDialogRotZ:
	case PropertyDialogScale:
	case PropertyDialogScaleX:
	case PropertyDialogScaleY:
	case PropertyDialogScaleZ:
	{
		CMFCPropertyGridProperty* pControl = NULL;
		switch (PropertyId)
		{
		case PropertyDialogPosX:
		case PropertyDialogPosY:
		case PropertyDialogPosZ:
		case PropertyDialogRotX:
		case PropertyDialogRotY:
		case PropertyDialogRotZ:
		case PropertyDialogScaleX:
		case PropertyDialogScaleY:
		case PropertyDialogScaleZ:
			pControl = pProp->GetParent()->GetParent();
			break;
		case PropertyDialogPos:
		case PropertyDialogRot:
		case PropertyDialogScale:
			pControl = pProp->GetParent();
			break;
		}
		my::Dialog* dialog = dynamic_cast<my::Dialog*>((my::Control*)pControl->GetValue().pulVal);
		unsigned int PropId = GetControlPropCount(my::Control::ControlTypeControl);
		dialog->m_World = my::Matrix4::Compose(
			my::Vector3(
				pControl->GetSubItem(PropId + 3)->GetSubItem(0)->GetValue().fltVal,
				pControl->GetSubItem(PropId + 3)->GetSubItem(1)->GetValue().fltVal,
				pControl->GetSubItem(PropId + 3)->GetSubItem(2)->GetValue().fltVal),
			my::Quaternion::RotationEulerAngles(
				D3DXToRadian(pControl->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().fltVal),
				D3DXToRadian(pControl->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().fltVal),
				D3DXToRadian(pControl->GetSubItem(PropId + 2)->GetSubItem(2)->GetValue().fltVal)),
			my::Vector3(
				pControl->GetSubItem(PropId + 1)->GetSubItem(0)->GetValue().fltVal,
				pControl->GetSubItem(PropId + 1)->GetSubItem(1)->GetValue().fltVal,
				pControl->GetSubItem(PropId + 1)->GetSubItem(2)->GetValue().fltVal));
		my::EventArg arg;
		pFrame->m_EventAttributeChanged(&arg);
		break;
	}
	}
	return 0;
}

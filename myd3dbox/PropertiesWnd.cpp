
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

	pComponent->GetSubItem(0)->GetSubItem(0)->SetValue((_variant_t)cmp->m_Position.x);
	pComponent->GetSubItem(0)->GetSubItem(1)->SetValue((_variant_t)cmp->m_Position.y);
	pComponent->GetSubItem(0)->GetSubItem(2)->SetValue((_variant_t)cmp->m_Position.z);

	my::Vector3 angle = cmp->m_Rotation.ToEulerAngles();
	pComponent->GetSubItem(1)->GetSubItem(0)->SetValue((_variant_t)angle.x);
	pComponent->GetSubItem(1)->GetSubItem(1)->SetValue((_variant_t)angle.y);
	pComponent->GetSubItem(1)->GetSubItem(2)->SetValue((_variant_t)angle.z);

	pComponent->GetSubItem(2)->GetSubItem(0)->SetValue((_variant_t)cmp->m_Scale.x);
	pComponent->GetSubItem(2)->GetSubItem(1)->SetValue((_variant_t)cmp->m_Scale.y);
	pComponent->GetSubItem(2)->GetSubItem(2)->SetValue((_variant_t)cmp->m_Scale.z);

	switch (cmp->m_Type)
	{
	case Component::ComponentTypeActor:
		UpdatePropertiesActor(pComponent, dynamic_cast<Actor *>(cmp));
		break;
	case Component::ComponentTypeMesh:
		UpdatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeEmitter:
		UpdatePropertiesEmitter(pComponent, dynamic_cast<EmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		UpdatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		UpdatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
		break;
	//case Component::ComponentTypeRigid:
	//	m_pProp[PropertyMesh]->Show(FALSE, FALSE);
	//	m_pProp[PropertyEmitter]->Show(FALSE, FALSE);
	//	m_pProp[PropertyMaterialList]->Show(FALSE, FALSE);
	//	m_pProp[PropertyRigidShapeList]->Show(TRUE, FALSE);
	//	m_pProp[PropertyTerrain]->Show(FALSE, FALSE);
	//	UpdatePropertiesRigid(dynamic_cast<RigidComponent *>(cmp));
	//	break;
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

void CPropertiesWnd::UpdatePropertiesActor(CMFCPropertyGridProperty * pComponent, Actor * actor)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyActorAABB)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesActor(pComponent, actor);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->GetSubItem(0)->SetValue((_variant_t)actor->m_aabb.m_min.x);
	pComponent->GetSubItem(PropId + 0)->GetSubItem(1)->SetValue((_variant_t)actor->m_aabb.m_min.y);
	pComponent->GetSubItem(PropId + 0)->GetSubItem(2)->SetValue((_variant_t)actor->m_aabb.m_min.z);
	pComponent->GetSubItem(PropId + 0)->GetSubItem(3)->SetValue((_variant_t)actor->m_aabb.m_max.x);
	pComponent->GetSubItem(PropId + 0)->GetSubItem(4)->SetValue((_variant_t)actor->m_aabb.m_max.y);
	pComponent->GetSubItem(PropId + 0)->GetSubItem(5)->SetValue((_variant_t)actor->m_aabb.m_max.z);
}

void CPropertiesWnd::UpdatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyMeshResPath)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesMesh(pComponent, mesh_cmp);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)ms2ts(mesh_cmp->m_MeshRes.m_Path).c_str());
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)mesh_cmp->m_bAnimation);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)mesh_cmp->m_bInstance);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)(VARIANT_BOOL)mesh_cmp->m_StaticCollision);
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

void CPropertiesWnd::UpdatePropertiesEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyEmitterParticleList)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesEmitter(pComponent, emit_cmp);
		return;
	}
	for (unsigned int i = 0; i < emit_cmp->m_Emitter->m_ParticleList.size(); i++)
	{
		if ((unsigned int)pProp->GetSubItemsCount() <= i + 1)
		{
			CreatePropertiesEmitterParticle(pProp, i, emit_cmp);
			continue;
		}
		UpdatePropertiesEmitterParticle(pProp, i, emit_cmp);
	}
	RemovePropertiesFrom(pProp, emit_cmp->m_Emitter->m_ParticleList.size() + 1);
	UpdatePropertiesMaterial(pComponent, PropId + 1, emit_cmp->m_Material.get());
}

void CPropertiesWnd::UpdatePropertiesEmitterParticle(CMFCPropertyGridProperty * pParentProp, DWORD NodeId, EmitterComponent * emit_cmp)
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
	pProp = pParticle->GetSubItem(2)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorR); pProp->SetValue((_variant_t)particle.m_Color.x);
	pProp = pParticle->GetSubItem(2)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorG); pProp->SetValue((_variant_t)particle.m_Color.y);
	pProp = pParticle->GetSubItem(2)->GetSubItem(2); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorB); pProp->SetValue((_variant_t)particle.m_Color.z);
	pProp = pParticle->GetSubItem(2)->GetSubItem(3); _ASSERT(pProp->GetData() == PropertyEmitterParticleColorA); pProp->SetValue((_variant_t)particle.m_Color.w);
	pProp = pParticle->GetSubItem(3)->GetSubItem(0); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeX); pProp->SetValue((_variant_t)particle.m_Size.x);
	pProp = pParticle->GetSubItem(3)->GetSubItem(1); _ASSERT(pProp->GetData() == PropertyEmitterParticleSizeY); pProp->SetValue((_variant_t)particle.m_Size.y);
	pProp = pParticle->GetSubItem(4); _ASSERT(pProp->GetData() == PropertyEmitterParticleAngle); pProp->SetValue((_variant_t)particle.m_Angle);
}

void CPropertiesWnd::UpdatePropertiesSphericalEmitter(CMFCPropertyGridProperty * pComponent, SphericalEmitterComponent * sphe_emit_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
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
	_ASSERT(pSpline->GetValue().ulVal == (DWORD_PTR)spline);
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

void CPropertiesWnd::UpdatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId, const my::SplineNode * node)
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
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	CMFCPropertyGridProperty * pProp = pComponent->GetSubItem(PropId);
	if (!pProp || pProp->GetData() != PropertyTerrainRowChunks)
	{
		RemovePropertiesFrom(pComponent, PropId);
		CreatePropertiesTerrain(pComponent, terrain);
		return;
	}
	pComponent->GetSubItem(PropId + 0)->SetValue((_variant_t)terrain->m_RowChunks);
	pComponent->GetSubItem(PropId + 1)->SetValue((_variant_t)terrain->m_ChunkRows);
	pComponent->GetSubItem(PropId + 2)->SetValue((_variant_t)terrain->m_HeightScale);
	pComponent->GetSubItem(PropId + 3)->SetValue((_variant_t)terrain->m_WrappedU);
	pComponent->GetSubItem(PropId + 4)->SetValue((_variant_t)terrain->m_WrappedV);
	pComponent->GetSubItem(PropId + 5);
	pComponent->GetSubItem(PropId + 6)->SetValue((_variant_t)(VARIANT_BOOL)terrain->m_StaticCollision);
	UpdatePropertiesMaterial(pComponent, PropId + 7, terrain->m_Material.get());
}
//
//void CPropertiesWnd::UpdatePropertiesRigid(RigidComponent * cmp)
//{
//	unsigned int NbShapes = cmp->m_RigidActor->getNbShapes();
//	std::vector<PxShape *> shapes(NbShapes);
//	NbShapes = cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
//	unsigned int i = 0;
//	for (; i < NbShapes; i++)
//	{
//		if ((unsigned int)m_pProp[PropertyRigidShapeList]->GetSubItemsCount() > i + 1
//			&& HIWORD(m_pProp[PropertyRigidShapeList]->GetSubItem(i + 1)->GetData()) != shapes[i]->getGeometryType())
//		{
//			RemovePropertiesFrom(m_pProp[PropertyRigidShapeList], i + 1);
//		}
//		if ((unsigned int)m_pProp[PropertyRigidShapeList]->GetSubItemsCount() <= i + 1)
//		{
//			CreatePropertiesShape(m_pProp[PropertyRigidShapeList], i, shapes[i]->getGeometryType());
//		}
//		UpdatePropertiesShape(m_pProp[PropertyRigidShapeList], i, shapes[i]);
//	}
//	RemovePropertiesFrom(m_pProp[PropertyRigidShapeList], i + 1);
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
			pComponent = new CSimpleProp(GetComponentTypeName(cmp->m_Type), PropertyComponent, FALSE);
			pParentCtrl->AddSubItem(pComponent);
		}
	}
	else
	{
		while (i >= m_wndPropList.GetPropertyCount())
		{
			pComponent = new CSimpleProp(GetComponentTypeName(cmp->m_Type), PropertyComponent, FALSE);
			m_wndPropList.AddProperty(pComponent);
		}
	}
	ASSERT(pComponent);
	pComponent->SetValue((_variant_t)(DWORD_PTR)cmp); // ! only worked on 32bit system

	CMFCPropertyGridProperty * pPosition = new CMFCPropertyGridProperty(_T("Position"), PropertyComponentPos, TRUE);
	pComponent->AddSubItem(pPosition);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)cmp->m_Position.x, NULL, PropertyComponentPosX);
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
	case Component::ComponentTypeActor:
		CreatePropertiesActor(pComponent, dynamic_cast<Actor *>(cmp));
		break;
	case Component::ComponentTypeMesh:
		CreatePropertiesMesh(pComponent, dynamic_cast<MeshComponent *>(cmp));
		break;
	case Component::ComponentTypeEmitter:
		CreatePropertiesEmitter(pComponent, dynamic_cast<EmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeSphericalEmitter:
		CreatePropertiesSphericalEmitter(pComponent, dynamic_cast<SphericalEmitterComponent *>(cmp));
		break;
	case Component::ComponentTypeTerrain:
		CreatePropertiesTerrain(pComponent, dynamic_cast<Terrain *>(cmp));
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

void CPropertiesWnd::CreatePropertiesActor(CMFCPropertyGridProperty * pComponent, Actor * actor)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
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
}

void CPropertiesWnd::CreatePropertiesMesh(CMFCPropertyGridProperty * pComponent, MeshComponent * mesh_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CFileProp(_T("ResPath"), TRUE, (_variant_t)ms2ts(mesh_cmp->m_MeshRes.m_Path).c_str(), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, PropertyMeshResPath);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Animation"), (_variant_t)mesh_cmp->m_bAnimation, NULL, PropertyMeshAnimation);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("Instance"), (_variant_t)mesh_cmp->m_bInstance, NULL, PropertyMeshInstance);
	pComponent->AddSubItem(pProp);
	pProp = new CCheckBoxProp(_T("StaticCollision"), (_variant_t)mesh_cmp->m_StaticCollision, NULL, PropertyMeshStaticCollision);
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

void CPropertiesWnd::CreatePropertiesEmitter(CMFCPropertyGridProperty * pComponent, EmitterComponent * emit_cmp)
{
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pParticleList = new CSimpleProp(_T("ParticleList"), PropertyEmitterParticleList, FALSE);
	pComponent->AddSubItem(pParticleList);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("ParticleCount"), (_variant_t)(size_t)emit_cmp->m_Emitter->m_ParticleList.size(), NULL, PropertyEmitterParticleCount);
	pParticleList->AddSubItem(pProp);
	for (unsigned int i = 0; i < emit_cmp->m_Emitter->m_ParticleList.size(); i++)
	{
		CreatePropertiesEmitterParticle(pParticleList, i, emit_cmp);
	}
	CreatePropertiesMaterial(pComponent, 0, emit_cmp->m_Material.get());
}

void CPropertiesWnd::CreatePropertiesEmitterParticle(CMFCPropertyGridProperty * pParentProp, DWORD NodeId, EmitterComponent * emit_cmp)
{
	TCHAR buff[128];
	_stprintf_s(buff, _countof(buff), _T("Particle%u"), NodeId);
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

	CMFCPropertyGridProperty * pColor = new CMFCPropertyGridProperty(_T("Color"), PropertyEmitterParticleColor, TRUE);
	pParticle->AddSubItem(pColor);
	pProp = new CSimpleProp(_T("r"), (_variant_t)particle.m_Color.x, NULL, PropertyEmitterParticleColorR);
	pColor->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("g"), (_variant_t)particle.m_Color.y, NULL, PropertyEmitterParticleColorG);
	pColor->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("b"), (_variant_t)particle.m_Color.z, NULL, PropertyEmitterParticleColorB);
	pColor->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("a"), (_variant_t)particle.m_Color.w, NULL, PropertyEmitterParticleColorA);
	pColor->AddSubItem(pProp);

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
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
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
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Count"), (_variant_t)(size_t)spline->size(), NULL, PropertySplineNodeCount);
	pSpline->AddSubItem(pProp);
	for (unsigned int i = 0; i < spline->size(); i++)
	{
		CreatePropertiesSplineNode(pSpline, i, &(*spline)[i]);
	}
}

void CPropertiesWnd::CreatePropertiesSplineNode(CMFCPropertyGridProperty * pSpline, DWORD NodeId, my::SplineNode * node)
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
	unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
	RemovePropertiesFrom(pComponent, PropId);
	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("RowChunks"), (_variant_t)terrain->m_RowChunks, NULL, PropertyTerrainRowChunks);
	pProp->Enable(FALSE);
	pComponent->AddSubItem(pProp);
	pProp = new CSimpleProp(_T("ChunkRows"), (_variant_t)terrain->m_ChunkRows, NULL, PropertyTerrainChunkRows);
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
	pProp = new CCheckBoxProp(_T("StaticCollision"), terrain->m_StaticCollision, NULL, PropertyTerrainStaticCollision);
	pComponent->AddSubItem(pProp);
	CreatePropertiesMaterial(pComponent, 0, terrain->m_Material.get());
}
//
//void CPropertiesWnd::CreatePropertiesShape(CMFCPropertyGridProperty * pParentCtrl, DWORD NodeId, PxGeometryType::Enum type)
//{
//	TCHAR buff[128];
//	_stprintf_s(buff, _countof(buff), _T("%u.%s"), NodeId, g_ShapeTypeDesc[type].desc);
//	CMFCPropertyGridProperty * pShape = new CMFCPropertyGridProperty(buff, MAKELONG(NodeId, type), FALSE);
//	pParentCtrl->AddSubItem(pShape);
//	CMFCPropertyGridProperty * pPos = new CSimpleProp(_T("Pos"), PropertyRigidShapePos, TRUE);
//	pShape->AddSubItem(pPos);
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyRigidShapePosX);
//	pPos->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyRigidShapePosY);
//	pPos->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyRigidShapePosZ);
//	pPos->AddSubItem(pProp);
//	CMFCPropertyGridProperty * pRot = new CSimpleProp(_T("Rot"), PropertyRigidShapeRot, TRUE);
//	pShape->AddSubItem(pRot);
//	pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyRigidShapeRotX);
//	pRot->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyRigidShapeRotY);
//	pRot->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyRigidShapeRotZ);
//	pRot->AddSubItem(pProp);
//	switch (type)
//	{
//	case PxGeometryType::eSPHERE:
//		CreatePropertiesShapeSphere(pShape);
//		break;
//	case PxGeometryType::ePLANE:
//		break;
//	case PxGeometryType::eCAPSULE:
//		CreatePropertiesShapeCapsule(pShape);
//		break;
//	case PxGeometryType::eBOX:
//		CreatePropertiesShapeBox(pShape);
//		break;
//	}
//}
//
//void CPropertiesWnd::CreatePropertiesShapeBox(CMFCPropertyGridProperty * pShape)
//{
//	CMFCPropertyGridProperty * pHalfExtents = new CSimpleProp(_T("HalfExtents"), PropertyRigidShapeBoxHalfExtents, TRUE);
//	pShape->AddSubItem(pHalfExtents);
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("x"), (_variant_t)0.0f, NULL, PropertyRigidShapeBoxHalfExtentsX);
//	pHalfExtents->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("y"), (_variant_t)0.0f, NULL, PropertyRigidShapeBoxHalfExtentsY);
//	pHalfExtents->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("z"), (_variant_t)0.0f, NULL, PropertyRigidShapeBoxHalfExtentsZ);
//	pHalfExtents->AddSubItem(pProp);
//}
//
//void CPropertiesWnd::CreatePropertiesShapeSphere(CMFCPropertyGridProperty * pShape)
//{
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Radius"), (_variant_t)0.0f, NULL, PropertyRigidShapeSphereRadius);
//	pShape->AddSubItem(pProp);
//}
//
//void CPropertiesWnd::CreatePropertiesShapeCapsule(CMFCPropertyGridProperty * pShape)
//{
//	CMFCPropertyGridProperty * pProp = new CSimpleProp(_T("Radius"), (_variant_t)0.0f, NULL, PropertyRigidShapeCapsuleRadius);
//	pShape->AddSubItem(pProp);
//	pProp = new CSimpleProp(_T("HalfHeight"), (_variant_t)0.0f, NULL, PropertyRigidShapeCapsuleHalfHeight);
//	pShape->AddSubItem(pProp);
//}

unsigned int CPropertiesWnd::GetComponentAttrCount(Component::ComponentType type)
{
	switch (type)
	{
	case Component::ComponentTypeComponent:
		return 3;
	case Component::ComponentTypeActor:
		return GetComponentAttrCount(Component::ComponentTypeComponent) + 1;
	case Component::ComponentTypeMesh:
		return GetComponentAttrCount(Component::ComponentTypeComponent) + 5;
	case Component::ComponentTypeEmitter:
		return GetComponentAttrCount(Component::ComponentTypeComponent) + 2;
	case Component::ComponentTypeSphericalEmitter:
		return GetComponentAttrCount(Component::ComponentTypeComponent) + 14;
	case Component::ComponentTypeTerrain:
		return GetComponentAttrCount(Component::ComponentTypeComponent) + 8;
	}
	return 0;
}

LPCTSTR CPropertiesWnd::GetComponentTypeName(Component::ComponentType type)
{
	switch (type)
	{
	case Component::ComponentTypeComponent:
		return _T("Component");
	case Component::ComponentTypeActor:
		return _T("Actor");
	case Component::ComponentTypeMesh:
		return _T("Mesh");
	case Component::ComponentTypeEmitter:
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
			CMFCPropertyGridProperty * pPosition = NULL, * pRotation = NULL, * pScale = NULL;
			switch (PropertyId)
			{
			case PropertyComponentPos:
			case PropertyComponentRot:
			case PropertyComponentScale:
				cmp = (Component *)pProp->GetParent()->GetValue().ulVal;
				pPosition = pProp->GetParent()->GetSubItem(0);
				pRotation = pProp->GetParent()->GetSubItem(1);
				pScale = pProp->GetParent()->GetSubItem(2);
				break;
			default:
				cmp = (Component *)pProp->GetParent()->GetParent()->GetValue().ulVal;
				pPosition = pProp->GetParent()->GetParent()->GetSubItem(0);
				pRotation = pProp->GetParent()->GetParent()->GetSubItem(1);
				pScale = pProp->GetParent()->GetParent()->GetSubItem(2);
				break;
			}
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
	case PropertyEmitterParticleCount:
		{
			EmitterComponent * emit_cmp = (EmitterComponent *)pProp->GetParent()->GetParent()->GetValue().ulVal;
			emit_cmp->m_Emitter->m_ParticleList.resize(pProp->GetValue().uintVal,
				my::Emitter::Particle(my::Vector3(0,0,0), my::Vector3(0,0,0), my::Vector4(1,1,1,1), my::Vector2(10,10), 0, 0));
			UpdatePropertiesEmitter(pProp->GetParent()->GetParent(), emit_cmp);
			EventArg arg;
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
	case PropertyEmitterParticleColorA:
	case PropertyEmitterParticleColorR:
	case PropertyEmitterParticleColorG:
	case PropertyEmitterParticleColorB:
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
			case PropertyEmitterParticleColorA:
			case PropertyEmitterParticleColorR:
			case PropertyEmitterParticleColorG:
			case PropertyEmitterParticleColorB:
			case PropertyEmitterParticleSizeX:
			case PropertyEmitterParticleSizeY:
				pParticle = pProp->GetParent()->GetParent();
				break;
			case PropertyEmitterParticlePosition:
			case PropertyEmitterParticleVelocity:
			case PropertyEmitterParticleColor:
			case PropertyEmitterParticleSize:
			case PropertyEmitterParticleAngle:
				pParticle = pProp->GetParent();
				break;
			}
			DWORD NodeId = pParticle->GetData();
			EmitterComponent * emit_cmp = (EmitterComponent *)pParticle->GetParent()->GetParent()->GetValue().ulVal;
			my::Emitter::Particle & particle = emit_cmp->m_Emitter->m_ParticleList[NodeId];
			particle.m_Position.x = pParticle->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Position.y = pParticle->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Position.z = pParticle->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal;
			particle.m_Velocity.x = pParticle->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Velocity.y = pParticle->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Velocity.z = pParticle->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal;
			particle.m_Color.x = pParticle->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Color.y = pParticle->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Color.z = pParticle->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal;
			particle.m_Color.w = pParticle->GetSubItem(2)->GetSubItem(3)->GetValue().fltVal;
			particle.m_Size.x = pParticle->GetSubItem(3)->GetSubItem(0)->GetValue().fltVal;
			particle.m_Size.y = pParticle->GetSubItem(3)->GetSubItem(1)->GetValue().fltVal;
			particle.m_Angle = pParticle->GetSubItem(4)->GetValue().fltVal;
			EventArg arg;
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
			unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
			sphe_emit_cmp->m_ParticleLifeTime = pComponent->GetSubItem(PropId + 0)->GetValue().fltVal;
			sphe_emit_cmp->m_SpawnInterval = pComponent->GetSubItem(PropId + 1)->GetValue().fltVal;
			sphe_emit_cmp->m_HalfSpawnArea.x = pComponent->GetSubItem(PropId + 2)->GetSubItem(0)->GetValue().fltVal;
			sphe_emit_cmp->m_HalfSpawnArea.y = pComponent->GetSubItem(PropId + 2)->GetSubItem(1)->GetValue().fltVal;
			sphe_emit_cmp->m_HalfSpawnArea.z = pComponent->GetSubItem(PropId + 2)->GetSubItem(2)->GetValue().fltVal;
			sphe_emit_cmp->m_SpawnSpeed = pComponent->GetSubItem(PropId + 3)->GetValue().fltVal;
			sphe_emit_cmp->m_SpawnLoopTime = pComponent->GetSubItem(PropId + 13)->GetValue().fltVal;
			EventArg arg;
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
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyTerrainHeightScale:
		{
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->m_HeightScale = pProp->GetValue().fltVal;
			terrain->UpdateHeightMapNormal();
			terrain->UpdateChunks();
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyTerrainWrappedU:
	case PropertyTerrainWrappedV:
		{
			unsigned int PropId = GetComponentAttrCount(Component::ComponentTypeComponent);
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->m_WrappedU = pProp->GetParent()->GetSubItem(PropId + 3)->GetValue().fltVal;
			terrain->m_WrappedV = pProp->GetParent()->GetSubItem(PropId + 4)->GetValue().fltVal;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	case PropertyTerrainHeightMap:
		{
			std::string path = ts2ms(pProp->GetValue().bstrVal);
			my::Texture2DPtr res = boost::dynamic_pointer_cast<my::Texture2D>(theApp.LoadTexture(path));
			if (res)
			{
				Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
				terrain->UpdateHeightMap(res);
				EventArg arg;
				pFrame->m_EventAttributeChanged(&arg);
			}
		}
		break;
	case PropertyTerrainStaticCollision:
		{
			Terrain * terrain = (Terrain *)pProp->GetParent()->GetValue().ulVal;
			terrain->m_StaticCollision = pProp->GetValue().boolVal;
			EventArg arg;
			pFrame->m_EventAttributeChanged(&arg);
		}
		break;
	//case PropertyRigidShapeAdd:
	//	{
	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	//		DWORD NodeId = rigid_cmp->m_RigidActor->getNbShapes();
	//		int i = (DYNAMIC_DOWNCAST(CComboProp, pProp))->m_iSelIndex;
	//		PxShape * shape = NULL;
	//		switch (i)
	//		{
	//		case PxGeometryType::eSPHERE:
	//			shape = rigid_cmp->m_RigidActor->createShape(PxSphereGeometry(1.0f),
	//				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	//			break;
	//		case PxGeometryType::ePLANE:
	//			shape = rigid_cmp->m_RigidActor->createShape(PxPlaneGeometry(),
	//				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	//			break;
	//		case PxGeometryType::eCAPSULE:
	//			shape = rigid_cmp->m_RigidActor->createShape(PxCapsuleGeometry(1.0f, 1.0f),
	//				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	//			break;
	//		case PxGeometryType::eBOX:
	//			shape = rigid_cmp->m_RigidActor->createShape(PxBoxGeometry(PxVec3(1,1,1)),
	//				*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	//			break;
	//		case PxGeometryType::eCONVEXMESH:
	//			break;
	//		case PxGeometryType::eTRIANGLEMESH:
	//			break;
	//		case PxGeometryType::eHEIGHTFIELD:
	//			break;
	//		}
	//		if (shape)
	//		{
	//			CreatePropertiesShape(m_pProp[PropertyRigidShapeList], NodeId, (PxGeometryType::Enum)i);
	//			UpdatePropertiesShape(m_pProp[PropertyRigidShapeList], NodeId, shape);
	//		}
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyRigidShapePos:
	//case PropertyRigidShapePosX:
	//case PropertyRigidShapePosY:
	//case PropertyRigidShapePosZ:
	//case PropertyRigidShapeRot:
	//case PropertyRigidShapeRotX:
	//case PropertyRigidShapeRotY:
	//case PropertyRigidShapeRotZ:
	//	{
	//		CMFCPropertyGridProperty * pShape = NULL;
	//		switch (PropertyId)
	//		{
	//		case PropertyRigidShapePos:
	//		case PropertyRigidShapeRot:
	//			pShape = pProp->GetParent();
	//			break;
	//		case PropertyRigidShapePosX:
	//		case PropertyRigidShapePosY:
	//		case PropertyRigidShapePosZ:
	//		case PropertyRigidShapeRotX:
	//		case PropertyRigidShapeRotY:
	//		case PropertyRigidShapeRotZ:
	//			pShape = pProp->GetParent()->GetParent();
	//			break;
	//		}
	//		unsigned int NodeId = LOWORD(pShape->GetData());
	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	//		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	//		std::vector<PxShape *> shapes(NbShapes);
	//		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	//		my::Quaternion rot = my::Quaternion::RotationEulerAngles(my::Vector3(
	//			D3DXToRadian(pShape->GetSubItem(1)->GetSubItem(0)->GetValue().fltVal),
	//			D3DXToRadian(pShape->GetSubItem(1)->GetSubItem(1)->GetValue().fltVal),
	//			D3DXToRadian(pShape->GetSubItem(1)->GetSubItem(2)->GetValue().fltVal)));
	//		shapes[NodeId]->setLocalPose(PxTransform(PxVec3(
	//			pShape->GetSubItem(0)->GetSubItem(0)->GetValue().fltVal,
	//			pShape->GetSubItem(0)->GetSubItem(1)->GetValue().fltVal,
	//			pShape->GetSubItem(0)->GetSubItem(2)->GetValue().fltVal), (PxQuat&)rot));
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyRigidShapeCapsuleRadius:
	//case PropertyRigidShapeCapsuleHalfHeight:
	//	{
	//		CMFCPropertyGridProperty * pShape = pProp->GetParent();
	//		unsigned int NodeId = LOWORD(pShape->GetData());
	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	//		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	//		std::vector<PxShape *> shapes(NbShapes);
	//		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	//		shapes[NodeId]->setGeometry(PxCapsuleGeometry(
	//			pShape->GetSubItem(2)->GetValue().fltVal,
	//			pShape->GetSubItem(3)->GetValue().fltVal));
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyRigidShapeBoxHalfExtents:
	//case PropertyRigidShapeBoxHalfExtentsX:
	//case PropertyRigidShapeBoxHalfExtentsY:
	//case PropertyRigidShapeBoxHalfExtentsZ:
	//	{
	//		CMFCPropertyGridProperty * pShape = NULL;
	//		switch (PropertyId)
	//		{
	//		case PropertyRigidShapeBoxHalfExtents:
	//			pShape = pProp->GetParent();
	//			break;
	//		case PropertyRigidShapeBoxHalfExtentsX:
	//		case PropertyRigidShapeBoxHalfExtentsY:
	//		case PropertyRigidShapeBoxHalfExtentsZ:
	//			pShape = pProp->GetParent()->GetParent();
	//			break;
	//		}
	//		unsigned int NodeId = LOWORD(pShape->GetData());
	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	//		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	//		std::vector<PxShape *> shapes(NbShapes);
	//		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	//		shapes[NodeId]->setGeometry(PxBoxGeometry(PxVec3(
	//			pShape->GetSubItem(2)->GetSubItem(0)->GetValue().fltVal,
	//			pShape->GetSubItem(2)->GetSubItem(1)->GetValue().fltVal,
	//			pShape->GetSubItem(2)->GetSubItem(2)->GetValue().fltVal)));
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	//case PropertyRigidShapeSphereRadius:
	//	{
	//		CMFCPropertyGridProperty * pShape = pProp->GetParent();
	//		unsigned int NodeId = LOWORD(pShape->GetData());
	//		RigidComponent * rigid_cmp = dynamic_cast<RigidComponent *>(cmp);
	//		unsigned int NbShapes = rigid_cmp->m_RigidActor->getNbShapes();
	//		std::vector<PxShape *> shapes(NbShapes);
	//		NbShapes = rigid_cmp->m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	//		shapes[NodeId]->setGeometry(PxSphereGeometry(
	//			pShape->GetSubItem(2)->GetValue().fltVal));
	//		EventArg arg;
	//		pFrame->m_EventAttributeChanged(&arg);
	//	}
	//	break;
	}
	return 0;
}

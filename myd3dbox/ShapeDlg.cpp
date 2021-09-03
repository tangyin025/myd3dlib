// ShapeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ShapeDlg.h"
#include "afxdialogex.h"
#include "Actor.h"
#include "Terrain.h"

// CShapeDlg dialog

IMPLEMENT_DYNAMIC(CShapeDlg, CDialogEx)

CShapeDlg::CShapeDlg(CWnd* pParent, Component * m_cmp, int type)
	: CDialogEx(CShapeDlg::IDD, pParent)
	, m_cmp(m_cmp)
	, m_type(type)
	, m_pos(0,0,0)
	, m_angle(0, 0, 0)
	, m_param(1,1,1)
	, m_filterWord0(1)
	, m_InflateConvex(FALSE)
{

}

CShapeDlg::~CShapeDlg()
{
}

void CShapeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_type);
	DDX_Text(pDX, IDC_EDIT1, m_pos.x);
	DDX_Text(pDX, IDC_EDIT2, m_pos.y);
	DDX_Text(pDX, IDC_EDIT3, m_pos.z);
	DDX_Text(pDX, IDC_EDIT4, m_angle.x);
	DDX_Text(pDX, IDC_EDIT5, m_angle.y);
	DDX_Text(pDX, IDC_EDIT6, m_angle.z);
	DDX_Text(pDX, IDC_EDIT7, m_param.x);
	DDX_Text(pDX, IDC_EDIT8, m_param.y);
	DDX_Text(pDX, IDC_EDIT9, m_param.z);
	DDX_Text(pDX, IDC_EDIT10, m_filterWord0);
	DDX_Check(pDX, IDC_CHECK1, m_InflateConvex);
}


BEGIN_MESSAGE_MAP(CShapeDlg, CDialogEx)
END_MESSAGE_MAP()


// CShapeDlg message handlers


void CShapeDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	// ! physx attached shape is not writable
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	BOOL NeedRequest = FALSE;
	if (m_cmp->IsRequested())
	{
		m_cmp->ReleaseResource();
		NeedRequest = TRUE;
	}

	m_cmp->ClearShape();
	my::Quaternion rot(my::Quaternion::RotationEulerAngles(D3DXToRadian(m_angle.x), D3DXToRadian(m_angle.y), D3DXToRadian(m_angle.z)));
	switch (m_type)
	{
	case physx::PxGeometryType::eSPHERE:
		m_cmp->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
		m_cmp->CreateSphereShape(m_pos, rot, m_param.x, true, pFrame->m_CollectionObjs);
		m_cmp->SetSimulationFilterWord0(m_filterWord0);
		m_cmp->SetQueryFilterWord0(m_filterWord0);
		break;
	case physx::PxGeometryType::ePLANE:
		m_cmp->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
		m_cmp->CreatePlaneShape(m_pos, rot, true, pFrame->m_CollectionObjs);
		m_cmp->SetSimulationFilterWord0(m_filterWord0);
		m_cmp->SetQueryFilterWord0(m_filterWord0);
		break;
	case physx::PxGeometryType::eCAPSULE:
		m_cmp->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
		m_cmp->CreateCapsuleShape(m_pos, rot, m_param.x, m_param.y, true, pFrame->m_CollectionObjs);
		m_cmp->SetSimulationFilterWord0(m_filterWord0);
		m_cmp->SetQueryFilterWord0(m_filterWord0);
		break;
	case physx::PxGeometryType::eBOX:
		m_cmp->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
		m_cmp->CreateBoxShape(m_pos, rot, m_param.x, m_param.y, m_param.z, true, pFrame->m_CollectionObjs);
		m_cmp->SetSimulationFilterWord0(m_filterWord0);
		m_cmp->SetQueryFilterWord0(m_filterWord0);
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		if (m_cmp->GetComponentType() == Component::ComponentTypeMesh)
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_cmp);
			mesh_cmp->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
			mesh_cmp->CreateConvexMeshShape(m_InflateConvex != FALSE, true, pFrame->m_CollectionObjs);
			//mesh_cmp->SetSimulationFilterWord0(m_filterWord0);
			//mesh_cmp->SetQueryFilterWord0(m_filterWord0);
		}
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		if (m_cmp->GetComponentType() == Component::ComponentTypeMesh)
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_cmp);
			mesh_cmp->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
			mesh_cmp->CreateTriangleMeshShape("mesh/aaa.triangle_mesh", true, pFrame->m_CollectionObjs);
			//mesh_cmp->SetSimulationFilterWord0(m_filterWord0);
			//mesh_cmp->SetQueryFilterWord0(m_filterWord0);
		}
		break;
	case physx::PxGeometryType::eHEIGHTFIELD:
		if (m_cmp->GetComponentType() == Component::ComponentTypeTerrain)
		{
			Terrain * terrain = dynamic_cast<Terrain *>(m_cmp);
			terrain->m_PxShapeGeometryType = (physx::PxGeometryType::Enum)m_type;
			terrain->CreateHeightFieldShape(true, pFrame->m_CollectionObjs);
			terrain->SetSimulationFilterWord0(m_filterWord0);
			terrain->SetQueryFilterWord0(m_filterWord0);
		}
		break;
	}

	if (NeedRequest)
	{
		m_cmp->RequestResource();
	}

	EndDialog(IDOK);
}

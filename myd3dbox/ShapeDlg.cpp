// ShapeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "ShapeDlg.h"
#include "afxdialogex.h"
#include "../demo2_3/Component/Actor.h"
#include "../demo2_3/Component/Terrain.h"

// CShapeDlg dialog

IMPLEMENT_DYNAMIC(CShapeDlg, CDialogEx)

CShapeDlg::CShapeDlg(CWnd* pParent, Component * m_cmp, int type)
	: CDialogEx(CShapeDlg::IDD, pParent)
	, m_cmp(m_cmp)
	, m_type(type)
	, m_pos(0,0,0)
	, m_angle(0, 0, 0)
	, m_param(1,1,1)
{

}

CShapeDlg::~CShapeDlg()
{
}

void CShapeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_pos.x);
	DDX_Text(pDX, IDC_EDIT2, m_pos.y);
	DDX_Text(pDX, IDC_EDIT3, m_pos.z);
	DDX_Text(pDX, IDC_EDIT4, m_angle.x);
	DDX_Text(pDX, IDC_EDIT5, m_angle.y);
	DDX_Text(pDX, IDC_EDIT6, m_angle.z);
	DDX_Text(pDX, IDC_EDIT7, m_param.x);
	DDX_Text(pDX, IDC_EDIT8, m_param.y);
	DDX_Text(pDX, IDC_EDIT9, m_param.z);
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
	m_cmp->ClearShape();
	my::Quaternion rot(my::Quaternion::RotationEulerAngles(my::Vector3(D3DXToRadian(m_angle.x), D3DXToRadian(m_angle.y), D3DXToRadian(m_angle.z))));
	switch (m_type)
	{
	case physx::PxGeometryType::eSPHERE:
		m_cmp->CreateSphereShape(m_pos, rot, m_param.x);
		break;
	case physx::PxGeometryType::ePLANE:
		m_cmp->CreatePlaneShape(m_pos, rot);
		break;
	case physx::PxGeometryType::eCAPSULE:
		m_cmp->CreateCapsuleShape(m_pos, rot, m_param.x, m_param.y);
		break;
	case physx::PxGeometryType::eBOX:
		m_cmp->CreateBoxShape(m_pos, rot, m_param.x, m_param.y, m_param.z);
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		if (m_cmp->m_Type == Component::ComponentTypeMesh)
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_cmp);
			mesh_cmp->CreateTriangleMeshShape(m_param);
		}
		break;
	case physx::PxGeometryType::eHEIGHTFIELD:
		if (m_cmp->m_Type == Component::ComponentTypeTerrain)
		{
			Terrain * terrain = dynamic_cast<Terrain *>(m_cmp);
			terrain->CreateHeightFieldShape(m_param);
		}
		break;
	}
	EndDialog(IDOK);
}

// ShapeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "ShapeDlg.h"
#include "afxdialogex.h"
#include "Component\Actor.h"
#include "Component\Terrain.h"

// CShapeDlg dialog

IMPLEMENT_DYNAMIC(CShapeDlg, CDialogEx)

CShapeDlg::CShapeDlg(CWnd* pParent, Component * m_cmp, int type)
	: CDialogEx(CShapeDlg::IDD, pParent)
	, m_cmp(m_cmp)
	, m_type(type)
{

}

CShapeDlg::~CShapeDlg()
{
}

void CShapeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShapeDlg, CDialogEx)
END_MESSAGE_MAP()


// CShapeDlg message handlers


void CShapeDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	// ! physx attached shape is not writable
	m_cmp->ClearShape();
	switch (m_type)
	{
	case physx::PxGeometryType::eSPHERE:
		m_cmp->CreateSphereShape(m_cmp->m_Actor->m_Scale.x);
		break;
	case physx::PxGeometryType::ePLANE:
		m_cmp->CreatePlaneShape();
		break;
	case physx::PxGeometryType::eCAPSULE:
		m_cmp->CreateCapsuleShape(m_cmp->m_Actor->m_Scale.x, m_cmp->m_Actor->m_Scale.y);
		break;
	case physx::PxGeometryType::eBOX:
		m_cmp->CreateBoxShape(m_cmp->m_Actor->m_Scale.x, m_cmp->m_Actor->m_Scale.y, m_cmp->m_Actor->m_Scale.z);
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		if (m_cmp->m_Type == Component::ComponentTypeMesh)
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_cmp);
			mesh_cmp->CreateTriangleMeshShape(m_cmp->m_Actor->m_Scale);
		}
		break;
	case physx::PxGeometryType::eHEIGHTFIELD:
		if (m_cmp->m_Type == Component::ComponentTypeTerrain)
		{
			Terrain * terrain = dynamic_cast<Terrain *>(m_cmp);
			terrain->CreateHeightFieldShape(m_cmp->m_Actor->m_Scale);
		}
		break;
	}
	CDialogEx::OnOK();
}

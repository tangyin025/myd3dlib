// Copyright (c) 2011-2024 tangyin025
// License: MIT

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
	, m_InflateConvex(FALSE)
	, m_AssetPath(_T(""))
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
	DDX_Text(pDX, IDC_EDIT10, theApp.default_physx_shape_filterword0);
	DDX_Check(pDX, IDC_CHECK1, m_InflateConvex);
	DDX_Text(pDX, IDC_EDIT11, m_AssetPath);
}


BEGIN_MESSAGE_MAP(CShapeDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT11, &CShapeDlg::OnChangeEdit11)
	ON_BN_CLICKED(IDC_BUTTON4, &CShapeDlg::OnClickedButton4)
END_MESSAGE_MAP()


// CShapeDlg message handlers


BOOL CShapeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	OnChangeEdit11();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CShapeDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	if ((m_type == physx::PxGeometryType::eCONVEXMESH
		|| m_type == physx::PxGeometryType::eTRIANGLEMESH
		|| m_type == physx::PxGeometryType::eHEIGHTFIELD)
		&& m_AssetPath.IsEmpty())
	{
		MessageBox(_T("m_AssetPath.IsEmpty()"));
		return;
	}

	// ! physx attached shape is not writable
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	m_cmp->ClearShape();
	my::Quaternion rot(my::Quaternion::RotationEulerAngles(D3DXToRadian(m_angle.x), D3DXToRadian(m_angle.y), D3DXToRadian(m_angle.z)));
	switch (m_type)
	{
	case physx::PxGeometryType::eSPHERE:
		m_cmp->CreateSphereShape(m_pos, rot, m_param.x, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		m_cmp->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
		m_cmp->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		break;
	case physx::PxGeometryType::ePLANE:
		m_cmp->CreatePlaneShape(m_pos, rot, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		m_cmp->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
		m_cmp->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		break;
	case physx::PxGeometryType::eCAPSULE:
		m_cmp->CreateCapsuleShape(m_pos, rot, m_param.x, m_param.y, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		m_cmp->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
		m_cmp->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		break;
	case physx::PxGeometryType::eBOX:
		m_cmp->CreateBoxShape(m_pos, rot, m_param.x, m_param.y, m_param.z, MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);
		m_cmp->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
		m_cmp->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		break;
	case physx::PxGeometryType::eCONVEXMESH:
		if (m_cmp->GetComponentType() == Component::ComponentTypeMesh)
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_cmp);
			mesh_cmp->CreateConvexMeshShape(
				theApp.LoadMesh(mesh_cmp->m_MeshPath.c_str()).get(), ts2ms((LPCTSTR)m_AssetPath).c_str(), m_InflateConvex != FALSE);
			mesh_cmp->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
			mesh_cmp->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		}
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		if (m_cmp->GetComponentType() == Component::ComponentTypeMesh)
		{
			MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(m_cmp);
			mesh_cmp->CreateTriangleMeshShape(
				theApp.LoadMesh(mesh_cmp->m_MeshPath.c_str()).get(), ts2ms((LPCTSTR)m_AssetPath).c_str());
			mesh_cmp->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
			mesh_cmp->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		}
		break;
	case physx::PxGeometryType::eHEIGHTFIELD:
		if (m_cmp->GetComponentType() == Component::ComponentTypeTerrain)
		{
			Terrain * terrain = dynamic_cast<Terrain *>(m_cmp);
			terrain->CreateHeightFieldShape(
				&TerrainStream(terrain), ts2ms((LPCTSTR)m_AssetPath).c_str(), terrain->m_Actor->m_Scale);
			terrain->SetSimulationFilterWord0(theApp.default_physx_shape_filterword0);
			terrain->SetQueryFilterWord0(theApp.default_physx_shape_filterword0);
		}
		break;
	}

	EndDialog(IDOK);
}


void CShapeDlg::OnChangeEdit11()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	if (m_type != physx::PxGeometryType::eCONVEXMESH
		&& m_type != physx::PxGeometryType::eTRIANGLEMESH
		&& m_type != physx::PxGeometryType::eHEIGHTFIELD
		&& GetDlgItem(IDC_BUTTON4)->IsWindowEnabled())
	{
		GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
		return;
	}

	CString strText;
	GetDlgItemText(IDC_EDIT11, strText);
	if (my::ResourceMgr::getSingleton().CheckPath(theApp.GetFullPath(ts2ms((LPCTSTR)strText).c_str()).c_str()))
	{
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	}
}


void CShapeDlg::OnClickedButton4()
{
	// TODO: Add your control notification handler code here
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	std::string FullPath = theApp.GetFullPath(ts2ms((LPCTSTR)m_AssetPath).c_str());
	SHFILEOPSTRUCTA shfo;
	ZeroMemory(&shfo, sizeof(shfo));
	shfo.hwnd = AfxGetMainWnd()->m_hWnd;
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = FullPath.c_str();
	shfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_NORECURSION;
	int res = SHFileOperationA(&shfo);
	if (res != 0)
	{
		MessageBox(str_printf(_T("SHFileOperation failed: %s"), ms2ts(FullPath.c_str()).c_str()).c_str());
	}

	OnChangeEdit11();
}

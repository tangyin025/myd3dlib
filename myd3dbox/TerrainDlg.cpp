// TerrainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "TerrainDlg.h"
#include "afxdialogex.h"
#include "CtrlProps.h"
#include "Component.h"
#include "PropertiesWnd.h"

// CTerrainDlg dialog

IMPLEMENT_DYNAMIC(CTerrainDlg, CDialogEx)

CTerrainDlg::CTerrainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTerrainDlg::IDD, pParent)
	, m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(32)
	, m_AlignToCenter(TRUE)
	, m_UseTerrainMaterial(TRUE)
	, m_UseWaterMaterial(FALSE)
{
}

CTerrainDlg::~CTerrainDlg()
{
}

void CTerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_RowChunks);
	DDX_Text(pDX, IDC_EDIT2, m_ColChunks);
	DDX_Text(pDX, IDC_EDIT3, m_ChunkSize);
	DDX_Check(pDX, IDC_CHECK1, m_AlignToCenter);
	DDX_Check(pDX, IDC_CHECK2, m_UseTerrainMaterial);
	DDX_Check(pDX, IDC_CHECK3, m_UseWaterMaterial);
}


BEGIN_MESSAGE_MAP(CTerrainDlg, CDialogEx)
END_MESSAGE_MAP()


// CTerrainDlg message handlers


BOOL CTerrainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	m_terrain.reset(new Terrain(my::NamedObject::MakeUniqueName("editor_terrain").c_str(), m_RowChunks, m_ColChunks, m_ChunkSize, 1.0f));

	if (m_UseTerrainMaterial)
	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_shader;
		mtl->ParseShaderParameters();
		m_terrain->SetMaterial(mtl);
	}

	if (m_UseWaterMaterial)
	{
		MaterialPtr mtl(new Material());
		mtl->m_Shader = theApp.default_water_shader;
		mtl->ParseShaderParameters();
		m_terrain->SetMaterial(mtl);
	}

	EndDialog(IDOK);
}

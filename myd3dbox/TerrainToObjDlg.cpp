// TerrainToObjDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainToObjDlg.h"
#include "afxdialogex.h"
#include "MainApp.h"
#include "Terrain.h"

// CTerrainToObjDlg dialog

IMPLEMENT_DYNAMIC(CTerrainToObjDlg, CDialogEx)

CTerrainToObjDlg::CTerrainToObjDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG8, pParent)
	, m_terrain(NULL)
	, m_path(_T("aaa.obj"))
	, m_i0(0)
	, m_j0(0)
	, m_i1(1)
	, m_j1(1)
{
}

CTerrainToObjDlg::~CTerrainToObjDlg()
{
}

void CTerrainToObjDlg::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(m_terrain);
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_path);
	DDX_Text(pDX, IDC_EDIT2, m_i0);
	DDV_MinMaxInt(pDX, m_i0, 0, m_terrain->m_RowChunks * m_terrain->m_ChunkSize - 1);
	DDX_Text(pDX, IDC_EDIT3, m_j0);
	DDV_MinMaxInt(pDX, m_j0, 0, m_terrain->m_ColChunks * m_terrain->m_ChunkSize - 1);
	DDX_Text(pDX, IDC_EDIT4, m_i1);
	DDV_MinMaxInt(pDX, m_i1, 1, m_terrain->m_RowChunks * m_terrain->m_ChunkSize);
	DDX_Text(pDX, IDC_EDIT5, m_j1);
	DDV_MinMaxInt(pDX, m_j1, 1, m_terrain->m_ColChunks * m_terrain->m_ChunkSize);
}


BEGIN_MESSAGE_MAP(CTerrainToObjDlg, CDialogEx)
END_MESSAGE_MAP()


// CTerrainToObjDlg message handlers


BOOL CTerrainToObjDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CTerrainToObjDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	if (!UpdateData(TRUE))
	{
		TRACE(traceAppMsg, 0, "UpdateData failed during dialog termination.\n");
		// the UpdateData routine will set focus to correct item
		return;
	}

	TerrainStream tstr(m_terrain);
	tstr.SaveObjMesh(ts2ms((LPCTSTR)m_path).c_str(), m_i0, m_j0, m_i1, m_j1);

	CDialogEx::OnOK();
}

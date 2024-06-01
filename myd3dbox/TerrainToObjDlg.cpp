// Copyright (c) 2011-2024 tangyin025
// License: MIT

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
	, m_path(theApp.GetProfileString(_T("Settings"), _T("TerrainToObjPath"), _T("aaa.obj")))
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
	DDX_Text(pDX, IDC_EDIT2, m_j0);
	DDV_MinMaxInt(pDX, m_j0, 0, m_terrain->m_ColChunks * m_terrain->m_ChunkSize - 1);
	DDX_Text(pDX, IDC_EDIT3, m_i0);
	DDV_MinMaxInt(pDX, m_i0, 0, m_terrain->m_RowChunks * m_terrain->m_ChunkSize - 1);
	DDX_Text(pDX, IDC_EDIT4, m_j1);
	DDV_MinMaxInt(pDX, m_j1, 1, m_terrain->m_ColChunks * m_terrain->m_ChunkSize);
	DDX_Text(pDX, IDC_EDIT5, m_i1);
	DDV_MinMaxInt(pDX, m_i1, 1, m_terrain->m_RowChunks * m_terrain->m_ChunkSize);

	if (pDX->m_bSaveAndValidate)
	{
		theApp.WriteProfileString(_T("Settings"), _T("TerrainToObjPath"), m_path);
	}
}


BEGIN_MESSAGE_MAP(CTerrainToObjDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CTerrainToObjDlg::OnBnClickedButton1)
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
	std::ofstream ofs(ts2ms((LPCTSTR)m_path));
	ofs << "g aaa\n";

	std::vector<int> faces;
	for (int i = m_i0; i <= m_i1; i++)
	{
		for (int j = m_j0; j <= m_j1; j++)
		{
			const my::Vector3 pos = tstr.GetPos(i, j);
			const my::Vector3 nor = tstr.GetNormal(i, j);
			const my::Vector2 tex(pos.x / m_terrain->m_ColChunks / m_terrain->m_ChunkSize, 1 - pos.z / m_terrain->m_RowChunks / m_terrain->m_ChunkSize);

			ofs << "v " << pos.x << " " << pos.y << " " << pos.z << std::endl;
			ofs << "vn " << nor.x << " " << nor.y << " " << nor.z << std::endl;
			ofs << "vt " << tex.x << " " << tex.y << std::endl;

			const int f0 = (i - m_i0 + 0) * (m_j1 - m_j0 + 1) + (j - m_j0 + 0) + 1;
			const int f1 = (i - m_i0 + 1) * (m_j1 - m_j0 + 1) + (j - m_j0 + 0) + 1;
			const int f2 = (i - m_i0 + 0) * (m_j1 - m_j0 + 1) + (j - m_j0 + 1) + 1;
			const int f3 = (i - m_i0 + 1) * (m_j1 - m_j0 + 1) + (j - m_j0 + 1) + 1;

			if (i < m_i1 && j < m_j1)
			{
				faces.push_back(f0);
				faces.push_back(f1);
				faces.push_back(f2);
				faces.push_back(f2);
				faces.push_back(f1);
				faces.push_back(f3);
			}
		}
	}

	for (int i = 0; i < faces.size(); i += 3)
	{
		ofs << "f " << faces[i + 0] << "/" << faces[i + 0] << "/" << faces[i + 0]
			<< " " << faces[i + 1] << "/" << faces[i + 1] << "/" << faces[i + 1]
			<< " " << faces[i + 2] << "/" << faces[i + 2] << "/" << faces[i + 2] << std::endl;
	}

	CDialogEx::OnOK();
}


void CTerrainToObjDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(FALSE, NULL, m_path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_EDIT1, dlg.GetPathName());
	}
}

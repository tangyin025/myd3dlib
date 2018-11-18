// TerrainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "TerrainDlg.h"
#include "afxdialogex.h"


// TerrainDlg dialog

IMPLEMENT_DYNAMIC(TerrainDlg, CDialogEx)

TerrainDlg::TerrainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(TerrainDlg::IDD, pParent)
	, m_RowChunks(8)
	, m_ColChunks(8)
	, m_ChunkSize(32)
	, m_DiffuseTexture(ms2ts(theApp.default_texture).c_str())
	, m_NormalTexture(ms2ts(theApp.default_normal_texture).c_str())
	, m_SpecularTexture(ms2ts(theApp.default_specular_texture).c_str())
{

}

TerrainDlg::~TerrainDlg()
{
}

void TerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_RowChunks);
	DDX_Text(pDX, IDC_EDIT2, m_ColChunks);
	DDX_Text(pDX, IDC_EDIT3, m_ChunkSize);
	DDX_Text(pDX, IDC_EDIT4, m_DiffuseTexture);
	DDX_Text(pDX, IDC_EDIT5, m_NormalTexture);
	DDX_Text(pDX, IDC_EDIT6, m_SpecularTexture);
}


BEGIN_MESSAGE_MAP(TerrainDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &TerrainDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &TerrainDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &TerrainDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// TerrainDlg message handlers


void TerrainDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, m_DiffuseTexture, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (IDOK != dlg.DoModal())
	{
		return;
	}
	m_DiffuseTexture = dlg.GetPathName();
	UpdateData(FALSE);
}


void TerrainDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, m_NormalTexture, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (IDOK != dlg.DoModal())
	{
		return;
	}
	m_NormalTexture = dlg.GetPathName();
	UpdateData(FALSE);
}


void TerrainDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, m_SpecularTexture, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (IDOK != dlg.DoModal())
	{
		return;
	}
	m_SpecularTexture = dlg.GetPathName();
	UpdateData(FALSE);
}

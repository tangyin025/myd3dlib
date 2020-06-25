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
	, m_UseWaterMaterial(TRUE)
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


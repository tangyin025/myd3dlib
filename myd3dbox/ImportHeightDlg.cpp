// ImportHeightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImportHeightDlg.h"
#include "afxdialogex.h"
#include "MainApp.h"


// ImportHeightDlg dialog

IMPLEMENT_DYNAMIC(ImportHeightDlg, CDialogEx)

ImportHeightDlg::ImportHeightDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG3, pParent)
	, m_AssetPath(_T(""))
	, m_TerrainSize(513)
	, m_MaxHeight(2625)
	, m_WaterLevel(1150)
{

}

ImportHeightDlg::~ImportHeightDlg()
{
}

void ImportHeightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_AssetPath);
	DDX_Text(pDX, IDC_EDIT2, m_TerrainSize);
	DDX_Text(pDX, IDC_EDIT3, m_MaxHeight);
	DDX_Text(pDX, IDC_EDIT4, m_WaterLevel);
}


BEGIN_MESSAGE_MAP(ImportHeightDlg, CDialogEx)
END_MESSAGE_MAP()


// ImportHeightDlg message handlers

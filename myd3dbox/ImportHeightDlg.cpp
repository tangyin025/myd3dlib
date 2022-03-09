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
	, m_TextureSize(513, 513)
	, m_MaxHeight(theApp.default_terrain_max_height)
	, m_WaterLevel(theApp.default_terrain_water_level)
{

}

ImportHeightDlg::~ImportHeightDlg()
{
}

void ImportHeightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_AssetPath);
	DDX_Text(pDX, IDC_EDIT2, m_TextureSize.cx);
	DDX_Text(pDX, IDC_EDIT3, m_TextureSize.cy);
	DDX_Text(pDX, IDC_EDIT4, m_MaxHeight);
	DDX_Text(pDX, IDC_EDIT5, m_WaterLevel);
}


BEGIN_MESSAGE_MAP(ImportHeightDlg, CDialogEx)
END_MESSAGE_MAP()


// ImportHeightDlg message handlers

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
{

}

TerrainDlg::~TerrainDlg()
{
}

void TerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TerrainDlg, CDialogEx)
END_MESSAGE_MAP()


// TerrainDlg message handlers

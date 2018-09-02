// NavigationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "NavigationDlg.h"
#include "afxdialogex.h"


// CNavigationDlg dialog

IMPLEMENT_DYNAMIC(CNavigationDlg, CDialogEx)

CNavigationDlg::CNavigationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNavigationDlg::IDD, pParent)
{

}

CNavigationDlg::~CNavigationDlg()
{
}

void CNavigationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNavigationDlg, CDialogEx)
END_MESSAGE_MAP()


// CNavigationDlg message handlers

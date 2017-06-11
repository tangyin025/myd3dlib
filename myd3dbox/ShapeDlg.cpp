// ShapeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainApp.h"
#include "ShapeDlg.h"
#include "afxdialogex.h"


// CShapeDlg dialog

IMPLEMENT_DYNAMIC(CShapeDlg, CDialogEx)

CShapeDlg::CShapeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CShapeDlg::IDD, pParent)
{

}

CShapeDlg::~CShapeDlg()
{
}

void CShapeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShapeDlg, CDialogEx)
END_MESSAGE_MAP()


// CShapeDlg message handlers

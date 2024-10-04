// Copyright (c) 2011-2024 tangyin025
// License: MIT

#include "stdafx.h"
#include "DeleteCmpsDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "MainFrm.h"

// CDeleteCmpsDlg dialog

IMPLEMENT_DYNAMIC(CDeleteCmpsDlg, CDialogEx)

CDeleteCmpsDlg::CDeleteCmpsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG6, pParent)
{

}

CDeleteCmpsDlg::~CDeleteCmpsDlg()
{
}

void CDeleteCmpsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CDeleteCmpsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDeleteCmpsDlg::OnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDeleteCmpsDlg::OnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDeleteCmpsDlg::OnClickedButton3)
END_MESSAGE_MAP()


// CDeleteCmpsDlg message handlers


BOOL CDeleteCmpsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	m_listCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES);
	m_listCtrl.InsertColumn(0, _T("Name"), 0, 150, -1);
	m_listCtrl.InsertColumn(1, _T("Type"), 0, 100, -1);
	Actor::ComponentPtrList::iterator cmp_iter = pFrame->m_selactors.front()->m_Cmps.begin();
	for (; cmp_iter != pFrame->m_selactors.front()->m_Cmps.end(); cmp_iter++)
	{
		int nItem = std::distance(pFrame->m_selactors.front()->m_Cmps.begin(), cmp_iter);
		m_listCtrl.InsertItem(LVIF_TEXT | LVIF_STATE, nItem, ms2ts((*cmp_iter)->GetName()).c_str(), 0, 0, 0, 0);
		m_listCtrl.SetItemText(nItem, 1, CPropertiesWnd::GetComponentTypeName((*cmp_iter)->GetComponentType()));

		if (!pFrame->m_selcmp || pFrame->m_selcmp == cmp_iter->get())
		{
			m_listCtrl.SetCheck(nItem, TRUE);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDeleteCmpsDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	for (int nItem = m_listCtrl.GetItemCount() - 1; nItem >= 0; nItem--)
	{
		if (m_listCtrl.GetCheck(nItem))
		{
			pFrame->m_selactors.front()->RemoveComponent(nItem);
		}
	}

	CDialogEx::OnOK();
}


void CDeleteCmpsDlg::OnClickedButton1()
{
	// TODO: Add your control notification handler code here
	for (int nItem = m_listCtrl.GetItemCount() - 1; nItem >= 0; nItem--)
	{
		m_listCtrl.SetCheck(nItem, TRUE);
	}
}


void CDeleteCmpsDlg::OnClickedButton2()
{
	// TODO: Add your control notification handler code here
	for (int nItem = m_listCtrl.GetItemCount() - 1; nItem >= 0; nItem--)
	{
		m_listCtrl.SetCheck(nItem, FALSE);
	}
}


void CDeleteCmpsDlg::OnClickedButton3()
{
	// TODO: Add your control notification handler code here
	for (int nItem = m_listCtrl.GetItemCount() - 1; nItem >= 0; nItem--)
	{
		m_listCtrl.SetCheck(nItem, m_listCtrl.GetCheck(nItem) ? FALSE : TRUE);
	}
}

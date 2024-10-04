// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

// CDeleteCmpsDlg dialog

class CDeleteCmpsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDeleteCmpsDlg)

public:
	CDeleteCmpsDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDeleteCmpsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG6 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_listCtrl;
	virtual void OnOK();
	afx_msg void OnClickedButton1();
	afx_msg void OnClickedButton2();
	afx_msg void OnClickedButton3();
};

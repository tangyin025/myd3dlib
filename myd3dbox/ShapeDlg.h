// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

class Component;

// CShapeDlg dialog

class CShapeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShapeDlg)

public:
	CShapeDlg(CWnd* pParent, Component * cmp, int type);   // standard constructor
	virtual ~CShapeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	Component *m_cmp;
	int m_type;
	my::Vector3 m_pos;
	my::Vector3 m_angle;
	my::Vector3 m_param;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
public:
	BOOL m_InflateConvex;
	CString m_AssetPath;
	afx_msg void OnChangeEdit11();
	afx_msg void OnClickedButton4();
};

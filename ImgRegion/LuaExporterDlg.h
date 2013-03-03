#pragma once

#include "resource.h"

#include "ImgRegionDoc.h"

// CLuaExporterDlg dialog

class CLuaExporterDlg : public CDialog
{
	DECLARE_DYNAMIC(CLuaExporterDlg)

public:
	CLuaExporterDlg(CImgRegionDoc * pDoc, CWnd* pParent = NULL);   // standard constructor

	virtual ~CLuaExporterDlg();

	enum { IDD = IDD_LUA_EXPORTER };

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CImgRegionDoc * m_pDoc;

	CString m_strProjectDir;

	CString m_strLuaPath;

	afx_msg void OnBnClickedButton1();

	afx_msg void OnBnClickedButton2();

	virtual void OnOK();
};

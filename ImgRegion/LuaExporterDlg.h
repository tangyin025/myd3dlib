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
public:
	CString m_strProjectDir;

	CString m_strLuaPath;

	BOOL m_dirtyFlag;

	afx_msg void OnBnClickedButton1();

	afx_msg void OnBnClickedButton2();

	void ExportTreeNodeToLua(std::ofstream & ofs, HTREEITEM hItem, int indent = 1);

	virtual void OnOK();
};

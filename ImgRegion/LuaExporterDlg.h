#pragma once

#include "resource.h"

// CLuaExporterDlg dialog

class CLuaExporterDlg : public CDialog
{
	DECLARE_DYNAMIC(CLuaExporterDlg)

public:
	CLuaExporterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLuaExporterDlg();

// Dialog Data
	enum { IDD = IDD_LUA_EXPORTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

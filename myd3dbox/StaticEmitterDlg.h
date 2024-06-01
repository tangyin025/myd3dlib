// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "StaticEmitter.h"

// CStaticEmitterDlg dialog

class CStaticEmitterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStaticEmitterDlg)

public:
	CStaticEmitterDlg(const char* StaticEmitterName, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CStaticEmitterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG5 };
#endif

	StaticEmitterPtr m_emit_cmp;

	std::string m_emit_cmp_name;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_AssetPath;
	my::AABB m_BoundingBox;
	float m_ChunkWidth;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeEdit1();
	float m_ChunkLodScale;
};

// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "resource.h"

class CImgRegionFilePropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(CImgRegionFilePropertyDlg)

public:
	CImgRegionFilePropertyDlg(CWnd* pParent = NULL);

	virtual ~CImgRegionFilePropertyDlg();

	enum { IDD = IDD_FILE_PROPERTY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	CSize m_Size;

	CMFCColorButton m_btnColor;

	COLORREF m_Color;

	CString m_ImageStr;

	ImagePtr m_Image;

	afx_msg void OnBnClickedOpenImage();

	virtual BOOL OnInitDialog();
};

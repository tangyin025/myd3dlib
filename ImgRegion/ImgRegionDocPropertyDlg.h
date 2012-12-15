#pragma once

#include "resource.h"
#include "afxwin.h"

class CImgRegionDocPropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(CImgRegionDocPropertyDlg)

public:
	CImgRegionDocPropertyDlg(CWnd* pParent = NULL);

	virtual ~CImgRegionDocPropertyDlg();

	enum { IDD = IDD_DOC_PROPERTY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	CSize m_Size;

	CMFCColorButton m_btnColor;

	COLORREF m_Color;

	CString m_ImageStr;

	CComboBox m_cbxFontFamily;

	CString m_strFontFamily;

	LONG m_FontSize;

	afx_msg void OnBnClickedOpenImage();

	virtual BOOL OnInitDialog();
};

#pragma once

#include "resource.h"
#include "afxwin.h"

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

	CComboBox m_cbxFontFamily;

	CString m_strFontFamily;

	LONG m_FontSize;

	afx_msg void OnBnClickedOpenImage();

	virtual BOOL OnInitDialog();
};

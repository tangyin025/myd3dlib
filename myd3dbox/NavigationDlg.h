#pragma once


// CNavigationDlg dialog

class CNavigationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNavigationDlg)

public:
	CNavigationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNavigationDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

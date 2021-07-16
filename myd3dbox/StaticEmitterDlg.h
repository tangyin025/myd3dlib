#pragma once


// CStaticEmitterDlg dialog

class CStaticEmitterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStaticEmitterDlg)

public:
	CStaticEmitterDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CStaticEmitterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG5 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

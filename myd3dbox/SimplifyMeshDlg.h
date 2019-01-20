#pragma once


// CSimplifyMeshDlg dialog

class CSimplifyMeshDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSimplifyMeshDlg)

public:
	CSimplifyMeshDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSimplifyMeshDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG4 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

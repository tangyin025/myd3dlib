#pragma once


// CSnapshotDlg dialog

class CSnapshotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSnapshotDlg)

public:
	CSnapshotDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSnapshotDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG7 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CString m_TexPath;
	int m_TexWidth;
	int m_TexHeight;
	my::Rectangle m_SnapArea;
	int m_SnapCol;
	int m_SnapRow;
	afx_msg void OnClickedButton1();
};

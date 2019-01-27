#pragma once


// CTerrainGrassBrashDlg dialog

class CTerrainGrassBrashDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTerrainGrassBrashDlg)

public:
	CTerrainGrassBrashDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTerrainGrassBrashDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG5 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

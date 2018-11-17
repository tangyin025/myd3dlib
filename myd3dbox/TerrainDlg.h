#pragma once


// TerrainDlg dialog

class TerrainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TerrainDlg)

public:
	TerrainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~TerrainDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

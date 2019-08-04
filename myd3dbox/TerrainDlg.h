#pragma once
#include "afxpropertygridctrl.h"


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
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	int m_RowChunks;
	int m_ColChunks;
	int m_ChunkSize;
};

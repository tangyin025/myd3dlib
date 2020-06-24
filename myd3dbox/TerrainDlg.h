#pragma once
#include "afxpropertygridctrl.h"


// CTerrainDlg dialog

class CTerrainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTerrainDlg)

public:
	CTerrainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTerrainDlg();

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
	BOOL m_AlignToCenter;
	BOOL m_UseTerrainMaterial;
	BOOL m_UseWaterMaterial;
};

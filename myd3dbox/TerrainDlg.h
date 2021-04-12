#pragma once
#include "afxpropertygridctrl.h"
#include "Terrain.h"

// CTerrainDlg dialog

class CTerrainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTerrainDlg)

public:
	CTerrainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTerrainDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

	TerrainPtr m_terrain;

	std::string m_terrain_name;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	int m_RowChunks;
	int m_ColChunks;
	int m_ChunkSize;
	CString m_ChunkPath;
	BOOL m_AlignToCenter;
	virtual void OnOK();
};

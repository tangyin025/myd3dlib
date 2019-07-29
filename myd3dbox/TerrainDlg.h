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
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	int m_RowChunks;
	int m_ColChunks;
	int m_ChunkSize;
//	CString m_DiffuseTexture;
//	CString m_NormalTexture;
//	CString m_SpecularTexture;
//	afx_msg void OnBnClickedButton1();
//	afx_msg void OnBnClickedButton2();
//	afx_msg void OnBnClickedButton3();
	CMFCPropertyGridCtrl m_PropGridCtrl;
	Material m_Material;
};

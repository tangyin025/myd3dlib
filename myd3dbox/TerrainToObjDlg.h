#pragma once

class Terrain;

// CTerrainToObjDlg dialog

class CTerrainToObjDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTerrainToObjDlg)

public:
	CTerrainToObjDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CTerrainToObjDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG8 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	Terrain* m_terrain;
	CString m_path;
	int m_i0;
	int m_j0;
	int m_i1;
	int m_j1;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};

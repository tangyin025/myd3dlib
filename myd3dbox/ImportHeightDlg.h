#pragma once


// ImportHeightDlg dialog

class ImportHeightDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ImportHeightDlg)

public:
	ImportHeightDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~ImportHeightDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_AssetPath;
	CSize m_TextureSize;
	float m_MaxHeight;
	float m_WaterLevel;
};

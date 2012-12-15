
#include "stdafx.h"
#include "ImgRegionDocPropertyDlg.h"
#include "MainApp.h"

IMPLEMENT_DYNAMIC(CImgRegionDocPropertyDlg, CDialog)

CImgRegionDocPropertyDlg::CImgRegionDocPropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImgRegionDocPropertyDlg::IDD, pParent)
	, m_Size(500,500)
	, m_Color(RGB(255,255,255))
	, m_strBkImage(_T(""))
	, m_strFontFamily(_T("Arial"))
	, m_FontSize(12)
{
}

CImgRegionDocPropertyDlg::~CImgRegionDocPropertyDlg()
{
}

void CImgRegionDocPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Size.cx);
	DDX_Text(pDX, IDC_EDIT2, m_Size.cy);
	DDX_Control(pDX, IDC_BUTTON1, m_btnColor);
	DDX_Text(pDX, IDC_EDIT3, m_strBkImage);
	DDX_Control(pDX, IDC_COMBO1, m_cbxFontFamily);
	DDX_Text(pDX, IDC_EDIT4, m_FontSize);

	if(pDX->m_bSaveAndValidate)
	{
		m_Color = m_btnColor.GetColor();
		m_cbxFontFamily.GetLBText(m_cbxFontFamily.GetCurSel(), m_strFontFamily);
	}
	else
	{
		m_btnColor.SetColor(m_Color);
	}
}

BEGIN_MESSAGE_MAP(CImgRegionDocPropertyDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON2, &CImgRegionDocPropertyDlg::OnBnClickedOpenImage)
END_MESSAGE_MAP()

void CImgRegionDocPropertyDlg::OnBnClickedOpenImage()
{
}

BOOL CImgRegionDocPropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for(int i = 0; i < theApp.fontFamilies.GetSize(); i++)
	{
		CString strFamily;
		theApp.fontFamilies[i].GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE));
		m_cbxFontFamily.AddString(strFamily);
	}

	int nSel = m_cbxFontFamily.SelectString(-1, m_strFontFamily);
	ASSERT(nSel != CB_ERR);

	return TRUE;
}

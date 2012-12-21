
#include "stdafx.h"
#include "ImgRegionFilePropertyDlg.h"
#include "MainApp.h"

IMPLEMENT_DYNAMIC(CImgRegionFilePropertyDlg, CDialog)

CImgRegionFilePropertyDlg::CImgRegionFilePropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImgRegionFilePropertyDlg::IDD, pParent)
	, m_Size(500,500)
	, m_Color(RGB(255,255,255))
	, m_ImageStr(_T(""))
	, m_strFontFamily(_T("微软雅黑"))
	, m_FontSize(16)
{
}

CImgRegionFilePropertyDlg::~CImgRegionFilePropertyDlg()
{
}

void CImgRegionFilePropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Size.cx);
	DDX_Text(pDX, IDC_EDIT2, m_Size.cy);
	DDX_Control(pDX, IDC_BUTTON1, m_btnColor);
	DDX_Text(pDX, IDC_EDIT3, m_ImageStr);
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

BEGIN_MESSAGE_MAP(CImgRegionFilePropertyDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON2, &CImgRegionFilePropertyDlg::OnBnClickedOpenImage)
END_MESSAGE_MAP()

void CImgRegionFilePropertyDlg::OnBnClickedOpenImage()
{
	CFileDialog dlg(TRUE, NULL, m_ImageStr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("图片文件(*.bmp; *.jpg; *.png; *.tga)|*.bmp;*.jpg;*.png;*.tga|All Files(*.*)|*.*||"), this);
	if(dlg.DoModal() == IDOK)
	{
		m_ImageStr = dlg.GetPathName();
		SetDlgItemText(IDC_EDIT3, m_ImageStr);
	}
}

BOOL CImgRegionFilePropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnColor.EnableAutomaticButton(_T("默认"), RGB(255,255,255));
	m_btnColor.EnableOtherButton(_T("其它"));

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

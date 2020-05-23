
#include "stdafx.h"
#include "ImgRegionFilePropertyDlg.h"
#include "MainApp.h"

IMPLEMENT_DYNAMIC(CImgRegionFilePropertyDlg, CDialog)

CImgRegionFilePropertyDlg::CImgRegionFilePropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImgRegionFilePropertyDlg::IDD, pParent)
	, m_Size(800,600)
	, m_Color(RGB(255,255,255))
	, m_ImageStr(_T(""))
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

	if(pDX->m_bSaveAndValidate)
	{
		m_Color = m_btnColor.GetColor();
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

		m_Image = theApp.GetImage(m_ImageStr);
		if(m_Image)
		{
			SetDlgItemInt(IDC_EDIT1, m_Image->GetWidth());
			SetDlgItemInt(IDC_EDIT2, m_Image->GetHeight());
		}
	}
}

BOOL CImgRegionFilePropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnColor.EnableOtherButton(_T("其它"));

	return TRUE;
}

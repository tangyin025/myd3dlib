// ExportLuaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExportLuaDlg.h"
#include "MainFrm.h"


// CExportLuaDlg dialog

IMPLEMENT_DYNAMIC(CExportLuaDlg, CDialog)

CExportLuaDlg::CExportLuaDlg(CImgRegionDoc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CExportLuaDlg::IDD, pParent)
	, m_pDoc(pDoc)
	, m_strProjectDir(_T(""))
	, m_strLuaPath(_T(""))
	, m_dirtyFlag(FALSE)
{
}

CExportLuaDlg::~CExportLuaDlg()
{
}

void CExportLuaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strProjectDir);
	DDX_Text(pDX, IDC_EDIT2, m_strLuaPath);
}


BEGIN_MESSAGE_MAP(CExportLuaDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CExportLuaDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CExportLuaDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

void CExportLuaDlg::OnBnClickedButton1()
{
	TCHAR szDir[MAX_PATH];
	GetDlgItem(IDC_EDIT1)->GetWindowText(szDir, MAX_PATH);
	BROWSEINFO bi;
	ITEMIDLIST *pidl;
	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szDir;
	bi.lpszTitle = _T("请选择目录");
	bi.ulFlags = BIF_STATUSTEXT | BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	pidl = SHBrowseForFolder(&bi);
	if(pidl == NULL)
		return;
	if(!SHGetPathFromIDList(pidl, szDir))
		return;

	GetDlgItem(IDC_EDIT1)->SetWindowText(szDir);
}

void CExportLuaDlg::OnBnClickedButton2()
{
	TCHAR szFile[MAX_PATH];
	GetDlgItem(IDC_EDIT2)->GetWindowText(szFile, MAX_PATH);

	CFileDialog dlgFile(FALSE, _T("lua"), szFile);
	if(IDOK == dlgFile.DoModal())
		GetDlgItem(IDC_EDIT2)->SetWindowText(dlgFile.GetPathName());
}

void CExportLuaDlg::ExportTreeNodeToLua(std::ofstream & ofs, HTREEITEM hItem)
{
	if(hItem)
	{
		CImgRegionPtr pReg = m_pDoc->GetItemNode(hItem);
		ASSERT(pReg);

		HTREEITEM hChildItem = m_pDoc->m_TreeCtrl.GetChildItem(hItem);
		for (; hChildItem; hChildItem = m_pDoc->m_TreeCtrl.GetNextSiblingItem(hChildItem))
		{
			ExportTreeNodeToLua(ofs, hChildItem);
		}
	}
}

void CExportLuaDlg::OnOK()
{
	ASSERT(m_pDoc);

	UpdateData();

	std::ofstream ofs(m_strLuaPath);
	if(ofs.bad())
	{
		MessageBox(_T("无法打开脚本文件"));
		return;
	}

	LPCTSTR szName, szExtent, szBuff = m_strLuaPath.GetBuffer();
	szName = PathFindFileName(szBuff);
	szExtent = PathFindExtension(szBuff);
	std::basic_string<TCHAR> strName(szName, szExtent - szName);

	ofs << "module(\"" << ts2ms(strName.c_str()) << "\", package.seeall)" << std::endl;
	ofs << std::endl;
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.GetRootItem();
	for (; hItem; hItem = m_pDoc->m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		ExportTreeNodeToLua(ofs, hItem);
	}
	ofs.close();

	MessageBox(_T("成功导出lua脚本文件"));

	m_dirtyFlag = TRUE;
}

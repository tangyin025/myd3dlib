// LuaExporterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LuaExporterDlg.h"
#include <libc.h>
#include "MainFrm.h"


// CLuaExporterDlg dialog

IMPLEMENT_DYNAMIC(CLuaExporterDlg, CDialog)

CLuaExporterDlg::CLuaExporterDlg(CImgRegionDoc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CLuaExporterDlg::IDD, pParent)
	, m_pDoc(pDoc)
	, m_strProjectDir(_T(""))
	, m_strLuaPath(_T(""))
{
}

CLuaExporterDlg::~CLuaExporterDlg()
{
}

void CLuaExporterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strProjectDir);
	DDX_Text(pDX, IDC_EDIT2, m_strLuaPath);
}


BEGIN_MESSAGE_MAP(CLuaExporterDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CLuaExporterDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CLuaExporterDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

void CLuaExporterDlg::OnBnClickedButton1()
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

void CLuaExporterDlg::OnBnClickedButton2()
{
}

void CLuaExporterDlg::ExportTreeNodeToLua(std::ofstream & ofs, HTREEITEM hItem, int indent)
{
	if(hItem)
	{
		CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);

		ofs << std::string(indent, '\t') << "Gui.Control \"" << ts2ms(m_pDoc->m_TreeCtrl.GetItemText(hItem)) << "\"" << std::endl;
		ofs << std::string(indent, '\t') << "{" << std::endl;

		pReg->ExportToLua(ofs, indent + 1);

		ExportTreeNodeToLua(ofs, m_pDoc->m_TreeCtrl.GetChildItem(hItem), indent + 1);

		ofs << std::string(indent, '\t') << "}" << std::endl;

		ExportTreeNodeToLua(ofs, m_pDoc->m_TreeCtrl.GetNextSiblingItem(hItem));
	}
}

void CLuaExporterDlg::OnOK()
{
	ASSERT(m_pDoc);

	UpdateData();

	if(!PathIsFileSpec(m_strLuaPath))
	{
		MessageBox(_T("脚本文件名无效"));
		return;
	}

	std::ofstream ofs(m_strLuaPath);
	if(ofs.bad())
	{
		MessageBox(_T("无法打开脚本文件"));
		return;
	}

	ofs << "local ui=Gui.Create()" << std::endl;
	ofs << "{" << std::endl;

	ExportTreeNodeToLua(ofs, m_pDoc->m_TreeCtrl.GetRootItem());

	ofs << "}" << std::endl;

	ofs.close();

	MessageBox(_T("成功导出lua脚本文件"));
}

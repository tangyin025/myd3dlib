// ExportLuaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExportLuaDlg.h"
#include "MainFrm.h"
#include <boost/algorithm/string.hpp>


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

		HTREEITEM hParentItem = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
		std::string var_scope = (hParentItem ? "local " : "");
		std::string var_name = ts2ms((LPCTSTR)m_pDoc->m_TreeCtrl.GetItemText(hItem));
		std::string var_class = (hParentItem ? ts2ms((LPCTSTR)pReg->m_Class) : "Dialog");
		ofs << var_scope << var_name << "=" << var_class << "()" << std::endl;
		ofs << var_name << ".Name=\"" << var_name << "\"" << std::endl;
		ofs << var_name << ".Location=Vector2(" << pReg->m_Location.x << "," << pReg->m_Location.y << ")" << std::endl;
		ofs << var_name << ".Size=Vector2(" << pReg->m_Size.cx << "," << pReg->m_Size.cy << ")" << std::endl;
		ofs << var_name << ".Text=\"" << ts2ms((LPCTSTR)pReg->m_Text) << "\"" << std::endl;
		ofs << var_name << ".Skin=";
		if (var_class == "Dialog")
		{
			ofs << "DialogSkin";
		}
		else if (var_class == "ProgressBar")
		{
			ofs << "ProgressBarSkin";
		}
		else if (var_class == "Button")
		{
			ofs << "ButtonSkin";
		}
		else if (var_class == "EditBox")
		{
			ofs << "EditBoxSkin";
		}
		else if (var_class == "ScrollBar")
		{
			ofs << "ScrollBarSkin";
		}
		else
		{
			ofs << "ControlSkin";
		}
		ofs << "()" << std::endl;
		ofs << var_name << ".Skin.Color=ARGB(" << (int)pReg->m_Color.GetAlpha() << "," << (int)pReg->m_Color.GetRed() << "," << (int)pReg->m_Color.GetGreen() << "," << (int)pReg->m_Color.GetBlue() << ")" << std::endl;
		ofs << var_name << ".Skin.Image=ControlImage()" << std::endl;
		std::basic_string<TCHAR> strRelatedPath(MAX_PATH, _T('\0'));
		PathRelativePathTo(&strRelatedPath[0], m_strProjectDir, FILE_ATTRIBUTE_DIRECTORY, pReg->m_ImageStr, FILE_ATTRIBUTE_DIRECTORY);
		boost::trim_if(strRelatedPath, boost::algorithm::is_any_of(_T(".\\")));
		boost::algorithm::replace_all(strRelatedPath, _T("\\"), ("/"));
		ofs << var_name << ".Skin.Image.Texture=game:LoadTexture(\"" << ts2ms(strRelatedPath.c_str()) << "\")" << std::endl;
		ofs << var_name << ".Skin.Image.Rect=Rectangle(" << pReg->m_Rect.left << "," << pReg->m_Rect.top << "," << pReg->m_Rect.right << "," << pReg->m_Rect.bottom << ")" << std::endl;
		ofs << var_name << ".Skin.Image.Border=Vector4(" << pReg->m_Border.x << "," << pReg->m_Border.y << "," << pReg->m_Border.z << "," << pReg->m_Border.w << ")" << std::endl;
		ofs << var_name << ".Skin.Font=game.Font" << std::endl;
		ofs << var_name << ".Skin.TextColor=ARGB(" << (int)pReg->m_FontColor.GetAlpha() << "," << (int)pReg->m_FontColor.GetRed() << "," << (int)pReg->m_FontColor.GetGreen() << "," << (int)pReg->m_FontColor.GetBlue() << ")" << std::endl;
		ofs << var_name << ".Skin.TextAlign=Font.";
		switch (pReg->m_TextAlign)
		{
		default:
			ofs << "AlignLeftTop";
			break;
		case TextAlignCenterTop:
			ofs << "AlignCenterTop";
			break;
		case TextAlignRightTop:
			ofs << "AlignRightTop";
			break;
		case TextAlignLeftMiddle:
			ofs << "AlignLeftMiddle";
			break;
		case TextAlignCenterMiddle:
			ofs << "AlignCenterMiddle";
			break;
		case TextAlignRightMiddle:
			ofs << "AlignRightMiddle";
			break;
		case TextAlignLeftBottom:
			ofs << "AlignLeftBottom";
			break;
		case TextAlignCenterBottom:
			ofs << "AlignCenterBottom";
			break;
		case TextAlignRightBottom:
			ofs << "AlignRightBottom";
			break;
		}
		ofs << std::endl;
		if (hParentItem)
		{
			std::string parent_var_name = ts2ms((LPCTSTR)m_pDoc->m_TreeCtrl.GetItemText(hParentItem));
			ofs << parent_var_name << ":InsertControl(" << var_name << ")" << std::endl;
		}
		ofs << std::endl;

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

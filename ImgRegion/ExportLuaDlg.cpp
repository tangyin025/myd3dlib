// ExportLuaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExportLuaDlg.h"
#include "MainFrm.h"
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

// CExportLuaDlg dialog

IMPLEMENT_DYNAMIC(CExportLuaDlg, CDialog)

CExportLuaDlg::CExportLuaDlg(CImgRegionDoc * pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CExportLuaDlg::IDD, pParent)
	, m_pDoc(pDoc)
	, m_strProjectDir(_T(""))
	, m_strLuaPath(_T(""))
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
	bi.lpszTitle = _T("��ѡ��Ŀ¼");
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

static size_t _hash_value(const std::string & Class, Gdiplus::Color Color, const CString & ImageStr, const CRect & Rect, const Vector4i & Border, FontPtr2 Font, Gdiplus::Color FontColor, DWORD TextAlign)
{
	// ! maybe hash conflict
	size_t seed = 0;
	boost::hash_combine(seed, Class);
	boost::hash_combine(seed, Color.GetValue());
	boost::hash_combine(seed, std::basic_string<TCHAR>((LPCTSTR)ImageStr));
	boost::hash_combine(seed, Rect.left);
	boost::hash_combine(seed, Rect.top);
	boost::hash_combine(seed, Rect.right);
	boost::hash_combine(seed, Rect.bottom);
	boost::hash_combine(seed, Border.x);
	boost::hash_combine(seed, Border.y);
	boost::hash_combine(seed, Border.z);
	boost::hash_combine(seed, Border.w);
	boost::hash_combine(seed, FontColor.GetValue());
	boost::hash_combine(seed, TextAlign);
	return seed;
}

void CExportLuaDlg::ExportTreeNodeSkin(std::ofstream & ofs, HTREEITEM hItem)
{
	ASSERT(hItem);
	CImgRegionPtr pReg = m_pDoc->GetItemNode(hItem);
	ASSERT(pReg);
	HTREEITEM hParentItem = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	std::string var_class = (hParentItem ? tstou8((LPCTSTR)pReg->m_Class) : "Dialog");
	size_t seed = _hash_value(var_class, pReg->m_Color, pReg->m_ImageStr, pReg->m_Rect, pReg->m_Border, pReg->m_Font, pReg->m_FontColor, pReg->m_TextAlign);
	RegSkinMap::const_iterator skin_iter = m_SkinMap.find(seed);
	if (skin_iter == m_SkinMap.end())
	{
		std::string skin_class;
		if (var_class == "Dialog")
		{
			skin_class = "DialogSkin";
		}
		else if (var_class == "ProgressBar")
		{
			skin_class = "ProgressBarSkin";
		}
		else if (var_class == "Button" || var_class == "CheckBox")
		{
			skin_class = "ButtonSkin";
		}
		else if (var_class == "EditBox" || var_class == "ImeEditBox")
		{
			skin_class = "EditBoxSkin";
		}
		else if (var_class == "ScrollBar")
		{
			skin_class = "ScrollBarSkin";
		}
		else if (var_class == "ComboBox")
		{
			skin_class = "ComboBoxSkin";
		}
		else
		{
			skin_class = "ControlSkin";
		}
		std::string skin_var_name = str_printf("skin_%u", seed);
		m_SkinMap.insert(std::make_pair(seed, skin_var_name));
		ofs << skin_var_name << "=" << skin_class << "()" << std::endl;
		ofs << skin_var_name << ".Color=ARGB(" << (int)pReg->m_Color.GetAlpha() << "," << (int)pReg->m_Color.GetRed() << "," << (int)pReg->m_Color.GetGreen() << "," << (int)pReg->m_Color.GetBlue() << ")" << std::endl;
		ofs << skin_var_name << ".Image=ControlImage()" << std::endl;
		std::basic_string<TCHAR> strRelatedPath(MAX_PATH, _T('\0'));
		PathRelativePathTo(&strRelatedPath[0], m_strProjectDir, FILE_ATTRIBUTE_DIRECTORY, pReg->m_ImageStr, FILE_ATTRIBUTE_DIRECTORY);
		boost::trim_if(strRelatedPath, boost::algorithm::is_any_of(_T(".\\")));
		boost::algorithm::replace_all(strRelatedPath, _T("\\"), ("/"));
		ofs << skin_var_name << ".Image.Texture=game:LoadTexture(\"" << tstou8(strRelatedPath.c_str()) << "\")" << std::endl;
		ofs << skin_var_name << ".Image.Rect=Rectangle(" << pReg->m_Rect.left << "," << pReg->m_Rect.top << "," << pReg->m_Rect.right << "," << pReg->m_Rect.bottom << ")" << std::endl;
		ofs << skin_var_name << ".Image.Border=Vector4(" << pReg->m_Border.x << "," << pReg->m_Border.y << "," << pReg->m_Border.z << "," << pReg->m_Border.w << ")" << std::endl;
		ofs << skin_var_name << ".Font=game.Font" << std::endl;
		ofs << skin_var_name << ".TextColor=ARGB(" << (int)pReg->m_FontColor.GetAlpha() << "," << (int)pReg->m_FontColor.GetRed() << "," << (int)pReg->m_FontColor.GetGreen() << "," << (int)pReg->m_FontColor.GetBlue() << ")" << std::endl;
		ofs << skin_var_name << ".TextAlign=Font.";
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
		ofs << std::endl;
	}

	HTREEITEM hChildItem = m_pDoc->m_TreeCtrl.GetChildItem(hItem);
	for (; hChildItem; hChildItem = m_pDoc->m_TreeCtrl.GetNextSiblingItem(hChildItem))
	{
		ExportTreeNodeSkin(ofs, hChildItem);
	}
}

void CExportLuaDlg::ExportTreeNode(std::ofstream & ofs, HTREEITEM hItem)
{
	ASSERT(hItem);
	CImgRegionPtr pReg = m_pDoc->GetItemNode(hItem);
	ASSERT(pReg);
	HTREEITEM hParentItem = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	std::string var_scope = (hParentItem ? "local " : "");
	std::string var_name = tstou8((LPCTSTR)m_pDoc->m_TreeCtrl.GetItemText(hItem));
	std::string var_class = (hParentItem ? tstou8((LPCTSTR)pReg->m_Class) : "Dialog");
	ofs << var_scope << var_name << "=" << var_class << "()" << std::endl;
	ofs << var_name << ".Name=\"" << var_name << "\"" << std::endl;
	ofs << var_name << ".Location=Vector2(" << pReg->m_Location.x << "," << pReg->m_Location.y << ")" << std::endl;
	ofs << var_name << ".Size=Vector2(" << pReg->m_Size.cx << "," << pReg->m_Size.cy << ")" << std::endl;
	ofs << var_name << ".Text=\"" << tstou8((LPCTSTR)pReg->m_Text) << "\"" << std::endl;

	size_t seed = _hash_value(var_class, pReg->m_Color, pReg->m_ImageStr, pReg->m_Rect, pReg->m_Border, pReg->m_Font, pReg->m_FontColor, pReg->m_TextAlign);
	RegSkinMap::const_iterator skin_iter = m_SkinMap.find(seed);
	if (skin_iter != m_SkinMap.end())
	{
		ofs << var_name << ".Skin=" << skin_iter->second << std::endl;
	}

	if (hParentItem)
	{
		std::string parent_var_name = tstou8((LPCTSTR)m_pDoc->m_TreeCtrl.GetItemText(hParentItem));
		ofs << parent_var_name << ":InsertControl(" << var_name << ")" << std::endl;
	}
	ofs << std::endl;

	HTREEITEM hChildItem = m_pDoc->m_TreeCtrl.GetChildItem(hItem);
	for (; hChildItem; hChildItem = m_pDoc->m_TreeCtrl.GetNextSiblingItem(hChildItem))
	{
		ExportTreeNode(ofs, hChildItem);
	}
}

void CExportLuaDlg::OnOK()
{
	ASSERT(m_pDoc);

	UpdateData();

	std::ofstream ofs(m_strLuaPath);
	if(ofs.bad())
	{
		MessageBox(_T("�޷��򿪽ű��ļ�"));
		return;
	}

	LPCTSTR szName, szExtent, szBuff = m_strLuaPath.GetBuffer();
	szName = PathFindFileName(szBuff);
	szExtent = PathFindExtension(szBuff);
	std::basic_string<TCHAR> strName(szName, szExtent - szName);

	ofs << "module(\"" << tstou8(strName.c_str()) << "\", package.seeall)" << std::endl;
	ofs << std::endl;
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.GetRootItem();
	for (; hItem; hItem = m_pDoc->m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		ExportTreeNodeSkin(ofs, hItem);
	}
	hItem = m_pDoc->m_TreeCtrl.GetRootItem();
	for (; hItem; hItem = m_pDoc->m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		ExportTreeNode(ofs, hItem);
	}
	ofs.close();

	MessageBox(_T("�ɹ�����lua�ű��ļ�"));

	EndDialog(IDOK);
}

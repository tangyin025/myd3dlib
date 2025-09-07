// Copyright (c) 2011-2024 tangyin025
// License: MIT

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

static size_t _hash_value(const std::string & Class, Gdiplus::Color Color, const CString & ImageStr, const Gdiplus::Rect & Rect, const Vector4i & Border, FontPtr2 Font, Gdiplus::Color FontColor, DWORD TextAlign)
{
	// ! maybe hash conflict
	size_t seed = 0;
	boost::hash_combine(seed, Class);
	boost::hash_combine(seed, Color.GetValue());
	boost::hash_combine(seed, std::basic_string<TCHAR>((LPCTSTR)ImageStr));
	boost::hash_combine(seed, Rect.X);
	boost::hash_combine(seed, Rect.Y);
	boost::hash_combine(seed, Rect.Width);
	boost::hash_combine(seed, Rect.Height);
	boost::hash_combine(seed, Border.x);
	boost::hash_combine(seed, Border.y);
	boost::hash_combine(seed, Border.z);
	boost::hash_combine(seed, Border.w);
	boost::hash_combine(seed, FontColor.GetValue());
	if (Font)
	{
		CString strFamily;
		Gdiplus::FontFamily family;
		Font->GetFamily(&family);
		family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE));
		strFamily.ReleaseBuffer();
		boost::hash_combine(seed, std::basic_string<TCHAR>((LPCTSTR)strFamily));
		boost::hash_combine(seed, Font->GetSize());
	}
	boost::hash_combine(seed, TextAlign);
	return seed;
}

void CExportLuaDlg::ExportTreeNodeSkin(std::ofstream & ofs, HTREEITEM hItem)
{
	ASSERT(hItem);
	CImgRegionPtr pReg = m_pDoc->GetItemNode(hItem);
	ASSERT(pReg);
	HTREEITEM hParentItem = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	const std::string var_class = (hParentItem ? tstou8((LPCTSTR)pReg->m_Class) : "Dialog");
	size_t seed = _hash_value(var_class, pReg->m_Color, pReg->m_ImageStr, pReg->m_ImageRect, pReg->m_ImageBorder, pReg->m_Font, pReg->m_FontColor, pReg->m_TextAlign);
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
		std::string skin_var_name = str_printf("editor_skin_%u", seed);
		m_SkinMap.insert(std::make_pair(seed, skin_var_name));
		ofs << skin_var_name << "=" << skin_class << "()" << std::endl;
		ofs << skin_var_name << ".Color=ARGB(" << (int)pReg->m_Color.GetAlpha() << "," << (int)pReg->m_Color.GetRed() << "," << (int)pReg->m_Color.GetGreen() << "," << (int)pReg->m_Color.GetBlue() << ")" << std::endl;
		if (!pReg->m_ImageStr.IsEmpty())
		{
			ofs << skin_var_name << ".Image=ControlImage()" << std::endl;
			std::basic_string<TCHAR> strRelatedPath(MAX_PATH, _T('\0'));
			PathRelativePathTo(&strRelatedPath[0], m_strProjectDir, FILE_ATTRIBUTE_DIRECTORY, pReg->m_ImageStr, FILE_ATTRIBUTE_DIRECTORY);
			boost::trim_if(strRelatedPath, boost::algorithm::is_any_of(_T(".\\")));
			boost::algorithm::replace_all(strRelatedPath, _T("\\"), _T("/"));
			ofs << skin_var_name << ".Image.Texture=game:LoadTexture(\"" << tstou8(strRelatedPath.c_str()) << "\")" << std::endl;
			ofs << skin_var_name << ".Image.Rect=Rectangle.LeftTop(" << pReg->m_ImageRect.X << "," << pReg->m_ImageRect.Y << "," << pReg->m_ImageRect.Width << "," << pReg->m_ImageRect.Height << ")" << std::endl;
			ofs << skin_var_name << ".Image.Border=Vector4(" << pReg->m_ImageBorder.x << "," << pReg->m_ImageBorder.y << "," << pReg->m_ImageBorder.z << "," << pReg->m_ImageBorder.w << ")" << std::endl;
		}
		ofs << skin_var_name << ".Font=";
		if (pReg->m_Font)
		{
			CString strFamily;
			Gdiplus::FontFamily family;
			pReg->m_Font->GetFamily(&family);
			family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE));
			strFamily.ReleaseBuffer();
			if (strFamily == _T("Noto Sans CJK SC Regular"))
			{
				ofs << "game:LoadFont(\"font/NotoSansCJK.ttc\", " << pReg->m_Font->GetSize() << ", 14)";
			}
			else
			{
				ofs << "game.Font";
			}
		}
		else
		{
			ofs << "game.Font";
		}
		ofs << std::endl;
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
		if (var_class == "Dialog")
		{
		}
		else if (var_class == "ProgressBar")
		{
			ofs << skin_var_name << ".ForegroundImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ForegroundImage.Texture=game:LoadTexture(\"texture / CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ForegroundImage.Rect=Rectangle.LeftTop(35,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ForegroundImage.Border=Vector4(7,7,7,7)" << std::endl;
		}
		else if (var_class == "Button")
		{
			ofs << skin_var_name << ".PressedOffset=Vector2(1,2)" << std::endl;
			ofs << skin_var_name << ".DisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Rect=Rectangle.LeftTop(1,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".PressedImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".PressedImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".PressedImage.Rect=Rectangle.LeftTop(18,43,16,16)" << std::endl;
			ofs << skin_var_name << ".PressedImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".MouseOverImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Rect=Rectangle.LeftTop(35,43,16,16)" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Border=Vector4(7,7,7,7)" << std::endl;
		}
		else if (var_class == "CheckBox")
		{
			ofs << skin_var_name << ".PressedOffset=Vector2(0,0)" << std::endl;
			ofs << skin_var_name << ".DisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Rect=Rectangle.LeftTop(69,43,20,20)" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Border=Vector4(0,0,0,0)" << std::endl;
			ofs << skin_var_name << ".PressedImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".PressedImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".PressedImage.Rect=Rectangle.LeftTop(90,43,20,20)" << std::endl;
			ofs << skin_var_name << ".PressedImage.Border=Vector4(0,0,0,0)" << std::endl;
			ofs << skin_var_name << ".MouseOverImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Rect=Rectangle.LeftTop(111,43,20,20)" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Border=Vector4(0,0,0,0)" << std::endl;
		}
		else if (var_class == "EditBox" || var_class == "ImeEditBox")
		{
			ofs << skin_var_name << ".DisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Rect=Rectangle.LeftTop(1,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".FocusedImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".FocusedImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".FocusedImage.Rect=Rectangle.LeftTop(18,43,16,16)" << std::endl;
			ofs << skin_var_name << ".FocusedImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".SelBkColor=ARGB(255,255,128,0)" << std::endl;
			ofs << skin_var_name << ".CaretColor=ARGB(255,255,255,255)" << std::endl;
			ofs << skin_var_name << ".CaretImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".CaretImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".CaretImage.Rect=Rectangle.LeftTop(154,43,2,2)" << std::endl;
			ofs << skin_var_name << ".CaretImage.Border=Vector4(7,7,7,7)" << std::endl;
		}
		else if (var_class == "ScrollBar")
		{
			ofs << skin_var_name << ".UpBtnNormalImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".UpBtnNormalImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".UpBtnNormalImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".UpBtnNormalImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".UpBtnDisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".UpBtnDisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".UpBtnDisabledImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".UpBtnDisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".DownBtnNormalImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DownBtnNormalImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DownBtnNormalImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DownBtnNormalImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".DownBtnDisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DownBtnDisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DownBtnDisabledImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DownBtnDisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ThumbBtnNormalImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ThumbBtnNormalImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ThumbBtnNormalImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ThumbBtnNormalImage.Border=Vector4(7,7,7,7)" << std::endl;
		}
		else if (var_class == "ComboBox")
		{
			ofs << skin_var_name << ".PressedOffset=Vector2(1, 2)" << std::endl;
			ofs << skin_var_name << ".DisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Rect=Rectangle.LeftTop(1,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".PressedImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".PressedImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".PressedImage.Rect=Rectangle.LeftTop(18,43,16,16)" << std::endl;
			ofs << skin_var_name << ".PressedImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".MouseOverImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Rect=Rectangle.LeftTop(35,43,16,16)" << std::endl;
			ofs << skin_var_name << ".MouseOverImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".DropdownImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DropdownImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DropdownImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DropdownImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".DropdownItemTextColor=ARGB(255,255,255,255)" << std::endl;
			ofs << skin_var_name << ".DropdownItemTextAlign=Font.AlignCenterMiddle" << std::endl;
			ofs << skin_var_name << ".DropdownItemMouseOverImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".DropdownItemMouseOverImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".DropdownItemMouseOverImage.Rect=Rectangle.LeftTop(35,43,16,16)" << std::endl;
			ofs << skin_var_name << ".DropdownItemMouseOverImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnNormalImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnNormalImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnNormalImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnNormalImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnDisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnDisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnDisabledImage.Rect=Rectangle.LeftTop(1,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ScrollBarUpBtnDisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnNormalImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnNormalImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnNormalImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnNormalImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnDisabledImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnDisabledImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnDisabledImage.Rect=Rectangle.LeftTop(1,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ScrollBarDownBtnDisabledImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ScrollBarThumbBtnNormalImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ScrollBarThumbBtnNormalImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ScrollBarThumbBtnNormalImage.Rect=Rectangle.LeftTop(52,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ScrollBarThumbBtnNormalImage.Border=Vector4(7,7,7,7)" << std::endl;
			ofs << skin_var_name << ".ScrollBarImage=ControlImage()" << std::endl;
			ofs << skin_var_name << ".ScrollBarImage.Texture=game:LoadTexture(\"texture/CommonUI.png\")" << std::endl;
			ofs << skin_var_name << ".ScrollBarImage.Rect=Rectangle.LeftTop(1,43,16,16)" << std::endl;
			ofs << skin_var_name << ".ScrollBarImage.Border=Vector4(7,7,7,7)" << std::endl;
		}
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
	ofs << var_scope << var_name << "=" << var_class << "(\"" << var_name << "\")" << std::endl;
	ofs << var_name << ".x=UDim(" << pReg->m_x.scale << "," << pReg->m_x.offset << ")" << std::endl;
	ofs << var_name << ".y=UDim(" << pReg->m_y.scale << "," << pReg->m_y.offset << ")" << std::endl;
	ofs << var_name << ".Width=UDim(" << pReg->m_Width.scale << "," << pReg->m_Width.offset << ")" << std::endl;
	ofs << var_name << ".Height=UDim(" << pReg->m_Height.scale << "," << pReg->m_Height.offset << ")" << std::endl;
	CString strInfo;
	strInfo.Format(pReg->m_Text, pReg->m_Rect.X, pReg->m_Rect.Y, pReg->m_Rect.Width, pReg->m_Rect.Height);
	ofs << var_name << ".Text=" << "\"" << tstou8((LPCTSTR)strInfo) << "\"";
	ofs << std::endl;

	size_t seed = _hash_value(var_class, pReg->m_Color, pReg->m_ImageStr, pReg->m_ImageRect, pReg->m_ImageBorder, pReg->m_Font, pReg->m_FontColor, pReg->m_TextAlign);
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
		MessageBox(_T("无法打开脚本文件"));
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

	MessageBox(_T("成功导出lua脚本文件"));

	EndDialog(IDOK);
}

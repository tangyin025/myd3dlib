#include "stdafx.h"
#include "ImgRegionDoc.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "resource.h"
#include "ImgRegionFilePropertyDlg.h"
#include "ImgRegionView.h"
#include "LuaExporterDlg.h"
#include "ImgRegionDocFileVersions.h"

//#pragma comment(lib, "UxTheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const TCHAR * TextAlignDesc[TextAlignCount] = {
	_T("LeftTop"),
	_T("CenterTop"),
	_T("RightTop"),
	_T("LeftMiddle"),
	_T("CenterMiddle"),
	_T("RightMiddle"),
	_T("LeftBottom"),
	_T("CenterBottom"),
	_T("RightBottom"),
};

void CImgRegion::CreateProperties(CPropertiesWnd * pPropertiesWnd)
{
	CMFCPropertyGridProperty * pGroup = new CSimpleProp(_T("���"));

	CMFCPropertyGridProperty * pProp = new CCheckBoxProp(_T("��ס"), m_Locked, _T("��ס�ƶ�����"), CPropertiesWnd::PropertyItemLocked);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocked] = pProp);

	CMFCPropertyGridProperty * pLocal = new CSimpleProp(_T("Local"), CPropertiesWnd::PropertyItemLocation, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)m_Location.x, _T("x����"), CPropertiesWnd::PropertyItemLocationX);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocationX] = pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_Location.y, _T("y����"), CPropertiesWnd::PropertyItemLocationY);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocationY] = pProp);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocation] = pLocal);

	pLocal = new CSimpleProp(_T("Size"), CPropertiesWnd::PropertyItemSize, TRUE);
	pProp = new CSimpleProp(_T("w"), (_variant_t)m_Size.cx, _T("���"), CPropertiesWnd::PropertyItemSizeW);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemSizeW] = pProp);
	pProp = new CSimpleProp(_T("h"), (_variant_t)m_Size.cy, _T("�߶�"), CPropertiesWnd::PropertyItemSizeH);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemSizeH] = pProp);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemSize] = pLocal);

	pPropertiesWnd->m_wndPropList.AddProperty(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyGroupCoord] = pGroup);

	pGroup = new CSimpleProp(_T("��ɫ"));

	pProp = new CSliderProp(_T("Alpha"), (_variant_t)(long)m_Color.GetAlpha(), _T("͸��ֵ"), CPropertiesWnd::PropertyItemAlpha);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemAlpha] = pProp);

	CColorProp * pColorProp = new CColorProp(_T("��ɫ"), m_Color.ToCOLORREF(), NULL, _T("��ɫ"), CPropertiesWnd::PropertyItemRGB);
	pColorProp->EnableOtherButton(_T("����..."));
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemRGB] = pColorProp);

	CMFCPropertyGridFileProperty * pFileProp = new CFileProp(_T("ͼƬ"), TRUE, m_ImageStr, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("ͼƬ�ļ�(*.bmp; *.jpg; *.png; *.tga)|*.bmp;*.jpg;*.png;*.tga|All Files(*.*)|*.*||"), _T("ͼƬ�ļ�"), CPropertiesWnd::PropertyItemImage);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemImage] = pFileProp);

	pLocal = new CSimpleProp(_T("Border"), CPropertiesWnd::PropertyItemBorder, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)m_Border.x, _T("��߾�"), CPropertiesWnd::PropertyItemBorderX);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderX] = pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_Border.y, _T("�ϱ߾�"), CPropertiesWnd::PropertyItemBorderY);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderY] = pProp);
	pProp = new CSimpleProp(_T("z"), (_variant_t)m_Border.z, _T("�ұ߾�"), CPropertiesWnd::PropertyItemBorderZ);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderZ] = pProp);
	pProp = new CSimpleProp(_T("w"), (_variant_t)m_Border.w, _T("�±߾�"), CPropertiesWnd::PropertyItemBorderW);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderW] = pProp);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorder] = pLocal);

	pPropertiesWnd->m_wndPropList.AddProperty(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyGroupImage] = pGroup);

	pGroup = new CSimpleProp(_T("����"));

	CString strFamily;
	if(m_Font)
	{
		Gdiplus::FontFamily family; m_Font->GetFamily(&family); family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer();
	}
	pProp = new CSimpleProp(_T("����"), strFamily, _T("ѡ������"), CPropertiesWnd::PropertyItemFont);
	pProp->AllowEdit(FALSE);
	for(int i = 0; i < theApp.fontFamilies.GetSize(); i++)
	{
		CString strFamily;
		theApp.fontFamilies[i].GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE));
		pProp->AddOption(strFamily, FALSE);
	}
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFont] = pProp);

	pProp = new CSimpleProp(_T("�ֺ�"), (_variant_t)m_Font ? (long)m_Font->GetSize() : 16, _T("�����С"), CPropertiesWnd::PropertyItemFontSize);
	pProp->AddOption(_T("6"));
	pProp->AddOption(_T("8"));
	pProp->AddOption(_T("9"));
	pProp->AddOption(_T("10"));
	pProp->AddOption(_T("11"));
	pProp->AddOption(_T("12"));
	pProp->AddOption(_T("14"));
	pProp->AddOption(_T("16"));
	pProp->AddOption(_T("18"));
	pProp->AddOption(_T("24"));
	pProp->AddOption(_T("36"));
	pProp->AddOption(_T("48"));
	pProp->AddOption(_T("60"));
	pProp->AddOption(_T("72"));
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFontSize] = pProp);
	pProp = new CSliderProp(_T("Alpha"), (_variant_t)(long)m_FontColor.GetAlpha(), _T("͸��ֵ"), CPropertiesWnd::PropertyItemFontAlpha);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFontAlpha] = pProp);
	pColorProp = new CColorProp(_T("��ɫ"), m_FontColor.ToCOLORREF(), NULL, _T("��ɫ"), CPropertiesWnd::PropertyItemFontRGB);
	pColorProp->EnableOtherButton(_T("����..."));
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFontRGB] = pColorProp);

	pPropertiesWnd->m_wndPropList.AddProperty(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyGroupFont] = pGroup);

	pGroup = new CSimpleProp(_T("�ı�"));

	pProp = new CSimpleProp(_T("�ı�"), m_Text, _T("���ο��ڵ���������"), CPropertiesWnd::PropertyItemText);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemText] = pProp);

	pProp = new CComboProp(_T("����"), TextAlignDesc[m_TextAlign], _T("�ı��ھ��ο��ڵĶ��䷽ʽ"), CPropertiesWnd::PropertyItemTextAlign);
	pProp->AllowEdit(FALSE);
	for(int i = 0; i < TextAlignCount; i++)
	{
		pProp->AddOption(TextAlignDesc[i], FALSE);
	}
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextAlign] = pProp);

	pProp = new CCheckBoxProp(_T("�Զ�����"), (long)m_TextWrap, _T("�ı��ھ��ο����Զ�����"), CPropertiesWnd::PropertyItemTextWrap);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextWrap] = pProp);

	pLocal = new CSimpleProp(_T("ƫ��"), CPropertiesWnd::PropertyItemTextOff, TRUE);
	pProp = new CSimpleProp(_T("x"), (_variant_t)m_TextOff.x, _T("x����"), CPropertiesWnd::PropertyItemTextOffX);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextOffX] = pProp);
	pProp = new CSimpleProp(_T("y"), (_variant_t)m_TextOff.y, _T("y����"), CPropertiesWnd::PropertyItemTextOffY);
	pLocal->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextOffY] = pProp);
	pGroup->AddSubItem(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextOff] = pLocal);

	pPropertiesWnd->m_wndPropList.AddProperty(pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyGroupText] = pGroup);
}

void CImgRegion::UpdateProperties(CPropertiesWnd * pPropertiesWnd)
{
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocked]->SetValue((_variant_t)(long)m_Locked);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocationX]->SetValue((_variant_t)m_Location.x);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemLocationY]->SetValue((_variant_t)m_Location.y);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemSizeW]->SetValue((_variant_t)m_Size.cx);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemSizeH]->SetValue((_variant_t)m_Size.cy);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemAlpha]->SetValue((_variant_t)(long)m_Color.GetAlpha());
	((CColorProp *)pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemRGB])->SetColor(m_Color.ToCOLORREF());

	((CFileProp *)pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemImage])->SetValue((_variant_t)m_ImageStr);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderX]->SetValue((_variant_t)m_Border.x);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderY]->SetValue((_variant_t)m_Border.y);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderZ]->SetValue((_variant_t)m_Border.z);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemBorderW]->SetValue((_variant_t)m_Border.w);

	CString strFamily;
	if(m_Font)
	{
		Gdiplus::FontFamily family; m_Font->GetFamily(&family); family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE)); strFamily.ReleaseBuffer();
	}
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFont]->SetValue((_variant_t)strFamily);
	long fntSize = 16;
	if(m_Font)
	{
		fntSize = (long)m_Font->GetSize();
	}
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFontSize]->SetValue(fntSize);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFontAlpha]->SetValue((_variant_t)(long)m_FontColor.GetAlpha());
	((CColorProp *)pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemFontRGB])->SetColor(m_FontColor.ToCOLORREF());

	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemText]->SetValue((_variant_t)m_Text);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextAlign]->SetValue((_variant_t)TextAlignDesc[m_TextAlign]);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextWrap]->SetValue((_variant_t)(long)m_TextWrap);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextOffX]->SetValue((_variant_t)m_TextOff.x);
	pPropertiesWnd->m_pProp[CPropertiesWnd::PropertyItemTextOffY]->SetValue((_variant_t)m_TextOff.y);
}

void CImgRegion::Draw(Gdiplus::Graphics & grap)
{
	if(m_Image && Gdiplus::ImageTypeUnknown != m_Image->GetType())
	{
		CImgRegionView::DrawRegionDocImage(grap, m_Image.get(), CRect(m_Location, m_Size), m_Border, m_Color);
	}
	else
	{
		Gdiplus::SolidBrush brush(m_Color);
		grap.FillRectangle(&brush, m_Location.x, m_Location.y, m_Size.cx, m_Size.cy);
	}

	if(m_Font)
	{
		CString strInfo;
		strInfo.Format(m_Text, m_Location.x, m_Location.y, m_Size.cx, m_Size.cy);

		Gdiplus::RectF rectF(
			(float)m_Location.x + m_TextOff.x, (float)m_Location.y + m_TextOff.y, (float)m_Size.cx, (float)m_Size.cy);

		Gdiplus::StringFormat format((m_TextWrap ? 0 : Gdiplus::StringFormatFlagsNoWrap) | Gdiplus::StringFormatFlagsNoClip);
		format.SetTrimming(Gdiplus::StringTrimmingNone);
		switch(m_TextAlign)
		{
		case TextAlignLeftTop:
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			format.SetLineAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TextAlignCenterTop:
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			format.SetLineAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TextAlignRightTop:
			format.SetAlignment(Gdiplus::StringAlignmentFar);
			format.SetLineAlignment(Gdiplus::StringAlignmentNear);
			break;
		case TextAlignLeftMiddle:
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TextAlignCenterMiddle:
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TextAlignRightMiddle:
			format.SetAlignment(Gdiplus::StringAlignmentFar);
			format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			break;
		case TextAlignLeftBottom:
			format.SetAlignment(Gdiplus::StringAlignmentNear);
			format.SetLineAlignment(Gdiplus::StringAlignmentFar);
			break;
		case TextAlignCenterBottom:
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
			format.SetLineAlignment(Gdiplus::StringAlignmentFar);
			break;
		case TextAlignRightBottom:
			format.SetAlignment(Gdiplus::StringAlignmentFar);
			format.SetLineAlignment(Gdiplus::StringAlignmentFar);
			break;
		}

		Gdiplus::SolidBrush brush(m_FontColor);
		grap.DrawString(strInfo, strInfo.GetLength(), m_Font.get(), rectF, &format, &brush);

		//Gdiplus::GraphicsPath path;
		//Gdiplus::FontFamily family;
		//m_Font->GetFamily(&family);
		//path.AddString(strInfo, strInfo.GetLength(), &family, m_Font->GetStyle(), m_Font->GetSize(), rectF, &format);
		//Gdiplus::Pen pen(m_FontColor, 2.0f);
		//Gdiplus::SolidBrush brush(m_Color);
		//Gdiplus::SmoothingMode sm = grap.GetSmoothingMode();
		//grap.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		//grap.DrawPath(&pen, &path);
		//grap.FillPath(&brush, &path);
		//grap.SetSmoothingMode(sm);
	}
}

void CImgRegion::Serialize(CArchive& ar, int version)
{
	CImgRegionDocFileVersions::SerializeImgRegion(this, ar, version);
}

void CImgRegion::ExportToLua(std::ofstream & ofs, int indent)
{
	ofs << std::string(indent, '\t') << str_printf("Location=Vector2(%d,%d),", m_Location.x, m_Location.y) << std::endl;
	ofs << std::string(indent, '\t') << str_printf("Size=Vector2(%d,%d),", m_Size.cx, m_Size.cy) << std::endl;
	ofs << std::string(indent, '\t') << str_printf("BackgroundColor=ARGB(%d,%d,%d,%d),", m_Color.GetA(), m_Color.GetR(), m_Color.GetG(), m_Color.GetB()) << std::endl;
	ofs << std::string(indent, '\t') << str_printf("Text=\"%s\",", ts2ms(m_Text).c_str()) << std::endl;
}

void HistoryChangeItemLocation::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Location = m_newValue;
}

void HistoryChangeItemLocation::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Location = m_oldValue;
}

void HistoryChangeItemSize::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Size = m_newValue;
}

void HistoryChangeItemSize::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_Size = m_oldValue;
}

void HistoryChangeItemTextOff::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_TextOff = m_newValue;
}

void HistoryChangeItemTextOff::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	pReg->m_TextOff = m_oldValue;
}

HistoryAddRegion::HistoryAddRegion(CImgRegionDoc * pDoc, LPCTSTR itemID, LPCTSTR parentID, LPCTSTR beforeID)
	: m_pDoc(pDoc)
	, m_itemID(itemID)
	, m_parentID(parentID)
	, m_beforeID(beforeID)
	, m_OverideRegId(0)
{
}

void HistoryAddRegion::Do(void)
{
	HTREEITEM hParent = m_parentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_parentID];
	HTREEITEM hBefore = m_beforeID.empty() ? TVI_LAST : m_pDoc->m_TreeCtrl.m_ItemMap[m_beforeID];
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.InsertItem(m_itemID.c_str(), hParent, hBefore);

	CImgRegion * pReg = new CImgRegion;
	ASSERT(pReg);

	m_pDoc->m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	// ! ʹ�� OverideRegId����ֹ�����������ֿ�����undo֮����������
	if(0 == m_OverideRegId)
	{
		m_OverideRegId = m_pDoc->m_NextRegId;
	}

	DWORD oldRegId = m_pDoc->m_NextRegId;
	{
		m_pDoc->m_NextRegId = m_OverideRegId;
		ASSERT(m_NodeCache.GetLength() > 0);
		m_NodeCache.SeekToBegin();
		CArchive ar(&m_NodeCache, CArchive::load);
		pReg->Serialize(ar, CImgRegionDocFileVersions::FILE_VERSION);
		m_pDoc->SerializeSubTreeNode(ar, CImgRegionDocFileVersions::FILE_VERSION, hItem, TRUE);
		ar.Close();
	}
	m_pDoc->m_NextRegId = max(oldRegId, m_pDoc->m_NextRegId);

	pReg->m_Locked = FALSE;

	m_pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);

	m_pDoc->m_TreeCtrl.SelectItem(hItem);
}

void HistoryAddRegion::Undo(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	m_pDoc->m_TreeCtrl.DeleteTreeItem<CImgRegion>(hItem, TRUE);
}

void HistoryDelRegion::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	m_NodeCache.SetLength(0);
	CArchive ar(&m_NodeCache, CArchive::store);
	pReg->Serialize(ar, CImgRegionDocFileVersions::FILE_VERSION);
	m_pDoc->SerializeSubTreeNode(ar, CImgRegionDocFileVersions::FILE_VERSION, hItem);
	ar.Close();

	HTREEITEM hParent = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	m_parentID = hParent ? m_pDoc->m_TreeCtrl.GetItemText(hParent) : _T("");

	HTREEITEM hBefore = m_pDoc->m_TreeCtrl.GetPrevSiblingItem(hItem);
	m_beforeID = hBefore ? m_pDoc->m_TreeCtrl.GetItemText(hBefore) : _T("");

	m_pDoc->m_TreeCtrl.DeleteTreeItem<CImgRegion>(hItem, TRUE);
}

void HistoryDelRegion::Undo(void)
{
	HTREEITEM hParent = m_parentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_parentID];
	HTREEITEM hBefore = m_beforeID.empty() ? TVI_LAST : m_pDoc->m_TreeCtrl.m_ItemMap[m_beforeID];
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.InsertItem(m_itemID.c_str(), hParent, hBefore);

	CImgRegion * pReg = new CImgRegion;
	ASSERT(pReg);

	m_pDoc->m_TreeCtrl.SetItemData(hItem, (DWORD_PTR)pReg);

	ASSERT(m_NodeCache.GetLength() > 0);
	m_NodeCache.SeekToBegin();
	CArchive ar(&m_NodeCache, CArchive::load);
	pReg->Serialize(ar, CImgRegionDocFileVersions::FILE_VERSION);
	m_pDoc->SerializeSubTreeNode(ar, CImgRegionDocFileVersions::FILE_VERSION, hItem);
	ar.Close();

	if(pReg->m_Locked)
		m_pDoc->m_TreeCtrl.SetItemImage(hItem, 1, 1);
	m_pDoc->m_TreeCtrl.Expand(hItem, TVE_EXPAND);
}

void HistoryMovRegion::Do(void)
{
	ASSERT(m_pDoc->m_TreeCtrl.m_ItemMap.find(m_itemID) != m_pDoc->m_TreeCtrl.m_ItemMap.end());

	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];
	HTREEITEM hParent = m_pDoc->m_TreeCtrl.GetParentItem(hItem);
	HTREEITEM hBefore = m_pDoc->m_TreeCtrl.GetPrevSiblingItem(hItem);

	m_oldParentID = hParent ? m_pDoc->m_TreeCtrl.GetItemText(hParent) : _T("");
	m_oldBeforeID = hBefore ? m_pDoc->m_TreeCtrl.GetItemText(hBefore) : _T("");

	CPoint ptOrg = m_pDoc->LocalToRoot(hItem, CPoint(0,0));

	HTREEITEM hNewParent = m_newParentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_newParentID];
	HTREEITEM hNewBefore = m_newBeforeID.empty() ? TVI_LAST : m_pDoc->m_TreeCtrl.m_ItemMap[m_newBeforeID];

	HTREEITEM hNewItem = m_pDoc->m_TreeCtrl.MoveTreeItem(hNewParent, hNewBefore, hItem);
	if(hNewItem != hItem)
	{
		m_pDoc->m_TreeCtrl.SelectItem(hNewItem);

		CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hNewItem);
		ASSERT(pReg);

		pReg->m_Location = m_pDoc->RootToLocal(hNewParent, ptOrg);
	}
}

void HistoryMovRegion::Undo(void)
{
	HTREEITEM hItem = m_pDoc->m_TreeCtrl.m_ItemMap[m_itemID];

	CPoint ptOrg = m_pDoc->LocalToRoot(hItem, CPoint(0,0));

	HTREEITEM hOldParent = m_oldParentID.empty() ? TVI_ROOT : m_pDoc->m_TreeCtrl.m_ItemMap[m_oldParentID];
	HTREEITEM hOldBefore = m_oldBeforeID.empty() ? TVI_FIRST : m_pDoc->m_TreeCtrl.m_ItemMap[m_oldBeforeID];

	HTREEITEM hOldItem = m_pDoc->m_TreeCtrl.MoveTreeItem(hOldParent, hOldBefore, hItem);
	if(hOldItem != hItem)
	{
		m_pDoc->m_TreeCtrl.SelectItem(hOldItem);

		CImgRegion * pReg = (CImgRegion *)m_pDoc->m_TreeCtrl.GetItemData(hOldItem);
		ASSERT(pReg);

		pReg->m_Location = m_pDoc->RootToLocal(hOldParent, ptOrg);
	}
}

void HistoryModifyRegion::Do(void)
{
	iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		(*hist_iter)->Do();
	}
}

void HistoryModifyRegion::Undo(void)
{
	iterator hist_iter = begin();
	for(; hist_iter != end(); hist_iter++)
	{
		(*hist_iter)->Undo();
	}
}

IMPLEMENT_DYNCREATE(CImgRegionDoc, CDocument)

BEGIN_MESSAGE_MAP(CImgRegionDoc, CDocument)
	ON_COMMAND(ID_ADD_REGION, &CImgRegionDoc::OnAddRegion)
	ON_UPDATE_COMMAND_UI(ID_ADD_REGION, &CImgRegionDoc::OnUpdateAddRegion)
	ON_COMMAND(ID_DEL_REGION, &CImgRegionDoc::OnDelRegion)
	ON_UPDATE_COMMAND_UI(ID_DEL_REGION, &CImgRegionDoc::OnUpdateDelRegion)
	ON_COMMAND(ID_EXPORT_IMG, &CImgRegionDoc::OnExportImg)
	ON_COMMAND(ID_FILE_PROPERTY, &CImgRegionDoc::OnFileProperty)
	ON_COMMAND(ID_EDIT_COPY, &CImgRegionDoc::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CImgRegionDoc::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CImgRegionDoc::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CImgRegionDoc::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, &CImgRegionDoc::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CImgRegionDoc::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CImgRegionDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CImgRegionDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_EXPORT_LUA, &CImgRegionDoc::OnExportLua)
END_MESSAGE_MAP()

CImgRegionDoc::CImgRegionDoc(void)
	: m_HistoryStep(0)
	, m_NextRegId(1)
{
}

CPoint CImgRegionDoc::LocalToRoot(HTREEITEM hItem, const CPoint & ptLocal)
{
	if(NULL == hItem)
		return ptLocal;

	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	return LocalToRoot(m_TreeCtrl.GetParentItem(hItem), ptLocal + pReg->m_Location);
}

CPoint CImgRegionDoc::RootToLocal(HTREEITEM hItem, const CPoint & ptRoot)
{
	if(NULL == hItem || TVI_ROOT == hItem)
		return ptRoot;

	CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	return RootToLocal(m_TreeCtrl.GetParentItem(hItem), ptRoot - pReg->m_Location);
}

BOOL CImgRegionDoc::CreateTreeCtrl(void)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	static DWORD dwCtrlID = 4;

	if (!m_TreeCtrl.Create(WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, CRect(), &pFrame->m_wndFileView, dwCtrlID++))
	{
		TRACE0("CImgRegionDoc::CreateTreeCtrl failed \n");
		return FALSE;
	}

	//SetWindowTheme(m_TreeCtrl.m_hWnd, L"Explorer", NULL);

	CBitmap bmp;
	if (!bmp.LoadBitmap(IDB_BITMAP2))
	{
		TRACE0("bmp.LoadBitmap failed \n");
		return FALSE;
	}
	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	m_TreeImageList.Create(16, bmpObj.bmHeight, ILC_MASK | ILC_COLOR24, 0, 0);
	m_TreeImageList.Add(&bmp, RGB(255,0,255));
	m_TreeCtrl.SetImageList(&m_TreeImageList, TVSIL_NORMAL);

	return TRUE;
}

void CImgRegionDoc::DestroyTreeCtrl(void)
{
	HTREEITEM hNextItem = NULL;
	for(HTREEITEM hItem = m_TreeCtrl.GetRootItem(); NULL != hItem; hItem = hNextItem)
	{
		hNextItem = m_TreeCtrl.GetNextSiblingItem(hItem);

		m_TreeCtrl.DeleteTreeItem<CImgRegion>(hItem, TRUE);
	}

	ASSERT(m_TreeCtrl.m_ItemMap.empty());
	m_TreeCtrl.DestroyWindow();

	m_TreeImageList.DeleteImageList();
}

HTREEITEM CImgRegionDoc::GetPointedRegionNode(HTREEITEM hItem, const CPoint & ptLocal)
{
	// �������ײ���Ӧ����������⣬��Ϊ���Ȼ���ǰ���
	if(hItem)
	{
		HTREEITEM hRet;
		if(hRet = GetPointedRegionNode(m_TreeCtrl.GetNextSiblingItem(hItem), ptLocal))
			return hRet;

		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hItem);
		ASSERT(pReg);

		if(hRet = GetPointedRegionNode(m_TreeCtrl.GetChildItem(hItem), ptLocal - pReg->m_Location))
			return hRet;

		if(CRect(pReg->m_Location, pReg->m_Size).PtInRect(ptLocal))
			return hItem;
	}
	return NULL;
}

BOOL CImgRegionDoc::OnNewDocument(void)
{
	CImgRegionFilePropertyDlg dlg;
	dlg.m_Size.cx = theApp.GetProfileInt(_T("Settings"), _T("SizeX"), dlg.m_Size.cx);
	dlg.m_Size.cy = theApp.GetProfileInt(_T("Settings"), _T("SizeY"), dlg.m_Size.cy);
	dlg.m_Color = theApp.GetProfileInt(_T("Settings"), _T("Color"), dlg.m_Color);
	if(dlg.DoModal() == IDOK)
	{
		if (!CreateTreeCtrl())
			return FALSE;

		if (!CDocument::OnNewDocument())
			return FALSE;

		m_Size = dlg.m_Size;
		m_Color.SetFromCOLORREF(dlg.m_Color);
		m_ImageStr = dlg.m_ImageStr;
		m_Image = theApp.GetImage(m_ImageStr);

		theApp.WriteProfileInt(_T("Settings"), _T("SizeX"), dlg.m_Size.cx);
		theApp.WriteProfileInt(_T("Settings"), _T("SizeY"), dlg.m_Size.cy);
		theApp.WriteProfileInt(_T("Settings"), _T("Color"), dlg.m_Color);

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);

		pFrame->m_wndFileView.InvalidLayout();

		UpdateImageSizeTable(m_Size);

		return TRUE;
	}

	return FALSE;
}

BOOL CImgRegionDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CImgRegionDoc::CreateTreeCtrl())
		return FALSE;

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	UpdateImageSizeTable(m_Size);

	return TRUE;
}

BOOL CImgRegionDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if(!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;

	return TRUE;
}

void CImgRegionDoc::OnCloseDocument()
{
	if(m_TreeCtrl.m_hWnd)
	{
		DestroyTreeCtrl();
	}

	CDocument::OnCloseDocument();
}

static const TCHAR FILE_VERSION_DESC[] = _T("ImgRegion File Version: %d");

void CImgRegionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	// storing code
		CString strVersion; strVersion.Format(FILE_VERSION_DESC, CImgRegionDocFileVersions::FILE_VERSION); ar << strVersion;

		CImgRegionDocFileVersions::Serialize(this, ar, CImgRegionDocFileVersions::FILE_VERSION);
	}
	else
	{	// loading code
		CString strVersion; ar >> strVersion;

		// ! �汾�������Խ���ת���Ͱ汾�ĵ�
		int version;
		if(1 != _stscanf_s(strVersion, FILE_VERSION_DESC, &version))
		{
			AfxThrowArchiveException(CArchiveException::badIndex);
		}

		CImgRegionDocFileVersions::Serialize(this, ar, version);
	}
}

void CImgRegionDoc::SerializeSubTreeNode(CArchive & ar, int version, HTREEITEM hParent, BOOL bOverideName)
{
	CImgRegionDocFileVersions::SerializeSubTreeNode(this, ar, version, hParent, bOverideName);
}

void CImgRegionDoc::UpdateImageSizeTable(const CSize & sizeRoot)
{
	POSITION pos = GetFirstViewPosition();
	while(NULL != pos)
	{
		CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
		ASSERT(pView);

		pView->ZoomImage(pView->m_ImageZoomFactor);

		pView->Invalidate();
	}
}

void CImgRegionDoc::AddNewHistory(HistoryPtr hist)
{
	m_HistoryList.resize(m_HistoryStep++);
	m_HistoryList.push_back(hist);
}

void CImgRegionDoc::OnAddRegion()
{
	HTREEITEM hParent = NULL;
	CPoint ptOrg(10,10);
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	if(hSelected)
	{
		hParent = m_TreeCtrl.GetParentItem(hSelected);
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hSelected);
		ptOrg += pReg->m_Location;
	}

	CString strName;
	strName.Format(CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME, m_NextRegId++);
	HistoryAddRegionPtr hist(new HistoryAddRegion(
		this, strName, hParent ? m_TreeCtrl.GetItemText(hParent) : _T(""), hSelected ? m_TreeCtrl.GetItemText(hSelected) : _T("")));

	CImgRegion reg;
	reg.m_Location = ptOrg;
	reg.m_Size = CSize(100,100);
	reg.m_Color = Gdiplus::Color(255,my::Random<int>(0,255),my::Random<int>(0,255),my::Random<int>(0,255));
	reg.m_Font = theApp.GetFont(_T("΢���ź�"), 16);
	reg.m_FontColor = Gdiplus::Color(255,my::Random<int>(0,255),my::Random<int>(0,255),my::Random<int>(0,255));
	CArchive ar(&hist->m_NodeCache, CArchive::store);
	reg.Serialize(ar, CImgRegionDocFileVersions::FILE_VERSION);
	ar << 0;
	ar.Close();

	AddNewHistory(hist);
	hist->Do();

	UpdateAllViews(NULL);

	SetModifiedFlag();

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	pFrame->m_wndProperties.InvalidProperties();

	// ! ȥ��ActiveView��m_nSelectedHandle����
	POSITION pos = GetFirstViewPosition();
	while(NULL != pos)
	{
		CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
		ASSERT(pView);

		pView->m_nSelectedHandle = CImgRegionView::HandleTypeNone;
	}
}

void CImgRegionDoc::OnUpdateAddRegion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CImgRegionDoc::OnDelRegion()
{
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	if(hSelected)
	{
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hSelected);
		ASSERT(pReg);

		if(!pReg->m_Locked)
		{
			HistoryPtr hist(new HistoryDelRegion(
				this, m_TreeCtrl.GetItemText(hSelected)));

			AddNewHistory(hist);
			hist->Do();

			UpdateAllViews(NULL);

			SetModifiedFlag();

			CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
			ASSERT(pFrame);
			pFrame->m_wndProperties.InvalidProperties();
		}
		else
			::MessageBeep(MB_OK);
	}
}

void CImgRegionDoc::OnUpdateDelRegion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(NULL != m_TreeCtrl.GetSelectedItem());
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void CImgRegionDoc::OnExportImg()
{
	CFileDialog dlg(FALSE, _T("png"), GetTitle(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("PNG(*.png)|*.png|JPEG(*.jpg; *.jpeg; *.jpe)|*.jpg; *.jpeg; *.jpe|BMP(*.bmp; *.rle; *.dib)|*.bmp; *.rle; *.dib||"), NULL);
	if(dlg.DoModal() == IDOK)
	{
		CStringW format;
		format.Format(L"image/%s", theApp.GetMIME(dlg.GetFileExt()));
		CLSID encoderClsid;
		if(GetEncoderClsid(format, &encoderClsid) < 0)
		{
			AfxMessageBox(_T("the specified format was not supported"));
			return;
		}

		Gdiplus::Bitmap bmp(m_Size.cx, m_Size.cy, PixelFormat24bppRGB);
		Gdiplus::Graphics grap(&bmp);
		Gdiplus::Matrix world;

		CImgRegionView::DrawRegionDoc(grap, world, this);

		bmp.Save(dlg.GetPathName(), &encoderClsid, NULL);
	}
}

void CImgRegionDoc::OnFileProperty()
{
	CImgRegionFilePropertyDlg dlg;
	dlg.m_Size = m_Size;
	dlg.m_Color = m_Color.ToCOLORREF();
	dlg.m_ImageStr = m_ImageStr;
	if(dlg.DoModal() == IDOK)
	{
		m_Size = dlg.m_Size;
		m_Color.SetFromCOLORREF(dlg.m_Color);
		m_ImageStr = dlg.m_ImageStr;
		m_Image = theApp.GetImage(m_ImageStr);

		UpdateImageSizeTable(m_Size);

		SetModifiedFlag();
	}
}

void CImgRegionDoc::OnEditCopy()
{
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	if(hSelected)
	{
		CImgRegion * pReg = (CImgRegion *)m_TreeCtrl.GetItemData(hSelected);
		ASSERT(pReg);

		theApp.m_ClipboardFile.SetLength(0);
		CArchive ar(&theApp.m_ClipboardFile, CArchive::store);
		pReg->Serialize(ar, CImgRegionDocFileVersions::FILE_VERSION);
		SerializeSubTreeNode(ar, CImgRegionDocFileVersions::FILE_VERSION, hSelected);
		ar.Close();
	}
}

void CImgRegionDoc::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(NULL != m_TreeCtrl.GetSelectedItem());
}

#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)

void CImgRegionDoc::OnEditPaste()
{
	if(theApp.m_ClipboardFile.GetLength() > 0)
	{
		HTREEITEM hParent = NULL;
		HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
		if(hSelected)
		{
			hParent = m_TreeCtrl.GetParentItem(hSelected);
		}

		CString strName;
		strName.Format(CImgRegionDocFileVersions::DEFAULT_CONTROL_NAME, m_NextRegId++);
		HistoryAddRegionPtr hist(new HistoryAddRegion(
			this, strName, hParent ? m_TreeCtrl.GetItemText(hParent) : _T(""), m_TreeCtrl.GetItemText(hSelected)));

		void * pBuffer;
		void * pBufferMax;
		theApp.m_ClipboardFile.SeekToBegin();
		UINT len = theApp.m_ClipboardFile.GetBufferPtr(CFile::bufferRead, -1, &pBuffer, &pBufferMax);
		hist->m_NodeCache.Write(pBuffer, len);
		hist->m_NodeCache.Flush();

		AddNewHistory(hist);
		hist->Do();

		UpdateAllViews(NULL);

		SetModifiedFlag();

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.InvalidProperties();

		// ! ȥ��ActiveView��m_nSelectedHandle����
		POSITION pos = GetFirstViewPosition();
		while(NULL != pos)
		{
			CImgRegionView * pView = DYNAMIC_DOWNCAST(CImgRegionView, GetNextView(pos));
			ASSERT(pView);

			pView->m_nSelectedHandle = CImgRegionView::HandleTypeNone;
		}
	}
}

void CImgRegionDoc::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.m_ClipboardFile.GetLength() > 0);
}

void CImgRegionDoc::OnEditUndo()
{
	if(m_HistoryStep > 0)
	{
		m_HistoryList[--m_HistoryStep]->Undo();

		UpdateAllViews(NULL);

		SetModifiedFlag();

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.InvalidProperties();
	}
}

void CImgRegionDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_HistoryStep > 0);
}

void CImgRegionDoc::OnEditRedo()
{
	if(m_HistoryStep < m_HistoryList.size())
	{
		m_HistoryList[m_HistoryStep++]->Do();

		UpdateAllViews(NULL);

		SetModifiedFlag();

		CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
		ASSERT(pFrame);
		pFrame->m_wndProperties.InvalidProperties();
	}
}

void CImgRegionDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_HistoryStep < m_HistoryList.size());
}

void CImgRegionDoc::OnExportLua()
{
	CLuaExporterDlg dlg(this);
	dlg.m_strProjectDir = m_strProjectDir;
	dlg.m_strLuaPath = m_strLuaPath;
	dlg.DoModal();
	if(dlg.m_dirtyFlag)
	{
		m_strProjectDir = dlg.m_strProjectDir;
		m_strLuaPath = dlg.m_strLuaPath;

		SetModifiedFlag();
	}
}

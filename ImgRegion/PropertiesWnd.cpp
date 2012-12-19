
#include "stdafx.h"
#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "ImgRegionDoc.h"
#include "MainApp.h"

void CImgRegionPropertyGridFileProperty::OnClickButton(CPoint point)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndList);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	m_bButtonIsDown = TRUE;
	Redraw();

	CString strPath = m_varValue.bstrVal;
	BOOL bUpdate = FALSE;

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	CMDIChildWnd * pChild = pFrame->MDIGetActive();
	if(pChild)
	{
		CImgRegionDoc * pDoc = (CImgRegionDoc *)pChild->GetActiveDocument();
		if(pDoc)
		{
			CFileDialog dlg(m_bOpenFileDialog, m_strDefExt, strPath, m_dwFileOpenFlags, m_strFilter, m_pWndList);
			if (dlg.DoModal() == IDOK)
			{
				bUpdate = TRUE;
				strPath = dlg.GetPathName();
			}
		}
	}

	if (bUpdate)
	{
		if (m_pWndInPlace != NULL)
		{
			m_pWndInPlace->SetWindowText(strPath);
		}

		m_varValue = (LPCTSTR) strPath;
	}

	m_bButtonIsDown = FALSE;
	Redraw();

	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->SetFocus();
	}
	else
	{
		m_pWndList->SetFocus();
	}
}

CCheckBoxProp::CCheckBoxProp(const CString& strName, BOOL bCheck, LPCTSTR lpszDescr, DWORD dwData) :
	CMFCPropertyGridProperty(strName, COleVariant((long)bCheck), lpszDescr, dwData)
{
	m_rectCheck.SetRectEmpty();
}

void CCheckBoxProp::OnDrawName(CDC* pDC, CRect rect)
{
	m_rectCheck = rect;
	m_rectCheck.DeflateRect(1, 1);

	m_rectCheck.right = m_rectCheck.left + m_rectCheck.Height();

	rect.left = m_rectCheck.right + 1;

	CMFCPropertyGridProperty::OnDrawName(pDC, rect);

	OnDrawCheckBox(pDC, m_rectCheck, (m_varValue.boolVal));
}

void CCheckBoxProp::OnClickName(CPoint point)
{
	if (m_bEnabled && m_rectCheck.PtInRect(point))
	{
		m_varValue.boolVal = !(m_varValue.boolVal);
		m_pWndList->InvalidateRect(m_rectCheck);
		m_pWndList->OnPropertyChanged(this);
	}
}

BOOL CCheckBoxProp::OnDblClk(CPoint point)
{
	if (m_bEnabled && m_rectCheck.PtInRect(point))
	{
		return TRUE;
	}

	m_varValue.boolVal = !(m_varValue.boolVal);
	m_pWndList->InvalidateRect(m_rectCheck);
	m_pWndList->OnPropertyChanged(this);
	return TRUE;
}

void CCheckBoxProp::OnDrawCheckBox(CDC * pDC, CRect rect, BOOL bChecked)
{
	COLORREF clrTextOld = pDC->GetTextColor();

	CMFCVisualManager::GetInstance()->OnDrawCheckBox(pDC, rect, FALSE, bChecked, m_bEnabled);

	pDC->SetTextColor(clrTextOld);
}

BOOL CCheckBoxProp::PushChar(UINT nChar)
{
	if (nChar == VK_SPACE)
	{
		OnDblClk(CPoint(-1, -1));
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

CPropertiesWnd::CPropertiesWnd()
	: m_bIsPropInvalid(FALSE)
{
}

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_wndPropList.SetWindowPos(NULL,
		rectClient.left,
		rectClient.top,
		rectClient.Width(),
		rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, CRect(), this, 2))
	{
		TRACE0("未能创建属性网格\n");
		return -1;
	}

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	SetPropListFont();

	AdjustLayout();

	CMFCPropertyGridProperty * pGroup = new CMFCPropertyGridProperty(_T("外观"));

	CMFCPropertyGridProperty * pProp = new CCheckBoxProp(_T("锁住"), FALSE, _T("锁住移动属性"), PropertyItemLocked);
	pGroup->AddSubItem(m_pProp[PropertyItemLocked] = pProp);

	CMFCPropertyGridProperty * pLocal = new CMFCPropertyGridProperty(_T("Local"), PropertyItemLocal, TRUE);
	pProp = new CMFCPropertyGridProperty(_T("x"), (_variant_t)0l, _T("x坐标"), PropertyItemLocalX);
	pLocal->AddSubItem(m_pProp[PropertyItemLocalX] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("y"), (_variant_t)0l, _T("y坐标"), PropertyItemLocalY);
	pLocal->AddSubItem(m_pProp[PropertyItemLocalY] = pProp);
	pGroup->AddSubItem(m_pProp[PropertyItemLocal] = pLocal);

	pLocal = new CMFCPropertyGridProperty(_T("Size"), PropertyItemSize, TRUE);
	pProp = new CMFCPropertyGridProperty(_T("w"), (_variant_t)100l, _T("宽度"), PropertyItemSizeW);
	pLocal->AddSubItem(m_pProp[PropertyItemSizeW] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("h"), (_variant_t)100l, _T("高度"), PropertyItemSizeH);
	pLocal->AddSubItem(m_pProp[PropertyItemSizeH] = pProp);
	pGroup->AddSubItem(m_pProp[PropertyItemSize] = pLocal);

	m_wndPropList.AddProperty(m_pProp[PropertyGroupCoord] = pGroup);

	pGroup = new CMFCPropertyGridProperty(_T("颜色"));

	pProp = new CMFCPropertyGridProperty(_T("Alpha"), (_variant_t)255l, _T("透明值"), PropertyItemAlpha);
	pGroup->AddSubItem(m_pProp[PropertyItemAlpha] = pProp);

	CMFCPropertyGridColorProperty * pColorProp = new CMFCPropertyGridColorProperty(_T("颜色"), RGB(255,0,0), NULL, _T("颜色"), PropertyItemRGB);
	pColorProp->EnableOtherButton(_T("其他..."));
	pColorProp->EnableAutomaticButton(_T("默认"), RGB(255,255,255));
	pGroup->AddSubItem(m_pProp[PropertyItemRGB] = pColorProp);

	CMFCPropertyGridFileProperty * pFileProp = new CImgRegionPropertyGridFileProperty(
		_T("图片"), TRUE, _T("aaa.png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("图片文件(*.bmp; *.jpg; *.png)|*.bmp;*.jpg;*.png|All Files(*.*)|*.*||"), _T("图片文件"), PropertyItemImage);
	pGroup->AddSubItem(m_pProp[PropertyItemImage] = pFileProp);

	pLocal = new CMFCPropertyGridProperty(_T("Border"), PropertyItemBorder, TRUE);
	pProp = new CMFCPropertyGridProperty(_T("x"), (_variant_t)0l, _T("左边距"), PropertyItemBorderX);
	pLocal->AddSubItem(m_pProp[PropertyItemBorderX] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("y"), (_variant_t)0l, _T("上边距"), PropertyItemBorderY);
	pLocal->AddSubItem(m_pProp[PropertyItemBorderY] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("z"), (_variant_t)0l, _T("右边距"), PropertyItemBorderZ);
	pLocal->AddSubItem(m_pProp[PropertyItemBorderZ] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("w"), (_variant_t)0l, _T("下边距"), PropertyItemBorderW);
	pLocal->AddSubItem(m_pProp[PropertyItemBorderW] = pProp);
	pGroup->AddSubItem(m_pProp[PropertyItemBorder] = pLocal);

	m_wndPropList.AddProperty(m_pProp[PropertyGroupImage] = pGroup);

	pGroup = new CMFCPropertyGridProperty(_T("字体"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);
	lstrcpy(lf.lfFaceName, _T("宋体, Arial"));
	pProp = new CMFCPropertyGridProperty(_T("字体"), _T("新宋体"), _T("选择字体"), PropertyItemFont);
	pProp->AllowEdit(FALSE);
	for(int i = 0; i < theApp.fontFamilies.GetSize(); i++)
	{
		CString strFamily;
		theApp.fontFamilies[i].GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE));
		pProp->AddOption(strFamily);
	}
	pGroup->AddSubItem(m_pProp[PropertyItemFont] = pProp);

	pProp = new CMFCPropertyGridProperty(_T("字号"), (_variant_t)12l, _T("字体大小"), PropertyItemFontSize);
	pGroup->AddSubItem(m_pProp[PropertyItemFontSize] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("Alpha"), (_variant_t)255l, _T("透明值"), PropertyItemFontAlpha);
	pGroup->AddSubItem(m_pProp[PropertyItemFontAlpha] = pProp);
	pColorProp = new CMFCPropertyGridColorProperty(_T("颜色"), RGB(0,0,255), NULL, _T("颜色"), PropertyItemFontRGB);
	pColorProp->EnableOtherButton(_T("其他..."));
	pColorProp->EnableAutomaticButton(_T("默认"), RGB(0,0,255));
	pGroup->AddSubItem(m_pProp[PropertyItemFontRGB] = pColorProp);

	m_wndPropList.AddProperty(m_pProp[PropertyGroupFont] = pGroup);

	pGroup = new CMFCPropertyGridProperty(_T("文本"));

	pLocal = new CMFCPropertyGridProperty(_T("偏移"), PropertyItemTextOff, TRUE);
	pProp = new CMFCPropertyGridProperty(_T("x"), (_variant_t)0l, _T("x坐标"), PropertyItemTextOffX);
	pLocal->AddSubItem(m_pProp[PropertyItemTextOffX] = pProp);
	pProp = new CMFCPropertyGridProperty(_T("y"), (_variant_t)0l, _T("y坐标"), PropertyItemTextOffY);
	pLocal->AddSubItem(m_pProp[PropertyItemTextOffY] = pProp);
	pGroup->AddSubItem(m_pProp[PropertyItemTextOff] = pLocal);

	pProp = new CMFCPropertyGridProperty(_T("文本"), _T(""), _T("输出内容"), PropertyItemText);
	pGroup->AddSubItem(m_pProp[PropertyItemText] = pProp);

	m_wndPropList.AddProperty(m_pProp[PropertyGroupText] = pGroup);

	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);

	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

void CPropertiesWnd::OnIdleUpdate()
{
	if(m_bIsPropInvalid)
	{
		UpdateProperties();

		m_bIsPropInvalid = FALSE;
	}
}

void CPropertiesWnd::UpdateProperties(void)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	CMDIChildWnd * pChild = pFrame->MDIGetActive();
	if(pChild)
	{
		CImgRegionDoc * pDoc = (CImgRegionDoc *)pChild->GetActiveDocument();
		if(pDoc)
		{
			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			if(hSelected)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				m_pProp[PropertyItemLocked]->SetValue((_variant_t)(long)pReg->m_Locked);
				m_pProp[PropertyItemLocalX]->SetValue((_variant_t)pReg->m_Local.x);
				m_pProp[PropertyItemLocalY]->SetValue((_variant_t)pReg->m_Local.y);
				m_pProp[PropertyItemSizeW]->SetValue((_variant_t)pReg->m_Size.cx);
				m_pProp[PropertyItemSizeH]->SetValue((_variant_t)pReg->m_Size.cy);
				m_pProp[PropertyItemAlpha]->SetValue((_variant_t)(long)pReg->m_Color.GetAlpha());
				((CMFCPropertyGridColorProperty *)m_pProp[PropertyItemRGB])->SetColor(pReg->m_Color.ToCOLORREF());

				((CImgRegionPropertyGridFileProperty *)m_pProp[PropertyItemImage])->SetValue((_variant_t)pReg->m_ImageStr);
				m_pProp[PropertyItemBorderX]->SetValue((_variant_t)pReg->m_Border.x);
				m_pProp[PropertyItemBorderY]->SetValue((_variant_t)pReg->m_Border.y);
				m_pProp[PropertyItemBorderZ]->SetValue((_variant_t)pReg->m_Border.z);
				m_pProp[PropertyItemBorderW]->SetValue((_variant_t)pReg->m_Border.w);

				Gdiplus::FontFamily family;
				pReg->m_Font->GetFamily(&family);
				CString strFamily;
				family.GetFamilyName(strFamily.GetBufferSetLength(LF_FACESIZE));
				m_pProp[PropertyItemFont]->SetValue((_variant_t)strFamily);
				m_pProp[PropertyItemFontSize]->SetValue((long)pReg->m_Font->GetSize());
				m_pProp[PropertyItemFontAlpha]->SetValue((_variant_t)(long)pReg->m_FontColor.GetAlpha());
				((CMFCPropertyGridColorProperty *)m_pProp[PropertyItemFontRGB])->SetColor(pReg->m_FontColor.ToCOLORREF());

				m_pProp[PropertyItemTextOffX]->SetValue((_variant_t)pReg->m_TextOff.x);
				m_pProp[PropertyItemTextOffY]->SetValue((_variant_t)pReg->m_TextOff.y);
				m_pProp[PropertyItemText]->SetValue((_variant_t)pReg->m_Text);

				return;
			}
		}
	}
}

LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	ASSERT(2 == wParam);
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	CMDIChildWnd * pChild = pFrame->MDIGetActive();
	if(pChild)
	{
		CImgRegionDoc * pDoc = (CImgRegionDoc *)pChild->GetActiveDocument();
		if(pDoc)
		{
			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			if(hSelected)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
				ASSERT(pProp);
				Property PropertyIdx = (Property)pProp->GetData();
				COLORREF color;
				switch(PropertyIdx)
				{
				case PropertyItemLocked:
					pReg->m_Locked = m_pProp[PropertyItemLocked]->GetValue().lVal;
					break;

				case PropertyItemLocal:
				case PropertyItemLocalX:
				case PropertyItemLocalY:
					pReg->m_Local.x = m_pProp[PropertyItemLocalX]->GetValue().lVal;
					pReg->m_Local.y = m_pProp[PropertyItemLocalY]->GetValue().lVal;
					break;

				case PropertyItemSize:
				case PropertyItemSizeW:
				case PropertyItemSizeH:
					pReg->m_Size.cx = m_pProp[PropertyItemSizeW]->GetValue().lVal;
					pReg->m_Size.cy = m_pProp[PropertyItemSizeH]->GetValue().lVal;
					break;

				case PropertyItemAlpha:
				case PropertyItemRGB:
					color = ((CMFCPropertyGridColorProperty *)m_pProp[PropertyItemRGB])->GetColor();
					pReg->m_Color = Gdiplus::Color(
						(BYTE)my::Clamp(m_pProp[PropertyItemAlpha]->GetValue().lVal, 0l, 255l), GetRValue(color), GetGValue(color), GetBValue(color));
					break;

				case PropertyItemImage:
					pReg->m_ImageStr = ((CMFCPropertyGridFileProperty *)m_pProp[PropertyItemImage])->GetValue().bstrVal;
					pReg->m_Image = theApp.GetImage(pReg->m_ImageStr);
					break;

				case PropertyItemBorder:
				case PropertyItemBorderX:
				case PropertyItemBorderY:
				case PropertyItemBorderZ:
				case PropertyItemBorderW:
					pReg->m_Border.x = m_pProp[PropertyItemBorderX]->GetValue().lVal;
					pReg->m_Border.y = m_pProp[PropertyItemBorderY]->GetValue().lVal;
					pReg->m_Border.z = m_pProp[PropertyItemBorderZ]->GetValue().lVal;
					pReg->m_Border.w = m_pProp[PropertyItemBorderW]->GetValue().lVal;
					break;

				case PropertyItemFont:
				case PropertyItemFontSize:
					pReg->m_Font = theApp.GetFont(
						m_pProp[PropertyItemFont]->GetValue().bstrVal,
						(float)m_pProp[PropertyItemFontSize]->GetValue().lVal);
					break;

				case PropertyItemFontAlpha:
				case PropertyItemFontRGB:
					color = ((CMFCPropertyGridColorProperty *)m_pProp[PropertyItemFontRGB])->GetColor();
					pReg->m_FontColor = Gdiplus::Color(
						(BYTE)my::Clamp(m_pProp[PropertyItemFontAlpha]->GetValue().lVal, 0l, 255l), GetRValue(color), GetGValue(color), GetBValue(color));
					break;

				case PropertyItemTextOff:
				case PropertyItemTextOffX:
				case PropertyItemTextOffY:
					pReg->m_TextOff.x = m_pProp[PropertyItemTextOffX]->GetValue().lVal;
					pReg->m_TextOff.y = m_pProp[PropertyItemTextOffY]->GetValue().lVal;
					break;

				case PropertyItemText:
					pReg->m_Text = m_pProp[PropertyItemText]->GetValue().bstrVal;
				}

				pDoc->UpdateAllViews(NULL);

				pDoc->SetModifiedFlag();
			}
		}
	}
	return 0;
}

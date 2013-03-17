
#include "stdafx.h"
#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "ImgRegionDoc.h"
#include "MainApp.h"

void CSimpleProp::SetValue(const COleVariant& varValue)
{
	ASSERT_VALID(this);

	if (m_varValue.vt != VT_EMPTY && m_varValue.vt != varValue.vt)
	{
		ASSERT(FALSE);
		return;
	}

	BOOL bInPlaceEdit = m_bInPlaceEdit;
	if (bInPlaceEdit)
	{
		OnEndEdit();
	}

	m_varValue = varValue;
	//Redraw();

	if (bInPlaceEdit)
	{
		ASSERT_VALID(m_pWndList);
		m_pWndList->EditItem(this);
	}
}

void CColorProp::SetColor(COLORREF color)
{
	ASSERT_VALID(this);

	m_Color = color;
	m_varValue = (LONG) color;

	//if (::IsWindow(m_pWndList->GetSafeHwnd()))
	//{
	//	CRect rect = m_Rect;
	//	rect.DeflateRect(0, 1);

	//	m_pWndList->InvalidateRect(rect);
	//	m_pWndList->UpdateWindow();
	//}

	if (m_pWndInPlace != NULL)
	{
		ASSERT_VALID(m_pWndInPlace);
		m_pWndInPlace->SetWindowText(FormatProperty());
	}
}

void CFileProp::OnClickButton(CPoint point)
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

void CFileProp::SetValue(const COleVariant& varValue)
{
	ASSERT_VALID(this);

	if (m_varValue.vt != VT_EMPTY && m_varValue.vt != varValue.vt)
	{
		ASSERT(FALSE);
		return;
	}

	BOOL bInPlaceEdit = m_bInPlaceEdit;
	if (bInPlaceEdit)
	{
		OnEndEdit();
	}

	m_varValue = varValue;
	//Redraw();

	if (bInPlaceEdit)
	{
		ASSERT_VALID(m_pWndList);
		m_pWndList->EditItem(this);
	}
}

CWnd* CSliderProp::CreateInPlaceEdit(CRect rectEdit, BOOL& bDefaultFormat)
{
	CPropSliderCtrl* pWndSlider = new CPropSliderCtrl(this);

	CRect rectSlider(CPoint(rectEdit.right-120,rectEdit.bottom),CSize(120,30));

	pWndSlider->Create(TBS_HORZ | TBS_TOP | WS_VISIBLE | WS_CHILD, rectSlider, m_pWndList, AFX_PROPLIST_ID_INPLACE);
	pWndSlider->SetRange(0,255);
	pWndSlider->SetPos(m_varValue.lVal);

	bDefaultFormat = TRUE;
	return pWndSlider;
}

BOOL CSliderProp::OnUpdateValue()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndInPlace);
	ASSERT_VALID(m_pWndList);
	ASSERT(::IsWindow(m_pWndInPlace->GetSafeHwnd()));

	long lCurrValue = m_varValue.lVal;

	CSliderCtrl* pSlider = (CSliderCtrl*) m_pWndInPlace;

	m_varValue = (long) pSlider->GetPos();

	if (lCurrValue != m_varValue.lVal)
	{
		m_pWndList->OnPropertyChanged(this);
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPropSliderCtrl, CSliderCtrl)
	ON_WM_HSCROLL_REFLECT()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CPropSliderCtrl::HScroll(UINT nSBCode, UINT nPos)
{
	ASSERT_VALID(m_pProp);

	m_pProp->OnUpdateValue();
	m_pProp->Redraw();
}

void CPropSliderCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CSliderCtrl::OnKillFocus(pNewWnd);

	ASSERT_VALID(m_pProp);

	m_pProp->OnEndEdit();
}

void CComboProp::OnSelectCombo()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndCombo);
	ASSERT_VALID(m_pWndInPlace);

	m_iSelIndex = m_pWndCombo->GetCurSel();
	if (m_iSelIndex >= 0)
	{
		CString str;
		m_pWndCombo->GetLBText(m_iSelIndex, str);
		m_pWndInPlace->SetWindowText(str);
		OnUpdateValue();
	}
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
			if(hSelected && hSelected == m_hSelectedNode)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				pReg->UpdateProperties(this);

				m_wndPropList.Invalidate();

				return;
			}
			else if(hSelected)
			{
				m_hSelectedNode = hSelected;

				m_wndPropList.RemoveAll();

				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				pReg->CreateProperties(this);

				return;
			}
		}
	}

	m_hSelectedNode = NULL;

	m_wndPropList.RemoveAll();

	m_wndPropList.Invalidate();
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
					if(pReg->m_Locked = m_pProp[PropertyItemLocked]->GetValue().lVal)
						pDoc->m_TreeCtrl.SetItemImage(hSelected, 1, 1);
					else
						pDoc->m_TreeCtrl.SetItemImage(hSelected, 0, 0);
					break;

				case PropertyItemLocation:
				case PropertyItemLocationX:
				case PropertyItemLocationY:
					{
						HistoryModifyRegionPtr hist(new HistoryModifyRegion());
						hist->push_back(HistoryChangePtr(new HistoryChangeItemLocation(
							pDoc, pDoc->m_TreeCtrl.GetItemText(hSelected), pReg->m_Location, CPoint(m_pProp[PropertyItemLocationX]->GetValue().lVal, m_pProp[PropertyItemLocationY]->GetValue().lVal))));
						pDoc->AddNewHistory(hist);
						hist->Do();
					}
					break;

				case PropertyItemSize:
				case PropertyItemSizeW:
				case PropertyItemSizeH:
					{
						HistoryModifyRegionPtr hist(new HistoryModifyRegion());
						hist->push_back(HistoryChangePtr(new HistoryChangeItemSize(
							pDoc, pDoc->m_TreeCtrl.GetItemText(hSelected), pReg->m_Size, CPoint(m_pProp[PropertyItemSizeW]->GetValue().lVal, m_pProp[PropertyItemSizeH]->GetValue().lVal))));
						pDoc->AddNewHistory(hist);
						hist->Do();
					}
					break;

				case PropertyItemAlpha:
				case PropertyItemRGB:
					color = ((CColorProp *)m_pProp[PropertyItemRGB])->GetColor();
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
					color = ((CColorProp *)m_pProp[PropertyItemFontRGB])->GetColor();
					pReg->m_FontColor = Gdiplus::Color(
						(BYTE)my::Clamp(m_pProp[PropertyItemFontAlpha]->GetValue().lVal, 0l, 255l), GetRValue(color), GetGValue(color), GetBValue(color));
					break;

				case PropertyItemText:
					pReg->m_Text = m_pProp[PropertyItemText]->GetValue().bstrVal;
					break;

				case PropertyItemTextAlign:
					pReg->m_TextAlign = my::Clamp<DWORD>(((CComboProp *)m_pProp[PropertyItemTextAlign])->m_iSelIndex, 0, TextAlignCount - 1);
					break;

				case PropertyItemTextWrap:
					pReg->m_TextWrap = m_pProp[PropertyItemTextWrap]->GetValue().lVal;
					break;

				case PropertyItemTextOff:
				case PropertyItemTextOffX:
				case PropertyItemTextOffY:
					{
						HistoryModifyRegionPtr hist(new HistoryModifyRegion());
						hist->push_back(HistoryChangePtr(new HistoryChangeItemTextOff(
							pDoc, pDoc->m_TreeCtrl.GetItemText(hSelected), pReg->m_TextOff, CPoint(m_pProp[PropertyItemTextOffX]->GetValue().lVal, m_pProp[PropertyItemTextOffY]->GetValue().lVal))));
						pDoc->AddNewHistory(hist);
						hist->Do();
					}
					break;
				}

				pDoc->UpdateAllViews(NULL);

				pDoc->SetModifiedFlag();
			}
		}
	}
	return 0;
}

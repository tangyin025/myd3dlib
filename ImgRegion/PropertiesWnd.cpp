
#include "stdafx.h"
#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "ImgRegionDoc.h"
#include "MainApp.h"

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CPropertiesWnd::OnIdleUpdateCmdUI)
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

LRESULT CPropertiesWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if(m_bIsPropInvalid)
	{
		UpdateProperties();

		m_bIsPropInvalid = FALSE;
	}
	return 0;
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
				CImgRegionPtr pReg = pDoc->GetItemNode(hSelected);
				ASSERT(pReg);

				pReg->UpdateProperties(this, pDoc->m_TreeCtrl.GetItemText(hSelected));

				m_wndPropList.Invalidate();

				return;
			}
			else if(hSelected)
			{
				m_hSelectedNode = hSelected;

				m_wndPropList.RemoveAll();

				CImgRegionPtr pReg = pDoc->GetItemNode(hSelected);
				ASSERT(pReg);

				pReg->CreateProperties(this, pDoc->m_TreeCtrl.GetItemText(hSelected));

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
			if(m_hSelectedNode)
			{
				CImgRegionPtr pReg = pDoc->GetItemNode(m_hSelectedNode);
				ASSERT(pReg);

				CMFCPropertyGridProperty * pProp = (CMFCPropertyGridProperty *)lParam;
				ASSERT(pProp);
				Property PropertyIdx = (Property)pProp->GetData();
				COLORREF color;
				switch(PropertyIdx)
				{
				case PropertyItemName:
					pDoc->m_TreeCtrl.SetItemText(m_hSelectedNode, m_pProp[PropertyItemName]->GetValue().bstrVal);
					break;

				case PropertyItemClass:
					pReg->m_Class = m_pProp[PropertyItemClass]->GetValue().bstrVal;
					break;

				case PropertyItemLocked:
					if(pReg->m_Locked = m_pProp[PropertyItemLocked]->GetValue().boolVal)
						pDoc->m_TreeCtrl.SetItemImage(m_hSelectedNode, 1, 1);
					else
						pDoc->m_TreeCtrl.SetItemImage(m_hSelectedNode, 0, 0);
					break;

				case PropertyItemLocation:
				case PropertyItemLocationX:
				case PropertyItemLocationXScale:
				case PropertyItemLocationY:
				case PropertyItemLocationYScale:
					{
						HistoryModifyRegionPtr hist(new HistoryModifyRegion());
						hist->push_back(HistoryChangePtr(new HistoryChangeItemLocation(
							pDoc, pDoc->GetItemId(m_hSelectedNode), std::make_pair(pReg->m_x, pReg->m_y), std::make_pair(
								my::UDim(m_pProp[PropertyItemLocationXScale]->GetValue().fltVal, m_pProp[PropertyItemLocationX]->GetValue().fltVal),
								my::UDim(m_pProp[PropertyItemLocationYScale]->GetValue().fltVal, m_pProp[PropertyItemLocationY]->GetValue().fltVal)))));
						pDoc->AddNewHistory(hist);
						hist->Do();
					}
					break;

				case PropertyItemSize:
				case PropertyItemSizeW:
				case PropertyItemSizeWScale:
				case PropertyItemSizeH:
				case PropertyItemSizeHScale:
					{
						HistoryModifyRegionPtr hist(new HistoryModifyRegion());
						hist->push_back(HistoryChangePtr(new HistoryChangeItemSize(
							pDoc, pDoc->GetItemId(m_hSelectedNode), std::make_pair(pReg->m_Width, pReg->m_Height), std::make_pair(
								my::UDim(m_pProp[PropertyItemSizeWScale]->GetValue().fltVal, m_pProp[PropertyItemSizeW]->GetValue().fltVal),
								my::UDim(m_pProp[PropertyItemSizeHScale]->GetValue().fltVal, m_pProp[PropertyItemSizeH]->GetValue().fltVal)))));
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
					pReg->m_ImageRect = Gdiplus::Rect(0, 0, pReg->m_Image->GetWidth(), pReg->m_Image->GetHeight());
					break;

				case PropertyItemRect:
				case PropertyItemRectL:
				case PropertyItemRectT:
				case PropertyItemRectW:
				case PropertyItemRectH:
					pReg->m_ImageRect.X = m_pProp[PropertyItemRectL]->GetValue().lVal;
					pReg->m_ImageRect.Y = m_pProp[PropertyItemRectT]->GetValue().lVal;
					pReg->m_ImageRect.Width = m_pProp[PropertyItemRectW]->GetValue().lVal;
					pReg->m_ImageRect.Height = m_pProp[PropertyItemRectH]->GetValue().lVal;
					break;

				case PropertyItemBorder:
				case PropertyItemBorderX:
				case PropertyItemBorderY:
				case PropertyItemBorderZ:
				case PropertyItemBorderW:
					pReg->m_ImageBorder.x = m_pProp[PropertyItemBorderX]->GetValue().lVal;
					pReg->m_ImageBorder.y = m_pProp[PropertyItemBorderY]->GetValue().lVal;
					pReg->m_ImageBorder.z = m_pProp[PropertyItemBorderZ]->GetValue().lVal;
					pReg->m_ImageBorder.w = m_pProp[PropertyItemBorderW]->GetValue().lVal;
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
					pReg->m_TextWrap = m_pProp[PropertyItemTextWrap]->GetValue().boolVal;
					break;

				case PropertyItemTextOff:
				case PropertyItemTextOffX:
				case PropertyItemTextOffY:
					{
						HistoryModifyRegionPtr hist(new HistoryModifyRegion());
						hist->push_back(HistoryChangePtr(new HistoryChangeItemTextOff(
							pDoc, pDoc->GetItemId(m_hSelectedNode), pReg->m_TextOff, CPoint(m_pProp[PropertyItemTextOffX]->GetValue().lVal, m_pProp[PropertyItemTextOffY]->GetValue().lVal))));
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

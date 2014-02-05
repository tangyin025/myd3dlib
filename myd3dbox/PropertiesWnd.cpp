#include "StdAfx.h"
#include "PropertiesWnd.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "MainView.h"

IMPLEMENT_DYNAMIC(CSimpleProp, CMFCPropertyGridProperty)

void CSimpleProp::OnEventChanged(void)
{
	if(m_EventChanged)
	{
		m_EventChanged();
	}
	else
	{
		for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
		{
			CSimpleProp * pProp = DYNAMIC_DOWNCAST(CSimpleProp, m_lstSubItems.GetNext(pos));
			ASSERT_VALID(pProp);
			pProp->OnEventChanged();
		}
	}
}

void CSimpleProp::OnEventUpdated(void)
{
	if(m_EventUpdated)
	{
		m_EventUpdated();
	}
	else
	{
		for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
		{
			CSimpleProp * pProp = DYNAMIC_DOWNCAST(CSimpleProp, m_lstSubItems.GetNext(pos));
			ASSERT_VALID(pProp);
			pProp->OnEventUpdated();
		}
	}
}

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

	CString strPath(m_varValue.bstrVal);
	BOOL bUpdate = FALSE;

	if (m_bIsFolder)
	{
		if (theApp.GetShellManager() == NULL)
		{
			CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
			if (pApp != NULL)
			{
				pApp->InitShellManager();
			}
		}

		if (theApp.GetShellManager() == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			bUpdate = theApp.GetShellManager()->BrowseForFolder(strPath, m_pWndList, strPath);
		}
	}
	else
	{
		CFileDialog dlg(m_bOpenFileDialog, m_strDefExt, strPath, m_dwFileOpenFlags, m_strFilter, m_pWndList);
		if (dlg.DoModal() == IDOK)
		{
			bUpdate = TRUE;
			strPath = dlg.GetPathName();
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
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CPropertiesWnd::OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd())
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

		m_wndToolBar.SetWindowPos(
			NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

		m_wndPropList.SetWindowPos(
			NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	}
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
	m_wndPropList.SetFont(&(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_fntPropList);

	if (!m_wndToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_HIDE_INPLACE | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR1)
		|| !m_wndToolBar.LoadToolBar(IDR_TOOLBAR2, 0, 0, TRUE))
		return -1;

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

LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	CSimpleProp * pProp = DYNAMIC_DOWNCAST(CSimpleProp, (CMFCPropertyGridProperty *)lParam);
	_ASSERT(pProp);
	pProp->OnEventChanged();

	TreeNodeBasePtr node = COutlinerView::getSingleton().GetSelectedNode();
	if(node)
	{
		CMainView::getSingleton().m_PivotController.m_Position = node->m_Position;
		CMainView::getSingleton().m_PivotController.m_Rotation = node->m_Rotation;
		CMainView::getSingleton().m_PivotController.UpdateViewTransform(CMainView::getSingleton().m_Camera.m_ViewProj, CMainView::getSingleton().m_SwapChainBufferDesc.Width);
		CMainView::getSingleton().Invalidate();
	}
	return 0;
}

LRESULT CPropertiesWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	CDockablePane::OnIdleUpdateCmdUI(wParam, lParam);

	TreeNodeBasePtr node = COutlinerView::getSingleton().GetSelectedNode();
	if(!node)
	{
		if(m_wndPropList.GetPropertyCount() > 0)
		{
			m_wndPropList.RemoveAll();

			m_wndPropList.Invalidate();

			m_SelectedNode.reset();
		}
	}
	else if(node == m_SelectedNode.lock())
	{
		if(m_bIsPropInvalid)
		{
			for(int i = 0; i < m_wndPropList.GetPropertyCount(); i++)
			{
				CSimpleProp * pProp = DYNAMIC_DOWNCAST(CSimpleProp, m_wndPropList.GetProperty(i));
				_ASSERT(pProp);
				pProp->OnEventUpdated();
			}

			m_bIsPropInvalid = FALSE;

			m_wndPropList.Invalidate();
		}
	}
	else
	{
		m_wndPropList.RemoveAll();

		m_wndPropList.Invalidate();

		node->SetupProperties(&m_wndPropList);

		m_bIsPropInvalid = TRUE;

		m_SelectedNode = node;
	}
	return 0;
}

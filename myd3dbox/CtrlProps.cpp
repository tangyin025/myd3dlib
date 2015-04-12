#include "stdafx.h"
#include "CtrlProps.h"

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
	CFileDialog dlg(m_bOpenFileDialog, m_strDefExt, strPath, m_dwFileOpenFlags, m_strFilter, m_pWndList);
	if (dlg.DoModal() == IDOK)
	{
		bUpdate = TRUE;
		strPath = dlg.GetPathName();
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

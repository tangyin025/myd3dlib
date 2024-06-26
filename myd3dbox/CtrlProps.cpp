// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "stdafx.h"
#include "CtrlProps.h"

BOOL CMFCPropertyGridPropertyReader::RemoveSubItem(CMFCPropertyGridProperty*& pProp, BOOL bDelete/* = TRUE*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	for (POSITION pos = m_lstSubItems.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSaved = pos;

		CMFCPropertyGridProperty* pListProp = m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pListProp);

		if (pListProp == pProp)
		{
			m_lstSubItems.RemoveAt(posSaved);

			if (m_pWndList != NULL && (static_cast<CMFCPropertyGridCtrlReader *>(m_pWndList)->m_pSel != NULL))
			{
				if (static_cast<CMFCPropertyGridCtrlReader *>(m_pWndList)->m_pSel == pProp
					|| static_cast<CMFCPropertyGridPropertyReader *>(pProp)->IsSubItem(static_cast<CMFCPropertyGridCtrlReader *>(m_pWndList)->m_pSel))
				{
					static_cast<CMFCPropertyGridCtrlReader *>(m_pWndList)->m_pSel = NULL;
				}
			}

			if (bDelete)
			{
				delete pProp;
				pProp = NULL;
			}

			return TRUE;
		}

		if (pListProp->RemoveSubItem(pProp, bDelete))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMFCPropertyGridCtrlReader::DeleteProperty(CMFCPropertyGridProperty*& pProp, BOOL bRedraw, BOOL bAdjustLayout)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pProp);

	BOOL bFound = FALSE;

	for (POSITION pos = m_lstProps.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSaved = pos;

		CMFCPropertyGridProperty* pListProp = m_lstProps.GetNext(pos);
		ASSERT_VALID(pListProp);

		if (pListProp == pProp) // Top level property
		{
			bFound = TRUE;

			m_lstProps.RemoveAt(posSaved);
			break;
		}

		if (pListProp->RemoveSubItem(pProp, FALSE))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		return FALSE;
	}

	if (m_pSel != NULL && (m_pSel == pProp || static_cast<CMFCPropertyGridPropertyReader *>(pProp)->IsSubItem(m_pSel)))
	{
		m_pSel = NULL;
	}

	delete pProp;
	pProp = NULL;

	if (bAdjustLayout)
	{
		AdjustLayout();
		return TRUE;
	}

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}

	return TRUE;
}

IMPLEMENT_DYNAMIC(CSimpleProp, CMFCPropertyGridProperty)

INT CSimpleProp::GetSubIndexInParent(CMFCPropertyGridProperty * pProp)
{
	CSimpleProp * pParent = DYNAMIC_DOWNCAST(CSimpleProp, pProp->GetParent());
	INT i = 0;
	for (POSITION pos = pParent->m_lstSubItems.GetHeadPosition(); pos != NULL; i++)
	{
		CMFCPropertyGridProperty* pSub = pParent->m_lstSubItems.GetNext(pos);
		ASSERT_VALID(pSub);
		if (pSub == pProp)
		{
			return i;
		}
	}
	ASSERT(FALSE);
	return 0;
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

IMPLEMENT_DYNAMIC(CColorProp, CMFCPropertyGridColorProperty)

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
	pWndSlider->SetRange(0, RANGE);
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

IMPLEMENT_DYNAMIC(CComboProp, CSimpleProp)

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

BOOL CComboProp::OnEdit(LPPOINT lptClick)
{
	return CSimpleProp::OnEdit(lptClick);
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

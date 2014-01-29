#include "StdAfx.h"
#include "OutputWnd.h"
#include "MainFrm.h"

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_EditCtrl.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | ES_WANTRETURN, CRect(10,10,100,100), this, 4))
		return -1;
	m_EditCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_EditCtrl.SetFont(&(DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd()))->m_fntPropList);
	return 0;
}

void COutputWnd::AdjustLayout(void)
{
	if (GetSafeHwnd())
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		m_EditCtrl.SetWindowPos(
			NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void COutputWnd::AddString(LPCTSTR lpString)
{
	m_EditCtrl.SetSel(-1, -1);
	m_EditCtrl.ReplaceSel(lpString);
}

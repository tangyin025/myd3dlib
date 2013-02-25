#include "StdAfx.h"
#include "resource.h"
#include "OutlinerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COutlinerView::SingleInstance * my::SingleInstance<COutlinerView>::s_ptr(NULL);

BEGIN_MESSAGE_MAP(COutlinerView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutlinerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_HIDE_INPLACE | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR1)
		|| !m_wndToolBar.LoadToolBar(IDR_TOOLBAR1, 0, 0, TRUE))
		return -1;

	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	if (!m_wndTreeCtrl.Create(WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, CRect(0,0,0,0), this, 4))
		return -1;

	return 0;
}

void COutlinerView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(&rectClient);
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndTreeCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

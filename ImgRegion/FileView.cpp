
#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "ChildFrm.h"
#include "ImgRegionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CImgRegionTreeCtrl::CImgRegionTreeCtrl(void)
	: m_bDrag(FALSE)
{
}

BEGIN_MESSAGE_MAP(CImgRegionTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CImgRegionTreeCtrl::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CImgRegionTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	SetCapture();
	m_bDrag = TRUE;
	*pResult = 0;
}

void CImgRegionTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CImgRegionTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	m_bDrag = FALSE;
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY_RANGE(TVN_SELCHANGED, 4, 400, &CFileView::OnTvnSelchangedTree)
END_MESSAGE_MAP()

CFileView::CFileView()
	: m_pDoc(NULL)
{
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	if (!m_TreeCtrlSet.empty())
	{
		int nCtrlHeight = rectClient.Height() / m_TreeCtrlSet.size();
		int y = 0;
		std::set<CTreeCtrl *>::iterator tree_iter = m_TreeCtrlSet.begin();
		for(; tree_iter != m_TreeCtrlSet.end(); tree_iter++)
		{
			(*tree_iter)->SetWindowPos(
				NULL,
				rectClient.left,
				rectClient.top + y,
				rectClient.Width(),
				nCtrlHeight, SWP_NOACTIVATE | SWP_NOZORDER);
			y += nCtrlHeight;
		}
	}
}

void CFileView::OnChangeVisualStyle()
{
}

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	OnChangeVisualStyle();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFileView::OnPaint()
{
	CPaintDC dc(this);
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}

void CFileView::OnIdleUpdate()
{
}

afx_msg void CFileView::OnTvnSelchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	CChildFrame * pChildFrame = DYNAMIC_DOWNCAST(CChildFrame, pFrame->MDIGetActive());
	if(pChildFrame)
	{
		CImgRegionDoc * pDoc = DYNAMIC_DOWNCAST(CImgRegionDoc, pChildFrame->GetActiveDocument());
		if(pDoc)
		{
			pDoc->UpdateAllViews(NULL);

			pFrame->m_wndProperties.InvalidProperties();
		}
	}
	*pResult = 0;
}

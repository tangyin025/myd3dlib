
#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "ChildFrm.h"
#include "ImgRegionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CFileView::OnIdleUpdateCmdUI)
	ON_NOTIFY_RANGE(TVN_SELCHANGED, 4, 1000, &CFileView::OnTvnSelchangedTree)
	ON_NOTIFY_RANGE(CDragableTreeCtrl::TVN_DRAGCHANGED, 4, 1000, &CFileView::OnTvnDragchangedTree)
END_MESSAGE_MAP()

CFileView::CFileView()
	: m_pDoc(NULL)
	, m_bIsLayoutInvalid(FALSE)
{
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	if(pFrame)
	{
		CChildFrame * pChildFrame = DYNAMIC_DOWNCAST(CChildFrame, pFrame->MDIGetActive());
		if(pChildFrame)
		{
			CImgRegionDoc * pDoc = DYNAMIC_DOWNCAST(CImgRegionDoc, pChildFrame->GetActiveDocument());
			if(pDoc && pDoc->m_TreeCtrl.m_hWnd)
			{
				CRect rectClient;
				GetClientRect(rectClient);

				pDoc->m_TreeCtrl.SetWindowPos(
					&wndTop, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), 0);

				pDoc->m_TreeCtrl.Invalidate();
			}
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

BOOL CFileView::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(&rectClient, GetSysColor(COLOR_WINDOW));

	return TRUE;
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}

LRESULT CFileView::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if(m_bIsLayoutInvalid)
	{
		AdjustLayout();

		m_bIsLayoutInvalid = FALSE;
	}
	return 0;
}

afx_msg void CFileView::OnTvnSelchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW ptv = (LPNMTREEVIEW)pNMHDR;
	if(ptv->action != TVC_UNKNOWN)
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
	}
}

afx_msg void CFileView::OnTvnDragchangedTree(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	CDragableTreeCtrl::NMTREEVIEWDRAG * pDragInfo = reinterpret_cast<CDragableTreeCtrl::NMTREEVIEWDRAG *>(pNMHDR);
	ASSERT(pDragInfo);

	CDragableTreeCtrl * pTreeCtrl = DYNAMIC_DOWNCAST(CDragableTreeCtrl, GetDlgItem(id));
	ASSERT(pTreeCtrl);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	CChildFrame * pChildFrame = DYNAMIC_DOWNCAST(CChildFrame, pFrame->MDIGetActive());
	if(pChildFrame)
	{
		CImgRegionDoc * pDoc = DYNAMIC_DOWNCAST(CImgRegionDoc, pChildFrame->GetActiveDocument());
		if(pDoc && &pDoc->m_TreeCtrl == pTreeCtrl)
		{
			if(pDoc->CanItemMove(pDragInfo->hDragTagParent, pDragInfo->hDragTagFront, pDragInfo->hDragItem))
			{
				UINT newParentID = pDragInfo->hDragTagParent ? pDoc->GetItemId(pDragInfo->hDragTagParent) : 0;
				UINT newBeforeID = pDragInfo->hDragTagFront ? pDoc->GetItemId(pDragInfo->hDragTagFront) : 0;
				HistoryPtr hist(new HistoryMovRegion(
					pDoc, pDoc->GetItemId(pDragInfo->hDragItem), newParentID, newBeforeID));

				pDoc->AddNewHistory(hist);
				hist->Do();

				pDoc->UpdateAllViews(NULL);

				pDoc->SetModifiedFlag();

				pFrame->m_wndProperties.InvalidProperties();
			}
			else
				MessageBox(_T("无法移动节点"));
		}
	}
}

BOOL CFileView::PreTranslateMessage(MSG* pMsg)
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	CChildFrame * pChildFrame = DYNAMIC_DOWNCAST(CChildFrame, pFrame->MDIGetActive());
	if(pChildFrame)
	{
		CImgRegionDoc * pDoc = DYNAMIC_DOWNCAST(CImgRegionDoc, pChildFrame->GetActiveDocument());
		if(pDoc)
		{
			switch(pMsg->message)
			{
			case WM_KEYDOWN:
				switch(pMsg->wParam)
				{
				case VK_INSERT:
					pDoc->OnAddRegion();
					return TRUE;

				case VK_DELETE:
					pDoc->OnDelRegion();
					return TRUE;
				}
				break;
			}
		}
	}

	return CDockablePane::PreTranslateMessage(pMsg);
}


#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "ChildFrm.h"
#include "ImgRegionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TVN_DRAGCHANGED (TVN_LAST + 1)

struct NMTREEVIEWDRAG
{
	NMHDR hdr;
	HTREEITEM hDragItem;
	HTREEITEM hDragTagParent;
	HTREEITEM hDragTagFront;
};

IMPLEMENT_DYNAMIC(CImgRegionTreeCtrl, CTreeCtrl)

CImgRegionTreeCtrl::CImgRegionTreeCtrl(void)
	: m_bDrag(FALSE)
	, m_hDragItem(NULL)
	, m_DragDropType(DropTypeNone)
{
}

BEGIN_MESSAGE_MAP(CImgRegionTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CImgRegionTreeCtrl::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CImgRegionTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	TVHITTESTINFO info;
	info.pt = pNMTreeView->ptDrag;
	HitTest(&info);
	if((info.flags & TVHT_ONITEM) && info.hItem)
	{
		SetCapture();
		m_bDrag = TRUE;
		m_hDragItem = info.hItem;
		m_DragDropType = DropTypeNone;
	}
	*pResult = 0;
}

void CImgRegionTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bDrag)
	{
		TVHITTESTINFO info;
		info.pt = point;
		HitTest(&info);
		if(info.flags & TVHT_TOLEFT)
		{
			SetInsertMark(NULL);
			SelectDropTarget(NULL);
			m_DragDropType = DropTypeNone;

			if(point.x < m_LastDragPos.x)
				SendMessage(WM_HSCROLL, SB_LINEUP, 0);
		}
		else if(info.flags & TVHT_TORIGHT)
		{
			SetInsertMark(NULL);
			SelectDropTarget(NULL);
			m_DragDropType = DropTypeNone;

			if(point.x > m_LastDragPos.x)
				SendMessage(WM_HSCROLL, SB_LINEDOWN, 0);
		}
		else if(info.flags & TVHT_ABOVE)
		{
			if(point.y < m_LastDragPos.y)
				SendMessage(WM_VSCROLL, SB_LINEUP, 0);

			HTREEITEM hFirst = GetFirstVisibleItem();
			if(hFirst)
			{
				CRect rectItem;
				GetItemRect(hFirst, &rectItem, FALSE);
				if(point.y < rectItem.top)
				{
					SetInsertMark(hFirst, FALSE);
					SelectDropTarget(hFirst);
					m_DragDropType = DropTypeFront;
				}
			}
		}
		else if(info.flags & TVHT_BELOW)
		{
			if(point.y > m_LastDragPos.y)
				SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);

			HTREEITEM hLast = GetLastVisibleItem();
			if(hLast)
			{
				CRect rectItem;
				GetItemRect(hLast, &rectItem, FALSE);
				if(point.y >= rectItem.bottom)
				{
					SetInsertMark(hLast, TRUE);
					SelectDropTarget(hLast);
					m_DragDropType = DropTypeBack;
				}
			}
		}
		else if((info.flags & (TVHT_ONITEM | TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT)) && info.hItem)
		{
			CRect rectItem;
			GetItemRect(info.hItem, &rectItem, FALSE);

			int nBorder = rectItem.Height() / 3;
			if(point.y < rectItem.top + nBorder)
			{
				SetInsertMark(info.hItem, FALSE);
				SelectDropTarget(info.hItem);
				m_DragDropType = DropTypeFront;
			}
			else if(point.y >= rectItem.bottom - nBorder)
			{
				SetInsertMark(info.hItem, TRUE);
				SelectDropTarget(info.hItem);
				m_DragDropType = DropTypeBack;
			}
			else
			{
				SetInsertMark(NULL);
				SelectDropTarget(info.hItem);
				m_DragDropType = DropTypeChild;
			}
		}
		else
		{
			HTREEITEM hLast = GetLastVisibleItem();
			if(hLast)
			{
				CRect rectItem;
				GetItemRect(hLast, &rectItem, FALSE);
				if(point.y >= rectItem.bottom)
				{
					SetInsertMark(hLast, TRUE);
					SelectDropTarget(hLast);
					m_DragDropType = DropTypeBack;
				}
			}
		}

		m_LastDragPos = point;
	}
	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CImgRegionTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bDrag)
	{
		ReleaseCapture();
		m_bDrag = FALSE;

		NMTREEVIEWDRAG dragInfo;
		dragInfo.hdr.hwndFrom = GetSafeHwnd();
		dragInfo.hdr.idFrom = GetDlgCtrlID();
		dragInfo.hdr.code = TVN_DRAGCHANGED;
		dragInfo.hDragItem = m_hDragItem;

		HTREEITEM hDropItem = GetDropHilightItem();
		SetInsertMark(NULL);
		SelectDropTarget(NULL);

		switch(m_DragDropType)
		{
		case DropTypeFront:
			ASSERT(hDropItem);
			dragInfo.hDragTagParent = GetSafeParentItem(hDropItem);
			dragInfo.hDragTagFront = GetSafePreSiblingItem(hDropItem);
			GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
			break;

		case DropTypeChild:
			ASSERT(hDropItem);
			dragInfo.hDragTagParent = hDropItem;
			dragInfo.hDragTagFront = TVI_LAST;
			GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
			break;

		case DropTypeBack:
			ASSERT(hDropItem);
			dragInfo.hDragTagParent = GetSafeParentItem(hDropItem);
			dragInfo.hDragTagFront = hDropItem;
			GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
			break;
		}
	}
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

BOOL CImgRegionTreeCtrl::FindTreeChildItem(HTREEITEM hParent, HTREEITEM hChild)
{
	if(hParent == hChild)
		return TRUE;

	HTREEITEM hItem = GetChildItem(hParent);
	for(; hItem; hItem = GetNextSiblingItem(hItem))
	{
		if(FindTreeChildItem(hItem, hChild))
			return TRUE;
	}

	return FALSE;
}

HTREEITEM CImgRegionTreeCtrl::MoveTreeItem(HTREEITEM hParent, HTREEITEM hInsertAfter, HTREEITEM hOtherItem)
{
	if(hParent == hOtherItem || hInsertAfter == hOtherItem || FindTreeChildItem(hOtherItem, hParent))
	{
		return hOtherItem;
	}

	HTREEITEM hItem = InsertItem(GetItemText(hOtherItem), 0, 0, hParent, hInsertAfter);
	SetItemData(hItem, GetItemData(hOtherItem));

	HTREEITEM hNextOtherChild = NULL;
	HTREEITEM hChild = TVI_LAST;
	for(HTREEITEM hOtherChild = GetChildItem(hOtherItem); hOtherChild; hOtherChild = hNextOtherChild)
	{
		HTREEITEM hNextOtherChild = GetNextSiblingItem(hOtherChild);

		hChild = MoveTreeItem(hItem, hChild, hOtherChild);
	}

	Expand(hItem, TVE_EXPAND);

	DeleteItem(hOtherItem);
	return hItem;
}

HTREEITEM CImgRegionTreeCtrl::GetSafeParentItem(HTREEITEM hItem)
{
	HTREEITEM hParent = GetParentItem(hItem);
	if(!hParent)
		return TVI_ROOT;

	return hParent;
}

HTREEITEM CImgRegionTreeCtrl::GetSafePreSiblingItem(HTREEITEM hItem)
{
	HTREEITEM hSibling = GetPrevSiblingItem(hItem);
	if(!hSibling)
		return TVI_FIRST;

	return hSibling;
}

void CImgRegionTreeCtrl::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CTreeCtrl::OnTimer(nIDEvent);
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_NOTIFY_RANGE(TVN_SELCHANGED, 4, 1000, &CFileView::OnTvnSelchangedTree)
	ON_NOTIFY_RANGE(TVN_DRAGCHANGED, 4, 1000, &CFileView::OnTvnDragchangedTree)
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

void CFileView::OnIdleUpdate()
{
	if(m_bIsLayoutInvalid)
	{
		AdjustLayout();

		m_bIsLayoutInvalid = FALSE;
	}
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
	NMTREEVIEWDRAG * pDragInfo = reinterpret_cast<NMTREEVIEWDRAG *>(pNMHDR);
	ASSERT(pDragInfo);

	CImgRegionTreeCtrl * pTreeCtrl = DYNAMIC_DOWNCAST(CImgRegionTreeCtrl, GetDlgItem(id));
	ASSERT(pTreeCtrl);

	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);

	CChildFrame * pChildFrame = DYNAMIC_DOWNCAST(CChildFrame, pFrame->MDIGetActive());
	if(pChildFrame)
	{
		CImgRegionDoc * pDoc = DYNAMIC_DOWNCAST(CImgRegionDoc, pChildFrame->GetActiveDocument());
		if(pDoc && &pDoc->m_TreeCtrl == pTreeCtrl)
		{
			CPoint ptOrg = pDoc->LocalToRoot(pDragInfo->hDragItem, CPoint(0,0));

			HTREEITEM hItem = pTreeCtrl->MoveTreeItem(pDragInfo->hDragTagParent, pDragInfo->hDragTagFront, pDragInfo->hDragItem);
			pTreeCtrl->SelectItem(hItem);

			if(hItem != pDragInfo->hDragItem)
			{
				CImgRegion * pReg = (CImgRegion *)pTreeCtrl->GetItemData(hItem);
				ASSERT(pReg);

				pReg->m_Local = pDoc->RootToLocal(pDragInfo->hDragTagParent, ptOrg);
			}

			pDoc->UpdateAllViews(NULL);

			pDoc->SetModifiedFlag();

			pFrame->m_wndProperties.InvalidProperties();
		}
	}
}

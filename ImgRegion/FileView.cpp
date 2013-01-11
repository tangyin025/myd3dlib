
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
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CImgRegionTreeCtrl::OnNMCustomdraw)
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
			dragInfo.hDragTagParent = GetParentItem(hDropItem);
			dragInfo.hDragTagFront = GetPrevSiblingItem(hDropItem);
			GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
			break;

		case DropTypeChild:
			ASSERT(hDropItem);
			dragInfo.hDragTagParent = hDropItem;
			dragInfo.hDragTagFront = NULL;
			GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
			break;

		case DropTypeBack:
			ASSERT(hDropItem);
			dragInfo.hDragTagParent = GetParentItem(hDropItem);
			dragInfo.hDragTagFront = hDropItem;
			GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dragInfo);
			break;
		}
	}
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

HTREEITEM CImgRegionTreeCtrl::InsertItem(LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	std::wstring key(lpszItem);
	ASSERT(m_ItemMap.find(key) == m_ItemMap.end());

	HTREEITEM hItem;
	m_ItemMap[key] = hItem = CTreeCtrl::InsertItem(lpszItem, hParent, hInsertAfter);
	return hItem;
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

	std::wstring key(GetItemText(hOtherItem));
	int nImage, nSelectedImage;
	GetItemImage(hOtherItem, nImage, nSelectedImage);
	HTREEITEM hItem = CTreeCtrl::InsertItem(key.c_str(), nImage, nSelectedImage, hParent, hInsertAfter);
	SetItemData(hItem, GetItemData(hOtherItem));

	HTREEITEM hNextOtherChild = NULL;
	HTREEITEM hChild = TVI_LAST;
	for(HTREEITEM hOtherChild = GetChildItem(hOtherItem); hOtherChild; hOtherChild = hNextOtherChild)
	{
		hNextOtherChild = GetNextSiblingItem(hOtherChild);

		hChild = MoveTreeItem(hItem, hChild, hOtherChild);
	}

	Expand(hItem, TVE_EXPAND);

	DeleteItem(hOtherItem);

	ASSERT(m_ItemMap.find(key) != m_ItemMap.end());
	return m_ItemMap[key] = hItem;
}

void CImgRegionTreeCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	// http://stackoverflow.com/questions/2119717/changing-the-color-of-a-selected-ctreectrl-item
    NMTVCUSTOMDRAW *pcd = (NMTVCUSTOMDRAW   *)pNMHDR;
    switch ( pcd->nmcd.dwDrawStage )
    {
    case CDDS_PREPAINT: 
        *pResult = CDRF_NOTIFYITEMDRAW;     
        break;

    case CDDS_ITEMPREPAINT : 
        {
            HTREEITEM   hItem = (HTREEITEM)pcd->nmcd.dwItemSpec;

            if ( GetSelectedItem() == hItem )
            {
                pcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);    
                pcd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
            }

            *pResult = CDRF_DODEFAULT;// do not set *pResult = CDRF_SKIPDEFAULT
            break;
        }
    }
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
			CString newParentID = pDragInfo->hDragTagParent ? pDoc->m_TreeCtrl.GetItemText(pDragInfo->hDragTagParent) : _T("");
			CString newBeforeID = pDragInfo->hDragTagFront ? pDoc->m_TreeCtrl.GetItemText(pDragInfo->hDragTagFront) : _T("");
			HistoryPtr hist(new HistoryMovRegion(
				pDoc, pDoc->m_TreeCtrl.GetItemText(pDragInfo->hDragItem), newParentID, newBeforeID));

			pDoc->AddNewHistory(hist);
			hist->Do();

			pDoc->UpdateAllViews(NULL);

			pDoc->SetModifiedFlag();

			pFrame->m_wndProperties.InvalidProperties();
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

#include "StdAfx.h"
#include "resource.h"
#include "DragableTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDragableTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CDragableTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CDragableTreeCtrl::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CDragableTreeCtrl::OnNMCustomdraw)
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CDragableTreeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}

void CDragableTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
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

void CDragableTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
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

void CDragableTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
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

int CDragableTreeCtrl::CalcChildCount(HTREEITEM hItem)
{
	int nChilds = 0;
	for(HTREEITEM hChild = GetChildItem(hItem);
		hChild; hChild = GetNextSiblingItem(hChild))
		nChilds++;

	return nChilds;
}

HTREEITEM CDragableTreeCtrl::CalcLastChildItem(HTREEITEM hItem)
{
	HTREEITEM hLast = NULL;
	for(HTREEITEM hChild = GetChildItem(hItem);
		hChild; hChild = GetNextSiblingItem(hChild))
		hLast = hChild;

	return hLast;
}

void CDragableTreeCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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

void CDragableTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_DELETE:
		{
			HTREEITEM hSelected = GetSelectedItem();
			if(hSelected)
			{
				NMTREEVIEW NMTreeView = {0};
				NMTreeView.hdr.hwndFrom = GetSafeHwnd();
				NMTreeView.hdr.idFrom = GetDlgCtrlID();
				NMTreeView.hdr.code = TVN_USERDELETING;
				NMTreeView.itemOld.hItem = hSelected;
				GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&NMTreeView);
			}
		}
		break;

	default:
		CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}

BOOL CDragableTreeCtrl::FindTreeChildItem(HTREEITEM hParent, HTREEITEM hChild)
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

void CDragableTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (point != CPoint(-1, -1))
	{
		UINT flags = 0;
		HTREEITEM hTreeItem = HitTest(point, &flags);
		if (hTreeItem != NULL)
		{
			SelectItem(hTreeItem);
		}
	}

	SetFocus();

	//CTreeCtrl::OnRButtonDown(nFlags, point);
}

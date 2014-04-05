#include "StdAfx.h"
#include "resource.h"
#include "OutlinerView.h"
#include "MainFrm.h"
#include "MainDoc.h"
#include "MainView.h"

using namespace my;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TVN_DRAGCHANGED		(TVN_LAST + 1)
#define TVN_USERDELETING	(TVN_LAST + 2)

struct NMTREEVIEWDRAG
{
	NMHDR hdr;
	HTREEITEM hDragItem;
	HTREEITEM hDragTagParent;
	HTREEITEM hDragTagFront;
};

BEGIN_MESSAGE_MAP(COutlinerTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &COutlinerTreeCtrl::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &COutlinerTreeCtrl::OnNMCustomdraw)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

BOOL COutlinerTreeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
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

void COutlinerTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
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

void COutlinerTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
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

void COutlinerTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
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

int COutlinerTreeCtrl::CalcChildCount(HTREEITEM hItem)
{
	int nChilds = 0;
	for(HTREEITEM hChild = GetChildItem(hItem);
		hChild; hChild = GetNextSiblingItem(hChild))
		nChilds++;

	return nChilds;
}

HTREEITEM COutlinerTreeCtrl::CalcLastChildItem(HTREEITEM hItem)
{
	HTREEITEM hLast = NULL;
	for(HTREEITEM hChild = GetChildItem(hItem);
		hChild; hChild = GetNextSiblingItem(hChild))
		hLast = hChild;

	return hLast;
}

void COutlinerTreeCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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

void COutlinerTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

BEGIN_MESSAGE_MAP(COutlinerView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, 4, &COutlinerView::OnTvnSelchangedTree)
	ON_NOTIFY(TVN_DRAGCHANGED, 4, &COutlinerView::OnTvnDragchangedTree)
	ON_NOTIFY(TVN_DELETEITEM, 4, &COutlinerView::OnTvnDeleteitem)
	ON_NOTIFY(TVN_BEGINLABELEDIT, 4, &COutlinerView::OnTvnBeginlabeledit)
	ON_NOTIFY(TVN_ENDLABELEDIT, 4, &COutlinerView::OnTvnEndlabeledit)
	ON_NOTIFY(TVN_USERDELETING, 4, &COutlinerView::OnTvnUserDeleting)
END_MESSAGE_MAP()

int COutlinerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_TreeCtrl.Create(WS_CHILD | WS_VISIBLE | TVS_EDITLABELS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, CRect(10,10,100,100), this, 4))
		return -1;

	if (!m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_TOOLBAR1) || !m_wndToolBar.LoadToolBar(IDR_TOOLBAR1, 0, 0, TRUE))
		return -1;

	return 0;
}

void COutlinerView::AdjustLayout(void)
{
	if (GetSafeHwnd())
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

		m_wndToolBar.SetWindowPos(
			NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

		m_TreeCtrl.SetWindowPos(
			NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

BOOL COutlinerView::PreTranslateMessage(MSG* pMsg)
{
	// http://support.microsoft.com/kb/167960/en-us
	if (pMsg->message == WM_KEYDOWN &&  
		pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)  
	{  
		CEdit * pEdit = m_TreeCtrl.GetEditControl();  
		if (pEdit)  
		{  
			pEdit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);  
			return TRUE;  
		}  
	}  

	return CDockablePane::PreTranslateMessage(pMsg);
}

void COutlinerView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void COutlinerView::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW ptv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if(/*ptv->action != TVC_UNKNOWN &&*/ ptv->itemNew.hItem)
	{
		CMainView::getSingleton().SendMessage(WM_UPDATE_PIVOTCONTROLLER);
	}
}

void COutlinerView::OnTvnDragchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTREEVIEWDRAG * pTVDragInfo = reinterpret_cast<NMTREEVIEWDRAG *>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void COutlinerView::OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	ItemDataType * data = (ItemDataType *)m_TreeCtrl.GetItemData(pNMTreeView->itemOld.hItem);
	ASSERT(data);

	ASSERT(m_ItemMap.end() != m_ItemMap.find(data->first));
	m_ItemMap.erase(data->first);
	delete data;

	*pResult = 0;
}

void COutlinerView::OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void COutlinerView::OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	// ! 需要由HistoryMgr托管
	ASSERT(pTVDispInfo->item.hItem);
	m_TreeCtrl.SetItemText(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText);
	CMainDoc * pDoc = CMainDoc::getSingletonPtr();
	ASSERT(pDoc);
	pDoc->SetModifiedFlag();
}

void COutlinerView::OnTvnUserDeleting(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	ASSERT(pNMTreeView->itemOld.hItem);
	CMainDoc * pDoc = CMainDoc::getSingletonPtr();
	ASSERT(pDoc);
	pDoc->DeleteTreeNode(pNMTreeView->itemOld.hItem);
	pDoc->SetModifiedFlag();
	pDoc->UpdateAllViews(NULL);
}

HTREEITEM COutlinerView::InsertItem(UINT id, const std::basic_string<TCHAR> & strItem, TreeNodeBasePtr node, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	ASSERT(m_ItemMap.end() == m_ItemMap.find(id));

	HTREEITEM hItem = m_TreeCtrl.InsertItem(strItem.c_str(), hParent, hInsertAfter);
	m_TreeCtrl.SetItemData(hItem, (DWORD_PTR) new ItemDataType(id, node));
	m_ItemMap[id] = hItem;
	return hItem;
}

UINT COutlinerView::GetItemId(HTREEITEM hItem)
{
	return ((ItemDataType *)m_TreeCtrl.GetItemData(hItem))->first;
}

TreeNodeBasePtr COutlinerView::GetItemNode(HTREEITEM hItem)
{
	return ((ItemDataType *)m_TreeCtrl.GetItemData(hItem))->second;
}

TreeNodeBasePtr COutlinerView::GetSelectedNode(void)
{
	HTREEITEM hSelected = m_TreeCtrl.GetSelectedItem();
	return hSelected ? GetItemNode(hSelected) : TreeNodeBasePtr();
}

void COutlinerView::DrawItemNode(IDirect3DDevice9 * pd3dDevice, float fElapsedTime, HTREEITEM hItem, const Matrix4 & World)
{
	_ASSERT(hItem);

	TreeNodeBasePtr node = GetItemNode(hItem);

	node->Draw(pd3dDevice, fElapsedTime, World);

	if(m_TreeCtrl.GetNextSiblingItem(hItem))
	{
		DrawItemNode(pd3dDevice, fElapsedTime, m_TreeCtrl.GetNextSiblingItem(hItem), World);
	}

	if(m_TreeCtrl.GetChildItem(hItem))
	{
		DrawItemNode(pd3dDevice, fElapsedTime, m_TreeCtrl.GetChildItem(hItem), Matrix4::Compose(node->m_Scale, node->m_Rotation, node->m_Position) * World);
	}
}

bool COutlinerView::RayTestItemNode(const std::pair<my::Vector3, my::Vector3> & ray, HTREEITEM hItem, const my::Matrix4 & World)
{
	_ASSERT(hItem);

	TreeNodeBasePtr node = GetItemNode(hItem);

	if(node->RayTest(ray, World))
	{
		m_TreeCtrl.SelectItem(hItem);
		return true;
	}

	if(m_TreeCtrl.GetNextSiblingItem(hItem) && RayTestItemNode(ray, m_TreeCtrl.GetNextSiblingItem(hItem), World))
	{
		return true;
	}

	if(m_TreeCtrl.GetChildItem(hItem) && RayTestItemNode(ray, m_TreeCtrl.GetChildItem(hItem), Matrix4::Compose(node->m_Scale, node->m_Rotation, node->m_Position) * World))
	{
		return true;
	}
	return false;
}

void COutlinerView::SerializeSubItemRecursively(CArchive & ar, HTREEITEM hParent)
{
	ASSERT(hParent);

	if(ar.IsStoring())
	{
		int nChilds = m_TreeCtrl.CalcChildCount(hParent);
		ar << nChilds;
		HTREEITEM hItem = m_TreeCtrl.GetChildItem(hParent);
		for(; hItem; hItem = m_TreeCtrl.GetNextSiblingItem(hItem))
		{
			UINT id = GetItemId(hItem);
			ar << id;
			CString szItem = m_TreeCtrl.GetItemText(hItem);
			ar << szItem;
			CObject * pNode = GetItemNode(hItem).get();
			ar << pNode;
			SerializeSubItemRecursively(ar, hItem);
		}
	}
	else
	{
		int nChilds;
		ar >> nChilds;
		for(int i = 0; i < nChilds; i++)
		{
			UINT id;
			ar >> id;
			CString szItem;
			ar >> szItem;
			CObject * pNode;
			ar >> pNode;
			SerializeSubItemRecursively(ar, InsertItem(id, (LPCTSTR)szItem, TreeNodeBasePtr(DYNAMIC_DOWNCAST(TreeNodeBase, pNode)), hParent, TVI_LAST));
		}
	}
}

void COutlinerView::Serialize(CArchive & ar)
{
	SerializeSubItemRecursively(ar, TVI_ROOT);
}

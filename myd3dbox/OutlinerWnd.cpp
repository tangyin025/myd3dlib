#include "StdAfx.h"
#include "OutlinerWnd.h"
#include "MainApp.h"
#include "../demo2_3/Component/Actor.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_MENU_START				(40000L)
#define ID_MENU_ADD_MESH_COMPONENT	(40001L)

BEGIN_MESSAGE_MAP(CTC, CMultiTree)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CTC::OnNMCustomdraw)
END_MESSAGE_MAP()

void CTC::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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

            if ( IsSelected(hItem) )
            {
                pcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);    
                pcd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
            }

            *pResult = CDRF_DODEFAULT;// do not set *pResult = CDRF_SKIPDEFAULT
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COutlinerWnd::COutlinerWnd()
{
}

COutlinerWnd::~COutlinerWnd()
{
}

BEGIN_MESSAGE_MAP(COutlinerWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, 2, &COutlinerWnd::OnTvnSelchangedTree)
	ON_COMMAND(ID_MENU_ADD_MESH_COMPONENT, &COutlinerWnd::OnMenuAddMeshComponent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutlinerWnd message handlers

int COutlinerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	OnChangeVisualStyle();

	//HTREEITEM hRoot = m_wndClassView.InsertItem(_T("FakeApp classes"), 0, 0);
	//m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	//HTREEITEM hClass = m_wndClassView.InsertItem(_T("CFakeAboutDlg"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAboutDlg()"), 3, 3, hClass);

	//m_wndClassView.Expand(hRoot, TVE_EXPAND);

	//hClass = m_wndClassView.InsertItem(_T("CFakeApp"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeApp()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("InitInstance()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("OnAppAbout()"), 3, 3, hClass);

	//hClass = m_wndClassView.InsertItem(_T("CFakeAppDoc"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAppDoc()"), 4, 4, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppDoc()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("OnNewDocument()"), 3, 3, hClass);

	//hClass = m_wndClassView.InsertItem(_T("CFakeAppView"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAppView()"), 4, 4, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppView()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("GetDocument()"), 3, 3, hClass);
	//m_wndClassView.Expand(hClass, TVE_EXPAND);

	//hClass = m_wndClassView.InsertItem(_T("CFakeAppFrame"), 1, 1, hRoot);
	//m_wndClassView.InsertItem(_T("CFakeAppFrame()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppFrame()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("m_wndMenuBar"), 6, 6, hClass);
	//m_wndClassView.InsertItem(_T("m_wndToolBar"), 6, 6, hClass);
	//m_wndClassView.InsertItem(_T("m_wndStatusBar"), 6, 6, hClass);

	//hClass = m_wndClassView.InsertItem(_T("Globals"), 2, 2, hRoot);
	//m_wndClassView.InsertItem(_T("theFakeApp"), 5, 5, hClass);
	//m_wndClassView.Expand(hClass, TVE_EXPAND);

	return 0;
}

void COutlinerWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void COutlinerWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndClassView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	HTREEITEM hItem = m_wndClassView.GetSelectedItem();
	if (!hItem)
		return;

	if (m_ContextMenu.m_hMenu)
		m_ContextMenu.DestroyMenu();

	if (m_ContextMenuAdd.m_hMenu)
		m_ContextMenuAdd.DestroyMenu();

	m_ContextMenu.CreatePopupMenu();

	//TreeItemData * pItemData = GetTreeItemData(hItem);
	//ASSERT(pItemData);
	//if (pItemData->Type == TreeItemTypeActor)
	//{
	//	m_ContextMenuAdd.CreatePopupMenu();
	//	m_ContextMenuAdd.AppendMenu(MF_STRING, ID_MENU_ADD_MESH_COMPONENT, _T("Mesh Component"));
	//	m_ContextMenu.AppendMenu(MF_POPUP, (UINT_PTR)m_ContextMenuAdd.operator HMENU(), _T("Add"));
	//}

	if(m_ContextMenu.GetMenuItemCount() == 0)
	{
		m_ContextMenu.AppendMenu(MF_DISABLED, ID_MENU_START, _T("Empty"));
	}

	CPoint ptMenu = point;
	ScreenToClient(&point);
	m_ContextMenu.TrackPopupMenu(0, ptMenu.x, ptMenu.y, this);
}

void COutlinerWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = 0;
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}
//
//BOOL COutlinerWnd::CanTreeItemMove(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
//{
//	if(hParent == hMoveItem || hInsertAfter == hMoveItem || m_wndClassView.FindTreeChildItem(hMoveItem, hParent))
//	{
//		return FALSE;
//	}
//
//	return TRUE;
//}

HTREEITEM COutlinerWnd::InsertTreeItem(LPCTSTR lpszItem, DWORD_PTR pData, int nImage, int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	ASSERT(m_Data2HTree.find(pData) == m_Data2HTree.end());
	HTREEITEM hItem = m_wndClassView.InsertItem(lpszItem, nImage, nSelectedImage, hParent, hInsertAfter);
	ASSERT(hItem);
	m_wndClassView.SetItemData(hItem, pData);
	m_Data2HTree.insert(std::make_pair(pData, hItem));
	return hItem;
}

void COutlinerWnd::DeleteTreeItem(HTREEITEM hItem)
{
	ASSERT(hItem);
	DWORD_PTR pData = m_wndClassView.GetItemData(hItem);
	ASSERT(pData);
	m_Data2HTree.erase(pData);

	HTREEITEM hNextChild = NULL;
	for(HTREEITEM hChild = m_wndClassView.GetChildItem(hItem); NULL != hChild; hChild = hNextChild)
	{
		hNextChild = m_wndClassView.GetNextSiblingItem(hChild);
		DeleteTreeItem(hChild);
	}

	m_wndClassView.DeleteItem(hItem);
}

void COutlinerWnd::DeleteAllTreeItems(void)
{
	m_wndClassView.DeleteAllItems();
	m_Data2HTree.clear();
}
//
//HTREEITEM COutlinerWnd::MoveTreeItem(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
//{
//	if(!CanTreeItemMove(hParent, hInsertAfter, hMoveItem))
//	{
//		return NULL;
//	}
//
//	int nImage, nSelectedImage;
//	m_wndClassView.GetItemImage(hMoveItem, nImage, nSelectedImage);
//	HTREEITEM hItem = m_wndClassView.InsertItem(m_wndClassView.GetItemText(hMoveItem), nImage, nSelectedImage, hParent, hInsertAfter);
//	m_wndClassView.SetItemData(hItem, m_wndClassView.GetItemData(hMoveItem));
//
//	HTREEITEM hNextOtherChild = NULL;
//	HTREEITEM hChild = TVI_LAST;
//	for(HTREEITEM hOtherChild = m_wndClassView.GetChildItem(hMoveItem); hOtherChild; hOtherChild = hNextOtherChild)
//	{
//		hNextOtherChild = m_wndClassView.GetNextSiblingItem(hOtherChild);
//		hChild = MoveTreeItem(hItem, hChild, hOtherChild);
//	}
//
//	m_wndClassView.DeleteItem(hMoveItem);
//	return hItem;
//}

void COutlinerWnd::InsertComponent(Component * cmp, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	HTREEITEM hItem = InsertTreeItem(_T("component"), (DWORD_PTR)cmp, 1, 1, hParent, hInsertAfter);
}

HTREEITEM COutlinerWnd::GetTreeItemByData(DWORD_PTR pData)
{
	Data2HTreeMap::const_iterator item_iter = m_Data2HTree.find(pData);
	if (item_iter != m_Data2HTree.end())
	{
		return item_iter->second;
	}
	return NULL;
}

BOOL COutlinerWnd::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void COutlinerWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void COutlinerWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndClassView.SetFocus();
}

void COutlinerWnd::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);
}

void COutlinerWnd::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMainFrame::getSingleton().m_EventSelectChanged();
}

void COutlinerWnd::OnMenuAddMeshComponent()
{
	// TODO: Add your command handler code here
}

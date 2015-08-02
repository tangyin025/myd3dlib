#include "StdAfx.h"
#include "OutlinerWnd.h"
#include "MainApp.h"
#include "Component/Actor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CClassViewMenuButton : public CMFCToolBarMenuButton
{
	friend class COutlinerWnd;

	DECLARE_SERIAL(CClassViewMenuButton)

public:
	CClassViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CClassViewMenuButton, CMFCToolBarMenuButton, 1)

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
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

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

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
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

BOOL COutlinerWnd::CanTreeItemMove(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	if(hParent == hMoveItem || hInsertAfter == hMoveItem || m_wndClassView.FindTreeChildItem(hMoveItem, hParent))
	{
		return FALSE;
	}

	return TRUE;
}

HTREEITEM COutlinerWnd::InsertTreeItem(LPCTSTR lpszItem, DWORD type, void * data, int nImage, int nSelectedImage, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	HTREEITEM hItem = m_wndClassView.InsertItem(lpszItem, nImage, nSelectedImage, hParent, hInsertAfter);
	ASSERT(hItem);
	m_wndClassView.SetItemData(hItem, (DWORD_PTR)(new TreeItemData(type, data)));
	return hItem;
}

void COutlinerWnd::DeleteTreeItem(HTREEITEM hItem)
{
	ASSERT(hItem);
	delete (TreeItemData *)m_wndClassView.GetItemData(hItem);

	HTREEITEM hNextChild = NULL;
	for (HTREEITEM hChild = m_wndClassView.GetChildItem(hItem); hChild; hChild = hNextChild)
	{
		hNextChild = m_wndClassView.GetNextSiblingItem(hChild);
		DeleteTreeItem(hItem);
	}

	m_wndClassView.DeleteItem(hItem);
}

HTREEITEM COutlinerWnd::MoveTreeItem(HTREEITEM hMoveItem, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	if(!CanTreeItemMove(hParent, hInsertAfter, hMoveItem))
	{
		return NULL;
	}

	int nImage, nSelectedImage;
	m_wndClassView.GetItemImage(hMoveItem, nImage, nSelectedImage);
	HTREEITEM hItem = m_wndClassView.InsertItem(m_wndClassView.GetItemText(hMoveItem), nImage, nSelectedImage, hParent, hInsertAfter);
	m_wndClassView.SetItemData(hItem, m_wndClassView.GetItemData(hMoveItem));

	HTREEITEM hNextOtherChild = NULL;
	HTREEITEM hChild = TVI_LAST;
	for(HTREEITEM hOtherChild = m_wndClassView.GetChildItem(hMoveItem); hOtherChild; hOtherChild = hNextOtherChild)
	{
		hNextOtherChild = m_wndClassView.GetNextSiblingItem(hOtherChild);
		hChild = MoveTreeItem(hItem, hChild, hOtherChild);
	}

	m_wndClassView.DeleteItem(hMoveItem);
	return hItem;
}

void COutlinerWnd::InsertActor(Actor * actor, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	HTREEITEM hItem = InsertTreeItem(_T("actor"), TreeItemTypeActor, actor, 0, 0, hParent, hInsertAfter);

	struct CallBack : public my::IQueryCallback
	{
		COutlinerWnd * m_wnd;

		HTREEITEM m_hParent;

		HTREEITEM m_hInsertAfter;

		CallBack(COutlinerWnd * wnd, HTREEITEM hParent, HTREEITEM hInsertAfter)
			: m_wnd(wnd)
			, m_hParent(hParent)
			, m_hInsertAfter(hInsertAfter)
		{
		}

		void operator() (my::AABBComponent * comp, my::IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Component *>(comp));
			m_wnd->InsertComponent(static_cast<Component *>(comp), m_hParent, m_hInsertAfter);
		}
	};

	actor->QueryComponentAll(&CallBack(this, hItem, TVI_LAST));
}

void COutlinerWnd::InsertComponent(Component * cmp, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	HTREEITEM hItem = InsertTreeItem(_T("component"), TreeItemTypeComponent, cmp, 1, 1, hParent, hInsertAfter);
	m_Cmp2HTree[cmp] = hItem;
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


#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
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

	m_wndFileView.SetWindowPos(NULL,
		rectClient.left + 1,
		rectClient.top + 1,
		rectClient.Width() - 2,
		rectClient.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnChangeVisualStyle()
{
}

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndFileView.Create(WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, CRect(), this, 4))
	{
		TRACE0("未能创建文件视图\n");
		return -1;
	}

	OnChangeVisualStyle();

	AdjustLayout();

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

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnIdleUpdate()
{
	CMainFrame * pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT(pFrame);
	CChildFrame * pChildFrame = DYNAMIC_DOWNCAST(CChildFrame, pFrame->MDIGetActive());
	if(pChildFrame)
	{
		CImgRegionDoc * pDoc = DYNAMIC_DOWNCAST(CImgRegionDoc, pChildFrame->GetActiveDocument());
		if(pDoc)
		{
			if(pDoc != m_pDoc)
			{
				m_wndFileView.DeleteAllItems();

				m_pDoc = pDoc;
	
				InsertRegionNode(pDoc->m_root.get());
			}
			return;
		}
	}

	m_wndFileView.DeleteAllItems();
}

void CFileView::InsertRegionNode(const CImgRegionNode * node, HTREEITEM hParent)
{
	HTREEITEM hChild = m_wndFileView.InsertItem(node->m_name, 0, 0, hParent);

	CImgRegionNodePtrList::const_iterator reg_iter = node->m_childs.begin();
	for(; reg_iter != node->m_childs.end(); reg_iter++)
	{
		InsertRegionNode(reg_iter->get(), hChild);
	}

	m_wndFileView.Expand(hParent, TVE_EXPAND);
}

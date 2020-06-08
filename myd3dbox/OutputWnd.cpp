
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputEdit

COutputEdit::COutputEdit()
{
	theApp.m_EventLog.connect(boost::bind(&COutputEdit::OnEventLog, this, _1));
}

COutputEdit::~COutputEdit()
{
	theApp.m_EventLog.connect(boost::bind(&COutputEdit::OnEventLog, this, _1));
}

BEGIN_MESSAGE_MAP(COutputEdit, CRichEditCtrl)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	////ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputEdit::OnEventLog(const char * str)
{
	SendMessage(EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
	SendMessage(EM_REPLACESEL, 0, (LPARAM)ms2ts(str).c_str());
}

void COutputEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CRichEditCtrl::OnContextMenu(pWnd, point);
	//CMenu menu;
	//menu.LoadMenu(IDR_OUTPUT_POPUP);

	//CMenu* pSumMenu = menu.GetSubMenu(0);

	//if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	//{
	//	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

	//	if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
	//		return;

	//	((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
	//	UpdateDialogControls(this, FALSE);
	//}

	//SetFocus();
}

void COutputEdit::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}

void COutputEdit::OnEditClear()
{
	MessageBox(_T("Clear output"));
}

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	//// Create output panes:
	if (!m_wndOutputDebug.Create(LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE/* | ES_READONLY*/,
		CRect(0, 0, 100, 100), &m_wndTabs, 2))
	{
		TRACE0("Failed to create output windows\n");
		return -1;
	}

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	//// Attach list windows to tab:
	m_wndTabs.AddTab(&m_wndOutputDebug, _T("Debug"), (UINT)0);

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}
//
//void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
//{
//	CClientDC dc(this);
//	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);
//
//	int cxExtentMax = 0;
//
//	for (int i = 0; i < wndListBox.GetCount(); i ++)
//	{
//		CString strItem;
//		wndListBox.GetText(i, strItem);
//
//		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
//	}
//
//	wndListBox.SetHorizontalExtent(cxExtentMax);
//	dc.SelectObject(pOldFont);
//}

void COutputWnd::UpdateFonts()
{
	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
}

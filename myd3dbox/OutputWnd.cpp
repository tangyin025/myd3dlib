
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MainApp.h"
#include <boost/algorithm/string.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputEdit

COutputEdit::COutputEdit()
{
}

COutputEdit::~COutputEdit()
{
}

BEGIN_MESSAGE_MAP(COutputEdit, CRichEditCtrl)
	ON_WM_CONTEXTMENU()
	////ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CRichEditCtrl::OnContextMenu(pWnd, point);
}

BOOL COutputEdit::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		// ! rich edit VK_ESCAPE will destroy window
		return TRUE;
	}

	return CRichEditCtrl::PreTranslateMessage(pMsg);
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
	ON_WM_DESTROY()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &COutputWnd::OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create output panes:
	if (!m_wndOutputDebug.Create(LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
		CRect(0, 0, 100, 100), this, 2))
	{
		TRACE0("Failed to create output windows\n");
		return -1;
	}

	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);

	theApp.m_EventLog.connect(boost::bind(&COutputWnd::OnEventLog, this, boost::placeholders::_1));

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndOutputDebug.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
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

static std::list<std::basic_string<TCHAR> > dummy_log;
static my::CriticalSection dummy_log_sec;

void COutputWnd::OnEventLog(const char * str)
{
	std::basic_string<TCHAR> logs = ms2ts(str);
	boost::trim_if(logs, boost::algorithm::is_any_of(_T("\n\r")));
	logs.append(_T("\n"));
	if (!my::D3DContext::getSingleton().m_d3dDeviceSec.TryEnter())
	{
		m_wndOutputDebug.SetSel(-1, -1);
		m_wndOutputDebug.ReplaceSel(logs.c_str());
	}
	else
	{
		// ! ResourceMgr::LoadIORequestAndWait
		my::D3DContext::getSingleton().m_d3dDeviceSec.Leave();
		my::CriticalSectionLock lock(dummy_log_sec);
		dummy_log.push_back(logs);
	}
}

void COutputWnd::OnDestroy()
{
	theApp.m_EventLog.connect(boost::bind(&COutputWnd::OnEventLog, this, boost::placeholders::_1));

	CDockablePane::OnDestroy();
	// TODO: Add your message handler code here
}


afx_msg LRESULT COutputWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	my::CriticalSectionLock lock(dummy_log_sec);
	std::list<std::basic_string<TCHAR> >::iterator log_iter = dummy_log.begin();
	for (; log_iter != dummy_log.end(); log_iter++)
	{
		m_wndOutputDebug.SetSel(-1, -1);
		m_wndOutputDebug.ReplaceSel(log_iter->c_str());
	}
	dummy_log.clear();
	lock.Unlock();
	return CDockablePane::OnIdleUpdateCmdUI(wParam, lParam);
}

#include "stdafx.h"
#include "OutlinerWnd.h"
#include "MainApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(COutlinerWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

COutlinerWnd::COutlinerWnd() noexcept
{

}

COutlinerWnd::~COutlinerWnd()
{

}

int COutlinerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!m_listCtrl.CreateEx(WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_SINGLESEL | LVS_REPORT | LVS_SHOWSELALWAYS,
		CRect(0, 0, 100, 100), this, 1))
	{
		TRACE0("Failed to create outliner windows\n");
		return -1;
	}

	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_listCtrl.SetFont(&afxGlobalData.fontRegular);

	m_listCtrl.InsertColumn(0, _T("aaa"), 0, 100);
	m_listCtrl.InsertColumn(1, _T("bbb"), 0, 30);

	theApp.m_EventNamedObjectAdded.connect(boost::bind(&COutlinerWnd::OnNamedObjectAdded, this, _1));
	theApp.m_EventNamedObjectRemoved.connect(boost::bind(&COutlinerWnd::OnNamedObjectRemoved, this, _1));

	return 0;
}

void COutlinerWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	m_listCtrl.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}


void COutlinerWnd::OnDestroy()
{
	CDockablePane::OnDestroy();

	// TODO: Add your message handler code here
	theApp.m_EventNamedObjectAdded.disconnect(boost::bind(&COutlinerWnd::OnNamedObjectAdded, this, _1));
	theApp.m_EventNamedObjectRemoved.disconnect(boost::bind(&COutlinerWnd::OnNamedObjectRemoved, this, _1));
}

void COutlinerWnd::OnNamedObjectAdded(my::EventArg* arg)
{
	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), ms2ts(named_obj_arg->pObj->GetName()).c_str());
}

void COutlinerWnd::OnNamedObjectRemoved(my::EventArg* arg)
{
	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	CString Name(ms2ts(named_obj_arg->pObj->GetName()).c_str());
	LVFINDINFO info = { LVFI_STRING, Name, 0 };
	int nItem = m_listCtrl.FindItem(&info, -1);
	ASSERT(nItem != -1);
	m_listCtrl.DeleteItem(nItem);
}

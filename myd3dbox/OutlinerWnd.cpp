#include "stdafx.h"
#include "OutlinerWnd.h"
#include "MainApp.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(COutlinerWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_GETDISPINFO, 1, &COutlinerWnd::OnLvnGetdispinfoList)
	ON_NOTIFY(LVN_ODCACHEHINT, 1, &COutlinerWnd::OnLvnOdcachehintList)
	ON_NOTIFY(LVN_ODFINDITEM, 1, &COutlinerWnd::OnLvnOdfinditemList)
END_MESSAGE_MAP()

COutlinerWnd::COutlinerWnd() noexcept
{
	m_IgnoreNamedObjectRemoved = FALSE;
}

COutlinerWnd::~COutlinerWnd()
{

}

int COutlinerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!m_listCtrl.CreateEx(WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR,
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, CRect(0, 0, 100, 100), this, 1))
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
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventSelectionChanged.connect(boost::bind(&COutlinerWnd::OnSelectionChanged, this, _1));

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
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventSelectionChanged.disconnect(boost::bind(&COutlinerWnd::OnSelectionChanged, this, _1));
}

void COutlinerWnd::OnNamedObjectAdded(my::EventArg* arg)
{
	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	m_Items.push_back(ListItem(named_obj_arg->pObj, ms2ts(named_obj_arg->pObj->GetName())));
}

void COutlinerWnd::OnNamedObjectRemoved(my::EventArg* arg)
{
	if (m_IgnoreNamedObjectRemoved)
	{
		return;
	}

	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	std::basic_string<TCHAR> name = ms2ts(named_obj_arg->pObj->GetName());
	ListItemSet::nth_index<1>::type& name_index = m_Items.get<1>();
	ListItemSet::nth_index<1>::type::iterator obj_iter = name_index.find(name);
	ASSERT(obj_iter != name_index.end());
	name_index.erase(obj_iter);
}

void COutlinerWnd::OnSelectionChanged(my::EventArg* arg)
{
	if (m_Items.size() != m_listCtrl.GetItemCount())
	{
		m_listCtrl.SetItemCountEx(m_Items.size(), /*LVSICF_NOINVALIDATEALL |*/ LVSICF_NOSCROLL);
	}
}

void COutlinerWnd::OnLvnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem = &(pDispInfo)->item;

	int iItem = pItem->iItem;

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		switch (pItem->iSubItem)
		{
		case 0: //fill in main text
			//_tcscpy_s(pItem->pszText, pItem->cchTextMax,
			//	m_Items[iItem].name.c_str());
			pItem->pszText = const_cast<TCHAR*>(m_Items[iItem].name.c_str());
			break;
		}
	}
}

void COutlinerWnd::OnLvnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCACHEHINT pCacheHint = reinterpret_cast<LPNMLVCACHEHINT>(pNMHDR);

	// Update the cache with the recommended range.
	for (int i = pCacheHint->iFrom; i <= pCacheHint->iTo; i++)
	{
		m_Items[i];
	}
}

void COutlinerWnd::OnLvnOdfinditemList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVFINDITEM* pFindInfo = reinterpret_cast<NMLVFINDITEM*>(pNMHDR);
	if (!(pFindInfo->lvfi.flags & LVFI_STRING))
	{
		*pResult = -1;
		return;
	}

	ListItemSet::nth_index<1>::type& name_index = m_Items.get<1>();
	ListItemSet::nth_index<1>::type::const_iterator obj_iter = name_index.find(pFindInfo->lvfi.psz);
	if (obj_iter == name_index.end())
	{
		*pResult = -1;
		return;
	}

	*pResult = std::distance(name_index.begin(), obj_iter);
}

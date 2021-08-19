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
	ON_NOTIFY(NM_CLICK, 1, &COutlinerWnd::OnNotifyClick)
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
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT /*| LVS_SINGLESEL*/ | LVS_SHOWSELALWAYS | LVS_OWNERDATA, CRect(0, 0, 100, 100), this, 1))
	{
		TRACE0("Failed to create outliner windows\n");
		return -1;
	}

	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_listCtrl.SetFont(&afxGlobalData.fontRegular);
	m_listCtrl.SetCallbackMask(LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK);
	m_listCtrl.InsertColumn(0, _T("Name"), 0, 100);
	m_listCtrl.InsertColumn(1, _T("Type"), 0, 30);

	theApp.m_EventNamedObjectAdded.connect(boost::bind(&COutlinerWnd::OnNamedObjectAdded, this, _1));
	theApp.m_EventNamedObjectRemoved.connect(boost::bind(&COutlinerWnd::OnNamedObjectRemoved, this, _1));
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventSelectionChanged.connect(boost::bind(&COutlinerWnd::OnSelectionChanged, this, _1));
	pFrame->m_EventAttributeChanged.connect(boost::bind(&COutlinerWnd::OnAttributeChanged, this, _1));

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
	pFrame->m_EventAttributeChanged.disconnect(boost::bind(&COutlinerWnd::OnAttributeChanged, this, _1));
}

void COutlinerWnd::OnNamedObjectAdded(my::EventArg* arg)
{
	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	m_Items.push_back(named_obj_arg->pObj);
}

void COutlinerWnd::OnNamedObjectRemoved(my::EventArg* arg)
{
	if (m_IgnoreNamedObjectRemoved)
	{
		return;
	}

	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	ItemSet::nth_index<1>::type& object_index = m_Items.get<1>();
	ItemSet::nth_index<1>::type::iterator obj_iter = object_index.find(named_obj_arg->pObj);
	ASSERT(obj_iter != object_index.end());
	object_index.erase(obj_iter);
}

void COutlinerWnd::OnSelectionChanged(my::EventArg* arg)
{
	if (m_Items.size() != m_listCtrl.GetItemCount())
	{
		m_listCtrl.SetItemCountEx((int)m_Items.size(), /*LVSICF_NOINVALIDATEALL |*/ LVSICF_NOSCROLL);
	}
	else
	{
		m_listCtrl.Invalidate();
	}
}

void COutlinerWnd::OnAttributeChanged(my::EventArg* arg)
{
	m_listCtrl.Invalidate();
}

void COutlinerWnd::OnLvnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem = &(pDispInfo)->item;

	int iItem = pItem->iItem;

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		switch (pItem->iSubItem)
		{
		case 0: //fill in main text
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_Items[iItem]->GetName(), -1, pItem->pszText, pItem->cchTextMax);
#else
			_tcscpy_s(pItem->pszText, pItem->cchTextMax, m_Items[iItem]->GetName());
#endif
			break;
		}
	}

	if (pItem->mask & LVIF_STATE)
	{
		my::NamedObject* obj = m_Items[pItem->iItem];
		my::Control* control = dynamic_cast<my::Control*>(obj);
		if (control)
		{
			if (control == pFrame->m_selctl)
			{
				pItem->state = (pItem->state | LVIS_SELECTED);
			}
			else
			{
				pItem->state = (pItem->state & ~LVIS_SELECTED);
			}
		}

		Actor* actor = dynamic_cast<Actor*>(obj);
		if (actor)
		{
			if (pFrame->m_selactors.end() != std::find(pFrame->m_selactors.begin(), pFrame->m_selactors.end(), actor))
			{
				pItem->state = (pItem->state | LVIS_SELECTED);
			}
			else
			{
				pItem->state = (pItem->state & ~LVIS_SELECTED);
			}
		}

		Component* cmp = dynamic_cast<Component*>(obj);
		if (cmp)
		{
			if (cmp == pFrame->m_selcmp)
			{
				pItem->state = (pItem->state | LVIS_SELECTED);
			}
			else
			{
				pItem->state = (pItem->state & ~LVIS_SELECTED);
			}
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

	std::string psz = ts2ms(pFindInfo->lvfi.psz);
	int i = pFindInfo->iStart;
	for (; i < m_Items.size(); i++)
	{
		if (_strnicmp(m_Items[i]->GetName(), psz.c_str(), psz.length()) == 0)
		{
			*pResult = i;
			return;
		}
	}

	for (i = 0; i < pFindInfo->iStart; i++)
	{
		if (_strnicmp(m_Items[i]->GetName(), psz.c_str(), psz.length()) == 0)
		{
			*pResult = i;
			return;
		}
	}

	*pResult = -1;
}

void COutlinerWnd::OnNotifyClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	NMITEMACTIVATE* pItemActivate = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	if (pItemActivate->iItem < 0 || pItemActivate->iItem >= m_Items.size())
	{
		pFrame->m_selactors.clear();
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctl = NULL;
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}

	my::NamedObject* obj = m_Items[pItemActivate->iItem];
	my::Control* control = dynamic_cast<my::Control*>(obj);
	if (control)
	{
		pFrame->m_selactors.clear();
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctl = control;
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}

	Actor* actor = dynamic_cast<Actor*>(obj);
	if (actor)
	{
		pFrame->m_selactors.clear();
		pFrame->m_selactors.push_back(actor);
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctl = NULL;
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}

	Component* cmp = dynamic_cast<Component*>(obj);
	if (cmp)
	{
		pFrame->m_selactors.clear();
		pFrame->m_selactors.push_back(cmp->m_Actor);
		pFrame->m_selcmp = cmp;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctl = NULL;
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}
}

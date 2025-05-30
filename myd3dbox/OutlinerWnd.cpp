// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "stdafx.h"
#include "OutlinerWnd.h"
#include "MainApp.h"
#include "MainFrm.h"
#include "ChildView.h"
#include <boost/assign/list_of.hpp>

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
	ON_NOTIFY(NM_DBLCLK, 1, &COutlinerWnd::OnNotifyDblclk)
	ON_NOTIFY(LVN_COLUMNCLICK, 1, &COutlinerWnd::OnLvnColumnClickList)
	ON_NOTIFY(NM_CUSTOMDRAW, 1, &COutlinerWnd::OnNMCustomdraw)
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
		TRACE0("Failed to create list control\n");
		return -1;
	}

	if (!m_listImage.Create(IDR_MAINFRAME_256, 16, 30, RGB(192, 192, 192)))
	{
		TRACE0("Failed to create list image\n");
		return -1;
	}

	m_listCtrl.SetImageList(&m_listImage, LVSIL_SMALL);
	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_listCtrl.SetFont(&afxGlobalData.fontRegular);
	m_listCtrl.SetCallbackMask(LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK);
	m_listCtrl.InsertColumn(0, _T("Name"), 0, 150);

	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventSelectionChanged.connect(boost::bind(&COutlinerWnd::OnSelectionChanged, this, boost::placeholders::_1));
	pFrame->m_EventAttributeChanged.connect(boost::bind(&COutlinerWnd::OnAttributeChanged, this, boost::placeholders::_1));

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
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);
	pFrame->m_EventSelectionChanged.disconnect(boost::bind(&COutlinerWnd::OnSelectionChanged, this, boost::placeholders::_1));
	pFrame->m_EventAttributeChanged.disconnect(boost::bind(&COutlinerWnd::OnAttributeChanged, this, boost::placeholders::_1));
}

void COutlinerWnd::OnInitItemList()
{
	CMainApp::NamedObjectMap::const_iterator named_obj_iter = theApp.m_NamedObjects.begin();
	for (; named_obj_iter != theApp.m_NamedObjects.end(); named_obj_iter++)
	{
		m_Items.push_back(named_obj_iter->second);
	}
	m_listCtrl.SetItemCountEx(m_Items.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	theApp.m_EventNamedObjectCreate.connect(boost::bind(&COutlinerWnd::OnNamedObjectAdded, this, boost::placeholders::_1));
	theApp.m_EventNamedObjectDestroy.connect(boost::bind(&COutlinerWnd::OnNamedObjectRemoved, this, boost::placeholders::_1));
}

void COutlinerWnd::OnDestroyItemList()
{
	theApp.m_EventNamedObjectCreate.disconnect(boost::bind(&COutlinerWnd::OnNamedObjectAdded, this, boost::placeholders::_1));
	theApp.m_EventNamedObjectDestroy.disconnect(boost::bind(&COutlinerWnd::OnNamedObjectRemoved, this, boost::placeholders::_1));
	m_listCtrl.DeleteAllItems();
	m_Items.clear();
}

void COutlinerWnd::OnNamedObjectAdded(my::EventArg* arg)
{
	ASSERT(GetCurrentThreadId() == my::D3DContext::getSingleton().m_d3dThreadId);

	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	m_Items.push_back(named_obj_arg->pObj);
	m_listCtrl.InsertItem(m_Items.size() - 1, LVIF_TEXT, LPSTR_TEXTCALLBACK, 0, 0, 0, 0);
}

void COutlinerWnd::OnNamedObjectRemoved(my::EventArg* arg)
{
	ASSERT(GetCurrentThreadId() == my::D3DContext::getSingleton().m_d3dThreadId);

	CMainApp::NamedObjectEventArgs* named_obj_arg = dynamic_cast<CMainApp::NamedObjectEventArgs*>(arg);
	ASSERT(named_obj_arg);
	ItemSet::nth_index<1>::type& object_index = m_Items.get<1>();
	ItemSet::nth_index<1>::type::iterator obj_iter = object_index.find(named_obj_arg->pObj);
	ASSERT(obj_iter != object_index.end());
	int nItem = (int)std::distance(m_Items.begin(), m_Items.project<0>(obj_iter));
	object_index.erase(obj_iter);
	m_listCtrl.DeleteItem(nItem);
}

void COutlinerWnd::OnSelectionChanged(my::EventArg* arg)
{
	m_listCtrl.Invalidate();
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

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		switch (pItem->iSubItem)
		{
		case 0: //fill in main text
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_Items[pItem->iItem]->GetName(), -1, pItem->pszText, pItem->cchTextMax);
#else
			_tcscpy_s(pItem->pszText, pItem->cchTextMax, m_Items[pItem->iItem]->GetName());
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
			CMainFrame::ControlList::iterator ctrl_iter = std::find(pFrame->m_selctls.begin(), pFrame->m_selctls.end(), control);
			if (ctrl_iter != pFrame->m_selctls.end())
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

	if (pItem->mask & LVIF_IMAGE)
	{
		my::NamedObject* obj = m_Items[pItem->iItem];
		my::Control* control = dynamic_cast<my::Control*>(obj);
		if (control && control->GetTopControl()->m_Manager)
		{
			switch (control->GetControlType())
			{
			case my::Control::ControlTypeStatic:
				pItem->iImage = 21 + 1;
				break;
			case my::Control::ControlTypeProgressBar:
				pItem->iImage = 21 + 2;
				break;
			case my::Control::ControlTypeButton:
				pItem->iImage = 21 + 3;
				break;
			case my::Control::ControlTypeEditBox:
				pItem->iImage = 21 + 4;
				break;
			case my::Control::ControlTypeImeEditBox:
				pItem->iImage = 21 + 4;
				break;
			case my::Control::ControlTypeScrollBar:
			case my::Control::ControlTypeHorizontalScrollBar:
				pItem->iImage = 21 + 8;
				break;
			case my::Control::ControlTypeCheckBox:
				pItem->iImage = 21 + 5;
				break;
			case my::Control::ControlTypeComboBox:
				pItem->iImage = 21 + 6;
				break;
			case my::Control::ControlTypeListBox:
				pItem->iImage = 21 + 7;
				break;
			case my::Control::ControlTypeDialog:
				pItem->iImage = 21;
				break;
			}
		}

		Actor* actor = dynamic_cast<Actor*>(obj);
		if (actor && actor->m_Node)
		{
			pItem->iImage = 6;
		}

		Component* cmp = dynamic_cast<Component*>(obj);
		if (cmp && cmp->m_Actor && cmp->m_Actor->m_Node)
		{
			switch (cmp->GetComponentType())
			{
			case Component::ComponentTypeController:
				pItem->iImage = 12;
				break;
			case Component::ComponentTypeMesh:
				pItem->iImage = 7;
				break;
			case Component::ComponentTypeCloth:
				pItem->iImage = 8;
				break;
			case Component::ComponentTypeStaticEmitter:
				pItem->iImage = 9;
				break;
			case Component::ComponentTypeSphericalEmitter:
				pItem->iImage = 10;
				break;
			case Component::ComponentTypeTerrain:
				pItem->iImage = 11;
				break;
			case Component::ComponentTypeAnimator:
				pItem->iImage = 13;
				break;
			case Component::ComponentTypeNavigation:
				pItem->iImage = 14;
				break;
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
		pFrame->m_selctls.clear();
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}

	my::NamedObject* obj = m_Items[pItemActivate->iItem];
	my::Control* control = dynamic_cast<my::Control*>(obj);
	if (control && control->GetTopControl()->m_Manager)
	{
		pFrame->m_selactors.clear();
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctls = boost::assign::list_of(control);
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}

	Actor* actor = dynamic_cast<Actor*>(obj);
	if (actor && actor->m_Node)
	{
		pFrame->m_selactors.clear();
		pFrame->m_selactors.push_back(actor);
		pFrame->m_selcmp = NULL;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctls.clear();
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}

	Component* cmp = dynamic_cast<Component*>(obj);
	if (cmp && cmp->m_Actor && cmp->m_Actor->m_Node)
	{
		pFrame->m_selactors.clear();
		pFrame->m_selactors.push_back(cmp->m_Actor);
		pFrame->m_selcmp = cmp;
		pFrame->m_selchunkid.SetPoint(0, 0);
		pFrame->m_selinstid = 0;
		pFrame->m_selctls.clear();
		pFrame->OnSelChanged();
		*pResult = 0;
		return;
	}
}

void COutlinerWnd::OnNotifyDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMainFrame* pFrame = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd());
	ASSERT_VALID(pFrame);

	CChildView* pView = DYNAMIC_DOWNCAST(CChildView, pFrame->GetActiveView());
	pView->SendMessage(WM_KEYDOWN, 'F', MAKEWORD(1, 0));
}

void COutlinerWnd::OnLvnColumnClickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	if (pListView->iSubItem == 0)
	{
		std::vector<boost::reference_wrapper<const ItemSet::value_type> > v;
		ItemSet::iterator item_iter = m_Items.begin();
		for (; item_iter != m_Items.end(); item_iter++)
		{
			v.push_back(boost::cref(*item_iter));
		}

		struct ItemCompare
		{
			bool operator() (my::NamedObject* lhs, my::NamedObject* rhs)
			{
				return stricmp(lhs->GetName(), rhs->GetName()) < 0;
			}
		};
		std::sort(v.begin(), v.end(), ItemCompare());
		m_Items.rearrange(v.begin());
		m_listCtrl.Invalidate();
	}
}

void COutlinerWnd::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)pNMHDR;
	switch (pcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
	{
		my::NamedObject* obj = m_Items[pcd->nmcd.dwItemSpec];
		my::Control* control = dynamic_cast<my::Control*>(obj);
		if (control && !control->GetVisibleHierarchy())
		{
			pcd->clrText = GetSysColor(COLOR_GRAYTEXT);
		}
		*pResult = CDRF_DODEFAULT;// do not set *pResult = CDRF_SKIPDEFAULT
		break;
	}
	}
}

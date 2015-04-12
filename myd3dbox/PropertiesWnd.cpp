#include "StdAfx.h"
#include "PropertiesWnd.h"
#include "MainApp.h"
#include "MainFrm.h"
//#include "MainView.h"
#include "CtrlProps.h"

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	//ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	//ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	//ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	//ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CPropertiesWnd::OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd())
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

		m_wndToolBar.SetWindowPos(
			NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

		m_wndPropList.SetWindowPos(
			NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, CRect(), this, 2))
	{
		TRACE0("未能创建属性网格\n");
		return -1;
	}

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	if (!m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, 2001) || !m_wndToolBar.LoadToolBar(2001, 0, 0, TRUE))
		return -1;

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	SetPropListFont();

	AdjustLayout();

	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);

	SetPropListFont();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	CSimpleProp * pProp = DYNAMIC_DOWNCAST(CSimpleProp, (CMFCPropertyGridProperty *)lParam);
	_ASSERT(pProp);

	//pProp->OnEventChanged();

	//CMainView::getSingleton().SendMessage(WM_UPDATE_PIVOTCONTROLLER);

	return 0;
}

LRESULT CPropertiesWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	CDockablePane::OnIdleUpdateCmdUI(wParam, lParam);

	//TreeNodeBasePtr node = COutlinerView::getSingleton().GetSelectedNode();
	//if(!node)
	//{
	//	if(m_wndPropList.GetPropertyCount() > 0)
	//	{
	//		m_wndPropList.RemoveAll();

	//		m_wndPropList.Invalidate();

	//		m_SelectedNode.reset();
	//	}
	//}
	//else if(node == m_SelectedNode.lock())
	//{
	//	if(m_bIsPropInvalid)
	//	{
	//		for(int i = 0; i < m_wndPropList.GetPropertyCount(); i++)
	//		{
	//			CSimpleProp * pProp = DYNAMIC_DOWNCAST(CSimpleProp, m_wndPropList.GetProperty(i));
	//			_ASSERT(pProp);

	//			pProp->OnEventUpdated();
	//		}

	//		m_bIsPropInvalid = FALSE;

	//		m_wndPropList.Invalidate();
	//	}
	//}
	//else
	//{
	//	m_wndPropList.RemoveAll();

	//	m_wndPropList.Invalidate();

	//	node->SetupProperties(&m_wndPropList);

	//	m_SelectedNode = node;
	//}
	return 0;
}

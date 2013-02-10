#include "StdAfx.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "MainView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

CMainFrame::SingleInstance * my::SingleInstance<CMainFrame>::s_ptr(NULL);

CMainFrame::CMainFrame(void)
{
	RegisterFileDir("Media");
	RegisterZipArchive("Media.zip");
	RegisterFileDir("..\\demo2_3\\Media");
	RegisterZipArchive("..\\demo2_3\\Media.zip");
}

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	TRACE0("CMainFrame::OnCreate \n");

	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
    m_d3dpp.Windowed = TRUE;
    m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dpp.hDeviceWindow = m_hWnd;
	m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	HRESULT hr = theApp.m_d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_d3dDevice);
	if(FAILED(hr))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return -1;
	}
	m_DeviceObjectsCreated = true;

	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(FAILED(hr = OnDeviceReset()))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return -1;
	}
	m_DeviceObjectsReset = true;

	OnApplicationLook(theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_WIN_2000));

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;
	}
	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	CString strToolBarName;
	strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);

	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);
	CMFCToolBar::EnableQuickCustomization();

	return 0;
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	switch (id)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (id)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), m_nAppLook = id);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnDestroy()
{
	if(m_DeviceObjectsReset)
	{
		OnDeviceLost();
		m_DeviceObjectsReset = false;
	}

	TRACE0("CMainFrame::OnDestroy \n");

	CFrameWndEx::OnDestroy();

	if(m_DeviceObjectsCreated)
	{
		LoaderMgr::OnDestroyDevice();

		UINT references = m_d3dDevice.Detach()->Release();
		if(references > 0)
		{
			CString msg;
			msg.Format(_T("no zero reference count: %u"), references);
			AfxMessageBox(msg);
		}
		m_DeviceObjectsCreated = false;
	}
}

HRESULT CMainFrame::ResetD3DDevice(void)
{
	if(m_DeviceObjectsReset)
	{
		OnDeviceLost();
		m_DeviceObjectsReset = false;
	}

	HRESULT hr;
	if(FAILED(hr = m_d3dDevice->Reset(&m_d3dpp)))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	if(FAILED(hr = OnDeviceReset()))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}
	m_DeviceObjectsReset = true;

	return S_OK;
}

HRESULT CMainFrame::OnDeviceReset(void)
{
	TRACE0("CMainFrame::OnDeviceReset \n");

	my::Surface BackBuffer;
	HRESULT hr;
	if(FAILED(hr = m_d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer.m_ptr)))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	D3DSURFACE_DESC desc = BackBuffer.GetDesc();

	if(FAILED(hr = LoaderMgr::OnResetDevice(m_d3dDevice, &desc)))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	if(FAILED(hr = CMainView::getSingleton().OnDeviceReset()))
	{
		TRACE(my::D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}
	return S_OK;
}

void CMainFrame::OnDeviceLost(void)
{
	TRACE0("CMainFrame::OnDeviceLost \n");

	LoaderMgr::OnLostDevice();

	CMainView::getSingleton().OnDeviceLost();
}

void CMainFrame::OnFrameMove(
	double fTime,
	float fElapsedTime)
{
	CMainView::getSingleton().OnFrameMove(fTime, fElapsedTime);
}

void CMainFrame::OnFrameRender(
	double fTime,
	float fElapsedTime)
{
	CMainView::getSingleton().OnFrameRender(m_d3dDevice, fTime, fElapsedTime);
}

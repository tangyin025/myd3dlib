#include "StdAfx.h"
#include "MainFrm.h"
#include "MainApp.h"
#include "MainView.h"

using namespace my;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void EffectUIRender::Begin(void)
{
	CRect rectClient;
	CMainView::getSingleton().GetClientRect(&rectClient);
	m_UIEffect->SetVector("g_ScreenDim", Vector4((float)rectClient.Width(), (float)rectClient.Height(), 0, 0));

	m_Passes = m_UIEffect->Begin();
}

void EffectUIRender::End(void)
{
	m_UIEffect->End();

	m_Passes = 0;
}

void EffectUIRender::SetWorldViewProj(const Matrix4 & WorldViewProj)
{
	m_UIEffect->SetMatrix("g_WorldViewProjection", WorldViewProj);
}

void EffectUIRender::SetTexture(IDirect3DBaseTexture9 * pTexture)
{
	_ASSERT(CMainFrame::getSingleton().m_WhiteTex);

	m_UIEffect->SetTexture("g_MeshTexture", pTexture ? pTexture : CMainFrame::getSingleton().m_WhiteTex->m_ptr);
}

void EffectUIRender::DrawVertexList(void)
{
	if(vertex_count > 0)
	{
		for(UINT p = 0; p < m_Passes; p++)
		{
			m_UIEffect->BeginPass(p);
			V(m_Device->SetFVF(D3DFVF_CUSTOMVERTEX));
			V(m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, vertex_count / 3, vertex_list, sizeof(vertex_list[0])));
			m_UIEffect->EndPass();
		}
	}
}

CMainFrame::SingleInstance * SingleInstance<CMainFrame>::s_ptr(NULL);

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

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
	m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr;
	if(FAILED(hr = theApp.m_d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_d3dDevice)))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return -1;
	}
	m_DeviceObjectsCreated = true;

	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(FAILED(hr = ResourceMgr::OnCreateDevice(m_d3dDevice, NULL)))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return -1;
	}

	m_UIRender.reset(new EffectUIRender(m_d3dDevice, LoadEffect("shader/UIEffect.fx")));

	m_WhiteTex = LoadTexture("texture/white.bmp");

	m_Font = LoadFont("font/wqy-microhei.ttc", 13);

	m_SimpleSample = LoadEffect("shader/SimpleSample.fx");

	if(FAILED(hr = OnDeviceReset()))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return -1;
	}
	m_DeviceObjectsReset = true;

	OnApplicationLook(theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_WIN_2000));

	EnableDocking(CBRS_ALIGN_ANY);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;
	}
	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}
	m_wndToolBar.SetWindowText(_T("Standard"));
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, _T("Customize..."));
	DockPane(&m_wndToolBar);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	if (!m_wndOutliner.Create(_T("Outliner"), this, CRect(0,0,200,200), TRUE,
		ID_VIEW_OUTLINER, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create outliner\n");
		return -1;
	}
	m_wndOutliner.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutliner);

	if (!m_wndProperties.Create(_T("Properties"), this, CRect(0,0,200,200), TRUE,
		ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE;
	}
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, _T("Customize..."), ID_VIEW_TOOLBAR);
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

	m_UIRender.reset();

	CFrameWndEx::OnDestroy();

	if(m_DeviceObjectsCreated)
	{
		//EmitterInstance::OnDestroyDevice();

		ResourceMgr::OnDestroyDevice();

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
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	if(FAILED(hr = OnDeviceReset()))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}
	m_DeviceObjectsReset = true;

	return S_OK;
}

HRESULT CMainFrame::OnDeviceReset(void)
{
	TRACE0("CMainFrame::OnDeviceReset \n");

	HRESULT hr;
	if(FAILED(hr = ResourceMgr::OnResetDevice(m_d3dDevice, NULL)))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}

	if(FAILED(hr = CMainView::getSingleton().OnDeviceReset()))
	{
		TRACE(D3DException(hr, __FILE__, __LINE__).GetFullDescription().c_str());
		return hr;
	}
	return S_OK;
}

void CMainFrame::OnDeviceLost(void)
{
	TRACE0("CMainFrame::OnDeviceLost \n");

	ResourceMgr::OnLostDevice();

	CMainView::getSingleton().OnDeviceLost();
}

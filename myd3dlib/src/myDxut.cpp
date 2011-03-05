
#include "myDxut.h"
#include <SDKmisc.h>

#define IDC_TOGGLEFULLSCREEN	1
#define IDC_TOGGLEREF			2
#define IDC_CHANGEDEVICE		3

namespace my
{
	Singleton<DxutApp>::DrivedClassPtr DxutApp::s_ptr;

	bool DxutApp::IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
		IDirect3D9 * pD3D = DXUTGetD3D9Object();
		if(FAILED((pD3D->CheckDeviceFormat(
			pCaps->AdapterOrdinal,
			pCaps->DeviceType,
			AdapterFormat,
			D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
			D3DRTYPE_TEXTURE,
			BackBufferFormat))))
		{
			return false;
		}

		if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		{
			return false;
		}
		return true;
	}

	bool DxutApp::ModifyDeviceSettings(
		DXUTDeviceSettings * pDeviceSettings)
	{
		if(DXUT_D3D9_DEVICE == pDeviceSettings->ver
			&& D3DDEVTYPE_REF == pDeviceSettings->d3d9.DeviceType)
		{
			DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
		}
		return true;
	}

	HRESULT DxutApp::OnD3D9CreateDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hr;
		V_RETURN(m_dlgResourceMgr.OnD3D9CreateDevice(pd3dDevice));
		V_RETURN(m_settingsDlg.OnD3D9CreateDevice(pd3dDevice));
		V_RETURN(D3DXCreateFont(
			pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &m_txtFont));
		V_RETURN(D3DXCreateSprite(pd3dDevice, &m_txtSprite));

		return S_OK;
	}

	HRESULT DxutApp::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		HRESULT hr;
		V_RETURN(m_dlgResourceMgr.OnD3D9ResetDevice());
		V_RETURN(m_settingsDlg.OnD3D9ResetDevice());
		V_RETURN(m_txtFont->OnResetDevice());
		V_RETURN(m_txtSprite->OnResetDevice());

		m_hudDlg.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
		m_hudDlg.SetSize(170, 170);

		return S_OK;
	}

	void DxutApp::OnD3D9LostDevice(void)
	{
		m_dlgResourceMgr.OnD3D9LostDevice();
		m_settingsDlg.OnD3D9LostDevice();
		m_txtFont->OnLostDevice();
		m_txtSprite->OnLostDevice();
	}

	void DxutApp::OnD3D9DestroyDevice(void)
	{
		m_dlgResourceMgr.OnD3D9DestroyDevice();
		m_settingsDlg.OnD3D9DestroyDevice();
		m_txtFont.Release();
		m_txtSprite.Release();
	}

	void DxutApp::OnFrameMove(
		double fTime,
		float fElapsedTime)
	{
	}

	void DxutApp::OnD3D9FrameRender(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime)
	{
		HRESULT hr;
		V(pd3dDevice->Clear(
			0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 66, 75, 121), 1.0f, 0));

		if(m_settingsDlg.IsActive())
		{
			m_settingsDlg.OnRender(fElapsedTime);
			return;
		}

		if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
		{
			CDXUTTextHelper txtHelper(m_txtFont, m_txtSprite, 15);
			txtHelper.Begin();
			txtHelper.SetInsertionPos(5, 5);
			txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
			txtHelper.DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
			txtHelper.DrawTextLine(DXUTGetDeviceStats());
			txtHelper.End();
			V(m_hudDlg.OnRender(fElapsedTime));
			V(pd3dDevice->EndScene());
		}
	}

	void DxutApp::OnGUIEvent(
		UINT nEvent,
		int nControlID,
		CDXUTControl * pControl)
	{
		switch(nControlID)
		{
		case IDC_TOGGLEFULLSCREEN:
			DXUTToggleFullScreen();
			break;

		case IDC_TOGGLEREF:
			DXUTToggleREF();
			break;

		case IDC_CHANGEDEVICE:
			m_settingsDlg.SetActive(!m_settingsDlg.IsActive());
			break;
		}
	}

	LRESULT DxutApp::MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		*pbNoFurtherProcessing = m_dlgResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam);
		if(*pbNoFurtherProcessing)
		{
			return 0;
		}

		if(m_settingsDlg.IsActive())
		{
			m_settingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
			return 0;
		}

		*pbNoFurtherProcessing = m_hudDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		if(*pbNoFurtherProcessing)
		{
			return 0;
		}

		return 0;
	}

	void DxutApp::OnKeyboard(
		UINT nChar,
		bool bKeyDown,
		bool bAltDown)
	{
	}

	DxutApp::DxutApp(void)
	{
		DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable_s, this);
		DXUTSetCallbackDeviceChanging(ModifyDeviceSettings_s, this);
		DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice_s, this);
		DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice_s, this);
		DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice_s, this);
		DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice_s, this);
		DXUTSetCallbackFrameMove(OnFrameMove_s, this);
		DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender_s, this);
		DXUTSetCallbackMsgProc(MsgProc_s, this);
		DXUTSetCallbackKeyboard(OnKeyboard_s, this);
	}

	int DxutApp::Run(bool bWindowed, int nSuggestedWidth, int nSuggestedHeight)
	{
		OnInit();

		DXUTInit(true, true, NULL);
		DXUTSetCursorSettings(true, true);
		WCHAR szPath[MAX_PATH];
		GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
		DXUTCreateWindow(szPath);
		DXUTCreateDevice(bWindowed, nSuggestedWidth, nSuggestedHeight);
		DXUTMainLoop();

		return DXUTGetExitCode();
	}

	void DxutApp::OnInit(void)
	{
		m_settingsDlg.Init(&m_dlgResourceMgr);
		m_hudDlg.Init(&m_dlgResourceMgr);
		m_hudDlg.SetCallback(OnGUIEvent_s, this);
		int nY = 10;
		m_hudDlg.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, nY, 125, 22);
		m_hudDlg.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, nY += 24, 125, 22, VK_F3);
		m_hudDlg.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 35, nY += 24, 125, 22, VK_F2);
	}
};

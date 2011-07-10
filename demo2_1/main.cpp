
// --------------------------------------------------------------------------------------
// File: demo2_1/main.cpp
// --------------------------------------------------------------------------------------

#include <dxut.h>
#include <dxutgui.h>
#include <sdkmisc.h>
#include <DXUTsettingsdlg.h>
#include <atlbase.h>

// --------------------------------------------------------------------------------------
// Global variables
// --------------------------------------------------------------------------------------

CDXUTDialogResourceManager	g_DialogResourceManager;
CD3DSettingsDlg				g_SettingsDlg;
CDXUTDialog					g_HUD;
CComPtr<ID3DXFont>			g_font;
CComPtr<ID3DXSprite>		g_sprite;

// --------------------------------------------------------------------------------------
// UI control IDs
// --------------------------------------------------------------------------------------

#define IDC_TOGGLEFULLSCREEN	1
#define IDC_TOGGLEREF			3
#define IDC_CHANGEDEVICE		4

// --------------------------------------------------------------------------------------
// Forward declarations 
// --------------------------------------------------------------------------------------

bool CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void * pUserContext);
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings * pDeviceSettings, void * pUserContext);
HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc, void * pUserContext);
HRESULT CALLBACK OnResetDevice(IDirect3DDevice9 * pd3dDevice, const D3DSURFACE_DESC * pBackBufferSurfaceDesc, void * pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void * pUserContext);
void CALLBACK OnFrameRender(IDirect3DDevice9 * pd3dDevice, double fTime, float fElapsedTime, void * pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool * pbNoFurtherProcessing, void * pUserContext);
void CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown, void * pUserContext);
void CALLBACK OnLostDevice(void * pUserContext);
void CALLBACK OnDestroyDevice(void * pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl * pControl, void * pUserContext);
void InitApp(void);

// --------------------------------------------------------------------------------------
// wWinMain 
// --------------------------------------------------------------------------------------

INT WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	DXUTSetCallbackD3D9DeviceAcceptable(IsDeviceAcceptable);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackD3D9DeviceCreated(OnCreateDevice);
	DXUTSetCallbackD3D9DeviceReset(OnResetDevice);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackD3D9FrameRender(OnFrameRender);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(KeyboardProc);
	DXUTSetCallbackD3D9DeviceLost(OnLostDevice);
	DXUTSetCallbackD3D9DeviceDestroyed(OnDestroyDevice);

	DXUTSetCursorSettings(true, true);

	InitApp();

	DXUTInit(true, true);
	DXUTSetHotkeyHandling(true, true, true);
	DXUTCreateWindow(L"demo2_1");
	DXUTCreateDevice(true, 800, 600);
	DXUTMainLoop();

	return DXUTGetExitCode();
}

// --------------------------------------------------------------------------------------
// InitApp 
// --------------------------------------------------------------------------------------

void InitApp(void)
{
	g_SettingsDlg.Init(&g_DialogResourceManager);
	g_HUD.Init(&g_DialogResourceManager);
	g_HUD.SetCallback(OnGUIEvent);
	g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, 10, 125, 22);
	g_HUD.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, 34, 125, 22);
	g_HUD.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 35, 58, 125, 22, VK_F2);
}

// --------------------------------------------------------------------------------------
// IsDeviceAcceptable 
// --------------------------------------------------------------------------------------

bool CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps,
								 D3DFORMAT AdapterFormat,
								 D3DFORMAT BackBufferFormat,
								 bool bWindowed,
								 void * pUserContext)
{
	IDirect3D9 * pd3d = DXUTGetD3D9Object();
	if(FAILED(pd3d->CheckDeviceFormat(
		pCaps->AdapterOrdinal,
		pCaps->DeviceType,
		AdapterFormat,
		D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE,
		BackBufferFormat)))
	{
		return false;
	}

	if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		return false;
	}

	return true;
}

// --------------------------------------------------------------------------------------
// ModifyDeviceSettings 
// --------------------------------------------------------------------------------------

bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings * pDeviceSettings,
								   void * pUserContext)
{
	_ASSERT(DXUT_D3D9_DEVICE == pDeviceSettings->ver);

	IDirect3D9 * pd3d = DXUTGetD3D9Object();
	D3DCAPS9 caps;
	HRESULT hr;
	V(pd3d->GetDeviceCaps(
		pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps));

	if((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0
		|| caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if(pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF)
	{
		DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	}

	return true;
}

// --------------------------------------------------------------------------------------
// OnCreateDevice 
// --------------------------------------------------------------------------------------

HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9 * pd3dDevice,
								const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
								void * pUserContext)
{
	HRESULT hr;
	V_RETURN(g_DialogResourceManager.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(g_SettingsDlg.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(D3DXCreateFont(pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &g_font));
	return S_OK;
}

// --------------------------------------------------------------------------------------
// OnResetDevice 
// --------------------------------------------------------------------------------------

HRESULT CALLBACK OnResetDevice(IDirect3DDevice9 * pd3dDevice,
							   const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
							   void * pUserContext)
{
	HRESULT hr;
	V_RETURN(g_DialogResourceManager.OnD3D9ResetDevice());
	V_RETURN(g_SettingsDlg.OnD3D9ResetDevice());
	g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_HUD.SetSize(170, 170);
	V_RETURN(g_font->OnResetDevice());
	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_sprite));
	return S_OK;
}

// --------------------------------------------------------------------------------------
// OnFrameMove 
// --------------------------------------------------------------------------------------

void CALLBACK OnFrameMove(double fTime,
						  float fElapsedTime,
						  void * pUserContext)
{
}

// --------------------------------------------------------------------------------------
// OnFrameRender 
// --------------------------------------------------------------------------------------

void CALLBACK OnFrameRender(IDirect3DDevice9 * pd3dDevice,
							double fTime,
							float fElapsedTime,
							void * pUserContext)
{
	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.OnRender(fElapsedTime);
		return;
	}

	HRESULT hr;
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 76, 76, 76), 1.0f, 0));

	if(SUCCEEDED(pd3dDevice->BeginScene()))
	{
		V(g_HUD.OnRender(fElapsedTime));
	    CDXUTTextHelper txtHelper(g_font, g_sprite, 15);
		txtHelper.Begin();
		txtHelper.SetInsertionPos(5, 5);
		txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		txtHelper.DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
		txtHelper.DrawTextLine(DXUTGetDeviceStats());
		txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
		txtHelper.DrawTextLine(L"Press ESC to quit");
		txtHelper.End();
		V(pd3dDevice->EndScene());
	}
}

// --------------------------------------------------------------------------------------
// MsgProc 
// --------------------------------------------------------------------------------------

LRESULT CALLBACK MsgProc(HWND hWnd,
						 UINT uMsg,
						 WPARAM wParam,
						 LPARAM lParam,
						 bool * pbNoFurtherProcessing,
						 void * pUserContext)
{
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
	{
		return 0;
	}

	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	*pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
	{
		return 0;
	}

	return 0;
}

// --------------------------------------------------------------------------------------
// KeyboardProc 
// --------------------------------------------------------------------------------------

void CALLBACK KeyboardProc(UINT nChar,
						   bool bKeyDown,
						   bool bAltDown,
						   void * pUserContext)
{
}

// --------------------------------------------------------------------------------------
// OnLostDevice 
// --------------------------------------------------------------------------------------

void CALLBACK OnLostDevice(void * pUserContext)
{
	g_DialogResourceManager.OnD3D9LostDevice();
	g_SettingsDlg.OnD3D9LostDevice();
	g_font->OnLostDevice();
	g_sprite.Release();
}

// --------------------------------------------------------------------------------------
// OnDestroyDevice 
// --------------------------------------------------------------------------------------

void CALLBACK OnDestroyDevice(void * pUserContext)
{
	g_DialogResourceManager.OnD3D9DestroyDevice();
	g_SettingsDlg.OnD3D9DestroyDevice();
	g_font.Release();
}

// --------------------------------------------------------------------------------------
// OnGUIEvent 
// --------------------------------------------------------------------------------------

void CALLBACK OnGUIEvent(UINT nEvent,
						 int nControlID,
						 CDXUTControl * pControl,
						 void * pUserContext)
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
		g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
		break;
	}
}


// --------------------------------------------------------------------------------------
// File: demo2_1/main.cpp
// --------------------------------------------------------------------------------------

#include <dxut.h>
#include <dxutgui.h>
#include <sdkmisc.h>

// --------------------------------------------------------------------------------------
// Global variables
// --------------------------------------------------------------------------------------

CDXUTDialogResourceManager g_DialogResourceManager;

// --------------------------------------------------------------------------------------
// UI control IDs
// --------------------------------------------------------------------------------------

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
	HRESULT hr;
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 76, 76, 76), 1.0f, 0));

	if(SUCCEEDED(pd3dDevice->BeginScene()))
	{
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
}

// --------------------------------------------------------------------------------------
// OnDestroyDevice 
// --------------------------------------------------------------------------------------

void CALLBACK OnDestroyDevice(void * pUserContext)
{
	g_DialogResourceManager.OnD3D9DestroyDevice();
}

// --------------------------------------------------------------------------------------
// OnGUIEvent 
// --------------------------------------------------------------------------------------

void CALLBACK OnGUIEvent(UINT nEvent,
						 int nControlID,
						 CDXUTControl * pControl,
						 void * pUserContext)
{
}

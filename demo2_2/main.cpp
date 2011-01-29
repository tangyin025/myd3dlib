
#include "DXUT.h"
#include "SDKmisc.h"

// ------------------------------------------------------------------------------------------
// IsD3D9DeviceAcceptable
// ------------------------------------------------------------------------------------------

bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9 * pCaps,
									 D3DFORMAT AdapterFormat,
									 D3DFORMAT BackBufferFormat,
									 bool bWindowed,
									 void * pUserContext)
{
	// 跳过不支持alpha blending的后缓存
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

	// 至少要支持ps2.0，这还是要看实际使用情况
	if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
	{
		return false;
	}
	return true;
}

// ------------------------------------------------------------------------------------------
// ModifyDeviceSettings
// ------------------------------------------------------------------------------------------

bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings * pDeviceSettings,
								   void * pUserContext)
{
	// 如果创建一个ref设备（即软件模拟），则应该给出一个警告
	if(DXUT_D3D9_DEVICE == pDeviceSettings->ver
		&& D3DDEVTYPE_REF == pDeviceSettings->d3d9.DeviceType)
	{
		DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	}

	return true;
}

// ------------------------------------------------------------------------------------------
// OnD3D9CreateDevice
// ------------------------------------------------------------------------------------------

HRESULT CALLBACK OnD3D9CreateDevice(IDirect3DDevice9 * pd3dDevice,
									const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
									void * pUserContext)
{
	// 在这里创建d3d9资源，但这些资源应该不受device reset限制的
	return S_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9ResetDevice
// ------------------------------------------------------------------------------------------

HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9 * pd3dDevice,
								   const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
								   void * pUserContext)
{
	// 在这里创建d3d9资源，但这些资源将受到device reset限制
	return S_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9LostDevice
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9LostDevice(void * pUserContext)
{
	// 在这里处理在reset中创建的资源
}

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9DestroyDevice(void * pUserContext)
{
	// 在这里销毁在create中创建的资源
}

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice);
	DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice);
	DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice);
	DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice);

	DXUTInit(true, true, NULL);
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"demo2_1");
	DXUTCreateDevice(true, 800, 600);
	DXUTMainLoop();

	return DXUTGetExitCode();
}


#include <atlbase.h>
#include <atlstr.h>
#include <DXUT.h>
#include <DXUTgui.h>
#include <SDKmisc.h>
#include <DXUTSettingsDlg.h>
#include <DXUTCamera.h>

// ------------------------------------------------------------------------------------------
// Global variables
// ------------------------------------------------------------------------------------------

CDXUTDialogResourceManager		g_DialogResourceMgr;
CD3DSettingsDlg					g_SettingsDlg;
CDXUTDialog						g_HUD;
CComPtr<ID3DXFont>				g_Font9;
CComPtr<ID3DXSprite>			g_Sprite9;
CComPtr<ID3DXEffect>			g_Effect9;
CModelViewerCamera				g_Camera;

// ------------------------------------------------------------------------------------------
// UI control IDs
// ------------------------------------------------------------------------------------------

#define IDC_TOGGLEFULLSCREEN	1
#define IDC_TOGGLEREF			2
#define IDC_CHANGEDEVICE		3

// ------------------------------------------------------------------------------------------
// IsD3D9DeviceAcceptable
// ------------------------------------------------------------------------------------------

bool CALLBACK IsD3D9DeviceAcceptable(D3DCAPS9 * pCaps,
									 D3DFORMAT AdapterFormat,
									 D3DFORMAT BackBufferFormat,
									 bool bWindowed,
									 void * pUserContext)
{
	// ������֧��alpha blending�ĺ󻺴�
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

	//// ����Ҫ֧��ps2.0���⻹��Ҫ��ʵ��ʹ�����
	//if(pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
	//{
	//	return false;
	//}
	return true;
}

// ------------------------------------------------------------------------------------------
// ModifyDeviceSettings
// ------------------------------------------------------------------------------------------

bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings * pDeviceSettings,
								   void * pUserContext)
{
	// �������һ��ref�豸�������ģ�⣩����Ӧ�ø���һ������
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
	// �����ﴴ��d3d9��Դ������Щ��ԴӦ�ò���device reset���Ƶ�
	HRESULT hr;
	V_RETURN(g_DialogResourceMgr.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(g_SettingsDlg.OnD3D9CreateDevice(pd3dDevice));
	V_RETURN(D3DXCreateFont(
		pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &g_Font9));
	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_Sprite9));

	// ��ȡD3DX Effect�ļ�
	WCHAR str[MAX_PATH];
	V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"SimpleSample.fx"));
	V_RETURN(D3DXCreateEffectFromFile(
		pd3dDevice, str, NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &g_Effect9, NULL));

	// ��ʼ�����
	D3DXVECTOR3 vecEye(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 vecAt(0.0f, 0.0f, -0.0f);
	g_Camera.SetViewParams(&vecEye, &vecAt);
	return S_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9ResetDevice
// ------------------------------------------------------------------------------------------

HRESULT CALLBACK OnD3D9ResetDevice(IDirect3DDevice9 * pd3dDevice,
								   const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
								   void * pUserContext)
{
	// �����ﴴ��d3d9��Դ������Щ��Դ���ܵ�device reset����
	HRESULT hr;
	V_RETURN(g_DialogResourceMgr.OnD3D9ResetDevice());
	V_RETURN(g_SettingsDlg.OnD3D9ResetDevice());
	V_RETURN(g_Font9->OnResetDevice());
	V_RETURN(g_Sprite9->OnResetDevice());
	V_RETURN(g_Effect9->OnResetDevice());

	// �������������ͶӰ
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f);
	g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	g_HUD.SetSize(170, 170);
	return S_OK;
}

// ------------------------------------------------------------------------------------------
// OnD3D9LostDevice
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9LostDevice(void * pUserContext)
{
	// �����ﴦ����reset�д�������Դ
	g_DialogResourceMgr.OnD3D9LostDevice();
	g_SettingsDlg.OnD3D9LostDevice();
	g_Font9->OnLostDevice();
	g_Sprite9->OnLostDevice();
	g_Effect9->OnLostDevice();
}

// ------------------------------------------------------------------------------------------
// wWinMain
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9DestroyDevice(void * pUserContext)
{
	// ������������create�д�������Դ
	g_DialogResourceMgr.OnD3D9DestroyDevice();
	g_SettingsDlg.OnD3D9DestroyDevice();
	g_Font9.Release();
	g_Sprite9.Release();
	g_Effect9.Release();
}

// ------------------------------------------------------------------------------------------
// OnFrameMove
// ------------------------------------------------------------------------------------------

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void * pUserContext)
{
	// ��������³���
	g_Camera.FrameMove(fElapsedTime);
}

// ------------------------------------------------------------------------------------------
// OnD3D9FrameRender
// ------------------------------------------------------------------------------------------

void CALLBACK OnD3D9FrameRender(IDirect3DDevice9 * pd3dDevice,
								double fTime,
								float fElapsedTime,
								void * pUserContext)
{
	// ��������Ⱦ����
	HRESULT hr;

	V(pd3dDevice->Clear(
		0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 66, 75, 121), 1.0f, 0));

	if(g_SettingsDlg.IsActive())
	{
		g_SettingsDlg.OnRender(fElapsedTime);
		return;
	}

	if(SUCCEEDED(hr = pd3dDevice->BeginScene()))
	{
		// ������ͶӰ����
		D3DXMATRIXA16 mWorld = *g_Camera.GetWorldMatrix();
		D3DXMATRIXA16 mProj = *g_Camera.GetProjMatrix();
		D3DXMATRIXA16 mView = *g_Camera.GetViewMatrix();
		D3DXMATRIXA16 mWorldViewProjection = mWorld * mView * mProj;

		// ����D3DX Effectֵ
		V(g_Effect9->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection));
		V(g_Effect9->SetMatrix("g_mWorld", &mWorld));
		V(g_Effect9->SetFloat("g_fTime", (float)fTime));

		CDXUTTextHelper txtHelper(g_Font9, g_Sprite9, 15);
		txtHelper.Begin();
		txtHelper.SetInsertionPos(5, 5);
		txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
		txtHelper.DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
		txtHelper.DrawTextLine(DXUTGetDeviceStats());
		txtHelper.End();
		V(g_HUD.OnRender(fElapsedTime));
		V(pd3dDevice->EndScene());
	}
}

// ------------------------------------------------------------------------------------------
// OnGUIEvent
// ------------------------------------------------------------------------------------------

void CALLBACK OnGUIEvent(UINT nEvent,
						 int nControlID,
						 CDXUTControl * pControl,
						 void * pUserContext)
{
	// �����ﴦ��ui�¼�
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

// ------------------------------------------------------------------------------------------
// MsgProc
// ------------------------------------------------------------------------------------------

LRESULT CALLBACK MsgProc(HWND hWnd,
						 UINT uMsg,
						 WPARAM wParam,
						 LPARAM lParam,
						 bool * pbNoFurtherProcessing,
						 void * pUserContext)
{
	// �����������Ϣ����
	*pbNoFurtherProcessing = g_DialogResourceMgr.MsgProc(hWnd, uMsg, wParam, lParam);
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

// ------------------------------------------------------------------------------------------
// OnKeyboard
// ------------------------------------------------------------------------------------------

void CALLBACK OnKeyboard(UINT nChar,
						 bool bKeyDown,
						 bool bAltDown,
						 void * pUserContext)
{
	// ��������м����¼�����
	// ����DXUT��Դ������Կ��������Ҫ��ֹEscape�Ƴ����ڣ�Ӧ��
	// ��MsgProc����WM_KEYDOWN�е�VK_ESCAPE��������bNoFurtherProcessing�������
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
	// ����crtdbg�����ڴ�й©
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// ����DXUT��Դ����Ļص�����
	DXUTSetCallbackD3D9DeviceAcceptable(IsD3D9DeviceAcceptable);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackD3D9DeviceCreated(OnD3D9CreateDevice);
	DXUTSetCallbackD3D9DeviceReset(OnD3D9ResetDevice);
	DXUTSetCallbackD3D9DeviceLost(OnD3D9LostDevice);
	DXUTSetCallbackD3D9DeviceDestroyed(OnD3D9DestroyDevice);

	// ������Ⱦ�Ļص�����
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackD3D9FrameRender(OnD3D9FrameRender);

	// ������Ϣ�ص�����
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(OnKeyboard);

	// ȫ�ֳ�ʼ������
	g_SettingsDlg.Init(&g_DialogResourceMgr);
	g_HUD.Init(&g_DialogResourceMgr);
	g_HUD.SetCallback(OnGUIEvent);
	int nY = 10;
	g_HUD.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, nY, 125, 22);
	g_HUD.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", 35, nY += 24, 125, 22, VK_F3);
	g_HUD.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", 35, nY += 24, 125, 22, VK_F2);

	// ����DXUT
	DXUTInit(true, true, NULL);
	DXUTSetCursorSettings(true, true);
	WCHAR szPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
	DXUTCreateWindow(szPath);
	DXUTCreateDevice(true, 800, 600);
	DXUTMainLoop();

	// ��ȡ������DXUT�˳�ֵ
	return DXUTGetExitCode();
}

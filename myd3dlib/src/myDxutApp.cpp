
#include "stdafx.h"
#include "myException.h"
#include "myDxutApp.h"
#include <SDKmisc.h>

namespace my
{
	DeviceRelatedObjectBase::DeviceRelatedObjectBase(void)
	{
		DeviceRelatedObjectBaseSet & set = DeviceRelatedObjectBaseSet::getSingleton();
		_ASSERT(set.end() == set.find(this));
		set.insert(this);
	}

	DeviceRelatedObjectBase::~DeviceRelatedObjectBase(void)
	{
		DeviceRelatedObjectBaseSet & set = DeviceRelatedObjectBaseSet::getSingleton();
		DeviceRelatedObjectBaseSet::iterator this_iter = set.find(this);
		_ASSERT(set.end() != this_iter);
		set.erase(this_iter);
	}

	void DeviceRelatedObjectBaseSet::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		iterator obj_iter = begin();
		for(; obj_iter != end(); obj_iter++)
		{
			(*obj_iter)->OnD3D9ResetDevice(pd3dDevice, pBackBufferSurfaceDesc);
		}
	}

	Singleton<DeviceRelatedObjectBaseSet>::DrivedClassPtr DeviceRelatedObjectBaseSet::s_ptr;

	void DeviceRelatedObjectBaseSet::OnD3D9LostDevice(void)
	{
		iterator obj_iter = begin();
		for(; obj_iter != end(); obj_iter++)
		{
			(*obj_iter)->OnD3D9LostDevice();
		}
	}

	void DeviceRelatedObjectBaseSet::OnD3D9DestroyDevice(void)
	{
		iterator obj_iter = begin();
		for(; obj_iter != end(); obj_iter++)
		{
			(*obj_iter)->OnD3D9DestroyDevice();
		}
	}

	bool CALLBACK DxutAppBase::IsD3D9DeviceAcceptable_s(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed,
		void * pUserContext)
	{
		try
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

			return reinterpret_cast<DxutAppBase *>(pUserContext)->IsD3D9DeviceAcceptable(
				pCaps, AdapterFormat, BackBufferFormat, bWindowed);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return false;
	}

	bool CALLBACK DxutAppBase::ModifyDeviceSettings_s(
		DXUTDeviceSettings * pDeviceSettings,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->ModifyDeviceSettings(
				pDeviceSettings);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return false;
	}

	HRESULT CALLBACK DxutAppBase::OnD3D9CreateDevice_s(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9CreateDevice(
				pd3dDevice, pBackBufferSurfaceDesc);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return D3DERR_INVALIDCALL;
	}

	HRESULT CALLBACK DxutAppBase::OnD3D9ResetDevice_s(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9ResetDevice(
				pd3dDevice, pBackBufferSurfaceDesc);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return D3DERR_INVALIDCALL;
	}

	void CALLBACK DxutAppBase::OnD3D9LostDevice_s(
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9LostDevice();
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void CALLBACK DxutAppBase::OnD3D9DestroyDevice_s(
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9DestroyDevice();
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void CALLBACK DxutAppBase::OnFrameMove_s(
		double fTime,
		float fElapsedTime,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnFrameMove(
				fTime, fElapsedTime);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	void CALLBACK DxutAppBase::OnD3D9FrameRender_s(
		IDirect3DDevice9 * pd3dDevice,
		double fTime,
		float fElapsedTime,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnD3D9FrameRender(
				pd3dDevice, fTime, fElapsedTime);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	LRESULT CALLBACK DxutAppBase::MsgProc_s(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing,
		void * pUserContext)
	{
		try
		{
			return reinterpret_cast<DxutAppBase *>(pUserContext)->MsgProc(
				hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
		return 0;
	}

	void CALLBACK DxutAppBase::OnKeyboard_s(
		UINT nChar,
		bool bKeyDown,
		bool bAltDown,
		void * pUserContext)
	{
		try
		{
			reinterpret_cast<DxutAppBase *>(pUserContext)->OnKeyboard(
				nChar, bKeyDown, bAltDown);
		}
		catch(const my::Exception & e)
		{
			MessageBox(DXUTGetHWND(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
		}
	}

	DxutAppBase::DxutAppBase(void)
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

	DxutAppBase::~DxutAppBase(void)
	{
	}

	int DxutAppBase::Run(
		bool bWindowed /*= true*/,
		int nSuggestedWidth /*= 800*/,
		int nSuggestedHeight /*= 600*/)
	{
		DXUTInit(true, true, NULL);
		DXUTSetCursorSettings(true, true);
		WCHAR szPath[MAX_PATH];
		GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
		DXUTCreateWindow(szPath);
		DXUTCreateDevice(bWindowed, nSuggestedWidth, nSuggestedHeight);
		DXUTMainLoop();
		int nExitCode = DXUTGetExitCode();
		DXUTDestroyState();
		return nExitCode;
	}

	SingleInstance<DxutApp> * DxutApp::s_ptr = NULL;

	bool DxutApp::IsD3D9DeviceAcceptable(
		D3DCAPS9 * pCaps,
		D3DFORMAT AdapterFormat,
		D3DFORMAT BackBufferFormat,
		bool bWindowed)
	{
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
		return S_OK;
	}

	HRESULT DxutApp::OnD3D9ResetDevice(
		IDirect3DDevice9 * pd3dDevice,
		const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
	{
		DeviceRelatedObjectBaseSet::getSingleton().OnD3D9ResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

		return S_OK;
	}

	void DxutApp::OnD3D9LostDevice(void)
	{
		DeviceRelatedObjectBaseSet::getSingleton().OnD3D9LostDevice();
	}

	void DxutApp::OnD3D9DestroyDevice(void)
	{
		DeviceRelatedObjectBaseSet::getSingleton().OnD3D9DestroyDevice();
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
	}

	LRESULT DxutApp::MsgProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool * pbNoFurtherProcessing)
	{
		return 0;
	}

	void DxutApp::OnKeyboard(
		UINT nChar,
		bool bKeyDown,
		bool bAltDown)
	{
	}

	void DxutApp::OnInit(void)
	{
	}

	int DxutApp::Run(
		bool bWindowed /*= true*/,
		int nSuggestedWidth /*= 800*/,
		int nSuggestedHeight /*= 600*/)
	{
		try
		{
			OnInit();
		}
		catch(const my::Exception & e)
		{
			MessageBox(GetDesktopWindow(), e.GetFullDescription().c_str(), _T("Exception"), MB_OK);
			return 0;
		}

		return DxutAppBase::Run(bWindowed, nSuggestedWidth, nSuggestedHeight);
	}
};

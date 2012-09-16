#pragma once

#include "mySingleton.h"
#include <d3d9.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "myUi.h"
#include "myThread.h"
#include <atlbase.h>
#include <hash_set>
#include "DXUTenum.h"

namespace my
{
	class DxutWindow
		: public Window
	{
	protected:
		bool m_Minimized;

		bool m_Maximized;

		bool m_InSizeMove;

	public:
		DxutWindow(void)
			: m_Minimized(false)
			, m_Maximized(false)
			, m_InSizeMove(false)
		{
		}

		BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);
	};

	typedef boost::shared_ptr<DxutWindow> DxutWindowPtr;

	class DxutApp
		: public Application
		, public SingleInstance<DxutApp>
	{
	public:
		FT_Library m_Library; // ! Font dependency

		CComPtr<IDirect3DStateBlock9> m_StateBlock; // ! UI dependency

		boost::weak_ptr<Control> m_ControlFocus;

	protected:
		HRESULT hr;

		CComPtr<IDirect3D9> m_d3d9;

		DxutWindowPtr m_wnd;

		CComPtr<IDirect3DDevice9> m_d3dDevice;

		DXUTD3D9DeviceSettings m_DeviceSettings;

		D3DSURFACE_DESC m_BackBufferSurfaceDesc;

		bool m_DeviceObjectsCreated;

		bool m_DeviceObjectsReset;

		UINT m_FullScreenBackBufferWidthAtModeChange;

		UINT m_FullScreenBackBufferHeightAtModeChange;

		UINT m_WindowBackBufferWidthAtModeChange;

		UINT m_WindowBackBufferHeightAtModeChange;

		DWORD m_WindowedStyleAtModeChange;

		WINDOWPLACEMENT m_WindowedPlacement;

		bool m_TopmostWhileWindowed;

		bool m_IgnoreSizeChange;

		bool m_DeviceLost;

		double m_fAbsoluteTime;

		double m_fLastTime;

		DWORD m_dwFrames;

		wchar_t m_strFPS[64];

		LONGLONG m_llQPFTicksPerSec;

		LONGLONG m_llLastElapsedTime;

	public:
		DxutApp(void);

		virtual ~DxutApp(void);

		HWND GetHWND(void)
		{
			return m_wnd->m_hWnd;
		}

		IDirect3D9 * GetD3D9(void)
		{
			return m_d3d9;
		}

		IDirect3DDevice9 * GetD3D9Device(void)
		{
			return m_d3dDevice;
		}

		const D3DSURFACE_DESC & GetD3D9BackBufferSurfaceDesc(void)
		{
			return m_BackBufferSurfaceDesc;
		}

		double GetAbsoluteTime(void)
		{
			return m_fAbsoluteTime;
		}

		virtual bool IsDeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed)
		{
			return true;
		}

		virtual bool ModifyDeviceSettings(
			DXUTD3D9DeviceSettings * pDeviceSettings)
		{
			return true;
		}

		virtual HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
		{
			return S_OK;
		}

		virtual HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
		{
			return S_OK;
		}

		virtual void OnLostDevice(void)
		{
		}

		virtual void OnDestroyDevice(void)
		{
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime)
		{
		}

		virtual void OnFrameRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime)
		{
		}

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing)
		{
			return 0;
		}

		int Run(void);

		void CreateDevice(bool bWindowed, int nSuggestedWidth, int nSuggestedHeight);

		void CheckForWindowSizeChange(void);

		void ToggleFullScreen(void);

		bool CanDeviceBeReset(const DXUTD3D9DeviceSettings & oldDeviceSettings, const DXUTD3D9DeviceSettings & newDeviceSettings);

		void ChangeDevice(DXUTD3D9DeviceSettings & deviceSettings);

		HRESULT Create3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings);

		HRESULT Reset3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings);

		void Render3DEnvironment(void);

		void Cleanup3DEnvironment(void);
	};
};

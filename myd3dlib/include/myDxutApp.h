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
		, public CD3D9Enumeration
	{
	public:
		boost::weak_ptr<Control> m_ControlFocus; // ! UI dependency

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

		DXUTD3D9DeviceSettings GetD3D9DeviceSettings(void)
		{
			return m_DeviceSettings;
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

		HRESULT DXUTFindValidD3D9DeviceSettings(
			DXUTD3D9DeviceSettings * pOut,
			DXUTD3D9DeviceSettings * pIn,
			DXUTMatchOptions * pMatchOptions,
			DXUTD3D9DeviceSettings * pOptimal);

		void DXUTBuildOptimalD3D9DeviceSettings(
			DXUTD3D9DeviceSettings * pOptimalDeviceSettings,
			DXUTD3D9DeviceSettings * pDeviceSettingsIn,
			DXUTMatchOptions * pMatchOptions);

		bool DXUTDoesD3D9DeviceComboMatchPreserveOptions(
			CD3D9EnumDeviceSettingsCombo * pDeviceSettingsCombo,
			DXUTD3D9DeviceSettings * pDeviceSettingsIn,
			DXUTMatchOptions * pMatchOptions);

		float DXUTRankD3D9DeviceCombo(
			CD3D9EnumDeviceSettingsCombo * pDeviceSettingsCombo,
			DXUTD3D9DeviceSettings * pOptimalDeviceSettings,
			D3DDISPLAYMODE * pAdapterDesktopDisplayMode);

		void DXUTBuildValidD3D9DeviceSettings(
			DXUTD3D9DeviceSettings * pValidDeviceSettings,
			CD3D9EnumDeviceSettingsCombo * pBestDeviceSettingsCombo,
			DXUTD3D9DeviceSettings * pDeviceSettingsIn,
			DXUTMatchOptions * pMatchOptions);

		HRESULT DXUTFindValidD3D9Resolution(
			CD3D9EnumDeviceSettingsCombo * pBestDeviceSettingsCombo,
			D3DDISPLAYMODE displayModeIn,
			D3DDISPLAYMODE * pBestDisplayMode);

		static const char * DXUTD3DDeviceTypeToString(D3DDEVTYPE devType);

		static const char * DXUTD3DFormatToString(D3DFORMAT format);

		static const char * DXUTMultisampleTypeToString(D3DMULTISAMPLE_TYPE MultiSampleType);

		static const char * DXUTVertexProcessingTypeToString(DWORD vpt);

		bool GetSoftwareVP(void) const
		{
			return m_bSoftwareVP;
		}

		void SetSoftwareVP(bool value)
		{
			m_bSoftwareVP = value;
		}

		bool GetHardwareVP(void) const
		{
			return m_bHardwareVP;
		}

		void SetHardwareVP(bool value)
		{
			m_bHardwareVP = value;
		}

		bool GetPureHardwareVP(void) const
		{
			return m_bPureHarewareVP;
		}

		void SetPureHardwareVP(bool value)
		{
			m_bPureHarewareVP = value;
		}

		bool GetMixedVP(void) const
		{
			return m_bMixedVP;
		}

		void SetMixedVP(bool value)
		{
			m_bMixedVP = value;
		}

		DXUTD3D9DeviceSettings FindValidDeviceSettings(const DXUTD3D9DeviceSettings & deviceSettings, const DXUTMatchOptions & matchOptions);

		void CreateDevice(bool bWindowed, int nSuggestedWidth, int nSuggestedHeight);

		void CheckForWindowSizeChange(void);

		void ToggleFullScreen(void);

		void ToggleREF(void);

		bool CanDeviceBeReset(const DXUTD3D9DeviceSettings & oldDeviceSettings, const DXUTD3D9DeviceSettings & newDeviceSettings);

		void ChangeDevice(DXUTD3D9DeviceSettings & deviceSettings);

		HRESULT Create3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings);

		HRESULT Reset3DEnvironment(const DXUTD3D9DeviceSettings & deviceSettings);

		void Render3DEnvironment(void);

		void Cleanup3DEnvironment(void);
	};
};

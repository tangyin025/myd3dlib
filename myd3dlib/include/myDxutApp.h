#pragma once

#include "mySingleton.h"
#include "myUi.h"
#include "myThread.h"
#include <atlbase.h>
#include "DXUTenum.h"
#include <boost/signals2.hpp>

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

	class Clock
	{
	public:
		LONGLONG m_llQPFTicksPerSec;

		LONGLONG m_llLastElapsedTime;

		double m_fAbsoluteTime;

		float m_fElapsedTime;

		Clock(void);

		void UpdateClock(void);
	};

	class D3DContext
		: public SingleInstance<D3DContext>
	{
	public:
		CComPtr<IDirect3D9> m_d3d9;

		DWORD m_d3dThreadId;

		CComPtr<IDirect3DDevice9> m_d3dDevice;

		DXUTD3D9DeviceSettings m_DeviceSettings;

		D3DSURFACE_DESC m_BackBufferSurfaceDesc;

		bool m_DeviceObjectsCreated;

		bool m_DeviceObjectsReset;

		typedef boost::signals2::signal<void (void)> Event;

		Event m_EventDeviceReset;

		Event m_EventDeviceLost;

		Event m_EventDeviceDestroy;

	public:
		D3DContext(void)
			: m_d3dThreadId(-1)
			, m_DeviceObjectsCreated(false)
			, m_DeviceObjectsReset(false)
		{
		}
	};

	class DxutApp
		: public D3DContext
		, public Clock
		, public Application
		, public CD3D9Enumeration
	{
	public:
		HRESULT hr;

		DxutWindowPtr m_wnd;

		UINT m_FullScreenBackBufferWidthAtModeChange;

		UINT m_FullScreenBackBufferHeightAtModeChange;

		UINT m_WindowBackBufferWidthAtModeChange;

		UINT m_WindowBackBufferHeightAtModeChange;

		DWORD m_WindowedStyleAtModeChange;

		WINDOWPLACEMENT m_WindowedPlacement;

		bool m_IgnoreSizeChange;

		bool m_DeviceLost;

		DWORD m_dwFrames;

		double m_fLastTime;

		float m_fFps;

	public:
		DxutApp(void)
			: m_FullScreenBackBufferWidthAtModeChange(0)
			, m_FullScreenBackBufferHeightAtModeChange(0)
			, m_WindowBackBufferWidthAtModeChange(800)
			, m_WindowBackBufferHeightAtModeChange(600)
			, m_IgnoreSizeChange(false)
			, m_DeviceLost(false)
			, m_dwFrames(0)
			, m_fLastTime(0)
			, m_fFps(0)
		{
		}

		static DxutApp & getSingleton(void)
		{
			return *getSingletonPtr();
		}

		static DxutApp * getSingletonPtr(void)
		{
			return static_cast<DxutApp *>(D3DContext::getSingletonPtr());
		}

		virtual bool IsDeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed);

		virtual bool ModifyDeviceSettings(
			DXUTD3D9DeviceSettings * pDeviceSettings);

		virtual HRESULT OnCreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual HRESULT OnResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnLostDevice(void);

		virtual void OnDestroyDevice(void);

		virtual void OnFrameTick(
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

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

		HRESULT Create3DEnvironment(DXUTD3D9DeviceSettings & deviceSettings);

		HRESULT Reset3DEnvironment(DXUTD3D9DeviceSettings & deviceSettings);

		void Render3DEnvironment(void);

		void Present(CONST RECT* pSourceRect=NULL,CONST RECT* pDestRect=NULL,HWND hDestWindowOverride=NULL,CONST RGNDATA* pDirtyRegion=NULL);

		void Cleanup3DEnvironment(void);
	};
}

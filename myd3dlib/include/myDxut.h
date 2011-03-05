
#pragma once

#include "mySingleton.h"
#include <atlbase.h>
#include <DXUT.h>
#include <DXUTgui.h>
#include <DXUTSettingsDlg.h>

namespace my
{
	class DxutApp : public Singleton<DxutApp>
	{
	protected:
		CDXUTDialogResourceManager m_dlgResourceMgr;

		CD3DSettingsDlg m_settingsDlg;

		CDXUTDialog m_hudDlg;

		CComPtr<ID3DXFont> m_txtFont;

		CComPtr<ID3DXSprite> m_txtSprite;

	public:
		static bool CALLBACK IsD3D9DeviceAcceptable_s(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed,
			void * pUserContext)
		{
			return reinterpret_cast<DxutApp *>(pUserContext)->IsD3D9DeviceAcceptable(
				pCaps, AdapterFormat, BackBufferFormat, bWindowed);
		}

		virtual bool IsD3D9DeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed);

		static bool CALLBACK ModifyDeviceSettings_s(
			DXUTDeviceSettings * pDeviceSettings,
			void * pUserContext)
		{
			return reinterpret_cast<DxutApp *>(pUserContext)->ModifyDeviceSettings(
				pDeviceSettings);
		}

		virtual bool ModifyDeviceSettings(
			DXUTDeviceSettings * pDeviceSettings);

		static HRESULT CALLBACK OnD3D9CreateDevice_s(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
			void * pUserContext)
		{
			return reinterpret_cast<DxutApp *>(pUserContext)->OnD3D9CreateDevice(
				pd3dDevice, pBackBufferSurfaceDesc);
		}

		virtual HRESULT OnD3D9CreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		static HRESULT CALLBACK OnD3D9ResetDevice_s(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
			void * pUserContext)
		{
			return reinterpret_cast<DxutApp *>(pUserContext)->OnD3D9ResetDevice(
				pd3dDevice, pBackBufferSurfaceDesc);
		}

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		static void CALLBACK OnD3D9LostDevice_s(
			void * pUserContext)
		{
			reinterpret_cast<DxutApp *>(pUserContext)->OnD3D9LostDevice();
		}

		virtual void OnD3D9LostDevice(void);

		static void CALLBACK OnD3D9DestroyDevice_s(
			void * pUserContext)
		{
			reinterpret_cast<DxutApp *>(pUserContext)->OnD3D9DestroyDevice();
		}

		virtual void OnD3D9DestroyDevice(void);

		static void CALLBACK OnFrameMove_s(
			double fTime,
			float fElapsedTime,
			void * pUserContext)
		{
			reinterpret_cast<DxutApp *>(pUserContext)->OnFrameMove(
				fTime, fElapsedTime);
		}

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		static void CALLBACK OnD3D9FrameRender_s(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime,
			void * pUserContext)
		{
			reinterpret_cast<DxutApp *>(pUserContext)->OnD3D9FrameRender(
				pd3dDevice, fTime, fElapsedTime);
		}

		virtual void OnD3D9FrameRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);

		static void CALLBACK OnGUIEvent_s(
			UINT nEvent,
			int nControlID,
			CDXUTControl * pControl,
			void * pUserContext)
		{
			reinterpret_cast<DxutApp *>(pUserContext)->OnGUIEvent(
				nEvent, nControlID, pControl);
		}

		virtual void OnGUIEvent(
			UINT nEvent,
			int nControlID,
			CDXUTControl * pControl);

		static LRESULT CALLBACK MsgProc_s(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing,
			void * pUserContext)
		{
			return reinterpret_cast<DxutApp *>(pUserContext)->MsgProc(
				hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);
		}

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		static void CALLBACK OnKeyboard_s(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown,
			void * pUserContext)
		{
			reinterpret_cast<DxutApp *>(pUserContext)->OnKeyboard(
				nChar, bKeyDown, bAltDown);
		}

		virtual void OnKeyboard(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown);

	public:
		DxutApp(void);

		int Run(bool bWindowed, int nSuggestedWidth, int nSuggestedHeight);

		virtual void OnInit(void);
	};
};

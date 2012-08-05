#pragma once

#include "mySingleton.h"
#include <d3d9.h>
#include <DXUT.h>
#include <DXUTgui.h>
#include <DXUTsettingsdlg.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "myUi.h"
#include <atlbase.h>
#include <hash_set>

namespace my
{
	class DxutApp : public SingleInstance<DxutApp>
	{
	protected:
		HRESULT hr;

		static bool CALLBACK IsD3D9DeviceAcceptable_s(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed,
			void * pUserContext);

		static bool CALLBACK ModifyDeviceSettings_s(
			DXUTDeviceSettings * pDeviceSettings,
			void * pUserContext);

		static HRESULT CALLBACK OnD3D9CreateDevice_s(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
			void * pUserContext);

		static HRESULT CALLBACK OnD3D9ResetDevice_s(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc,
			void * pUserContext);

		static void CALLBACK OnD3D9LostDevice_s(
			void * pUserContext);

		static void CALLBACK OnD3D9DestroyDevice_s(
			void * pUserContext);

		static void CALLBACK OnFrameMove_s(
			double fTime,
			float fElapsedTime,
			void * pUserContext);

		static void CALLBACK OnD3D9FrameRender_s(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime,
			void * pUserContext);

		static LRESULT CALLBACK MsgProc_s(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing,
			void * pUserContext);

		static void CALLBACK OnKeyboard_s(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown,
			void * pUserContext);

	protected:
		virtual bool IsD3D9DeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed);

		virtual bool ModifyDeviceSettings(
			DXUTDeviceSettings * pDeviceSettings);

		virtual HRESULT OnD3D9CreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

		virtual void OnD3D9DestroyDevice(void);

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual void OnD3D9FrameRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

		virtual void OnKeyboard(
			UINT nChar,
			bool bKeyDown,
			bool bAltDown);

	public:
		FT_Library m_Library; // ! Font dependency

		CComPtr<IDirect3DStateBlock9> m_StateBlock; // ! UI dependency

		boost::weak_ptr<Control> m_ControlFocus;

		CComPtr<ID3DXEffectPool> m_EffectPool; // ! Effect dependency

		typedef stdext::hash_set<DeviceRelatedObjectBase *> DeviceRelatedObjectBasePtrSet;

		DeviceRelatedObjectBasePtrSet m_deviceRelatedObjs; // ! DeviceRelatedObject dependency

		void RegisterDeviceRelatedObject(DeviceRelatedObjectBase * obj)
		{
			_ASSERT(m_deviceRelatedObjs.end() == m_deviceRelatedObjs.find(obj));

			m_deviceRelatedObjs.insert(obj);
		}

		void UnregisterDeviceRelatedObject(DeviceRelatedObjectBase * obj)
		{
			_ASSERT(m_deviceRelatedObjs.end() != m_deviceRelatedObjs.find(obj));

			m_deviceRelatedObjs.erase(m_deviceRelatedObjs.find(obj));
		}

	public:
		DxutApp(void);

		virtual ~DxutApp(void);

		virtual int Run(
			bool bWindowed = true,
			int nSuggestedWidth = 800,
			int nSuggestedHeight = 600);

		HWND GetHWND(void)
		{
			return DXUTGetHWND();
		}

		IDirect3DDevice9 * GetD3D9Device(void)
		{
			return DXUTGetD3D9Device();
		}

		double GetAbsoluteTime(void)
		{
			return DXUTGetGlobalTimer()->GetAbsoluteTime();
		}

		double GetTime(void)
		{
			return DXUTGetTime();
		}
	};

	class DxutSample : public DxutApp
	{
	protected:
		enum
		{
			IDC_TOGGLEFULLSCREEN,
			IDC_TOGGLEREF,
			IDC_CHANGEDEVICE
		};

		CDXUTDialogResourceManager m_dlgResourceMgr;

		CD3DSettingsDlg m_settingsDlg;

		CDXUTDialog m_hudDlg;

		CComPtr<ID3DXFont> m_font;

		CComPtr<ID3DXSprite> m_sprite;

		virtual bool IsD3D9DeviceAcceptable(
			D3DCAPS9 * pCaps,
			D3DFORMAT AdapterFormat,
			D3DFORMAT BackBufferFormat,
			bool bWindowed);

		virtual bool ModifyDeviceSettings(
			DXUTDeviceSettings * pDeviceSettings);

		virtual HRESULT OnD3D9CreateDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual HRESULT OnD3D9ResetDevice(
			IDirect3DDevice9 * pd3dDevice,
			const D3DSURFACE_DESC * pBackBufferSurfaceDesc);

		virtual void OnD3D9LostDevice(void);

		virtual void OnD3D9DestroyDevice(void);

		virtual void OnFrameMove(
			double fTime,
			float fElapsedTime);

		virtual void OnD3D9FrameRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);

		virtual void OnRender(
			IDirect3DDevice9 * pd3dDevice,
			double fTime,
			float fElapsedTime);

		static void CALLBACK OnGUIEvent_s(
			UINT nEvent,
			int nControlID,
			CDXUTControl * pControl,
			void * pUserContext);

		virtual void OnGUIEvent(
			UINT nEvent,
			int nControlID,
			CDXUTControl * pControl);

		virtual LRESULT MsgProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			bool * pbNoFurtherProcessing);

	public:
		DxutSample(void);

		~DxutSample(void);
	};
};

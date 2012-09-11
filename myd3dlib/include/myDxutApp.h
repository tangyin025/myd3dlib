#pragma once

#include "mySingleton.h"
#include <d3d9.h>
#include <DXUT.h>
#include <DXUTgui.h>
#include <DXUTsettingsdlg.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "myUi.h"
#include "myThread.h"
#include <atlbase.h>
#include <hash_set>

namespace my
{
	class DxutWindow
		: public Window
	{
	public:
		BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);
	};

	typedef boost::shared_ptr<DxutWindow> DxutWindowPtr;

	class DxutApplication
		: public Application
		, public SingleInstance<DxutApplication>
	{
	public:
		HRESULT hr;

		FT_Library m_Library; // ! Font dependency

		CComPtr<IDirect3DStateBlock9> m_StateBlock; // ! UI dependency

		boost::weak_ptr<Control> m_ControlFocus;

		typedef stdext::hash_set<DeviceRelatedObjectBase *> DeviceRelatedObjectBasePtrSet;

		DeviceRelatedObjectBasePtrSet m_deviceRelatedObjs; // ! DeviceRelatedObject dependency

		void RegisterDeviceRelatedObject(DeviceRelatedObjectBase * obj)
		{
			_ASSERT(m_deviceRelatedObjs.end() == m_deviceRelatedObjs.find(obj));

			m_deviceRelatedObjs.insert(obj);
		}

		void UnregisterDeviceRelatedObject(DeviceRelatedObjectBase * obj)
		{
			DeviceRelatedObjectBasePtrSet::iterator obj_iter = m_deviceRelatedObjs.find(obj);
			if(obj_iter != m_deviceRelatedObjs.end())
			{
				m_deviceRelatedObjs.erase(obj_iter);
			}
		}

		HWND GetHWND(void)
		{
			return m_wnd->m_hWnd;
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
			return 0;
		}

		double GetTime(void)
		{
			return 0;
		}

	public:
		CComPtr<IDirect3D9> m_d3d9;

		CComPtr<IDirect3DDevice9> m_d3dDevice;

		DxutWindowPtr m_wnd;

		D3DSURFACE_DESC m_BackBufferSurfaceDesc;

	public:
		DxutApplication(void);

		virtual ~DxutApplication(void);

		int Run(void);

	public:
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
	};
};

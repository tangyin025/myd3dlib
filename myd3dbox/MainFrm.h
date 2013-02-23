#pragma once

#include "OutlinerView.h"

class CMainFrame
	: public CFrameWndEx
	, public my::SingleInstance<CMainFrame>
	, public my::ResourceMgr
{
public:
	CMainFrame(void);

	DECLARE_DYNCREATE(CMainFrame)

	UINT  m_nAppLook;

	CMFCMenuBar m_wndMenuBar;

	CMFCToolBar m_wndToolBar;

	CMFCStatusBar m_wndStatusBar;

	COutlinerView m_wndOutliner;

	D3DPRESENT_PARAMETERS m_d3dpp;

	CComPtr<IDirect3DDevice9> m_d3dDevice;

	bool m_DeviceObjectsCreated;

	bool m_DeviceObjectsReset;

	DECLARE_MESSAGE_MAP()

	IDirect3DDevice9 * GetD3D9Device(void)
	{
		return m_d3dDevice;
	}

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnViewCustomize();

	afx_msg void OnApplicationLook(UINT id);

	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	afx_msg void OnDestroy();
public:
	HRESULT ResetD3DDevice(void);

	HRESULT OnDeviceReset(void);

	void OnDeviceLost(void);
};

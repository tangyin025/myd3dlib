#pragma once

class CMainFrame : public CFrameWndEx
{
public:
	CMainFrame(void);

	DECLARE_DYNCREATE(CMainFrame)

	CMFCMenuBar m_wndMenuBar;

	CMFCToolBar m_wndToolBar;

	CMFCStatusBar m_wndStatusBar;

	CComPtr<IDirect3DDevice9> m_d3dDevice;

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnViewCustomize();

	afx_msg void OnApplicationLook(UINT id);

	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	afx_msg void OnDestroy();
};

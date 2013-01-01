#pragma once

class CMainFrame : public CFrameWndEx
{
public:
	CMainFrame(void);

	DECLARE_DYNCREATE(CMainFrame)

	UINT m_nAppLook;

	CMFCMenuBar m_wndMenuBar;

	CMFCToolBar m_wndToolBar;

	CMFCStatusBar m_wndStatusBar;

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnViewCustomize();

	afx_msg void OnApplicationLook(UINT id);

	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
};

// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "FileView.h"
#include "PropertiesWnd.h"
#include "AtlasView.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame(void);

	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnWindowManager(void);

	afx_msg void OnViewCustomize(void);

	afx_msg void OnApplicationLook(UINT id);

	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);

public:
	CMFCMenuBar m_wndMenuBar;

	CMFCToolBar m_wndToolBar;

	CMFCStatusBar m_wndStatusBar;

	CFileView m_wndFileView;

	CPropertiesWnd m_wndProperties;

	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnToolbarReset(WPARAM wp,LPARAM lp);

	CAtlasView m_wndAtlas;
};

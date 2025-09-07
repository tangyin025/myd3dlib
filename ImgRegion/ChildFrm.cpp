// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "stdafx.h"
#include "ChildFrm.h"
#include "MainApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
END_MESSAGE_MAP()

CChildFrame::CChildFrame(void)
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	//cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
	//	| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	return TRUE;
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
	// TODO: Add your specialized code here and/or call the base class
	CMDIFrameWnd* pFrameWnd = GetMDIFrame();
	ASSERT_VALID(pFrameWnd);

	// get maximized state of frame window (previously active child)
	BOOL bMaximized;
	CMDIChildWnd * pChild = pFrameWnd->MDIGetActive(&bMaximized);
	if(pChild)
	{
		theApp.WriteProfileInt(_T("Settings"), _T("ChildCmdShow"), bMaximized);
	}
	else
	{
		bMaximized = theApp.GetProfileInt(_T("Settings"), _T("ChildCmdShow"), WS_MAXIMIZE);
	}

	// convert show command based on current style
	DWORD dwStyle = GetStyle();
	if (bMaximized || (dwStyle & WS_MAXIMIZE))
		nCmdShow = SW_SHOWMAXIMIZED;
	else if (dwStyle & WS_MINIMIZE)
		nCmdShow = SW_SHOWMINIMIZED;

	CMDIChildWndEx::ActivateFrame(nCmdShow);
}

BOOL CChildFrame::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	CMDIFrameWnd* pFrameWnd = GetMDIFrame();
	ASSERT_VALID(pFrameWnd);

	// get maximized state of frame window (previously active child)
	BOOL bMaximized;
	CMDIChildWnd * pChild = pFrameWnd->MDIGetActive(&bMaximized);
	if(pChild && this == pChild)
	{
		theApp.WriteProfileInt(_T("Settings"), _T("ChildCmdShow"), bMaximized);
	}

	return CMDIChildWndEx::DestroyWindow();
}

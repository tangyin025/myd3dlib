// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

class CChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrame)

public:
	CChildFrame(void);

	DECLARE_MESSAGE_MAP()

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	virtual void ActivateFrame(int nCmdShow = -1);

	virtual BOOL DestroyWindow();
};

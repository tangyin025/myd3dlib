#pragma once

#include <afxcontrolbars.h>
#include <afxmdiframewndex.h>

class CMainFrame : public CMDIFrameWndEx
{
public:
	CMainFrame(void);

	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};

#include "myDxutApp11.h"

using namespace my;

BOOL DxutWindow11::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
{
	switch (dwMsgMapID)
	{
	case 0:
		switch (uMsg)
		{
		case WM_CREATE:
			ATLASSERT(m_hWnd);
			lResult = 0;
			return TRUE;

		case WM_DESTROY:
			lResult = 0;
			return TRUE;

		case WM_SIZE:
			switch (wParam)
			{
			case SIZE_MINIMIZED:
				m_Minimized = true;
				break;

			case SIZE_MAXIMIZED:
				//DxutApp::getSingleton().CheckForWindowSizeChange();
				m_Maximized = true;
				break;

			case SIZE_RESTORED:
				if (!m_InSizeMove)
				{
					//DxutApp::getSingleton().CheckForWindowSizeChange();
					m_Minimized = false;
					m_Maximized = false;
				}
				break;
			}
			break;

		case WM_GETMINMAXINFO:
			((MINMAXINFO *)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO *)lParam)->ptMinTrackSize.y = 200;
			break;

		case WM_ENTERSIZEMOVE:
			m_InSizeMove = true;
			break;

		case WM_EXITSIZEMOVE:
			//DxutApp::getSingleton().CheckForWindowSizeChange();
			m_InSizeMove = false;
			break;

		case WM_ACTIVATEAPP:
			m_ActivateEvent(wParam != 0);
			break;

		case WM_MENUCHAR:
			lResult = MAKELRESULT(0, MNC_CLOSE);
			return TRUE;

		case WM_SYSKEYDOWN:
			switch (wParam)
			{
			case VK_RETURN:
				if (GetKeyState(VK_MENU))
				{
					//DxutApp::getSingleton().ToggleFullScreen();
					lResult = 0;
					return TRUE;
				}
				break;
			}
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			// goto default

		default:
			bool bNoFurtherProcessing = false;
			//lResult = DxutApp::getSingleton().MsgProc(hWnd, uMsg, wParam, lParam, &bNoFurtherProcessing);
			if (bNoFurtherProcessing)
			{
				switch (uMsg)
				{
				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
					SetCapture();
					break;
				}
				return TRUE;
			}
		}
		break;

	default:
		ATLTRACE(ATL::atlTraceWindowing, 0, _T("Invalid message map ID (%i)\n"), dwMsgMapID);
		ATLASSERT(FALSE);
		break;
	}
	return FALSE;
}

int DxutApp11::Run(void)
{
	m_wnd.reset(new DxutWindow11());
	CRect desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	CRect clientRect(0, 0, 800, 600);
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
	clientRect.MoveToXY((desktopRect.Width() - clientRect.Width()) / 2, (desktopRect.Height() - clientRect.Height()) / 2);
	m_wnd->Create(NULL, clientRect, GetModuleFileName().c_str());
	m_wnd->ShowWindow(SW_SHOW);
	m_wnd->UpdateWindow();

	MSG msg = { 0 };
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message)
	{
		if (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else
		{
			//Render3DEnvironment();
		}
	}
	return 0;
}

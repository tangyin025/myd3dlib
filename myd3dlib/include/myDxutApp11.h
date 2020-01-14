#pragma once

#include "myThread.h"
#include <boost/signals2.hpp>

namespace my
{
	class DxutWindow11
		: public Window
	{
	public:
		bool m_Minimized;

		bool m_Maximized;

		bool m_InSizeMove;

		typedef boost::signals2::signal<void(bool)> ActivateEvent;

		ActivateEvent m_ActivateEvent;

	public:
		DxutWindow11(void)
			: m_Minimized(false)
			, m_Maximized(false)
			, m_InSizeMove(false)
		{
		}

		BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);
	};

	class DxutApp11
		: public Application
	{
	public:
		boost::shared_ptr<DxutWindow11> m_wnd;

	public:
		DxutApp11(void)
		{
		}

		int Run(void);
	};
}

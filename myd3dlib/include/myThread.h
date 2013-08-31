#pragma once

#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <string>
#include <boost/shared_ptr.hpp>

namespace my
{
	class CriticalSection
	{
	protected:
		CRITICAL_SECTION m_section;

	public:
		CriticalSection(void);

		~CriticalSection(void);

		void Enter(void);

		void Leave(void);

		BOOL TryEnterCriticalSection(void);
	};

	class CriticalSectionLock
	{
	protected:
		CriticalSection & m_cs;

	public:
		CriticalSectionLock(CriticalSection & cs);

		~CriticalSectionLock(void);
	};

	class Event
	{
	public:
		HANDLE m_hevent;

		BOOL bres;

	public:
		Event(LPSECURITY_ATTRIBUTES lpEventAttributes = NULL, BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPCTSTR lpName = NULL);

		~Event(void);

		void ResetEvent(void);

		void SetEvent(void);

		bool WaitEvent(DWORD dwMilliseconds = INFINITE);
	};

	class Thread
	{
	protected:
		HANDLE m_hThread;

	protected:
		static DWORD WINAPI ThreadProc(__in LPVOID lpParameter);

		virtual DWORD OnProc(void) = 0;

	public:
		Thread(void);

		~Thread(void);

		void CreateThread(DWORD dwCreationFlags = CREATE_SUSPENDED);

		void ResumeThread(void);

		void SuspendThread(void);

		void SetThreadPriority(int nPriority);

		int GetThreadPriority(void);

		void TerminateThread(DWORD dwExitCode);

		bool WaitForThreadStopped(DWORD dwMilliseconds = INFINITE);
	};

	class Window
		: public CWindowImpl<Window, CWindow, CWinTraits<WS_OVERLAPPEDWINDOW, 0> >
	{
	public:
		static std::string GetWindowMessageStr(UINT message);

	public:
		Window(void);

		virtual ~Window(void); // ! virtual distructor can avoid message processing after being destroyed

		LONG SetStyle(LONG dwStyle);

		LONG SetExStyle(LONG dwExStyle);

		BOOL AdjustClientRect(const CRect & rect);

		DECLARE_WND_CLASS_EX(GetWndClassName(), CS_DBLCLKS, -1);

		DECLARE_EMPTY_MSG_MAP();

		void OnFinalMessage(HWND hwnd);
	};

	typedef boost::shared_ptr<Window> WindowPtr;

	class Application
	{
	public:
		HINSTANCE m_hinst;

	public:
		Application(HINSTANCE hInstance = ::GetModuleHandle(NULL));

		virtual ~Application(void);

		HINSTANCE GetHandle(void) const;

		static std::basic_string<TCHAR> GetModuleFileName(void);

		int Run(void);
	};

	typedef boost::shared_ptr<Application> ApplicationPtr;
}

#pragma once

#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace my
{
	class CriticalSection
	{
	protected:
		friend class ConditionVariable;

		CRITICAL_SECTION m_section;

	public:
		CriticalSection(void);

		~CriticalSection(void);

		void Enter(void);

		void Leave(void);

		BOOL TryEnter(void);
	};

	class CriticalSectionLock
	{
	protected:
		CriticalSection & m_cs;

		bool m_locked;

	public:
		CriticalSectionLock(CriticalSection & cs, bool init_lock = true)
			: m_cs(cs)
			, m_locked(false)
		{
			if (init_lock)
			{
				Lock();
			}
		}

		~CriticalSectionLock(void)
		{
			if (m_locked)
			{
				Unlock();
			}
		}

		void Lock(void)
		{
			_ASSERT(!m_locked); m_cs.Enter(); m_locked = true;
		}

		void Unlock(void)
		{
			_ASSERT(m_locked); m_cs.Leave(); m_locked = false;
		}
	};

	class SynchronizationObj
	{
	public:
		HANDLE m_handle;

	public:
		SynchronizationObj(HANDLE handle);

		~SynchronizationObj(void);

		BOOL Wait(DWORD dwMilliseconds = INFINITE);
	};

	class Event : public SynchronizationObj
	{
	public:
		Event(LPSECURITY_ATTRIBUTES lpEventAttributes = NULL, BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPCTSTR lpName = NULL);

		void ResetEvent(void);

		void SetEvent(void);
	};

	class Mutex : public SynchronizationObj
	{
	public:
		Mutex(LPSECURITY_ATTRIBUTES lpMutexAttributes = NULL, BOOL bInitialOwner = FALSE, LPCTSTR lpName = NULL);

		void Release(void);
	};

	class MutexLock
	{
	protected:
		Mutex & m_mutex;

		bool m_locked;

	public:
		MutexLock(Mutex & mutex, bool init_lock = true)
			: m_mutex(mutex)
			, m_locked(false)
		{
			if (init_lock)
			{
				Lock();
			}
		}

		~MutexLock(void)
		{
			if (m_locked)
			{
				Unlock();
			}
		}

		void Lock(void)
		{
			_ASSERT(!m_locked); m_mutex.Wait(INFINITE); m_locked = true;
		}

		void Unlock(void)
		{
			_ASSERT(m_locked); m_mutex.Release(); m_locked = false;
		}
	};

	class Semaphore : public SynchronizationObj
	{
	public:
		Semaphore(LONG lInitialCount, LONG lMaximumCount, LPSECURITY_ATTRIBUTES lpSemaphoreAttributes = NULL, LPCTSTR lpName = NULL);

		LONG Release(LONG lReleaseCount);
	};

	class ConditionVariable
	{
	protected:
		Semaphore m_sema;

	public:
		ConditionVariable(void);

		~ConditionVariable(void);

		BOOL Sleep(SynchronizationObj & obj, DWORD dwMilliseconds = INFINITE, BOOL bAlertable = FALSE);

		void Wake(LONG lReleaseCount);
	};

	typedef boost::function<DWORD (void)> ThreadCallback;

	class Thread : public SynchronizationObj
	{
	protected:
		ThreadCallback m_Callback;

		static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	public:
		Thread(const ThreadCallback & Callback);

		void CreateThread(DWORD dwCreationFlags = CREATE_SUSPENDED);

		DWORD GetThreadId(void) const;

		void ResumeThread(void);

		void SuspendThread(void);

		void SetThreadPriority(int nPriority);

		int GetThreadPriority(void);

		void TerminateThread(DWORD dwExitCode);

		BOOL WaitForThreadStopped(DWORD dwMilliseconds = INFINITE);

		void CloseThread(void);
	};

	typedef boost::shared_ptr<Thread> ThreadPtr;

	class Window
		: public CWindowImpl<Window, CWindow, CWinTraits<WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 0> >
	{
	public:
		static const char * GetWindowMessageStr(UINT message);

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

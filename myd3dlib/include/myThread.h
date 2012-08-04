#pragma once

namespace my
{
	class CriticalSection
	{
	protected:
		CRITICAL_SECTION m_section;

	public:
		CriticalSection(void)
		{
			::InitializeCriticalSection(&m_section);
		}

		~CriticalSection(void)
		{
			::DeleteCriticalSection(&m_section);
		}

		void enter(void)
		{
			::EnterCriticalSection(&m_section);
		}

		void leave(void)
		{
			::LeaveCriticalSection(&m_section);
		}

		BOOL TryEnterCriticalSection(void)
		{
			return ::TryEnterCriticalSection(&m_section);
		}
	};

	class CriticalSectionLock
	{
	protected:
		CriticalSection & m_cs;

	public:
		CriticalSectionLock(CriticalSection & cs)
			: m_cs(cs)
		{
			m_cs.enter();
		}

		~CriticalSectionLock(void)
		{
			m_cs.leave();
		}
	};

	class Event
	{
	public:
		HANDLE m_hevent;

		BOOL bres;

	public:
		Event(
			LPSECURITY_ATTRIBUTES lpEventAttributes = NULL,
			BOOL bManualReset = FALSE,
			BOOL bInitialState = FALSE,
			LPCTSTR lpName = NULL)
		{
			m_hevent = ::CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
		}

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
}

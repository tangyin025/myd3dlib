
#include "StdAfx.h"
#include "myThread.h"
#include "myException.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

using namespace my;

Event::~Event(void)
{
	::CloseHandle(m_hevent);
}

void Event::ResetEvent(void)
{
	bres = ::ResetEvent(m_hevent); _ASSERT(bres);
}

void Event::SetEvent(void)
{
	bres = ::SetEvent(m_hevent); _ASSERT(bres);
}

bool Event::WaitEvent(DWORD dwMilliseconds)
{
	return WAIT_TIMEOUT != ::WaitForSingleObject(m_hevent, dwMilliseconds);
}

DWORD WINAPI Thread::ThreadProc(__in LPVOID lpParameter)
{
	Thread * pThread = reinterpret_cast<Thread *>(lpParameter);

	return pThread->OnProc();
}

Thread::Thread(void)
	: m_hThread(NULL)
{
}

Thread::~Thread(void)
{
	if(NULL != m_hThread)
	{
		_ASSERT(WAIT_TIMEOUT != ::WaitForSingleObject(m_hThread, 0));

		//// The thread object remains in the system until the thread has terminated
		//// and all handles to it have been closed through a call to CloseHandle.
		//::CloseHandle(m_hThread);
	}
}

void Thread::CreateThread(DWORD dwCreationFlags)
{
	_ASSERT(NULL == m_hThread);

	if(NULL == (m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, dwCreationFlags, NULL)))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}
}

void Thread::ResumeThread(void)
{
	_ASSERT(NULL != m_hThread);

	if(-1 == ::ResumeThread(m_hThread))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}
}

void Thread::SuspendThread(void)
{
	_ASSERT(NULL != m_hThread);

	if(-1 == ::SuspendThread(m_hThread))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}
}

void Thread::SetThreadPriority(int nPriority)
{
	_ASSERT(NULL != m_hThread);

	if(!::SetThreadPriority(m_hThread, nPriority))
	{
		THROW_WINEXCEPTION(::GetLastError());
	}
}

int Thread::GetThreadPriority(void)
{
	_ASSERT(NULL != m_hThread);

	int nRet = ::GetThreadPriority(m_hThread);

	if(THREAD_PRIORITY_ERROR_RETURN == nRet)
	{
		THROW_WINEXCEPTION(::GetLastError());
	}

	return nRet;
}

void Thread::TerminateThread(DWORD dwExitCode)
{
	_ASSERT(!"you should not use TerminateThread anymore");

	_ASSERT(NULL != m_hThread);

	if(!::TerminateThread(m_hThread, dwExitCode))
	{
		//THROW_WINEXCEPTION(::GetLastError());
	}
}

bool Thread::WaitForThreadStopped(DWORD dwMilliseconds)
{
	if(WAIT_TIMEOUT == ::WaitForSingleObject(m_hThread, dwMilliseconds))
	{
		return false;
	}

	return true;
}

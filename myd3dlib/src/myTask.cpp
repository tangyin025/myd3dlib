#include "myTask.h"
#include <boost/bind.hpp>

using namespace my;

ParallelTaskManager::ParallelTaskManager(void)
	: m_bStopped(false)
{
}

bool ParallelTaskManager::ParallelThreadDoTask(void)
{
	if(!m_Tasks.empty())
	{
		ParallelTask * task = m_Tasks.back();

		m_Tasks.pop_back();

		m_TasksMutex.Release();

		task->DoTask();

		return true;
	}
	return false;
}

DWORD ParallelTaskManager::ParallelThreadFunc(int i)
{
	m_TasksMutex.Wait(INFINITE);
	while(!m_bStopped)
	{
		if(ParallelThreadDoTask())
		{
			m_TasksMutex.Wait(INFINITE);
		}
		else
		{
			::SetEvent(m_Handles[i]);
			m_TasksCondition.Sleep(m_TasksMutex, INFINITE);
			::ResetEvent(m_Handles[i]);
		}
	}
	return 0;
}

void ParallelTaskManager::StartParallelThread(LONG lMaximumCount)
{
	_ASSERT(lMaximumCount > 0);

	m_bStopped = false;

	for(LONG i = 0; i < lMaximumCount; i++)
	{
		m_Threads.push_back(ThreadPtr(new Thread(boost::bind(&ParallelTaskManager::ParallelThreadFunc, this, i))));
		m_Handles.push_back(::CreateEvent(NULL, TRUE, FALSE, NULL));
	}

	for(size_t i = 0; i < m_Threads.size(); i++)
	{
		m_Threads[i]->CreateThread();
		m_Threads[i]->ResumeThread();
	}
}

void ParallelTaskManager::StopParallelThread(void)
{
	m_TasksMutex.Wait(INFINITE);
	m_bStopped = true;
	m_TasksMutex.Release();
	m_TasksCondition.Wake(m_Threads.size());

	for(size_t i = 0; i < m_Threads.size(); i++)
	{
		m_Threads[i]->WaitForThreadStopped(INFINITE);
		m_Threads[i]->CloseThread();
	}

	m_Threads.clear();
	m_Handles.clear();
}

void ParallelTaskManager::PushTask(ParallelTask * task)
{
	m_TasksMutex.Wait(INFINITE);
	m_Tasks.push_back(task);
	m_TasksMutex.Release();
	m_TasksCondition.Wake(1);
}

void ParallelTaskManager::DoAllParallelTasks(void)
{
	m_TasksMutex.Wait();
	while(ParallelThreadDoTask())
	{
		m_TasksMutex.Wait(INFINITE);
	}
	m_TasksMutex.Release();

	::WaitForMultipleObjects(m_Handles.size(), &m_Handles[0], TRUE, INFINITE);
}

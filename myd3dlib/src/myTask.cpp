#include "StdAfx.h"
#include "myTask.h"
#include <boost/bind.hpp>

using namespace my;

ParallelTaskManager::ParallelTaskManager(LONG lMaximumCount)
	: m_bStopped(false)
{
	_ASSERT(lMaximumCount >= 2);

	for(LONG i = 0; i < (lMaximumCount - 1); i++)
	{
		m_Threads.push_back(ThreadPtr(new Thread(boost::bind(&ParallelTaskManager::ParallelThreadFunc, this))));
	}
}

bool ParallelTaskManager::ParallelThreadDoTask(void)
{
	if(!m_Tasks.empty())
	{
		Task * task = m_Tasks.back();

		m_Tasks.pop_back();

		m_TasksMutex.Release();

		task->DoTask();

		return true;
	}
	return false;
}

DWORD ParallelTaskManager::ParallelThreadFunc(void)
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
			m_TasksCondition.Sleep(m_TasksMutex, INFINITE);
		}
	}
	return 0;
}

void ParallelTaskManager::StartParallelThread(void)
{
	m_bStopped = false;
	for(LONG i = 0; i < m_Threads.size(); i++)
	{
		m_Threads[i]->CreateThread();
		m_Threads[i]->ResumeThread();
	}
}

void ParallelTaskManager::StopParallelThread(void)
{
	m_TasksMutex.Wait(INFINITE);
	m_bStopped = true;
	m_TasksCondition.Wake();
	m_TasksMutex.Release();
}

void ParallelTaskManager::PushTask(Task * task)
{
	m_TasksMutex.Wait(INFINITE);
	m_Tasks.push_back(task);
	m_TasksCondition.Wake();
	m_TasksMutex.Release();
}

void ParallelTaskManager::DoAllTasks(void)
{
	m_TasksMutex.Wait();
	while(ParallelThreadDoTask())
	{
		m_TasksMutex.Wait(INFINITE);
	}
	m_TasksMutex.Release();
}

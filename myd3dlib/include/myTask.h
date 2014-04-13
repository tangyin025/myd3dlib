#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include "myThread.h"

namespace my
{
	class Task
	{
	public:
		virtual void DoTask(void) = 0;
	};

	typedef boost::shared_ptr<Task> TaskPtr;

	class ParallelTaskManager
	{
	protected:
		typedef std::vector<ThreadPtr> ThreadPtrList;

		ThreadPtrList m_Threads;

		typedef std::vector<HANDLE> HANDLEList;

		HANDLEList m_Handles;

		typedef std::vector<Task *> TaskList;

		TaskList m_Tasks;

		Mutex m_TasksMutex;

		ConditionVariable m_TasksCondition;

		bool m_bStopped;

	public:
		ParallelTaskManager(LONG lMaximumCount);

		bool ParallelThreadDoTask(void);

		DWORD ParallelThreadFunc(int i);

		void StartParallelThread(void);

		void StopParallelThread(void);

		void PushTask(Task * task);

		void DoAllTasks(void);
	};
}

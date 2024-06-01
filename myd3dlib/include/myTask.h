// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include "mySingleton.h"
#include "myThread.h"

namespace my
{
	class ParallelTask
	{
	public:
		virtual ~ParallelTask(void);

		virtual void DoTask(void) = 0;
	};

	typedef boost::shared_ptr<ParallelTask> ParallelTaskPtr;

	class ParallelTaskManager : public SingletonInstance<ParallelTaskManager>
	{
	public:
		typedef std::vector<ThreadPtr> ThreadPtrList;

		ThreadPtrList m_Threads;

		typedef std::vector<HANDLE> HANDLEList;

		HANDLEList m_Handles;

		typedef std::vector<ParallelTask *> TaskList;

		TaskList m_Tasks;

		Mutex m_TasksMutex;

		ConditionVariable m_TasksCondition;

		bool m_bStopped;

	public:
		ParallelTaskManager(void);

		bool ParallelThreadDoTask(void);

		DWORD ParallelThreadFunc(int i);

		void StartParallelThread(LONG lMaximumCount);

		void StopParallelThread(void);

		void PushTask(ParallelTask * task);

		bool FindTask(ParallelTask * task);

		void DoAllParallelTasks(void);
	};
}

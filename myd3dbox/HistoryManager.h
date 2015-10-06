#pragma once

#include "HistoryStep.h"

class HistoryManager
{
public:
	typedef std::deque<HistoryStepPtr> HistoryStepList;

	HistoryStepList m_HistoryList;

	HistoryStepList::iterator m_CurrentIter;

public:
	HistoryManager(void)
		: m_CurrentIter(m_HistoryList.end())
	{
	}

	virtual ~HistoryManager(void)
	{
	}

	void PushAndDo(HistoryStepPtr step);

	void ClearHistory(void);

	bool CanDo(void);

	void Do(void);

	bool CanUndo(void);

	void Undo(void);
};

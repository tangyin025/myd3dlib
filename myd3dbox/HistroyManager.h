#pragma once

#include "HistroyStep.h"

class HistroyManager
{
public:
	typedef std::deque<HistoryStepPtr> HistoryStepList;

	HistoryStepList m_HistoryList;

	HistoryStepList::iterator m_CurrentIter;

public:
	HistroyManager(void)
		: m_CurrentIter(m_HistoryList.end())
	{
	}

	virtual ~HistroyManager(void)
	{
	}

	void PushAndDo(HistoryStepPtr step);

	bool CanDo(void);

	void Do(void);

	bool CanUndo(void);

	void Undo(void);
};

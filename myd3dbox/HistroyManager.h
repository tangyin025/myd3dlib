#pragma once

#include "HistroyStep.h"

class CHistroyManager
{
public:
	typedef std::deque<HistoryStepPtr> HistoryStepList;

	HistoryStepList m_HistoryList;

	HistoryStepList::iterator m_CurrentIter;

public:
	CHistroyManager(void)
		: m_CurrentIter(m_HistoryList.end())
	{
	}

	virtual ~CHistroyManager(void)
	{
	}

	void PushAndDo(HistoryStepPtr step);

	bool CanDo(void);

	void Do(void);

	bool CanUndo(void);

	void Undo(void);
};

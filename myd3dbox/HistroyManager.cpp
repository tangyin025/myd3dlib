#include "StdAfx.h"
#include "HistroyManager.h"

void HistroyManager::PushAndDo(HistoryStepPtr step)
{
	m_HistoryList.erase(m_CurrentIter, m_HistoryList.end());
	m_HistoryList.push_back(step);
	m_CurrentIter = m_HistoryList.end();
	step->Do();
}

bool HistroyManager::CanDo(void)
{
	return m_CurrentIter != m_HistoryList.end();
}

void HistroyManager::Do(void)
{
	ASSERT(CanDo());
	(*m_CurrentIter++)->Do();
}

bool HistroyManager::CanUndo(void)
{
	return m_CurrentIter != m_HistoryList.begin();
}

void HistroyManager::Undo(void)
{
	ASSERT(CanUndo());
	(*--m_CurrentIter)->Undo();
}

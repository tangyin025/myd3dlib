#include "StdAfx.h"
#include "HistoryManager.h"

void HistoryManager::PushAndDo(HistoryStepPtr step)
{
	m_HistoryList.erase(m_CurrentIter, m_HistoryList.end());
	m_HistoryList.push_back(step);
	m_CurrentIter = m_HistoryList.end();
	step->Do();
}

void HistoryManager::ClearHistory(void)
{
	m_HistoryList.clear();
	m_CurrentIter = m_HistoryList.end();
}

bool HistoryManager::CanDo(void)
{
	return m_CurrentIter != m_HistoryList.end();
}

void HistoryManager::Do(void)
{
	ASSERT(CanDo());
	(*m_CurrentIter++)->Do();
}

bool HistoryManager::CanUndo(void)
{
	return m_CurrentIter != m_HistoryList.begin();
}

void HistoryManager::Undo(void)
{
	ASSERT(CanUndo());
	(*--m_CurrentIter)->Undo();
}

#include "StdAfx.h"
#include "HistroyManager.h"

void CHistroyManager::PushAndDo(HistoryStepPtr step)
{
	m_HistoryList.erase(m_CurrentIter, m_HistoryList.end());
	m_HistoryList.push_back(step);
	m_CurrentIter = m_HistoryList.end();
	step->Do();
}

bool CHistroyManager::CanDo(void)
{
	return m_CurrentIter != m_HistoryList.end();
}

void CHistroyManager::Do(void)
{
	ASSERT(CanDo());
	(*m_CurrentIter++)->Do();
}

bool CHistroyManager::CanUndo(void)
{
	return m_CurrentIter != m_HistoryList.begin();
}

void CHistroyManager::Undo(void)
{
	ASSERT(CanUndo());
	(*--m_CurrentIter)->Undo();
}

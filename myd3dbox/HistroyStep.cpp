#include "StdAfx.h"
#include "HistroyStep.h"

void HistoryStep::Do(void)
{
	OperatorPairList::iterator pair_iter = m_Ops.begin();
	for (; pair_iter != m_Ops.end(); pair_iter++)
	{
		pair_iter->first->Do();
	}
}

void HistoryStep::Undo(void)
{
	OperatorPairList::reverse_iterator pair_iter = m_Ops.rbegin();
	for (; pair_iter != m_Ops.rend(); pair_iter++)
	{
		pair_iter->second->Do();
	}
}

void HistroyAddComponent::Do(void)
{
	m_level->AddComponent(m_cmp);
}

void HistroyRemoveComponent::Do(void)
{
	m_level->RemoveComponent(m_cmp);
}

#include "StdAfx.h"
#include "HistoryStep.h"

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

HistoryStepPtr HistoryStep::AddComponent(ComponentLevel * level, ComponentPtr cmp)
{
	HistoryStepPtr ret(new HistoryStep);
	ret->m_Ops.push_back(std::make_pair(
		OperatorPtr(new OperatorAddComponent(level, cmp)), OperatorPtr(new OperatorRemoveComponent(level, cmp))));
	return ret;
}

HistoryStepPtr HistoryStep::RemoveComponent(ComponentLevel * level, ComponentPtr cmp)
{
	HistoryStepPtr ret(new HistoryStep);
	ret->m_Ops.push_back(std::make_pair(
		OperatorPtr(new OperatorRemoveComponent(level, cmp)), OperatorPtr(new OperatorAddComponent(level, cmp))));
	return ret;
}

void OperatorAddComponent::Do(void)
{
	m_level->AddComponent(m_cmp);
}

void OperatorRemoveComponent::Do(void)
{
	m_level->RemoveComponent(m_cmp);
}

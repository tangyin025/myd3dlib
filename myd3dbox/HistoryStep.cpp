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

HistoryStepPtr HistoryStep::AddComponentList(ComponentLevel * level, ComponentPtrList cmp_list)
{
	HistoryStepPtr ret(new HistoryStep);
	ret->m_Ops.push_back(std::make_pair(
		OperatorPtr(new OperatorAddComponentList(level, cmp_list)), OperatorPtr(new OperatorRemoveComponentList(level, cmp_list))));
	return ret;
}

HistoryStepPtr HistoryStep::RemoveComponentList(ComponentLevel * level, ComponentPtrList cmp_list)
{
	HistoryStepPtr ret(new HistoryStep);
	ret->m_Ops.push_back(std::make_pair(
		OperatorPtr(new OperatorRemoveComponentList(level, cmp_list)), OperatorPtr(new OperatorAddComponentList(level, cmp_list))));
	return ret;
}

void OperatorAddComponentList::Do(void)
{
	HistoryStep::ComponentPtrList::iterator cmp_iter = m_cmp_list.begin();
	for (; cmp_iter != m_cmp_list.end(); cmp_iter++)
	{
		m_level->AddComponent(*cmp_iter);
	}
}

void OperatorRemoveComponentList::Do(void)
{
	HistoryStep::ComponentPtrList::reverse_iterator cmp_iter = m_cmp_list.rbegin();
	for (; cmp_iter != m_cmp_list.rend(); cmp_iter++)
	{
		bool res = m_level->RemoveComponent(*cmp_iter);
		ASSERT(res);
	}
}

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
//
//void OperatorAddComponent::Do(void)
//{
//	m_level->AddComponent(m_cmp, m_cmp->m_aabb.transform(m_cmp->m_World));
//}
//
//void OperatorRemoveComponent::Do(void)
//{
//	bool res = m_level->RemoveComponent(m_cmp);
//	ASSERT(res);
//}
//
//void OperatorMatrix4Property::Do(void)
//{
//	m_OldValue = m_NewValue;
//}
//
//void OperatorComponentWorld::Do(void)
//{
//	__super::Do();
//	ASSERT(m_cmp->GetOctNode());
//	bool res = m_cmp->GetOctNode()->RemoveComponent(m_cmp);
//	ASSERT(res);
//	m_level->AddComponent(m_cmp, m_cmp->m_aabb.transform(m_cmp->m_World));
//}

#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"
#include "ActorComponent.h"

using namespace my;

void Actor::Attacher::UpdateWorld(void)
{
	_ASSERT(m_Owner);
	if (m_Owner->m_Animator && m_SlotId < m_Owner->m_Animator->m_DualQuats.size())
	{
		my::Matrix4 Slot = BoneList::UDQtoRM(m_Owner->m_Animator->m_DualQuats[m_SlotId]);
		m_World = Slot * m_Owner->m_World;
	}
	else
	{
		m_World = m_Owner->m_World;
	}
}

void Actor::Update(float fElapsedTime)
{
	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}

	RenderComponentPtrList::iterator cmp_iter = m_ComponentList.begin();
	for (; cmp_iter != m_ComponentList.end(); cmp_iter++)
	{
		(*cmp_iter)->Update(fElapsedTime);
	}
}

void Actor::OnPxThreadSubstep(float fElapsedTime)
{
	ClothComponentPtrList::iterator cloth_iter = m_Clothes.begin();
	for (; cloth_iter != m_Clothes.end(); cloth_iter++)
	{
		(*cloth_iter)->OnPxThreadSubstep(fElapsedTime);
	}
}

void Actor::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	RenderComponentPtrList::iterator cmp_iter = m_ComponentList.begin();
	for (; cmp_iter != m_ComponentList.end(); cmp_iter++)
	{
		(*cmp_iter)->QueryMesh(pipeline, stage);
	}
}

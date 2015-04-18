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

void Actor::OnResetDevice(void)
{
}

void Actor::OnLostDevice(void)
{
}

void Actor::OnDestroyDevice(void)
{
	ClothComponentPtrList::iterator cloth_iter = m_Clothes.begin();
	for (; cloth_iter != m_Clothes.end(); cloth_iter++)
	{
		(*cloth_iter)->m_Decl.Release();
	}
}

void Actor::Update(float fElapsedTime)
{
	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}

	struct CallBack : public my::IQueryCallback
	{
		float m_fElapsedTime;

		CallBack(float fElapsedTime)
			: m_fElapsedTime(fElapsedTime)
		{
		}

		void operator() (AABBComponent * comp, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<RenderComponent *>(comp));
			static_cast<RenderComponent *>(comp)->Update(m_fElapsedTime);
		}
	};

	QueryComponentAll(&CallBack(fElapsedTime));
}

void Actor::OnPxThreadSubstep(float dtime)
{
	ClothComponentPtrList::iterator cloth_iter = m_Clothes.begin();
	for (; cloth_iter != m_Clothes.end(); cloth_iter++)
	{
		(*cloth_iter)->OnPxThreadSubstep(dtime);
	}
}

void Actor::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	struct CallBack : public my::IQueryCallback
	{
		RenderPipeline * m_pipeline;

		RenderPipeline::DrawStage m_stage;

		CallBack(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
			: m_pipeline(pipeline)
			, m_stage(stage)
		{
		}

		void operator() (AABBComponent * comp, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<RenderComponent *>(comp));
			static_cast<RenderComponent *>(comp)->QueryMesh(m_pipeline, m_stage);
		}
	};

	QueryComponentAll(&CallBack(pipeline, stage));
}

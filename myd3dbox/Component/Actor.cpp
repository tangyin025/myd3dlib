#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"
#include "ActorComponent.h"

using namespace my;

void Actor::Attacher::UpdateWorld(void)
{
	_ASSERT(m_Owner);
	if (m_AnimId < m_Owner->m_AnimatorList.size() && m_SlotId < m_Owner->m_AnimatorList[m_AnimId]->m_DualQuats.size())
	{
		my::Matrix4 Slot = TransformList::UDQtoRM(m_Owner->m_AnimatorList[m_AnimId]->m_DualQuats[m_SlotId]);
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
		(*cloth_iter)->m_Cloth.reset();
	}
}

void Actor::Update(float fElapsedTime)
{
	AnimatorPtrList::iterator anim_iter = m_AnimatorList.begin();
	for (; anim_iter != m_AnimatorList.end(); anim_iter++)
	{
		(*anim_iter)->Update(fElapsedTime);
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

void Actor::QueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public my::IQueryCallback
	{
		RenderPipeline * m_pipeline;

		unsigned int m_PassMask;

		CallBack(RenderPipeline * pipeline, unsigned int PassMask)
			: m_pipeline(pipeline)
			, m_PassMask(PassMask)
		{
		}

		void operator() (AABBComponent * comp, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<RenderComponent *>(comp));

			static_cast<RenderComponent *>(comp)->QueryMesh(m_pipeline, m_PassMask);
		}
	};

	OctRoot::QueryComponent(frustum, &CallBack(pipeline, PassMask));
}

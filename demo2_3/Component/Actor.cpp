#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"

using namespace my;

ComponentLevel * LODComponent::GetComponentLevel(unsigned int level)
{
	if (m_lvls.size() < level + 1)
	{
		m_lvls.resize(level + 1, ComponentLevelPtr(new ComponentLevel(m_aabb, 1.0f)));
	}
	return m_lvls[level].get();
}

void LODComponent::OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	GetComponentLevel(m_level)->QueryComponent(frustum, pipeline, PassMask);
}

void ComponentLevel::QueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public my::IQueryCallback
	{
		const my::Frustum & m_frustum;

		RenderPipeline * m_pipeline;

		unsigned int m_PassMask;

		CallBack(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
			: m_frustum(frustum)
			, m_pipeline(pipeline)
			, m_PassMask(PassMask)
		{
		}

		void operator() (AABBComponent * comp, IntersectionTests::IntersectionType)
		{
			RenderComponent * render_cmp = dynamic_cast<RenderComponent *>(comp);
			if (render_cmp)
			{
				render_cmp->OnQueryComponent(m_frustum, m_pipeline, m_PassMask);
			}
		}
	};

	OctRoot::QueryComponent(frustum, &CallBack(frustum, pipeline, PassMask));
}

void LODComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(false);
}

void Actor::Update(float fElapsedTime)
{
	struct CallBack : public my::IQueryCallback
	{
		float m_fElapsedTime;

		CallBack(float fElapsedTime)
			: m_fElapsedTime(fElapsedTime)
		{
		}

		void operator() (AABBComponent * comp, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Component *>(comp));
			static_cast<Component *>(comp)->Update(m_fElapsedTime);
		}
	};

	OctRoot::QueryComponentAll(&CallBack(fElapsedTime));
}

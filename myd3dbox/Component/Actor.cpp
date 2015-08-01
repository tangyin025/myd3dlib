#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"

using namespace my;

ComponentLevel * RenderComponentLod::GetComponentLevel(unsigned int level)
{
	if (m_lvls.size() < level + 1)
	{
		m_lvls.resize(level + 1, ComponentLevelPtr(new ComponentLevel(m_aabb, 1.0f)));
	}
	return m_lvls[level].get();
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
			RenderComponent * render_cmp = dynamic_cast<RenderComponent *>(comp);
			if (render_cmp)
			{
				render_cmp->QueryMesh(m_pipeline, m_PassMask);
			}
		}
	};

	OctRoot::QueryComponent(frustum, &CallBack(pipeline, PassMask));
}

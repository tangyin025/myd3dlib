#include "StdAfx.h"
#include "World.h"
#include "Actor.h"

using namespace my;

void WorldL::_QueryRenderComponent(const CPoint & level_id, const CPoint & offset, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public my::IQueryCallback
	{
		const my::Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;

		CallBack(const my::Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask)
			: frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
		{
		}

		void operator() (OctActor * oct_actor, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			actor->AddToPipeline(frustum, pipeline, PassMask);
		}
	};

	CPoint new_level_id = level_id + offset;
	if (new_level_id.x >= 0 && new_level_id.x < m_Dim && new_level_id.y >= 0 && new_level_id.y < m_Dim)
	{
		Frustum loc_frustum = frustum.transform(Matrix4::Translation(offset.x * 512.0f, 0, offset.y * 512.0f).transpose());
		m_levels[new_level_id.y * m_Dim + new_level_id.x].QueryActor(frustum, &CallBack(frustum, pipeline, PassMask));
	}
}

void WorldL::QueryRenderComponent(const CPoint & level_id, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	_QueryRenderComponent(level_id, CPoint(-1, -1), frustum, pipeline, PassMask);
	_QueryRenderComponent(level_id, CPoint( 0, -1), frustum, pipeline, PassMask);
	_QueryRenderComponent(level_id, CPoint( 1, -1), frustum, pipeline, PassMask);

	_QueryRenderComponent(level_id, CPoint(-1,  0), frustum, pipeline, PassMask);
	_QueryRenderComponent(level_id, CPoint( 0,  0), frustum, pipeline, PassMask);
	_QueryRenderComponent(level_id, CPoint( 1,  0), frustum, pipeline, PassMask);

	_QueryRenderComponent(level_id, CPoint(-1,  1), frustum, pipeline, PassMask);
	_QueryRenderComponent(level_id, CPoint( 0,  1), frustum, pipeline, PassMask);
	_QueryRenderComponent(level_id, CPoint( 1,  1), frustum, pipeline, PassMask);
}

void WorldL::_ResetViewedActors(const CPoint & level_id, const CPoint & offset, const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
}

void WorldL::ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
}

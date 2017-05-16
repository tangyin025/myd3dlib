#include "StdAfx.h"
#include "World.h"
#include "Actor.h"
#include "PhysXContext.h"

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
		m_levels[new_level_id.y * m_Dim + new_level_id.x].QueryActor(loc_frustum, &CallBack(loc_frustum, pipeline, PassMask));
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
	struct CallBack : public my::IQueryCallback
	{
		WorldL * world;
		const my::Vector3 & ViewedPos;
		const my::Vector3 & TargetPos;
		CallBack(WorldL * _world, const my::Vector3 & _ViewedPos, const my::Vector3 & _TargetPos)
			: world(_world)
			, ViewedPos(_ViewedPos)
			, TargetPos(_TargetPos)
		{
		}
		void operator() (OctActor * oct_actor, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			OctActorSet::iterator actor_iter = world->m_ViewedActors.find(actor);
			if (actor_iter == world->m_ViewedActors.end())
			{
				if (!actor->IsRequested())
				{
					actor->RequestResource();
				}
				world->m_ViewedActors.insert(actor);
				actor->OnEnterPxScene(PhysXSceneContext::getSingleton().m_PxScene.get());
			}
			actor->UpdateLod(ViewedPos, TargetPos);
		}
	};

	CPoint new_level_id = level_id + offset;
	if (new_level_id.x >= 0 && new_level_id.x < m_Dim && new_level_id.y >= 0 && new_level_id.y < m_Dim)
	{
		const Vector3 InExtent(1000, 1000, 1000);
		Vector3 LocalPos = TargetPos + Vector3(offset.x * 512.0f, 0, offset.y * 512.0f);
		AABB InBox(LocalPos - InExtent, LocalPos + InExtent);
		m_levels[new_level_id.y * m_Dim + new_level_id.x].QueryActor(InBox, &CallBack(this, ViewedPos, TargetPos));
	}
}

void WorldL::ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
	const Vector3 OutExtent(1050, 1050, 1050);
	AABB OutBox(TargetPos - OutExtent, TargetPos + OutExtent);
	OctActorSet::iterator cmp_iter = m_ViewedActors.begin();
	for (; cmp_iter != m_ViewedActors.end(); )
	{
		// ! all components world should be updated according to level_id
		if (IntersectionTests::IntersectionTypeOutside
			== IntersectionTests::IntersectAABBAndAABB(OutBox, (*cmp_iter)->m_aabb.transform((*cmp_iter)->m_World)))
		{
			if ((*cmp_iter)->IsRequested())
			{
				(*cmp_iter)->ReleaseResource();
			}
			(*cmp_iter)->OnLeavePxScene(PhysXSceneContext::getSingleton().m_PxScene.get());
			cmp_iter = m_ViewedActors.erase(cmp_iter);
		}
		else
			cmp_iter++;
	}

	_ResetViewedActors(level_id, CPoint(-1, -1), ViewedPos, TargetPos);
	_ResetViewedActors(level_id, CPoint( 0, -1), ViewedPos, TargetPos);
	_ResetViewedActors(level_id, CPoint( 1, -1), ViewedPos, TargetPos);

	_ResetViewedActors(level_id, CPoint(-1,  0), ViewedPos, TargetPos);
	_ResetViewedActors(level_id, CPoint( 0,  0), ViewedPos, TargetPos);
	_ResetViewedActors(level_id, CPoint( 1,  0), ViewedPos, TargetPos);

	_ResetViewedActors(level_id, CPoint(-1,  1), ViewedPos, TargetPos);
	_ResetViewedActors(level_id, CPoint( 0,  1), ViewedPos, TargetPos);
	_ResetViewedActors(level_id, CPoint( 1,  1), ViewedPos, TargetPos);
}

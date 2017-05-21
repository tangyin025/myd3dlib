#include "StdAfx.h"
#include "World.h"
#include "Actor.h"
#include "Terrain.h"
#include "PhysXContext.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Octree)

void WorldL::CreateLevels(int x, int y)
{
	m_Dim = x;
	m_levels.resize(x * y, Octree(AABB(
		Vector3(-Terrain::m_RowChunks * Terrain::m_ChunkRows, -3000, Terrain::m_ColChunks * Terrain::m_ChunkRows),
		Vector3(Terrain::m_RowChunks * Terrain::m_ChunkRows * 2, 3000, Terrain::m_ColChunks * Terrain::m_ChunkRows * 2)), 1.0f));
}

void WorldL::ClearAllLevels(void)
{
	m_ViewedActors.clear();
	m_levels.clear();
	m_Dim = 0;
	m_LevelId.SetPoint(0,0);
}

void WorldL::_QueryRenderComponent(const CPoint & level_id, const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public IQueryCallback
	{
		const Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;

		CallBack(const Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask)
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

	if (level_id.x >= 0 && level_id.x < m_Dim && level_id.y >= 0 && level_id.y < m_Dim)
	{
		Vector3 Offset((level_id.x - m_LevelId.x) * 512.0f, 0, (level_id.y - m_LevelId.y) * 512.0f);
		Frustum loc_frustum = frustum.transform(Matrix4::Translation(Offset).transpose());
		GetLevel(level_id).QueryActor(loc_frustum, &CallBack(loc_frustum, pipeline, PassMask));
	}
}

void WorldL::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	_QueryRenderComponent(m_LevelId + CPoint(-1, -1), frustum, pipeline, PassMask);
	_QueryRenderComponent(m_LevelId + CPoint( 0, -1), frustum, pipeline, PassMask);
	_QueryRenderComponent(m_LevelId + CPoint( 1, -1), frustum, pipeline, PassMask);

	_QueryRenderComponent(m_LevelId + CPoint(-1,  0), frustum, pipeline, PassMask);
	_QueryRenderComponent(m_LevelId + CPoint( 0,  0), frustum, pipeline, PassMask);
	_QueryRenderComponent(m_LevelId + CPoint( 1,  0), frustum, pipeline, PassMask);

	_QueryRenderComponent(m_LevelId + CPoint(-1,  1), frustum, pipeline, PassMask);
	_QueryRenderComponent(m_LevelId + CPoint( 0,  1), frustum, pipeline, PassMask);
	_QueryRenderComponent(m_LevelId + CPoint( 1,  1), frustum, pipeline, PassMask);
}

void WorldL::_ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewPos)
{
	struct CallBack : public IQueryCallback
	{
		WorldL * world;
		const Vector3 & Offset;
		const Vector3 & ViewPos;
		CallBack(WorldL * _world, const Vector3 & _Offset, const Vector3 & _ViewPos)
			: world(_world)
			, Offset(_Offset)
			, ViewPos(_ViewPos)
		{
		}
		void operator() (OctActor * oct_actor, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			OctActorSet::iterator actor_iter = world->m_ViewedActors.find(actor);
			if (actor_iter == world->m_ViewedActors.end())
			{
				actor->UpdateWorld(Matrix4::Translation(Offset));
				if (!actor->IsRequested())
				{
					actor->RequestResource();
				}
				world->m_ViewedActors.insert(actor);
				actor->OnEnterPxScene(PhysXSceneContext::getSingleton().m_PxScene.get());
			}
			actor->UpdateLod(ViewPos);
		}
	};

	if (level_id.x >= 0 && level_id.x < m_Dim && level_id.y >= 0 && level_id.y < m_Dim)
	{
		const Vector3 InExtent(1000, 1000, 1000);
		Vector3 Offset((level_id.x - m_LevelId.x) * 512.0f, 0, (level_id.y - m_LevelId.y) * 512.0f);
		AABB InBox(ViewPos + Offset - InExtent, ViewPos + Offset + InExtent);
		GetLevel(level_id).QueryActor(InBox, &CallBack(this, Offset, ViewPos));
	}
}

void WorldL::ResetViewedActors(const my::Vector3 & ViewPos)
{
	const Vector3 OutExtent(1050, 1050, 1050);
	AABB OutBox(ViewPos - OutExtent, ViewPos + OutExtent);
	OctActorSet::iterator cmp_iter = m_ViewedActors.begin();
	for (; cmp_iter != m_ViewedActors.end(); )
	{
		// ! all components world should be updated according to m_LevelId
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

	_ResetViewedActors(m_LevelId + CPoint(-1, -1), ViewPos);
	_ResetViewedActors(m_LevelId + CPoint( 0, -1), ViewPos);
	_ResetViewedActors(m_LevelId + CPoint( 1, -1), ViewPos);

	_ResetViewedActors(m_LevelId + CPoint(-1,  0), ViewPos);
	_ResetViewedActors(m_LevelId + CPoint( 0,  0), ViewPos);
	_ResetViewedActors(m_LevelId + CPoint( 1,  0), ViewPos);

	_ResetViewedActors(m_LevelId + CPoint(-1,  1), ViewPos);
	_ResetViewedActors(m_LevelId + CPoint( 0,  1), ViewPos);
	_ResetViewedActors(m_LevelId + CPoint( 1,  1), ViewPos);
}

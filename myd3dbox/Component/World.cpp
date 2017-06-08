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

template<>
void WorldL::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Dimension);
	ar << BOOST_SERIALIZATION_NVP(m_levels);
}

template<>
void WorldL::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Dimension);
	ar >> BOOST_SERIALIZATION_NVP(m_levels);
	for (unsigned int i = 0; i < m_levels.size(); i++)
	{
		m_levels[i].m_World = this;
	}
}

void WorldL::ChangeLevelId(const CPoint & new_id)
{
	if (m_LevelId != new_id)
	{
		OctActorSet::iterator cmp_iter = m_ViewedActors.begin();
		for (; cmp_iter != m_ViewedActors.end(); )
		{
			CPoint level_id = GetLevelId(dynamic_cast<Octree *>((*cmp_iter)->m_Node->GetTopNode()));
			Vector3 Offset((level_id.x - m_LevelId.x) * LEVEL_SIZE, 0, (level_id.y - m_LevelId.y) * LEVEL_SIZE);
			(*cmp_iter)->UpdateWorld(Matrix4::Translation(Offset));
		}
	}
}

void WorldL::CreateLevels(int dimension)
{
	m_Dimension = dimension;
	AABB level_bound(Vector3(-LEVEL_SIZE, -LEVEL_SIZE, -LEVEL_SIZE), Vector3(LEVEL_SIZE * 2, LEVEL_SIZE * 2, LEVEL_SIZE * 2));
	m_levels.resize(m_Dimension * m_Dimension, Octree(this, level_bound, 1.0f));
}

void WorldL::ClearAllLevels(void)
{
	m_ViewedActors.clear();
	m_levels.clear();
	m_Dimension = 0;
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

	if (level_id.x >= 0 && level_id.x < m_Dimension && level_id.y >= 0 && level_id.y < m_Dimension)
	{
		Vector3 Offset((level_id.x - m_LevelId.x) * LEVEL_SIZE, 0, (level_id.y - m_LevelId.y) * LEVEL_SIZE);
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

void WorldL::_ResetViewedActors(const CPoint & level_id, const my::Vector3 & ViewPos, PhysXSceneContext * scene)
{
	struct CallBack : public IQueryCallback
	{
		WorldL * world;
		const Vector3 & Offset;
		const Vector3 & ViewPos;
		PhysXSceneContext * scene;
		CallBack(WorldL * _world, const Vector3 & _Offset, const Vector3 & _ViewPos, PhysXSceneContext * _scene)
			: world(_world)
			, Offset(_Offset)
			, ViewPos(_ViewPos)
			, scene(_scene)
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
					actor->OnEnterPxScene(scene);
				}
				world->m_ViewedActors.insert(actor);
			}
			actor->UpdateLod(ViewPos);
		}
	};

	if (level_id.x >= 0 && level_id.x < m_Dimension && level_id.y >= 0 && level_id.y < m_Dimension)
	{
		const Vector3 InExtent(VIEWED_DIST);
		Vector3 Offset((level_id.x - m_LevelId.x) * LEVEL_SIZE, 0, (level_id.y - m_LevelId.y) * LEVEL_SIZE);
		AABB InBox(ViewPos + Offset - InExtent, ViewPos + Offset + InExtent);
		GetLevel(level_id).QueryActor(InBox, &CallBack(this, Offset, ViewPos, scene));
	}
}

void WorldL::ResetViewedActors(const my::Vector3 & ViewPos, PhysXSceneContext * scene)
{
	const Vector3 OutExtent(VIEWED_DIST + VIEWED_THRESHOLD);
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
				(*cmp_iter)->OnLeavePxScene(scene);
			}
			cmp_iter = m_ViewedActors.erase(cmp_iter);
		}
		else
			cmp_iter++;
	}

	_ResetViewedActors(m_LevelId + CPoint(-1, -1), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint( 0, -1), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint( 1, -1), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint(-1,  0), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint( 0,  0), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint( 1,  0), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint(-1,  1), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint( 0,  1), ViewPos, scene);
	_ResetViewedActors(m_LevelId + CPoint( 1,  1), ViewPos, scene);
}

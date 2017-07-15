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

void WorldL::CreateLevels(long dimension)
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

void WorldL::QueryLevel(const CPoint & level_id, QueryCallback * callback)
{
	for (long i = Max(0L, level_id.x - 1); i <= Min(m_Dimension - 1, level_id.x + 1); i++)
	{
		for (long j = Max(0L, level_id.y - 1); j <= Min(m_Dimension - 1, level_id.y + 1); j++)
		{
			CPoint id(i, j);
			(*callback)(GetLevel(id), id);
		}
	}
}

void WorldL::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct Callback : public QueryCallback
	{
		WorldL * world;
		const Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		Callback(WorldL * _world, const Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask)
			: world(_world)
			, frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
		{
		}
		void operator () (Octree * level, const CPoint & level_id)
		{
			struct Callback : public my::OctNodeBase::QueryCallback
			{
				const Frustum & frustum;
				RenderPipeline * pipeline;
				unsigned int PassMask;
				Callback(const Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask)
					: frustum(_frustum)
					, pipeline(_pipeline)
					, PassMask(_PassMask)
				{
				}
				void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
				{
					_ASSERT(dynamic_cast<Actor *>(oct_actor));
					Actor * actor = static_cast<Actor *>(oct_actor);
					actor->AddToPipeline(frustum, pipeline, PassMask);
				}
			};

			Vector3 Offset((level_id.x - world->m_LevelId.x) * LEVEL_SIZE, 0, (level_id.y - world->m_LevelId.y) * LEVEL_SIZE);
			Frustum loc_frustum = frustum.transform(Matrix4::Translation(Offset).transpose());
			level->QueryActor(loc_frustum, &Callback(loc_frustum, pipeline, PassMask));
		}
	};

	QueryLevel(m_LevelId, &Callback(this, frustum, pipeline, PassMask));
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

	struct Callback : public QueryCallback
	{
		WorldL * world;
		const Vector3 & ViewPos;
		PhysXSceneContext * scene;
		Callback(WorldL * _world, const Vector3 & _ViewPos, PhysXSceneContext * _scene)
			: world(_world)
			, ViewPos(_ViewPos)
			, scene(_scene)
		{
		}
		void operator () (Octree * level, const CPoint & level_id)
		{
			struct Callback : public my::OctNodeBase::QueryCallback
			{
				WorldL * world;
				const Vector3 & Offset;
				const Vector3 & ViewPos;
				PhysXSceneContext * scene;
				Callback(WorldL * _world, const Vector3 & _Offset, const Vector3 & _ViewPos, PhysXSceneContext * _scene)
					: world(_world)
					, Offset(_Offset)
					, ViewPos(_ViewPos)
					, scene(_scene)
				{
				}
				void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
				{
					_ASSERT(dynamic_cast<Actor *>(oct_actor));
					Actor * actor = static_cast<Actor *>(oct_actor);
					OctActorSet::iterator actor_iter = world->m_ViewedActors.find(actor);
					if (actor_iter == world->m_ViewedActors.end())
					{
						actor->UpdateWorld(Matrix4::Translation(Offset));
						actor->UpdateRigidActorPose();
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

			const Vector3 InExtent(WorldL::VIEWED_DIST);
			Vector3 Offset((level_id.x - world->m_LevelId.x) * LEVEL_SIZE, 0, (level_id.y - world->m_LevelId.y) * LEVEL_SIZE);
			AABB InBox(ViewPos - Offset - InExtent, ViewPos - Offset + InExtent);
			level->QueryActor(InBox, &Callback(world, Offset, ViewPos, scene));
		}
	};

	QueryLevel(m_LevelId, &Callback(this, ViewPos, scene));
}

void WorldL::UpdateViewedActorsWorld(void)
{
	OctActorSet::iterator cmp_iter = m_ViewedActors.begin();
	for (; cmp_iter != m_ViewedActors.end(); cmp_iter++)
	{
		CPoint level_id = GetLevelId(dynamic_cast<Octree *>((*cmp_iter)->m_Node->GetTopNode()));
		Vector3 Offset((level_id.x - m_LevelId.x) * LEVEL_SIZE, 0, (level_id.y - m_LevelId.y) * LEVEL_SIZE);
		(*cmp_iter)->UpdateWorld(Matrix4::Translation(Offset));
	}
}

void WorldL::ResetLevelId(const CPoint & level_id, PhysXSceneContext * scene)
{
	_ASSERT(level_id.x >= 0 && level_id.x < m_Dimension);
	_ASSERT(level_id.y >= 0 && level_id.y < m_Dimension);
	CPoint level_off = level_id - m_LevelId;
	m_LevelId = level_id;
	UpdateViewedActorsWorld();
	scene->m_PxScene->shiftOrigin(physx::PxVec3(level_off.x * LEVEL_SIZE, 0, level_off.y * LEVEL_SIZE));
}

bool WorldL::ResetLevelId(my::Vector3 & ViewPos, PhysXSceneContext * scene)
{
	CPoint level_off((long)floor(ViewPos.x / LEVEL_SIZE), (long)floor(ViewPos.z / LEVEL_SIZE));
	if (level_off.x < 0)
	{
		level_off.x = Max(level_off.x, 0 - m_LevelId.x);
	}
	else if (level_off.x > 0)
	{
		level_off.x = Min(level_off.x, m_Dimension - m_LevelId.x - 1);
	}
	if (level_off.y < 0)
	{
		level_off.y = Max(level_off.y, 0 - m_LevelId.y);
	}
	else if (level_off.y > 0)
	{
		level_off.y = Min(level_off.y, m_Dimension - m_LevelId.y - 1);
	}
	if (level_off.x != 0 || level_off.y != 0)
	{
		ResetLevelId(m_LevelId + level_off, scene);
		ViewPos.x -= level_off.x * LEVEL_SIZE;
		ViewPos.z -= level_off.y * LEVEL_SIZE;
		return true;
	}
	return false;
}

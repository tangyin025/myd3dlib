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
	AABB level_bound(-LEVEL_EDGE, -LEVEL_SIZE - LEVEL_EDGE, -LEVEL_EDGE, LEVEL_SIZE + LEVEL_EDGE, LEVEL_SIZE + LEVEL_EDGE, LEVEL_SIZE + LEVEL_EDGE);
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

			Vector3 Offset((float)(level_id.x - world->m_LevelId.x) * LEVEL_SIZE, 0, (float)(level_id.y - world->m_LevelId.y) * LEVEL_SIZE);
			Frustum loc_frustum = frustum.transform(Matrix4::Translation(Offset).transpose());
			level->QueryActor(loc_frustum, &Callback(frustum, pipeline, PassMask));
		}
	};

	QueryLevel(m_LevelId, &Callback(this, frustum, pipeline, PassMask));
}

void WorldL::ResetViewedActors(const my::Vector3 & ViewPos, PhysXSceneContext * scene, float ViewDist, float ViewThreshold)
{
	const Vector3 OutExtent(ViewDist + ViewThreshold);
	AABB OutBox(ViewPos - OutExtent, ViewPos + OutExtent);
	OctActorSet::iterator actor_iter = m_ViewedActors.begin();
	for (; actor_iter != m_ViewedActors.end(); )
	{
		// ! all components world should be updated according to m_LevelId
		if (IntersectionTests::IntersectionTypeOutside
			== IntersectionTests::IntersectAABBAndAABB(OutBox, (*actor_iter)->m_aabb.transform((*actor_iter)->m_World)))
		{
			if ((*actor_iter)->IsRequested())
			{
				(*actor_iter)->ReleaseResource();
				(*actor_iter)->OnLeavePxScene(scene);
			}
			actor_iter = m_ViewedActors.erase(actor_iter);
		}
		else
			actor_iter++;
	}

	struct Callback : public QueryCallback
	{
		WorldL * world;
		const Vector3 & ViewPos;
		PhysXSceneContext * scene;
		float ViewDist;
		Callback(WorldL * _world, const Vector3 & _ViewPos, PhysXSceneContext * _scene, float _ViewDist)
			: world(_world)
			, ViewPos(_ViewPos)
			, scene(_scene)
			, ViewDist(_ViewDist)
		{
		}
		void operator () (Octree * level, const CPoint & level_id)
		{
			struct Callback : public my::OctNodeBase::QueryCallback
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
				void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
				{
					_ASSERT(dynamic_cast<Actor *>(oct_actor));
					Actor * actor = static_cast<Actor *>(oct_actor);
					OctActorSet::iterator actor_iter = world->m_ViewedActors.find(actor);
					if (actor_iter == world->m_ViewedActors.end())
					{
						actor->UpdateWorld();
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

			const Vector3 InExtent(ViewDist);
			Vector3 Offset((float)(level_id.x - world->m_LevelId.x) * LEVEL_SIZE, 0, (float)(level_id.y - world->m_LevelId.y) * LEVEL_SIZE);
			AABB InBox(ViewPos - Offset - InExtent, ViewPos - Offset + InExtent);
			level->QueryActor(InBox, &Callback(world, ViewPos, scene));
		}
	};

	QueryLevel(m_LevelId, &Callback(this, ViewPos, scene, ViewDist));
}

void WorldL::AdjustLevelIdAndPosition(CPoint & level_id, my::Vector3 & pos)
{
	CPoint level_off((long)floor(pos.x / LEVEL_SIZE), (long)floor(pos.z / LEVEL_SIZE));
	pos.x -= level_off.x * LEVEL_SIZE;
	pos.z -= level_off.y * LEVEL_SIZE;
	level_id = level_id + level_off;
	if (level_id.x < 0)
	{
		pos.x -= -level_id.x * LEVEL_SIZE;
		level_id.x = 0;
	}
	else if (level_id.x >= m_Dimension)
	{
		pos.x += (level_id.x - m_Dimension + 1) * LEVEL_SIZE;
		level_id.x = m_Dimension - 1;
	}
	if (level_id.y < 0)
	{
		pos.z -= -level_id.y * LEVEL_SIZE;
		level_id.y = 0;
	}
	else if (level_id.y >= m_Dimension)
	{
		pos.z += (level_id.y - m_Dimension + 1) * LEVEL_SIZE;
		level_id.y = m_Dimension - 1;
	}
	pos.y = pos.y;
}

void WorldL::OnActorPoseChanged(Actor * actor, const CPoint & level_id)
{
	ActorPtr actor_ptr = actor->shared_from_this();
	my::OctNodeBase * Root = actor_ptr->m_Node->GetTopNode();
	Root->RemoveActor(actor_ptr);
	GetLevel(level_id)->AddActor(actor_ptr, actor->m_aabb.transform(actor->CalculateLocal()));
	actor->UpdateWorld();
}

void WorldL::ResetLevelId(const CPoint & level_id, PhysXSceneContext * scene)
{
	CPoint level_off = level_id - m_LevelId;
	m_LevelId = level_id;
	OctActorSet::iterator actor_iter = m_ViewedActors.begin();
	for (; actor_iter != m_ViewedActors.end(); actor_iter++)
	{
		(*actor_iter)->UpdateWorld();
	}
	scene->m_PxScene->shiftOrigin(physx::PxVec3((float)level_off.x * LEVEL_SIZE, 0, (float)level_off.y * LEVEL_SIZE));
}

bool WorldL::ResetLevelId(my::Vector3 & ViewPos, PhysXSceneContext * scene)
{
	CPoint level_off((long)floor(ViewPos.x / LEVEL_SIZE), (long)floor(ViewPos.z / LEVEL_SIZE));
	if (level_off.x != 0 || level_off.y != 0)
	{
		ResetLevelId(m_LevelId + level_off, scene);
		ViewPos.x -= level_off.x * LEVEL_SIZE;
		ViewPos.z -= level_off.y * LEVEL_SIZE;
		return true;
	}
	return false;
}

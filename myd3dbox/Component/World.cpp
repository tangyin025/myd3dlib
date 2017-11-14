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

BOOST_CLASS_EXPORT(OctLevel)

template<>
void WorldL::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Dimension);
	unsigned int level_count = m_levels.size();
	ar << BOOST_SERIALIZATION_NVP(level_count);
	for (unsigned int i = 0; i < m_levels.size(); i++)
	{
		ar << boost::serialization::make_nvp(str_printf("level%d", i).c_str(), m_levels[i]);
	}
}

template<>
void WorldL::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Dimension);
	unsigned int level_count;
	ar >> BOOST_SERIALIZATION_NVP(level_count);
	m_levels.resize(level_count);
	for (unsigned int i = 0; i < m_levels.size(); i++)
	{
		ar >> boost::serialization::make_nvp(str_printf("level%d", i).c_str(), m_levels[i]);
		m_levels[i].m_World = this;
	}
}

void WorldL::CreateLevels(long dimension)
{
	m_Dimension = dimension;
	AABB level_bound(-LEVEL_EDGE, -LEVEL_SIZE - LEVEL_EDGE, -LEVEL_EDGE, LEVEL_SIZE + LEVEL_EDGE, LEVEL_SIZE + LEVEL_EDGE, LEVEL_SIZE + LEVEL_EDGE);
	m_levels.resize(m_Dimension * m_Dimension, OctLevel(this, level_bound, 1.0f));
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

void WorldL::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos)
{
	struct Callback : public QueryCallback
	{
		WorldL * world;
		const Frustum & frustum;
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const Vector3 & ViewPos;
		Callback(WorldL * _world, const Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask, const Vector3 & _ViewPos)
			: world(_world)
			, frustum(_frustum)
			, pipeline(_pipeline)
			, PassMask(_PassMask)
			, ViewPos(_ViewPos)
		{
		}
		void operator () (OctLevel * level, const CPoint & level_id)
		{
			struct Callback : public my::OctNodeBase::QueryCallback
			{
				const Frustum & frustum;
				RenderPipeline * pipeline;
				unsigned int PassMask;
				const Vector3 & ViewPos;
				Callback(const Frustum & _frustum, RenderPipeline * _pipeline, unsigned int _PassMask, const Vector3 & _ViewPos)
					: frustum(_frustum)
					, pipeline(_pipeline)
					, PassMask(_PassMask)
					, ViewPos(_ViewPos)
				{
				}
				void operator() (my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					_ASSERT(dynamic_cast<Actor *>(oct_actor));
					Actor * actor = static_cast<Actor *>(oct_actor);
					actor->AddToPipeline(frustum, pipeline, PassMask, ViewPos);
				}
			};

			Vector3 Offset((float)(level_id.x - world->m_LevelId.x) * LEVEL_SIZE, 0, (float)(level_id.y - world->m_LevelId.y) * LEVEL_SIZE);
			Frustum loc_frustum = frustum.transform(Matrix4::Translation(Offset).transpose());
			level->QueryActor(loc_frustum, &Callback(frustum, pipeline, PassMask, ViewPos));
		}
	};

	QueryLevel(m_LevelId, &Callback(this, frustum, pipeline, PassMask, ViewPos));
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
			(*actor_iter)->ReleaseResource();
			(*actor_iter)->OnLeavePxScene(scene);
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
		void operator () (OctLevel * level, const CPoint & level_id)
		{
			struct Callback : public my::OctNodeBase::QueryCallback
			{
				WorldL * world;
				PhysXSceneContext * scene;
				Callback(WorldL * _world, PhysXSceneContext * _scene)
					: world(_world)
					, scene(_scene)
				{
				}
				void operator() (my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
				{
					_ASSERT(dynamic_cast<Actor *>(oct_actor));
					Actor * actor = static_cast<Actor *>(oct_actor);
					OctActorSet::iterator actor_iter = world->m_ViewedActors.find(actor);
					if (actor_iter == world->m_ViewedActors.end())
					{
						actor->UpdateWorld();
						actor->UpdateRigidActorPose();
						actor->RequestResource();
						actor->OnEnterPxScene(scene);
						world->m_ViewedActors.insert(actor);
					}
				}
			};

			const Vector3 InExtent(ViewDist);
			Vector3 Offset((float)(level_id.x - world->m_LevelId.x) * LEVEL_SIZE, 0, (float)(level_id.y - world->m_LevelId.y) * LEVEL_SIZE);
			AABB InBox(ViewPos - Offset - InExtent, ViewPos - Offset + InExtent);
			level->QueryActor(InBox, &Callback(world, scene));
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

void WorldL::OnActorPoseChanged(Actor * actor, CPoint level_id)
{
	ActorPtr actor_ptr = boost::dynamic_pointer_cast<Actor>(actor->shared_from_this());
	my::OctNodeBase * Root = actor_ptr->m_Node->GetTopNode();
	Root->RemoveActor(actor_ptr);
	AdjustLevelIdAndPosition(level_id, actor_ptr->m_Position);
	GetLevel(level_id)->AddActor(actor_ptr, actor->m_aabb.transform(actor->CalculateLocal()));
	actor->UpdateWorld();
}

void WorldL::ApplyWorldOffset(const CPoint & level_id, PhysXSceneContext * scene)
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

bool WorldL::ApplyWorldOffset(my::Vector3 & ViewPos, PhysXSceneContext * scene)
{
	CPoint level_off((long)floor(ViewPos.x / LEVEL_SIZE), (long)floor(ViewPos.z / LEVEL_SIZE));
	if (level_off.x != 0 || level_off.y != 0)
	{
		ApplyWorldOffset(m_LevelId + level_off, scene);
		ViewPos.x -= level_off.x * LEVEL_SIZE;
		ViewPos.z -= level_off.y * LEVEL_SIZE;
		return true;
	}
	return false;
}

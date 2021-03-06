#include "StaticEmitterComponent.h"
#include "Actor.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(StaticEmitterChunk)

BOOST_CLASS_EXPORT(StaticEmitterComponent)

using namespace my;

template<class Archive>
void StaticEmitterComponent::save(Archive& ar, const unsigned int version) const
{
	//ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	//ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	//ParticleList::capacity_type buffer_capacity = m_ParticleList.capacity();
	//ar << BOOST_SERIALIZATION_NVP(buffer_capacity);
	//boost::serialization::stl::save_collection<Archive, ParticleList>(ar, m_ParticleList);
	//ar << BOOST_SERIALIZATION_NVP(m_ChunkStep);
	//DWORD ChunkSize = m_Chunks.size();
	//ar << BOOST_SERIALIZATION_NVP(ChunkSize);
	//for (int i = 0; i < (int)m_Chunks.size(); i++)
	//{
	//	ar << boost::serialization::make_nvp(str_printf("m_chunk_%d", i).c_str(), m_Chunks[i]);
	//	ar << boost::serialization::make_nvp(str_printf("m_chunk_%d_aabb", i).c_str(), m_Chunks[i]->m_OctAabb);
	//}
}

template<class Archive>
void StaticEmitterComponent::load(Archive& ar, const unsigned int version)
{
	//ClearAllEntity();
	//ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	//ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	//ParticleList::capacity_type buffer_capacity;
	//ar >> BOOST_SERIALIZATION_NVP(buffer_capacity);
	//m_ParticleList.set_capacity(buffer_capacity);
	//boost::serialization::item_version_type item_version(0);
	//boost::serialization::collection_size_type count;
	//ar >> BOOST_SERIALIZATION_NVP(count);
	//ar >> BOOST_SERIALIZATION_NVP(item_version);
	//m_ParticleList.resize(count);
	//boost::serialization::stl::collection_load_impl<Archive, ParticleList>(ar, m_ParticleList, count, item_version);
	//ar >> BOOST_SERIALIZATION_NVP(m_ChunkStep);
	//DWORD ChunkSize;
	//ar >> BOOST_SERIALIZATION_NVP(ChunkSize);
	//m_Chunks.resize(ChunkSize);
	//for (int i = 0; i < (int)ChunkSize; i++)
	//{
	//	AABB aabb;
	//	ar >> boost::serialization::make_nvp(str_printf("m_chunk_%d", i).c_str(), m_Chunks[i]);
	//	ar >> boost::serialization::make_nvp(str_printf("m_chunk_%d_aabb", i).c_str(), aabb);
	//	AddEntity(m_Chunks[i].get(), aabb, m_ChunkStep, 0.01f);
	//}
}

void StaticEmitterComponent::CopyFrom(const StaticEmitterComponent& rhs)
{
	EmitterComponent::CopyFrom(rhs);
	// TODO:
}

ComponentPtr StaticEmitterComponent::Clone(void) const
{
	StaticEmitterComponentPtr ret(new StaticEmitterComponent());
	ret->CopyFrom(*this);
	return ret;
}

void StaticEmitterComponent::Update(float fElapsedTime)
{

}
//
//void StaticEmitterComponent::BuildChunks(void)
//{
//	_ASSERT(m_ChunkStep > 0.0f);
//
//	ClearAllEntity();
//
//	m_Chunks.clear();
//
//	AABB aabb = CalculateAABB();
//
//	m_Half = aabb.Center();
//
//	const Vector3 extent = aabb.Extent();
//
//	const int HalfStage[3] = {
//		(int)ceilf(extent.x / 2 / m_ChunkStep),
//		(int)ceilf(extent.y / 2 / m_ChunkStep),
//		(int)ceilf(extent.z / 2 / m_ChunkStep) };
//
//	const Vector3 HalfExtent(HalfStage[0] * m_ChunkStep, HalfStage[1] * m_ChunkStep, HalfStage[2] * m_ChunkStep);
//
//	_ASSERT(HalfExtent.magnitudeSq() > 0.0f);
//
//	m_min = m_Half - HalfExtent;
//
//	m_max = m_Half + HalfExtent;
//
//	boost::multi_array<std::vector<Particle>, 3> dim(boost::extents[HalfStage[0] * 2][HalfStage[1] * 2][HalfStage[2] * 2]);
//	ParticleList::const_iterator particle_iter = m_ParticleList.begin();
//	for (; particle_iter != m_ParticleList.end(); particle_iter++)
//	{
//		Vector3 local_part;
//		if (m_EmitterSpaceType == EmitterComponent::SpaceTypeWorld)
//		{
//			local_part = particle_iter->m_Position.transformCoord(m_Actor->m_World.inverse());
//		}
//		else
//		{
//			local_part = particle_iter->m_Position;
//		}
//		int i = (int)((local_part.x - m_min.x) / m_ChunkStep);
//		int j = (int)((local_part.y - m_min.y) / m_ChunkStep);
//		int k = (int)((local_part.z - m_min.z) / m_ChunkStep);
//		dim[i][j][k].push_back(*particle_iter);
//	}
//
//	m_ParticleList.clear();
//
//	int particle_i = 0;
//	for (int i = 0; i < dim.shape()[0]; i++)
//	{
//		for (int j = 0; j < dim.shape()[1]; j++)
//		{
//			for (int k = 0; k < dim.shape()[2]; k++)
//			{
//				if (!dim[i][j][k].empty())
//				{
//					StaticEmitterChunkPtr chunk(new StaticEmitterChunk());
//					chunk->m_Start = particle_i;
//					chunk->m_Count = dim[i][j][k].size();
//					m_ParticleList.insert(m_ParticleList.begin() + particle_i, dim[i][j][k].begin(), dim[i][j][k].end());
//					particle_i += chunk->m_Count;
//					_ASSERT(m_ParticleList.size() == particle_i);
//
//					AddEntity(chunk.get(), AABB(
//						m_min.x + (i + 0) * m_ChunkStep, m_min.y + (j + 0) * m_ChunkStep, m_min.z + (k + 0) * m_ChunkStep,
//						m_min.x + (i + 1) * m_ChunkStep, m_min.y + (j + 1) * m_ChunkStep, m_min.z + (k + 1) * m_ChunkStep), m_ChunkStep, 0.01f);
//
//					m_Chunks.push_back(chunk);
//				}
//			}
//		}
//	}
//
//	m_ParticleList.linearize();
//}

void StaticEmitterComponent::AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		RenderPipeline* pipeline;
		unsigned int PassMask;
		const Vector3& LocalViewPos;
		StaticEmitterComponent* cmp;
		Callback(RenderPipeline* _pipeline, unsigned int _PassMask, const Vector3& _LocalViewPos, StaticEmitterComponent* _cmp)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, cmp(_cmp)
		{
		}
		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			StaticEmitterNode* node = dynamic_cast<StaticEmitterNode*>(oct_entity);

			int k = node->i / cmp->m_ChunkSize;

			int l = node->j / cmp->m_ChunkSize;

			StaticEmitterComponent::ChunkMap::const_iterator chunk_iter = cmp->m_Chunks.find(std::make_pair(k, l));
			_ASSERT(chunk_iter != cmp->m_Chunks.end());

			if (chunk_iter->second.m_buff)
			{
				cmp->AddParticlePairToPipeline(pipeline, PassMask, chunk_iter->second.m_buff.get() + node->particle_begin, node->particle_num, NULL, 0);
			}
		}
	};

	Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
	Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
	Callback cb(pipeline, PassMask, LocalViewPos, this);
	QueryEntity(LocalFrustum, &cb);
}

#include "StaticEmitterComponent.h"
#include "Actor.h"
#include "myResource.h"
#include "libc.h"
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

class StaticEmitterChunkIORequest : public my::IORequest
{
protected:
	std::string m_path;

	int m_Row;

	int m_Col;

public:
	StaticEmitterChunkIORequest(const char* path, int Row, int Col, int Priority)
		: IORequest(Priority)
		, m_path(path)
		, m_Row(Row)
		, m_Col(Col)
	{
		m_res.reset(new StaticEmitterChunkBuffer());
	}

	virtual void StaticEmitterChunkIORequest::LoadResource(void)
	{
		if (ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
		{
			IStreamPtr istr = ResourceMgr::getSingleton().OpenIStream(m_path.c_str());
			unsigned long BufferSize = istr->GetSize();
			StaticEmitterChunkBufferPtr buff = boost::static_pointer_cast<StaticEmitterChunkBuffer>(m_res);
			buff->resize(BufferSize / sizeof(my::Emitter::Particle));
			BufferSize = buff->size() * sizeof(my::Emitter::Particle);
			BOOST_VERIFY(istr->read(&(*buff)[0], BufferSize) == BufferSize);
		}
	}

	virtual void StaticEmitterChunkIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
	}
};

void StaticEmitterChunk::RequestResource(void)
{
	m_Requested = true;

	StaticEmitterComponent * emit_cmp = dynamic_cast<StaticEmitterComponent*>(m_Node->GetTopNode());
	if (!emit_cmp->m_ChunkPath.empty())
	{
		_ASSERT(!m_buff);

		std::string path = StaticEmitterChunk::MakeChunkPath(emit_cmp->m_ChunkPath, m_Row, m_Col);
		IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), m_Row, m_Col, 0));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(
			path, request, boost::bind(&StaticEmitterChunk::OnChunkBufferReady, this, boost::placeholders::_1));
	}
}

void StaticEmitterChunk::ReleaseResource(void)
{
	StaticEmitterComponent * emit_cmp = dynamic_cast<StaticEmitterComponent*>(m_Node->GetTopNode());
	if (!emit_cmp->m_ChunkPath.empty())
	{
		std::string path = StaticEmitterChunk::MakeChunkPath(emit_cmp->m_ChunkPath, m_Row, m_Col);
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(
			path, boost::bind(&StaticEmitterChunk::OnChunkBufferReady, this, boost::placeholders::_1));

		m_buff.reset();
	}

	m_Requested = false;
}

std::string StaticEmitterChunk::MakeChunkPath(const std::string& ChunkPath, int Row, int Col)
{
	return str_printf("%s_%d_%d", ChunkPath.c_str(), Row, Col);
}

void StaticEmitterChunk::OnChunkBufferReady(my::DeviceResourceBasePtr res)
{
	m_buff = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(res);
}

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
			StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);

			if (!chunk->IsRequested())
			{
				chunk->RequestResource();

				cmp->m_ViewedChunks.push_back(boost::shared_ptr<StaticEmitterChunk>(chunk, AutoReleaseResource<StaticEmitterChunk>()));
			}

			if (chunk->m_buff)
			{
				cmp->AddParticlePairToPipeline(pipeline, PassMask, &(*chunk->m_buff)[0], chunk->m_buff->size(), NULL, 0);
			}
		}
	};

	Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
	Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
	Callback cb(pipeline, PassMask, LocalViewPos, this);
	QueryEntity(LocalFrustum, &cb);
}

void StaticEmitterStream::Release(void)
{

}

void StaticEmitterStream::Spawn(const my::Vector3 & pos)
{
	int k = (int)(pos.z / m_emit->m_ChunkStep), l = (int)(pos.x / m_emit->m_ChunkStep);

	StaticEmitterChunkBufferPtr buff;

	std::pair<StaticEmitterComponent::ChunkMap::iterator, bool> chunk_res = m_emit->m_Chunks.insert(std::make_pair(std::make_pair(k, l), StaticEmitterChunk()));
	if (chunk_res.second)
	{
	}
}

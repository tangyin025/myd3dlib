#include "StaticEmitterComponent.h"
#include "Actor.h"
#include "myResource.h"
#include "RenderPipeline.h"
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
#include <fstream>

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

			if ((PassMask | RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)) && cmp->m_ViewedChunks.insert(chunk).second)
			{
				_ASSERT(!chunk->IsRequested());

				chunk->RequestResource();
			}

			if (chunk->m_buff)
			{
				cmp->AddParticlePairToPipeline(pipeline, PassMask, &(*chunk->m_buff)[0], chunk->m_buff->size(), NULL, 0);
			}
		}
	};

	Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
	Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
	if (PassMask | RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
	{
		ChunkSet::iterator chunk_iter = m_ViewedChunks.begin();
		float CullingDistSq = powf(m_Actor->m_CullingDist, 2.0);
		for (; chunk_iter != m_ViewedChunks.end(); )
		{
			if (((*chunk_iter)->m_OctAabb->Center() - LocalViewPos).magnitudeSq() > CullingDistSq)
			{
				(*chunk_iter)->ReleaseResource();
				chunk_iter = m_ViewedChunks.erase(chunk_iter);
			}
			else
				chunk_iter++;
		}
	}
	Callback cb(pipeline, PassMask, LocalViewPos, this);
	QueryEntity(LocalFrustum, &cb);
}

void StaticEmitterStream::Release(void)
{
	BufferMap::const_iterator buff_iter = m_buffs.begin();
	for (; buff_iter != m_buffs.end(); buff_iter++)
	{
		StaticEmitterComponent::ChunkMap::iterator chunk_iter = m_emit->m_Chunks.find(buff_iter->first);
		_ASSERT(chunk_iter != m_emit->m_Chunks.end());
		_ASSERT(chunk_iter->second.m_OctAabb);

		std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, buff_iter->first.first, buff_iter->first.second);
		std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
		std::fstream fstr(FullPath, std::ios::in | std::ios::out | std::ios::binary);
		_ASSERT(fstr.is_open());

		my::AABB chunk_box = *chunk_iter->second.m_OctAabb;
		chunk_box.m_min.y = FLT_MAX;
		chunk_box.m_max.y = FLT_MIN;
		StaticEmitterChunkBuffer::const_iterator part_iter = buff_iter->second->begin();
		for (; part_iter != buff_iter->second->end(); part_iter++)
		{
			chunk_box.unionYSelf(part_iter->m_Position.y);
			fstr.write((char *)&(*part_iter), sizeof(my::Emitter::Particle));
		}
		m_emit->RemoveEntity(&chunk_iter->second);
		m_emit->AddEntity(&chunk_iter->second, chunk_box, m_emit->m_ChunkStep, 0.1f);
	}
}

StaticEmitterChunkBuffer * StaticEmitterStream::GetBuffer(int k, int l)
{
	std::pair<StaticEmitterComponent::ChunkMap::iterator, bool> chunk_res = m_emit->m_Chunks.insert(std::make_pair(std::make_pair(k, l), StaticEmitterChunk(k, l)));
	if (chunk_res.first->second.m_buff)
	{
		return chunk_res.first->second.m_buff.get();
	}

	std::pair<BufferMap::iterator, bool> buff_res = m_buffs.insert(std::make_pair(std::make_pair(k, l), StaticEmitterChunkBufferPtr()));
	if (!buff_res.second)
	{
		return buff_res.first->second.get();
	}

	if (!chunk_res.second)
	{
		std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, k, l);
		IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), k, l, INT_MAX));
		my::ResourceMgr::getSingleton().LoadIORequestAndWait(path, request, boost::bind(&StaticEmitterStream::SetBuffer, this, k, l, boost::placeholders::_1));
		_ASSERT(!buff_res.first->second);
	}
	else
	{
		m_emit->AddEntity(&chunk_res.first->second,
			my::AABB(l * m_emit->m_ChunkStep, -4096.0f, k * m_emit->m_ChunkStep, (l + 1) * m_emit->m_ChunkStep, 4096.0f, (k + 1) * m_emit->m_ChunkStep), m_emit->m_ChunkStep, 0.1f);
		buff_res.first->second.reset(new StaticEmitterChunkBuffer());
	}

	return buff_res.first->second.get();
}

void StaticEmitterStream::SetBuffer(int k, int l, my::DeviceResourceBasePtr res)
{
	m_buffs[std::make_pair(k, l)] = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(res);
}

void StaticEmitterStream::Spawn(const my::Vector3 & Pos)
{
	int k = (int)(Pos.z / m_emit->m_ChunkStep), l = (int)(Pos.x / m_emit->m_ChunkStep);

	StaticEmitterChunkBuffer * buff = GetBuffer(k, l);

	buff->push_back(my::Emitter::Particle(Pos, my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(1, 1), 0, 0));
}

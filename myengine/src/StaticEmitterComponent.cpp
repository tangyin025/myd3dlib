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
//
//BOOST_CLASS_EXPORT(StaticEmitterChunk)

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

StaticEmitterChunk::~StaticEmitterChunk(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void StaticEmitterChunk::RequestResource(void)
{
	m_Requested = true;

	StaticEmitterComponent * emit_cmp = dynamic_cast<StaticEmitterComponent*>(m_Node->GetTopNode());
	if (!emit_cmp->m_EmitterChunkPath.empty())
	{
		_ASSERT(!m_buff);

		std::string path = StaticEmitterChunk::MakeChunkPath(emit_cmp->m_EmitterChunkPath, m_Row, m_Col);
		IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), m_Row, m_Col, 0));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(
			path, request, boost::bind(&StaticEmitterChunk::OnChunkBufferReady, this, boost::placeholders::_1));
	}
}

void StaticEmitterChunk::ReleaseResource(void)
{
	StaticEmitterComponent * emit_cmp = dynamic_cast<StaticEmitterComponent*>(m_Node->GetTopNode());
	if (!emit_cmp->m_EmitterChunkPath.empty())
	{
		std::string path = StaticEmitterChunk::MakeChunkPath(emit_cmp->m_EmitterChunkPath, m_Row, m_Col);
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

StaticEmitterComponent::StaticEmitterComponent(const char* Name, int RowChunks, int ChunkSize, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
	: EmitterComponent(ComponentTypeStaticEmitter, Name, _FaceType, _SpaceTypeWorld, _VelocityType, _PrimitiveType)
	, m_EmitterRowChunks(RowChunks)
	, m_EmitterChunkSize(ChunkSize)
	, OctRoot(0, RowChunks * ChunkSize)
{
}

template<class Archive>
void StaticEmitterComponent::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar << BOOST_SERIALIZATION_NVP(m_EmitterRowChunks);
	ar << BOOST_SERIALIZATION_NVP(m_EmitterChunkSize);
	ar << BOOST_SERIALIZATION_NVP(m_EmitterChunkPath);
	DWORD ChunkSize = m_Chunks.size();
	ar << BOOST_SERIALIZATION_NVP(ChunkSize);
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ar << boost::serialization::make_nvp("m_chunk_row", chunk_iter->first.first);
		ar << boost::serialization::make_nvp("m_chunk_col", chunk_iter->first.second);
		ar << boost::serialization::make_nvp("m_chunk", chunk_iter->second);
		ar << boost::serialization::make_nvp("m_chunk_aabb", *chunk_iter->second.m_OctAabb);
	}
}

template<class Archive>
void StaticEmitterComponent::load(Archive& ar, const unsigned int version)
{
	ClearAllEntity();
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar >> BOOST_SERIALIZATION_NVP(m_EmitterRowChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_EmitterChunkSize);
	ar >> BOOST_SERIALIZATION_NVP(m_EmitterChunkPath);
	DWORD ChunkSize;
	ar >> BOOST_SERIALIZATION_NVP(ChunkSize);
	for (int i = 0; i < (int)ChunkSize; i++)
	{
		int row, col; AABB aabb;
		ar >> boost::serialization::make_nvp("m_chunk_row", row);
		ar >> boost::serialization::make_nvp("m_chunk_col", col);
		std::pair<ChunkMap::iterator, bool> chunk_res = m_Chunks.insert(std::make_pair(std::make_pair(row, col), StaticEmitterChunk(row, col)));
		_ASSERT(chunk_res.second);
		ar >> boost::serialization::make_nvp("m_chunk", chunk_res.first->second);
		ar >> boost::serialization::make_nvp("m_chunk_aabb", aabb);
		AddEntity(&chunk_res.first->second, aabb, m_EmitterChunkSize, 0.01f);
	}
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

void StaticEmitterComponent::RequestResource(void)
{
	EmitterComponent::RequestResource();
}

void StaticEmitterComponent::ReleaseResource(void)
{
	EmitterComponent::ReleaseResource();

	ChunkSet::iterator chunk_iter = m_ViewedChunks.begin();
	for (; chunk_iter != m_ViewedChunks.end(); chunk_iter++)
	{
		(*chunk_iter)->ReleaseResource();
	}
	m_ViewedChunks.clear();
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
		StaticEmitterComponent* emit_cmp;
		Callback(RenderPipeline* _pipeline, unsigned int _PassMask, const Vector3& _LocalViewPos, StaticEmitterComponent* _emit_cmp)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, emit_cmp(_emit_cmp)
		{
		}
		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);

			if ((PassMask | RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal)) && emit_cmp->m_ViewedChunks.insert(chunk).second)
			{
				_ASSERT(!chunk->IsRequested());

				chunk->RequestResource();
			}

			if (chunk->m_buff)
			{
				emit_cmp->AddParticlePairToPipeline(pipeline, PassMask, &(*chunk->m_buff)[0], chunk->m_buff->size(), NULL, 0);
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

		std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_EmitterChunkPath, buff_iter->first.first, buff_iter->first.second);
		std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
		std::ofstream ofs(FullPath, std::ios::binary);
		_ASSERT(ofs.is_open());

		my::AABB chunk_box = *chunk_iter->second.m_OctAabb;
		chunk_box.m_min.y = FLT_MAX;
		chunk_box.m_max.y = FLT_MIN;
		StaticEmitterChunkBuffer::const_iterator part_iter = buff_iter->second->begin();
		for (; part_iter != buff_iter->second->end(); part_iter++)
		{
			chunk_box.unionYSelf(part_iter->m_Position.y);
			ofs.write((char *)&(*part_iter), sizeof(my::Emitter::Particle));
		}
		m_emit->RemoveEntity(&chunk_iter->second);
		m_emit->AddEntity(&chunk_iter->second, chunk_box, m_emit->m_EmitterChunkSize, 0.1f);
	}

	m_buffs.clear();
}

StaticEmitterChunkBuffer * StaticEmitterStream::GetBuffer(int k, int l)
{
	std::pair<BufferMap::iterator, bool> buff_res = m_buffs.insert(std::make_pair(std::make_pair(k, l), StaticEmitterChunkBufferPtr()));
	if (!buff_res.second)
	{
		return buff_res.first->second.get();
	}

	std::pair<StaticEmitterComponent::ChunkMap::iterator, bool> chunk_res = m_emit->m_Chunks.insert(std::make_pair(std::make_pair(k, l), StaticEmitterChunk(k, l)));
	if (chunk_res.first->second.m_buff)
	{
		buff_res.first->second = chunk_res.first->second.m_buff; // mask chunk buff as dirty

		return buff_res.first->second.get();
	}

	std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_EmitterChunkPath, k, l);

	if (chunk_res.second)
	{
		std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
		std::ofstream ofs(FullPath, std::ios::binary);
		_ASSERT(ofs.is_open());
		my::Emitter::Particle tmp(my::Vector3(0, 0, 0), my::Vector3(0, 0, 0), my::Vector4(1, 1, 1, 1), my::Vector2(10, 10), 0.0f, 0.0f);
		ofs.write((char*)&tmp, sizeof(tmp));
		ofs.close();

		m_emit->AddEntity(&chunk_res.first->second,
			my::AABB(l * m_emit->m_EmitterChunkSize, m_emit->m_min.y, k * m_emit->m_EmitterChunkSize, (l + 1) * m_emit->m_EmitterChunkSize, m_emit->m_max.y, (k + 1) * m_emit->m_EmitterChunkSize), m_emit->m_EmitterChunkSize, 0.1f);
	}

	IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), k, l, INT_MAX));
	my::ResourceMgr::getSingleton().LoadIORequestAndWait(path, request, boost::bind(&StaticEmitterStream::SetBuffer, this, k, l, boost::placeholders::_1));
	_ASSERT(buff_res.first->second);

	if (chunk_res.second)
	{
		buff_res.first->second->clear();
	}

	return buff_res.first->second.get();
}

void StaticEmitterStream::SetBuffer(int k, int l, my::DeviceResourceBasePtr res)
{
	m_buffs[std::make_pair(k, l)] = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(res);
}

void StaticEmitterStream::Spawn(const my::Vector3 & Position, const my::Vector3 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time)
{
	int k = (int)(Position.z / m_emit->m_EmitterChunkSize), l = (int)(Position.x / m_emit->m_EmitterChunkSize);

	if (k >= 0 && k < m_emit->m_EmitterRowChunks && l >= 0 && l < m_emit->m_EmitterRowChunks)
	{
		StaticEmitterChunkBuffer* buff = GetBuffer(k, l);

		buff->push_back(my::Emitter::Particle(Position, Velocity, Color, Size, Angle, Time));
	}
}

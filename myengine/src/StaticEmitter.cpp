#include "StaticEmitter.h"
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

BOOST_CLASS_EXPORT(StaticEmitter)

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
		if (boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(m_res)->empty())
		{
			m_res.reset();
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
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

	StaticEmitter * emit_cmp = dynamic_cast<StaticEmitter*>(m_Node->GetTopNode());
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
	StaticEmitter * emit_cmp = dynamic_cast<StaticEmitter*>(m_Node->GetTopNode());
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

StaticEmitter::StaticEmitter(const char* Name, const my::AABB & LocalRootAabb, float ChunkWidth, FaceType _FaceType, SpaceType _SpaceTypeWorld, VelocityType _VelocityType, PrimitiveType _PrimitiveType)
	: EmitterComponent(Name, _FaceType, _SpaceTypeWorld, _VelocityType, _PrimitiveType)
	, m_ChunkWidth(ChunkWidth)
	, m_ChunkLodScale(1.0f)
	, OctRoot(LocalRootAabb.m_min, LocalRootAabb.m_max)
{
}

template<class Archive>
void StaticEmitter::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkWidth);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkPath);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
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
void StaticEmitter::load(Archive& ar, const unsigned int version)
{
	ClearAllEntity();
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(EmitterComponent);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkWidth);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkPath);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
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
		AddEntity(&chunk_res.first->second, aabb, m_ChunkWidth, 0.01f);
	}
}

void StaticEmitter::RequestResource(void)
{
	EmitterComponent::RequestResource();
}

void StaticEmitter::ReleaseResource(void)
{
	EmitterComponent::ReleaseResource();

	ChunkSet::iterator chunk_iter = m_ViewedChunks.begin();
	for (; chunk_iter != m_ViewedChunks.end(); chunk_iter++)
	{
		chunk_iter->ReleaseResource();
	}
	m_ViewedChunks.clear();
}

void StaticEmitter::Update(float fElapsedTime)
{

}

my::AABB StaticEmitter::CalculateAABB(void) const
{
	if (m_Chunks.empty())
	{
		return Component::CalculateAABB();
	}
	AABB ret = AABB::Invalid();
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ret.unionSelf(*chunk_iter->second.m_OctAabb);
	}
	return ret;
}

void StaticEmitter::AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		RenderPipeline* pipeline;
		unsigned int PassMask;
		const Vector3& LocalViewPos;
		StaticEmitter* emit_cmp;
		ChunkSet::iterator insert_chunk_iter;
		Callback(RenderPipeline* _pipeline, unsigned int _PassMask, const Vector3& _LocalViewPos, StaticEmitter* _emit_cmp)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, emit_cmp(_emit_cmp)
			, insert_chunk_iter(_emit_cmp->m_ViewedChunks.begin())
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);
			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod = emit_cmp->m_Actor->CalculateLod(chunk->m_OctAabb->Center(), LocalViewPos, emit_cmp->m_ChunkLodScale / emit_cmp->m_Actor->m_Scale.x);
			}

			if (chunk->m_Lod >= 1)
			{
				return true;
			}

			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				if (chunk->is_linked())
				{
					ChunkSet::iterator chunk_iter = emit_cmp->m_ViewedChunks.iterator_to(*chunk);
					if (chunk_iter != insert_chunk_iter)
					{
						emit_cmp->m_ViewedChunks.erase(chunk_iter);

						emit_cmp->m_ViewedChunks.insert(insert_chunk_iter, *chunk);
					}
					else
					{
						_ASSERT(insert_chunk_iter != emit_cmp->m_ViewedChunks.end());

						insert_chunk_iter++;
					}
				}
				else
				{
					_ASSERT(!chunk->IsRequested());

					chunk->RequestResource();

					emit_cmp->m_ViewedChunks.insert(insert_chunk_iter, *chunk);
				}
			}

			if (chunk->m_buff)
			{
				emit_cmp->AddParticlePairToPipeline(pipeline, PassMask, &(*chunk->m_buff)[0], chunk->m_buff->size(), NULL, 0);
			}
			return true;
		}
	};

	Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
	Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
	Callback cb(pipeline, PassMask, LocalViewPos, this);
	QueryEntity(LocalFrustum, &cb);

	if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
	{
		const float LocalCullingDistSq = powf(m_Actor->m_LodDist * powf(m_Actor->m_LodFactor, 1) * m_ChunkLodScale / m_Actor->m_Scale.x, 2.0);
		ChunkSet::iterator chunk_iter = cb.insert_chunk_iter;
		for (; chunk_iter != m_ViewedChunks.end(); )
		{
			if ((chunk_iter->m_OctAabb->Center() - LocalViewPos).magnitudeSq() > LocalCullingDistSq)
			{
				chunk_iter->ReleaseResource();

				chunk_iter = m_ViewedChunks.erase(chunk_iter);
			}
			else
				chunk_iter++;
		}
	}
}

void StaticEmitterStream::Release(void)
{
	std::map<std::pair<int, int>, bool>::iterator dirty_iter = m_dirty.begin();
	for (; dirty_iter != m_dirty.end(); dirty_iter++)
	{
		if (dirty_iter->second)
		{
			StaticEmitterChunkBuffer* buff = GetBuffer(dirty_iter->first.first, dirty_iter->first.second);
			_ASSERT(buff);

			StaticEmitter::ChunkMap::iterator chunk_iter = m_emit->m_Chunks.find(dirty_iter->first);
			_ASSERT(chunk_iter != m_emit->m_Chunks.end());
			_ASSERT(chunk_iter->second.m_OctAabb);

			std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, dirty_iter->first.first, dirty_iter->first.second);
			std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
			std::ofstream ofs(FullPath, std::ios::binary, _SH_DENYRW);
			_ASSERT(ofs.is_open());

			my::AABB chunk_box = *chunk_iter->second.m_OctAabb;
			chunk_box.m_min.y = FLT_MAX;
			chunk_box.m_max.y = FLT_MIN;
			StaticEmitterChunkBuffer::const_iterator part_iter = buff->begin();
			for (; part_iter != buff->end(); part_iter++)
			{
				chunk_box.m_min.y = Min(chunk_box.m_min.y, part_iter->m_Position.y - part_iter->m_Size.y * 0.5f);
				chunk_box.m_max.y = Max(chunk_box.m_max.y, part_iter->m_Position.y + part_iter->m_Size.y * 0.5f);
				ofs.write((char*)&(*part_iter), sizeof(my::Emitter::Particle));
			}
			m_emit->RemoveEntity(&chunk_iter->second);
			m_emit->AddEntity(&chunk_iter->second, chunk_box, m_emit->m_ChunkWidth, 0.1f);

			dirty_iter->second = false;
		}
	}
}

StaticEmitterChunkBuffer * StaticEmitterStream::GetBuffer(int i, int j)
{
	BufferMap::const_iterator buff_iter = m_buffs.find(std::make_pair(i, j));
	if (buff_iter != m_buffs.end())
	{
		return buff_iter->second.get();
	}

	StaticEmitter::ChunkMap::const_iterator chunk_iter = m_emit->m_Chunks.find(std::make_pair(i, j));
	if (chunk_iter == m_emit->m_Chunks.end())
	{
		return NULL;
	}

	if (chunk_iter->second.m_buff)
	{
		std::pair<BufferMap::iterator, bool> buff_res = m_buffs.insert(std::make_pair(std::make_pair(i, j), chunk_iter->second.m_buff));
		_ASSERT(buff_res.second);
		return buff_res.first->second.get();
	}

	// ! m_emit->m_Chunks[std::make_pair(i, j)] may have requested
	std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, i, j);
	IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), i, j, INT_MAX));
	my::ResourceMgr::getSingleton().LoadIORequestAndWait(path, request, boost::bind(&StaticEmitterStream::SetBuffer, this, i, j, boost::placeholders::_1));

	buff_iter = m_buffs.find(std::make_pair(i, j));
	_ASSERT(buff_iter != m_buffs.end());
	return buff_iter->second.get();
}

void StaticEmitterStream::SetBuffer(int i, int j, my::DeviceResourceBasePtr res)
{
	m_buffs[std::make_pair(i, j)] = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(res);
}

void StaticEmitterStream::Spawn(const my::Vector4 & Position, const my::Vector4 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time)
{
	int i = Position.z / m_emit->m_ChunkWidth, j = Position.x / m_emit->m_ChunkWidth;

	StaticEmitterChunkBuffer* buff = GetBuffer(i, j);
	if (!buff)
	{
		std::pair<StaticEmitter::ChunkMap::iterator, bool> chunk_res = m_emit->m_Chunks.insert(std::make_pair(std::make_pair(i, j), StaticEmitterChunk(i, j)));
		_ASSERT(chunk_res.second);

		m_emit->AddEntity(&chunk_res.first->second,
			my::AABB(j * m_emit->m_ChunkWidth, m_emit->m_min.y, i * m_emit->m_ChunkWidth, (j + 1) * m_emit->m_ChunkWidth, m_emit->m_max.y, (i + 1) * m_emit->m_ChunkWidth), m_emit->m_ChunkWidth, 0.1f);

		std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, i, j);

		std::pair<BufferMap::iterator, bool> buff_res = m_buffs.insert(std::make_pair(std::make_pair(i, j),
			boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(my::ResourceMgr::getSingleton().AddResource(path, DeviceResourceBasePtr(new StaticEmitterChunkBuffer())))));
		_ASSERT(buff_res.second);

		buff = buff_res.first->second.get();
	}

	buff->push_back(my::Emitter::Particle(Position, Velocity, Color, Size, Angle, Time));

	m_dirty[std::make_pair(i, j)] = true;
}

my::Emitter::Particle * StaticEmitterStream::GetFirstNearParticle2D(const my::Vector3 & Center, float Range)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		StaticEmitterStream & estr;
		const Vector3 & Center;
		const float RangeSq;
		my::Emitter::Particle * near_particle;
		Callback(StaticEmitterStream & _estr, const Vector3 & _Center, float _range)
			: estr(_estr)
			, Center(_Center)
			, RangeSq(_range * _range)
			, near_particle(NULL)
		{
		}
		virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			StaticEmitterChunk * chunk = dynamic_cast<StaticEmitterChunk *>(oct_entity);
			StaticEmitterChunkBuffer * buff = estr.GetBuffer(chunk->m_Row, chunk->m_Col);
			_ASSERT(buff);
			StaticEmitterChunkBuffer::iterator part_iter = buff->begin();
			for (; part_iter != buff->end(); part_iter++)
			{
				float DistSq = (part_iter->m_Position.xyz - Center).magnitudeSq2D();
				if (DistSq < RangeSq)
				{
					near_particle = &(*part_iter);
					return false;
				}
			}
			return true;
		}
	};

	Callback cb(*this, Center, Range);
	m_emit->QueryEntity(AABB(Center.x - Range, m_emit->m_min.y, Center.z - Range, Center.x + Range, m_emit->m_max.y, Center.z + Range), &cb);
	return cb.near_particle;
}

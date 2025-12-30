// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "StaticEmitter.h"
#include "Actor.h"
#include "myResource.h"
#include "myDxutApp.h"
#include "Material.h"
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
		IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), m_Row, m_Col,
			m_Lod <= 0 ? Component::ResPriorityLod0 : m_Lod <= 1 ? Component::ResPriorityLod1 : Component::ResPriorityLod2));
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
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);
			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod = emit_cmp->m_Actor->CalculateLod((chunk->m_OctAabb->Center() - LocalViewPos).magnitude() / emit_cmp->m_ChunkLodScale);
			}

			if (chunk->m_Lod >= StaticEmitter::LastLod)
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
						emit_cmp->m_ViewedChunks.splice(insert_chunk_iter, emit_cmp->m_ViewedChunks, chunk_iter);
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

			if (chunk->m_Lod >= 0 && chunk->m_buff)
			{
				switch (emit_cmp->m_ParticlePrimitiveType)
				{
				case PrimitiveTypeTri:
				case PrimitiveTypeQuad:
				{
					_ASSERT(emit_cmp->m_ParticlePrimitiveType < RenderPipeline::ParticlePrimitiveTypeCount);
					emit_cmp->AddParticlePairToPipeline(pipeline, pipeline->m_ParticleVb.m_ptr, pipeline->m_ParticleIb.m_ptr, pipeline->m_ParticleIEDecl,
						RenderPipeline::m_ParticlePrimitiveInfo[emit_cmp->m_ParticlePrimitiveType][RenderPipeline::ParticlePrimitiveMinVertexIndex],
						RenderPipeline::m_ParticlePrimitiveInfo[emit_cmp->m_ParticlePrimitiveType][RenderPipeline::ParticlePrimitiveNumVertices],
						pipeline->m_ParticleVertStride,
						RenderPipeline::m_ParticlePrimitiveInfo[emit_cmp->m_ParticlePrimitiveType][RenderPipeline::ParticlePrimitiveStartIndex],
						RenderPipeline::m_ParticlePrimitiveInfo[emit_cmp->m_ParticlePrimitiveType][RenderPipeline::ParticlePrimitivePrimitiveCount],
						PassMask, chunk->m_buff->data(), chunk->m_buff->size() >> chunk->m_Lod, NULL, 0, MAKELONG(chunk->m_Row, chunk->m_Col));
					break;
				}
				case PrimitiveTypeMesh:
					if (emit_cmp->m_Mesh)
					{
						if (!emit_cmp->m_Decl)
						{
							std::vector<D3DVERTEXELEMENT9> ve = emit_cmp->m_Mesh->m_VertexElems.BuildVertexElementList(0);
							std::vector<D3DVERTEXELEMENT9> ie = pipeline->m_ParticleInstanceElems.BuildVertexElementList(1);
							ve.insert(ve.end(), ie.begin(), ie.end());
							D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
							ve.push_back(ve_end);

							HRESULT hr;
							if (FAILED(hr = my::D3DContext::getSingleton().m_d3dDevice->CreateVertexDeclaration(&ve[0], &emit_cmp->m_Decl)))
							{
								THROW_D3DEXCEPTION(hr);
							}
						}

						emit_cmp->AddParticlePairToPipeline(pipeline, emit_cmp->m_Mesh->m_Vb.m_ptr, emit_cmp->m_Mesh->m_Ib.m_ptr, emit_cmp->m_Decl,
							emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].VertexStart,
							emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].VertexCount,
							emit_cmp->m_Mesh->GetNumBytesPerVertex(),
							emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].FaceStart * 3,
							emit_cmp->m_Mesh->m_AttribTable[emit_cmp->m_MeshSubMeshId].FaceCount,
							PassMask, chunk->m_buff->data(), chunk->m_buff->size() >> chunk->m_Lod, NULL, 0, MAKELONG(chunk->m_Row, chunk->m_Col));
					}
					break;
				}
			}
			return true;
		}
	};

	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		const float LocalCullingDist = m_Actor->m_LodDist * powf(m_Actor->m_LodFactor, StaticEmitter::LastLod) * m_ChunkLodScale;

		LocalFrustum.Near.normalizeSelf();
		LocalFrustum.Near.d = Min(LocalFrustum.Near.d, LocalCullingDist - LocalViewPos.dot(LocalFrustum.Near.normal));
		LocalFrustum.Far.normalizeSelf();
		LocalFrustum.Far.d = Min(LocalFrustum.Far.d, LocalCullingDist - LocalViewPos.dot(LocalFrustum.Far.normal));

		Callback cb(pipeline, PassMask, LocalViewPos, this);
		QueryEntity(LocalFrustum, &cb);

		if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
		{
			ChunkSet::iterator chunk_iter = cb.insert_chunk_iter;
			for (; chunk_iter != m_ViewedChunks.end(); )
			{
				if ((chunk_iter->m_OctAabb->Center() - LocalViewPos).magnitudeSq() > LocalCullingDist * LocalCullingDist)
				{
					chunk_iter->ReleaseResource();

					chunk_iter = m_ViewedChunks.erase(chunk_iter);
				}
				else
					chunk_iter++;
			}
		}
	}
}

void StaticEmitterStream::Flush(void)
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
			std::basic_string<TCHAR> FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
			if (buff->empty())
			{
				StaticEmitterChunkBufferPtr dummy = buff->shared_from_this();
				if (chunk_iter->second.is_linked())
				{
					StaticEmitter::ChunkSet::iterator viewed_chunk_iter = m_emit->m_ViewedChunks.iterator_to(chunk_iter->second);
					_ASSERT(viewed_chunk_iter != m_emit->m_ViewedChunks.end());
					viewed_chunk_iter->ReleaseResource();
					m_emit->m_ViewedChunks.erase(viewed_chunk_iter);
				}

				BOOST_VERIFY(!my::ResourceMgr::getSingleton().CheckPath(path.c_str()) || 0 == _tunlink(FullPath.c_str()));
				m_emit->RemoveEntity(&chunk_iter->second);
				m_buffs.erase(chunk_iter->first);
				m_emit->m_Chunks.erase(chunk_iter);

				_ASSERT(dummy.use_count() == 2);
				dirty_iter->second = false;
				continue;
			}

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
			m_buffs.erase(chunk_iter->first);

			dirty_iter->second = false;
		}
	}

	// ! StaticEmitterStream::Spawn, AddResource
	my::ResourceMgr::getSingleton().CheckIORequests(0);
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

	std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, i, j);
	IORequestPtr request(new StaticEmitterChunkIORequest(path.c_str(), i, j, INT_MAX));
	//request->LoadResource();
	//request->CreateResource(NULL);
	//m_buffs[std::make_pair(i, j)] = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(request->m_res);

	struct Tmp
	{
		static void Set(BufferMap* buffs, int i, int j, my::DeviceResourceBasePtr res)
		{
			(*buffs)[std::make_pair(i, j)] = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(res);
		}
	};
	my::ResourceMgr::getSingleton().LoadIORequestAndWait(path, request, boost::bind(&Tmp::Set, &m_buffs, i, j, boost::placeholders::_1));

	buff_iter = m_buffs.find(std::make_pair(i, j));
	_ASSERT(buff_iter != m_buffs.end());
	return buff_iter->second.get();
}

void StaticEmitterStream::SpawnBuffer(const my::Vector4 & Position, const my::Vector4 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time)
{
	int i = floor(Position.z / m_emit->m_ChunkWidth), j = floor(Position.x / m_emit->m_ChunkWidth);

	StaticEmitterChunkBuffer* buff = GetBuffer(i, j);
	_ASSERT(buff);

	buff->push_back(my::Emitter::Particle(Position, Velocity, Color, Size, Angle, Time));

	m_dirty[std::make_pair(i, j)] = true;
}

void StaticEmitterStream::Spawn(const my::Vector4 & Position, const my::Vector4 & Velocity, const my::Vector4 & Color, const my::Vector2 & Size, float Angle, float Time)
{
	int i = floor(Position.z / m_emit->m_ChunkWidth), j = floor(Position.x / m_emit->m_ChunkWidth);

	BufferMap::const_iterator buff_iter = m_buffs.find(std::make_pair(i, j));
	if (buff_iter != m_buffs.end())
	{
		SpawnBuffer(Position, Velocity, Color, Size, Angle, Time);
		return;
	}

	StaticEmitter::ChunkMap::const_iterator chunk_iter = m_emit->m_Chunks.find(std::make_pair(i, j));
	if (chunk_iter == m_emit->m_Chunks.end())
	{
		std::pair<StaticEmitter::ChunkMap::iterator, bool> chunk_res = m_emit->m_Chunks.insert(std::make_pair(std::make_pair(i, j), StaticEmitterChunk(i, j)));
		_ASSERT(chunk_res.second);

		m_emit->AddEntity(&chunk_res.first->second, my::AABB(
			(j + 0) * m_emit->m_ChunkWidth, Position.y - Size.y * 0.5f, (i + 0) * m_emit->m_ChunkWidth,
			(j + 1) * m_emit->m_ChunkWidth, Position.y + Size.y * 0.5f, (i + 1) * m_emit->m_ChunkWidth), m_emit->m_ChunkWidth, 0.1f);

		std::pair<BufferMap::iterator, bool> res = m_buffs.insert(std::make_pair(std::make_pair(i, j), StaticEmitterChunkBufferPtr(new StaticEmitterChunkBuffer())));
		_ASSERT(res.second);
		my::ResourceMgr::getSingleton().AddResource(StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, i, j), res.first->second);
	}
	//else if (chunk_iter->second.IsRequested()) //else if (chunk_iter->second.m_buff)
	//{
	//	SpawnBuffer(Position, Velocity, Color, Size, Angle, Time);
	//	return;
	//}

	//std::string path = StaticEmitterChunk::MakeChunkPath(m_emit->m_ChunkPath, i, j);
	//std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
	//std::ofstream ofs(FullPath, std::ios::binary | std::ios::app, _SH_DENYRW);
	//_ASSERT(ofs.is_open());

	//my::Emitter::Particle part(Position, Velocity, Color, Size, Angle, Time);
	//ofs.write((char*)&part, sizeof(part));

	SpawnBuffer(Position, Velocity, Color, Size, Angle, Time);
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
		virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb)
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

void StaticEmitterStream::EraseParticles(const my::Vector3& Center, float Range)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		StaticEmitterStream& estr;
		const Vector3& Center;
		const float RangeSq;
		my::Emitter::Particle* near_particle;
		Callback(StaticEmitterStream& _estr, const Vector3& _Center, float _range)
			: estr(_estr)
			, Center(_Center)
			, RangeSq(_range* _range)
			, near_particle(NULL)
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb)
		{
			StaticEmitterChunk* chunk = dynamic_cast<StaticEmitterChunk*>(oct_entity);
			StaticEmitterChunkBuffer* buff = estr.GetBuffer(chunk->m_Row, chunk->m_Col);
			_ASSERT(buff);
			StaticEmitterChunkBuffer::iterator part_iter = buff->begin();
			for (; part_iter != buff->end(); )
			{
				float DistSq = (part_iter->m_Position.xyz - Center).magnitudeSq2D();
				if (DistSq < RangeSq)
				{
					part_iter = buff->erase(part_iter);

					estr.m_dirty[std::make_pair(chunk->m_Row, chunk->m_Col)] = true;
				}
				else
				{
					part_iter++;
				}
			}
			return true;
		}
	};

	Callback cb(*this, Center, Range);
	m_emit->QueryEntity(AABB(Center.x - Range, m_emit->m_min.y, Center.z - Range, Center.x + Range, m_emit->m_max.y, Center.z + Range), &cb);
}

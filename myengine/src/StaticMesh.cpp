#include "StaticMesh.h"
#include "Actor.h"
#include "myEffect.h"
#include "Material.h"
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

BOOST_CLASS_EXPORT(StaticMesh)

using namespace my;

StaticMeshChunk::StaticMeshChunk(void)
	: m_Requested(false)
{

}

template<class Archive>
void StaticMeshChunk::save(Archive& ar, const unsigned int version) const
{

}

template<class Archive>
void StaticMeshChunk::load(Archive& ar, const unsigned int version)
{

}

void StaticMeshChunk::RequestResource(void)
{
	m_Requested = true;
}

void StaticMeshChunk::ReleaseResource(void)
{
	m_Requested = false;
}

template<class Archive>
void StaticMesh::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
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
void StaticMesh::load(Archive& ar, const unsigned int version)
{
	ClearAllEntity();
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
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
		std::pair<ChunkMap::iterator, bool> chunk_res = m_Chunks.insert(std::make_pair(std::make_pair(row, col), StaticMeshChunk()));
		_ASSERT(chunk_res.second);
		ar >> boost::serialization::make_nvp("m_chunk", chunk_res.first->second);
		ar >> boost::serialization::make_nvp("m_chunk_aabb", aabb);
		AddEntity(&chunk_res.first->second, aabb, m_ChunkWidth, 0.01f);
	}
}

void StaticMesh::RequestResource(void)
{
	Component::RequestResource();
}

void StaticMesh::ReleaseResource(void)
{
	Component::ReleaseResource();

	ChunkSet::iterator chunk_iter = m_ViewedChunks.begin();
	for (; chunk_iter != m_ViewedChunks.end(); chunk_iter++)
	{
		chunk_iter->ReleaseResource();
	}
	m_ViewedChunks.clear();
}

void StaticMesh::OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam)
{

}

void StaticMesh::Update(float fElapsedTime)
{

}

void StaticMesh::AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
{
	_ASSERT(m_Actor);

	struct Callback : public my::OctNode::QueryCallback
	{
		RenderPipeline* pipeline;
		unsigned int PassMask;
		const Vector3& LocalViewPos;
		StaticMesh* mesh_cmp;
		ChunkSet::iterator insert_chunk_iter;
		Callback(RenderPipeline* _pipeline, unsigned int _PassMask, const Vector3& _LocalViewPos, StaticMesh* _mesh_cmp)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, mesh_cmp(_mesh_cmp)
			, insert_chunk_iter(_mesh_cmp->m_ViewedChunks.begin())
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			StaticMeshChunk* chunk = dynamic_cast<StaticMeshChunk*>(oct_entity);

			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				if (chunk->is_linked())
				{
					ChunkSet::iterator chunk_iter = mesh_cmp->m_ViewedChunks.iterator_to(*chunk);
					if (chunk_iter != insert_chunk_iter)
					{
						mesh_cmp->m_ViewedChunks.splice(insert_chunk_iter, mesh_cmp->m_ViewedChunks, chunk_iter);
					}
					else
					{
						_ASSERT(insert_chunk_iter != mesh_cmp->m_ViewedChunks.end());

						insert_chunk_iter++;
					}
				}
				else
				{
					_ASSERT(!chunk->IsRequested());

					chunk->RequestResource();

					mesh_cmp->m_ViewedChunks.insert(insert_chunk_iter, *chunk);
				}
			}

			return true;
		}
	};

	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		const float LocalCullingDist = m_Actor->m_LodDist * powf(m_Actor->m_LodFactor, 1) * m_ChunkLodScale;

		LocalFrustum.Near.normalizeSelf();
		LocalFrustum.Near.d = Min(LocalFrustum.Near.d, LocalCullingDist - LocalViewPos.dot(LocalFrustum.Near.normal));
		LocalFrustum.Far.normalizeSelf();
		LocalFrustum.Far.d = Min(LocalFrustum.Far.d, LocalCullingDist - LocalViewPos.dot(LocalFrustum.Far.normal));

		Callback cb(pipeline, PassMask, LocalViewPos, this);
		QueryEntity(LocalFrustum, &cb);

		if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
		{
			ChunkSet::iterator chunk_iter = cb.insert_chunk_iter;
			for (; chunk_iter != m_ViewedChunks.end(); chunk_iter++)
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

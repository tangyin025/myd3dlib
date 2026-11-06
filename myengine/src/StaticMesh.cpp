#include "StaticMesh.h"
#include "Actor.h"
#include "myEffect.h"
#include "myDxutApp.h"
#include "Material.h"
#include "PhysxContext.h"
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

BOOST_CLASS_EXPORT(StaticMesh)

using namespace my;

StaticMeshChunk::~StaticMeshChunk(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void StaticMeshChunk::RequestResource(void)
{
	m_Requested = true;

	StaticMesh* mesh_cmp = dynamic_cast<StaticMesh*>(m_Node->GetTopNode());
	if (!mesh_cmp->m_ChunkPath.empty())
	{
		_ASSERT(!m_Mesh);

		std::string path = StaticMeshChunk::MakeChunkPath(mesh_cmp->m_ChunkPath, m_Row, m_Col);
		IORequestPtr request(new MeshIORequest(path.c_str(),
			m_Lod <= 0 ? Component::ResPriorityLod0 : m_Lod <= 1 ? Component::ResPriorityLod1 : Component::ResPriorityLod2));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(
			path, request, boost::bind(&StaticMeshChunk::OnChunkMeshReady, this, boost::placeholders::_1));
	}

	if (!mesh_cmp->m_PxChunkPath.empty())
	{
		_ASSERT(!m_PxMesh);

		std::string path = StaticMeshChunk::MakeChunkPath(mesh_cmp->m_PxChunkPath, m_Row, m_Col);
		IORequestPtr request(new MeshComponent::PhysxTriangleMeshIORequest(path.c_str(),
			m_Lod <= 0 ? Component::ResPriorityLod0 : m_Lod <= 1 ? Component::ResPriorityLod1 : Component::ResPriorityLod2));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(
			path, request, boost::bind(&StaticMeshChunk::OnChunkPxMeshReady, this, boost::placeholders::_1));
	}
}

void StaticMeshChunk::ReleaseResource(void)
{
	StaticMesh* mesh_cmp = dynamic_cast<StaticMesh*>(m_Node->GetTopNode());
	if (!mesh_cmp->m_ChunkPath.empty())
	{
		std::string path = StaticMeshChunk::MakeChunkPath(mesh_cmp->m_ChunkPath, m_Row, m_Col);
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(path.c_str(),
			boost::bind(&StaticMeshChunk::OnChunkMeshReady, this, boost::placeholders::_1));

		m_Mesh.reset();
	}

	if (!mesh_cmp->m_PxChunkPath.empty())
	{
		std::string path = StaticMeshChunk::MakeChunkPath(mesh_cmp->m_PxChunkPath, m_Row, m_Col);
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(path.c_str(),
			boost::bind(&StaticMeshChunk::OnChunkPxMeshReady, this, boost::placeholders::_1));

		if (m_PxMesh)
		{
			_ASSERT(m_PxShape);

			if (mesh_cmp->m_Actor->m_PxActor)
			{
				_ASSERT(m_PxShape->getActor() == mesh_cmp->m_Actor->m_PxActor.get());

				mesh_cmp->m_Actor->m_PxActor->detachShape(*m_PxShape);
			}

			m_PxShape.reset();

			m_PxMesh.reset();
		}
	}

	m_Requested = false;
}

std::string StaticMeshChunk::MakeChunkPath(const std::string& ChunkPath, int Row, int Col)
{
	return str_printf("%s_%d_%d", ChunkPath.c_str(), Row, Col);
}

void StaticMeshChunk::OnChunkMeshReady(my::DeviceResourceBasePtr res)
{
	m_Mesh = boost::dynamic_pointer_cast<OgreMesh>(res);
}

void StaticMeshChunk::OnChunkPxMeshReady(my::DeviceResourceBasePtr res)
{
	_ASSERT(!m_PxMesh);

	m_PxMesh = boost::dynamic_pointer_cast<MeshComponent::PhysxBaseResource>(res);

	StaticMesh* mesh_cmp = dynamic_cast<StaticMesh*>(m_Node->GetTopNode());

	_ASSERT(mesh_cmp->m_Actor && !m_PxShape);

	physx::PxMaterial* material = Component::CreatePhysxMaterial(MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)mesh_cmp->m_Actor->m_Scale, physx::PxQuat(physx::PxIdentity));
	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxTriangleMeshGeometry(m_PxMesh->m_ptr->is<physx::PxTriangleMesh>(), mesh_scaling, physx::PxMeshGeometryFlags()),
		*material, true, physx::PxShapeFlags(mesh_cmp->m_DescShapeFlags)), PhysxDeleter<physx::PxShape>());
	m_PxShape->userData = this;
	m_PxShape->setSimulationFilterData(mesh_cmp->m_DescSimulationFilter);
	m_PxShape->setQueryFilterData(mesh_cmp->m_DescQueryFilter);
	if (mesh_cmp->m_Actor && mesh_cmp->m_Actor->m_PxActor)
	{
		mesh_cmp->m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

template<class Archive>
void StaticMesh::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkWidth);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkPath);
	ar << BOOST_SERIALIZATION_NVP(m_PxChunkPath);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
	unsigned int SimulationFilterWord0, SimulationFilterWord1, SimulationFilterWord2, SimulationFilterWord3;
	GetSimulationFilter(SimulationFilterWord0, SimulationFilterWord1, SimulationFilterWord2, SimulationFilterWord3);
	ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
	ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord1);
	//ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord2);
	//ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord3);
	unsigned int QueryFilterWord0, QueryFilterWord1, QueryFilterWord2, QueryFilterWord3;
	GetQueryFilter(QueryFilterWord0, QueryFilterWord1, QueryFilterWord2, QueryFilterWord3);
	ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
	ar << BOOST_SERIALIZATION_NVP(QueryFilterWord1);
	//ar << BOOST_SERIALIZATION_NVP(QueryFilterWord2);
	//ar << BOOST_SERIALIZATION_NVP(QueryFilterWord3);
	unsigned int ShapeFlags = GetShapeFlags();
	ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
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
	ar >> BOOST_SERIALIZATION_NVP(m_PxChunkPath);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
	unsigned int SimulationFilterWord0, SimulationFilterWord1, SimulationFilterWord2 = 0, SimulationFilterWord3 = 0;
	ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
	ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord1);
	//ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord2);
	//ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord3);
	SetSimulationFilter(SimulationFilterWord0, SimulationFilterWord1, SimulationFilterWord2, SimulationFilterWord3);
	unsigned int QueryFilterWord0, QueryFilterWord1, QueryFilterWord2 = 0, QueryFilterWord3 = 0;
	ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
	ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord1);
	//ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord2);
	//ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord3);
	SetQueryFilter(QueryFilterWord0, QueryFilterWord1, QueryFilterWord2, QueryFilterWord3);
	unsigned int ShapeFlags;
	ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
	SetShapeFlags(ShapeFlags);
	DWORD ChunkSize;
	ar >> BOOST_SERIALIZATION_NVP(ChunkSize);
	for (int i = 0; i < (int)ChunkSize; i++)
	{
		int row, col; AABB aabb;
		ar >> boost::serialization::make_nvp("m_chunk_row", row);
		ar >> boost::serialization::make_nvp("m_chunk_col", col);
		std::pair<ChunkMap::iterator, bool> chunk_res = m_Chunks.insert(std::make_pair(std::make_pair(row, col), StaticMeshChunk(row, col)));
		_ASSERT(chunk_res.second);
		ar >> boost::serialization::make_nvp("m_chunk", chunk_res.first->second);
		ar >> boost::serialization::make_nvp("m_chunk_aabb", aabb);
		AddEntity(&chunk_res.first->second, aabb, m_ChunkWidth, 0.01f);
	}
}

void StaticMesh::OnResetShader(void)
{
	handle_World = NULL;
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
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, Vector4(1.0f));
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
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, DWORD intersect_type)
		{
			StaticMeshChunk* chunk = dynamic_cast<StaticMeshChunk*>(oct_entity);
			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod = mesh_cmp->m_Actor->CalculateLod((chunk->m_OctAabb->Center() - LocalViewPos).magnitude() / mesh_cmp->m_ChunkLodScale);
			}

			if (chunk->m_Lod >= StaticMesh::LastLod)
			{
				return true;
			}

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

			if (chunk->m_Lod >= 0 && chunk->m_Mesh)
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (mesh_cmp->m_Material->m_PassMask & PassMask))
					{
						D3DXMACRO macros[4] = { { "MESH_TYPE", "0" }, { 0 } };
						my::Effect* shader = pipeline->QueryShader(mesh_cmp->m_Material->m_Shader.c_str(), macros, PassID);
						if (shader)
						{
							if (!mesh_cmp->handle_World)
							{
								BOOST_VERIFY(mesh_cmp->handle_World = shader->GetParameterByName(NULL, "g_World"));
								BOOST_VERIFY(mesh_cmp->handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
							}

							for (unsigned int subid = 0; subid < chunk->m_Mesh->GetNumAttributes(); subid++)
							{
								pipeline->PushMesh(PassID, chunk->m_Mesh.get(), subid, shader, mesh_cmp, mesh_cmp->m_Material.get(), MAKELONG(chunk->m_Row, chunk->m_Col));
							}
						}
					}
				}
			}
			return true;
		}
	};

	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		const float LocalCullingDist = m_Actor->m_LodDist * powf(m_Actor->m_LodFactor, StaticMesh::LastLod) * m_ChunkLodScale;
		LocalFrustum.Near.d = Min(LocalFrustum.Near.d, LocalFrustum.Near.tangentLower(LocalViewPos, LocalCullingDist));
		LocalFrustum.Far.d = Min(LocalFrustum.Far.d, LocalFrustum.Far.tangentLower(LocalViewPos, LocalCullingDist));

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

StaticMeshChunk* StaticMesh::GetChunk(int i, int j)
{
	std::pair<ChunkMap::iterator, bool> chunk_res = m_Chunks.insert(std::make_pair(std::make_pair(i, j), StaticMeshChunk(i, j)));
	if (chunk_res.second)
	{
		AddEntity(&chunk_res.first->second, my::AABB(
			(j + 0) * m_ChunkWidth, -0.5f, (i + 0) * m_ChunkWidth,
			(j + 1) * m_ChunkWidth,  0.5f, (i + 1) * m_ChunkWidth), m_ChunkWidth, 0.01f);
	}
	return &chunk_res.first->second;
}

void StaticMesh::SetSimulationFilter(unsigned int word0, unsigned int word1, unsigned int word2, unsigned int word3)
{
	m_DescSimulationFilter = physx::PxFilterData(word0, word1, word2, word3);

	ChunkMap::iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		if (chunk_iter->second.m_PxShape)
		{
			chunk_iter->second.m_PxShape->setSimulationFilterData(m_DescSimulationFilter);
		}
	}
}

void StaticMesh::GetSimulationFilter(unsigned int& word0, unsigned int& word1, unsigned int& word2, unsigned int& word3) const
{
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		if (chunk_iter->second.m_PxShape)
		{
			physx::PxFilterData filter_data = chunk_iter->second.m_PxShape->getSimulationFilterData();
			word0 = filter_data.word0;
			word1 = filter_data.word1;
			word2 = filter_data.word2;
			word3 = filter_data.word3;
			return;
		}
	}

	word0 = m_DescSimulationFilter.word0;
	word1 = m_DescSimulationFilter.word1;
	word2 = m_DescSimulationFilter.word2;
	word3 = m_DescSimulationFilter.word3;
}

void StaticMesh::SetQueryFilter(unsigned int word0, unsigned int word1, unsigned int word2, unsigned int word3)
{
	m_DescQueryFilter = physx::PxFilterData(word0, word1, word2, word3);

	ChunkMap::iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		if (chunk_iter->second.m_PxShape)
		{
			chunk_iter->second.m_PxShape->setQueryFilterData(m_DescQueryFilter);
		}
	}
}

void StaticMesh::GetQueryFilter(unsigned int& word0, unsigned int& word1, unsigned int& word2, unsigned int& word3) const
{
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		if (chunk_iter->second.m_PxShape)
		{
			physx::PxFilterData filter_data = chunk_iter->second.m_PxShape->getQueryFilterData();
			word0 = filter_data.word0;
			word1 = filter_data.word1;
			word2 = filter_data.word2;
			word3 = filter_data.word3;
			return;
		}
	}

	word0 = m_DescQueryFilter.word0;
	word1 = m_DescQueryFilter.word1;
	word2 = m_DescQueryFilter.word2;
	word3 = m_DescQueryFilter.word3;
}

void StaticMesh::SetShapeFlags(unsigned int Flags)
{
	m_DescShapeFlags = Flags;

	ChunkMap::iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		if (chunk_iter->second.m_PxShape)
		{
			chunk_iter->second.m_PxShape->setFlags(physx::PxShapeFlags(Flags));
		}
	}
}

unsigned int StaticMesh::GetShapeFlags(void) const
{
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		if (chunk_iter->second.m_PxShape)
		{
			return chunk_iter->second.m_PxShape->getFlags();
		}
	}

	return m_DescShapeFlags;
}

void StaticMesh::ClearShape(void)
{
	Component::ClearShape();

	ChunkMap::iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		chunk_iter->second.m_PxMesh.reset();

		chunk_iter->second.m_PxShape.reset();
	}

	m_PxChunkPath.clear();
}

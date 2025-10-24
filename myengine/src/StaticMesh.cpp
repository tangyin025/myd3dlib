#include "StaticMesh.h"
#include "Actor.h"
#include "myEffect.h"
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
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
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

void StaticMeshStream::Flush(void)
{
	std::map<std::pair<int, int>, bool>::iterator dirty_iter = m_dirty.begin();
	for (; dirty_iter != m_dirty.end(); dirty_iter++)
	{
		if (dirty_iter->second)
		{
			OgreMesh* mesh = GetMesh(dirty_iter->first.first, dirty_iter->first.second);
			_ASSERT(mesh);

			StaticMesh::ChunkMap::iterator chunk_iter = m_mesh->m_Chunks.find(dirty_iter->first);
			_ASSERT(chunk_iter != m_mesh->m_Chunks.end());
			_ASSERT(chunk_iter->second.m_OctAabb);

			std::string path = StaticMeshChunk::MakeChunkPath(m_mesh->m_ChunkPath, dirty_iter->first.first, dirty_iter->first.second);
			std::basic_string<TCHAR> FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
			if (mesh->GetNumFaces() <= 0)
			{
				//OgreMeshPtr dummy = chunk_iter->second.m_Mesh;
				//_ASSERT(dummy.get() == mesh);
				if (chunk_iter->second.is_linked())
				{
					StaticMesh::ChunkSet::iterator viewed_chunk_iter = m_mesh->m_ViewedChunks.iterator_to(chunk_iter->second);
					_ASSERT(viewed_chunk_iter != m_mesh->m_ViewedChunks.end());
					viewed_chunk_iter->ReleaseResource();
					m_mesh->m_ViewedChunks.erase(viewed_chunk_iter);
				}

				BOOST_VERIFY(!my::ResourceMgr::getSingleton().CheckPath(path.c_str()) || 0 == _tunlink(FullPath.c_str()));
				m_mesh->RemoveEntity(&chunk_iter->second);
				m_meshes.erase(chunk_iter->first);
				m_mesh->m_Chunks.erase(chunk_iter);

				//_ASSERT(dummy.use_count() == 2);
				dirty_iter->second = false;
				continue;
			}

			my::AABB chunk_box = *chunk_iter->second.m_OctAabb;
			mesh->SaveOgreMesh(FullPath.c_str(), true);
			m_mesh->RemoveEntity(&chunk_iter->second);
			m_mesh->AddEntity(&chunk_iter->second, chunk_box, m_mesh->m_ChunkWidth, 0.1f);
			m_meshes.erase(chunk_iter->first);

			dirty_iter->second = false;
		}
	}
}

OgreMesh * StaticMeshStream::GetMesh(int i, int j)
{
	MeshMap::const_iterator mesh_iter = m_meshes.find(std::make_pair(i, j));
	if (mesh_iter != m_meshes.end())
	{
		return mesh_iter->second.get();
	}

	StaticMesh::ChunkMap::const_iterator chunk_iter = m_mesh->m_Chunks.find(std::make_pair(i, j));
	if (chunk_iter == m_mesh->m_Chunks.end())
	{
		return NULL;
	}

	if (chunk_iter->second.m_Mesh)
	{
		std::pair<MeshMap::iterator, bool> buff_res = m_meshes.insert(std::make_pair(std::make_pair(i, j), chunk_iter->second.m_Mesh));
		_ASSERT(buff_res.second);
		return buff_res.first->second.get();
	}

	std::string path = StaticMeshChunk::MakeChunkPath(m_mesh->m_ChunkPath, i, j);
	IORequestPtr request(new my::MeshIORequest(path.c_str(), INT_MAX));
	//request->LoadResource();
	//request->CreateResource(NULL);
	//m_buffs[std::make_pair(i, j)] = boost::dynamic_pointer_cast<StaticEmitterChunkBuffer>(request->m_res);

	struct Tmp
	{
		static void Set(MeshMap* buffs, int i, int j, my::DeviceResourceBasePtr res)
		{
			(*buffs)[std::make_pair(i, j)] = boost::dynamic_pointer_cast<OgreMesh>(res);
		}
	};
	my::ResourceMgr::getSingleton().LoadIORequestAndWait(path, request, boost::bind(&Tmp::Set, &m_meshes, i, j, boost::placeholders::_1));

	mesh_iter = m_meshes.find(std::make_pair(i, j));
	_ASSERT(mesh_iter != m_meshes.end());
	return mesh_iter->second.get();
}

void StaticMeshStream::SpawnBuffer(const my::Vector3 & Pos, const my::Quaternion & Rot, const my::Vector3 & Scale, my::OgreMesh * other)
{
	int i = floor(Pos.z / m_mesh->m_ChunkWidth), j = floor(Pos.x / m_mesh->m_ChunkWidth);

	OgreMesh * mesh = GetMesh(i, j);
	_ASSERT(mesh);

	Matrix4 World = Matrix4::Compose(Scale, Rot, Pos);
	for (unsigned int subid = 0; subid < other->GetNumAttributes(); subid++)
	{
		mesh->AppendMesh(other, subid, World, Matrix4::identity);
	}

	m_dirty[std::make_pair(i, j)] = true;
}

void StaticMeshStream::Spawn(const my::Vector3 & Pos, const my::Quaternion & Rot, const my::Vector3 & Scale, my::OgreMesh * other)
{
	int i = floor(Pos.z / m_mesh->m_ChunkWidth), j = floor(Pos.x / m_mesh->m_ChunkWidth);

	MeshMap::const_iterator mesh_iter = m_meshes.find(std::make_pair(i, j));
	if (mesh_iter != m_meshes.end())
	{
		SpawnBuffer(Pos, Rot, Scale, other);
		return;
	}

	StaticMesh::ChunkMap::const_iterator chunk_iter = m_mesh->m_Chunks.find(std::make_pair(i, j));
	if (chunk_iter == m_mesh->m_Chunks.end())
	{
		std::pair<StaticMesh::ChunkMap::iterator, bool> chunk_res = m_mesh->m_Chunks.insert(std::make_pair(std::make_pair(i, j), StaticMeshChunk(i, j)));
		_ASSERT(chunk_res.second);

		m_mesh->AddEntity(&chunk_res.first->second, my::AABB(
			(j + 0) * m_mesh->m_ChunkWidth, Pos.y - 0.5f, (i + 0) * m_mesh->m_ChunkWidth,
			(j + 1) * m_mesh->m_ChunkWidth, Pos.y + 0.5f, (i + 1) * m_mesh->m_ChunkWidth), m_mesh->m_ChunkWidth, 0.1f);

		Matrix4 World = Matrix4::Compose(Scale, Rot, Pos);
		OgreMeshPtr mesh(new OgreMesh());
		mesh->CreateMeshFromOther(other, 0, World, Matrix4::identity, 65536, 65536);
		for (unsigned int subid = 1; subid < other->GetNumAttributes(); subid++)
		{
			mesh->AppendMesh(other, subid, World, Matrix4::identity);
		}

		std::pair<MeshMap::iterator, bool> res = m_meshes.insert(std::make_pair(std::make_pair(i, j), mesh));
		_ASSERT(res.second);
		my::ResourceMgr::getSingleton().AddResource(StaticMeshChunk::MakeChunkPath(m_mesh->m_ChunkPath, i, j), res.first->second);

		m_dirty[std::make_pair(i, j)] = true;
		return;
	}

	SpawnBuffer(Pos, Rot, Scale, other);
}

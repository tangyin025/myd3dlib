#include "Terrain.h"
#include "Actor.h"
#include "Material.h"
#include "PhysXContext.h"
#include "RenderPipeline.h"
#include "myDxutApp.h"
#include "myEffect.h"
#include "myResource.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>

using namespace my;

BOOST_CLASS_EXPORT(TerrainChunk)

BOOST_CLASS_EXPORT(Terrain)

TerrainChunk::TerrainChunk(void)
	: m_aabb(my::AABB::Invalid())
	, m_Row(0)
	, m_Col(0)
{
}

TerrainChunk::TerrainChunk(int Row, int Col, int ChunkSize)
	: m_aabb((float)Col * ChunkSize, -1, (float)Row * ChunkSize, (float)Col * ChunkSize + ChunkSize, 1, (float)Row * ChunkSize + ChunkSize)
	, m_Row(Row)
	, m_Col(Col)
{
	m_vb.CreateVertexBuffer((ChunkSize + 1) * (ChunkSize + 1) * Terrain::m_VertexStride, 0, 0, D3DPOOL_MANAGED);
}

TerrainChunk::~TerrainChunk(void)
{
	m_vb.OnDestroyDevice();
}

template<>
void TerrainChunk::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Row);
	ar << BOOST_SERIALIZATION_NVP(m_Col);
	D3DVERTEXBUFFER_DESC desc = const_cast<my::VertexBuffer&>(m_vb).GetDesc();
	ar << boost::serialization::make_nvp("BufferSize", desc.Size);
	void * pVertices = const_cast<my::VertexBuffer&>(m_vb).Lock(0, 0, D3DLOCK_READONLY);
	ar << boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, desc.Size));
	const_cast<my::VertexBuffer&>(m_vb).Unlock();
}

template<>
void TerrainChunk::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Row);
	ar >> BOOST_SERIALIZATION_NVP(m_Col);
	DWORD BufferSize;
	ar >> BOOST_SERIALIZATION_NVP(BufferSize);
	ResourceMgr::getSingleton().EnterDeviceSectionIfNotMainThread();
	m_vb.OnDestroyDevice();
	m_vb.CreateVertexBuffer(BufferSize, 0, 0, D3DPOOL_MANAGED);
	void * pVertices = m_vb.Lock(0, 0, 0);
	ResourceMgr::getSingleton().LeaveDeviceSectionIfNotMainThread();
	ar >> boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, BufferSize));
	ResourceMgr::getSingleton().EnterDeviceSectionIfNotMainThread();
	m_vb.Unlock();
	ResourceMgr::getSingleton().LeaveDeviceSectionIfNotMainThread();
}

void TerrainChunk::UpdateAABB(void)
{
	Terrain * terrain = dynamic_cast<Terrain *>(m_Node->GetTopNode());
	_ASSERT(terrain);
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		m_aabb = AABB::Invalid();
		for (unsigned int i = 0; i < terrain->m_IndexTable.shape()[0]; i++)
		{
			for (unsigned int j = 0; j < terrain->m_IndexTable.shape()[1]; j++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + terrain->m_IndexTable[i][j] * terrain->m_VertexStride;
				m_aabb.unionSelf(terrain->m_VertexElems.GetPosition(pVertex));
			}
		}
		m_vb.Unlock();
	}
}

template <typename T>
void TerrainChunk::UpdateVertices(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, float HeightScale)
{
	Terrain * terrain = dynamic_cast<Terrain *>(m_Node->GetTopNode());
	_ASSERT(terrain);
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int i = 0; i < terrain->m_IndexTable.shape()[0]; i++)
		{
			for (unsigned int j = 0; j < terrain->m_IndexTable.shape()[1]; j++)
			{
				int pos_i = m_Row * terrain->m_ChunkSize + i;
				int pos_j = m_Col * terrain->m_ChunkSize + j;
				const Vector3 Pos = terrain->GetSamplePos<T>(desc, lrc, pos_i, pos_j, HeightScale);
				const Vector3 Dirs[4] = {
					terrain->GetSamplePos<T>(desc, lrc, pos_i - 1, pos_j, HeightScale) - Pos,
					terrain->GetSamplePos<T>(desc, lrc, pos_i, pos_j - 1, HeightScale) - Pos,
					terrain->GetSamplePos<T>(desc, lrc, pos_i + 1, pos_j, HeightScale) - Pos,
					terrain->GetSamplePos<T>(desc, lrc, pos_i, pos_j + 1, HeightScale) - Pos,
				};
				const Vector3 Nors[4] = {
					Dirs[0].cross(Dirs[1]).normalize(),
					Dirs[1].cross(Dirs[2]).normalize(),
					Dirs[2].cross(Dirs[3]).normalize(),
					Dirs[3].cross(Dirs[0]).normalize(),
				};
				const Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();

				unsigned char * pVertex = (unsigned char *)pVertices + terrain->m_IndexTable[i][j] * terrain->m_VertexStride;
				terrain->m_VertexElems.SetPosition(pVertex, Pos);
				terrain->m_VertexElems.SetNormal(pVertex, Normal);
				terrain->m_VertexElems.SetTangent(pVertex, Normal.cross(Vector3(0, 0, 1)));
				terrain->m_VertexElems.SetTexcoord(pVertex, Vector2((float)j / terrain->m_ChunkSize, (float)i / terrain->m_ChunkSize));
			}
		}
		m_vb.Unlock();
	}
}

void TerrainChunk::UpdateColors(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc)
{
	Terrain * terrain = dynamic_cast<Terrain *>(m_Node->GetTopNode());
	_ASSERT(terrain);
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		boost::multi_array_ref<D3DCOLOR, 2> Color((D3DCOLOR *)lrc.pBits, boost::extents[desc.Height][lrc.Pitch / sizeof(D3DCOLOR)]);
		for (unsigned int i = 0; i < terrain->m_IndexTable.shape()[0]; i++)
		{
			for (unsigned int j = 0; j < terrain->m_IndexTable.shape()[1]; j++)
			{
				int pos_i = m_Row * terrain->m_ChunkSize + i;
				int pos_j = m_Col * terrain->m_ChunkSize + j;

				unsigned char * pVertex = (unsigned char *)pVertices + terrain->m_IndexTable[i][j] * terrain->m_VertexStride;
				terrain->m_VertexElems.SetColor(pVertex, Color[Clamp<int>(pos_i, 0, desc.Height - 1)][Clamp<int>(pos_j, 0, desc.Width - 1)]);
			}
		}
		m_vb.Unlock();
	}
}

static unsigned int FillVertexTable(Terrain::IndexTable & verts, int N, int hs, Terrain::IndexTable::element k)
{
    _ASSERT((hs & (hs - 1)) == 0);
    for (int i = 0; i < N - 1; i += hs * 2)
    {
        for (int j = 0; j < N - 1; j += hs * 2)
        {
            verts[i + hs * 0][j + hs * 1] = k++;
            verts[i + hs * 1][j + hs * 0] = k++;
            verts[i + hs * 1][j + hs * 1] = k++;
            if (j + hs * 2 >= N - 1)
            {
                verts[i + hs * 1][j + hs * 2] = k++;
            }
            if (i + hs * 2 >= N - 1)
            {
                verts[i + hs * 2][j + hs * 1] = k++;
            }
        }
    }

    if (hs / 2 > 0)
    {
        k = FillVertexTable(verts, N, hs / 2, k);
    }
    return k;
}

static unsigned int FillVertexTable(Terrain::IndexTable & verts, int N)
{
    _ASSERT(((N - 1) & (N - 2)) == 0);
	Terrain::IndexTable::element k = 0;
    verts[0][0] = k++;
    verts[0][N - 1] = k++;
    verts[N - 1][0] = k++;
    verts[N - 1][N - 1] = k++;
    return FillVertexTable(verts, N, (N - 1) / 2, k);
}

Terrain::Terrain(void)
	: Component(ComponentTypeTerrain)
	, m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(8)
	, m_HeightScale(1)
	, handle_World(NULL)
{
	CreateElements();
}

Terrain::Terrain(int RowChunks, int ColChunks, int ChunkSize, float HeightScale)
	: Component(ComponentTypeTerrain)
	, OctRoot(my::AABB(0, (float)ChunkSize * my::Max(RowChunks, ColChunks)))
	, m_RowChunks(RowChunks)
	, m_ColChunks(ColChunks)
	, m_ChunkSize(ChunkSize)
	, m_IndexTable(boost::extents[ChunkSize + 1][ChunkSize + 1])
	, m_HeightScale(HeightScale)
	, m_Chunks(boost::extents[RowChunks][ColChunks])
	, handle_World(NULL)
{
	CreateElements();
	FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	my::Texture2D HeightMap;
	HeightMap.CreateTexture(1, 1, 1, 0, D3DFMT_L8);
	D3DSURFACE_DESC desc = HeightMap.GetLevelDesc(0);
	D3DLOCKED_RECT lrc = HeightMap.LockRect(NULL, 0, 0);
	memset(lrc.pBits, 0, lrc.Pitch * 1 * sizeof(unsigned char));
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			TerrainChunkPtr chunk(new TerrainChunk(i, j, m_ChunkSize));
			AddActor(chunk, chunk->m_aabb);
			m_Chunks[i][j] = chunk.get();
			chunk->UpdateVertices<unsigned char>(desc, lrc, 1.0f);
		}
	}
	HeightMap.UnlockRect(0);
}

Terrain::~Terrain(void)
{
	m_Decl.Release();
	ClearAllActor();
}

static int _Quad(int v)
{
	if (v > 1)
	{
		return _Quad(v / 2) + 1;
	}
	return 0;
}

unsigned int Terrain::CalculateLod(int i, int j, const my::Vector3 & LocalViewPos)
{
	float DistanceSq = Vector2(
		(j + 0.5f) * m_ChunkSize - LocalViewPos.x,
		(i + 0.5f) * m_ChunkSize - LocalViewPos.z).magnitudeSq();
	int Lod = (int)(logf(sqrt(DistanceSq) / m_Actor->m_LodDist) / logf(m_Actor->m_LodFactor));
	return Clamp(Lod, 0, _Quad(m_ChunkSize));
}

void Terrain::CreateElements(void)
{
	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_VertexElems.InsertColorElement(offset);
	offset += sizeof(D3DCOLOR);
	m_VertexElems.InsertNormalElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTangentElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTexcoordElement(offset);
	offset += sizeof(Vector2);
	_ASSERT(m_VertexStride == offset);
}

template <class T>
unsigned int EdgeNv1(T & setter, int N, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        int cb = c0 + cs * i;
		if (i < N - 1)
		{
			setter.set(k++, r0, cb);
			setter.set(k++, r0 + rs, cb + cs);
			setter.set(k++, r0, cb + cs);

			if (i > 0)
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0 + rs, cb + cs);
			}
		}
		else
		{
			setter.set(k++, r0, cb);
			setter.set(k++, r0 + rs, cb);
			setter.set(k++, r0, cb + cs);
		}
	}
	return k;
};

template <class T>
unsigned int EdgeNvM(T & setter, int N, int M, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    _ASSERT((M & (M - 1)) == 0);
	if (M == 1)
	{
		return EdgeNv1(setter, N, r0, rs, c0, cs);
	}
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        int cb = c0 + cs * i * M;

        setter.set(k++, r0, cb);
        setter.set(k++, r0 + rs, cb + cs * M / 2);
        setter.set(k++, r0, cb + cs * M);

        int j = 0;
        for (; j < M / 2; j++)
        {
            if (i > 0 || j > 0)
            {
                setter.set(k++, r0 + rs, cb + cs * j);
                setter.set(k++, r0 + rs, cb + cs * (j + 1));
                setter.set(k++, r0, cb);
            }
        }

        for (; j < M; j++)
        {
            if (i < N - 1 || j < M - 1)
            {
                setter.set(k++, r0 + rs, cb + cs * j);
                setter.set(k++, r0 + rs, cb + cs * (j + 1));
                setter.set(k++, r0, cb + cs * M);
            }
        }
    }
    return k;
}

template <class T>
unsigned int FillNvM(T & setter, int N, int M, int r0, int rs, int c0, int cs)
{
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            setter.set(k++, r0 + rs * (i + 0), c0 + cs * (j + 0));
            setter.set(k++, r0 + rs * (i + 1), c0 + cs * (j + 0));
            setter.set(k++, r0 + rs * (i + 0), c0 + cs * (j + 1));

            setter.set(k++, r0 + rs * (i + 0), c0 + cs * (j + 1));
            setter.set(k++, r0 + rs * (i + 1), c0 + cs * (j + 0));
            setter.set(k++, r0 + rs * (i + 1), c0 + cs * (j + 1));
        }
    }
    return k;
}

const Terrain::Fragment & Terrain::GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom)
{
	unsigned int id = ((center & 15) << 16) | ((left & 15) << 12) | ((top & 15) << 8) | ((right & 15) << 4) | ((bottom & 15) << 0);
	FragmentMap::iterator frag_iter = m_Fragment.find(id);
	if (frag_iter != m_Fragment.end())
	{
		return frag_iter->second;
	}

    struct Setter
    {
		IndexTable & verts;
		IndexTable::element * buff;
		Setter(IndexTable & _verts, IndexTable::element * _buff)
			: verts(_verts)
			, buff(_buff)
		{
		}
        void set(int k, int i, int j)
        {
			buff[k] = verts[i][j];
        }
    };

    struct SetterTranspose
    {
		IndexTable & verts;
		IndexTable::element * buff;
		SetterTranspose(IndexTable & _verts, IndexTable::element * _buff)
			: verts(_verts)
			, buff(_buff)
		{
		}
        void set(int k, int i, int j)
        {
			buff[k] = verts[j][i];
        }
    };

	Fragment & frag = m_Fragment[id];
	if (left > center || top > center || right > center || bottom > center)
	{
		const int N[] = {
			m_ChunkSize >> center,
			m_ChunkSize >> Max(center, left),
			m_ChunkSize >> Max(center, top),
			m_ChunkSize >> Max(center, right),
			m_ChunkSize >> Max(center, bottom)
		};
		const int M[] = {
			N[0] / N[0],
			N[0] / N[1],
			N[0] / N[2],
			N[0] / N[3],
			N[0] / N[4]
		};
		frag.VertNum = (N[0] + 1) * (N[0] + 1);
		frag.PrimitiveCount = (N[0] - 2) * (N[0] - 2) * 2
			+ N[1] * (M[1] + 1) + N[2] * (M[2] + 1) + N[3] * (M[3] + 1) + N[4] * (M[4] + 1) - 8;
		frag.ib.CreateIndexBuffer(frag.PrimitiveCount * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
		VOID * pIndices = frag.ib.Lock(0, 0, 0);
		IndexTable::element k = 0;
		const int step = 1 << center;
		k += FillNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[0] - 2, N[0] - 2, 0 + step, step, 0 + step, step);
		k += EdgeNvM(SetterTranspose(m_IndexTable, (IndexTable::element *)pIndices + k), N[1], M[1], 0, step, N[0] * step, -step);
		k += EdgeNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[2], M[2], 0, step, 0, step);
		k += EdgeNvM(SetterTranspose(m_IndexTable, (IndexTable::element *)pIndices + k), N[3], M[3], N[0] * step, -step, 0, step);
		k += EdgeNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[4], M[4], N[0] * step, -step, N[0] * step, -step);
		_ASSERT(k == frag.PrimitiveCount * 3);
		frag.ib.Unlock();
	}
	else
	{
		const int N = m_ChunkSize >> center;
		frag.VertNum = (N + 1) * (N + 1);
		frag.PrimitiveCount = N * N * 2;
		frag.ib.CreateIndexBuffer(frag.PrimitiveCount * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
		VOID * pIndices = frag.ib.Lock(0, 0, 0);
		const int step = 1 << center;
		IndexTable::element k = FillNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices), N, N, 0, step, 0, step);
		frag.ib.Unlock();
	}
	return frag;
}

template<>
void Terrain::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar << BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkSize);
	ar << BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar << BOOST_SERIALIZATION_NVP(m_Material);
}

template<>
void Terrain::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar >> BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkSize);
	m_IndexTable.resize(boost::extents[m_ChunkSize + 1][m_ChunkSize + 1]);
	FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	ar >> BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar >> BOOST_SERIALIZATION_NVP(m_Material);
	m_Chunks.resize(boost::extents[m_RowChunks][m_ColChunks]);
	struct Callback : public my::OctNode::QueryActorCallback
	{
		Terrain * terrain;
		Callback(Terrain * _terrain)
			: terrain(_terrain)
		{
		}
		void operator() (my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_actor);
			terrain->m_Chunks[chunk->m_Row][chunk->m_Col] = chunk;
		}
	};
	QueryActorAll(&Callback(this));
}

void Terrain::RequestResource(void)
{
	Component::RequestResource();

	if (!m_Decl)
	{
		IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
		std::vector<D3DVERTEXELEMENT9> elems = m_VertexElems.BuildVertexElementList(0);
		D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
		elems.push_back(ve_end);
		HRESULT hr;
		V(pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl));
	}

	m_Material->RequestResource();
}

void Terrain::ReleaseResource(void)
{
	m_Decl.Release();
	m_Material->ReleaseResource();
	m_Fragment.clear();
	Component::ReleaseResource();
}

void Terrain::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);
}

void Terrain::OnShaderChanged(void)
{
	handle_World = NULL;
	m_Material->ParseShaderParameters();
}

void Terrain::Update(float fElapsedTime)
{

}

my::AABB Terrain::CalculateAABB(void) const
{
	AABB ret = AABB::Invalid();
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			ret.unionSelf(m_Chunks[i][j]->m_aabb);
		}
	}
	return ret;
}

void Terrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_Actor);

	struct Callback : public my::OctNode::QueryActorCallback
	{
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const Vector3 & LocalViewPos;
		Terrain * terrain;
		Callback(RenderPipeline * _pipeline, unsigned int _PassMask, const Vector3 & _LocalViewPos, Terrain * _terrain)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, terrain(_terrain)
		{
		}
		void operator() (my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_actor);
			const unsigned int lod[5] =
			{
				terrain->CalculateLod(chunk->m_Row, chunk->m_Col, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, LocalViewPos)
			};
			if (terrain->m_Material && terrain->m_Material->m_PassMask & PassMask)
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (terrain->m_Material->m_PassMask & PassMask))
					{
						Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, NULL, terrain->m_Material->m_Shader.c_str(), PassID);
						if (shader)
						{
							if (!terrain->handle_World)
							{
								BOOST_VERIFY(terrain->handle_World = shader->GetParameterByName(NULL, "g_World"));
							}

							const Fragment & frag = terrain->GetFragment(lod[0], lod[1], lod[2], lod[3], lod[4]);
							pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, chunk->m_vb.m_ptr, frag.ib.m_ptr, D3DPT_TRIANGLELIST,
								0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, shader, terrain, terrain->m_Material.get(), MAKELONG(chunk->m_Row, chunk->m_Col));
						}
					}
				}
			}
		}
	};

	if (m_Decl)
	{
		// ! do not use m_World for level offset
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		QueryActor(LocalFrustum, &Callback(pipeline, PassMask, LocalViewPos, this));
	}
}

void Terrain::CreateHeightFieldShape(unsigned int filterWord0)
{
	_ASSERT(!m_PxShape);

	_ASSERT(m_Actor);

	if (!m_Actor->m_PxActor)
	{
		return;
	}

	if (m_Actor->m_PxActor->getType() == physx::PxActorType::eRIGID_DYNAMIC
		&& !m_Actor->m_PxActor->isRigidBody()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	std::vector<physx::PxHeightFieldSample> Samples((m_RowChunks * m_ChunkSize + 1) * (m_ColChunks * m_ChunkSize + 1));
	for (int raw = 0; raw < m_RowChunks; raw++)
	{
		for (int col = 0; col < m_ColChunks; col++)
		{
			TerrainChunk * chunk = m_Chunks[raw][col];
			VOID * pVertices = chunk->m_vb.Lock(0, 0, 0);
			for (int i = 0; i < ((raw < m_RowChunks - 1) ? m_ChunkSize : m_ChunkSize + 1); i++)
			{
				for (int j = 0; j < ((col < m_ColChunks - 1) ? m_ChunkSize : m_ChunkSize + 1); j++)
				{
					//// ! Reverse physx height field row, column
					unsigned char * pVertex = (unsigned char *)pVertices + m_IndexTable[i][j] * m_VertexStride;
					int sample_i = (col * m_ChunkSize + j) * (m_RowChunks * m_ChunkSize + 1) + raw * m_ChunkSize + i;
					Samples[sample_i].height = (signed short)(m_VertexElems.GetPosition(pVertex).y / m_HeightScale);
					Samples[sample_i].materialIndex0 = physx::PxBitAndByte(0, false);
					Samples[sample_i].materialIndex1 = physx::PxBitAndByte(0, false);
				}
			}
			chunk->m_vb.Unlock();
		}
	}

	physx::PxHeightFieldDesc hfDesc;
	hfDesc.nbRows             = m_ColChunks * m_ChunkSize + 1;
	hfDesc.nbColumns          = m_RowChunks * m_ChunkSize + 1;
	hfDesc.format             = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.samples.data       = &Samples[0];
	hfDesc.samples.stride     = sizeof(Samples[0]);
	m_PxHeightField.reset(PhysXContext::getSingleton().m_sdk->createHeightField(hfDesc), PhysXDeleter<physx::PxHeightField>());

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysXDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxHeightFieldGeometry(m_PxHeightField.get(), physx::PxMeshGeometryFlags(), m_HeightScale * m_Actor->m_Scale.y, m_Actor->m_Scale.x, m_Actor->m_Scale.z),
		*m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysXDeleter<physx::PxShape>());

	m_PxShape->setQueryFilterData(physx::PxFilterData(filterWord0, 0, 0, 0));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
}

void Terrain::ClearShape(void)
{
	Component::ClearShape();

	m_PxHeightField.reset();
}

void Terrain::UpdateHeightMap(my::Texture2D * HeightMap, float HeightScale)
{
	D3DSURFACE_DESC desc = HeightMap->GetLevelDesc(0);
	D3DLOCKED_RECT lrc = HeightMap->LockRect(NULL, D3DLOCK_READONLY, 0);
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			switch (desc.Format)
			{
			case D3DFMT_A8:
			case D3DFMT_L8:
				m_Chunks[i][j]->UpdateVertices<unsigned char>(desc, lrc, HeightScale);
				break;
			case D3DFMT_L16:
				m_Chunks[i][j]->UpdateVertices<unsigned short>(desc, lrc, HeightScale);
				break;
			}
			m_Chunks[i][j]->UpdateAABB();
			TerrainChunkPtr chunk = boost::dynamic_pointer_cast<TerrainChunk>(m_Chunks[i][j]->shared_from_this());
			RemoveActor(chunk);
			AddActor(chunk, chunk->m_aabb);
		}
	}
	HeightMap->UnlockRect(0);
}

void Terrain::UpdateSplatmap(my::Texture2D * ColorMap)
{
	D3DSURFACE_DESC desc = ColorMap->GetLevelDesc(0);
	D3DLOCKED_RECT lrc = ColorMap->LockRect(NULL, D3DLOCK_READONLY, 0);
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			switch (desc.Format)
			{
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
				m_Chunks[i][j]->UpdateColors(desc, lrc);
				break;
			}
		}
	}
	ColorMap->UnlockRect(0);
}

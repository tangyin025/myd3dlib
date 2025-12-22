// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "Terrain.h"
#include "Actor.h"
#include "Material.h"
#include "PhysxContext.h"
#include "RenderPipeline.h"
#include "myDxutApp.h"
#include "myEffect.h"
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
#include <boost/iterator/transform_iterator.hpp>
#include <boost/scope_exit.hpp>

using namespace my;
//
//BOOST_CLASS_EXPORT(TerrainChunk)

BOOST_CLASS_EXPORT(Terrain)

class TerrainChunkIORequest : public my::IORequest
{
protected:
	std::string m_path;

	int m_ColChunks;

	int m_Row;

	int m_Col;

	int m_ChunkSize;

	int m_VertexStride;

public:
	TerrainChunkIORequest(const char* path, int ColChunks, int Row, int Col, int ChunkSize, int VertexStride, int Priority)
		: IORequest(Priority)
		, m_path(path)
		, m_ColChunks(ColChunks)
		, m_Row(Row)
		, m_Col(Col)
		, m_ChunkSize(ChunkSize)
		, m_VertexStride(VertexStride)
	{
		m_res.reset(new VertexBuffer());
	}

	virtual void TerrainChunkIORequest::LoadResource(void)
	{
		if (ResourceMgr::getSingleton().CheckPath(m_path.c_str()))
		{
			IStreamPtr istr = ResourceMgr::getSingleton().OpenIStream(m_path.c_str());
			int BufferSize = (m_ChunkSize + 1) * (m_ChunkSize + 1) * m_VertexStride;
			VertexBufferPtr vb = boost::dynamic_pointer_cast<VertexBuffer>(m_res);
			D3DContext::getSingleton().m_d3dDeviceSec.Enter();
			vb->CreateVertexBuffer(BufferSize, 0, 0, D3DPOOL_MANAGED);
			void* buff = vb->Lock();
			D3DContext::getSingleton().m_d3dDeviceSec.Leave();
			BOOST_VERIFY(istr->read(buff, BufferSize) == BufferSize);
			D3DContext::getSingleton().m_d3dDeviceSec.Enter();
			vb->Unlock();
			D3DContext::getSingleton().m_d3dDeviceSec.Leave();
		}
	}

	virtual void TerrainChunkIORequest::CreateResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if (!boost::dynamic_pointer_cast<VertexBuffer>(m_res)->m_ptr)
		{
			m_res.reset();
			THROW_CUSEXCEPTION(str_printf("failed open %s", m_path.c_str()));
		}
	}
};

TerrainChunk::~TerrainChunk(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

void TerrainChunk::RequestResource(void)
{
	m_Requested = true;

	Terrain* terrain = dynamic_cast<Terrain*>(m_Node->GetTopNode());

	if (!terrain->m_ChunkPath.empty())
	{
		_ASSERT(!m_Vb);

		std::string path = TerrainChunk::MakeChunkPath(terrain->m_ChunkPath, m_Row, m_Col);
		IORequestPtr request(new TerrainChunkIORequest(path.c_str(), terrain->m_ColChunks, m_Row, m_Col, terrain->m_ChunkSize, terrain->m_VertexStride,
			m_Lod[0] <= 0 ? Component::ResPriorityLod0 : m_Lod[0] <= 1 ? Component::ResPriorityLod1 : Component::ResPriorityLod2));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(
			path, request, boost::bind(&TerrainChunk::OnVertexBufferReady, this, boost::placeholders::_1));
	}
}

void TerrainChunk::ReleaseResource(void)
{
	Terrain* terrain = dynamic_cast<Terrain*>(m_Node->GetTopNode());

	if (!terrain->m_ChunkPath.empty())
	{
		std::string path = TerrainChunk::MakeChunkPath(terrain->m_ChunkPath, m_Row, m_Col);
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(
			path, boost::bind(&TerrainChunk::OnVertexBufferReady, this, boost::placeholders::_1));

		m_Vb.reset();
	}

	m_Requested = false;
}

std::string TerrainChunk::MakeChunkPath(const std::string & ChunkPath, int Row, int Col)
{
	return str_printf("%s_%d_%d", ChunkPath.c_str(), Row, Col);
}

void TerrainChunk::OnVertexBufferReady(my::DeviceResourceBasePtr res)
{
	m_Vb = boost::dynamic_pointer_cast<VertexBuffer>(res);
}

static unsigned int _FillVertexTable(Terrain::IndexTable & verts, int N, int hs, Terrain::IndexTable::element k)
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
        k = _FillVertexTable(verts, N, hs / 2, k);
    }
    return k;
}

static unsigned int _FillVertexTable(Terrain::IndexTable & verts, int N)
{
    _ASSERT(((N - 1) & (N - 2)) == 0);
	Terrain::IndexTable::element k = 0;
    verts[0][0] = k++;
    verts[0][N - 1] = k++;
    verts[N - 1][0] = k++;
    verts[N - 1][N - 1] = k++;
	if (N > 2)
	{
		k = _FillVertexTable(verts, N, (N - 1) / 2, k);
	}
	return k;
}

const float Terrain::MinBlock = 1.0f;

const float Terrain::Threshold = 0.1f;

static int _Quad(int v, int min_v)
{
	if (v > min_v)
	{
		return _Quad(v / 2, min_v) + 1;
	}
	return 0;
}

Terrain::Terrain(void)
	: m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(8)
	, m_MinChunkLodSize(2)
	, m_ChunkLodScale(1.0f)
	, handle_World(NULL)
	, handle_TerrainSize(NULL)
{
	CreateElements();
}

Terrain::Terrain(const char * Name, int RowChunks, int ColChunks, int ChunkSize, int MinChunkLodSize)
	: Component(Name)
	, OctRoot(0, -1024.0f, 0, (float)ChunkSize * ColChunks, 1024.0f, (float)ChunkSize * RowChunks)
	, m_RowChunks(RowChunks)
	, m_ColChunks(ColChunks)
	, m_ChunkSize(ChunkSize)
	, m_MinChunkLodSize(Min(ChunkSize, MinChunkLodSize))
	, m_ChunkLodScale(1.0f)
	, m_IndexTable(boost::extents[ChunkSize + 1][ChunkSize + 1])
	, m_Chunks(boost::extents[RowChunks][ColChunks])
	, handle_World(NULL)
	, handle_TerrainSize(NULL)
{
	CreateElements();
	_FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	m_rootVb.CreateVertexBuffer((m_RowChunks * m_MinChunkLodSize + 1) * (m_ColChunks * m_MinChunkLodSize + 1) * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
	VOID * pVertices = m_rootVb.Lock(0, 0, 0);
	for (int i = 0; i < (int)m_RowChunks * m_MinChunkLodSize + 1; i++)
	{
		for (int j = 0; j < (int)m_ColChunks * m_MinChunkLodSize + 1; j++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + (i * (m_ColChunks * m_MinChunkLodSize + 1) + j) * m_VertexStride;
			m_VertexElems.SetPosition(pVertex, Vector3((float)j * m_ChunkSize / m_MinChunkLodSize, 0, (float)i * m_ChunkSize / m_MinChunkLodSize), 0);
			m_VertexElems.SetNormal(pVertex, Vector3(0, 1, 0), 0);
			m_VertexElems.SetColor(pVertex, D3DCOLOR_ARGB(255, 0, 0, 0), 0);
		}
	}
	m_rootVb.Unlock();
	m_rootIb.CreateIndexBuffer(m_RowChunks * m_MinChunkLodSize * m_ColChunks * m_MinChunkLodSize * 2 * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			m_Chunks[i][j].m_Row = i;
			m_Chunks[i][j].m_Col = j;
			std::fill_n(m_Chunks[i][j].m_Lod, _countof(m_Chunks[i][j].m_Lod), _Quad(m_ChunkSize, m_MinChunkLodSize));
			AABB aabb(j * m_ChunkSize, -1, i * m_ChunkSize, (j + 1) * m_ChunkSize, 1, (i + 1) * m_ChunkSize);
			AddEntity(&m_Chunks[i][j], aabb, MinBlock, Threshold);
		}
	}
}

Terrain::~Terrain(void)
{
	m_Decl.Release();
	m_rootVb.OnDestroyDevice();
	m_rootIb.OnDestroyDevice();
	ClearAllEntity();
}

int Terrain::CalculateLod(int i, int j, const my::Vector3 & LocalViewPos) const
{
	return Min(_Quad(m_ChunkSize, m_MinChunkLodSize), m_Actor->CalculateLod(
		my::Vector3((j + 0.5f) * m_ChunkSize - LocalViewPos.x, 0, (i + 0.5f) * m_ChunkSize - LocalViewPos.z).magnitude2D() / m_ChunkLodScale));
}

void Terrain::CreateElements(void)
{
	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_VertexElems.InsertNormalElement(offset, 0);
	offset += sizeof(Vector3);
	m_VertexElems.InsertColorElement(offset, 0);
	offset += sizeof(D3DCOLOR);
	_ASSERT(m_VertexStride == offset);
}

template <class T>
unsigned int _EdgeNv1(T & setter, int N, int M0, int M2, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        int cb = c0 + cs * i;

		if (i <= 0)
		{
			if (rs * cs >= 0 && M0 <= 1)
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0, cb + cs);
			}
			else
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb + cs);
				setter.set(k++, r0, cb + cs);
			}
		}
		else if (i >= N - 1)
		{
			if (rs * cs < 0 && M2 <= 1)
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0 + rs, cb + cs);
			}
			else
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0, cb + cs);
			}
		}
		else
		{
			if (rs * cs >= 0)
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0, cb + cs);

				setter.set(k++, r0, cb + cs);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0 + rs, cb + cs);
			}
			else
			{
				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb + cs);
				setter.set(k++, r0, cb + cs);

				setter.set(k++, r0, cb);
				setter.set(k++, r0 + rs, cb);
				setter.set(k++, r0 + rs, cb + cs);
			}
		}
	}
	return k;
};

template <class T>
unsigned int _EdgeNvM(T & setter, int N, int M0, int M1, int M2, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    _ASSERT((M1 & (M1 - 1)) == 0);
	if (M1 == 1)
	{
		return _EdgeNv1(setter, N, M0, M2, r0, rs, c0, cs);
	}
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        int cb = c0 + cs * i * M1;

        setter.set(k++, r0, cb);
        setter.set(k++, r0 + rs, cb + cs * M1 / 2);
        setter.set(k++, r0, cb + cs * M1);

        int j = 0;
        for (; j < M1 / 2; j++)
        {
            if (i > 0 || j > 0)
            {
                setter.set(k++, r0 + rs, cb + cs * j);
                setter.set(k++, r0 + rs, cb + cs * (j + 1));
                setter.set(k++, r0, cb);
            }
        }

        for (; j < M1; j++)
        {
            if (i < N - 1 || j < M1 - 1)
            {
                setter.set(k++, r0 + rs, cb + cs * j);
                setter.set(k++, r0 + rs, cb + cs * (j + 1));
                setter.set(k++, r0, cb + cs * M1);
            }
        }
    }
    return k;
}

template <class T>
unsigned int _FillNvM(T & setter, int N, int M, int r0, int rs, int c0, int cs)
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
		k += _FillNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[0] - 2, N[0] - 2, 0 + step, step, 0 + step, step);
		k += _EdgeNvM(SetterTranspose(m_IndexTable, (IndexTable::element *)pIndices + k), N[1], M[4], M[1], M[2], 0, step, N[0] * step, -step);
		k += _EdgeNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[2], M[1], M[2], M[3], 0, step, 0, step);
		k += _EdgeNvM(SetterTranspose(m_IndexTable, (IndexTable::element *)pIndices + k), N[3], M[2], M[3], M[4], N[0] * step, -step, 0, step);
		k += _EdgeNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[4], M[3], M[4], M[1], N[0] * step, -step, N[0] * step, -step);
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
		IndexTable::element k = _FillNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices), N, N, 0, step, 0, step);
		frag.ib.Unlock();
	}
	return frag;
}

template<class Archive>
void Terrain::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar << BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkSize);
	ar << BOOST_SERIALIZATION_NVP(m_MinChunkLodSize);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkPath);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
	D3DVERTEXBUFFER_DESC desc = const_cast<my::VertexBuffer&>(m_rootVb).GetDesc();
	ar << boost::serialization::make_nvp("BufferSize", desc.Size);
	void * pVertices = const_cast<my::VertexBuffer&>(m_rootVb).Lock(0, 0, D3DLOCK_READONLY);
	ar << boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, desc.Size));
	const_cast<my::VertexBuffer&>(m_rootVb).Unlock();
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			ar << boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d", i, j).c_str(), m_Chunks[i][j]);
			ar << boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d_aabb", i, j).c_str(), *m_Chunks[i][j].m_OctAabb);
		}
	}
	switch (m_PxGeometryType)
	{
	case physx::PxGeometryType::eHEIGHTFIELD:
	{
		_ASSERT(m_PxShape && m_PxGeometryType == m_PxShape->getGeometryType());
		ar << BOOST_SERIALIZATION_NVP(m_PxHeightFieldPath);
		_ASSERT(m_Actor);
		ar << boost::serialization::make_nvp("ActorScale", m_Actor->m_Scale);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		unsigned int ShapeFlags = GetShapeFlags();
		ar << BOOST_SERIALIZATION_NVP(ShapeFlags);
		break;
	}
	}
}

template<class Archive>
void Terrain::load(Archive & ar, const unsigned int version)
{
	ClearAllEntity();

	ActorSerializationContext* pxar = dynamic_cast<ActorSerializationContext*>(&ar);
	_ASSERT(pxar);

	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar >> BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkSize);
	m_IndexTable.resize(boost::extents[m_ChunkSize + 1][m_ChunkSize + 1]);
	_FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	ar >> BOOST_SERIALIZATION_NVP(m_MinChunkLodSize);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkPath);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
	DWORD BufferSize;
	ar >> BOOST_SERIALIZATION_NVP(BufferSize);
	D3DContext::getSingleton().m_d3dDeviceSec.Enter(); // ! unpaired lock/unlock will break the main thread m_d3dDevice->Present
	m_rootVb.OnDestroyDevice();
	m_rootVb.CreateVertexBuffer(BufferSize, 0, 0, D3DPOOL_MANAGED);
	void * pVertices = m_rootVb.Lock(0, 0, 0);
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();
	ar >> boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, BufferSize));
	D3DContext::getSingleton().m_d3dDeviceSec.Enter();
	m_rootVb.Unlock();
	m_rootIb.OnDestroyDevice();
	m_rootIb.CreateIndexBuffer(m_RowChunks * m_MinChunkLodSize * m_ColChunks * m_MinChunkLodSize * 2 * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	D3DContext::getSingleton().m_d3dDeviceSec.Leave();
	m_Chunks.resize(boost::extents[m_RowChunks][m_ColChunks]);
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			AABB aabb;
			ar >> boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d", i, j).c_str(), m_Chunks[i][j]);
			ar >> boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d_aabb", i, j).c_str(), aabb);
			std::fill_n(m_Chunks[i][j].m_Lod, _countof(m_Chunks[i][j].m_Lod), _Quad(m_ChunkSize, m_MinChunkLodSize));
			AddEntity(&m_Chunks[i][j], aabb, MinBlock, Threshold);
		}
	}
	switch (m_PxGeometryType)
	{
	case physx::PxGeometryType::eHEIGHTFIELD:
	{
		std::string PxHeightFieldPath;
		ar >> boost::serialization::make_nvp("m_PxHeightFieldPath", PxHeightFieldPath);
		my::Vector3 ActorScale;
		ar >> BOOST_SERIALIZATION_NVP(ActorScale);
		CreateHeightFieldShape(NULL, PxHeightFieldPath.c_str(), ActorScale);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		unsigned int ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		SetShapeFlags(ShapeFlags);
		break;
	}
	}
}

void Terrain::OnResetShader(void)
{
	handle_World = NULL;
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
}

void Terrain::ReleaseResource(void)
{
	Component::ReleaseResource();

	m_Decl.Release();

	m_Fragment.clear();

	ChunkSet::iterator chunk_iter = m_ViewedChunks.begin();
	for (; chunk_iter != m_ViewedChunks.end(); chunk_iter++)
	{
		chunk_iter->ReleaseResource();
	}
	m_ViewedChunks.clear();
}

void Terrain::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	short Row = LOWORD(lparam);

	short Col = HIWORD(lparam);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_TerrainSize, Vector2(m_ColChunks * m_ChunkSize, m_RowChunks * m_ChunkSize));
}

void Terrain::Update(float fElapsedTime)
{

}

my::AABB Terrain::CalculateAABB(void) const
{
	AABB ret = AABB::Invalid();
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			ret.unionSelf(*m_Chunks[i][j].m_OctAabb);
		}
	}
	return ret;
}

void Terrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	_ASSERT(m_Actor);

	struct Callback : public my::OctNode::QueryCallback
	{
		RenderPipeline * pipeline;
		unsigned int PassMask;
		const Vector3 & LocalViewPos;
		Terrain * terrain;
		int LastLod;
		IndexTable::element * pIndices;
		unsigned int RootPrimitiveCount;
		ChunkSet::iterator insert_chunk_iter;
		Callback(RenderPipeline * _pipeline, unsigned int _PassMask, const Vector3 & _LocalViewPos, Terrain * _terrain, IndexTable::element * _pIndices)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, terrain(_terrain)
			, LastLod(_Quad(_terrain->m_ChunkSize, _terrain->m_MinChunkLodSize))
			, pIndices(_pIndices)
			, RootPrimitiveCount(0)
			, insert_chunk_iter(_terrain->m_ViewedChunks.begin())
		{
		}
		virtual bool OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_entity);
			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod[0] = terrain->CalculateLod(chunk->m_Row, chunk->m_Col, LocalViewPos);
			}

			if (chunk->m_Lod[0] >= LastLod)
			{
				for (int i = chunk->m_Row * terrain->m_MinChunkLodSize; i < chunk->m_Row * terrain->m_MinChunkLodSize + terrain->m_MinChunkLodSize; i++)
				{
					for (int j = chunk->m_Col * terrain->m_MinChunkLodSize; j < chunk->m_Col * terrain->m_MinChunkLodSize + terrain->m_MinChunkLodSize; j++)
					{
						pIndices[RootPrimitiveCount * 3 + 0] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 1] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 2] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 3] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 4] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 5] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 1);
						RootPrimitiveCount += 2;
					}
				}
				return true;
			}

			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				if (chunk->is_linked())
				{
					ChunkSet::iterator chunk_iter = terrain->m_ViewedChunks.iterator_to(*chunk);
					if (chunk_iter != insert_chunk_iter)
					{
						terrain->m_ViewedChunks.splice(insert_chunk_iter, terrain->m_ViewedChunks, chunk_iter);
					}
					else
					{
						_ASSERT(insert_chunk_iter != terrain->m_ViewedChunks.end());

						insert_chunk_iter++;
					}
				}
				else
				{
					_ASSERT(!chunk->IsRequested());

					chunk->RequestResource();

					terrain->m_ViewedChunks.insert(insert_chunk_iter, *chunk);
				}
			}

			if (!chunk->m_Vb)
			{
				for (int i = chunk->m_Row * terrain->m_MinChunkLodSize; i < chunk->m_Row * terrain->m_MinChunkLodSize + terrain->m_MinChunkLodSize; i++)
				{
					for (int j = chunk->m_Col * terrain->m_MinChunkLodSize; j < chunk->m_Col * terrain->m_MinChunkLodSize + terrain->m_MinChunkLodSize; j++)
					{
						pIndices[RootPrimitiveCount * 3 + 0] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 1] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 2] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 3] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 4] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 5] = (terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 1);
						RootPrimitiveCount += 2;
					}
				}
				return true;
			}

			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod[1] = terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, LocalViewPos);
				chunk->m_Lod[2] = terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, LocalViewPos);
				chunk->m_Lod[3] = terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, LocalViewPos);
				chunk->m_Lod[4] = terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, LocalViewPos);
			}

			const Fragment & frag = terrain->GetFragment(chunk->m_Lod[0], chunk->m_Lod[1], chunk->m_Lod[2], chunk->m_Lod[3], chunk->m_Lod[4]);
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (terrain->m_Material->m_PassMask & PassMask))
				{
					D3DXMACRO macros[2] = { { "MESH_TYPE", "2" }, { 0 } };
					Effect* shader = pipeline->QueryShader(terrain->m_Material->m_Shader.c_str(), macros, PassID);
					if (shader)
					{
						if (!terrain->handle_World)
						{
							BOOST_VERIFY(terrain->handle_World = shader->GetParameterByName(NULL, "g_World"));

							BOOST_VERIFY(terrain->handle_TerrainSize = shader->GetParameterByName(NULL, "g_TerrainSize"));
						}

						pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, chunk->m_Vb->m_ptr, frag.ib.m_ptr, D3DPT_TRIANGLELIST,
							0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, shader, terrain, terrain->m_Material.get(), MAKELONG(chunk->m_Row, chunk->m_Col));
					}
				}
			}
			return true;
		}
	};

	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		// ! do not use m_World for level offset
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		Callback cb(pipeline, PassMask, LocalViewPos, this, (IndexTable::element *)m_rootIb.Lock(0, 0, 0));
		QueryEntity(LocalFrustum, &cb);
		m_rootIb.Unlock();

		if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
		{
			int LastLod = _Quad(m_ChunkSize, m_MinChunkLodSize);
			const float LocalCullingDist = m_Actor->m_LodDist * powf(m_Actor->m_LodFactor, LastLod) * m_ChunkLodScale;
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

		if (cb.RootPrimitiveCount > 0)
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					D3DXMACRO macros[2] = { { "MESH_TYPE", "2" }, { 0 } };
					Effect* shader = pipeline->QueryShader(m_Material->m_Shader.c_str(), macros, PassID);
					if (shader)
					{
						if (!handle_World)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));

							BOOST_VERIFY(handle_TerrainSize = shader->GetParameterByName(NULL, "g_TerrainSize"));
						}

						pipeline->PushIndexedPrimitive(PassID, m_Decl, m_rootVb.m_ptr, m_rootIb.m_ptr, D3DPT_TRIANGLELIST,
							0, 0, (m_RowChunks * m_MinChunkLodSize + 1) * (m_ColChunks* m_MinChunkLodSize + 1), m_VertexStride, 0, cb.RootPrimitiveCount, shader, this, m_Material.get(), MAKELONG(-1, -1));
					}
				}
			}
		}
	}
}

float Terrain::CalculateHeightScale(void) const
{
	AABB aabb = CalculateAABB();
	_ASSERT(aabb.IsValid());
	return Max(fabs(aabb.m_max.y), fabs(aabb.m_min.y)) / SHRT_MAX;
}

void Terrain::CreateHeightFieldShape(TerrainStream * tstr, const char * HeightFieldPath, const my::Vector3 & ActorScale)
{
	_ASSERT(!m_PxShape);

	float HeightScale = CalculateHeightScale();

	if (tstr)
	{
		boost::multi_array<physx::PxHeightFieldSample, 2> Samples(boost::extents[m_ColChunks * m_ChunkSize + 1][m_RowChunks * m_ChunkSize + 1]);
		for (int i = 0; i < m_RowChunks * m_ChunkSize + 1; i++)
		{
			for (int j = 0; j < m_ColChunks * m_ChunkSize + 1; j++)
			{
				Samples[j][i].height = (short)Clamp<int>((int)roundf(tstr->GetPos(i, j).y / HeightScale), SHRT_MIN, SHRT_MAX);
				Samples[j][i].materialIndex0 = physx::PxBitAndByte(0, false);
				Samples[j][i].materialIndex1 = physx::PxBitAndByte(0, false);
			}
		}

		physx::PxHeightFieldDesc hfDesc;
		hfDesc.nbRows = m_ColChunks * m_ChunkSize + 1;
		hfDesc.nbColumns = m_RowChunks * m_ChunkSize + 1;
		hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
		hfDesc.samples.data = Samples.data();
		hfDesc.samples.stride = sizeof(Samples[0][0]);

		PhysxFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(HeightFieldPath).c_str());
		bool status = PhysxSdk::getSingleton().m_Cooking->cookHeightField(hfDesc, writeBuffer);
		if (!status)
		{
			THROW_CUSEXCEPTION("cookHeightField failed");
		}
	}

	_ASSERT(m_PxHeightFieldPath.empty());

	m_PxHeightFieldPath.assign(HeightFieldPath);

	PhysxInputData readBuffer(my::ResourceMgr::getSingleton().OpenIStream(HeightFieldPath));
	m_PxHeightField.reset(PhysxSdk::getSingleton().m_sdk->createHeightField(readBuffer), PhysxDeleter<physx::PxHeightField>());

	physx::PxMaterial* material = CreatePhysxMaterial(MeshComponent::DefaultCollisionMaterial.x, MeshComponent::DefaultCollisionMaterial.y, MeshComponent::DefaultCollisionMaterial.z);

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxHeightFieldGeometry(m_PxHeightField.get(), physx::PxMeshGeometryFlags(), HeightScale * ActorScale.y, ActorScale.x, ActorScale.z),
		*material, true, /*physx::PxShapeFlag::eVISUALIZATION |*/ physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->userData = this;

	m_PxGeometryType = physx::PxGeometryType::eHEIGHTFIELD;

	if (m_Actor && m_Actor->m_PxActor)
	{
		m_Actor->m_PxActor->attachShape(*m_PxShape);
	}
}

void Terrain::ClearShape(void)
{
	Component::ClearShape();

	m_PxHeightField.reset();

	m_PxHeightFieldPath.clear();
}

my::RayResult Terrain::RayTest(const my::Ray& local_ray, const my::Vector3& LocalViewPos, CPoint& raychunkid)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Ray& ray;
		const my::Vector3& ViewPos;
		Terrain* terrain;
		my::RayResult ret;
		CPoint raychunkid;
		Callback(const my::Ray& _ray, const my::Vector3& _ViewPos, Terrain* _terrain)
			: ray(_ray)
			, ViewPos(_ViewPos)
			, terrain(_terrain)
			, ret(false, FLT_MAX)
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk* chunk = dynamic_cast<TerrainChunk*>(oct_entity);
			my::RayResult result;
			if (!chunk->m_Vb)
			{
				std::vector<unsigned short> ib;
				for (int i = chunk->m_Row * terrain->m_MinChunkLodSize; i < chunk->m_Row * terrain->m_MinChunkLodSize + terrain->m_MinChunkLodSize; i++)
				{
					for (int j = chunk->m_Col * terrain->m_MinChunkLodSize; j < chunk->m_Col * terrain->m_MinChunkLodSize + terrain->m_MinChunkLodSize; j++)
					{
						ib.push_back((terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 0));
						ib.push_back((terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 0));
						ib.push_back((terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 1));
						ib.push_back((terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 0) + (j + 1));
						ib.push_back((terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 0));
						ib.push_back((terrain->m_ColChunks * terrain->m_MinChunkLodSize + 1) * (i + 1) + (j + 1));
					}
				}
				result = my::Mesh::RayTest(
					ray,
					terrain->m_rootVb.Lock(0, 0, D3DLOCK_READONLY),
					(terrain->m_RowChunks + 1) * (terrain->m_ColChunks + 1),
					terrain->m_VertexStride,
					&ib[0],
					true,
					0,
					ib.size() / 3,
					terrain->m_VertexElems);
				terrain->m_rootVb.Unlock();
			}
			else
			{
				const Terrain::Fragment& frag = terrain->GetFragment(
					terrain->CalculateLod(chunk->m_Row, chunk->m_Col, ViewPos),
					terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, ViewPos),
					terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, ViewPos),
					terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, ViewPos),
					terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, ViewPos));
				result = my::Mesh::RayTest(
					ray,
					chunk->m_Vb->Lock(0, 0, D3DLOCK_READONLY),
					frag.VertNum,
					terrain->m_VertexStride,
					const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY),
					false,
					0,
					frag.PrimitiveCount,
					terrain->m_VertexElems);
				chunk->m_Vb->Unlock();
				const_cast<my::IndexBuffer&>(frag.ib).Unlock();
			}
			if (result.first && result.second < ret.second)
			{
				ret = result;
				raychunkid.SetPoint(chunk->m_Row, chunk->m_Col);
			}
			return true;
		}
	};

	Callback cb(local_ray, LocalViewPos, this);
	QueryEntity(local_ray, &cb);
	if (cb.ret.first)
	{
		raychunkid = cb.raychunkid;
	}
	return cb.ret;
}

float Terrain::RayTest2D(float x, float z)
{
	int k, l, m, n, o, p;
	TerrainStream::GetIndices(this, (int)z, (int)x, k, l, m, n, o, p);
	if (m_Chunks[k][l].m_Vb)
	{
		m = Min<int>(m, m_IndexTable.shape()[0] - 2);
		n = Min<int>(n, m_IndexTable.shape()[1] - 2);
		unsigned char* pVertices = (unsigned char*)m_Chunks[k][l].m_Vb->Lock(0, 0, D3DLOCK_READONLY);
		BOOST_SCOPE_EXIT(this_, k, l)
		{
			this_->m_Chunks[k][l].m_Vb->Unlock();
		}
		BOOST_SCOPE_EXIT_END;
		const Vector3& v0 = m_VertexElems.GetPosition(pVertices + m_IndexTable[m + 0][n + 0] * m_VertexStride, 0);
		const Vector3& v1 = m_VertexElems.GetPosition(pVertices + m_IndexTable[m + 1][n + 0] * m_VertexStride, 0);
		const Vector3& v2 = m_VertexElems.GetPosition(pVertices + m_IndexTable[m + 0][n + 1] * m_VertexStride, 0);
		const Vector3& v3 = m_VertexElems.GetPosition(pVertices + m_IndexTable[m + 1][n + 1] * m_VertexStride, 0);
		const Vector2 d0 = Vector2(x, z) - v2.xz();
		const Vector2 d1 = v1.xz() - v2.xz();
		if (d0.kross(d1) < 0)
		{
			RayResult res = IntersectionTests::rayAndHalfSpace(Vector3(x, m_max.y, z), Vector3(0, -1, 0), Plane::FromTriangle(v0, v1, v2));
			if (res.first)
			{
				return m_max.y - res.second;
			}
			return v0.y;
		}
		else
		{
			RayResult res = IntersectionTests::rayAndHalfSpace(Vector3(x, m_max.y, z), Vector3(0, -1, 0), Plane::FromTriangle(v2, v1, v3));
			if (res.first)
			{
				return m_max.y - res.second;
			}
			return v2.y;
		}
	}
	else
	{
		o = Min<int>(o, m_RowChunks * m_MinChunkLodSize - 1);
		p = Min<int>(p, m_ColChunks * m_MinChunkLodSize - 1);
		unsigned char* pVertices = (unsigned char*)m_rootVb.Lock(0, 0, D3DLOCK_READONLY);
		BOOST_SCOPE_EXIT(this_, k, l)
		{
			this_->m_rootVb.Unlock();
		}
		BOOST_SCOPE_EXIT_END;
		const Vector3& v0 = m_VertexElems.GetPosition(pVertices + ((m_ColChunks * m_MinChunkLodSize + 1) * (o + 0) + (p + 0)) * m_VertexStride, 0);
		const Vector3& v1 = m_VertexElems.GetPosition(pVertices + ((m_ColChunks * m_MinChunkLodSize + 1) * (o + 1) + (p + 0)) * m_VertexStride, 0);
		const Vector3& v2 = m_VertexElems.GetPosition(pVertices + ((m_ColChunks * m_MinChunkLodSize + 1) * (o + 0) + (p + 1)) * m_VertexStride, 0);
		const Vector3& v3 = m_VertexElems.GetPosition(pVertices + ((m_ColChunks * m_MinChunkLodSize + 1) * (o + 1) + (p + 1)) * m_VertexStride, 0);
		const Vector2 d0 = Vector2(x, z) - v2.xz();
		const Vector2 d1 = v1.xz() - v2.xz();
		if (d0.kross(d1) < 0)
		{
			RayResult res = IntersectionTests::rayAndHalfSpace(Vector3(x, m_max.y, z), Vector3(0, -1, 0), Plane::FromTriangle(v0, v1, v2));
			if (res.first)
			{
				return m_max.y - res.second;
			}
			return v0.y;
		}
		else
		{
			RayResult res = IntersectionTests::rayAndHalfSpace(Vector3(x, m_max.y, z), Vector3(0, -1, 0), Plane::FromTriangle(v2, v1, v3));
			if (res.first)
			{
				return m_max.y - res.second;
			}
			return v2.y;
		}
	}
}

TerrainStream::TerrainStream(Terrain* terrain)
	: m_terrain(terrain)
	, m_Vbs(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
	, m_NormalDirty(boost::extents[terrain->m_RowChunks * terrain->m_ChunkSize + 1][terrain->m_ColChunks * terrain->m_ChunkSize + 1])
	, m_AabbDirty(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
	, m_VertDirty(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
{
	std::fill_n(m_NormalDirty.origin(), m_NormalDirty.num_elements(), false);
	std::fill_n(m_AabbDirty.origin(), m_AabbDirty.num_elements(), false);
	std::fill_n(m_VertDirty.origin(), m_VertDirty.num_elements(), false);
}

TerrainStream::~TerrainStream(void)
{
	//Flush();

	_ASSERT(m_VertDirty.origin() + m_VertDirty.num_elements() == std::find(m_VertDirty.origin(), m_VertDirty.origin() + m_VertDirty.num_elements(), true));
}

void TerrainStream::Flush(void)
{
	UpdateNormal();

	for (int k = 0; k < m_terrain->m_RowChunks; k++)
	{
		for (int l = 0; l < m_terrain->m_ColChunks; l++)
		{
			if (m_AabbDirty[k][l])
			{
				AABB aabb = AABB::Invalid();
				for (int m = 0; m < (int)m_terrain->m_IndexTable.shape()[0]; m++)
				{
					for (int n = 0; n < (int)m_terrain->m_IndexTable.shape()[1]; n++)
					{
						aabb.unionSelf(GetPos(k * m_terrain->m_ChunkSize + m, l * m_terrain->m_ChunkSize + n));
					}
				}
				if (aabb.m_max.y - aabb.m_min.y < EPSILON_E6)
				{
					aabb.shrinkSelf(0, -1.0f, 0);
				}
				TerrainChunk* chunk = &m_terrain->m_Chunks[k][l];
				m_terrain->RemoveEntity(chunk);
				m_terrain->AddEntity(chunk, aabb, Terrain::MinBlock, Terrain::Threshold);

				m_AabbDirty[k][l] = false;
			}

			if (m_VertDirty[k][l])
			{
				_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

				my::VertexBuffer* vb = GetVB(k, l);
				std::string path = TerrainChunk::MakeChunkPath(m_terrain->m_ChunkPath, k, l);
				std::basic_string<TCHAR> FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
				std::fstream fstr(FullPath, std::ios::in | std::ios::out | std::ios::binary, _SH_DENYRW);
				_ASSERT(fstr.is_open());

				const D3DVERTEXBUFFER_DESC desc = vb->GetDesc();
				_ASSERT(desc.Size == m_terrain->m_IndexTable.shape()[0] * m_terrain->m_IndexTable.shape()[1] * m_terrain->m_VertexStride);
				void* pVertices = vb->Lock(0, 0, D3DLOCK_READONLY);
				fstr.write((char*)pVertices, desc.Size);
				vb->Unlock();

				m_VertDirty[k][l] = false;
			}
		}
	}
}

void TerrainStream::GetIndices(const Terrain* terrain, int i, int j, int& k, int& l, int& m, int& n, int& o, int& p)
{
	if (i < 0)
	{
		k = 0;
		m = 0;
		o = 0;
	}
	else if (i >= terrain->m_RowChunks * terrain->m_ChunkSize)
	{
		k = terrain->m_RowChunks - 1;
		m = terrain->m_ChunkSize;
		o = terrain->m_RowChunks * terrain->m_MinChunkLodSize;
	}
	else
	{
		k = i / terrain->m_ChunkSize;
		m = i % terrain->m_ChunkSize;
		o = i * terrain->m_MinChunkLodSize / terrain->m_ChunkSize;
	}

	if (j < 0)
	{
		l = 0;
		n = 0;
		p = 0;
	}
	else if (j >= terrain->m_ColChunks * terrain->m_ChunkSize)
	{
		l = terrain->m_ColChunks - 1;
		n = terrain->m_ChunkSize;
		p = terrain->m_ColChunks * terrain->m_MinChunkLodSize;
	}
	else
	{
		l = j / terrain->m_ChunkSize;
		n = j % terrain->m_ChunkSize;
		p = j * terrain->m_MinChunkLodSize / terrain->m_ChunkSize;
	}
}

my::VertexBuffer * TerrainStream::GetVB(int k, int l)
{
	if (m_terrain->m_Chunks[k][l].m_Vb)
	{
		return m_terrain->m_Chunks[k][l].m_Vb.get();
	}

	if (!m_Vbs[k][l])
	{
		std::string path = TerrainChunk::MakeChunkPath(m_terrain->m_ChunkPath, k, l);

		if (!my::ResourceMgr::getSingleton().CheckPath(path.c_str()))
		{
			_ASSERT(GetCurrentThreadId() == D3DContext::getSingleton().m_d3dThreadId);

			for (int k = 0; k < m_terrain->m_RowChunks; k++)
			{
				for (int l = 0; l < m_terrain->m_ColChunks; l++)
				{
					std::string path = TerrainChunk::MakeChunkPath(m_terrain->m_ChunkPath, k, l);
					std::basic_string<TCHAR> FullPath = my::ResourceMgr::getSingleton().GetFullPath(path.c_str());
					std::ofstream ofs(FullPath, std::ios::binary, _SH_DENYRW);
					_ASSERT(ofs.is_open());
					std::vector<char> buff(m_terrain->m_IndexTable.shape()[0] * m_terrain->m_IndexTable.shape()[1] * m_terrain->m_VertexStride);
					for (int m = 0; m < (int)m_terrain->m_IndexTable.shape()[0]; m++)
					{
						for (int n = 0; n < (int)m_terrain->m_IndexTable.shape()[1]; n++)
						{
							char* pVertex = &buff[0] + m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride;
							m_terrain->m_VertexElems.SetPosition(pVertex, Vector3((float)l * m_terrain->m_ChunkSize + n, 0, (float)k * m_terrain->m_ChunkSize + m), 0);
							m_terrain->m_VertexElems.SetNormal(pVertex, Vector3(0, 1, 0), 0);
							m_terrain->m_VertexElems.SetColor(pVertex, D3DCOLOR_ARGB(255, 0, 0, 0), 0);
						}
					}
					ofs.write(&buff[0], buff.size());
				}
			}
		}

		// ! BuildTileMeshTask::DoTask需要在多线程环境下读取地形
		IORequestPtr request(new TerrainChunkIORequest(path.c_str(), m_terrain->m_ColChunks, k, l, m_terrain->m_ChunkSize, m_terrain->m_VertexStride, INT_MAX));
		request->LoadResource();
		request->CreateResource(NULL);
		m_Vbs[k][l] = boost::dynamic_pointer_cast<my::VertexBuffer>(request->m_res);

		//struct Tmp
		//{
		//	static void Set(boost::multi_array<my::VertexBufferPtr, 2>* Vbs, int k, int l, my::DeviceResourceBasePtr res)
		//	{
		//		(*Vbs)[k][l] = boost::dynamic_pointer_cast<my::VertexBuffer>(res);
		//	}
		//};
		//my::ResourceMgr::getSingleton().LoadIORequestAndWait(path, request, boost::bind(&Tmp::Set, &m_Vbs, k, l, boost::placeholders::_1));
	}

	return m_Vbs[k][l].get();
}

my::Vector3 TerrainStream::GetPos(int i, int j)
{
	my::Vector3 ret;
	int k, l, m, n, o, p;
	GetIndices(m_terrain, i, j, k, l, m, n, o, p);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
	my::VertexBuffer* vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(ret), D3DLOCK_READONLY);
	ret = *(my::Vector3*)pbuff;
	vb->Unlock();
	return ret;
}

void TerrainStream::SetPos(int i, int j, const my::Vector3& Pos)
{
	int k, l, m, n, o, p;
	GetIndices(m_terrain, i, j, k, l, m, n, o, p);
	SetPos(k, l, m, n, Pos);

	if (m == 0 && k > 0)
	{
		SetPos(k - 1, l, m_terrain->m_ChunkSize, n, Pos);
	}

	if (n == 0 && l > 0)
	{
		SetPos(k, l - 1, m, m_terrain->m_ChunkSize, Pos);
	}

	if (m == 0 && k > 0 && n == 0 && l > 0)
	{
		SetPos(k - 1, l - 1, m_terrain->m_ChunkSize, m_terrain->m_ChunkSize, Pos);
	}

	if (m % (m_terrain->m_ChunkSize / m_terrain->m_MinChunkLodSize) == 0 && n % (m_terrain->m_ChunkSize / m_terrain->m_MinChunkLodSize) == 0)
	{
		m_terrain->m_VertexElems.SetPosition((unsigned char*)m_terrain->m_rootVb.Lock(0, 0, 0) + (o * (m_terrain->m_ColChunks * m_terrain->m_MinChunkLodSize + 1) + p) * m_terrain->m_VertexStride, Pos);
		m_terrain->m_rootVb.Unlock();
	}

	for (int _i = Max(i - 1, 0); _i <= Min(i + 1, (int)(m_NormalDirty.shape()[0] - 1)); _i++)
	{
		for (int _j = Max(j - 1, 0); _j <= Min(j + 1, (int)(m_NormalDirty.shape()[1] - 1)); _j++)
		{
			m_NormalDirty[_i][_j] = true;
		}
	}
}

void TerrainStream::SetPos(int i, int j, float height)
{
	SetPos(i, j, Vector3(GetPos(i, j).xz(), height));
}

void TerrainStream::SetPos(int k, int l, int m, int n, const my::Vector3& Pos)
{
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
	my::VertexBuffer* vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(Pos), 0);
	*(my::Vector3*)pbuff = Pos;
	vb->Unlock();
	m_VertDirty[k][l] = true;
	m_AabbDirty[k][l] = true;
}

D3DCOLOR TerrainStream::GetColor(int i, int j)
{
	D3DCOLOR ret;
	int k, l, m, n, o, p;
	GetIndices(m_terrain, i, j, k, l, m, n, o, p);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Offset;
	my::VertexBuffer* vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(ret), D3DLOCK_READONLY);
	ret = *(D3DCOLOR*)pbuff;
	vb->Unlock();
	return ret;
}

void TerrainStream::SetColor(int i, int j, D3DCOLOR Color)
{
	int k, l, m, n, o, p;
	GetIndices(m_terrain, i, j, k, l, m, n, o, p);
	SetColor(k, l, m, n, Color);

	if (m == 0 && k > 0)
	{
		SetColor(k - 1, l, m_terrain->m_ChunkSize, n, Color);
	}

	if (n == 0 && l > 0)
	{
		SetColor(k, l - 1, m, m_terrain->m_ChunkSize, Color);
	}

	if (m == 0 && k > 0 && n == 0 && l > 0)
	{
		SetColor(k - 1, l - 1, m_terrain->m_ChunkSize, m_terrain->m_ChunkSize, Color);
	}

	if (m % (m_terrain->m_ChunkSize / m_terrain->m_MinChunkLodSize) == 0 && n % (m_terrain->m_ChunkSize / m_terrain->m_MinChunkLodSize) == 0)
	{
		m_terrain->m_VertexElems.SetColor((unsigned char*)m_terrain->m_rootVb.Lock(0, 0, 0) + (o * (m_terrain->m_ColChunks * m_terrain->m_MinChunkLodSize + 1) + p) * m_terrain->m_VertexStride, Color, 0);
		m_terrain->m_rootVb.Unlock();
	}
}

void TerrainStream::SetColor(int k, int l, int m, int n, D3DCOLOR Color)
{
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Offset;
	my::VertexBuffer* vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(Color), 0);
	*(D3DCOLOR*)pbuff = Color;
	vb->Unlock();
	m_VertDirty[k][l] = true;
}

my::Vector3 TerrainStream::GetNormal(int i, int j)
{
	my::Vector3 ret;
	int k, l, m, n, o, p;
	GetIndices(m_terrain, i, j, k, l, m, n, o, p);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_NORMAL][0].Offset;
	my::VertexBuffer* vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(ret), D3DLOCK_READONLY);
	ret = *(my::Vector3*)pbuff;
	vb->Unlock();
	return ret;
}

void TerrainStream::SetNormal(int i, int j, const my::Vector3& Normal)
{
	int k, l, m, n, o, p;
	GetIndices(m_terrain, i, j, k, l, m, n, o, p);
	SetNormal(k, l, m, n, Normal);

	if (m == 0 && k > 0)
	{
		SetNormal(k - 1, l, m_terrain->m_ChunkSize, n, Normal);
	}

	if (n == 0 && l > 0)
	{
		SetNormal(k, l - 1, m, m_terrain->m_ChunkSize, Normal);
	}

	if (m == 0 && k > 0 && n == 0 && l > 0)
	{
		SetNormal(k - 1, l - 1, m_terrain->m_ChunkSize, m_terrain->m_ChunkSize, Normal);
	}

	if (m % (m_terrain->m_ChunkSize / m_terrain->m_MinChunkLodSize) == 0 && n % (m_terrain->m_ChunkSize / m_terrain->m_MinChunkLodSize) == 0)
	{
		m_terrain->m_VertexElems.SetNormal((unsigned char*)m_terrain->m_rootVb.Lock(0, 0, 0) + (o * (m_terrain->m_ColChunks * m_terrain->m_MinChunkLodSize + 1) + p) * m_terrain->m_VertexStride, Normal, 0);
		m_terrain->m_rootVb.Unlock();
	}

	m_NormalDirty[i][j] = false;
}

void TerrainStream::SetNormal(int k, int l, int m, int n, const my::Vector3& Normal)
{
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_NORMAL][0].Offset;
	my::VertexBuffer* vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(Normal), 0);
	*(my::Vector3*)pbuff = Normal;
	vb->Unlock();
	m_VertDirty[k][l] = true;
}

void TerrainStream::UpdateNormal(void)
{
	for (int i = 0; i < m_NormalDirty.shape()[0]; i++)
	{
		for (int j = 0; j < m_NormalDirty.shape()[1]; j++)
		{
			if (m_NormalDirty[i][j])
			{
				const Vector3 pos = GetPos(i, j);
				const Vector3 Dirs[4] = {
					Vector3(j, GetPos(i - 1, j).y, i - 1) - pos,
					Vector3(j - 1, GetPos(i, j - 1).y, i) - pos,
					Vector3(j, GetPos(i + 1, j).y, i + 1) - pos,
					Vector3(j + 1, GetPos(i, j + 1).y, i) - pos
				};
				const Vector3 Nors[4] = {
					Dirs[0].cross(Dirs[1]).normalize(),
					Dirs[1].cross(Dirs[2]).normalize(),
					Dirs[2].cross(Dirs[3]).normalize(),
					Dirs[3].cross(Dirs[0]).normalize()
				};
				const Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();
				SetNormal(i, j, Normal);

				_ASSERT(!m_NormalDirty[i][j]);
			}
		}
	}
}
//
//my::RayResult TerrainStream::RayTest(const my::Ray& local_ray)
//{
//	struct Callback : public my::OctNode::QueryCallback
//	{
//		const my::Ray& ray;
//		TerrainStream& tstr;
//		my::RayResult ret;
//		Callback(const my::Ray& _ray, TerrainStream& _tstr)
//			: ray(_ray)
//			, tstr(_tstr)
//			, ret(false, FLT_MAX)
//		{
//		}
//		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
//		{
//			my::RayResult result;
//			TerrainChunk* chunk = dynamic_cast<TerrainChunk*>(oct_entity);
//			const Terrain::Fragment& frag = tstr.m_terrain->GetFragment(0, 0, 0, 0, 0);
//			my::VertexBuffer* vb = tstr.GetVB(chunk->m_Row, chunk->m_Col);
//			result = Mesh::RayTest(
//				ray,
//				vb->Lock(0, 0, D3DLOCK_READONLY),
//				frag.VertNum,
//				tstr.m_terrain->m_VertexStride,
//				const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY),
//				false,
//				frag.PrimitiveCount,
//				tstr.m_terrain->m_VertexElems);
//			vb->Unlock();
//			const_cast<my::IndexBuffer&>(frag.ib).Unlock();
//			if (result.first && result.second < ret.second)
//			{
//				ret = result;
//			}
//			return true;
//		}
//	};
//
//	Callback cb(local_ray, *this);
//	m_terrain->QueryEntity(local_ray, &cb);
//	return cb.ret;
//}

float TerrainStream::RayTest2D(float x, float z)
{
	const int i = (int)z, j = (int)x;
	const Vector3 v0 = GetPos(i + 0, j + 0);
	const Vector3 v1 = GetPos(i + 1, j + 0);
	const Vector3 v2 = GetPos(i + 0, j + 1);
	const Vector3 v3 = GetPos(i + 1, j + 1);
	const Vector2 d0 = Vector2(x, z) - v2.xz();
	const Vector2 d1 = v1.xz() - v2.xz();
	if (d0.kross(d1) < 0)
	{
		RayResult res = IntersectionTests::rayAndHalfSpace(Vector3(x, m_terrain->m_max.y, z), Vector3(0, -1, 0), Plane::FromTriangle(v0, v1, v2));
		if (res.first)
		{
			return m_terrain->m_max.y - res.second;
		}
		return v0.y;
	}
	else
	{
		RayResult res = IntersectionTests::rayAndHalfSpace(Vector3(x, m_terrain->m_max.y, z), Vector3(0, -1, 0), Plane::FromTriangle(v2, v1, v3));
		if (res.first)
		{
			return m_terrain->m_max.y - res.second;
		}
		return v2.y;
	}
}

#include "Terrain.h"
#include "Actor.h"
#include "Material.h"
#include "PhysxContext.h"
#include "RenderPipeline.h"
#include "RenderPipeline.inl"
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
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>

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
			istr->seek(TerrainStream::CalculateStreamOff(m_ColChunks, m_Row, m_Col, m_ChunkSize, m_VertexStride), SEEK_SET);
			int BufferSize = (m_ChunkSize + 1) * (m_ChunkSize + 1) * m_VertexStride;
			VertexBufferPtr vb = boost::dynamic_pointer_cast<VertexBuffer>(m_res);
			ResourceMgr::getSingleton().EnterDeviceSection();
			vb->CreateVertexBuffer(BufferSize, 0, 0, D3DPOOL_MANAGED);
			void* buff = vb->Lock();
			ResourceMgr::getSingleton().LeaveDeviceSection();
			BOOST_VERIFY(istr->read(buff, BufferSize) == BufferSize);
			ResourceMgr::getSingleton().EnterDeviceSection();
			vb->Unlock();
			ResourceMgr::getSingleton().LeaveDeviceSection();
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

	static std::string TerrainChunkIORequest::BuildKey(const char* path, int Row, int Col)
	{
		return str_printf("%s %d %d", path, Row, Col);
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

		std::string key = TerrainChunkIORequest::BuildKey(terrain->m_ChunkPath.c_str(), m_Row, m_Col);
		IORequestPtr request(new TerrainChunkIORequest(terrain->m_ChunkPath.c_str(), terrain->m_ColChunks, m_Row, m_Col, terrain->m_ChunkSize, terrain->m_VertexStride, 0));
		my::ResourceMgr::getSingleton().LoadIORequestAsync(
			key, request, boost::bind(&TerrainChunk::OnVertexBufferReady, this, boost::placeholders::_1));
	}

	if (m_Material)
	{
		m_Material->RequestResource();
	}
}

void TerrainChunk::ReleaseResource(void)
{
	Terrain* terrain = dynamic_cast<Terrain*>(m_Node->GetTopNode());

	if (!terrain->m_ChunkPath.empty())
	{
		std::string key = TerrainChunkIORequest::BuildKey(terrain->m_ChunkPath.c_str(), m_Row, m_Col);
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(
			key, boost::bind(&TerrainChunk::OnVertexBufferReady, this, boost::placeholders::_1));

		m_Vb.reset();
	}

	if (m_Material)
	{
		m_Material->ReleaseResource();
	}

	m_Requested = false;
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
	, m_MinLodChunkSize(2)
	, handle_World(NULL)
	, handle_TerrainSize(NULL)
{
	CreateElements();
}

Terrain::Terrain(const char * Name, int RowChunks, int ColChunks, int ChunkSize, int MinLodChunkSize)
	: Component(Name)
	, OctRoot(0, -1.0f, 0, (float)ChunkSize * ColChunks, 1.0f, (float)ChunkSize * RowChunks)
	, m_RowChunks(RowChunks)
	, m_ColChunks(ColChunks)
	, m_ChunkSize(ChunkSize)
	, m_MinLodChunkSize(Min(ChunkSize, MinLodChunkSize))
	, m_IndexTable(boost::extents[ChunkSize + 1][ChunkSize + 1])
	, m_Chunks(boost::extents[RowChunks][ColChunks])
	, handle_World(NULL)
	, handle_TerrainSize(NULL)
{
	CreateElements();
	_FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	m_rootVb.CreateVertexBuffer((m_RowChunks * m_MinLodChunkSize + 1) * (m_ColChunks * m_MinLodChunkSize + 1) * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
	VOID * pVertices = m_rootVb.Lock(0, 0, 0);
	for (int i = 0; i < (int)m_RowChunks * m_MinLodChunkSize + 1; i++)
	{
		for (int j = 0; j < (int)m_ColChunks * m_MinLodChunkSize + 1; j++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + (i * (m_ColChunks * m_MinLodChunkSize + 1) + j) * m_VertexStride;
			m_VertexElems.SetPosition(pVertex, Vector3((float)j * m_ChunkSize / m_MinLodChunkSize, 0, (float)i * m_ChunkSize / m_MinLodChunkSize), 0);
			m_VertexElems.SetColor(pVertex, D3DCOLOR_ARGB(255, 0, 0, 0), 0);
			m_VertexElems.SetColor(pVertex, D3DCOLOR_COLORVALUE(0.5f, 1.0f, 0.5f, 0.0f), 1);
		}
	}
	m_rootVb.Unlock();
	m_rootIb.CreateIndexBuffer(m_RowChunks * m_MinLodChunkSize * m_ColChunks * m_MinLodChunkSize * 2 * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			m_Chunks[i][j].m_Row = i;
			m_Chunks[i][j].m_Col = j;
			std::fill_n(m_Chunks[i][j].m_Lod, _countof(m_Chunks[i][j].m_Lod), _Quad(m_ChunkSize, m_MinLodChunkSize));
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
	float DistanceSq = Vector2(
		(j + 0.5f) * m_ChunkSize - LocalViewPos.x,
		(i + 0.5f) * m_ChunkSize - LocalViewPos.z).magnitudeSq();
	int Lod = (int)(logf(sqrt(DistanceSq) / m_Actor->m_LodDist * m_Actor->m_Scale.x) / logf(m_Actor->m_LodFactor));
	return Max(0, Min(Lod, _Quad(m_ChunkSize, m_MinLodChunkSize)));
}

void Terrain::CreateElements(void)
{
	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_VertexElems.InsertColorElement(offset, 0);
	offset += sizeof(D3DCOLOR);
	m_VertexElems.InsertColorElement(offset, 1);
	offset += sizeof(D3DCOLOR);
	_ASSERT(m_VertexStride == offset);
}

template <class T>
unsigned int _EdgeNv1(T & setter, int N, int r0, int rs, int c0, int cs)
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
unsigned int _EdgeNvM(T & setter, int N, int M, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    _ASSERT((M & (M - 1)) == 0);
	if (M == 1)
	{
		return _EdgeNv1(setter, N, r0, rs, c0, cs);
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
		k += _EdgeNvM(SetterTranspose(m_IndexTable, (IndexTable::element *)pIndices + k), N[1], M[1], 0, step, N[0] * step, -step);
		k += _EdgeNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[2], M[2], 0, step, 0, step);
		k += _EdgeNvM(SetterTranspose(m_IndexTable, (IndexTable::element *)pIndices + k), N[3], M[3], N[0] * step, -step, 0, step);
		k += _EdgeNvM(Setter(m_IndexTable, (IndexTable::element *)pIndices + k), N[4], M[4], N[0] * step, -step, N[0] * step, -step);
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
	ar << BOOST_SERIALIZATION_NVP(m_MinLodChunkSize);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkPath);
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
	switch (m_PxShapeGeometryType)
	{
	case physx::PxGeometryType::eHEIGHTFIELD:
	{
		_ASSERT(m_PxShape && m_PxShapeGeometryType == m_PxShape->getGeometryType());
		ar << BOOST_SERIALIZATION_NVP(m_PxHeightFieldPath);
		_ASSERT(m_Actor);
		ar << boost::serialization::make_nvp("ActorScale", m_Actor->m_Scale);
		unsigned int SimulationFilterWord0 = GetSimulationFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		unsigned int QueryFilterWord0 = GetQueryFilterWord0();
		ar << BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags = (physx::PxShapeFlags::InternalType)m_PxShape->getFlags();
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
	ar >> BOOST_SERIALIZATION_NVP(m_MinLodChunkSize);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkPath);
	DWORD BufferSize;
	ar >> BOOST_SERIALIZATION_NVP(BufferSize);
	ResourceMgr::getSingleton().EnterDeviceSection(); // ! unpaired lock/unlock will break the main thread m_d3dDevice->Present
	m_rootVb.OnDestroyDevice();
	m_rootVb.CreateVertexBuffer(BufferSize, 0, 0, D3DPOOL_MANAGED);
	void * pVertices = m_rootVb.Lock(0, 0, 0);
	ResourceMgr::getSingleton().LeaveDeviceSection();
	ar >> boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, BufferSize));
	ResourceMgr::getSingleton().EnterDeviceSection();
	m_rootVb.Unlock();
	m_rootIb.OnDestroyDevice();
	m_rootIb.CreateIndexBuffer(m_RowChunks * m_MinLodChunkSize * m_ColChunks * m_MinLodChunkSize * 2 * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	ResourceMgr::getSingleton().LeaveDeviceSection();
	m_Chunks.resize(boost::extents[m_RowChunks][m_ColChunks]);
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			AABB aabb;
			ar >> boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d", i, j).c_str(), m_Chunks[i][j]);
			ar >> boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d_aabb", i, j).c_str(), aabb);
			std::fill_n(m_Chunks[i][j].m_Lod, _countof(m_Chunks[i][j].m_Lod), _Quad(m_ChunkSize, m_MinLodChunkSize));
			AddEntity(&m_Chunks[i][j], aabb, MinBlock, Threshold);
		}
	}
	switch (m_PxShapeGeometryType)
	{
	case physx::PxGeometryType::eHEIGHTFIELD:
	{
		std::string PxHeightFieldPath;
		ar >> boost::serialization::make_nvp("m_PxHeightFieldPath", PxHeightFieldPath);
		my::Vector3 ActorScale;
		ar >> BOOST_SERIALIZATION_NVP(ActorScale);
		CreateHeightFieldShape(PxHeightFieldPath.c_str(), ActorScale, pxar->m_CollectionObjs);
		unsigned int SimulationFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(SimulationFilterWord0);
		SetSimulationFilterWord0(SimulationFilterWord0);
		unsigned int QueryFilterWord0;
		ar >> BOOST_SERIALIZATION_NVP(QueryFilterWord0);
		SetQueryFilterWord0(QueryFilterWord0);
		physx::PxShapeFlags::InternalType ShapeFlags;
		ar >> BOOST_SERIALIZATION_NVP(ShapeFlags);
		m_PxShape->setFlags(physx::PxShapeFlags(ShapeFlags));
		break;
	}
	}
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

	shader->SetVector(handle_TerrainSize, Vector2(m_RowChunks * m_ChunkSize, m_ColChunks * m_ChunkSize));
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
		IndexTable::element * pIndices;
		unsigned int RootPrimitiveCount;
		ChunkSet::iterator insert_chunk_iter;
		Callback(RenderPipeline * _pipeline, unsigned int _PassMask, const Vector3 & _LocalViewPos, Terrain * _terrain, IndexTable::element * _pIndices)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, terrain(_terrain)
			, pIndices(_pIndices)
			, RootPrimitiveCount(0)
			, insert_chunk_iter(_terrain->m_ViewedChunks.begin())
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_entity);
			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod[0] = terrain->CalculateLod(chunk->m_Row, chunk->m_Col, LocalViewPos);
			}

			int LastLod = _Quad(terrain->m_ChunkSize, terrain->m_MinLodChunkSize);
			if (chunk->m_Lod[0] >= LastLod)
			{
				for (int i = chunk->m_Row * terrain->m_MinLodChunkSize; i < chunk->m_Row * terrain->m_MinLodChunkSize + terrain->m_MinLodChunkSize; i++)
				{
					for (int j = chunk->m_Col * terrain->m_MinLodChunkSize; j < chunk->m_Col * terrain->m_MinLodChunkSize + terrain->m_MinLodChunkSize; j++)
					{
						pIndices[RootPrimitiveCount * 3 + 0] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 1] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 2] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 3] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 4] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 5] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 1);
						RootPrimitiveCount += 2;
					}
				}
				return;
			}

			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				if (chunk->is_linked())
				{
					ChunkSet::iterator chunk_iter = terrain->m_ViewedChunks.iterator_to(*chunk);
					if (chunk_iter != insert_chunk_iter)
					{
						terrain->m_ViewedChunks.erase(chunk_iter);

						terrain->m_ViewedChunks.insert(insert_chunk_iter, *chunk);
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
				for (int i = chunk->m_Row * terrain->m_MinLodChunkSize; i < chunk->m_Row * terrain->m_MinLodChunkSize + terrain->m_MinLodChunkSize; i++)
				{
					for (int j = chunk->m_Col * terrain->m_MinLodChunkSize; j < chunk->m_Col * terrain->m_MinLodChunkSize + terrain->m_MinLodChunkSize; j++)
					{
						pIndices[RootPrimitiveCount * 3 + 0] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 1] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 2] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 3] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 0) + (j + 1);
						pIndices[RootPrimitiveCount * 3 + 4] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 0);
						pIndices[RootPrimitiveCount * 3 + 5] = (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1) * (i + 1) + (j + 1);
						RootPrimitiveCount += 2;
					}
				}
				return;
			}

			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod[1] = terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, LocalViewPos);
				chunk->m_Lod[2] = terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, LocalViewPos);
				chunk->m_Lod[3] = terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, LocalViewPos);
				chunk->m_Lod[4] = terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, LocalViewPos);
			}

			const Fragment & frag = terrain->GetFragment(chunk->m_Lod[0], chunk->m_Lod[1], chunk->m_Lod[2], chunk->m_Lod[3], chunk->m_Lod[4]);
			Material* mtl = chunk->m_Material ? chunk->m_Material.get() : terrain->m_Material.get();
			if (mtl && (mtl->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (mtl->m_PassMask & PassMask))
					{
						Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, NULL, mtl->m_Shader.c_str(), PassID);
						if (shader)
						{
							if (!terrain->handle_World)
							{
								BOOST_VERIFY(terrain->handle_World = shader->GetParameterByName(NULL, "g_World"));

								BOOST_VERIFY(terrain->handle_TerrainSize = shader->GetParameterByName(NULL, "g_TerrainSize"));
							}

							pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, chunk->m_Vb->m_ptr, frag.ib.m_ptr, D3DPT_TRIANGLELIST,
								0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, shader, terrain, mtl, MAKELONG(chunk->m_Row, chunk->m_Col));
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
		Callback cb(pipeline, PassMask, LocalViewPos, this, (IndexTable::element *)m_rootIb.Lock(0, 0, 0));
		QueryEntity(LocalFrustum, &cb);
		m_rootIb.Unlock();

		if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
		{
			int LastLod = _Quad(m_ChunkSize, m_MinLodChunkSize);
			float CullingDistSq = powf(m_Actor->m_LodDist * powf(m_Actor->m_LodFactor, LastLod), 2.0);
			ChunkSet::iterator chunk_iter = cb.insert_chunk_iter;
			for (; chunk_iter != m_ViewedChunks.end(); )
			{
				if ((chunk_iter->m_OctAabb->Center() - LocalViewPos).magnitudeSq() > CullingDistSq)
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
			if (m_Material && (m_Material->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
					{
						Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, NULL, m_Material->m_Shader.c_str(), PassID);
						if (shader)
						{
							if (!handle_World)
							{
								BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));

								BOOST_VERIFY(handle_TerrainSize = shader->GetParameterByName(NULL, "g_TerrainSize"));
							}

							pipeline->PushIndexedPrimitive(PassID, m_Decl, m_rootVb.m_ptr, m_rootIb.m_ptr, D3DPT_TRIANGLELIST,
								0, 0, (m_RowChunks * m_MinLodChunkSize + 1) * (m_ColChunks* m_MinLodChunkSize + 1), m_VertexStride, 0, cb.RootPrimitiveCount, shader, this, m_Material.get(), MAKELONG(-1, -1));
						}
					}
				}
			}
		}
	}
}

void Terrain::CreateHeightFieldShape(const char * HeightFieldPath, const my::Vector3 & ActorScale, CollectionObjMap & collectionObjs)
{
	_ASSERT(!m_PxShape);

	AABB aabb = CalculateAABB();
	if (!aabb.IsValid())
	{
		return;
	}

	float HeightScale = Max(fabs(aabb.m_max.y), fabs(aabb.m_min.y)) / SHRT_MAX;

	physx::PxMaterial* material = CreatePhysxMaterial(0.5f, 0.5f, 0.5f, collectionObjs);

	if (!my::ResourceMgr::getSingleton().CheckPath(HeightFieldPath))
	{
		boost::multi_array<physx::PxHeightFieldSample, 2> Samples(boost::extents[m_ColChunks * m_ChunkSize + 1][m_RowChunks * m_ChunkSize + 1]);
		TerrainStream tstr(this);
		for (int i = 0; i < m_RowChunks * m_ChunkSize + 1; i++)
		{
			for (int j = 0; j < m_ColChunks * m_ChunkSize + 1; j++)
			{
				Samples[j][i].height = (short)Clamp<int>((int)roundf(tstr.GetPos(i, j).y / HeightScale), SHRT_MIN, SHRT_MAX);
				Samples[j][i].materialIndex0 = physx::PxBitAndByte(0, false);
				Samples[j][i].materialIndex1 = physx::PxBitAndByte(0, false);
			}
		}
		tstr.Release();

		physx::PxHeightFieldDesc hfDesc;
		hfDesc.nbRows = m_ColChunks * m_ChunkSize + 1;
		hfDesc.nbColumns = m_RowChunks * m_ChunkSize + 1;
		hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
		hfDesc.samples.data = Samples.data();
		hfDesc.samples.stride = sizeof(Samples[0][0]);

		physx::PxDefaultFileOutputStream writeBuffer(my::ResourceMgr::getSingleton().GetFullPath(HeightFieldPath).c_str());
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

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(physx::PxHeightFieldGeometry(m_PxHeightField.get(), physx::PxMeshGeometryFlags(), HeightScale * ActorScale.y, ActorScale.x, ActorScale.z),
		*material, true, /*physx::PxShapeFlag::eVISUALIZATION |*/ physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_PxShape->userData = this;

	m_PxShapeGeometryType = physx::PxGeometryType::eHEIGHTFIELD;
}

void Terrain::ClearShape(void)
{
	Component::ClearShape();

	m_PxHeightField.reset();

	m_PxHeightFieldPath.clear();
}

my::RayResult Terrain::SimpleRayTest(const my::Ray & local_ray)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Ray& ray;
		Terrain* terrain;
		my::RayResult ret;
		Callback(const my::Ray& _ray, Terrain* _terrain)
			: ray(_ray)
			, terrain(_terrain)
			, ret(false, FLT_MAX)
		{
		}
		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			my::RayResult result;
			TerrainChunk* chunk = dynamic_cast<TerrainChunk*>(oct_entity);
			if (terrain->m_Chunks[chunk->m_Row][chunk->m_Col].m_Vb)
			{
				const Terrain::Fragment& frag = terrain->GetFragment(0, 0, 0, 0, 0);
				result = Mesh::RayTest(
					ray,
					terrain->m_Chunks[chunk->m_Row][chunk->m_Col].m_Vb->Lock(0, 0, D3DLOCK_READONLY),
					frag.VertNum,
					terrain->m_VertexStride,
					const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY),
					false,
					frag.PrimitiveCount,
					terrain->m_VertexElems);
				terrain->m_Chunks[chunk->m_Row][chunk->m_Col].m_Vb->Unlock();
				const_cast<my::IndexBuffer&>(frag.ib).Unlock();
				if (result.first && result.second < ret.second)
				{
					ret = result;
				}
			}
			else
			{
				result = Mesh::RayTest(
					ray,
					terrain->m_rootVb.Lock(0, 0, D3DLOCK_READONLY),
					(terrain->m_RowChunks * terrain->m_MinLodChunkSize + 1) * (terrain->m_ColChunks * terrain->m_MinLodChunkSize + 1),
					terrain->m_VertexStride,
					terrain->m_rootIb.Lock(0, 0, D3DLOCK_READONLY),
					false,
					terrain->m_RowChunks * terrain->m_MinLodChunkSize * terrain->m_ColChunks * terrain->m_MinLodChunkSize * 2,
					terrain->m_VertexElems);
				terrain->m_rootVb.Unlock();
				terrain->m_rootIb.Unlock();
				if (result.first && result.second < ret.second)
				{
					ret = result;
				}
			}
		}
	};

	Callback cb(local_ray, this);
	QueryEntity(local_ray, &cb);
	return cb.ret;
}

TerrainStream::TerrainStream(Terrain* terrain)
	: m_terrain(terrain)
	, m_Vbs(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
	, m_AabbDirty(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
	, m_VertDirty(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
{
}

TerrainStream::~TerrainStream(void)
{
	Release();
}

void TerrainStream::Release(void)
{
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
				my::VertexBufferPtr vb = GetVB(k, l);
				std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(m_terrain->m_ChunkPath.c_str());
				std::fstream fstr(FullPath, std::ios::in | std::ios::out | std::ios::binary);
				_ASSERT(fstr.is_open());
				int stream_off = CalculateStreamOff(m_terrain->m_ColChunks, k, l, m_terrain->m_ChunkSize, m_terrain->m_VertexStride);
				fstr.seekp(stream_off, std::ios::beg);
				const D3DVERTEXBUFFER_DESC desc = vb->GetDesc();
				_ASSERT(desc.Size == m_terrain->m_IndexTable.shape()[0] * m_terrain->m_IndexTable.shape()[1] * m_terrain->m_VertexStride);
				void* pVertices = vb->Lock(0, 0, D3DLOCK_READONLY);
				fstr.write((char*)pVertices, desc.Size);
				vb->Unlock();

				m_VertDirty[k][l] = false;
				m_Vbs[k][l].reset();
			}
		}
	}
}

void TerrainStream::GetIndices(int i, int j, int& k, int& l, int& m, int& n, int& o, int& p) const
{
	if (i < 0)
	{
		k = 0;
		m = 0;
		o = 0;
	}
	else if (i >= m_terrain->m_RowChunks * m_terrain->m_ChunkSize)
	{
		k = m_terrain->m_RowChunks - 1;
		m = m_terrain->m_ChunkSize;
		o = m_terrain->m_RowChunks * m_terrain->m_MinLodChunkSize;
	}
	else
	{
		k = i / m_terrain->m_ChunkSize;
		m = i % m_terrain->m_ChunkSize;
		o = i * m_terrain->m_MinLodChunkSize / m_terrain->m_ChunkSize;
	}

	if (j < 0)
	{
		l = 0;
		n = 0;
		p = 0;
	}
	else if (j >= m_terrain->m_ColChunks * m_terrain->m_ChunkSize)
	{
		l = m_terrain->m_ColChunks - 1;
		n = m_terrain->m_ChunkSize;
		p = m_terrain->m_ColChunks * m_terrain->m_MinLodChunkSize;
	}
	else
	{
		l = j / m_terrain->m_ChunkSize;
		n = j % m_terrain->m_ChunkSize;
		p = j * m_terrain->m_MinLodChunkSize / m_terrain->m_ChunkSize;
	}
}

my::VertexBufferPtr TerrainStream::GetVB(int k, int l)
{
	if (m_terrain->m_Chunks[k][l].m_Vb)
	{
		return m_terrain->m_Chunks[k][l].m_Vb;
	}

	if (!m_Vbs[k][l])
	{
		if (!my::ResourceMgr::getSingleton().CheckPath(m_terrain->m_ChunkPath.c_str()))
		{
			std::string FullPath = my::ResourceMgr::getSingleton().GetFullPath(m_terrain->m_ChunkPath.c_str());
			std::ofstream ofs(FullPath, std::ios::binary);
			_ASSERT(ofs.is_open());
			for (int k = 0; k < m_terrain->m_RowChunks; k++)
			{
				for (int l = 0; l < m_terrain->m_ColChunks; l++)
				{
					std::vector<char> buff(m_terrain->m_IndexTable.shape()[0] * m_terrain->m_IndexTable.shape()[1] * m_terrain->m_VertexStride);
					for (int m = 0; m < (int)m_terrain->m_IndexTable.shape()[0]; m++)
					{
						for (int n = 0; n < (int)m_terrain->m_IndexTable.shape()[1]; n++)
						{
							char* pVertex = &buff[0] + m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride;
							m_terrain->m_VertexElems.SetPosition(pVertex, Vector3((float)l * m_terrain->m_ChunkSize + n, 0, (float)k * m_terrain->m_ChunkSize + m), 0);
							m_terrain->m_VertexElems.SetColor(pVertex, D3DCOLOR_ARGB(255, 0, 0, 0), 0);
							m_terrain->m_VertexElems.SetColor(pVertex, D3DCOLOR_COLORVALUE(0.5f, 1.0f, 0.5f, 0.0f), 1);
						}
					}
					ofs.write(&buff[0], buff.size());
				}
			}
			ofs.close();
			_ASSERT(my::ResourceMgr::getSingleton().CheckPath(m_terrain->m_ChunkPath.c_str()));
		}

		std::string key = TerrainChunkIORequest::BuildKey(m_terrain->m_ChunkPath.c_str(), k, l);
		IORequestPtr request(new TerrainChunkIORequest(m_terrain->m_ChunkPath.c_str(), m_terrain->m_ColChunks, k, l, m_terrain->m_ChunkSize, m_terrain->m_VertexStride, INT_MAX));
		my::ResourceMgr::getSingleton().LoadIORequestAndWait(key, request, boost::bind(&TerrainStream::SetVB, this, k, l, boost::placeholders::_1));
	}

	return m_Vbs[k][l];
}

void TerrainStream::SetVB(int k, int l, my::DeviceResourceBasePtr res)
{
	_ASSERT(!m_Vbs[k][l]);
	m_Vbs[k][l] = boost::dynamic_pointer_cast<my::VertexBuffer>(res);
	_ASSERT(m_Vbs[k][l]->m_ptr);
}

int TerrainStream::CalculateStreamOff(int ColChunks, int Row, int Col, int ChunkSize, int VertexStride)
{
	return (ColChunks * Row + Col) * (ChunkSize + 1) * (ChunkSize + 1) * VertexStride;
}

my::Vector3 TerrainStream::GetPos(int i, int j)
{
	my::Vector3 ret;
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
	my::VertexBufferPtr vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(ret), D3DLOCK_READONLY);
	ret = *(my::Vector3*)pbuff;
	vb->Unlock();
	return ret;
}

void TerrainStream::SetPos(const my::Vector3& Pos, int i, int j, bool UpdateNormal)
{
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	SetPos(Pos, k, l, m, n);

	if (m == 0 && k > 0)
	{
		SetPos(Pos, k - 1, l, m_terrain->m_ChunkSize, n);
	}

	if (n == 0 && l > 0)
	{
		SetPos(Pos, k, l - 1, m, m_terrain->m_ChunkSize);
	}

	if (m == 0 && k > 0 && n == 0 && l > 0)
	{
		SetPos(Pos, k - 1, l - 1, m_terrain->m_ChunkSize, m_terrain->m_ChunkSize);
	}

	if (m % (m_terrain->m_ChunkSize / m_terrain->m_MinLodChunkSize) == 0 && n % (m_terrain->m_ChunkSize / m_terrain->m_MinLodChunkSize) == 0)
	{
		m_terrain->m_VertexElems.SetPosition((unsigned char*)m_terrain->m_rootVb.Lock(0, 0, 0) + (o * (m_terrain->m_ColChunks * m_terrain->m_MinLodChunkSize + 1) + p) * m_terrain->m_VertexStride, Pos);
		m_terrain->m_rootVb.Unlock();
	}

	if (UpdateNormal)
	{
		for (int _i = Max(0, i - 1); _i <= Min((int)m_terrain->m_Chunks.shape()[0] * ((int)m_terrain->m_IndexTable.shape()[0] - 1), i + 1); _i++)
		{
			for (int _j = Max(0, j - 1); _j <= Min((int)m_terrain->m_Chunks.shape()[1] * ((int)m_terrain->m_IndexTable.shape()[1] - 1), j + 1); _j++)
			{
				const Vector3 pos = GetPos(_i, _j);
				const Vector3 Dirs[4] = {
					GetPos(_i - 1, _j) - pos,
					GetPos(_i, _j - 1) - pos,
					GetPos(_i + 1, _j) - pos,
					GetPos(_i, _j + 1) - pos
				};
				const Vector3 Nors[4] = {
					Dirs[0].cross(Dirs[1]).normalize(),
					Dirs[1].cross(Dirs[2]).normalize(),
					Dirs[2].cross(Dirs[3]).normalize(),
					Dirs[3].cross(Dirs[0]).normalize()
				};
				const Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();
				SetNormal(Normal, _i, _j);
			}
		}
	}
}

void TerrainStream::SetPos(const my::Vector3& Pos, int k, int l, int m, int n)
{
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
	my::VertexBufferPtr vb = GetVB(k, l);
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
	GetIndices(i, j, k, l, m, n, o, p);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Offset;
	my::VertexBufferPtr vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(ret), D3DLOCK_READONLY);
	ret = *(D3DCOLOR*)pbuff;
	vb->Unlock();
	return ret;
}

void TerrainStream::SetColor(D3DCOLOR Color, int i, int j)
{
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	SetColor(Color, k, l, m, n);

	if (m == 0 && k > 0)
	{
		SetColor(Color, k - 1, l, m_terrain->m_ChunkSize, n);
	}

	if (n == 0 && l > 0)
	{
		SetColor(Color, k, l - 1, m, m_terrain->m_ChunkSize);
	}

	if (m == 0 && k > 0 && n == 0 && l > 0)
	{
		SetColor(Color, k - 1, l - 1, m_terrain->m_ChunkSize, m_terrain->m_ChunkSize);
	}

	if (m % (m_terrain->m_ChunkSize / m_terrain->m_MinLodChunkSize) == 0 && n % (m_terrain->m_ChunkSize / m_terrain->m_MinLodChunkSize) == 0)
	{
		m_terrain->m_VertexElems.SetColor((unsigned char*)m_terrain->m_rootVb.Lock(0, 0, 0) + (o * (m_terrain->m_ColChunks * m_terrain->m_MinLodChunkSize + 1) + p) * m_terrain->m_VertexStride, Color, 0);
		m_terrain->m_rootVb.Unlock();
	}
}

void TerrainStream::SetColor(D3DCOLOR Color, int k, int l, int m, int n)
{
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Offset;
	my::VertexBufferPtr vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(Color), 0);
	*(D3DCOLOR*)pbuff = Color;
	vb->Unlock();
	m_VertDirty[k][l] = true;
}

my::Vector3 TerrainStream::GetNormal(int i, int j)
{
	D3DCOLOR ret;
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][1].Offset;
	my::VertexBufferPtr vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(ret), D3DLOCK_READONLY);
	ret = *(D3DCOLOR*)pbuff;
	vb->Unlock();
	const float f = 1.0f / 255.0f;
	return my::Vector3(
		f * (float)(unsigned char)(ret >> 16),
		f * (float)(unsigned char)(ret >> 8),
		f * (float)(unsigned char)(ret >> 0)) * 2.0f - 1.0f;
}

void TerrainStream::SetNormal(const my::Vector3& Normal, int i, int j)
{
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	D3DCOLOR dw = D3DCOLOR_COLORVALUE((Normal.x + 1.0f) * 0.5f, (Normal.y + 1.0f) * 0.5f, (Normal.z + 1.0f) * 0.5f, 0);
	SetNormal(dw, k, l, m, n);

	if (m == 0 && k > 0)
	{
		SetNormal(dw, k - 1, l, m_terrain->m_ChunkSize, n);
	}

	if (n == 0 && l > 0)
	{
		SetNormal(dw, k, l - 1, m, m_terrain->m_ChunkSize);
	}

	if (m == 0 && k > 0 && n == 0 && l > 0)
	{
		SetNormal(dw, k - 1, l - 1, m_terrain->m_ChunkSize, m_terrain->m_ChunkSize);
	}

	if (m % (m_terrain->m_ChunkSize / m_terrain->m_MinLodChunkSize) == 0 && n % (m_terrain->m_ChunkSize / m_terrain->m_MinLodChunkSize) == 0)
	{
		m_terrain->m_VertexElems.SetColor((unsigned char*)m_terrain->m_rootVb.Lock(0, 0, 0) + (o * (m_terrain->m_ColChunks * m_terrain->m_MinLodChunkSize + 1) + p) * m_terrain->m_VertexStride, dw, 1);
		m_terrain->m_rootVb.Unlock();
	}
}

void TerrainStream::SetNormal(D3DCOLOR dw, int k, int l, int m, int n)
{
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][1].Offset;
	my::VertexBufferPtr vb = GetVB(k, l);
	void* pbuff = vb->Lock(off, sizeof(dw), 0);
	*(D3DCOLOR*)pbuff = dw;
	vb->Unlock();
	m_VertDirty[k][l] = true;
}

my::RayResult TerrainStream::RayTest(const my::Ray& local_ray)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		const my::Ray& ray;
		TerrainStream& tstr;
		my::RayResult ret;
		Callback(const my::Ray& _ray, TerrainStream& _tstr)
			: ray(_ray)
			, tstr(_tstr)
			, ret(false, FLT_MAX)
		{
		}
		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			my::RayResult result;
			TerrainChunk* chunk = dynamic_cast<TerrainChunk*>(oct_entity);
			const Terrain::Fragment& frag = tstr.m_terrain->GetFragment(0, 0, 0, 0, 0);
			my::VertexBufferPtr vb = tstr.GetVB(chunk->m_Row, chunk->m_Col);
			result = Mesh::RayTest(
				ray,
				vb->Lock(0, 0, D3DLOCK_READONLY),
				frag.VertNum,
				tstr.m_terrain->m_VertexStride,
				const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY),
				false,
				frag.PrimitiveCount,
				tstr.m_terrain->m_VertexElems);
			vb->Unlock();
			const_cast<my::IndexBuffer&>(frag.ib).Unlock();
			if (result.first && result.second < ret.second)
			{
				ret = result;
			}
		}
	};

	Callback cb(local_ray, *this);
	m_terrain->QueryEntity(local_ray, &cb);
	return cb.ret;
}

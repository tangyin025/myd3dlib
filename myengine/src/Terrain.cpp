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

BOOST_CLASS_EXPORT(TerrainChunk)

BOOST_CLASS_EXPORT(Terrain)

TerrainChunk::TerrainChunk(void)
	: m_Row(0)
	, m_Col(0)
	, m_Requested(false)
{
}

TerrainChunk::TerrainChunk(int Row, int Col, Terrain * terrain)
	: m_Row(Row)
	, m_Col(Col)
	, m_Requested(false)
{
}

TerrainChunk::~TerrainChunk(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
	}
}

template<class Archive>
void TerrainChunk::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Row);
	ar << BOOST_SERIALIZATION_NVP(m_Col);
}

template<class Archive>
void TerrainChunk::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Row);
	ar >> BOOST_SERIALIZATION_NVP(m_Col);
}

void TerrainChunk::OnReady(my::IORequest* request)
{
	m_vb = boost::dynamic_pointer_cast<VertexBuffer>(request->m_res);
}

void TerrainChunk::RequestResource(void)
{
	m_Requested = true;

	if (!dynamic_cast<Terrain *>(m_Node->GetTopNode())->m_ChunkPath.empty())
	{
		_ASSERT(!m_vb);

		std::ostringstream chunk_path;
		chunk_path << dynamic_cast<Terrain*>(m_Node->GetTopNode())->m_ChunkPath << "_" << m_Row << "_" << m_Col << ".chunk";
		my::ResourceMgr::getSingleton().LoadVertexBufferAsync(chunk_path.str().c_str(), this);
	}
}

void TerrainChunk::ReleaseResource(void)
{
	if (!dynamic_cast<Terrain*>(m_Node->GetTopNode())->m_ChunkPath.empty())
	{
		std::ostringstream chunk_path;
		chunk_path << dynamic_cast<Terrain*>(m_Node->GetTopNode())->m_ChunkPath << "_" << m_Row << "_" << m_Col << ".chunk";
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(chunk_path.str(), this);

		m_vb.reset();
	}

	m_Requested = false;
}

my::AABB TerrainChunk::CalculateAABB(Terrain * terrain) const
{
	AABB ret = AABB::Invalid();
	TerrainStream tstr(terrain);
	for (int i = 0; i < (int)terrain->m_IndexTable.shape()[0]; i++)
	{
		for (int j = 0; j < (int)terrain->m_IndexTable.shape()[1]; j++)
		{
			ret.unionSelf(tstr.GetPos(m_Row * terrain->m_ChunkSize + i, m_Col * terrain->m_ChunkSize + j));
		}
	}
	if (ret.m_max.y - ret.m_min.y < EPSILON_E6)
	{
		ret.shrinkSelf(0, -1.0f, 0);
	}
	return ret;
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

Terrain::Terrain(void)
	: m_RowChunks(1)
	, m_ColChunks(1)
	, m_ChunkSize(8)
	, m_HeightScale(1)
	, handle_World(NULL)
{
	CreateElements();
}

Terrain::Terrain(const char * Name, int RowChunks, int ColChunks, int ChunkSize, float HeightScale)
	: Component(ComponentTypeTerrain, Name)
	, OctRoot(0, -1.0f, 0, (float)ChunkSize * ColChunks, 1.0f, (float)ChunkSize * RowChunks)
	, m_RowChunks(RowChunks)
	, m_ColChunks(ColChunks)
	, m_ChunkSize(ChunkSize)
	, m_IndexTable(boost::extents[ChunkSize + 1][ChunkSize + 1])
	, m_HeightScale(HeightScale)
	, m_Chunks(boost::extents[RowChunks][ColChunks])
	, handle_World(NULL)
{
	CreateElements();
	_FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	m_RootVb.CreateVertexBuffer((m_RowChunks + 1) * (m_ColChunks + 1) * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
	VOID * pVertices = m_RootVb.Lock(0, 0, 0);
	for (int i = 0; i < (int)m_RowChunks + 1; i++)
	{
		for (int j = 0; j < (int)m_ColChunks + 1; j++)
		{
			unsigned char * pVertex = (unsigned char *)pVertices + (i * (m_ColChunks + 1) + j) * m_VertexStride;
			m_VertexElems.SetPosition(pVertex, Vector3((float)j * m_ChunkSize, 0, (float)i * m_ChunkSize), 0);
			m_VertexElems.SetColor(pVertex, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
			m_VertexElems.SetColor(pVertex, D3DCOLOR_COLORVALUE(0.5f, 1.0f, 0.5f, 0.0f), 1);
		}
	}
	m_RootVb.Unlock();
	m_RootIb.CreateIndexBuffer(m_RowChunks * m_ColChunks * 2 * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			m_Chunks[i][j].reset(new TerrainChunk(i, j, this));
			AABB aabb(j * m_ChunkSize, -1, i * m_ChunkSize, (j + 1) * m_ChunkSize, 1, (i + 1) * m_ChunkSize);
			AddEntity(m_Chunks[i][j].get(), aabb, MinBlock, Threshold);
		}
	}
}

Terrain::~Terrain(void)
{
	m_Decl.Release();
	m_RootVb.OnDestroyDevice();
	m_RootIb.OnDestroyDevice();
	ClearAllEntity();
}

static int _Quad(int v)
{
	if (v > 1)
	{
		return _Quad(v / 2) + 1;
	}
	return 0;
}

unsigned int Terrain::CalculateLod(int i, int j, const my::Vector3 & LocalViewPos) const
{
	float DistanceSq = Vector2(
		(j + 0.5f) * m_ChunkSize - LocalViewPos.x,
		(i + 0.5f) * m_ChunkSize - LocalViewPos.z).magnitudeSq();
	int Lod = (int)(logf(sqrt(DistanceSq) / m_Actor->m_LodDist) / logf(m_Actor->m_LodFactor));
	return Clamp(Lod, 0, _Quad(m_ChunkSize));
}

template <>
unsigned char Terrain::GetSampleValue<unsigned char>(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j)
{
	_ASSERT(i >= 0 && i < (int)desc.Height);
	_ASSERT(j >= 0 && j < (int)desc.Width);
	return *(unsigned char *)((unsigned char *)lrc.pBits + i * lrc.Pitch + j * sizeof(unsigned char));
}

template <>
short Terrain::GetSampleValue<short>(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j)
{
	_ASSERT(i >= 0 && i < (int)desc.Height);
	_ASSERT(j >= 0 && j < (int)desc.Width);
	int value = *(unsigned short *)((unsigned char *)lrc.pBits + i * lrc.Pitch + j * sizeof(unsigned short));
	return (short)(value - 32768);
}

template <typename T>
my::Vector3 Terrain::GetSamplePos(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j, float HeightScale)
{
	return my::Vector3((float)j, HeightScale * GetSampleValue<T>(desc, lrc, my::Clamp<int>(i, 0, desc.Height - 1), my::Clamp<int>(j, 0, desc.Width - 1)), (float)i);
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
	ar << BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkPath);
	D3DVERTEXBUFFER_DESC desc = const_cast<my::VertexBuffer&>(m_RootVb).GetDesc();
	ar << boost::serialization::make_nvp("BufferSize", desc.Size);
	void * pVertices = const_cast<my::VertexBuffer&>(m_RootVb).Lock(0, 0, D3DLOCK_READONLY);
	ar << boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, desc.Size));
	const_cast<my::VertexBuffer&>(m_RootVb).Unlock();
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			ar << boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d", i, j).c_str(), m_Chunks[i][j]);
			ar << boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d_aabb", i, j).c_str(), *m_Chunks[i][j]->m_OctAabb);
		}
	}
}

template<class Archive>
void Terrain::load(Archive & ar, const unsigned int version)
{
	ClearAllEntity();
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar >> BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkSize);
	m_IndexTable.resize(boost::extents[m_ChunkSize + 1][m_ChunkSize + 1]);
	_FillVertexTable(m_IndexTable, m_ChunkSize + 1);
	ar >> BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkPath);
	DWORD BufferSize;
	ar >> BOOST_SERIALIZATION_NVP(BufferSize);
	ResourceMgr::getSingleton().EnterDeviceSectionIfNotMainThread(); // ! unpaired lock/unlock will break the main thread m_d3dDevice->Present
	m_RootVb.OnDestroyDevice();
	m_RootVb.CreateVertexBuffer(BufferSize, 0, 0, D3DPOOL_MANAGED);
	void * pVertices = m_RootVb.Lock(0, 0, 0);
	ResourceMgr::getSingleton().LeaveDeviceSectionIfNotMainThread();
	ar >> boost::serialization::make_nvp("VertexBuffer", boost::serialization::binary_object(pVertices, BufferSize));
	ResourceMgr::getSingleton().EnterDeviceSectionIfNotMainThread();
	m_RootVb.Unlock();
	m_RootIb.OnDestroyDevice();
	m_RootIb.CreateIndexBuffer(m_RowChunks * m_ColChunks * 2 * 3 * sizeof(IndexTable::element), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	ResourceMgr::getSingleton().LeaveDeviceSectionIfNotMainThread();
	m_Chunks.resize(boost::extents[m_RowChunks][m_ColChunks]);
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			AABB aabb;
			ar >> boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d", i, j).c_str(), m_Chunks[i][j]);
			ar >> boost::serialization::make_nvp(str_printf("m_Chunk_%d_%d_aabb", i, j).c_str(), aabb);
			AddEntity(m_Chunks[i][j].get(), aabb, MinBlock, Threshold);
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
	m_Decl.Release();
	m_Fragment.clear();
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			if (m_Chunks[i][j]->IsRequested())
			{
				m_Chunks[i][j]->ReleaseResource();
			}
		}
	}
	Component::ReleaseResource();
}

void Terrain::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);
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
			ret.unionSelf(*m_Chunks[i][j]->m_OctAabb);
		}
	}
	return ret;
}

bool Terrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
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
		bool & ret;
		Callback(RenderPipeline * _pipeline, unsigned int _PassMask, const Vector3 & _LocalViewPos, Terrain * _terrain, IndexTable::element * _pIndices, bool & _ret)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, terrain(_terrain)
			, pIndices(_pIndices)
			, RootPrimitiveCount(0)
			, ret(_ret)
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_entity);
			const unsigned int lod0 = terrain->CalculateLod(chunk->m_Row, chunk->m_Col, LocalViewPos);
			if (lod0 == _Quad(terrain->m_ChunkSize))
			{
				if (chunk->IsRequested())
				{
					chunk->ReleaseResource();
				}
				pIndices[RootPrimitiveCount * 3 + 0] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 0);
				pIndices[RootPrimitiveCount * 3 + 1] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 0);
				pIndices[RootPrimitiveCount * 3 + 2] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 1);
				pIndices[RootPrimitiveCount * 3 + 3] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 1);
				pIndices[RootPrimitiveCount * 3 + 4] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 0);
				pIndices[RootPrimitiveCount * 3 + 5] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 1);
				RootPrimitiveCount += 2;
				return;
			}

			if (!chunk->m_vb)
			{
				if (!chunk->IsRequested())
				{
					chunk->RequestResource();
				}
				pIndices[RootPrimitiveCount * 3 + 0] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 0);
				pIndices[RootPrimitiveCount * 3 + 1] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 0);
				pIndices[RootPrimitiveCount * 3 + 2] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 1);
				pIndices[RootPrimitiveCount * 3 + 3] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 0) + (chunk->m_Col + 1);
				pIndices[RootPrimitiveCount * 3 + 4] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 0);
				pIndices[RootPrimitiveCount * 3 + 5] = (terrain->m_ColChunks + 1) * (chunk->m_Row + 1) + (chunk->m_Col + 1);
				RootPrimitiveCount += 2;
				return;
			}

			const unsigned int lod[5] =
			{
				lod0,
				terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, LocalViewPos),
				terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, LocalViewPos)
			};
			const Fragment & frag = terrain->GetFragment(lod[0], lod[1], lod[2], lod[3], lod[4]);
			if (terrain->m_Material && (terrain->m_Material->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (terrain->m_Material->m_PassMask & PassMask))
					{
						Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, NULL, terrain->m_Material->m_Shader.c_str(), PassID);
						if (shader)
						{
							if (!terrain->handle_World)
							{
								BOOST_VERIFY(terrain->handle_World = shader->GetParameterByName(NULL, "g_World"));
							}

							pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, chunk->m_vb->m_ptr, frag.ib.m_ptr, D3DPT_TRIANGLELIST,
								0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, shader, terrain, terrain->m_Material.get(), MAKELONG(chunk->m_Row, chunk->m_Col));

							ret = true;
						}
					}
				}
			}
		}
	};

	bool ret = false;

	if (m_Decl)
	{
		// ! do not use m_World for level offset
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		Callback cb(pipeline, PassMask, LocalViewPos, this, (IndexTable::element *)m_RootIb.Lock(0, 0, 0), ret);
		QueryEntity(LocalFrustum, &cb);
		m_RootIb.Unlock();
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
							}

							pipeline->PushIndexedPrimitive(PassID, m_Decl, m_RootVb.m_ptr, m_RootIb.m_ptr, D3DPT_TRIANGLELIST,
								0, 0, (m_RowChunks + 1) * (m_ColChunks + 1), m_VertexStride, 0, cb.RootPrimitiveCount, shader, this, m_Material.get(), MAKELONG(0, 0));

							ret = true;
						}
					}
				}
			}
		}
	}

	return ret;
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
		&& !m_Actor->m_PxActor->is<physx::PxRigidBody>()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		return;
	}

	std::vector<physx::PxHeightFieldSample> Samples((m_RowChunks * m_ChunkSize + 1) * (m_ColChunks * m_ChunkSize + 1));
	for (int Row = 0; Row < m_RowChunks; Row++)
	{
		for (int Col = 0; Col < m_ColChunks; Col++)
		{
			TerrainChunk * chunk = m_Chunks[Row][Col].get();
			VOID * pVertices = chunk->m_vb->Lock(0, 0, 0);
			for (int i = 0; i < ((Row < m_RowChunks - 1) ? m_ChunkSize : m_ChunkSize + 1); i++)
			{
				for (int j = 0; j < ((Col < m_ColChunks - 1) ? m_ChunkSize : m_ChunkSize + 1); j++)
				{
					//// ! Reverse physx height field row, column
					unsigned char * pVertex = (unsigned char *)pVertices + m_IndexTable[i][j] * m_VertexStride;
					int sample_i = (Col * m_ChunkSize + j) * (m_RowChunks * m_ChunkSize + 1) + Row * m_ChunkSize + i;
					Samples[sample_i].height = (short)Clamp<int>((int)(m_VertexElems.GetPosition(pVertex).y / m_HeightScale + 0.5f), SHRT_MIN, SHRT_MAX);
					Samples[sample_i].materialIndex0 = physx::PxBitAndByte(0, false);
					Samples[sample_i].materialIndex1 = physx::PxBitAndByte(0, false);
				}
			}
			chunk->m_vb->Unlock();
		}
	}

	physx::PxHeightFieldDesc hfDesc;
	hfDesc.nbRows             = m_ColChunks * m_ChunkSize + 1;
	hfDesc.nbColumns          = m_RowChunks * m_ChunkSize + 1;
	hfDesc.format             = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.samples.data       = &Samples[0];
	hfDesc.samples.stride     = sizeof(Samples[0]);
	m_PxHeightField.reset(PhysxSdk::getSingleton().m_Cooking->createHeightField(
		hfDesc, PhysxSdk::getSingleton().m_sdk->getPhysicsInsertionCallback()), PhysxDeleter<physx::PxHeightField>());

	m_PxMaterial.reset(PhysxSdk::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f), PhysxDeleter<physx::PxMaterial>());

	m_PxShape.reset(PhysxSdk::getSingleton().m_sdk->createShape(
		physx::PxHeightFieldGeometry(m_PxHeightField.get(), physx::PxMeshGeometryFlags(), m_HeightScale * m_Actor->m_Scale.y, m_Actor->m_Scale.x, m_Actor->m_Scale.z),
		*m_PxMaterial, true, /*physx::PxShapeFlag::eVISUALIZATION |*/ physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE), PhysxDeleter<physx::PxShape>());

	m_Actor->m_PxActor->attachShape(*m_PxShape);

	SetSimulationFilterWord0(filterWord0);

	SetQueryFilterWord0(filterWord0);

	m_PxShape->userData = this;
}

void Terrain::ClearShape(void)
{
	Component::ClearShape();

	m_PxHeightField.reset();
}

void Terrain::UpdateHeightMap(my::Texture2D * HeightMap, float HeightScale)
{
	my::TexturePixel2D pixel(HeightMap);
	TerrainStream tstr(this);
	for (int i = 0; i <= (int)m_Chunks.shape()[0] * ((int)m_IndexTable.shape()[0] - 1); i++)
	{
		for (int j = 0; j <= (int)m_Chunks.shape()[1] * ((int)m_IndexTable.shape()[1] - 1); j++)
		{
			switch (pixel.desc.Format)
			{
			case D3DFMT_A8:
			case D3DFMT_L8:
				tstr.SetPos(Vector3(j, HeightScale * pixel.Get<unsigned char>(i, j), i), i, j);
				break;
			case D3DFMT_L16:
				tstr.SetPos(Vector3(j, HeightScale * ((int)pixel.Get<unsigned short>(i, j) - 32768), i), i, j);
				break;
			}
		}
	}
	tstr.Release();
	pixel.Release();

	UpdateVerticesNormal();

	UpdateChunkAABB();
}

void Terrain::UpdateVerticesNormal(void)
{
	TerrainStream tstr(this);
	for (int i = 0; i <= (int)m_Chunks.shape()[0] * ((int)m_IndexTable.shape()[0] - 1); i++)
	{
		for (int j = 0; j <= (int)m_Chunks.shape()[1] * ((int)m_IndexTable.shape()[1] - 1); j++)
		{
			const Vector3 pos = tstr.GetPos(i, j);
			const Vector3 Dirs[4] = {
				tstr.GetPos(i - 1, j) - pos,
				tstr.GetPos(i, j - 1) - pos,
				tstr.GetPos(i + 1, j) - pos,
				tstr.GetPos(i, j + 1) - pos
			};
			const Vector3 Nors[4] = {
				Dirs[0].cross(Dirs[1]).normalize(),
				Dirs[1].cross(Dirs[2]).normalize(),
				Dirs[2].cross(Dirs[3]).normalize(),
				Dirs[3].cross(Dirs[0]).normalize()
			};
			const Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();
			tstr.SetNormal(Normal, i, j);
		}
	}
}

void Terrain::UpdateChunkAABB(void)
{
	for (int i = 0; i < m_RowChunks; i++)
	{
		for (int j = 0; j < m_ColChunks; j++)
		{
			TerrainChunk * chunk = m_Chunks[i][j].get();
			RemoveEntity(chunk);
			AddEntity(chunk, chunk->CalculateAABB(this), MinBlock, Threshold);
		}
	}
}

void Terrain::UpdateSplatmap(my::Texture2D * ColorMap)
{
	my::TexturePixel2D pixel(ColorMap);
	TerrainStream tstr(this);
	for (int i = 0; i <= (int)m_Chunks.shape()[0] * ((int)m_IndexTable.shape()[0] - 1); i++)
	{
		for (int j = 0; j <= (int)m_Chunks.shape()[1] * ((int)m_IndexTable.shape()[1] - 1); j++)
		{
			switch (pixel.desc.Format)
			{
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
				tstr.SetColor(pixel.Get<DWORD>(i, j), i, j);
				break;
			}
		}
	}
}

bool Terrain::Raycast(const my::Vector3 & origin, const my::Vector3 & dir, my::Vector3 & hitPos, my::Vector3 & hitNormal)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		Terrain * terrain;
		const my::Vector3 & origin;
		const my::Vector3 & dir;
		my::RayResult ret;
		my::Vector3 retNormal;
		Callback(Terrain * _terrain, const my::Vector3 & _origin, const my::Vector3 & _dir)
			: terrain(_terrain)
			, origin(_origin)
			, dir(_dir)
			, ret(false, FLT_MAX)
		{
		}
		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_entity);
			const Terrain::Fragment & frag = terrain->GetFragment(0, 0, 0, 0, 0);
			VOID * pVertices = chunk->m_vb->Lock(0, 0, D3DLOCK_READONLY);
			boost::multi_array_ref<IndexTable::element, 1> idx(
				(IndexTable::element *)const_cast<my::IndexBuffer&>(frag.ib).Lock(0, 0, D3DLOCK_READONLY), boost::extents[frag.PrimitiveCount * 3]);
			for (unsigned int i = 0; i < idx.shape()[0]; i += 3)
			{
				const my::Vector3 & v0 = terrain->m_VertexElems.GetPosition((unsigned char *)pVertices + idx[i + 0] * terrain->m_VertexStride);
				const my::Vector3 & v1 = terrain->m_VertexElems.GetPosition((unsigned char *)pVertices + idx[i + 1] * terrain->m_VertexStride);
				const my::Vector3 & v2 = terrain->m_VertexElems.GetPosition((unsigned char *)pVertices + idx[i + 2] * terrain->m_VertexStride);
				if (my::IntersectionTests::isValidTriangle(v0, v1, v2))
				{
					my::RayResult result = my::CollisionDetector::rayAndTriangle(origin, dir, v0, v1, v2);
					if (result.first && result.second < ret.second)
					{
						ret = result;
						retNormal = my::IntersectionTests::calculateTriangleNormal(v0, v1, v2);
					}
				}
			}
			chunk->m_vb->Unlock();
			const_cast<my::IndexBuffer&>(frag.ib).Unlock();
		}
	} cb(this, origin, dir);

	QueryEntity(my::Ray(origin, dir), &cb);

	if (cb.ret.first)
	{
		hitPos = origin + dir * cb.ret.second;
		hitNormal = cb.retNormal;
		return true;
	}
	return false;
}

void Terrain::SaveChunkData(const char* path)
{
	for (int Row = 0; Row < m_RowChunks; Row++)
	{
		for (int Col = 0; Col < m_ColChunks; Col++)
		{
			std::vector<char> buff((m_ChunkSize + 1) * (m_ChunkSize + 1) * Terrain::m_VertexStride);
			for (int i = 0; i < m_ChunkSize + 1; i++)
			{
				for (int j = 0; j < m_ChunkSize + 1; j++)
				{
					char* pVertex = &buff[0] + m_IndexTable[i][j] * m_VertexStride;
					m_VertexElems.SetPosition(pVertex, Vector3((float)Col * m_ChunkSize + j, 0, (float)Row * m_ChunkSize + i), 0);
					m_VertexElems.SetColor(pVertex, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
					m_VertexElems.SetColor(pVertex, D3DCOLOR_COLORVALUE(0.5f, 1.0f, 0.5f, 0.0f), 1);
				}
			}

			std::ostringstream chunk_path;
			chunk_path << path << "_" << Row << "_" << Col << ".chunk";
			std::ofstream ofs(chunk_path.str(), std::ios::binary);
			ofs.write(&buff[0], buff.size());
		}
	}
}

TerrainStream::TerrainStream(Terrain* terrain)
	: m_terrain(terrain)
	, m_fstrs(boost::extents[terrain->m_RowChunks][terrain->m_ColChunks])
	, m_RootVerts((unsigned char *)terrain->m_RootVb.Lock(0, 0, 0))
{
}

TerrainStream::~TerrainStream(void)
{
	if (m_RootVerts)
	{
		Release();
	}
}

void TerrainStream::Release(void)
{
	_ASSERT(m_RootVerts);
	m_terrain->m_RootVb.Unlock();
	m_RootVerts = NULL;
	for (int i = 0; i < m_fstrs.shape()[0]; i++)
	{
		for (int j = 0; j < m_fstrs.shape()[1]; j++)
		{
			if (m_fstrs[i][j].is_open())
			{
				m_fstrs[i][j].flush();
				m_fstrs[i][j].close();
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
		o = m_terrain->m_RowChunks;
	}
	else
	{
		k = i / m_terrain->m_ChunkSize;
		m = i % m_terrain->m_ChunkSize;
		o = i / m_terrain->m_ChunkSize;
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
		p = m_terrain->m_ColChunks;
	}
	else
	{
		l = j / m_terrain->m_ChunkSize;
		n = j % m_terrain->m_ChunkSize;
		p = j / m_terrain->m_ChunkSize;
	}
}

std::fstream& TerrainStream::GetStream(int k, int l)
{
	std::fstream& ret = m_fstrs[k][l];
	if (!ret.is_open())
	{
		std::ostringstream chunk_path;
		chunk_path << m_terrain->m_ChunkPath << "_" << k << "_" << l << ".chunk";
		ret.open(my::ResourceMgr::getSingleton().GetFullPath(chunk_path.str().c_str()), std::ios::in | std::ios::out | std::ios::binary);
	}
	_ASSERT(ret.is_open());
	return ret;
}

my::Vector3 TerrainStream::GetPos(int i, int j)
{
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	std::fstream& fstr = GetStream(k, l);
	my::Vector3 ret;
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
	fstr.seekg(off, std::ios::beg);
	fstr.read((char*)&ret, sizeof(ret));
	return ret;
}

void TerrainStream::SetPos(const my::Vector3& Pos, int i, int j)
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

	if ((m == 0 || m == m_terrain->m_ChunkSize) && (n == 0 || n == m_terrain->m_ChunkSize))
	{
		m_terrain->m_VertexElems.SetPosition(m_RootVerts + (o * (m_terrain->m_ColChunks + 1) + p) * m_terrain->m_VertexStride, Pos);
	}
}

void TerrainStream::SetPos(const my::Vector3& Pos, int k, int l, int m, int n)
{
	std::fstream& fstr = GetStream(k, l);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset;
	fstr.seekp(off, std::ios::beg);
	fstr.write((char*)&Pos, sizeof(Pos));

	if (m_terrain->m_Chunks[k][l]->IsRequested())
	{
		m_terrain->m_Chunks[k][l]->ReleaseResource();
		my::ResourceMgr::getSingleton().CheckIORequests(0);
	}
}

D3DCOLOR TerrainStream::GetColor(int i, int j)
{
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	std::fstream& fstr = GetStream(k, l);
	D3DCOLOR ret;
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Offset;
	fstr.seekg(off, std::ios::beg);
	fstr.read((char*)&ret, sizeof(ret));
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

	if ((m == 0 || m == m_terrain->m_ChunkSize) && (n == 0 || n == m_terrain->m_ChunkSize))
	{
		m_terrain->m_VertexElems.SetColor(m_RootVerts + (o * (m_terrain->m_ColChunks + 1) + p) * m_terrain->m_VertexStride, Color, 0);
	}
}

void TerrainStream::SetColor(D3DCOLOR Color, int k, int l, int m, int n)
{
	std::fstream& fstr = GetStream(k, l);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][0].Offset;
	fstr.seekp(off, std::ios::beg);
	fstr.write((char*)&Color, sizeof(Color));

	if (m_terrain->m_Chunks[k][l]->IsRequested())
	{
		m_terrain->m_Chunks[k][l]->ReleaseResource();
		my::ResourceMgr::getSingleton().CheckIORequests(0);
	}
}

my::Vector3 TerrainStream::GetNormal(int i, int j)
{
	int k, l, m, n, o, p;
	GetIndices(i, j, k, l, m, n, o, p);
	std::fstream& fstr = GetStream(k, l);
	D3DCOLOR ret;
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][1].Offset;
	fstr.seekg(off, std::ios::beg);
	fstr.read((char*)&ret, sizeof(ret));
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

	if ((m == 0 || m == m_terrain->m_ChunkSize) && (n == 0 || n == m_terrain->m_ChunkSize))
	{
		m_terrain->m_VertexElems.SetColor(m_RootVerts + (o * (m_terrain->m_ColChunks + 1) + p) * m_terrain->m_VertexStride, dw, 1);
	}
}

void TerrainStream::SetNormal(D3DCOLOR dw, int k, int l, int m, int n)
{
	std::fstream& fstr = GetStream(k, l);
	std::streamoff off = m_terrain->m_IndexTable[m][n] * m_terrain->m_VertexStride + m_terrain->m_VertexElems.elems[D3DDECLUSAGE_COLOR][1].Offset;
	fstr.seekp(off, std::ios::beg);
	fstr.write((char*)&dw, sizeof(dw));

	if (m_terrain->m_Chunks[k][l]->IsRequested())
	{
		m_terrain->m_Chunks[k][l]->ReleaseResource();
		my::ResourceMgr::getSingleton().CheckIORequests(0);
	}
}

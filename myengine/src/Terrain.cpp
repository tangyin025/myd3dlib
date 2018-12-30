#include "Terrain.h"
#include "Actor.h"
#include "Material.h"
#include "PhysXContext.h"
#include "RenderPipeline.h"
#include "myDxutApp.h"
#include "myEffect.h"
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

TerrainChunk::TerrainChunk(Terrain * Owner, int Row, int Col)
	: Emitter(PARTICLE_INSTANCE_MAX)
	, m_Owner(Owner)
	, m_Row(Row)
	, m_Col(Col)
{
	D3DLOCKED_RECT lrc = m_Owner->m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	m_aabb.m_min = m_Owner->GetSamplePos(lrc.pBits, lrc.Pitch, (m_Row + 0) * (Owner->m_ChunkSize), (m_Col + 0) * (Owner->m_ChunkSize));
	m_aabb.m_max = m_Owner->GetSamplePos(lrc.pBits, lrc.Pitch, (m_Row + 1) * (Owner->m_ChunkSize), (m_Col + 1) * (Owner->m_ChunkSize));
	m_Owner->m_HeightMap.UnlockRect(0);
}

TerrainChunk::TerrainChunk(void)
	: Emitter(PARTICLE_INSTANCE_MAX)
	, m_Owner(NULL)
	, m_Row(0)
	, m_Col(0)
{
	m_aabb = AABB::Invalid();
}

TerrainChunk::~TerrainChunk(void)
{
}

void TerrainChunk::UpdateAABB(void)
{
	m_aabb = AABB::Invalid();
	D3DLOCKED_RECT lrc = m_Owner->m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	for (int i = 0; i < m_Owner->m_ChunkSize + 1; i++)
	{
		const int row_i = m_Row * m_Owner->m_ChunkSize + i;
		for (int j = 0; j < m_Owner->m_ChunkSize + 1; j++)
		{
			const int col_i = m_Col * m_Owner->m_ChunkSize + j;
			Vector3 Pos = m_Owner->GetSamplePos(lrc.pBits, lrc.Pitch, row_i, col_i);
			m_aabb.unionSelf(Pos);
		}
	}
	m_Owner->m_HeightMap.UnlockRect(0);
}

static unsigned int FillVertexTable(Terrain::VertexArray2D & verts, int N, int hs, unsigned int k)
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

static unsigned int FillVertexTable(Terrain::VertexArray2D & verts, int N)
{
    _ASSERT(((N - 1) & (N - 2)) == 0);
    unsigned int k = 0;
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
	, m_bNavigation(false)
{
}

Terrain::Terrain(int RowChunks, int ColChunks, int ChunkSize, float HeightScale)
	: Component(ComponentTypeTerrain)
	, m_RowChunks(RowChunks)
	, m_ColChunks(ColChunks)
	, m_ChunkSize(ChunkSize)
	, m_VertexTable(boost::extents[ChunkSize + 1][ChunkSize + 1])
	, m_HeightScale(HeightScale)
	, m_bNavigation(false)
	, m_Root(my::AABB(0, (float)ChunkSize * my::Max(RowChunks, ColChunks)))
	, m_Chunks(boost::extents[RowChunks][ColChunks])
{
	FillVertexTable(m_VertexTable, m_ChunkSize + 1);
	CreateHeightMap();
	UpdateHeightMapNormal();
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			TerrainChunkPtr chunk(new TerrainChunk(this, i, j));
			m_Root.AddActor(chunk, chunk->m_aabb);
			m_Chunks[i][j] = chunk.get();
		}
	}
	InitLodMap();
	CreateElements();
}

Terrain::~Terrain(void)
{
	//m_HeightMap.OnDestroyDevice();
	//_ASSERT(!m_Decl);
	//_ASSERT(!m_vb.m_ptr);
	//m_Root.ClearAllActor();
}

static int Quad(int v)
{
	if (v <= 1)
	{
		return 0;
	}

	return Quad(v / 2) + 1;
}

void Terrain::InitLodMap(void)
{
	for (int i = 0; i <= Quad(m_ChunkSize); i++)
	{
		m_LodMap.insert(std::make_pair(pow(Vector2(m_ChunkSize * 0.6f, m_ChunkSize * 0.6f).magnitude() * (i + 1), 2), i));
	}
}

unsigned int Terrain::CalculateLod(int i, int j, const my::Vector3 & LocalViewPos)
{
	float DistanceSq = Vector2(
		(j + 0.5f) * m_ChunkSize - LocalViewPos.x,
		(i + 0.5f) * m_ChunkSize - LocalViewPos.z).magnitudeSq();
	LodMap::const_iterator lod_iter = m_LodMap.lower_bound(DistanceSq);
	if (lod_iter != m_LodMap.end())
	{
		return lod_iter->second;
	}
	return Quad(m_ChunkSize);
}

void Terrain::CreateHeightMap(void)
{
	m_HeightMap.OnDestroyDevice();
	m_HeightMap.CreateTexture(m_ColChunks * m_ChunkSize, m_RowChunks * m_ChunkSize, 1, 0, D3DFMT_A8R8G8B8);
}

void Terrain::UpdateHeightMap(my::Texture2DPtr HeightMap)
{
	D3DSURFACE_DESC SrcDesc = HeightMap->GetLevelDesc(0);
	switch (SrcDesc.Format)
	{
	case D3DFMT_A8:
	case D3DFMT_L8:
		{
			RECT rc = { 0, 0, my::Min<LONG>(m_ColChunks * m_ChunkSize, SrcDesc.Width), my::Min<LONG>(m_RowChunks * m_ChunkSize, SrcDesc.Height) };
			D3DLOCKED_RECT SrcLrc = HeightMap->LockRect(&rc, 0, 0);
			D3DLOCKED_RECT DstLrc = m_HeightMap.LockRect(&rc, 0, 0);
			for (int i = rc.top; i < rc.bottom; i++)
			{
				for (int j = rc.left; j < rc.right; j++)
				{
					unsigned char height = *((unsigned char *)SrcLrc.pBits + i * SrcLrc.Pitch + j);
					D3DCOLOR * DstBits = (D3DCOLOR *)((unsigned char *)DstLrc.pBits + i * DstLrc.Pitch + j * sizeof(D3DCOLOR));
					*DstBits = D3DCOLOR_ARGB(height,0,0,0);
				}
			}
			m_HeightMap.UnlockRect(0);
			HeightMap->UnlockRect(0);
		}
		break;
	default:
		return;
	}
	UpdateHeightMapNormal();
	UpdateChunks();
}

void Terrain::UpdateHeightMapNormal(void)
{
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	for (int i = 0; i < m_RowChunks * m_ChunkSize; i++)
	{
		for (int j = 0; j < m_ColChunks * m_ChunkSize; j++)
		{
			D3DCOLOR * Bits = (D3DCOLOR *)((unsigned char *)lrc.pBits + i * lrc.Pitch + j * sizeof(D3DCOLOR));
			Vector3 Pos = GetSamplePos(lrc.pBits, lrc.Pitch, i, j);
			const Vector3 Dirs[4] = {
				GetSamplePos(lrc.pBits, lrc.Pitch, i - 1, j) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i, j - 1) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i + 1, j) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i, j + 1) - Pos,
			};
			const Vector3 Nors[4] = {
				Dirs[0].cross(Dirs[1]).normalize(),
				Dirs[1].cross(Dirs[2]).normalize(),
				Dirs[2].cross(Dirs[3]).normalize(),
				Dirs[3].cross(Dirs[0]).normalize(),
			};
			Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();
			Normal = (Normal + 1.0f) / 2.0f * 255.0f;
			*Bits = D3DCOLOR_ARGB(GetCValue(*Bits), (unsigned char)Normal.x, (unsigned char)Normal.y, (unsigned char)Normal.z); 
		}
	}
	m_HeightMap.UnlockRect(0);
}

void Terrain::UpdateChunks(void)
{
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			m_Chunks[i][j]->UpdateAABB();
			TerrainChunkPtr chunk = boost::dynamic_pointer_cast<TerrainChunk>(m_Chunks[i][j]->shared_from_this());
			m_Root.RemoveActor(chunk);
			m_Root.AddActor(chunk, chunk->m_aabb);
		}
	}
}

D3DCOLOR Terrain::GetSampleValue(void * pBits, int pitch, int i, int j)
{
	i = Clamp<int>(i, 0, m_RowChunks * m_ChunkSize - 1);
	j = Clamp<int>(j, 0, m_ColChunks * m_ChunkSize - 1);
	D3DCOLOR * Bits = (D3DCOLOR *)((unsigned char *)pBits + i * pitch + j * sizeof(D3DCOLOR));
	return *Bits;
}

my::Vector3 Terrain::GetSamplePos(void * pBits, int pitch, int i, int j)
{
	return Vector3((float)j, m_HeightScale * GetCValue(GetSampleValue(pBits, pitch, i, j)), (float)i);
}

my::Vector3 Terrain::GetPosByVertexIndex(const void * pVertices, int Row, int Col, int VertexIndex, void * pBits, int pitch)
{
	unsigned char * pVertex = (unsigned char *)pVertices + VertexIndex * m_VertexStride;
	unsigned char * pIndices = m_VertexElems.GetVertexValue<unsigned char>(pVertex, D3DDECLUSAGE_TEXCOORD, 0);
	return GetSamplePos(pBits, pitch, Row * m_ChunkSize + pIndices[0], Col * m_ChunkSize + pIndices[1]);
}

void Terrain::CreateElements(void)
{
	m_VertexElems.InsertVertexElement(0, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLMETHOD_DEFAULT);
	WORD offset = sizeof(unsigned int);
	m_VertexStride = offset;
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
	unsigned int id = ((center & 7) << 12) | ((left & 7) << 9) | ((top & 7) << 6) | ((right & 7) << 3) | ((bottom & 7) << 0);
	FragmentMap::iterator frag_iter = m_Fragment.find(id);
	if (frag_iter != m_Fragment.end())
	{
		return frag_iter->second;
	}

    struct Setter
    {
		Terrain::VertexArray2D & verts;
        WORD * buff;
		Setter(Terrain::VertexArray2D & _verts, WORD * _buff)
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
		Terrain::VertexArray2D & verts;
        WORD * buff;
		SetterTranspose(Terrain::VertexArray2D & _verts, WORD * _buff)
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
		frag.ib.CreateIndexBuffer(frag.PrimitiveCount * 3 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
		VOID * pIndices = frag.ib.Lock(0, 0, 0);
		unsigned int k = 0;
		const int step = 1 << center;
		k += FillNvM(Setter(m_VertexTable, (WORD *)pIndices + k), N[0] - 2, N[0] - 2, 0 + step, step, 0 + step, step);
		k += EdgeNvM(SetterTranspose(m_VertexTable, (WORD *)pIndices + k), N[1], M[1], 0, step, N[0] * step, -step);
		k += EdgeNvM(Setter(m_VertexTable, (WORD *)pIndices + k), N[2], M[2], 0, step, 0, step);
		k += EdgeNvM(SetterTranspose(m_VertexTable, (WORD *)pIndices + k), N[3], M[3], N[0] * step, -step, 0, step);
		k += EdgeNvM(Setter(m_VertexTable, (WORD *)pIndices + k), N[4], M[4], N[0] * step, -step, N[0] * step, -step);
		_ASSERT(k == frag.PrimitiveCount * 3);
		frag.ib.Unlock();
	}
	else
	{
		const int N = m_ChunkSize >> center;
		frag.VertNum = (N + 1) * (N + 1);
		frag.PrimitiveCount = N * N * 2;
		frag.ib.CreateIndexBuffer(frag.PrimitiveCount * 3 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
		VOID * pIndices = frag.ib.Lock(0, 0, 0);
		const int step = 1 << center;
		unsigned int k = FillNvM(Setter(m_VertexTable, (WORD *)pIndices), N, N, 0, step, 0, step);
		frag.ib.Unlock();
	}
	return frag;
}

template<>
void Terrain::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkSize);
	ar << BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar << BOOST_SERIALIZATION_NVP(m_bNavigation);
	const DWORD BufferSize = m_RowChunks * m_ChunkSize * m_ColChunks * m_ChunkSize;
	std::vector<unsigned char> buff(BufferSize);
	D3DLOCKED_RECT lrc = const_cast<my::Texture2D&>(m_HeightMap).LockRect(NULL, D3DLOCK_READONLY, 0);
	std::copy(
		boost::make_transform_iterator((unsigned int *)lrc.pBits, boost::lambda::_1 >> 24),
		boost::make_transform_iterator((unsigned int *)lrc.pBits + BufferSize, boost::lambda::_1 >> 24), buff.begin());
	const_cast<my::Texture2D&>(m_HeightMap).UnlockRect(0);
	ar << boost::serialization::make_nvp("HeightMap", boost::serialization::binary_object(&buff[0], buff.size()));
	ar << BOOST_SERIALIZATION_NVP(m_Root);
	ar << BOOST_SERIALIZATION_NVP(m_GrassMaterial);
	ar << BOOST_SERIALIZATION_NVP(m_LodMap);
}

template<>
void Terrain::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkSize);
	m_VertexTable.resize(boost::extents[m_ChunkSize + 1][m_ChunkSize + 1]);
	FillVertexTable(m_VertexTable, m_ChunkSize + 1);
	ar >> BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar >> BOOST_SERIALIZATION_NVP(m_bNavigation);
	CreateHeightMap();
	const DWORD BufferSize = m_RowChunks * m_ChunkSize * m_ColChunks * m_ChunkSize;
	std::vector<unsigned char> buff(BufferSize);
	ar >> boost::serialization::make_nvp("HeightMap", boost::serialization::binary_object(&buff[0], buff.size()));
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, 0, 0);
	std::copy(
		boost::make_transform_iterator(buff.begin(), boost::lambda::ll_static_cast<unsigned int>(boost::lambda::_1) << 24),
		boost::make_transform_iterator(buff.begin() + BufferSize, boost::lambda::ll_static_cast<unsigned int>(boost::lambda::_1) << 24), (unsigned int *)lrc.pBits);
	m_HeightMap.UnlockRect(0);
	ar >> BOOST_SERIALIZATION_NVP(m_Root);
	ar >> BOOST_SERIALIZATION_NVP(m_GrassMaterial);
	ar >> BOOST_SERIALIZATION_NVP(m_LodMap);
	m_Chunks.resize(boost::extents[m_RowChunks][m_ColChunks]);
	struct Callback : public my::OctNode::QueryCallback
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
			chunk->m_Owner = terrain;
		}
	};
	m_Root.QueryActorAll(&Callback(this));
	UpdateHeightMapNormal();
	CreateElements();
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

	if (!m_vb.m_ptr)
	{
		m_vb.CreateVertexBuffer(Terrain::m_VertexTable.shape()[0] * Terrain::m_VertexTable.shape()[1] * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
		UpdateVertices();
	}

	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			m_Chunks[i][j]->m_Material->RequestResource();
		}
	}

	m_GrassMaterial->RequestResource();
}

void Terrain::ReleaseResource(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			m_Chunks[i][j]->m_Material->ReleaseResource();
		}
	}
	m_GrassMaterial->ReleaseResource();
	m_Fragment.clear();
	Component::ReleaseResource();
}

void Terrain::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	switch (GetBValue(lparam))
	{
	case RenderTypeTerrain:
	{
		shader->SetMatrix("g_World", m_Actor->m_World);
		shader->SetFloat("g_HeightScale", m_HeightScale);
		shader->SetVector("g_HeightTexSize", Vector2((float)m_RowChunks * m_ChunkSize, (float)m_ColChunks * m_ChunkSize));
		int ChunkId[2] = { GetRValue(lparam), GetGValue(lparam) };
		shader->SetIntArray("g_ChunkId", ChunkId, 2);
		shader->SetInt("g_ChunkSize", m_ChunkSize);
		shader->SetTexture("g_HeightTexture", &m_HeightMap);
		break;
	}
	case RenderTypeEmitter:
	{
		shader->SetMatrix("g_World", m_Actor->m_World);
		break;
	}
	default:
		break;
	}
}

void Terrain::UpdateVertices(void)
{
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int i = 0; i < Terrain::m_VertexTable.shape()[0]; i++)
		{
			for (unsigned int j = 0; j < Terrain::m_VertexTable.shape()[1]; j++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + Terrain::m_VertexTable[i][j] * m_VertexStride;
				unsigned char * pIndices = m_VertexElems.GetVertexValue<unsigned char>(pVertex, D3DDECLUSAGE_TEXCOORD, 0);
				pIndices[0] = i;
				pIndices[1] = j;
				pIndices[2] = 0;
				pIndices[3] = 0;
			}
		}
		m_vb.Unlock();
	}
}

my::AABB Terrain::CalculateAABB(void) const
{
	AABB ret = Component::CalculateAABB();
	for (unsigned int i = 0; i < m_Chunks.shape()[0]; i++)
	{
		for (unsigned int j = 0; j < m_Chunks.shape()[1]; j++)
		{
			ret.unionSelf(m_Chunks[i][j]->m_aabb);
		}
	}
	return ret;
}

void Terrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos)
{
	_ASSERT(m_Actor);

	struct Callback : public my::OctNode::QueryCallback
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
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (chunk->m_Material && (RenderPipeline::PassTypeToMask(PassID) & (chunk->m_Material->m_PassMask & PassMask)))
				{
					Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, false, chunk->m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						const Fragment & frag = terrain->GetFragment(
							terrain->CalculateLod(chunk->m_Row, chunk->m_Col, LocalViewPos),
							terrain->CalculateLod(chunk->m_Row, chunk->m_Col - 1, LocalViewPos),
							terrain->CalculateLod(chunk->m_Row - 1, chunk->m_Col, LocalViewPos),
							terrain->CalculateLod(chunk->m_Row, chunk->m_Col + 1, LocalViewPos),
							terrain->CalculateLod(chunk->m_Row + 1, chunk->m_Col, LocalViewPos));
						pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, terrain->m_vb.m_ptr,
							frag.ib.m_ptr, D3DPT_TRIANGLELIST, 0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, shader, terrain, chunk->m_Material.get(), RGB(chunk->m_Row, chunk->m_Col, RenderTypeTerrain));
					}
				}

				if (!chunk->m_ParticleList.empty() && terrain->m_GrassMaterial && (RenderPipeline::PassTypeToMask(PassID) & (terrain->m_GrassMaterial->m_PassMask & PassMask)))
				{
					Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeParticle, false, terrain->m_GrassMaterial->m_Shader.c_str(), PassID);
					if (shader)
					{
						pipeline->PushEmitter(PassID, chunk, shader, terrain, terrain->m_GrassMaterial.get(), RGB(chunk->m_Row, chunk->m_Col, RenderTypeEmitter));
					}
				}
			}
		}
	};

	if (m_vb.m_ptr)
	{
		// ! do not use m_World for level offset
		const Matrix4 & World = m_Actor->m_World;
		Frustum loc_frustum = frustum.transform(World.transpose());
		Vector3 loc_viewpos = ViewPos.transformCoord(World.inverse());
		m_Root.QueryActor(loc_frustum, &Callback(pipeline, PassMask, loc_viewpos, this));
	}
}

void Terrain::CreateHeightFieldShape(const my::Vector3 & Scale)
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

	if (!m_HeightMap.m_ptr)
	{
		return;
	}

	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	std::vector<physx::PxHeightFieldSample> Samples(
		(m_RowChunks * m_ChunkSize + 1) * (m_ColChunks * m_ChunkSize + 1));
	for (int i = 0; i < m_RowChunks * m_ChunkSize + 1; i++)
	{
		for (int j = 0; j < m_ColChunks * m_ChunkSize + 1; j++)
		{
			Samples[i * (m_ColChunks * m_ChunkSize + 1) + j].height = GetCValue(GetSampleValue(lrc.pBits, lrc.Pitch, i, j));
			Samples[i * (m_ColChunks * m_ChunkSize + 1) + j].materialIndex0 = physx::PxBitAndByte(0, false);
			Samples[i * (m_ColChunks * m_ChunkSize + 1) + j].materialIndex1 = physx::PxBitAndByte(0, false);
		}
	}
	m_HeightMap.UnlockRect(0);
	physx::PxHeightFieldDesc hfDesc;
	hfDesc.nbRows             = m_RowChunks * m_ChunkSize + 1;
	hfDesc.nbColumns          = m_ColChunks * m_ChunkSize + 1;
	hfDesc.format             = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.samples.data       = &Samples[0];
	hfDesc.samples.stride     = sizeof(Samples[0]);
	m_PxHeightField.reset(PhysXContext::getSingleton().m_sdk->createHeightField(hfDesc));

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f));

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxHeightFieldGeometry(m_PxHeightField.get(), physx::PxMeshGeometryFlags(), m_HeightScale * Scale.y, Scale.x, Scale.z),
		*m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
}

void Terrain::ClearShape(void)
{
	Component::ClearShape();

	m_PxHeightField.reset();
}

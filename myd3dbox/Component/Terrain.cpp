#include "StdAfx.h"
#include "Terrain.h"
#include "Actor.h"
#include "PhysXContext.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>

using namespace my;

BOOST_CLASS_EXPORT(TerrainChunk)

BOOST_CLASS_EXPORT(Terrain)

TerrainChunk::TerrainChunk(Terrain * Owner, int Row, int Column)
	: m_Owner(Owner)
	, m_Row(Row)
	, m_Column(Column)
	, m_lod(Terrain::LodDistanceList::static_size - 1)
{
	D3DLOCKED_RECT lrc = m_Owner->m_HeightMap.LockRect(NULL, 0, 0);
	m_aabb.m_min = m_Owner->GetSamplePos(lrc.pBits, lrc.Pitch, (m_Row + 0) * (Terrain::VertexArray2D::static_size - 1), (m_Column + 0) * (Terrain::VertexArray::static_size - 1));
	m_aabb.m_max = m_Owner->GetSamplePos(lrc.pBits, lrc.Pitch, (m_Row + 1) * (Terrain::VertexArray2D::static_size - 1), (m_Column + 1) * (Terrain::VertexArray::static_size - 1));
	m_Owner->m_HeightMap.UnlockRect(0);
}

TerrainChunk::TerrainChunk(void)
	: m_Owner(NULL)
	, m_Row(0)
	, m_Column(0)
	, m_lod(Terrain::LodDistanceList::static_size - 1)
{
	m_aabb = AABB::Invalid();
}

TerrainChunk::~TerrainChunk(void)
{
}

void TerrainChunk::UpdateAABB(void)
{
	m_aabb = AABB::Invalid();
	D3DLOCKED_RECT lrc = m_Owner->m_HeightMap.LockRect(NULL, 0, 0);
	for (unsigned int i = 0; i < Terrain::VertexArray2D::static_size; i++)
	{
		const int row_i = m_Row * (Terrain::VertexArray2D::static_size - 1) + i;
		for (unsigned int j = 0; j < Terrain::VertexArray::static_size; j++)
		{
			const int col_i = m_Column * (Terrain::VertexArray::static_size - 1) + j;
			Vector3 Pos = m_Owner->GetSamplePos(lrc.pBits, lrc.Pitch, row_i, col_i);
			m_aabb.unionSelf(Pos);
		}
	}
	m_Owner->m_HeightMap.UnlockRect(0);
}

template <class T, int N>
unsigned int FillVert(T & tri, int hs, unsigned int k)
{
    _ASSERT((hs & (hs - 1)) == 0);
    for (int i = 0; i < N - 1; i += hs * 2)
    {
        for (int j = 0; j < N - 1; j += hs * 2)
        {
            tri.set(i + hs * 0, j + hs * 1, k++);
            tri.set(i + hs * 1, j + hs * 0, k++);
            tri.set(i + hs * 1, j + hs * 1, k++);
            if (j + hs * 2 >= N - 1)
            {
                tri.set(i + hs * 1, j + hs * 2, k++);
            }
            if (i + hs * 2 >= N - 1)
            {
                tri.set(i + hs * 2, j + hs * 1, k++);
            }
        }
    }

    if (hs / 2 > 0)
    {
        k = FillVert<T, N>(tri, hs / 2, k);
    }
    return k;
}

template <class T, int N>
unsigned int FillVert(T & tri)
{
    BOOST_STATIC_ASSERT(((N - 1) & (N - 2)) == 0);
    unsigned int k = 0;
    tri.set(0, 0, k++);
    tri.set(0, N - 1, k++);
    tri.set(N - 1, 0, k++);
    tri.set(N - 1, N - 1, k++);
    return FillVert<T, N>(tri, (N - 1) / 2, k);
}

Terrain::VertexArray2D::VertexArray2D(void)
{
    struct Tri
    {
        VertexArray2D & verts;
		Tri(VertexArray2D & _verts)
			: verts(_verts)
		{
		}
        void set(int i, int j, int k)
        {
			verts[i][j] = k;
        }
    };
	FillVert<Tri, static_size>(Tri(*this));
}

const Terrain::VertexArray2D Terrain::m_VertTable;

Terrain::Terrain(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, float HeightScale, float WrappedU, float WrappedV)
	: RenderComponent(ComponentTypeTerrain, Position, Rotation, Scale)
	, m_HeightScale(HeightScale)
	, m_WrappedU(WrappedU)
	, m_WrappedV(WrappedV)
	, m_Root(NULL, AABB(Vector3(0,-3000,0), Vector3(ROW_CHUNKS * CHUNK_SIZE, 3000, COL_CHUNKS * CHUNK_SIZE)), 1.0f)
{
	CreateHeightMap();
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			TerrainChunkPtr chunk(new TerrainChunk(this, i, j));
			m_Root.AddActor(chunk, chunk->m_aabb, 0.1f);
			m_Chunks[i][j] = chunk.get();
		}
	}
	CalcLodDistanceSq();
	CreateElements();
}

Terrain::Terrain(void)
	: RenderComponent(ComponentTypeTerrain, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
	, m_HeightScale(1)
	, m_WrappedU(1)
	, m_WrappedV(1)
	, m_Root(NULL, AABB(Vector3(0,-3000,0), Vector3(ROW_CHUNKS * CHUNK_SIZE, 3000, COL_CHUNKS * CHUNK_SIZE)), 1.0f)
{
	CreateHeightMap();
}

Terrain::~Terrain(void)
{
	//m_HeightMap.OnDestroyDevice();
	//_ASSERT(!m_Decl);
	//_ASSERT(!m_vb.m_ptr);
	//m_Root.ClearAllActor();
}

void Terrain::CalcLodDistanceSq(void)
{
	for (unsigned int i = 0; i < LodDistanceList::static_size; i++)
	{
		m_LodDistanceSq[i] = pow(Vector2(CHUNK_SIZE * 0.6f, CHUNK_SIZE * 0.6f).magnitude() * (i + 1), 2);
	}
}

void Terrain::CreateHeightMap(void)
{
	m_HeightMap.CreateTexture(
		my::D3DContext::getSingleton().m_d3dDevice, COL_CHUNKS * CHUNK_SIZE, ROW_CHUNKS * CHUNK_SIZE, 1, 0, D3DFMT_A8R8G8B8);
	UpdateHeightMapNormal();
}

void Terrain::UpdateHeightMap(my::Texture2DPtr HeightMap)
{
	D3DSURFACE_DESC SrcDesc = HeightMap->GetLevelDesc(0);
	switch (SrcDesc.Format)
	{
	case D3DFMT_A8:
	case D3DFMT_L8:
		{
			RECT rc = { 0, 0, my::Min<LONG>(COL_CHUNKS * CHUNK_SIZE, SrcDesc.Width), my::Min<LONG>(ROW_CHUNKS * CHUNK_SIZE, SrcDesc.Height) };
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
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, 0, 0);
	for (int i = 0; i < ROW_CHUNKS * CHUNK_SIZE; i++)
	{
		for (int j = 0; j < COL_CHUNKS * CHUNK_SIZE; j++)
		{
			D3DCOLOR * Bits = (D3DCOLOR *)((unsigned char *)lrc.pBits + i * lrc.Pitch + j * sizeof(D3DCOLOR));
			Vector3 Pos = GetSamplePos(lrc.pBits, lrc.Pitch, i, j);
			const Vector3 Dirs[4] = {
				GetSamplePos(lrc.pBits, lrc.Pitch, i, j - 1) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i - 1, j) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i, j + 1) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i + 1, j) - Pos,
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
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j]->UpdateAABB();
			TerrainChunkPtr chunk = m_Chunks[i][j]->shared_from_this();
			m_Root.RemoveActor(chunk);
			m_Root.AddActor(chunk, chunk->m_aabb, 0.1f);
		}
	}
}

unsigned char Terrain::GetSampleHeight(void * pBits, int pitch, int i, int j)
{
	i = Clamp<int>(i, 0, ROW_CHUNKS * CHUNK_SIZE - 1);
	j = Clamp<int>(j, 0, COL_CHUNKS * CHUNK_SIZE - 1);
	D3DCOLOR * Bits = (D3DCOLOR *)((unsigned char *)pBits + i * pitch + j * sizeof(D3DCOLOR));
	return GetCValue(*Bits);
}

my::Vector3 Terrain::GetSamplePos(void * pBits, int pitch, int i, int j)
{
	return Vector3((float)i, m_HeightScale * GetSampleHeight(pBits, pitch, i, j), (float)j);
}

void Terrain::CreateElements(void)
{
	m_VertexElems.InsertVertexElement(0, D3DDECLTYPE_UBYTE4, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLMETHOD_DEFAULT);
	WORD offset = sizeof(unsigned int);
	m_VertexStride = offset;
}

template <class T>
unsigned int EdgeNv1(T & tri, int N, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        int cb = c0 + cs * i;
		if (i < N - 1)
		{
			tri.set(k + 0, r0, cb);
			tri.set(k + 1, r0, cb + cs);
			tri.set(k + 2, r0 + rs, cb + cs);
			k += 3;

			if (i > 0)
			{
				tri.set(k + 0, r0, cb);
				tri.set(k + 1, r0 + rs, cb + cs);
				tri.set(k + 2, r0 + rs, cb);
				k += 3;
			}
		}
		else
		{
			tri.set(k + 0, r0, cb);
			tri.set(k + 1, r0, cb + cs);
			tri.set(k + 2, r0 + rs, cb);
			k += 3;
		}
	}
	return k;
};

template <class T>
unsigned int EdgeNvM(T & tri, int N, int M, int r0, int rs, int c0, int cs)
{
    _ASSERT((N & (N - 1)) == 0);
    _ASSERT((M & (M - 1)) == 0);
	if (M == 1)
	{
		return EdgeNv1(tri, N, r0, rs, c0, cs);
	}
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        int cb = c0 + cs * i * M;

        tri.set(k + 0, r0, cb);
        tri.set(k + 1, r0, cb + cs * M);
        tri.set(k + 2, r0 + rs, cb + cs * M / 2);
        k += 3;

        int j = 0;
        for (; j < M / 2; j++)
        {
            if (i > 0 || j > 0)
            {
                tri.set(k + 0, r0 + rs, cb + cs * j);
                tri.set(k + 1, r0, cb);
                tri.set(k + 2, r0 + rs, cb + cs * (j + 1));
                k += 3;
            }
        }

        for (; j < M; j++)
        {
            if (i < N - 1 || j < M - 1)
            {
                tri.set(k + 0, r0 + rs, cb + cs * j);
                tri.set(k + 1, r0, cb + cs * M);
                tri.set(k + 2, r0 + rs, cb + cs * (j + 1));
                k += 3;
            }
        }
    }
    return k;
}

template <class T>
unsigned int FillNvM(T & tri, int N, int M, int r0, int rs, int c0, int cs)
{
    unsigned int k = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            tri.set(k + 0, r0 + rs * (i + 0), c0 + cs * (j + 0));
            tri.set(k + 1, r0 + rs * (i + 0), c0 + cs * (j + 1));
            tri.set(k + 2, r0 + rs * (i + 1), c0 + cs * (j + 0));
            k += 3;

            tri.set(k + 0, r0 + rs * (i + 0), c0 + cs * (j + 1));
            tri.set(k + 1, r0 + rs * (i + 1), c0 + cs * (j + 1));
            tri.set(k + 2, r0 + rs * (i + 1), c0 + cs * (j + 0));
            k += 3;
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

    struct Tri
    {
        WORD * buff;
		Tri(WORD * _buff)
			: buff(_buff)
		{
		}
        void set(int k, int i, int j)
        {
			buff[k] = m_VertTable[i][j];
        }
    };

    struct TriTranspose
    {
        WORD * buff;
		TriTranspose(WORD * _buff)
			: buff(_buff)
		{
		}
        void set(int k, int i, int j)
        {
			buff[k] = m_VertTable[j][i];
        }
    };

	IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
	Fragment & frag = m_Fragment[id];
	if (left > center || top > center || right > center || bottom > center)
	{
		const int N[] = {
			CHUNK_SIZE >> center,
			CHUNK_SIZE >> Max(center, left),
			CHUNK_SIZE >> Max(center, top),
			CHUNK_SIZE >> Max(center, right),
			CHUNK_SIZE >> Max(center, bottom)
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
		frag.ib.CreateIndexBuffer(pd3dDevice, frag.PrimitiveCount * 3 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
		VOID * pIndices = frag.ib.Lock(0, 0, 0);
		unsigned int k = 0;
		const int step = 1 << center;
		k += FillNvM(Tri((WORD *)pIndices + k), N[0] - 2, N[0] - 2, 0 + step, step, 0 + step, step);
		k += EdgeNvM(TriTranspose((WORD *)pIndices + k), N[1], M[1], 0, step, N[0] * step, -step);
		k += EdgeNvM(Tri((WORD *)pIndices + k), N[2], M[2], 0, step, 0, step);
		k += EdgeNvM(TriTranspose((WORD *)pIndices + k), N[3], M[3], N[0] * step, -step, 0, step);
		k += EdgeNvM(Tri((WORD *)pIndices + k), N[4], M[4], N[0] * step, -step, N[0] * step, -step);
		_ASSERT(k == frag.PrimitiveCount * 3);
		frag.ib.Unlock();
	}
	else
	{
		const int N = CHUNK_SIZE >> center;
		frag.VertNum = (N + 1) * (N + 1);
		frag.PrimitiveCount = N * N * 2;
		frag.ib.CreateIndexBuffer(pd3dDevice, frag.PrimitiveCount * 3 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
		VOID * pIndices = frag.ib.Lock(0, 0, 0);
		const int step = 1 << center;
		unsigned int k = FillNvM(Tri((WORD *)pIndices), N, N, 0, step, 0, step);
		frag.ib.Unlock();
	}
	return frag;
}

template<>
void Terrain::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar << BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar << BOOST_SERIALIZATION_NVP(m_WrappedU);
	ar << BOOST_SERIALIZATION_NVP(m_WrappedV);
	ar << BOOST_SERIALIZATION_NVP(m_Material);
	const DWORD BufferSize = ROW_CHUNKS * CHUNK_SIZE * COL_CHUNKS * CHUNK_SIZE;
	boost::array<unsigned char, BufferSize> buff;
	D3DLOCKED_RECT lrc = const_cast<my::Texture2D&>(m_HeightMap).LockRect(NULL, 0, 0);
	std::copy(
		boost::make_transform_iterator((unsigned int *)lrc.pBits, boost::lambda::_1 >> 24),
		boost::make_transform_iterator((unsigned int *)lrc.pBits + BufferSize, boost::lambda::_1 >> 24), buff.begin());
	const_cast<my::Texture2D&>(m_HeightMap).UnlockRect(0);
	ar << boost::serialization::make_nvp("HeightMap", boost::serialization::binary_object(&buff[0], buff.size()));
	ar << BOOST_SERIALIZATION_NVP(m_Root);
	ar << BOOST_SERIALIZATION_NVP(m_LodDistanceSq);
}

template<>
void Terrain::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar >> BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar >> BOOST_SERIALIZATION_NVP(m_WrappedU);
	ar >> BOOST_SERIALIZATION_NVP(m_WrappedV);
	ar >> BOOST_SERIALIZATION_NVP(m_Material);
	const DWORD BufferSize = ROW_CHUNKS * CHUNK_SIZE * COL_CHUNKS * CHUNK_SIZE;
	boost::array<unsigned char, BufferSize> buff;
	ar >> boost::serialization::make_nvp("HeightMap", boost::serialization::binary_object(&buff[0], buff.size()));
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, 0, 0);
	std::copy(
		boost::make_transform_iterator(buff.begin(), boost::lambda::ll_static_cast<unsigned int>(boost::lambda::_1) << 24),
		boost::make_transform_iterator(buff.begin() + BufferSize, boost::lambda::ll_static_cast<unsigned int>(boost::lambda::_1) << 24), (unsigned int *)lrc.pBits);
	m_HeightMap.UnlockRect(0);
	ar >> BOOST_SERIALIZATION_NVP(m_Root);
	ar >> BOOST_SERIALIZATION_NVP(m_LodDistanceSq);
	struct Callback : public my::OctNodeBase::QueryCallback
	{
		Terrain * terrain;
		Callback(Terrain * _terrain)
			: terrain(_terrain)
		{
		}
		void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_actor);
			terrain->m_Chunks[chunk->m_Row][chunk->m_Column] = chunk;
			chunk->m_Owner = terrain;
		}
	};
	m_Root.QueryActorAll(&Callback(this));
	UpdateHeightMapNormal();
	CreateElements();
}

void Terrain::RequestResource(void)
{
	RenderComponent::RequestResource();

	_ASSERT(!m_Decl);
	IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
	std::vector<D3DVERTEXELEMENT9> elems = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	elems.push_back(ve_end);
	HRESULT hr;
	V(pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl));

	m_Material->RequestResource();

	CreateVertices();
}

void Terrain::ReleaseResource(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_Fragment.clear();
	m_Material->ReleaseResource();
	RenderComponent::ReleaseResource();
}

void Terrain::CreateVertices(void)
{
	IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
	m_vb.CreateVertexBuffer(pd3dDevice, Terrain::VertexArray2D::static_size * Terrain::VertexArray::static_size * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
	UpdateVertices();
}

void Terrain::UpdateVertices(void)
{
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int i = 0; i < Terrain::VertexArray2D::static_size; i++)
		{
			for (unsigned int j = 0; j < Terrain::VertexArray::static_size; j++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + Terrain::m_VertTable[i][j] * m_VertexStride;
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

void Terrain::UpdateLod(const my::Vector3 & ViewPos)
{
	const Vector3 LocalTargetPos = ViewPos.transform(m_World.inverse()).xyz;
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			float DistanceSq = (m_Chunks[i][j]->m_aabb.Center() - LocalTargetPos).magnitudeSq();
			unsigned char lod = 0;
			for (; lod < LodDistanceList::static_size; lod++)
			{
				if (DistanceSq < m_LodDistanceSq[lod])
				{
					m_Chunks[i][j]->m_lod = lod;
					break;
				}
			}
		}
	}
}

void Terrain::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetFloat("g_Time", (float)D3DContext::getSingleton().m_fAbsoluteTime);

	shader->SetMatrix("g_World", m_World);

	shader->SetFloat("g_HeightScale", m_HeightScale);

	shader->SetVector("g_WrappedUV", Vector4(m_WrappedU, m_WrappedV, (float)ROW_CHUNKS * CHUNK_SIZE, (float)COL_CHUNKS * CHUNK_SIZE));

	int ChunkId[3] = { LOWORD(AttribId), HIWORD(AttribId), CHUNK_SIZE };

	shader->SetIntArray("g_ChunkId", ChunkId, 3);

	shader->SetTexture("g_HeightTexture", &m_HeightMap);

	m_Material->OnSetShader(shader, AttribId);
}

my::AABB Terrain::CalculateAABB(void) const
{
	AABB ret = RenderComponent::CalculateAABB();
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			ret.unionSelf(m_Chunks[i][j]->m_aabb);
		}
	}
	return ret;
}

void Terrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct Callback : public my::OctNodeBase::QueryCallback
	{
		RenderPipeline * pipeline;
		unsigned int PassID;
		Terrain * terrain;
		Effect * shader;
		Callback(RenderPipeline * _pipeline, unsigned int _PassID, Terrain * _terrain, Effect * _shader)
			: pipeline(_pipeline)
			, PassID(_PassID)
			, terrain(_terrain)
			, shader(_shader)
		{
		}
		void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_actor);
			const Fragment & frag = terrain->GetFragment(chunk->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column - 1, 0, Terrain::ChunkArray::static_size - 1)]->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row - 1, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column, 0, Terrain::ChunkArray::static_size - 1)]->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column + 1, 0, Terrain::ChunkArray::static_size - 1)]->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row + 1, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column, 0, Terrain::ChunkArray::static_size - 1)]->m_lod);
			pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, terrain->m_vb.m_ptr,
				frag.ib.m_ptr, D3DPT_TRIANGLELIST, 0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, MAKELONG(chunk->m_Row, chunk->m_Column), shader, terrain);
		}
	};

	if (m_vb.m_ptr)
	{
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, false, m_Material.get(), PassID);
					if (shader)
					{
						// ! do not use m_World for level offset
						Frustum loc_frustum = frustum.transform(CalculateLocal().transpose());
						m_Root.QueryActor(loc_frustum, &Callback(pipeline, PassID, this, shader));
					}
				}
			}
		}
	}
}

void Terrain::CreateHeightFieldShape(const my::Vector3 & Position, const my::Quaternion & Rotation)
{
	_ASSERT(!m_PxShape);

	if (!m_Actor || !m_Actor->m_PxActor)
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

	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, 0, 0);
	std::vector<physx::PxHeightFieldSample> Samples(
		(ROW_CHUNKS * CHUNK_SIZE + 1) * (COL_CHUNKS * CHUNK_SIZE + 1));
	for (unsigned int i = 0; i < ROW_CHUNKS * CHUNK_SIZE + 1; i++)
	{
		for (unsigned int j = 0; j < COL_CHUNKS * CHUNK_SIZE + 1; j++)
		{
			Samples[i * (COL_CHUNKS * CHUNK_SIZE + 1) + j].height = GetSampleHeight(lrc.pBits, lrc.Pitch, i, j);
			Samples[i * (COL_CHUNKS * CHUNK_SIZE + 1) + j].materialIndex0 = physx::PxBitAndByte(0, false);
			Samples[i * (COL_CHUNKS * CHUNK_SIZE + 1) + j].materialIndex1 = physx::PxBitAndByte(0, false);
		}
	}
	m_HeightMap.UnlockRect(0);
	physx::PxHeightFieldDesc hfDesc;
	hfDesc.nbRows             = ROW_CHUNKS * CHUNK_SIZE + 1;
	hfDesc.nbColumns          = COL_CHUNKS * CHUNK_SIZE + 1;
	hfDesc.format             = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.samples.data       = &Samples[0];
	hfDesc.samples.stride     = sizeof(Samples[0]);
	m_PxHeightField.reset(PhysXContext::getSingleton().m_sdk->createHeightField(hfDesc));

	m_PxMaterial.reset(PhysXContext::getSingleton().m_sdk->createMaterial(0.5f, 0.5f, 0.5f));

	m_PxShape.reset(PhysXContext::getSingleton().m_sdk->createShape(
		physx::PxHeightFieldGeometry(m_PxHeightField.get(), physx::PxMeshGeometryFlags(), m_HeightScale * m_Scale.y, m_Scale.x, m_Scale.z),
		*m_PxMaterial, false, physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE));

	m_PxShape->setLocalPose(physx::PxTransform((physx::PxVec3&)Position, (physx::PxQuat&)Rotation));

	m_Actor->m_PxActor->attachShape(*m_PxShape);
}

void Terrain::ClearShape(void)
{
	RenderComponent::ClearShape();

	m_PxHeightField.reset();
}

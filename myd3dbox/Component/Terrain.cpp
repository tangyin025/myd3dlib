#include "StdAfx.h"
#include "Terrain.h"
#include "PhysXContext.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Terrain)

TerrainChunk::TerrainChunk(Terrain * Owner, int Row, int Column)
	: m_Owner(Owner)
	, m_aabb(AABB::Invalid())
	, m_Row(Row)
	, m_Column(Column)
	, m_lod(Terrain::LodDistanceList::static_size - 1)
{
	m_aabb.m_min = m_Owner->GetSamplePos((m_Row + 0) * (Terrain::VertexArray2D::static_size - 1), (m_Column + 0) * (Terrain::VertexArray::static_size - 1));
	m_aabb.m_max = m_Owner->GetSamplePos((m_Row + 1) * (Terrain::VertexArray2D::static_size - 1), (m_Column + 1) * (Terrain::VertexArray::static_size - 1));
}

TerrainChunk::TerrainChunk(void)
	: m_Owner(NULL)
	, m_aabb(AABB::Invalid())
	, m_Row(0)
	, m_Column(0)
	, m_lod(Terrain::LodDistanceList::static_size - 1)
{
}

TerrainChunk::~TerrainChunk(void)
{
}

void TerrainChunk::UpdateVertices(void)
{
	m_aabb = AABB::Invalid();
	for (unsigned int i = 0; i < Terrain::VertexArray2D::static_size; i++)
	{
		const int row_i = m_Row * (Terrain::VertexArray2D::static_size - 1) + i;
		for (unsigned int j = 0; j < Terrain::VertexArray::static_size; j++)
		{
			const int col_i = m_Column * (Terrain::VertexArray::static_size - 1) + j;
			Vector3 Pos = m_Owner->GetSamplePos(row_i, col_i);
			m_aabb.unionSelf(Pos);
		}
	}
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

Terrain::Terrain(const my::Matrix4 & World, float HeightScale, float RowScale, float ColScale, float WrappedU, float WrappedV)
	: RenderComponent(my::AABB(Vector3(0,-1,0), Vector3(m_RowChunks * m_ChunkRows * RowScale, 1, m_ColChunks * m_ChunkRows * ColScale)), ComponentTypeTerrain)
	, m_World(World)
	, m_HeightScale(HeightScale)
	, m_RowScale(RowScale)
	, m_ColScale(ColScale)
	, m_WrappedU(WrappedU)
	, m_WrappedV(WrappedV)
	, m_Root(Vector3(0,-1000,0), Vector3(3000,1000,3000), 1.0f)
{
	CreateHeightMap();
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j].reset(new TerrainChunk(this, i, j));
			m_Root.AddComponent(m_Chunks[i][j].get(), m_Chunks[i][j]->m_aabb, 0.1f);
		}
	}
	CalcLodDistanceSq();
	CreateElements();
	CreateRigidActor(m_World);
	CreateShape();
}

Terrain::Terrain(void)
	: RenderComponent(my::AABB(-FLT_MAX,FLT_MAX), ComponentTypeTerrain)
	, m_HeightScale(1)
	, m_RowScale(1)
	, m_ColScale(1)
	, m_WrappedU(1)
	, m_WrappedV(1)
	, m_Root(Vector3(0,-1000,0), Vector3(3000,1000,3000), 1.0f)
{
	CreateHeightMap();
}

Terrain::~Terrain(void)
{
	m_HeightMap.OnDestroyDevice();
	if (IsRequested())
	{
		ReleaseResource();
	}
	_ASSERT(!m_Decl);
	_ASSERT(!m_vb.m_ptr);
	m_Root.ClearAllComponents();
}

void Terrain::CalcLodDistanceSq(void)
{
	for (unsigned int i = 0; i < LodDistanceList::static_size; i++)
	{
		//m_LodDistanceSq[i] = pow(Vector2(m_ChunkRows * m_RowScale * 0.6f, m_ChunkRows * m_ColScale * 0.6f).magnitude() * (i + 1), 2);
		m_LodDistanceSq[i] = FLT_MAX;
	}
}

void Terrain::CreateHeightMap(void)
{
	m_HeightMap.CreateTexture(
		my::D3DContext::getSingleton().m_d3dDevice, m_ColChunks * m_ChunkRows, m_RowChunks * m_ChunkRows, 1, 0, D3DFMT_A8R8G8B8);
}

void Terrain::UpdateHeightMap(my::Texture2DPtr HeightMap)
{
	D3DSURFACE_DESC SrcDesc = HeightMap->GetLevelDesc(0);
	switch (SrcDesc.Format)
	{
	case D3DFMT_A8:
	case D3DFMT_L8:
		{
			RECT rc = { 0, 0, my::Min<LONG>(m_ColChunks * m_ChunkRows, SrcDesc.Width), my::Min<LONG>(m_RowChunks * m_ChunkRows, SrcDesc.Height) };
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
			m_HeightMap.UnlockRect();
			HeightMap->UnlockRect();
		}
		break;
	default:
		return;
	}
	UpdateChunks();
	UpdateShape();
}

void Terrain::UpdateChunks(void)
{
	m_aabb = AABB::Invalid();
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j]->UpdateVertices();
			m_Root.RemoveComponent(m_Chunks[i][j].get());
			m_Root.AddComponent(m_Chunks[i][j].get(), m_Chunks[i][j]->m_aabb, 0.1f);
			m_aabb.unionSelf(m_Chunks[i][j]->m_aabb);
		}
	}
}

unsigned char Terrain::GetSampleHeight(int row, int col)
{
	_ASSERT(m_HeightMap.m_ptr);
	row = Clamp<int>(row, 0, m_RowChunks * m_ChunkRows - 1);
	col = Clamp<int>(col, 0, m_ColChunks * m_ChunkRows - 1);
	RECT rc = { col, row, col + 1, row + 1 };
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(&rc, D3DLOCK_READONLY, 0);
	unsigned char height = ((unsigned char *)lrc.pBits)[3];
	m_HeightMap.UnlockRect();
	return height;
}

my::Vector3 Terrain::GetSamplePos(int row, int col)
{
	return Vector3(row * m_RowScale, GetSampleHeight(row, col) * m_HeightScale, col * m_ColScale);
}

void Terrain::CreateRigidActor(const my::Matrix4 & World)
{
	my::Vector3 pos, scale; my::Quaternion rot;
	World.Decompose(scale, rot, pos);
	m_RigidActor.reset(PhysXContext::getSingleton().m_sdk->createRigidStatic(PxTransform((PxVec3&)pos, (PxQuat&)rot)));
}

void Terrain::CreateHeightField(void)
{
	std::vector<PxHeightFieldSample> Samples((m_RowChunks * m_ChunkRows + 1) * (m_ColChunks * m_ChunkRows + 1));
	for (unsigned int i = 0; i < m_RowChunks * m_ChunkRows + 1; i++)
	{
		for (unsigned int j = 0; j < m_ColChunks * m_ChunkRows + 1; j++)
		{
			Samples[i * (m_ColChunks * m_ChunkRows + 1) + j].height = GetSampleHeight(i, j);
			Samples[i * (m_ColChunks * m_ChunkRows + 1) + j].materialIndex0 = PxBitAndByte(0, false);
			Samples[i * (m_ColChunks * m_ChunkRows + 1) + j].materialIndex1 = PxBitAndByte(0, false);
		}
	}

	PxHeightFieldDesc hfDesc;
	hfDesc.nbRows             = m_RowChunks * m_ChunkRows + 1;
	hfDesc.nbColumns          = m_ColChunks * m_ChunkRows + 1;
	hfDesc.format             = PxHeightFieldFormat::eS16_TM;
	hfDesc.samples.data       = &Samples[0];
	hfDesc.samples.stride     = sizeof(Samples[0]);
	m_HeightField.reset(PhysXContext::getSingleton().m_sdk->createHeightField(hfDesc));
}

void Terrain::CreateShape(void)
{
	CreateHeightField();

	PxShape * shape = m_RigidActor->createShape(
		PxHeightFieldGeometry(m_HeightField.get(), PxMeshGeometryFlags(), m_HeightScale, m_RowScale, m_ColScale),
		*PhysXContext::getSingleton().m_PxMaterial, PxTransform::createIdentity());
	//shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
}

void Terrain::UpdateShape(void)
{
	CreateHeightField();

	unsigned int NbShapes = m_RigidActor->getNbShapes();
	std::vector<PxShape *> shapes(NbShapes);
	NbShapes = m_RigidActor->getShapes(&shapes[0], shapes.size(), 0);
	shapes[0]->setGeometry(
		PxHeightFieldGeometry(m_HeightField.get(), PxMeshGeometryFlags(), m_HeightScale, m_RowScale, m_ColScale));
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
			m_ChunkRows >> center,
			m_ChunkRows >> Max(center, left),
			m_ChunkRows >> Max(center, top),
			m_ChunkRows >> Max(center, right),
			m_ChunkRows >> Max(center, bottom)
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
		const int N = m_ChunkRows >> center;
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
void Terrain::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar << BOOST_SERIALIZATION_NVP(m_World);
	ar << BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar << BOOST_SERIALIZATION_NVP(m_RowScale);
	ar << BOOST_SERIALIZATION_NVP(m_ColScale);
	ar << BOOST_SERIALIZATION_NVP(m_WrappedU);
	ar << BOOST_SERIALIZATION_NVP(m_WrappedV);
	ar << BOOST_SERIALIZATION_NVP(m_Material);
	const DWORD BufferSize = m_RowChunks * m_ChunkRows * m_ColChunks * m_ChunkRows * sizeof(D3DCOLOR);
	D3DLOCKED_RECT lrc = const_cast<my::Texture2D&>(m_HeightMap).LockRect(NULL, D3DLOCK_READONLY, 0);
	ar << boost::serialization::make_nvp("HeightMap", boost::serialization::binary_object((void *)lrc.pBits, BufferSize));
	const_cast<my::Texture2D&>(m_HeightMap).UnlockRect();
	ar << BOOST_SERIALIZATION_NVP(m_Chunks);
	ar << BOOST_SERIALIZATION_NVP(m_LodDistanceSq);
}

template<>
void Terrain::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderComponent);
	ar >> BOOST_SERIALIZATION_NVP(m_World);
	ar >> BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar >> BOOST_SERIALIZATION_NVP(m_RowScale);
	ar >> BOOST_SERIALIZATION_NVP(m_ColScale);
	ar >> BOOST_SERIALIZATION_NVP(m_WrappedU);
	ar >> BOOST_SERIALIZATION_NVP(m_WrappedV);
	ar >> BOOST_SERIALIZATION_NVP(m_Material);
	Cache cache(m_RowChunks * m_ChunkRows * m_ColChunks * m_ChunkRows * sizeof(D3DCOLOR));
	ar >> boost::serialization::make_nvp("HeightMap", boost::serialization::binary_object((void *)&cache[0], cache.size()));
	const DWORD BufferSize = m_RowChunks * m_ChunkRows * m_ColChunks * m_ChunkRows;
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	memcpy(lrc.pBits, &cache[0], cache.size());
	m_HeightMap.UnlockRect();
	ar >> BOOST_SERIALIZATION_NVP(m_Chunks);
	ar >> BOOST_SERIALIZATION_NVP(m_LodDistanceSq);
	for (unsigned int i = 0; i < ChunkArray2D::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j]->m_Owner = this;
			m_Root.AddComponent(m_Chunks[i][j].get(), m_Chunks[i][j]->m_aabb, 0.1f);
		}
	}
	CreateElements();
	CreateRigidActor(m_World);
	CreateShape();
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

	PhysXSceneContext::getSingleton().m_PxScene->addActor(*m_RigidActor);
}

void Terrain::ReleaseResource(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_Fragment.clear();
	m_Material->ReleaseResource();
	PhysXSceneContext::getSingleton().m_PxScene->removeActor(*m_RigidActor);
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

void Terrain::UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
	const my::Vector3 LocalTargetPos = TargetPos.transform(m_World.inverse()).xyz;
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

	shader->SetVector("g_TerrainScale", Vector3(m_RowScale, m_HeightScale, m_ColScale));

	shader->SetVector("g_WrappedUV", Vector4(m_WrappedU, m_WrappedV, (float)m_RowChunks * m_ChunkRows, (float)m_ColChunks * m_ChunkRows));

	int ChunkId[3] = { LOWORD(AttribId), HIWORD(AttribId), m_ChunkRows };

	shader->SetIntArray("g_ChunkId", ChunkId, 3);

	shader->SetTexture("g_HeightTexture", &m_HeightMap);

	m_Material->OnSetShader(shader, AttribId);
}

void Terrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	struct CallBack : public my::IQueryCallback
	{
		RenderPipeline * pipeline;
		unsigned int PassID;
		Terrain * terrain;
		my::Effect * shader;
		CallBack(RenderPipeline * _pipeline, unsigned int _PassID, Terrain * _terrain, my::Effect * _shader)
			: pipeline(_pipeline)
			, PassID(_PassID)
			, terrain(_terrain)
			, shader(_shader)
		{
		}
		void operator() (my::OctComponent * oct_cmp, my::IntersectionTests::IntersectionType)
		{
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_cmp);
			const Fragment & frag = terrain->GetFragment(chunk->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column - 1, 0, Terrain::ChunkArray::static_size - 1)]->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row - 1, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column, 0, Terrain::ChunkArray::static_size - 1)]->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column + 1, 0, Terrain::ChunkArray::static_size - 1)]->m_lod,
				terrain->m_Chunks[Clamp<int>(chunk->m_Row + 1, 0, Terrain::ChunkArray2D::static_size - 1)][Clamp<int>(chunk->m_Column, 0, Terrain::ChunkArray::static_size - 1)]->m_lod);
			pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, terrain->m_vb.m_ptr,
				frag.ib.m_ptr, D3DPT_TRIANGLELIST, 0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, MAKELONG(chunk->m_Row, chunk->m_Column), shader, terrain);
		}
	};

	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, false, m_Material.get(), PassID);
				if (shader)
				{
					my::Frustum loc_frustum = frustum.transform(m_World.transpose());
					m_Root.QueryComponent(loc_frustum, &CallBack(pipeline, PassID, this, shader));
				}
			}
		}
	}
}

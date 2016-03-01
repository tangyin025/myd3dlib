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

TerrainChunk::TerrainChunk(Terrain * Owner, const my::Vector2 & PosStart, const my::Vector2 & PosEnd, const my::Vector2 & TexStart, const my::Vector2 & TexEnd)
	: m_Owner(Owner)
	, m_aabb(my::AABB(Vector3(PosStart.x, -1, PosStart.y), Vector3(PosEnd.x, 1, PosEnd.y)))
	, m_PosStart(PosStart)
	, m_PosEnd(PosEnd)
	, m_TexStart(TexStart)
	, m_TexEnd(TexEnd)
{
}

TerrainChunk::TerrainChunk(void)
	: m_Owner(NULL)
	, m_aabb(my::AABB(0,1))
	, m_PosStart(0,0)
	, m_PosEnd(1,1)
	, m_TexStart(0,0)
	, m_TexEnd(1,1)
{
}

TerrainChunk::~TerrainChunk(void)
{
	_ASSERT(!m_vb.m_ptr);
}

void TerrainChunk::CreateVertices(void)
{
	IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
	m_vb.CreateVertexBuffer(pd3dDevice, (m_Owner->m_ChunkRows + 1) * (m_Owner->m_ChunkRows + 1) * m_Owner->m_VertexStride, 0, 0, D3DPOOL_MANAGED);
	UpdateVertices();
}

void TerrainChunk::UpdateVertices(void)
{
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int i = 0; i <= m_Owner->m_ChunkRows; i++)
		{
			for (unsigned int j = 0; j <= m_Owner->m_ChunkRows; j++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + Terrain::m_VertTable[i][j] * m_Owner->m_VertexStride;
				Vector3 & Position = m_Owner->m_VertexElems.GetPosition(pVertex);
				Position.x = (i == 0 ? m_PosStart.x : (i == m_Owner->m_ChunkRows ? m_PosEnd.x : my::Lerp(m_PosStart.x, m_PosEnd.x, (float)i / m_Owner->m_ChunkRows)));
				Position.z = (j == 0 ? m_PosStart.y : (j == m_Owner->m_ChunkRows ? m_PosEnd.y : my::Lerp(m_PosStart.y, m_PosEnd.y, (float)j / m_Owner->m_ChunkRows)));
				Position.y = m_Owner->GetSampleHeight(Position.x, Position.z);
				if (Position.y < m_aabb.m_min.y)
				{
					m_aabb.m_min.y = Position.y;
				}
				else if (Position.y > m_aabb.m_max.y)
				{
					m_aabb.m_max.y = Position.y;
				}

				Vector3 & Normal = m_Owner->m_VertexElems.GetNormal(pVertex);
				const Vector3 Dirs[4] = {
					Vector3(Position.x, m_Owner->GetSampleHeight(Position.x, Position.z - m_Owner->m_ColScale), Position.z - m_Owner->m_ColScale) - Position,
					Vector3(Position.x + m_Owner->m_RowScale, m_Owner->GetSampleHeight(Position.x + m_Owner->m_RowScale, Position.z), Position.z) - Position,
					Vector3(Position.x, m_Owner->GetSampleHeight(Position.x, Position.z + m_Owner->m_ColScale), Position.z + m_Owner->m_ColScale) - Position,
					Vector3(Position.x - m_Owner->m_RowScale, m_Owner->GetSampleHeight(Position.x - m_Owner->m_RowScale, Position.z), Position.z) - Position,
				};
				const Vector3 Nors[4] = {
					Dirs[1].cross(Dirs[0]).normalize(),
					Dirs[2].cross(Dirs[1]).normalize(),
					Dirs[3].cross(Dirs[2]).normalize(),
					Dirs[0].cross(Dirs[3]).normalize(),
				};
				Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();

				Vector3 & Tangent = m_Owner->m_VertexElems.GetTangent(pVertex);
				Tangent = Normal.cross(Dirs[2]).normalize();

				Vector2 & Texcoord = m_Owner->m_VertexElems.GetTexcoord(pVertex, 0);
				Texcoord.x = (i == 0 ? m_TexStart.x : (i == m_Owner->m_ChunkRows ? m_TexEnd.x : my::Lerp(m_TexStart.x, m_TexEnd.x, (float)i / m_Owner->m_ChunkRows)));
				Texcoord.y = (j == 0 ? m_TexStart.y : (j == m_Owner->m_ChunkRows ? m_TexEnd.y : my::Lerp(m_TexStart.y, m_TexEnd.y, (float)j / m_Owner->m_ChunkRows)));
			}
		}
		m_vb.Unlock();
	}
}

void TerrainChunk::DestroyVertices(void)
{
	m_vb.OnDestroyDevice();
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
	: RenderComponent(my::AABB(Vector3(0,-1,0), Vector3(m_RowChunks * m_ChunkRows * RowScale, 1, m_RowChunks * m_ChunkRows * ColScale)), ComponentTypeTerrain)
	, m_World(World)
	, m_HeightScale(HeightScale)
	, m_RowScale(RowScale)
	, m_ColScale(ColScale)
	, m_WrappedU(WrappedU)
	, m_WrappedV(WrappedV)
	, m_Root(Vector3(0,-1000,0), Vector3(m_RowChunks * m_ChunkRows * RowScale, 1000, m_RowChunks * m_ChunkRows * ColScale), 1.0f)
{
	memset(&m_Samples, 0, sizeof(m_Samples));
	for (unsigned int i = 0; i < ChunkArray::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j].reset(new TerrainChunk(this,
				my::Vector2((i + 0) * m_ChunkRows * m_RowScale, (j + 0) * m_ChunkRows * m_ColScale),
				my::Vector2((i + 1) * m_ChunkRows * m_RowScale, (j + 1) * m_ChunkRows * m_ColScale),
				my::Vector2(0,0), my::Vector2(m_WrappedU,m_WrappedV)));
			m_Root.AddComponent(m_Chunks[i][j].get(), m_Chunks[i][j]->m_aabb, 0.1f);
		}
	}
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
	, m_Root(Vector3(0,-1000,0), Vector3(2000,1000,2000), 1.0f)
{
}

Terrain::~Terrain(void)
{
	if (IsRequested())
	{
		ReleaseResource();
	}
	_ASSERT(!m_Decl);
	m_Root.ClearAllComponents();
}

void Terrain::UpdateSamples(my::Texture2DPtr HeightMap)
{
	D3DSURFACE_DESC desc = HeightMap->GetLevelDesc(0);
	if (desc.Format == D3DFMT_L8 && desc.Width >= SampleArray::static_size && desc.Height >= SampleArray::static_size)
	{
		D3DLOCKED_RECT lrc = HeightMap->LockRect(NULL, D3DLOCK_READONLY, 0);
		for (unsigned int i = 0; i < SampleArray::static_size; i++)
		{
			for (unsigned int j = 0; j < SampleArray::static_size; j++)
			{
				m_Samples[i][j] = *((unsigned char *)lrc.pBits + j * lrc.Pitch + i);
			}
		}
		HeightMap->UnlockRect(0);
	}
	UpdateChunks();
	UpdateShape();
}

void Terrain::UpdateChunks(void)
{
	m_aabb = AABB(-1,1);
	for (unsigned int i = 0; i < ChunkArray::static_size; i++)
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

float Terrain::GetSampleHeight(float x, float z)
{
	int i = Clamp<int>((int)(x / m_RowScale), 0, SampleArray::static_size - 1);
	int j = Clamp<int>((int)(z / m_ColScale), 0, SampleArray::static_size - 1);
	return m_Samples[i][j] * m_HeightScale;
}

void Terrain::CreateRigidActor(const my::Matrix4 & World)
{
	my::Vector3 pos, scale; my::Quaternion rot;
	World.Decompose(scale, rot, pos);
	m_RigidActor.reset(PhysXContext::getSingleton().m_sdk->createRigidStatic(PxTransform((PxVec3&)pos, (PxQuat&)rot)));
}

void Terrain::CreateHeightField(void)
{
	std::vector<PxHeightFieldSample> Samples(SampleArray::static_size * SampleArray::static_size);
	for (unsigned int i = 0; i < SampleArray::static_size; i++)
	{
		for (unsigned int j = 0; j < SampleArray::static_size; j++)
		{
			Samples[i * SampleArray::static_size + j].height = m_Samples[i][j];
			Samples[i * SampleArray::static_size + j].materialIndex0 = PxBitAndByte(0, false);
			Samples[i * SampleArray::static_size + j].materialIndex1 = PxBitAndByte(0, false);
		}
	}

	PxHeightFieldDesc hfDesc;
	hfDesc.format             = PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns          = SampleArray::static_size;
	hfDesc.nbRows             = SampleArray::static_size;
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
	shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
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
	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_VertexElems.InsertNormalElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTangentElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTexcoordElement(offset, 0);
	offset += sizeof(Vector2);
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
	ar << boost::serialization::make_nvp("Samples", boost::serialization::binary_object((void *)&m_Samples, sizeof(m_Samples)));
	ar << BOOST_SERIALIZATION_NVP(m_Chunks);
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
	ar >> boost::serialization::make_nvp("Samples", boost::serialization::binary_object((void *)&m_Samples, sizeof(m_Samples)));
	ar >> BOOST_SERIALIZATION_NVP(m_Chunks);
	for (unsigned int i = 0; i < ChunkArray::static_size; i++)
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

	for (unsigned int i = 0; i < ChunkArray::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j]->CreateVertices();
		}
	}

	PhysXSceneContext::getSingleton().m_PxScene->addActor(*m_RigidActor);
}

void Terrain::ReleaseResource(void)
{
	m_Decl.Release();
	m_Fragment.clear();
	m_Material->ReleaseResource();
	for (unsigned int i = 0; i < ChunkArray::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j]->DestroyVertices();
		}
	}
	PhysXSceneContext::getSingleton().m_PxScene->removeActor(*m_RigidActor);
	RenderComponent::ReleaseResource();
}

void Terrain::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetFloat("g_Time", (float)D3DContext::getSingleton().m_fAbsoluteTime);

	shader->SetMatrix("g_World", m_World);

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
			unsigned char center=0, left=0, top=0, right=0, bottom=0;
			const Fragment & frag = terrain->GetFragment(center, left, top, right, bottom);
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_cmp);
			pipeline->PushIndexedPrimitive(PassID, terrain->m_Decl, chunk->m_vb.m_ptr,
				frag.ib.m_ptr, D3DPT_TRIANGLELIST, 0, 0, frag.VertNum, terrain->m_VertexStride, 0, frag.PrimitiveCount, 0, shader, terrain);
		}
	};

	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeStatic, false, m_Material.get(), PassID);
				if (shader)
				{
					my::Frustum loc_frustum = frustum.transform(m_World.transpose());
					m_Root.QueryComponent(loc_frustum, &CallBack(pipeline, PassID, this, shader));
				}
			}
		}
	}
}

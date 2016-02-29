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

template <int N>
unsigned int FillVert(Terrain::VertTable2D & verts, int hs, unsigned int k)
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
        k = FillVert<N>(verts, hs / 2, k);
    }
    return k;
}

template <int N>
unsigned int FillVert(Terrain::VertTable2D & verts)
{
    BOOST_STATIC_ASSERT(((N - 1) & (N - 2)) == 0);
    unsigned int k = 0;
    verts[0][0] = k++;
    verts[0][N - 1] = k++;
    verts[N - 1][0] = k++;
    verts[N - 1][N - 1] = k++;
    return FillVert<N>(verts, (N - 1) / 2, k);
}

Terrain::VertTable2D::VertTable2D(void)
{
	FillVert<static_size>(*this);
}

const Terrain::VertTable2D Terrain::m_VertTable;

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
	CreateElements();
	CreateChunks();
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
	_ASSERT(!m_ib.m_ptr);
	m_Root.ClearAllComponents();
}

void Terrain::UpdateSamples(my::Texture2DPtr HeightMap)
{
	D3DSURFACE_DESC desc = HeightMap->GetLevelDesc(0);
	if (desc.Format == D3DFMT_L8 && desc.Width >= Sample::static_size && desc.Height >= Sample::static_size)
	{
		D3DLOCKED_RECT lrc = HeightMap->LockRect(NULL, D3DLOCK_READONLY, 0);
		for (unsigned int i = 0; i < Sample::static_size; i++)
		{
			for (unsigned int j = 0; j < Sample::static_size; j++)
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
	int i = Clamp<int>((int)(x / m_RowScale), 0, Sample::static_size - 1);
	int j = Clamp<int>((int)(z / m_ColScale), 0, Sample::static_size - 1);
	return m_Samples[i][j] * m_HeightScale;
}

void Terrain::CreateChunks(void)
{
	for (unsigned int i = 0; i < ChunkArray::static_size; i++)
	{
		for (unsigned int j = 0; j < ChunkArray::static_size; j++)
		{
			m_Chunks[i][j].reset(new TerrainChunk(this,
				my::Vector2((i + 0) * m_ChunkRows * m_RowScale, (j + 0) * m_ChunkRows * m_ColScale),
				my::Vector2((i + 1) * m_ChunkRows * m_RowScale, (j + 1) * m_ChunkRows * m_ColScale),
				my::Vector2(0,0), my::Vector2(m_WrappedU,m_WrappedV)));
			m_Root.AddComponent(m_Chunks[i][j].get(), m_Chunks[i][j]->m_aabb, 0.1f);
			m_aabb.unionSelf(m_Chunks[i][j]->m_aabb);
		}
	}
}

void Terrain::CreateRigidActor(const my::Matrix4 & World)
{
	my::Vector3 pos, scale; my::Quaternion rot;
	World.Decompose(scale, rot, pos);
	m_RigidActor.reset(PhysXContext::getSingleton().m_sdk->createRigidStatic(PxTransform((PxVec3&)pos, (PxQuat&)rot)));
}

void Terrain::CreateHeightField(void)
{
	std::vector<PxHeightFieldSample> Samples(Sample::static_size * Sample::static_size);
	for (unsigned int i = 0; i < Sample::static_size; i++)
	{
		for (unsigned int j = 0; j < Sample::static_size; j++)
		{
			Samples[i * Sample::static_size + j].height = m_Samples[i][j];
			Samples[i * Sample::static_size + j].materialIndex0 = PxBitAndByte(0, false);
			Samples[i * Sample::static_size + j].materialIndex1 = PxBitAndByte(0, false);
		}
	}

	PxHeightFieldDesc hfDesc;
	hfDesc.format             = PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns          = Sample::static_size;
	hfDesc.nbRows             = Sample::static_size;
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
	CreateElements();
	CreateChunks();
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

	m_ib.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * m_ChunkRows * m_ChunkRows * 6, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
	VOID * pIndices = m_ib.Lock(0, 0, 0);
	if (pIndices)
	{
		for (unsigned int i = 0; i < m_ChunkRows; i++)
		{
			for (unsigned int j = 0; j < m_ChunkRows; j++)
			{
				int index = (i * m_ChunkRows + j) * 6;
				*((WORD *)pIndices + index + 0) = (WORD)m_VertTable[i + 0][j + 0];
				*((WORD *)pIndices + index + 1) = (WORD)m_VertTable[i + 0][j + 1];
				*((WORD *)pIndices + index + 2) = (WORD)m_VertTable[i + 1][j + 0];

				*((WORD *)pIndices + index + 3) = (WORD)m_VertTable[i + 0][j + 1];
				*((WORD *)pIndices + index + 4) = (WORD)m_VertTable[i + 1][j + 1];
				*((WORD *)pIndices + index + 5) = (WORD)m_VertTable[i + 1][j + 0];
			}
		}
		m_ib.Unlock();
	}

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
	m_ib.OnDestroyDevice();
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
			TerrainChunk * chunk = dynamic_cast<TerrainChunk *>(oct_cmp);
			pipeline->PushIndexedPrimitive(
				PassID,
				terrain->m_Decl,
				chunk->m_vb.m_ptr,
				terrain->m_ib.m_ptr,
				D3DPT_TRIANGLELIST, 0, 0,
				(terrain->m_ChunkRows + 1) * (terrain->m_ChunkRows + 1),
				terrain->m_VertexStride, 0,
				terrain->m_ChunkRows * terrain->m_ChunkRows * 2, 0, shader, terrain);
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

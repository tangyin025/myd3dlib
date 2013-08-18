#include "StdAfx.h"
#include "ApexRenderResourceMgr.h"
#include "Game.h"
#include "GameState.h"
#include <rapidxml.hpp>
#include <PxMat34Legacy.h>

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

ApexResourceCallback::ApexResourceCallback(void)
{
}

ApexResourceCallback::~ApexResourceCallback(void)
{
}

void* ApexResourceCallback::requestResource(const char* nameSpace, const char* name)
{
	if(0 == strcmp(nameSpace, "ApexMaterials"))
	{
		return new my::MaterialPtr(Game::getSingleton().LoadMaterial(name));
	}
	return NULL;
}

void  ApexResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{
	if(0 == strcmp(nameSpace, "ApexMaterials"))
	{
		delete static_cast<my::MaterialPtr *>(resource);
	}
}

ApexRenderResourceMgr::ApexRenderResourceMgr(void)
{
}

ApexRenderResourceMgr::~ApexRenderResourceMgr(void)
{
}

physx::apex::NxUserRenderVertexBuffer* ApexRenderResourceMgr::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	return new ApexRenderVertexBuffer(Game::getSingleton().GetD3D9Device(), desc);
}

void ApexRenderResourceMgr::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	delete &buffer;
}

physx::apex::NxUserRenderIndexBuffer* ApexRenderResourceMgr::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	return new ApexRenderIndexBuffer(Game::getSingleton().GetD3D9Device(), desc);
}

void ApexRenderResourceMgr::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
	delete &buffer;
}

physx::apex::NxUserRenderBoneBuffer* ApexRenderResourceMgr::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	return new ApexRenderBoneBuffer(Game::getSingleton().GetD3D9Device(), desc);
}

void ApexRenderResourceMgr::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
	delete &buffer;
}

physx::apex::NxUserRenderInstanceBuffer* ApexRenderResourceMgr::createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc)
{
	return NULL;
}

void ApexRenderResourceMgr::releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer)
{
}

physx::apex::NxUserRenderSpriteBuffer* ApexRenderResourceMgr::createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc)
{
	return NULL;
}

void ApexRenderResourceMgr::releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer)
{
}

physx::apex::NxUserRenderResource* ApexRenderResourceMgr::createResource(const physx::apex::NxUserRenderResourceDesc& desc)
{
	return new ApexRenderResource(Game::getSingleton().GetD3D9Device(), desc);
}

void ApexRenderResourceMgr::releaseResource(physx::apex::NxUserRenderResource& resource)
{
	delete &resource;
}

physx::PxU32 ApexRenderResourceMgr::getMaxBonesForMaterial(void* material)
{
	return 60;
}

ApexRenderer::ApexRenderer(void)
{
}

ApexRenderer::~ApexRenderer(void)
{
}

void ApexRenderer::renderResource(const physx::apex::NxApexRenderContext& context)
{
	static_cast<ApexRenderResource *>(context.renderResource)->Draw(
		Game::getSingleton().GetD3D9Device(), (my::Matrix4 &)context.local2world);
}

ApexRenderVertexBuffer::ApexRenderVertexBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	WORD offset = 0;
	for(unsigned int i = 0; i < physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		if(desc.buffersRequest[i] != physx::apex::NxRenderDataFormat::UNSPECIFIED)
		{
			switch(physx::apex::NxRenderVertexSemantic::Enum(i))
			{
			case physx::apex::NxRenderVertexSemantic::POSITION:
				m_VertexElems.InsertPositionElement(offset);
				offset += sizeof(my::Vector3);
				break;
			case physx::apex::NxRenderVertexSemantic::NORMAL:
				m_VertexElems.InsertNormalElement(offset);
				offset += sizeof(my::Vector3);
				break;
			case physx::apex::NxRenderVertexSemantic::TANGENT:
				m_VertexElems.InsertTangentElement(offset);
				offset += sizeof(my::Vector3);
				break;
			case physx::apex::NxRenderVertexSemantic::COLOR:
				m_VertexElems.InsertColorElement(offset);
				offset += sizeof(D3DCOLOR);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD0:
				m_VertexElems.InsertTexcoordElement(offset, 0);
				offset += sizeof(my::Vector2);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD1:
				m_VertexElems.InsertTexcoordElement(offset, 1);
				offset += sizeof(my::Vector2);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD2:
				m_VertexElems.InsertTexcoordElement(offset, 2);
				offset += sizeof(my::Vector2);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD3:
				m_VertexElems.InsertTexcoordElement(offset, 3);
				offset += sizeof(my::Vector2);
				break;
			case physx::apex::NxRenderVertexSemantic::BONE_INDEX:
				m_VertexElems.InsertBlendIndicesElement(offset);
				offset += sizeof(DWORD);
				break;
			case physx::apex::NxRenderVertexSemantic::BONE_WEIGHT:
				m_VertexElems.InsertBlendWeightElement(offset);
				offset += sizeof(my::Vector4);
				break;
			}
		}
	}
	// ! ignore DISPLACEMENT_TEXCOORD, DISPLACEMENT_FLAGS, desc.hint
	std::vector<D3DVERTEXELEMENT9> velist = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);
	m_stride = D3DXGetDeclVertexSize(&velist[0], 0);
	m_vb.CreateVertexBuffer(pd3dDevice, m_stride * desc.maxVerts, 0, 0, D3DPOOL_MANAGED);
}

ApexRenderVertexBuffer::~ApexRenderVertexBuffer(void)
{
}

void ApexRenderVertexBuffer::writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices)
{
	unsigned char * pVertices = (unsigned char *)m_vb.Lock(m_stride * firstVertex, m_stride * numVertices);
	for(unsigned int i = 0; i < physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		physx::apex::NxRenderVertexSemantic::Enum apexSemantic = (physx::apex::NxRenderVertexSemantic::Enum)i;
		const physx::apex::NxApexRenderSemanticData & semanticData = data.getSemanticData(apexSemantic);
		if(semanticData.data)
		{
			for(unsigned int j = 0; j < numVertices; j++)
			{
				switch(apexSemantic)
				{
				case physx::apex::NxRenderVertexSemantic::POSITION:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT3);
					m_VertexElems.SetPosition(pVertices + m_stride * j,
						*(my::Vector3 *)((unsigned char *)semanticData.data + semanticData.stride * j));
					break;
				case physx::apex::NxRenderVertexSemantic::NORMAL:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT3);
					m_VertexElems.SetNormal(pVertices + m_stride * j,
						*(my::Vector3 *)((unsigned char *)semanticData.data + semanticData.stride * j));
					break;
				case physx::apex::NxRenderVertexSemantic::TANGENT:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT3);
					m_VertexElems.SetTangent(pVertices + m_stride * j,
						*(my::Vector3 *)((unsigned char *)semanticData.data + semanticData.stride * j));
					break;
				case physx::apex::NxRenderVertexSemantic::COLOR:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::UBYTE4);
					m_VertexElems.SetColor(pVertices + m_stride * j,
						*(DWORD *)((unsigned char *)semanticData.data + semanticData.stride * j));
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD0:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_VertexElems.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 0);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD1:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_VertexElems.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 1);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD2:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_VertexElems.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 2);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD3:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_VertexElems.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 3);
					break;
				case physx::apex::NxRenderVertexSemantic::BONE_INDEX:
					switch(semanticData.format)
					{
					case physx::apex::NxRenderDataFormat::USHORT1:
						m_VertexElems.SetBlendIndices(pVertices + m_stride * j,
							*(unsigned short *)((unsigned char *)semanticData.data + semanticData.stride * j));
						break;
					default:
						_ASSERT(false);
						break;
					}
					break;
				case physx::apex::NxRenderVertexSemantic::BONE_WEIGHT:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT4);
					m_VertexElems.SetBlendWeight(pVertices + m_stride * j,
						*(my::Vector4 *)((unsigned char *)semanticData.data + semanticData.stride * j));
					break;
				}
			}
		}
	}
	m_vb.Unlock();
}

ApexRenderIndexBuffer::ApexRenderIndexBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderIndexBufferDesc&desc)
{
	_ASSERT(desc.format == physx::apex::NxRenderDataFormat::UINT1);
	m_ib.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * desc.maxIndices, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
}

ApexRenderIndexBuffer::~ApexRenderIndexBuffer(void)
{
}

void ApexRenderIndexBuffer::writeBuffer(const void* srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
{
	WORD * pIndices = (WORD *)m_ib.Lock(sizeof(WORD) * firstDestElement, sizeof(WORD) * numElements, 0);
	for(unsigned int j = 0; j < numElements; j++)
	{
		pIndices[j] = *(unsigned int *)((unsigned char *)srcData + srcStride * j);
	}
	m_ib.Unlock();
}

ApexRenderBoneBuffer::ApexRenderBoneBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	m_bones.resize(desc.maxBones, my::Matrix4::identity);
}

ApexRenderBoneBuffer::~ApexRenderBoneBuffer(void)
{
}

void ApexRenderBoneBuffer::writeBuffer(const physx::NxApexRenderBoneBufferData& data, physx::PxU32 firstBone, physx::PxU32 numBones)
{
	const physx::apex::NxApexRenderSemanticData & semanticData = data.getSemanticData(physx::apex::NxRenderBoneSemantic::POSE);
	if(semanticData.data)
	{
		for(unsigned int j = 0; j < numBones; j++)
		{
			m_bones[firstBone + j] = (my::Matrix4 &)physx::PxMat44(
				*(const physx::general_shared3::PxMat34Legacy *)((unsigned char *)semanticData.data + j * semanticData.stride));
		}
	}
}

ApexRenderResource::ApexRenderResource(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderResourceDesc& desc)
{
	m_ApexVbs.resize(desc.numVertexBuffers);
	std::vector<D3DVERTEXELEMENT9> velist;
	for(WORD i = 0; i < desc.numVertexBuffers; i++)
	{
		ApexRenderVertexBuffer * pVb = static_cast<ApexRenderVertexBuffer *>(desc.vertexBuffers[i]);
		m_ApexVbs[i] = pVb;
		std::vector<D3DVERTEXELEMENT9> ve = pVb->m_VertexElems.BuildVertexElementList(i);
		velist.insert(velist.end(), ve.begin(), ve.end());
	}
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	velist.push_back(ve_end);

	m_firstVertex = desc.firstVertex;

	m_numVerts = desc.numVerts;

	HRESULT hr;
	if(FAILED(hr = pd3dDevice->CreateVertexDeclaration(&velist[0], &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}

	m_ApexIb = static_cast<ApexRenderIndexBuffer *>(desc.indexBuffer);

	m_firstIndex = desc.firstIndex;

	m_numIndices = desc.numVerts;

	m_ApexBb = static_cast<ApexRenderBoneBuffer *>(desc.boneBuffer);

	m_firstBone = desc.firstBone;

	m_numBones = desc.numBones;

	m_material = NULL;
}

ApexRenderResource::~ApexRenderResource(void)
{
}

void ApexRenderResource::Draw(IDirect3DDevice9 * pd3dDevice, const my::Matrix4 & World)
{
	(*m_material)[0].first->SetMatrix("g_World", World);
	(*m_material)[0].first->SetMatrixArray("g_BoneMatrices", &m_ApexBb->m_bones[0], m_numBones);
	m_material->ApplyParameterBlock(0);

	HRESULT hr;
	UINT cPasses = m_material->Begin(0);
	for(UINT p = 0; p < cPasses; p++)
	{
		m_material->BeginPass(0, p);
		V(pd3dDevice->SetVertexDeclaration(m_Decl));
		for(size_t i = 0; i < m_ApexVbs.size(); i++)
		{
			V(pd3dDevice->SetStreamSource(i, m_ApexVbs[i]->m_vb.m_ptr, 0, m_ApexVbs[i]->m_stride));
		}
		V(pd3dDevice->SetIndices(m_ApexIb->m_ib.m_ptr));
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_firstVertex, m_numVerts, m_firstIndex, m_numIndices / 3));
		m_material->EndPass(0);
	}
	m_material->End(0);
}

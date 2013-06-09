#include "StdAfx.h"
#include "ApexRenderResourceMgr.h"
#include "Game.h"
#include "GameState.h"
#include <rapidxml.hpp>

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
		std::string full_path = Game::getSingleton().GetFullPath(name);
		if(full_path.empty())
		{
			THROW_CUSEXCEPTION("empty material path");
		}

		my::CachePtr cache = Game::getSingleton().OpenArchiveStream(full_path)->GetWholeCache();
		std::string xmlStr((char *)&(*cache)[0], cache->size());
		rapidxml::xml_document<char> doc;
		try
		{
			doc.parse<0>(&xmlStr[0]);
		}
		catch (rapidxml::parse_error & e)
		{
			THROW_CUSEXCEPTION(e.what());
		}

		rapidxml::xml_node<char> * node_root = &doc;
		DEFINE_XML_NODE_SIMPLE(material, root);
		DEFINE_XML_NODE_SIMPLE(variables, material);
		DEFINE_XML_NODE_SIMPLE(sampler2D, variables);
		std::string tex_path(node_sampler2D->value());

		my::Material * mat = new my::Material;
		mat->m_Effect = Game::getSingleton().LoadEffect("shader/ApexEffect.fx");
		mat->SetVector("g_MaterialAmbientColor", my::Vector4(0.1f,0.1f,0.1f,0.1f));
		mat->SetVector("g_MaterialDiffuseColor", my::Vector4(1.0f,1.0f,1.0f,1.0f));
		mat->SetTexture("g_MeshTexture", Game::getSingleton().LoadTexture(tex_path));
		return mat;
	}
	return NULL;
}

void  ApexResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{
	if(0 == strcmp(nameSpace, "ApexMaterials"))
	{
		delete static_cast<my::Material *>(resource);
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
	ApexRenderResource * renderResource = static_cast<ApexRenderResource *>(context.renderResource);
	_ASSERT(renderResource->m_material);
	IDirect3DDevice9 * pd3dDevice = Game::getSingleton().GetD3D9Device();
	GameStateMain * state = static_cast<GameStateMain *>(Game::getSingleton().CurrentState());
	renderResource->m_material->m_Effect->SetMatrixArray("g_BoneMatrices", &renderResource->m_ApexBb->m_bones[0], renderResource->m_ApexBb->m_bones.size());
	renderResource->m_material->ApplyParameterBlock();
	HRESULT hr;
	UINT cPasses = renderResource->m_material->m_Effect->Begin();
	for(UINT p = 0; p < cPasses; p++)
	{
		renderResource->m_material->m_Effect->BeginPass(p);
		pd3dDevice->SetVertexDeclaration(renderResource->m_Decl);
		for(WORD i = 0; i < renderResource->m_ApexVbs.size(); i++)
		{
			_ASSERT(renderResource->m_ApexVbs[i]->m_vb.m_ptr);
			V(pd3dDevice->SetStreamSource(i, renderResource->m_ApexVbs[i]->m_vb.m_ptr, 0, renderResource->m_ApexVbs[i]->m_stride));
		}
		_ASSERT(renderResource->m_ApexIb->m_ib.m_ptr);
		V(pd3dDevice->SetIndices(renderResource->m_ApexIb->m_ib.m_ptr));
		V(pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, renderResource->m_firstVertex, renderResource->m_numVerts, renderResource->m_firstIndex, renderResource->m_numIndices / 3));
		renderResource->m_material->m_Effect->EndPass();
	}
	renderResource->m_material->m_Effect->End();
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
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreatePositionElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::PositionType);
				break;
			case physx::apex::NxRenderVertexSemantic::NORMAL:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateNormalElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::NormalType);
				break;
			case physx::apex::NxRenderVertexSemantic::TANGENT:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTangentElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::TangentType);
				break;
			case physx::apex::NxRenderVertexSemantic::COLOR:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateColorElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::ColorType);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD0:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::TexcoordType);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD1:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 1));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::TexcoordType);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD2:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 2));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::TexcoordType);
				break;
			case physx::apex::NxRenderVertexSemantic::TEXCOORD3:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 3));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::TexcoordType);
				break;
			case physx::apex::NxRenderVertexSemantic::BONE_INDEX:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateBlendIndicesElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::BlendIndicesType);
				break;
			case physx::apex::NxRenderVertexSemantic::BONE_WEIGHT:
				m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateBlendWeightsElement(0, offset, 0));
				offset += sizeof(my::D3DVERTEXELEMENT9Set::BlendWeightsType);
				break;
			}
		}
	}
	// ! ignore DISPLACEMENT_TEXCOORD, DISPLACEMENT_FLAGS, desc.hint
	m_stride = m_ve.CalculateVertexStride();
	m_vb.CreateVertexBuffer(pd3dDevice, m_stride * desc.maxVerts, 0, 0, D3DPOOL_MANAGED);
}

ApexRenderVertexBuffer::~ApexRenderVertexBuffer(void)
{
}

void ApexRenderVertexBuffer::writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices)
{
	unsigned short bone_index;
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
					m_ve.SetPosition(pVertices + m_stride * j, *(my::Vector3 *)(
						(unsigned char *)semanticData.data + semanticData.stride * j), 0, 0);
					break;
				case physx::apex::NxRenderVertexSemantic::NORMAL:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT3);
					m_ve.SetNormal(pVertices + m_stride * j, *(my::Vector3 *)(
						(unsigned char *)semanticData.data + semanticData.stride * j), 0, 0);
					break;
				case physx::apex::NxRenderVertexSemantic::TANGENT:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT3);
					m_ve.SetTangent(pVertices + m_stride * j, *(my::Vector3 *)(
						(unsigned char *)semanticData.data + semanticData.stride * j), 0, 0);
					break;
				case physx::apex::NxRenderVertexSemantic::COLOR:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::UBYTE4);
					m_ve.SetColor(pVertices + m_stride * j, *(DWORD *)(
						(unsigned char *)semanticData.data + semanticData.stride * j), 0, 0);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD0:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_ve.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 0, 0);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD1:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_ve.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 0, 1);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD2:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_ve.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 0, 2);
					break;
				case physx::apex::NxRenderVertexSemantic::TEXCOORD3:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT2);
					m_ve.SetTexcoord(pVertices + m_stride * j,
						*(my::Vector2 *)((unsigned char *)semanticData.data + semanticData.stride * j), 0, 3);
					break;
				case physx::apex::NxRenderVertexSemantic::BONE_INDEX:
					switch(semanticData.format)
					{
					case physx::apex::NxRenderDataFormat::USHORT1:
						bone_index = *(unsigned short *)((unsigned char *)semanticData.data + semanticData.stride * j);
						m_ve.SetBlendIndices(pVertices + m_stride * j, D3DCOLOR_ARGB(0,0,0,bone_index), 0, 0);
						break;
					default:
						_ASSERT(false);
						break;
					}
					break;
				case physx::apex::NxRenderVertexSemantic::BONE_WEIGHT:
					_ASSERT(semanticData.format == physx::apex::NxRenderDataFormat::FLOAT4);
					m_ve.SetBlendWeights(pVertices + m_stride * j, *(my::Vector4 *)(
						(unsigned char *)semanticData.data + semanticData.stride * j), 0, 0);
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
	m_ib.CreateIndexBuffer(pd3dDevice, sizeof(DWORD) * desc.maxIndices, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
}

ApexRenderIndexBuffer::~ApexRenderIndexBuffer(void)
{
}

void ApexRenderIndexBuffer::writeBuffer(const void* srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
{
	DWORD * pIndices = (DWORD *)m_ib.Lock(sizeof(DWORD) * firstDestElement, sizeof(DWORD) * numElements, 0);
	for(unsigned int i = 0; i < numElements; i++)
	{
		pIndices[i] = *(unsigned int *)((unsigned char *)srcData + srcStride * i);
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
		for(unsigned int i = 0; i < numBones; i++)
		{
			m_bones[firstBone + i] = (my::Matrix4 &)physx::PxMat44(
				*(const physx::general_shared3::PxMat34Legacy *)((unsigned char *)semanticData.data + i * semanticData.stride));
		}
	}
}

ApexRenderResource::ApexRenderResource(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderResourceDesc& desc)
{
	m_ApexVbs.resize(desc.numVertexBuffers);
	std::vector<D3DVERTEXELEMENT9> elems;
	for(WORD i = 0; i < desc.numVertexBuffers; i++)
	{
		ApexRenderVertexBuffer * pVb = static_cast<ApexRenderVertexBuffer *>(desc.vertexBuffers[i]);
		m_ApexVbs[i] = pVb;
		std::vector<D3DVERTEXELEMENT9> ve = pVb->m_ve.BuildVertexElementList(i);
		elems.insert(elems.end(), ve.begin(), ve.end());
	}
	D3DVERTEXELEMENT9 elem_end = {0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0};
	elems.push_back(elem_end);

	m_firstVertex = desc.firstVertex;

	m_numVerts = desc.numVerts;

	pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl);

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

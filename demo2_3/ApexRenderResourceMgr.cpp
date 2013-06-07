#include "StdAfx.h"
#include "ApexRenderResourceMgr.h"
#include "Game.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

ApexRenderResourceMgr::ApexRenderResourceMgr(void)
{
}

ApexRenderResourceMgr::~ApexRenderResourceMgr(void)
{
}

physx::apex::NxUserRenderVertexBuffer* ApexRenderResourceMgr::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	return new ApexRendererVertexBuffer(Game::getSingleton().GetD3D9Device(), desc);
}

void ApexRenderResourceMgr::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	delete &buffer;
}

physx::apex::NxUserRenderIndexBuffer* ApexRenderResourceMgr::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	return NULL;
}

void ApexRenderResourceMgr::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
}

physx::apex::NxUserRenderBoneBuffer* ApexRenderResourceMgr::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	return NULL;
}

void ApexRenderResourceMgr::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
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
	return NULL;
}

void ApexRenderResourceMgr::releaseResource(physx::apex::NxUserRenderResource& resource)
{
}

physx::PxU32 ApexRenderResourceMgr::getMaxBonesForMaterial(void* material)
{
	return 0;
}

ApexRendererVertexBuffer::ApexRendererVertexBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	WORD offset = 0;
	for(unsigned int i = 0; i < physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		switch(physx::apex::NxRenderVertexSemantic::Enum(i))
		{
		case physx::apex::NxRenderVertexSemantic::POSITION:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreatePositionElement(0, offset, 0));
			break;
		case physx::apex::NxRenderVertexSemantic::NORMAL:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateNormalElement(0, offset, 0));
			break;
		case physx::apex::NxRenderVertexSemantic::TANGENT:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTangentElement(0, offset, 0));
			break;
		case physx::apex::NxRenderVertexSemantic::COLOR:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateColorElement(0, offset, 0));
			break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD0:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 0));
			break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD1:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 1));
			break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD2:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 2));
			break;
		case physx::apex::NxRenderVertexSemantic::TEXCOORD3:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateTexcoordElement(0, offset, 3));
			break;
		case physx::apex::NxRenderVertexSemantic::BONE_INDEX:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateBlendIndicesElement(0, offset, 0));
			break;
		case physx::apex::NxRenderVertexSemantic::BONE_WEIGHT:
			m_ve.insert(my::D3DVERTEXELEMENT9Set::CreateBlendWeightsElement(0, offset, 0));
			break;
		}
	}
	// ! ignore DISPLACEMENT_TEXCOORD, DISPLACEMENT_FLAGS, desc.hint
	m_vb.CreateVertexBuffer(pd3dDevice, m_ve.CalculateVertexStride() * desc.maxVerts, 0, 0, D3DPOOL_MANAGED);
}

ApexRendererVertexBuffer::~ApexRendererVertexBuffer(void)
{
}

void ApexRendererVertexBuffer::writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices)
{
}

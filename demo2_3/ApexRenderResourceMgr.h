#pragma once

class ApexRenderResourceMgr : public physx::apex::NxUserRenderResourceManager
{
public:
	ApexRenderResourceMgr(void);

	virtual ~ApexRenderResourceMgr(void);

	physx::apex::NxUserRenderVertexBuffer* createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc);

	void releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer);

	physx::apex::NxUserRenderIndexBuffer* createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc);

	void releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer);

	physx::apex::NxUserRenderBoneBuffer* createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc);

	void releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer);

	physx::apex::NxUserRenderInstanceBuffer* createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc);

	void releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer);

	physx::apex::NxUserRenderSpriteBuffer* createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc);

	void releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer);

	physx::apex::NxUserRenderResource* createResource(const physx::apex::NxUserRenderResourceDesc& desc);

	void releaseResource(physx::apex::NxUserRenderResource& resource);

	physx::PxU32 getMaxBonesForMaterial(void* material);
};

class ApexRendererVertexBuffer : public physx::apex::NxUserRenderVertexBuffer
{
public:
	ApexRendererVertexBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderVertexBufferDesc& desc);

	virtual ~ApexRendererVertexBuffer(void);

	void writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices);

	my::D3DVERTEXELEMENT9Set m_ve;

	my::VertexBuffer m_vb;
};

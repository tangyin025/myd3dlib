#pragma once

class ApexResourceCallback : public physx::apex::NxResourceCallback
{
public:
	ApexResourceCallback(void);

	virtual ~ApexResourceCallback(void);

	void* requestResource(const char* nameSpace, const char* name);

	void  releaseResource(const char* nameSpace, const char* name, void* resource);
};

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

class ApexRenderer : public physx::apex::NxUserRenderer
{
public:
	ApexRenderer(void);

	virtual ~ApexRenderer(void);

	void renderResource(const physx::apex::NxApexRenderContext& context);
};

class ApexRenderVertexBuffer : public physx::apex::NxUserRenderVertexBuffer
{
public:
	ApexRenderVertexBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderVertexBufferDesc& desc);

	virtual ~ApexRenderVertexBuffer(void);

	void writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices);

	my::D3DVERTEXELEMENT9Set m_ve;

	DWORD m_stride;

	my::VertexBuffer m_vb;
};

class ApexRenderIndexBuffer : public physx::apex::NxUserRenderIndexBuffer
{
public:
	ApexRenderIndexBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderIndexBufferDesc& desc);

	virtual ~ApexRenderIndexBuffer(void);

	void writeBuffer(const void* srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements);

	my::IndexBuffer m_ib;
};

class ApexRenderBoneBuffer : public physx::apex::NxUserRenderBoneBuffer
{
public:
	ApexRenderBoneBuffer(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderBoneBufferDesc& desc);

	virtual ~ApexRenderBoneBuffer(void);

	void writeBuffer(const physx::NxApexRenderBoneBufferData& data, physx::PxU32 firstBone, physx::PxU32 numBones);

	std::vector<my::Matrix4> m_bones;
};

class ApexRenderResource : public physx::apex::NxUserRenderResource
{
public:
	ApexRenderResource(IDirect3DDevice9 * pd3dDevice, const physx::apex::NxUserRenderResourceDesc& desc);

	virtual ~ApexRenderResource(void);

	void setVertexBufferRange(physx::PxU32 firstVertex, physx::PxU32 numVerts) {m_firstVertex = firstVertex; m_numVerts = numVerts;}

	void setIndexBufferRange(physx::PxU32 firstIndex, physx::PxU32 numIndices) {m_firstIndex = firstIndex; m_numIndices = numIndices;}

	void setBoneBufferRange(physx::PxU32 firstBone, physx::PxU32 numBones) {m_firstBone = firstBone; m_numBones = numBones;}

	void setInstanceBufferRange(physx::PxU32 firstInstance, physx::PxU32 numInstances) {}

	void setSpriteBufferRange(physx::PxU32 firstSprite, physx::PxU32 numSprites) {}

	void setMaterial(void* material) {m_material = static_cast<my::Material *>(material);}

	physx::PxU32 getNbVertexBuffers() const {return m_ApexVbs.size();}

	physx::apex::NxUserRenderVertexBuffer* getVertexBuffer(physx::PxU32 index) const {return m_ApexVbs[index];}

	physx::apex::NxUserRenderIndexBuffer* getIndexBuffer() const {return m_ApexIb;}

	physx::apex::NxUserRenderBoneBuffer* getBoneBuffer() const {return m_ApexBb;}

	physx::apex::NxUserRenderInstanceBuffer* getInstanceBuffer() const {return NULL;}

	physx::apex::NxUserRenderSpriteBuffer* getSpriteBuffer() const {return NULL;}

	std::vector<ApexRenderVertexBuffer *> m_ApexVbs;

	unsigned int m_firstVertex;

	unsigned int m_numVerts;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	ApexRenderIndexBuffer * m_ApexIb;

	unsigned int m_firstIndex;

	unsigned int m_numIndices;

	ApexRenderBoneBuffer * m_ApexBb;

	unsigned int m_firstBone;

	unsigned int m_numBones;

	my::Material * m_material;
};

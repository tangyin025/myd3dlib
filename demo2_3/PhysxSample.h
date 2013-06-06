#pragma once

#include "PhysxPtr.hpp"

class PhysxSampleAllocator : public PxAllocatorCallback
{
public:
	void * allocate(size_t size, const char * typeName, const char * filename, int line);

	void deallocate(void * ptr);
};

class PhysxSampleErrorCallback : public PxErrorCallback
{
public:
	PhysxSampleErrorCallback(void)
	{
	}

	~PhysxSampleErrorCallback(void)
	{
	}

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
};

class UserRenderResourceManager
	: public physx::apex::NxUserRenderResourceManager
{
	virtual physx::apex::NxUserRenderVertexBuffer* createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc) {return NULL;}

	virtual void releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer) {}

	virtual physx::apex::NxUserRenderIndexBuffer* createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc) {return NULL;}

	virtual void releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer) {}

	virtual physx::apex::NxUserRenderBoneBuffer* createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc) {return NULL;}

	virtual void releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer) {}

	virtual physx::apex::NxUserRenderInstanceBuffer* createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc) {return NULL;}

	virtual void releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer) {}

	virtual physx::apex::NxUserRenderSpriteBuffer* createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc) {return NULL;}

	virtual void releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer) {}

	virtual physx::apex::NxUserRenderResource* createResource(const physx::apex::NxUserRenderResourceDesc& desc) {return NULL;}

	virtual void releaseResource(physx::apex::NxUserRenderResource& resource) {}

	virtual physx::PxU32 getMaxBonesForMaterial(void* material) {return 0;}
};

class PhysxSample
	: public my::SingleInstance<PhysxSample>
{
public:
	PhysxSampleAllocator m_Allocator;

	PhysxSampleErrorCallback m_ErrorCallback;

	PhysxPtr<PxFoundation> m_Foundation;

	PhysxPtr<PxPhysics> m_Physics;

	PhysxPtr<PxCooking> m_Cooking;

	PhysxPtr<PxDefaultCpuDispatcher> m_CpuDispatcher;

	UserRenderResourceManager m_ApexUserRenderResMgr;

	PhysxPtr<physx::apex::NxApexSDK> m_ApexSDK;

	PhysxPtr<physx::apex::NxModuleDestructible> m_ModuleDestructible;

public:
	PhysxSample(void)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);
};

class PhysxScene;

class StepperTask
	: public physx::pxtask::LightCpuTask
{
public:
	PhysxScene * m_Scene;

	StepperTask(PhysxScene * Scene)
		: m_Scene(Scene)
	{
	}

	virtual void run(void);

	virtual const char * getName(void) const;
};

class PhysxScene
	: public my::DrawHelper
{
public:
	PhysxPtr<PxScene> m_Scene;

	PhysxPtr<physx::apex::NxApexScene> m_ApexScene;

	my::Timer m_Timer;

	StepperTask m_Completion0, m_Completion1;

	my::Event m_Sync;

	bool m_WaitForResults;

	PhysxPtr<PxMaterial> m_Material;

public:
	PhysxScene(void)
		: m_Timer(1/60.0f,0)
		, m_Completion0(this)
		, m_Completion1(this)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
	{
	}

	virtual ~PhysxScene(void)
	{
		_ASSERT(0 == m_Scene->getNbActors(PxActorTypeSelectionFlags(0xff)));
	}

	bool OnInit(void);

	void OnShutdown(void);

	void OnTickPreRender(float dtime);

	void OnTickPostRender(float dtime);

	bool Advance(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void DrawRenderBuffer(IDirect3DDevice9 * pd3dDevice, const PxRenderBuffer & debugRenderable);
};

#pragma once

class PhysxSample;

class StepperTask
	: public physx::pxtask::LightCpuTask
{
public:
	PhysxSample * m_Sample;

	StepperTask(PhysxSample * Sample)
		: m_Sample(Sample)
	{
	}

	virtual void run(void);

	virtual const char * getName(void) const;
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
{
protected:
	PxDefaultAllocator m_DefaultAllocator;

	PxDefaultErrorCallback m_DefaultErrorCallback;

	PxFoundation * m_Foundation;

	PxProfileZoneManager * m_ProfileZoneManager;

	PxPhysics * m_Physics;

	PxCooking * m_Cooking;

	PxDefaultCpuDispatcher * m_CpuDispatcher;

	PxScene * m_Scene;

	my::Timer m_Timer;

	StepperTask m_Completion0, m_Completion1;

	my::Event m_Sync;

	bool m_WaitForResults;

	PxMaterial * m_DefaultMaterial;

	PxRigidDynamic * m_Sphere;

	PxRigidStatic * m_Plane;

	UserRenderResourceManager m_ApexUserRenderResMgr;

	physx::apex::NxApexSDK * m_ApexSDK;

public:
	PhysxSample(void)
		: m_Timer(1/60.0f,0)
		, m_Completion0(this)
		, m_Completion1(this)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);

	void OnTickPreRender(float dtime);

	void OnTickPostRender(float dtime);

	bool Advance(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);
};

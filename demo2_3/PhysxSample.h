#pragma once

#include "PhysxPtr.hpp"
#include "ApexRenderResourceMgr.h"

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

class PhysxSample
	: public my::SingleInstance<PhysxSample>
{
public:
	PhysxSampleAllocator m_Allocator;

	PhysxSampleErrorCallback m_ErrorCallback;

	ApexRenderer m_ApexRenderer;

	PhysxPtr<PxFoundation> m_Foundation;

	PhysxPtr<PxPhysics> m_Physics;

	PhysxPtr<PxCooking> m_Cooking;

	PhysxPtr<PxDefaultCpuDispatcher> m_CpuDispatcher;

	ApexRenderResourceMgr m_ApexUserRenderResMgr;

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

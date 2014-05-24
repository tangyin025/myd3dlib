#pragma once

#include "physx_ptr.hpp"

class PhysXAllocator : public PxAllocatorCallback
{
public:
	PhysXAllocator(void)
	{
	}

	void * allocate(size_t size, const char * typeName, const char * filename, int line);

	void deallocate(void * ptr);
};

class PhysXErrorCallback : public PxErrorCallback
{
public:
	PhysXErrorCallback(void)
	{
	}

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
};

class PhysXContext
{
protected:
	PhysXAllocator m_Allocator;

	PhysXErrorCallback m_ErrorCallback;

	physx_ptr<PxFoundation> m_Foundation;

	physx_ptr<PxPhysics> m_sdk;

	physx_ptr<PxCooking> m_Cooking;

	physx_ptr<PxDefaultCpuDispatcher> m_CpuDispatcher;

public:
	PhysXContext(void)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);
};

class PhysXSceneContext;

class StepperTask
	: public physx::pxtask::LightCpuTask
{
public:
	PhysXSceneContext * m_Scene;

	StepperTask(PhysXSceneContext * Scene)
		: m_Scene(Scene)
	{
	}

	virtual void run(void);

	virtual const char * getName(void) const;
};

class PhysXSceneContext
{
protected:
	physx_ptr<PxScene> m_Scene;

	my::Timer m_Timer;

	StepperTask m_Completion0, m_Completion1;

	my::Event m_Sync;

	bool m_WaitForResults;

	physx::PxU32 m_ErrorState;

public:
	PhysXSceneContext(void)
		: m_Timer(1/60.0f,0)
		, m_Completion0(this)
		, m_Completion1(this)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
		, m_ErrorState(0)
	{
	}

	bool OnInit(PxPhysics * sdk, PxDefaultCpuDispatcher * dispatcher);

	void OnShutdown(void);

	void OnTickPreRender(float dtime);

	void OnTickPostRender(float dtime);

	bool Advance(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void PushRenderBuffer(my::DrawHelper * drawHelper);
};

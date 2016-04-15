#pragma once

#include "PhysXPtr.h"

class PhysXAllocator : public PxAllocatorCallback
{
public:
	PhysXAllocator(void)
	{
	}

	void * allocate(size_t size, const char * typeName, const char * filename, int line);

	void deallocate(void * ptr);
};

class PhysXContext
	: public my::SingleInstance<PhysXContext>
	, public PxErrorCallback
{
public:
	static const my::Vector3 Gravity;

	PhysXAllocator m_Allocator;

	PhysXPtr<PxFoundation> m_Foundation;

	PhysXPtr<PxPhysics> m_sdk;

	PhysXPtr<PxCooking> m_Cooking;

	PhysXPtr<PxDefaultCpuDispatcher> m_CpuDispatcher;

	PhysXPtr<PxControllerManager> m_ControllerMgr;

	PhysXPtr<PxMaterial> m_PxMaterial;

public:
	PhysXContext(void)
	{
	}

	bool Init(void);

	void Shutdown(void);
};

class PhysXSceneContext
	: public my::SingleInstance<PhysXSceneContext>
{
public:
	class StepperTask
		: public pxtask::LightCpuTask
	{
	public:
		PhysXSceneContext * m_PxScene;

		StepperTask(PhysXSceneContext * Scene)
			: m_PxScene(Scene)
		{
		}

		virtual void run(void);

		virtual const char * getName(void) const;
	};

	StepperTask m_Completion0, m_Completion1;

	my::Timer m_Timer;

	my::Event m_Sync;

	bool m_WaitForResults;

	PxU32 m_ErrorState;

	PhysXPtr<PxScene> m_PxScene;

	typedef boost::signals2::signal<void (float)> SubstepEvent;

	SubstepEvent m_EventPxThreadSubstep;

public:
	PhysXSceneContext(void)
		: m_Completion0(this)
		, m_Completion1(this)
		, m_Timer(1/60.0f,0)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
		, m_ErrorState(0)
	{
	}

	bool Init(PxPhysics * sdk, PxDefaultCpuDispatcher * dispatcher);

	void Shutdown(void);

	void TickPreRender(float dtime);

	void TickPostRender(float dtime);

	bool Advance(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void PushRenderBuffer(my::DrawHelper * drawHelper);
};

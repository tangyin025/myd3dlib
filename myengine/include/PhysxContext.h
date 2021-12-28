#pragma once

#include <PxPhysicsAPI.h>
#include "mySingleton.h"
#include "myMath.h"
#include "myUtility.h"
#include "myThread.h"

class PhysxAllocator : public physx::PxAllocatorCallback
{
public:
	PhysxAllocator(void)
	{
	}

	void * allocate(size_t size, const char * typeName, const char * filename, int line);

	void deallocate(void * ptr);
};

template<class T>
struct PhysxDeleter
{
	typedef void result_type;

	typedef T * argument_type;

	void operator()(T * x) const
	{
		_ASSERT(x); x->release();
	}
};

class PhysxInputData : public physx::PxInputData
{
protected:
	my::IStreamPtr m_istr;

public:
	PhysxInputData(my::IStreamPtr istr)
		: m_istr(istr)
	{
	}

	virtual uint32_t read(void* dest, uint32_t count)
	{
		return m_istr->read(dest, count);
	}

	virtual uint32_t getLength() const
	{
		return m_istr->GetSize();
	}

	virtual void seek(uint32_t offset)
	{
		m_istr->seek(offset, SEEK_SET);
	}

	virtual uint32_t tell() const
	{
		return m_istr->tell();
	}
};

class PhysxSdk
	: public my::SingleInstance<PhysxSdk>
	, public physx::PxErrorCallback
{
public:
	PhysxAllocator m_Allocator;

	boost::shared_ptr<physx::PxFoundation> m_Foundation;

	boost::shared_ptr<physx::PxPvd> m_Pvd;

	boost::shared_ptr<physx::PxPhysics> m_sdk;

	boost::shared_ptr<physx::PxCooking> m_Cooking;

	boost::shared_ptr<physx::PxDefaultCpuDispatcher> m_CpuDispatcher;

	bool m_RenderTickMuted;

public:
	PhysxSdk(void)
		: m_RenderTickMuted(false)
	{
	}

	bool Init(void);

	void Shutdown(void);

	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};

class PhysxScene
	: public physx::PxSimulationEventCallback
{
public:
	class StepperTask
		: public physx::PxLightCpuTask
	{
	public:
		PhysxScene * m_PxScene;

		StepperTask(PhysxScene * Scene)
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

	physx::PxU32 m_ErrorState;

	boost::shared_ptr<physx::PxScene> m_PxScene;

	typedef boost::signals2::signal<void (float)> SubstepEvent;

	SubstepEvent m_EventPxThreadSubstep;

	boost::shared_ptr<physx::PxControllerManager> m_ControllerMgr;

	std::vector<physx::PxActiveTransform> mBufferedActiveTransforms;

	std::vector<physx::PxActor *> mDeletedActors;

	physx::PxU32 mActiveTransformCount;

	typedef std::vector<physx::PxTriggerPair> TriggerPairList;
	
	TriggerPairList mTriggerPairs;

public:
	PhysxScene(void)
		: m_Completion0(this)
		, m_Completion1(this)
		, m_Timer(1/60.0f)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
		, m_ErrorState(0)
		, mActiveTransformCount(0)
	{
	}

	bool Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher);

	float GetVisualizationParameter(physx::PxVisualizationParameter::Enum paramEnum) const;

	void SetVisualizationParameter(physx::PxVisualizationParameter::Enum param, float value);

	void SetControllerDebugRenderingFlags(physx::PxU32 flags);

	void Shutdown(void);

	void TickPreRender(float dtime);

	void TickPostRender(float dtime);

	bool Advance(float dtime);

	void AdvanceSync(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void PushRenderBuffer(my::DrawHelper * drawHelper);

	void Flush(void);

	static physx::PxFilterFlags filter(
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags,
		const void* constantBlock,
		physx::PxU32 constantBlockSize);

	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count);

	virtual void onWake(physx::PxActor** actors, physx::PxU32 count);

	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count);

	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);

	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);

	virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count);

	void removeRenderActorsFromPhysicsActor(const physx::PxActor * actor);
};

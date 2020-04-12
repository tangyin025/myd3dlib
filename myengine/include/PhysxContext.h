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

class PhysxContext
	: public my::SingleInstance<PhysxContext>
	, public physx::PxErrorCallback
{
public:
	static const my::Vector3 Gravity;

	PhysxAllocator m_Allocator;

	boost::shared_ptr<physx::PxFoundation> m_Foundation;

	boost::shared_ptr<physx::PxPhysics> m_sdk;

	boost::shared_ptr<physx::PxCooking> m_Cooking;

	boost::shared_ptr<physx::PxDefaultCpuDispatcher> m_CpuDispatcher;

public:
	PhysxContext(void)
	{
	}

	bool Init(void);

	void Shutdown(void);

	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};

class PhysxSceneContext
	: public my::SingleInstance<PhysxSceneContext>
	, public physx::PxSimulationEventCallback
{
public:
	class StepperTask
		: public physx::PxLightCpuTask
	{
	public:
		PhysxSceneContext * m_PxScene;

		StepperTask(PhysxSceneContext * Scene)
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

	boost::shared_ptr<physx::PxSerializationRegistry> m_Registry;

	boost::shared_ptr<physx::PxCollection> m_Collection;

	typedef std::map<std::string, boost::shared_ptr<physx::PxBase> > PxObjectMap;

	PxObjectMap m_CollectionObjs;

	boost::shared_ptr<unsigned char> m_SerializeBuff;

public:
	PhysxSceneContext(void)
		: m_Completion0(this)
		, m_Completion1(this)
		, m_Timer(1/60.0f,0)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
		, m_ErrorState(0)
	{
	}

	bool Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher);

	float GetVisualizationParameter(void) const;

	void SetVisualizationParameter(float param);

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void ClearSerializedObjs(void);

	void Shutdown(void);

	void TickPreRender(float dtime);

	void TickPostRender(float dtime);

	bool Advance(float dtime);

	bool AdvanceSync(float dtime);

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
};

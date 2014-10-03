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

class PhysXTriangleMesh : public my::DeviceRelatedObjectBase
{
public:
	PxTriangleMesh * m_ptr;

public:
	PhysXTriangleMesh(PxTriangleMesh * ptr)
		: m_ptr(ptr)
	{
	}

	virtual ~PhysXTriangleMesh(void)
	{
		if (m_ptr)
		{
			m_ptr->release();
		}
	}

	virtual void OnResetDevice(void)
	{
	}

	virtual void OnLostDevice(void)
	{
	}

	virtual void OnDestroyDevice(void)
	{
	}
};

typedef boost::shared_ptr<PhysXTriangleMesh> PhysXTriangleMeshPtr;

class PhysXClothFabric : public my::DeviceRelatedObjectBase
{
public:
	PxClothFabric * m_ptr;

public:
	PhysXClothFabric(PxClothFabric * ptr)
		: m_ptr(ptr)
	{
	}

	virtual ~PhysXClothFabric(void)
	{
		if (m_ptr)
		{
			m_ptr->release();
		}
	}

	virtual void OnResetDevice(void)
	{
	}

	virtual void OnLostDevice(void)
	{
	}

	virtual void OnDestroyDevice(void)
	{
	}
};

typedef boost::shared_ptr<PhysXClothFabric> PhysXClothFabricPtr;

class PhysXContext
	: public PxErrorCallback
{
public:
	static const my::Vector3 Gravity;

	PhysXAllocator m_Allocator;

	physx_ptr<PxFoundation> m_Foundation;

	physx_ptr<PxPhysics> m_sdk;

	physx_ptr<PxCooking> m_Cooking;

	physx_ptr<PxDefaultCpuDispatcher> m_CpuDispatcher;

	physx_ptr<PxControllerManager> m_ControllerMgr;

	physx_ptr<PxMaterial> m_PxMaterial;

public:
	PhysXContext(void)
	{
	}

	bool OnInit(void);

	void OnShutdown(void);

	void CookTriangleMesh(my::OStreamPtr ostream, my::OgreMeshPtr mesh);

	void CookTriangleMeshToFile(std::string path, my::OgreMeshPtr mesh);

	PxTriangleMesh * CreateTriangleMesh(my::IStreamPtr istream);

	void CookClothFabric(my::OStreamPtr ostream, my::OgreMeshPtr mesh);

	void CookClothFabricToFile(std::string path, my::OgreMeshPtr mesh);

	PxClothFabric * CreateClothFabric(my::IStreamPtr istream);
};

class PhysXSceneContext
{
public:
	class StepperTask
		: public physx::pxtask::LightCpuTask
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

	physx::PxU32 m_ErrorState;

	physx_ptr<PxScene> m_PxScene;

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

	bool OnInit(PxPhysics * sdk, PxDefaultCpuDispatcher * dispatcher);

	void OnShutdown(void);

	void OnTickPreRender(float dtime);

	void OnTickPostRender(float dtime);

	bool Advance(float dtime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void PushRenderBuffer(my::DrawHelper * drawHelper);
};

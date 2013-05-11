#include "StdAfx.h"
#include "PhysxSample.h"

#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib")
#pragma comment(lib, "PhysX3Cooking_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysXProfileSDK.lib")
//#pragma comment(lib, "PxTask.lib")

#define PHYSX_SAFE_RELEASE(p) if(p) { p->release(); p=NULL; }

void StepperTask::run(void)
{
	m_Sample->SubstepDone(this);
	release();
}

const char * StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysxSample::OnInit(void)
{
	if(!(m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocator, m_DefaultErrorCallback)))
	{
		THROW_CUSEXCEPTION("PxCreateFoundation failed");
	}

	if(!(m_ProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_Foundation)))
	{
		THROW_CUSEXCEPTION("PxProfileZoneManager::createProfileZoneManager failed");
	}

	if(!(m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, m_ProfileZoneManager)))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!PxInitExtensions(*m_Physics))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	if(!(m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!(m_CpuDispatcher = PxDefaultCpuDispatcherCreate(1)))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}

	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_CpuDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_Scene = m_Physics->createScene(sceneDesc)))
	{
		THROW_CUSEXCEPTION("m_Physics->createScene failed");
	}

	if(!(m_DefaultMaterial = m_Physics->createMaterial(0.5f, 0.5f, 0.1f)))
	{
		THROW_CUSEXCEPTION("m_Physics->createMaterial failed");
	}

	if(!(m_Sphere = PxCreateDynamic(*m_Physics, PxTransform(PxVec3(0,10,0)), PxSphereGeometry(1), *m_DefaultMaterial, 1)))
	{
		THROW_CUSEXCEPTION("PxCreateDynamic failed");
	}
	m_Scene->addActor(*m_Sphere);

	if(!(m_Plane = PxCreatePlane(*m_Physics, PxPlane(PxVec3(0,1,0), 0), *m_DefaultMaterial)))
	{
		THROW_CUSEXCEPTION("PxCreatePlane failed");
	}
	m_Scene->addActor(*m_Plane);

	m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);

	m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);

	return true;
}

void PhysxSample::OnShutdown(void)
{
	PHYSX_SAFE_RELEASE(m_Plane);

	PHYSX_SAFE_RELEASE(m_Sphere);

	PHYSX_SAFE_RELEASE(m_DefaultMaterial);

	PHYSX_SAFE_RELEASE(m_Scene);

	PHYSX_SAFE_RELEASE(m_CpuDispatcher);

	PHYSX_SAFE_RELEASE(m_Cooking);

	PxCloseExtensions();

	PHYSX_SAFE_RELEASE(m_Physics);

	PHYSX_SAFE_RELEASE(m_ProfileZoneManager);

	PHYSX_SAFE_RELEASE(m_Foundation);
}

void PhysxSample::OnTickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	m_WaitForResults = Advance(dtime);
}

void PhysxSample::OnTickPostRender(float dtime)
{
	if(m_WaitForResults)
	{
		m_Sync.WaitEvent(INFINITE);
	}
}

bool PhysxSample::Advance(float dtime)
{
	m_Timer.m_RemainingTime = my::Min(0.1f, m_Timer.m_RemainingTime + dtime);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		return false;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	m_Completion0.setContinuation(*m_Scene->getTaskManager(), NULL);

	Substep(m_Completion0);

	m_Completion0.removeReference();

	return true;
}

void PhysxSample::Substep(StepperTask & completionTask)
{
	m_Scene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysxSample::SubstepDone(StepperTask * ownerTask)
{
	m_Scene->fetchResults(true);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		m_Sync.SetEvent();
		return;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	StepperTask & task = (ownerTask == &m_Completion0 ? m_Completion1 : m_Completion0);

	task.setContinuation(*m_Scene->getTaskManager(), NULL);

	Substep(task);

	task.removeReference();
}

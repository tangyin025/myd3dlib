#include "StdAfx.h"
#include "PhysxSample.h"

#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib")
#pragma comment(lib, "PhysX3Cooking_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysXProfileSDK.lib")
#pragma comment(lib, "PxTask.lib")

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

	return true;
}

void PhysxSample::OnShutdown(void)
{
	if(m_Scene)
		m_Scene->release();

	if(m_CpuDispatcher)
		m_CpuDispatcher->release();

	if(m_Cooking)
		m_Cooking->release();

	PxCloseExtensions();

	if(m_Physics)
		m_Physics->release();

	if(m_ProfileZoneManager)
		m_ProfileZoneManager->release();

	if(m_Foundation)
		m_Foundation->release();
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

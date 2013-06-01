#include "StdAfx.h"
#include "PhysxSample.h"

using namespace my;

void * PhysxSampleAllocator::allocate(size_t size, const char * typeName, const char * filename, int line)
{
#ifdef _DEBUG
	return _aligned_malloc_dbg(size, 16, filename, line);
#else
	return _aligned_malloc(size, 16);	
#endif
}

void PhysxSampleAllocator::deallocate(void * ptr)
{
#ifdef _DEBUG
	_aligned_free_dbg(ptr);
#else
	_aligned_free(ptr);
#endif
}

void PhysxSampleErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch(code)
	{
	case PxErrorCode::eDEBUG_INFO:
		OutputDebugStringA(str_printf("%s (%d) : info: %s\n", file, line, message).c_str());
		break;

	case PxErrorCode::eDEBUG_WARNING:
	case PxErrorCode::ePERF_WARNING:
		OutputDebugStringA(str_printf("%s (%d) : warning: %s\n", file, line, message).c_str());
		break;

	default:
		OutputDebugStringA(str_printf("%s (%d) : error: %s\n", file, line, message).c_str());
		break;
	}
}

bool PhysxSample::OnInit(void)
{
	if(!(m_Foundation.reset(PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback)), m_Foundation))
	{
		THROW_CUSEXCEPTION("PxCreateFoundation failed");
	}

	if(!(m_Physics.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(),
#ifdef _DEBUG
		true
#else
		false
#endif
		)), m_Physics))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())), m_Cooking))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!(m_CpuDispatcher.reset(PxDefaultCpuDispatcherCreate(1, NULL)), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}

	physx::apex::NxApexSDKDesc apexDesc;
	apexDesc.outputStream = &PhysxSample::getSingleton().m_ErrorCallback;
	apexDesc.physXSDKVersion = PX_PHYSICS_VERSION;
	apexDesc.physXSDK = PhysxSample::getSingleton().m_Physics.get();
	apexDesc.cooking = PhysxSample::getSingleton().m_Cooking.get();
	apexDesc.renderResourceManager = &m_ApexUserRenderResMgr;
	if(!(m_ApexSDK.reset(NxCreateApexSDK(apexDesc)), m_ApexSDK))
	{
		THROW_CUSEXCEPTION("NxCreateApexSDK failed");
	}

	if(!(m_ModuleDestructible.reset(static_cast<physx::apex::NxModuleDestructible *>(m_ApexSDK->createModule("Destructible"))),
		m_ModuleDestructible))
	{
		THROW_CUSEXCEPTION("m_ApexSDK->createModule failed");
	}

	NxParameterized::Interface * moduleDesc = m_ModuleDestructible->getDefaultModuleDesc();
	m_ModuleDestructible->init(*moduleDesc);

	if(!PxInitExtensions(*m_Physics))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	return true;
}

PhysxSample::SingleInstance * SingleInstance<PhysxSample>::s_ptr = NULL;

void PhysxSample::OnShutdown(void)
{
	if(m_Physics)
		PxCloseExtensions();
}

void StepperTask::run(void)
{
	m_Scene->SubstepDone(this);
	release();
}

const char * StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysxScene::OnInit(void)
{
	PxSceneDesc sceneDesc(PhysxSample::getSingleton().m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = PhysxSample::getSingleton().m_CpuDispatcher.get();
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_Scene.reset(PhysxSample::getSingleton().m_Physics->createScene(sceneDesc)), m_Scene))
	{
		THROW_CUSEXCEPTION("PhysxSample::getSingleton().m_Physics->createScene failed");
	}

	physx::apex::NxApexSceneDesc apexSceneDesc;
	apexSceneDesc.scene = m_Scene.get();
	if(!(m_ApexScene.reset(PhysxSample::getSingleton().m_ApexSDK->createScene(apexSceneDesc)), m_ApexScene))
	{
		THROW_CUSEXCEPTION("m_ApexSDK->createScene failed");
	}

	if(!(m_Material.reset(PhysxSample::getSingleton().m_Physics->createMaterial(0.5f, 0.5f, 0.1f)), m_Material))
	{
		THROW_CUSEXCEPTION("PhysxSample::getSingleton().m_Physics->createMaterial failed");
	}

	PhysxPtr<PxActor> actor;
	if(!(actor.reset(PxCreateDynamic(*PhysxSample::getSingleton().m_Physics, PxTransform(PxVec3(0,10,0)), PxSphereGeometry(1), *m_Material, 1)),
		actor))
	{
		THROW_CUSEXCEPTION("PxCreateDynamic failed");
	}
	m_Scene->addActor(*actor);
	m_Actors.push_back(actor);

	if(!(actor.reset(PxCreatePlane(*PhysxSample::getSingleton().m_Physics, PxPlane(PxVec3(0,1,0), 0), *m_Material)),
		actor))
	{
		THROW_CUSEXCEPTION("PxCreatePlane failed");
	}
	m_Scene->addActor(*actor);
	m_Actors.push_back(actor);

	m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);

	m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);

	return true;
}

void PhysxScene::OnShutdown(void)
{
	m_Actors.clear();

	_ASSERT(0 == m_Scene->getNbActors(PxActorTypeSelectionFlags(0xff)));
}

void PhysxScene::OnTickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	m_WaitForResults = Advance(dtime);
}

void PhysxScene::OnTickPostRender(float dtime)
{
	if(m_WaitForResults)
	{
		m_Sync.WaitEvent(INFINITE);
	}
}

bool PhysxScene::Advance(float dtime)
{
	m_Timer.m_RemainingTime = Min(0.1f, m_Timer.m_RemainingTime + dtime);

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

void PhysxScene::Substep(StepperTask & completionTask)
{
	m_Scene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysxScene::SubstepDone(StepperTask * ownerTask)
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

void PhysxScene::DrawRenderBuffer(IDirect3DDevice9 * pd3dDevice, const PxRenderBuffer & debugRenderable)
{
	const PxU32 numPoints = debugRenderable.getNbPoints();
	if(numPoints)
	{
		const PxDebugPoint* PX_RESTRICT points = debugRenderable.getPoints();
		for(PxU32 i=0; i<numPoints; i++)
		{
			const PxDebugPoint& point = points[i];
		}
	}

	const PxU32 numLines = debugRenderable.getNbLines();
	if(numLines)
	{
		const PxDebugLine* PX_RESTRICT lines = debugRenderable.getLines();
		for(PxU32 i=0; i<numLines; i++)
		{
			const PxDebugLine& line = lines[i];
			DrawLine(pd3dDevice, (Vector3 &)line.pos0, (Vector3 &)line.pos1, line.color0);
		}
	}

	const PxU32 numTriangles = debugRenderable.getNbTriangles();
	if(numTriangles)
	{
		const PxDebugTriangle* PX_RESTRICT triangles = debugRenderable.getTriangles();
		for(PxU32 i=0; i<numTriangles; i++)
		{
			const PxDebugTriangle& triangle = triangles[i];
			DrawTriangle(pd3dDevice, (Vector3 &)triangle.pos0, (Vector3 &)triangle.pos1, (Vector3 &)triangle.pos2, triangle.color0);
		}
	}
}

#include "StdAfx.h"
#include "PhysXContext.h"
#include "Game.h"

void * PhysXAllocator::allocate(size_t size, const char * typeName, const char * filename, int line)
{
#ifdef _DEBUG
	return _aligned_malloc_dbg(size, 16, filename, line);
#else
	return _aligned_malloc(size, 16);	
#endif
}

void PhysXAllocator::deallocate(void * ptr)
{
#ifdef _DEBUG
	_aligned_free_dbg(ptr);
#else
	_aligned_free(ptr);
#endif
}

void PhysXErrorCallback::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch(code)
	{
	case PxErrorCode::eDEBUG_INFO:
		Game::getSingleton().AddLine(ms2ws(str_printf("%s (%d) : info: %s", file, line, message)));
		break;

	case PxErrorCode::eDEBUG_WARNING:
	case PxErrorCode::ePERF_WARNING:
		Game::getSingleton().AddLine(ms2ws(str_printf("%s (%d) : warning: %s", file, line, message)), D3DCOLOR_ARGB(255,255,255,0));
		break;

	default:
		OutputDebugStringA(str_printf("%s (%d) : error: %s\n", file, line, message).c_str());
		Game::getSingleton().AddLine(ms2ws(str_printf("%s, (%d) : error: %s", file, line, message)), D3DCOLOR_ARGB(255,255,0,0));
		break;
	}
}

bool PhysXContext::OnInit(void)
{
	if(!(m_Foundation.reset(PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback)), m_Foundation))
	{
		THROW_CUSEXCEPTION(_T("PxCreateFoundation failed"));
	}

	if(!(m_Physics.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(),
#ifdef _DEBUG
		true,
#else
		false,
#endif
		NULL)), m_Physics))
	{
		THROW_CUSEXCEPTION(_T("PxCreatePhysics failed"));
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())), m_Cooking))
	{
		THROW_CUSEXCEPTION(_T("PxCreateCooking failed"));
	}

	if(!(m_CpuDispatcher.reset(PxDefaultCpuDispatcherCreate(1, NULL)), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION(_T("PxDefaultCpuDispatcherCreate failed"));
	}

	physx::apex::NxApexSDKDesc apexDesc;
	apexDesc.outputStream = &m_ErrorCallback;
	apexDesc.physXSDKVersion = PX_PHYSICS_VERSION;
	apexDesc.physXSDK = m_Physics.get();
	apexDesc.cooking = m_Cooking.get();
	apexDesc.renderResourceManager = &m_ApexUserRenderResMgr;
	apexDesc.resourceCallback = &m_ApexResourceCallback;
	if(!(m_ApexSDK.reset(NxCreateApexSDK(apexDesc)), m_ApexSDK))
	{
		THROW_CUSEXCEPTION(_T("NxCreateApexSDK failed"));
	}

	if(!(m_ModuleDestructible.reset(static_cast<physx::apex::NxModuleDestructible *>(m_ApexSDK->createModule("Destructible"))),
		m_ModuleDestructible))
	{
		THROW_CUSEXCEPTION(_T("m_ApexSDK->createModule failed"));
	}

	NxParameterized::Interface * moduleDesc = m_ModuleDestructible->getDefaultModuleDesc();
	m_ModuleDestructible->init(*moduleDesc);
	m_ModuleDestructible->setLODBenefitValue(200);
	m_ModuleDestructible->setLODEnabled(true);

	if(!PxInitExtensions(*m_Physics))
	{
		THROW_CUSEXCEPTION(_T("PxInitExtensions failed"));
	}
	return true;
}

void PhysXContext::OnShutdown(void)
{
	if(m_Physics)
		PxCloseExtensions();

	m_ModuleDestructible.reset();

	m_ApexSDK.reset();

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_Physics.reset();

	m_Foundation.reset();
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

bool PhysXSceneContext::OnInit(void)
{
	if(!PhysXContext::OnInit())
	{
		return false;
	}

	PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_CpuDispatcher.get();
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_Scene.reset(m_Physics->createScene(sceneDesc)), m_Scene))
	{
		THROW_CUSEXCEPTION(_T("m_Physics->createScene failed"));
	}

	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

	physx::apex::NxApexSceneDesc apexSceneDesc;
	apexSceneDesc.scene = m_Scene.get();
	if(!(m_ApexScene.reset(m_ApexSDK->createScene(apexSceneDesc)), m_ApexScene))
	{
		THROW_CUSEXCEPTION(_T("m_ApexSDK->createScene failed"));
	}

	m_ViewMatrixID = m_ApexScene->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_RH);
	m_ProjMatrixID = m_ApexScene->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);
	m_ApexScene->setUseViewProjMatrix(m_ViewMatrixID, m_ProjMatrixID);

	//NxParameterized::Interface * params = m_ApexScene->getDebugRenderParams();
	//NxParameterized::setParamF32(*params, "VISUALIZATION_ENABLE", 1.0f);
	//NxParameterized::setParamF32(*params, "VISUALIZATION_SCALE", 1.0f);
	//NxParameterized::setParamF32(*params, "VISUALIZE_LOD_BENEFITS", 1.0f);
	//NxParameterized::setParamF32(*params, "Destructible/VISUALIZE_DESTRUCTIBLE_ACTOR", 1.0f);
	//NxParameterized::setParamF32(*params, "Destructible/VISUALIZE_DESTRUCTIBLE_SUPPORT", 1.0f);

	if(!(m_Material.reset(m_Physics->createMaterial(0.5f, 0.5f, 0.1f)), m_Material))
	{
		THROW_CUSEXCEPTION(_T("m_Physics->createMaterial failed"));
	}

	return true;
}

void PhysXSceneContext::OnShutdown(void)
{
	_ASSERT(0 == m_Scene->getNbActors(PxActorTypeSelectionFlags(0xff)));

	m_Material.reset();

	m_ApexScene.reset();

	m_Scene.reset();

	PhysXContext::OnShutdown();
}

void PhysXSceneContext::SetViewMatrix(const my::Matrix4 & View)
{
	m_ApexScene->setViewMatrix((PxMat44&)View, m_ViewMatrixID);
}

void PhysXSceneContext::SetViewParams(const my::Vector3 & EyePos, const my::Vector3 & EyeDir, const my::Vector3 & WorldUp)
{
	m_ApexScene->setViewParams((PxVec3&)EyePos, (PxVec3&)EyeDir, (PxVec3&)WorldUp, m_ViewMatrixID);
}

void PhysXSceneContext::SetProjMatrix(const my::Matrix4 & Proj)
{
	m_ApexScene->setProjMatrix((PxMat44&)Proj, m_ProjMatrixID);
}

void PhysXSceneContext::SetProjParams(float nz, float fz, float fov, DWORD ViewportWidth, DWORD ViewportHeight)
{
	m_ApexScene->setProjParams(nz, fz, D3DXToDegree(fov), ViewportWidth, ViewportHeight, m_ProjMatrixID);
}

void PhysXSceneContext::OnTickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	m_WaitForResults = Advance(dtime);
}

void PhysXSceneContext::OnTickPostRender(float dtime)
{
	if(m_WaitForResults)
	{
		m_Sync.WaitEvent(INFINITE);
	}
}

bool PhysXSceneContext::Advance(float dtime)
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

void PhysXSceneContext::Substep(StepperTask & completionTask)
{
	m_ApexScene->simulate(m_Timer.m_Interval, true, &completionTask, 0, 0);
}

void PhysXSceneContext::SubstepDone(StepperTask * ownerTask)
{
	m_ApexScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

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

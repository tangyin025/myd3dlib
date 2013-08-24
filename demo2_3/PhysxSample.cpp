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

HRESULT PhysxSample::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	if(FAILED(hr = DxutApp::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

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
	return S_OK;
}

HRESULT PhysxSample::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	if(FAILED(hr = DxutApp::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	if(FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}
	return S_OK;
}

void PhysxSample::OnLostDevice(void)
{
	ResourceMgr::OnLostDevice();

	DxutApp::OnLostDevice();
}

void PhysxSample::OnDestroyDevice(void)
{
	if(m_Physics)
		PxCloseExtensions();

	m_ModuleDestructible.reset();

	m_ApexSDK.reset();

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_Physics.reset();

	m_Foundation.reset();

	ResourceMgr::OnDestroyDevice();

	DxutApp::OnDestroyDevice();
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
		THROW_CUSEXCEPTION(_T("PhysxSample::getSingleton().m_Physics->createScene failed"));
	}

	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

	physx::apex::NxApexSceneDesc apexSceneDesc;
	apexSceneDesc.scene = m_Scene.get();
	if(!(m_ApexScene.reset(PhysxSample::getSingleton().m_ApexSDK->createScene(apexSceneDesc)), m_ApexScene))
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

	if(!(m_Material.reset(PhysxSample::getSingleton().m_Physics->createMaterial(0.5f, 0.5f, 0.1f)), m_Material))
	{
		THROW_CUSEXCEPTION(_T("PhysxSample::getSingleton().m_Physics->createMaterial failed"));
	}

	return true;
}

void PhysxScene::OnShutdown(void)
{
	_ASSERT(0 == m_Scene->getNbActors(PxActorTypeSelectionFlags(0xff)));

	m_ApexScene.reset();

	m_Scene.reset();
}

void PhysxScene::SetProjParams(float nz, float fz, float fov, DWORD ViewportWidth, DWORD ViewportHeight)
{
	m_ApexScene->setProjParams(nz, fz, D3DXToDegree(fov), ViewportWidth, ViewportHeight, m_ProjMatrixID);
}

void PhysxScene::SetViewMatrix(const my::Matrix4 & View)
{
	m_ApexScene->setViewMatrix(PxMat44(&const_cast<my::Matrix4 &>(View)._11));
}

void PhysxScene::SetProjMatrix(const my::Matrix4 & Proj)
{
	m_ApexScene->setProjMatrix(PxMat44(&const_cast<my::Matrix4 &>(Proj)._11));
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
	m_ApexScene->simulate(m_Timer.m_Interval, true, &completionTask, 0, 0);
}

void PhysxScene::SubstepDone(StepperTask * ownerTask)
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

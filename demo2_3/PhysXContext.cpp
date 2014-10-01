#include "StdAfx.h"
#include "PhysXContext.h"

const my::Vector3 PhysXContext::Gravity(0.0f, -9.81f, 0.0f);

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

bool PhysXContext::OnInit(void)
{
	if(!(m_Foundation.reset(PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, *this)), m_Foundation))
	{
		THROW_CUSEXCEPTION(_T("PxCreateFoundation failed"));
	}

	if(!(m_sdk.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(),
#ifdef _DEBUG
		true,
#else
		false,
#endif
		NULL)), m_sdk))
	{
		THROW_CUSEXCEPTION(_T("PxCreatePhysics failed"));
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())), m_Cooking))
	{
		THROW_CUSEXCEPTION(_T("PxCreateCooking failed"));
	}

	if(!PxInitExtensions(*m_sdk))
	{
		THROW_CUSEXCEPTION(_T("PxInitExtensions failed"));
	}

	if(!(m_CpuDispatcher.reset(PxDefaultCpuDispatcherCreate(1, NULL)), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION(_T("PxDefaultCpuDispatcherCreate failed"));
	}

	if(!(m_ControllerMgr.reset(PxCreateControllerManager(*m_Foundation)), m_ControllerMgr))
	{
		THROW_CUSEXCEPTION(_T("PxCreateControllerManager failed"));
	}

	if(!(m_PxMaterial.reset(m_sdk->createMaterial(0.5f, 0.5f, 0.1f)), m_PxMaterial))
	{
		THROW_CUSEXCEPTION(_T("m_sdk->createMaterial failed"));
	}
	return true;
}

void PhysXContext::OnShutdown(void)
{
	m_PxMaterial.reset();

	if(m_sdk)
	{
		PxCloseExtensions();
	}

	m_ControllerMgr.reset();

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_sdk.reset();

	m_Foundation.reset();
}

HRESULT PhysXResourceMgr::OnCreateDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if (FAILED(hr = ResourceMgr::OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	OnInit();

	return S_OK;
}

HRESULT PhysXResourceMgr::OnResetDevice(
	IDirect3DDevice9 * pd3dDevice,
	const D3DSURFACE_DESC * pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if (FAILED(hr = ResourceMgr::OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc)))
	{
		return hr;
	}

	return S_OK;
}

void PhysXResourceMgr::OnLostDevice(void)
{
	ResourceMgr::OnLostDevice();
}

void PhysXResourceMgr::OnDestroyDevice(void)
{
	OnShutdown();

	ResourceMgr::OnDestroyDevice();
}

class PhysXOStream : public PxOutputStream
{
public:
	my::OStreamPtr ostream;

	PhysXOStream(my::OStreamPtr _ostream)
		: ostream(_ostream)
	{
	}

	virtual PxU32 write(const void* src, PxU32 count)
	{
		return ostream->write(src, count);
	}
};

class PhysXIStream : public PxInputStream
{
public:
	my::IStreamPtr istream;

	PhysXIStream(my::IStreamPtr _istream)
		: istream(_istream)
	{
	}

	virtual PxU32 read(void* dest, PxU32 count)
	{
		return istream->read(dest, count);
	}
};

void PhysXResourceMgr::CookTriangleMesh(my::OStreamPtr ostream, my::OgreMeshPtr mesh)
{
	PxTriangleMeshDesc desc;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();
	desc.points.data = mesh->LockVertexBuffer();
	desc.triangles.count = mesh->GetNumFaces();
	if (mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= PxMeshFlag::e16_BIT_INDICES;
	}
	desc.triangles.data = mesh->LockIndexBuffer();
	desc.materialIndices.stride = sizeof(DWORD);
	desc.materialIndices.data = (PxMaterialTableIndex *)mesh->LockAttributeBuffer();
	m_Cooking->cookTriangleMesh(desc, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
	mesh->UnlockAttributeBuffer();
}

void PhysXResourceMgr::CookTriangleMeshToFile(std::string path, my::OgreMeshPtr mesh)
{
	CookTriangleMesh(my::FileOStream::Open(ms2ts(path).c_str()), mesh);
}

PxTriangleMesh * PhysXResourceMgr::CreateTriangleMesh(my::IStreamPtr istream)
{
	// ! should be call at resource thread
	PxTriangleMesh * ret = m_sdk->createTriangleMesh(PhysXIStream(istream));
	return ret;
}

class TriangleMeshIORequest : public my::IORequest
{
protected:
	std::string m_path;

	PhysXResourceMgr * m_arc;

	my::CachePtr m_cache;

public:
	TriangleMeshIORequest(const my::ResourceCallback & callback, const std::string & path, PhysXResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		PxTriangleMesh * ptr = m_arc->CreateTriangleMesh(my::IStreamPtr(new my::MemoryIStream(&(*m_cache)[0], m_cache->size())));
		if (!ptr)
		{
			THROW_CUSEXCEPTION(str_printf(_T("CreateTriangleMesh failed")));
		}
		m_res.reset(new PhysXTriangleMesh(ptr));
	}
};

void PhysXResourceMgr::LoadTriangleMeshAsync(const std::string & path, const my::ResourceCallback & callback)
{
	LoadResourceAsync(path, my::IORequestPtr(new TriangleMeshIORequest(callback, path, this)));
}

PhysXTriangleMeshPtr PhysXResourceMgr::LoadTriangleMesh(const std::string & path)
{
	return LoadResource<PhysXTriangleMesh>(path, my::IORequestPtr(new TriangleMeshIORequest(my::ResourceCallback(), path, this)));
}

void PhysXResourceMgr::CookClothFabric(my::OStreamPtr ostream, my::OgreMeshPtr mesh)
{
	PxClothMeshDesc desc;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();
	desc.points.data = mesh->LockVertexBuffer();
	desc.triangles.count = mesh->GetNumFaces();
	if (mesh->GetOptions() & D3DXMESH_32BIT)
	{
		desc.triangles.stride = 3 * sizeof(DWORD);
	}
	else
	{
		desc.triangles.stride = 3 * sizeof(WORD);
		desc.flags |= PxMeshFlag::e16_BIT_INDICES;
	}
	desc.triangles.data = mesh->LockIndexBuffer();
	m_Cooking->cookClothFabric(desc, (PxVec3&)Gravity, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
}

void PhysXResourceMgr::CookClothFabricToFile(std::string path, my::OgreMeshPtr mesh)
{
	CookClothFabric(my::FileOStream::Open(ms2ts(path).c_str()), mesh);
}

PxClothFabric * PhysXResourceMgr::CreateClothFabric(my::IStreamPtr istream)
{
	PxClothFabric * ret = m_sdk->createClothFabric(PhysXIStream(istream));
	return ret;
}

class ClothFabricIORequest : public my::IORequest
{
protected:
	std::string m_path;

	PhysXResourceMgr * m_arc;

	my::CachePtr m_cache;

public:
	ClothFabricIORequest(const my::ResourceCallback & callback, const std::string & path, PhysXResourceMgr * arc)
		: m_path(path)
		, m_arc(arc)
	{
		if(callback)
		{
			m_callbacks.push_back(callback);
		}
	}

	virtual void DoLoad(void)
	{
		if(m_arc->CheckPath(m_path))
		{
			m_cache = m_arc->OpenIStream(m_path)->GetWholeCache();
		}
	}

	virtual void BuildResource(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		if(!m_cache)
		{
			THROW_CUSEXCEPTION(str_printf(_T("failed open %s"), ms2ts(m_path).c_str()));
		}
		PxClothFabric * ptr = m_arc->CreateClothFabric(my::IStreamPtr(new my::MemoryIStream(&(*m_cache)[0], m_cache->size())));
		if (!ptr)
		{
			THROW_CUSEXCEPTION(str_printf(_T("CreateClothFabric failed")));
		}
		m_res.reset(new PhysXClothFabric(ptr));
	}
};

void PhysXResourceMgr::LoadClothFabricAsync(const std::string & path, const my::ResourceCallback & callback)
{
	LoadResourceAsync(path, my::IORequestPtr(new ClothFabricIORequest(callback, path, this)));
}

PhysXClothFabricPtr PhysXResourceMgr::LoadClothFabric(const std::string & path)
{
	return LoadResource<PhysXClothFabric>(path, my::IORequestPtr(new ClothFabricIORequest(my::ResourceCallback(), path, this)));
}

void PhysXSceneContext::StepperTask::run(void)
{
	m_Scene->SubstepDone(this);
	release();
}

const char * PhysXSceneContext::StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysXSceneContext::OnInit(PxPhysics * sdk, PxDefaultCpuDispatcher * dispatcher)
{
	PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (PxVec3&)PhysXContext::Gravity;
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_Scene.reset(sdk->createScene(sceneDesc)), m_Scene))
	{
		THROW_CUSEXCEPTION(_T("sdk->createScene failed"));
	}

	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	//m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

	return true;
}

void PhysXSceneContext::OnShutdown(void)
{
	//_ASSERT(!m_Scene || 0 == m_Scene->getNbActors(PxActorTypeSelectionFlags(0xff)));

	m_Scene.reset();
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
		m_Sync.Wait(INFINITE);
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
	m_Scene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysXSceneContext::SubstepDone(StepperTask * ownerTask)
{
	m_Scene->fetchResults(true, &m_ErrorState);

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

void PhysXSceneContext::PushRenderBuffer(my::DrawHelper * drawHelper)
{
	const PxRenderBuffer & debugRenderable = m_Scene->getRenderBuffer();

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
			drawHelper->PushLine((my::Vector3 &)line.pos0, (my::Vector3 &)line.pos1, line.color0);
		}
	}

	//const PxU32 numTriangles = debugRenderable.getNbTriangles();
	//if(numTriangles)
	//{
	//	const PxDebugTriangle* PX_RESTRICT triangles = debugRenderable.getTriangles();
	//	for(PxU32 i=0; i<numTriangles; i++)
	//	{
	//		const PxDebugTriangle& triangle = triangles[i];
	//	}
	//}
}

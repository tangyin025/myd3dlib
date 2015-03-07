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

void PhysXContext::CookTriangleMesh(my::OStreamPtr ostream, my::MeshPtr mesh)
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

void PhysXContext::CookTriangleMeshToFile(std::string path, my::MeshPtr mesh)
{
	CookTriangleMesh(my::FileOStream::Open(ms2ts(path).c_str()), mesh);
}

PxTriangleMesh * PhysXContext::CreateTriangleMesh(my::IStreamPtr istream)
{
	// ! should be call at resource thread
	PxTriangleMesh * ret = m_sdk->createTriangleMesh(PhysXIStream(istream));
	return ret;
}

void PhysXContext::CookClothFabric(my::OStreamPtr ostream, my::MeshPtr mesh, WORD PositionOffset)
{
	PxClothMeshDesc desc;
	desc.points.data = (unsigned char *)mesh->LockVertexBuffer() + PositionOffset;
	desc.points.count = mesh->GetNumVertices();
	desc.points.stride = mesh->GetNumBytesPerVertex();

	desc.triangles.data = mesh->LockIndexBuffer();
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

	m_Cooking->cookClothFabric(desc, (PxVec3&)Gravity, PhysXOStream(ostream));
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
}

void PhysXContext::CookClothFabricToFile(std::string path, my::MeshPtr mesh, WORD PositionOffset)
{
	CookClothFabric(my::FileOStream::Open(ms2ts(path).c_str()), mesh, PositionOffset);
}

PxClothFabric * PhysXContext::CreateClothFabric(my::IStreamPtr istream)
{
	PxClothFabric * ret = m_sdk->createClothFabric(PhysXIStream(istream));
	return ret;
}

void PhysXSceneContext::StepperTask::run(void)
{
	m_PxScene->SubstepDone(this);
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
	if(!(m_PxScene.reset(sdk->createScene(sceneDesc)), m_PxScene))
	{
		THROW_CUSEXCEPTION(_T("sdk->createScene failed"));
	}

	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

	return true;
}

void PhysXSceneContext::OnShutdown(void)
{
	//_ASSERT(!m_PxScene || 0 == m_PxScene->getNbActors(PxActorTypeSelectionFlags(0xff)));

	m_PxScene.reset();
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

	m_Completion0.setContinuation(*m_PxScene->getTaskManager(), NULL);

	Substep(m_Completion0);

	m_Completion0.removeReference();

	return true;
}

void PhysXSceneContext::Substep(StepperTask & completionTask)
{
	m_PxScene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysXSceneContext::SubstepDone(StepperTask * ownerTask)
{
	m_PxScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

	OnPxThreadSubstep(m_Timer.m_Interval);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		m_Sync.SetEvent();
		return;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	StepperTask & task = (ownerTask == &m_Completion0 ? m_Completion1 : m_Completion0);

	task.setContinuation(*m_PxScene->getTaskManager(), NULL);

	Substep(task);

	task.removeReference();
}

void PhysXSceneContext::OnPxThreadSubstep(float dtime)
{
}

void PhysXSceneContext::PushRenderBuffer(my::DrawHelper * drawHelper)
{
	const PxRenderBuffer & debugRenderable = m_PxScene->getRenderBuffer();

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

void ClothMeshComponentLOD::CreateCloth(PhysXContext * px_sdk,
	my::OgreMeshPtr mesh, DWORD AttribId, const my::BoneHierarchy & hierarchy, DWORD root_i, const PxClothCollisionData& collData)
{
	if (m_VertexData.empty())
	{
		m_VertexStride = mesh->GetNumBytesPerVertex();
		m_VertexData.resize(mesh->GetNumVertices() * m_VertexStride);
		memcpy(&m_VertexData[0], mesh->LockVertexBuffer(), m_VertexData.size());
		mesh->UnlockVertexBuffer();

		m_IndexData.resize(mesh->GetNumFaces() * 3);
		if (m_IndexData.size() > USHRT_MAX)
		{
			THROW_CUSEXCEPTION(str_printf(_T("create deformation mesh with overflow index size %u"), m_IndexData.size()));
		}
		VOID * pIndices = mesh->LockIndexBuffer();
		for (unsigned int face_i = 0; face_i < mesh->GetNumFaces(); face_i++)
		{
			if(mesh->GetOptions() & D3DXMESH_32BIT)
			{
				m_IndexData[face_i * 3 + 0] = (WORD)*((DWORD *)pIndices + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = (WORD)*((DWORD *)pIndices + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = (WORD)*((DWORD *)pIndices + face_i * 3 + 2);
			}
			else
			{
				m_IndexData[face_i * 3 + 0] = *((WORD *)pIndices + face_i * 3 + 0);
				m_IndexData[face_i * 3 + 1] = *((WORD *)pIndices + face_i * 3 + 1);
				m_IndexData[face_i * 3 + 2] = *((WORD *)pIndices + face_i * 3 + 2);
			}
		}
		mesh->UnlockIndexBuffer();

		m_AttribTable = mesh->m_AttribTable;
		std::vector<D3DVERTEXELEMENT9> velist(MAX_FVF_DECL_SIZE);
		mesh->GetDeclaration(&velist[0]);
		HRESULT hr;
		if (FAILED(hr = mesh->GetDevice()->CreateVertexDeclaration(&velist[0], &m_Decl)))
		{
			THROW_D3DEXCEPTION(hr);
		}
	}

	m_VertexElems = mesh->m_VertexElems;

	m_particles.resize(mesh->GetNumVertices());
	unsigned char * pVertices = (unsigned char *)&m_VertexData[0];
	for(unsigned int i = 0; i < m_particles.size(); i++) {
		unsigned char * pVertex = pVertices + i * m_VertexStride;
		m_particles[i].pos = (PxVec3 &)m_VertexElems.GetPosition(pVertex);
		unsigned char * pIndices = (unsigned char *)&m_VertexElems.GetBlendIndices(pVertex);
		BOOST_STATIC_ASSERT(4 == my::D3DVertexElementSet::MAX_BONE_INDICES);
		m_particles[i].invWeight = (
			pIndices[0] == root_i || hierarchy.HaveChild(root_i, pIndices[0]) ||
			pIndices[1] == root_i || hierarchy.HaveChild(root_i, pIndices[1]) ||
			pIndices[2] == root_i || hierarchy.HaveChild(root_i, pIndices[2]) ||
			pIndices[3] == root_i || hierarchy.HaveChild(root_i, pIndices[3])) ? 1 / 1.0f : 0.0f;
	}

	my::MemoryOStreamPtr ofs(new my::MemoryOStream);
	px_sdk->CookClothFabric(
		ofs, mesh, m_VertexElems.elems[D3DDECLUSAGE_POSITION][0].Offset);
	my::IStreamPtr ifs(new my::MemoryIStream(&(*ofs->m_cache)[0], ofs->m_cache->size()));
	physx_ptr<PxClothFabric> fabric(px_sdk->CreateClothFabric(ifs));
	m_cloth.reset(px_sdk->m_sdk->createCloth(
		PxTransform(PxVec3(0,0,0), PxQuat(0,0,0,1)), *fabric, &m_particles[0], collData, PxClothFlags()));
}

void ClothMeshComponentLOD::UpdateCloth(const my::TransformList & dualQuaternionList)
{
	_ASSERT(m_particles.size() == m_VertexData.size() / m_VertexStride);
	PxClothReadData * readData = m_cloth->lockClothReadData();
	if (readData)
	{
		unsigned char * pVertices = &m_VertexData[0];
		const DWORD NbParticles = m_cloth->getNbParticles();
		m_NewParticles.resize(NbParticles);
		for (unsigned int i = 0; i < NbParticles; i++)
		{
			void * pVertex = pVertices + i * m_VertexStride;
			m_NewParticles[i].invWeight = readData->particles[i].invWeight;
			if (0 == m_NewParticles[i].invWeight)
			{
				my::Vector3 pos;
				my::BoneList::TransformVertexWithDualQuaternionList(pos,
					(my::Vector3 &)m_particles[i].pos,
					m_VertexElems.GetBlendIndices(pVertex),
					m_VertexElems.GetBlendWeight(pVertex), dualQuaternionList);
				m_NewParticles[i].pos = (PxVec3 &)pos;
			}
			else
			{
				m_NewParticles[i].pos = readData->particles[i].pos;
			}
			m_VertexElems.SetPosition(pVertex, (my::Vector3 &)m_NewParticles[i].pos);
		}
		readData->unlock();
		m_cloth->setParticles(&m_NewParticles[0], NULL);
		m_cloth->setTargetPose(PxTransform((PxMat44 &)m_owner->m_World));

		my::OgreMesh::ComputeNormalFrame(
			pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);

		my::OgreMesh::ComputeTangentFrame(
			pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);
	}
}

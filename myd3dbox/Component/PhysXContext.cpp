#include "StdAfx.h"
#include "PhysXContext.h"
#include "Component.h"
#include "Terrain.h"

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

bool PhysXContext::Init(void)
{
	if(!(m_Foundation.reset(PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, *this)), m_Foundation))
	{
		THROW_CUSEXCEPTION("PxCreateFoundation failed");
	}

	if(!(m_sdk.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(),
#ifdef _DEBUG
		true,
#else
		false,
#endif
		NULL)), m_sdk))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams())), m_Cooking))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!PxInitExtensions(*m_sdk))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	if(!(m_CpuDispatcher.reset(PxDefaultCpuDispatcherCreate(1, NULL)), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}

	if(!(m_ControllerMgr.reset(PxCreateControllerManager(*m_Foundation)), m_ControllerMgr))
	{
		THROW_CUSEXCEPTION("PxCreateControllerManager failed");
	}
	return true;
}

void PhysXContext::Shutdown(void)
{
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

void PhysXContext::ExportStaticCollision(my::OctTree & octRoot, const char * path)
{
	struct CallBack : public my::IQueryCallback
	{
		PxPhysics * sdk;
		PxCooking * cooking;
		PxCollection * collection;

		std::map<std::string, PhysXPtr<PxTriangleMesh> > triangle_mesh_map;
		std::vector<PhysXPtr<PxHeightField> > heightfields;
		std::vector<PhysXPtr<PxMaterial> > materials;
		std::vector<PhysXPtr<PxRigidStatic> > actors;

		CallBack(PxPhysics * _sdk, PxCooking * _cooking, PxCollection * _collection)
			: sdk(_sdk)
			, cooking(_cooking)
			, collection(_collection)
		{
		}
		void operator() (my::OctComponent * oct_cmp, my::IntersectionTests::IntersectionType)
		{
			Component * cmp = dynamic_cast<Component *>(oct_cmp);
			_ASSERT(cmp);
			switch(cmp->m_Type)
			{
			case Component::ComponentTypeMesh:
				{
					MeshComponent * mesh_cmp = dynamic_cast<MeshComponent *>(cmp);
					_ASSERT(mesh_cmp);
					if (!mesh_cmp->m_StaticCollision)
					{
						break;
					}

					my::OgreMeshPtr mesh = mesh_cmp->m_lods[mesh_cmp->m_lod].m_MeshRes.m_Res;
					if (!mesh)
					{
						break;
					}
					std::string mesh_key = my::ResourceMgr::getSingleton().GetResourceKey(mesh);
					if (mesh_key.empty())
					{
						break;
					}
					PhysXPtr<PxTriangleMesh> trianglemesh;
					std::map<std::string, PhysXPtr<PxTriangleMesh> >::iterator triangle_mesh_iter = triangle_mesh_map.find(mesh_key);
					if (triangle_mesh_iter != triangle_mesh_map.end())
					{
						trianglemesh = triangle_mesh_iter->second;
					}
					else
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
						PxDefaultMemoryOutputStream writeBuffer;
						bool status = cooking->cookTriangleMesh(desc, writeBuffer);
						mesh->UnlockIndexBuffer();
						mesh->UnlockVertexBuffer();
						if (!status)
						{
							break;
						}
						PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
						trianglemesh.reset(sdk->createTriangleMesh(readBuffer));
						triangle_mesh_map.insert(std::make_pair(mesh_key, trianglemesh));
					}
					trianglemesh->collectForExport(*collection);

					PhysXPtr<PxMaterial> material(sdk->createMaterial(0.5f, 0.5f, 0.5f));
					material->collectForExport(*collection);
					materials.push_back(material);

					my::Vector3 pos, scale; my::Quaternion rot;
					mesh_cmp->m_World.Decompose(scale, rot, pos);
					PhysXPtr<PxRigidStatic> actor(sdk->createRigidStatic(PxTransform((PxVec3&)pos, (PxQuat&)rot)));
					PxMeshScale mesh_scaling((PxVec3&)scale, PxQuat::createIdentity());
					PxShape * shape = actor->createShape(
						PxTriangleMeshGeometry(trianglemesh.get(), mesh_scaling),
						*material, PxTransform::createIdentity());
					//shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
					actor->collectForExport(*collection);
					actors.push_back(actor);
				}
				break;
			case Component::ComponentTypeTerrain:
				{
					Terrain * terrain = dynamic_cast<Terrain *>(cmp);
					_ASSERT(terrain);
					if (!terrain->m_StaticCollision)
					{
						break;
					}

					D3DLOCKED_RECT lrc = terrain->m_HeightMap.LockRect(NULL, 0, 0);
					std::vector<PxHeightFieldSample> Samples(
						(terrain->m_RowChunks * terrain->m_ChunkRows + 1) * (terrain->m_ColChunks * terrain->m_ChunkRows + 1));
					for (unsigned int i = 0; i < terrain->m_RowChunks * terrain->m_ChunkRows + 1; i++)
					{
						for (unsigned int j = 0; j < terrain->m_ColChunks * terrain->m_ChunkRows + 1; j++)
						{
							Samples[i * (terrain->m_ColChunks * terrain->m_ChunkRows + 1) + j].height = terrain->GetSampleHeight(lrc.pBits, lrc.Pitch, i, j);
							Samples[i * (terrain->m_ColChunks * terrain->m_ChunkRows + 1) + j].materialIndex0 = PxBitAndByte(0, false);
							Samples[i * (terrain->m_ColChunks * terrain->m_ChunkRows + 1) + j].materialIndex1 = PxBitAndByte(0, false);
						}
					}
					terrain->m_HeightMap.UnlockRect(0);
					PxHeightFieldDesc hfDesc;
					hfDesc.nbRows             = terrain->m_RowChunks * terrain->m_ChunkRows + 1;
					hfDesc.nbColumns          = terrain->m_ColChunks * terrain->m_ChunkRows + 1;
					hfDesc.format             = PxHeightFieldFormat::eS16_TM;
					hfDesc.samples.data       = &Samples[0];
					hfDesc.samples.stride     = sizeof(Samples[0]);
					PhysXPtr<PxHeightField> heightfield(sdk->createHeightField(hfDesc));
					heightfield->collectForExport(*collection);
					heightfields.push_back(heightfield);

					PhysXPtr<PxMaterial> material(sdk->createMaterial(0.5f, 0.5f, 0.5f));
					material->collectForExport(*collection);
					materials.push_back(material);

					my::Vector3 pos, scale; my::Quaternion rot;
					terrain->m_World.Decompose(scale, rot, pos);
					PhysXPtr<PxRigidStatic> actor(sdk->createRigidStatic(PxTransform((PxVec3&)pos, (PxQuat&)rot)));
					PxShape * shape = actor->createShape(
						PxHeightFieldGeometry(heightfield.get(), PxMeshGeometryFlags(), terrain->m_HeightScale * scale.y, scale.x, scale.z),
						*material, PxTransform::createIdentity());
					//shape->setFlag(PxShapeFlag::eVISUALIZATION, false);
					actor->collectForExport(*collection);
					actors.push_back(actor);
				}
				break;
			}
		}
	};

	PxDefaultFileOutputStream ostr(path);
	PhysXPtr<PxCollection> collection(m_sdk->createCollection());
	CallBack cb(m_sdk.get(), m_Cooking.get(), collection.get());
	octRoot.QueryComponentAll(&cb);
	collection->serialize(ostr, false);
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

bool PhysXSceneContext::Init(PxPhysics * sdk, PxDefaultCpuDispatcher * dispatcher)
{
	PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (PxVec3&)PhysXContext::Gravity;
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	if(!(m_PxScene.reset(sdk->createScene(sceneDesc)), m_PxScene))
	{
		THROW_CUSEXCEPTION("sdk->createScene failed");
	}

	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS, 1);
	//m_PxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1);

	return true;
}

void PhysXSceneContext::Shutdown(void)
{
	//_ASSERT(!m_PxScene || 0 == m_PxScene->getNbActors(PxActorTypeSelectionFlags(0xff)));
	ClearAllActors();
	ReleaseSerializeObjs();
	m_PxScene.reset();
}

void PhysXSceneContext::TickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	m_WaitForResults = Advance(dtime);
}

void PhysXSceneContext::TickPostRender(float dtime)
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

bool PhysXSceneContext::AdvanceSync(float dtime)
{
	m_PxScene->simulate(dtime, NULL, 0, 0, true);

	return m_PxScene->fetchResults(true, 0);
}

void PhysXSceneContext::Substep(StepperTask & completionTask)
{
	m_PxScene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysXSceneContext::SubstepDone(StepperTask * ownerTask)
{
	m_PxScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

	// ! take care of multi thread
	m_EventPxThreadSubstep(m_Timer.m_Interval);

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

void PhysXSceneContext::Flush(void)
{
	m_PxScene->flush(false);
}

void PhysXSceneContext::ClearAllActors(void)
{
	PxActorTypeSelectionFlags flags(PxActorTypeSelectionFlag::eRIGID_STATIC | PxActorTypeSelectionFlag::eRIGID_DYNAMIC);
	PxU32 numActors = m_PxScene->getNbActors(flags);
	if (numActors > 0)
	{
		std::vector<PxActor *> actorList(numActors);
		m_PxScene->getActors(flags, &actorList[0], actorList.size(), 0);
		for (PxU32 i = 0; i < actorList.size(); i++)
		{
			actorList[i]->release();
		}
	}
}

void PhysXSceneContext::ReleaseSerializeObjs(void)
{
	unsigned int numObjs = m_SerializeObjs.size();
	for (unsigned int i = 0; i < numObjs; i++)
	{
		switch (m_SerializeObjs[i]->getConcreteType())
		{
		case PxConcreteType::eMATERIAL:
			m_SerializeObjs[i]->is<PxMaterial>()->release();
			break;
		case PxConcreteType::eHEIGHTFIELD:
			m_SerializeObjs[i]->is<PxHeightField>()->release();
			break;
		case PxConcreteType::eCONVEX_MESH:
			m_SerializeObjs[i]->is<PxConvexMesh>()->release();
			break;
		case PxConcreteType::eTRIANGLE_MESH:
			m_SerializeObjs[i]->is<PxTriangleMesh>()->release();
			break;
		}
	}
	m_SerializeObjs.clear();
	m_SerializeBuffs.clear();
}

void PhysXSceneContext::ImportStaticCollision(const char * path)
{
	my::IStreamPtr istr = my::ResourceMgr::getSingleton().OpenIStream(path);
	boost::shared_ptr<unsigned char> buff((unsigned char *)_aligned_malloc(istr->GetSize(), PX_SERIAL_FILE_ALIGN), _aligned_free);
	istr->read(buff.get(), istr->GetSize());
	PhysXPtr<PxCollection> collection(PhysXContext::getSingleton().m_sdk->createCollection());
	collection->deserialize(buff.get(), NULL, NULL);
	PhysXContext::getSingleton().m_sdk->addCollection(*collection, *m_PxScene);
	m_SerializeBuffs.push_back(buff);

	PxU32 numObjs = collection->getNbObjects();
	for(PxU32 i = 0; i < numObjs; i++)
	{
		PxSerializable* object = collection->getObject(i);
		switch (object->getConcreteType())
		{
		case PxConcreteType::eMATERIAL:
		case PxConcreteType::eHEIGHTFIELD:
		case PxConcreteType::eCONVEX_MESH:
		case PxConcreteType::eTRIANGLE_MESH:
			m_SerializeObjs.push_back(object);
			break;
		}
	}
}

#include "StdAfx.h"
#include "PhysXContext.h"
#include "Component.h"
#include "Terrain.h"
#include <extensions/PxCollectionExt.h>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

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

	physx::PxTolerancesScale scale;
	if(!(m_sdk.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, scale,
#ifdef _DEBUG
		true,
#else
		false,
#endif
		NULL)), m_sdk))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, physx::PxCookingParams(scale))), m_Cooking))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!PxInitExtensions(*m_sdk))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	if(!(m_CpuDispatcher.reset(physx::PxDefaultCpuDispatcherCreate(1, NULL)), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}

	//if(!(m_ControllerMgr.reset(PxCreateControllerManager(*m_Foundation)), m_ControllerMgr))
	//{
	//	THROW_CUSEXCEPTION("PxCreateControllerManager failed");
	//}
	return true;
}

void PhysXContext::Shutdown(void)
{
	if(m_sdk)
	{
		PxCloseExtensions();
	}

	//m_ControllerMgr.reset();

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_sdk.reset();

	m_Foundation.reset();
}

template<>
void PhysXContext::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
}

template<>
void PhysXContext::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
}

void PhysXContext::ExportStaticCollision(OctTree & octRoot, const char * path)
{
	struct CallBack : public my::IQueryCallback
	{
		physx::PxPhysics * sdk;
		physx::PxCooking * cooking;
		physx::PxCollection * collection;

		std::map<std::string, PhysXPtr<physx::PxTriangleMesh> > triangle_mesh_map;
		std::vector<PhysXPtr<physx::PxHeightField> > heightfields;
		std::vector<PhysXPtr<physx::PxMaterial> > materials;
		std::vector<PhysXPtr<physx::PxRigidStatic> > actors;

		CallBack(physx::PxPhysics * _sdk, physx::PxCooking * _cooking, physx::PxCollection * _collection)
			: sdk(_sdk)
			, cooking(_cooking)
			, collection(_collection)
		{
		}

		void ExportComponent(Component * cmp)
		{
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
					if (!mesh_cmp->m_MeshRes.m_Res)
					{
						break;
					}
					std::string mesh_key = my::ResourceMgr::getSingleton().GetResourceKey(mesh_cmp->m_MeshRes.m_Res);
					if (mesh_key.empty())
					{
						break;
					}
					PhysXPtr<physx::PxTriangleMesh> trianglemesh;
					std::map<std::string, PhysXPtr<physx::PxTriangleMesh> >::iterator triangle_mesh_iter = triangle_mesh_map.find(mesh_key);
					if (triangle_mesh_iter != triangle_mesh_map.end())
					{
						trianglemesh = triangle_mesh_iter->second;
					}
					else
					{
						physx::PxTriangleMeshDesc desc;
						desc.points.count = mesh_cmp->m_MeshRes.m_Res->GetNumVertices();
						desc.points.stride = mesh_cmp->m_MeshRes.m_Res->GetNumBytesPerVertex();
						desc.points.data = mesh_cmp->m_MeshRes.m_Res->LockVertexBuffer();
						desc.triangles.count = mesh_cmp->m_MeshRes.m_Res->GetNumFaces();
						if (mesh_cmp->m_MeshRes.m_Res->GetOptions() & D3DXMESH_32BIT)
						{
							desc.triangles.stride = 3 * sizeof(DWORD);
						}
						else
						{
							desc.triangles.stride = 3 * sizeof(WORD);
							desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
						}
						desc.triangles.data = mesh_cmp->m_MeshRes.m_Res->LockIndexBuffer();
						physx::PxDefaultMemoryOutputStream writeBuffer;
						bool status = cooking->cookTriangleMesh(desc, writeBuffer);
						mesh_cmp->m_MeshRes.m_Res->UnlockIndexBuffer();
						mesh_cmp->m_MeshRes.m_Res->UnlockVertexBuffer();
						if (!status)
						{
							break;
						}
						physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
						trianglemesh.reset(sdk->createTriangleMesh(readBuffer));
						triangle_mesh_map.insert(std::make_pair(mesh_key, trianglemesh));
					}

					PhysXPtr<physx::PxMaterial> material(sdk->createMaterial(0.5f, 0.5f, 0.5f));
					materials.push_back(material);

					my::Vector3 pos, scale; my::Quaternion rot;
					mesh_cmp->m_World.Decompose(scale, rot, pos);
					PhysXPtr<physx::PxRigidStatic> actor(sdk->createRigidStatic(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot)));
					physx::PxMeshScale mesh_scaling((physx::PxVec3&)scale, physx::PxQuat::createIdentity());
					physx::PxShape * shape = actor->createShape(
						physx::PxTriangleMeshGeometry(trianglemesh.get(), mesh_scaling),
						*material, physx::PxTransform::createIdentity());
					//shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
					collection->add(*actor);
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
					std::vector<physx::PxHeightFieldSample> Samples(
						(terrain->m_RowChunks * terrain->m_ChunkRows + 1) * (terrain->m_ColChunks * terrain->m_ChunkRows + 1));
					for (unsigned int i = 0; i < terrain->m_RowChunks * terrain->m_ChunkRows + 1; i++)
					{
						for (unsigned int j = 0; j < terrain->m_ColChunks * terrain->m_ChunkRows + 1; j++)
						{
							Samples[i * (terrain->m_ColChunks * terrain->m_ChunkRows + 1) + j].height = terrain->GetSampleHeight(lrc.pBits, lrc.Pitch, i, j);
							Samples[i * (terrain->m_ColChunks * terrain->m_ChunkRows + 1) + j].materialIndex0 = physx::PxBitAndByte(0, false);
							Samples[i * (terrain->m_ColChunks * terrain->m_ChunkRows + 1) + j].materialIndex1 = physx::PxBitAndByte(0, false);
						}
					}
					terrain->m_HeightMap.UnlockRect(0);
					physx::PxHeightFieldDesc hfDesc;
					hfDesc.nbRows             = terrain->m_RowChunks * terrain->m_ChunkRows + 1;
					hfDesc.nbColumns          = terrain->m_ColChunks * terrain->m_ChunkRows + 1;
					hfDesc.format             = physx::PxHeightFieldFormat::eS16_TM;
					hfDesc.samples.data       = &Samples[0];
					hfDesc.samples.stride     = sizeof(Samples[0]);
					PhysXPtr<physx::PxHeightField> heightfield(sdk->createHeightField(hfDesc));
					heightfields.push_back(heightfield);

					PhysXPtr<physx::PxMaterial> material(sdk->createMaterial(0.5f, 0.5f, 0.5f));
					materials.push_back(material);

					my::Vector3 pos, scale; my::Quaternion rot;
					terrain->m_World.Decompose(scale, rot, pos);
					PhysXPtr<physx::PxRigidStatic> actor(sdk->createRigidStatic(physx::PxTransform((physx::PxVec3&)pos, (physx::PxQuat&)rot)));
					physx::PxShape * shape = actor->createShape(
						physx::PxHeightFieldGeometry(heightfield.get(), physx::PxMeshGeometryFlags(), terrain->m_HeightScale * scale.y, scale.x, scale.z),
						*material, physx::PxTransform::createIdentity());
					//shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
					collection->add(*actor);
					actors.push_back(actor);
				}
				break;
			}

			Component::ComponentPtrList::iterator cmp_iter = cmp->m_Cmps.begin();
			for (; cmp_iter != cmp->m_Cmps.end(); cmp_iter++)
			{
				ExportComponent(cmp_iter->get());
			}
		}

		void operator() (my::OctActor * oct_actor, my::IntersectionTests::IntersectionType)
		{
			ExportComponent(dynamic_cast<Component *>(oct_actor));
		}
	};

	PhysXPtr<physx::PxSerializationRegistry> registry(physx::PxSerialization::createSerializationRegistry(*m_sdk));
	PhysXPtr<physx::PxCollection> collection(PxCreateCollection());
	CallBack cb(m_sdk.get(), m_Cooking.get(), collection.get());
	octRoot.QueryActorAll(&cb);
	physx::PxSerialization::complete(*collection, *registry);
	physx::PxDefaultFileOutputStream ostr(path);
	physx::PxSerialization::serializeCollectionToXml(ostr, *collection, *registry);
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

bool PhysXSceneContext::Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher)
{
	physx::PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (physx::PxVec3&)PhysXContext::Gravity;
	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
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
	const physx::PxRenderBuffer & debugRenderable = m_PxScene->getRenderBuffer();

	const physx::PxU32 numPoints = debugRenderable.getNbPoints();
	if(numPoints)
	{
		const physx::PxDebugPoint* PX_RESTRICT points = debugRenderable.getPoints();
		for(physx::PxU32 i=0; i<numPoints; i++)
		{
			const physx::PxDebugPoint& point = points[i];
		}
	}

	const physx::PxU32 numLines = debugRenderable.getNbLines();
	if(numLines)
	{
		const physx::PxDebugLine* PX_RESTRICT lines = debugRenderable.getLines();
		for(physx::PxU32 i=0; i<numLines; i++)
		{
			const physx::PxDebugLine& line = lines[i];
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

void PhysXSceneContext::ReleaseSerializeObjs(void)
{
	if (m_collection)
	{
		physx::PxCollectionExt::releaseObjects(*m_collection);
		m_collection.reset();
	}
}

void PhysXSceneContext::ImportStaticCollision(const char * path)
{
	my::IStreamPtr istr = my::ResourceMgr::getSingleton().OpenIStream(path);
	boost::shared_ptr<unsigned char> buff((unsigned char *)_aligned_malloc(istr->GetSize(), PX_SERIAL_FILE_ALIGN), _aligned_free);
	istr->read(buff.get(), istr->GetSize());
	physx::PxDefaultMemoryInputData input(buff.get(), istr->GetSize());
	PhysXPtr<physx::PxSerializationRegistry> registry(physx::PxSerialization::createSerializationRegistry(*PhysXContext::getSingleton().m_sdk));
	m_collection.reset(physx::PxSerialization::createCollectionFromXml(input, *PhysXContext::getSingleton().m_Cooking, *registry, NULL));
	m_PxScene->addCollection(*m_collection);
}

#include "PhysXContext.h"
#include "FModContext.h"
#include "Component.h"
#include "Terrain.h"
#include "myDxutApp.h"
#include "libc.h"
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
	return true;
}

void PhysXContext::Shutdown(void)
{
	if(m_sdk)
	{
		PxCloseExtensions();
	}

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_sdk.reset();

	m_Foundation.reset();
}

void PhysXContext::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	switch (code)
	{
	case physx::PxErrorCode::eDEBUG_INFO:
		my::D3DContext::getSingleton().m_EventLog(str_printf("%s (%d) : info: %s", file, line, message).c_str());
		break;

	case physx::PxErrorCode::eDEBUG_WARNING:
	case physx::PxErrorCode::ePERF_WARNING:
		my::D3DContext::getSingleton().m_EventLog(str_printf("%s (%d) : warning: %s", file, line, message).c_str());
		break;

	default:
		my::D3DContext::getSingleton().m_EventLog(str_printf("%s, (%d) : error: %s", file, line, message).c_str());
		break;
	}
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
	sceneDesc.simulationEventCallback = this;
	sceneDesc.cpuDispatcher = dispatcher;
	//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = filter;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	m_PxScene.reset(sdk->createScene(sceneDesc));
	if (!m_PxScene)
	{
		THROW_CUSEXCEPTION("sdk->createScene failed");
	}

	m_ControllerMgr.reset(PxCreateControllerManager(*m_PxScene));
	if (!m_ControllerMgr)
	{
		THROW_CUSEXCEPTION("PxCreateControllerManager failed");
	}

	m_Registry.reset(physx::PxSerialization::createSerializationRegistry(*PhysXContext::getSingleton().m_sdk));
	if (!m_Registry)
	{
		THROW_CUSEXCEPTION("physx::PxSerialization::createSerializationRegistry failed");
	}

	m_Collection.reset(PxCreateCollection());
	if (!m_Collection)
	{
		THROW_CUSEXCEPTION("PxCreateCollection failed");
	}

	return true;
}

float PhysXSceneContext::GetVisualizationParameter(void) const
{
	return m_PxScene->getVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES);
}

void PhysXSceneContext::SetVisualizationParameter(float param)
{
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, param);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, param);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_FNORMALS, param);
	m_PxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, param);
}

template<>
void PhysXSceneContext::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	const_cast<PhysXSceneContext *>(this)->m_Registry.reset(physx::PxSerialization::createSerializationRegistry(*PhysXContext::getSingleton().m_sdk));
	const_cast<PhysXSceneContext *>(this)->m_Collection.reset(PxCreateCollection());
	TriangleMeshMap::const_iterator tri_mesh_iter = m_TriangleMeshes.begin();
	for (; tri_mesh_iter != m_TriangleMeshes.end(); tri_mesh_iter++)
	{
		m_Collection->add(*tri_mesh_iter->second);
	}
	physx::PxDefaultMemoryOutputStream ostr;
	physx::PxSerialization::createSerialObjectIds(*m_Collection, physx::PxSerialObjectId(1));
	physx::PxSerialization::serializeCollectionToBinary(ostr, *m_Collection, *m_Registry);
	unsigned int StreamBuffSize = ostr.getSize();
	ar << BOOST_SERIALIZATION_NVP(StreamBuffSize);
	ar << boost::serialization::make_nvp("StreamBuff", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	tri_mesh_iter = m_TriangleMeshes.begin();
	for (; tri_mesh_iter != m_TriangleMeshes.end(); tri_mesh_iter++)
	{
		std::string key = tri_mesh_iter->first;
		ar << BOOST_SERIALIZATION_NVP(key);
		physx::PxSerialObjectId id = m_Collection->getId(*tri_mesh_iter->second);
		ar << BOOST_SERIALIZATION_NVP(id);
	}
}

template<>
void PhysXSceneContext::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	unsigned int StreamBuffSize;
	ar >> BOOST_SERIALIZATION_NVP(StreamBuffSize);
	m_SerializeBuff.reset((unsigned char *)_aligned_malloc(StreamBuffSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
	ar >> boost::serialization::make_nvp("StreamBuff", boost::serialization::binary_object(m_SerializeBuff.get(), StreamBuffSize));
	m_Registry.reset(physx::PxSerialization::createSerializationRegistry(*PhysXContext::getSingleton().m_sdk));
	m_Collection.reset(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *m_Registry, NULL));
	const unsigned int numObjs = m_Collection->getNbObjects();
	for (unsigned int i = 0; i < numObjs; i++)
	{
		std::string key;
		ar >> BOOST_SERIALIZATION_NVP(key);
		physx::PxSerialObjectId id;
		ar >> BOOST_SERIALIZATION_NVP(id);
		PhysXPtr<physx::PxTriangleMesh> triangle_mesh(m_Collection->find(id)->is<physx::PxTriangleMesh>());
		m_TriangleMeshes.insert(std::make_pair(key, triangle_mesh));
	}
}

void PhysXSceneContext::ClearSerializedObjs(void)
{
	m_TriangleMeshes.clear();

	m_Collection.reset();

	m_Registry.reset();

	m_SerializeBuff.reset();
}

void PhysXSceneContext::Shutdown(void)
{
	m_EventPxThreadSubstep.disconnect_all_slots();
	ClearSerializedObjs();
	//_ASSERT(!m_PxScene || 0 == m_PxScene->getNbActors(PxActorTypeSelectionFlags(0xff)));
	m_ControllerMgr.reset();
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
	m_Timer.m_RemainingTime = my::Min(0.1f, m_Timer.m_RemainingTime + dtime);

	if(m_Timer.m_RemainingTime < m_Timer.m_Interval)
	{
		return false;
	}

	m_Timer.m_RemainingTime -= m_Timer.m_Interval;

	m_PxScene->simulate(m_Timer.m_Interval, NULL, 0, 0, true);

	m_PxScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

	m_EventPxThreadSubstep(dtime);

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

	// ! be aware of multi thread
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

physx::PxFilterFlags PhysXSceneContext::filter(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	physx::PxU32 constantBlockSize)
{
	pairFlags |= physx::PxPairFlag::eCONTACT_DEFAULT;
	pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	return physx::PxFilterFlags();
}

void PhysXSceneContext::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}

void PhysXSceneContext::onWake(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysXSceneContext::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysXSceneContext::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	std::vector<physx::PxContactPairPoint> contactPoints;
	for (physx::PxU32 i = 0; i < nbPairs; i++)
	{
		physx::PxU32 contactCount = pairs[i].contactCount;
		if (contactCount)
		{
			contactPoints.resize(contactCount);
			pairs[i].extractContacts(&contactPoints[0], contactCount);
			for (physx::PxU32 j = 0; j < contactCount; j++)
			{
				physx::PxVec3 point = contactPoints[j].position;
				physx::PxVec3 impulse = contactPoints[j].impulse;
				physx::PxU32 internalFaceIndex0 = contactPoints[j].internalFaceIndex0;
				physx::PxU32 internalFaceIndex1 = contactPoints[j].internalFaceIndex1;

				FModContext::getSingleton().PlaySound("aaa/untitled/15");
			}
		}
	}
}

void PhysXSceneContext::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
}

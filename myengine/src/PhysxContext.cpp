#include "PhysxContext.h"
#include "FModContext.h"
#include "Component.h"
#include "Terrain.h"
#include "Actor.h"
#include "myDxutApp.h"
#include "libc.h"
#include <extensions/PxCollectionExt.h>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include "CctCharacterControllerManager.h"

const my::Vector3 PhysxContext::Gravity(0.0f, -9.81f, 0.0f);

void * PhysxAllocator::allocate(size_t size, const char * typeName, const char * filename, int line)
{
#ifdef _DEBUG
	return _aligned_malloc_dbg(size, 16, filename, line);
#else
	return _aligned_malloc(size, 16);	
#endif
}

void PhysxAllocator::deallocate(void * ptr)
{
#ifdef _DEBUG
	_aligned_free_dbg(ptr);
#else
	_aligned_free(ptr);
#endif
}

bool PhysxContext::Init(void)
{
	if(!(m_Foundation.reset(PxCreateFoundation(PX_FOUNDATION_VERSION, m_Allocator, *this), PhysxDeleter<physx::PxFoundation>()), m_Foundation))
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
		NULL), PhysxDeleter<physx::PxPhysics>()), m_sdk))
	{
		THROW_CUSEXCEPTION("PxCreatePhysics failed");
	}

	if(!(m_Cooking.reset(PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, physx::PxCookingParams(scale)), PhysxDeleter<physx::PxCooking>()), m_Cooking))
	{
		THROW_CUSEXCEPTION("PxCreateCooking failed");
	}

	if(!PxInitExtensions(*m_sdk, NULL))
	{
		THROW_CUSEXCEPTION("PxInitExtensions failed");
	}

	if(!(m_CpuDispatcher.reset(physx::PxDefaultCpuDispatcherCreate(1, NULL), PhysxDeleter<physx::PxDefaultCpuDispatcher>()), m_CpuDispatcher))
	{
		THROW_CUSEXCEPTION("PxDefaultCpuDispatcherCreate failed");
	}
	return true;
}

void PhysxContext::Shutdown(void)
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

void PhysxContext::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
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

void PhysxSceneContext::StepperTask::run(void)
{
	m_PxScene->SubstepDone(this);
	release();
}

const char * PhysxSceneContext::StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysxSceneContext::Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher)
{
	physx::PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (physx::PxVec3&)PhysxContext::Gravity;
	sceneDesc.simulationEventCallback = this;
	sceneDesc.cpuDispatcher = dispatcher;
	//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = filter;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	m_PxScene.reset(sdk->createScene(sceneDesc), PhysxDeleter<physx::PxScene>());
	if (!m_PxScene)
	{
		THROW_CUSEXCEPTION("sdk->createScene failed");
	}

	m_ControllerMgr.reset(PxCreateControllerManager(*m_PxScene), PhysxDeleter<physx::PxControllerManager>());
	if (!m_ControllerMgr)
	{
		THROW_CUSEXCEPTION("PxCreateControllerManager failed");
	}

	// ! only enable it if experiencing collision problems
	m_ControllerMgr->setTessellation(false, 1.0f);

	return true;
}

float PhysxSceneContext::GetVisualizationParameter(physx::PxVisualizationParameter::Enum paramEnum) const
{
	return m_PxScene->getVisualizationParameter(paramEnum);
}

void PhysxSceneContext::SetVisualizationParameter(physx::PxVisualizationParameter::Enum param, float value)
{
	m_PxScene->setVisualizationParameter(param, value);
}

void PhysxSceneContext::SetControllerDebugRenderingFlags(physx::PxU32 flags)
{
	m_ControllerMgr->setDebugRenderingFlags(physx::PxControllerDebugRenderFlags(flags));
}

template<class Archive>
void PhysxSceneContext::save(Archive & ar, const unsigned int version) const
{
	const_cast<PhysxSceneContext *>(this)->m_Registry.reset(physx::PxSerialization::createSerializationRegistry(*PhysxContext::getSingleton().m_sdk), PhysxDeleter<physx::PxSerializationRegistry>());
	const_cast<PhysxSceneContext *>(this)->m_Collection.reset(PxCreateCollection(), PhysxDeleter<physx::PxCollection>());
	PxObjectMap::const_iterator collection_obj_iter = m_CollectionObjs.begin();
	for (; collection_obj_iter != m_CollectionObjs.end(); collection_obj_iter++)
	{
		m_Collection->add(*collection_obj_iter->second);
	}
	physx::PxDefaultMemoryOutputStream ostr;
	physx::PxSerialization::createSerialObjectIds(*m_Collection, physx::PxSerialObjectId(1));
	physx::PxSerialization::serializeCollectionToBinary(ostr, *m_Collection, *m_Registry);
	unsigned int StreamBuffSize = ostr.getSize();
	ar << BOOST_SERIALIZATION_NVP(StreamBuffSize);
	ar << boost::serialization::make_nvp("StreamBuff", boost::serialization::binary_object(ostr.getData(), ostr.getSize()));
	collection_obj_iter = m_CollectionObjs.begin();
	for (; collection_obj_iter != m_CollectionObjs.end(); collection_obj_iter++)
	{
		std::string key = collection_obj_iter->first;
		ar << BOOST_SERIALIZATION_NVP(key);
		physx::PxSerialObjectId id = m_Collection->getId(*collection_obj_iter->second);
		ar << BOOST_SERIALIZATION_NVP(id);
	}
}

template<class Archive>
void PhysxSceneContext::load(Archive & ar, const unsigned int version)
{
	unsigned int StreamBuffSize;
	ar >> BOOST_SERIALIZATION_NVP(StreamBuffSize);
	m_SerializeBuff.reset((unsigned char *)_aligned_malloc(StreamBuffSize, PX_SERIAL_FILE_ALIGN), _aligned_free);
	ar >> boost::serialization::make_nvp("StreamBuff", boost::serialization::binary_object(m_SerializeBuff.get(), StreamBuffSize));
	m_Registry.reset(physx::PxSerialization::createSerializationRegistry(*PhysxContext::getSingleton().m_sdk), PhysxDeleter<physx::PxSerializationRegistry>());
	m_Collection.reset(physx::PxSerialization::createCollectionFromBinary(m_SerializeBuff.get(), *m_Registry, NULL), PhysxDeleter<physx::PxCollection>());
	const unsigned int numObjs = m_Collection->getNbObjects();
	for (unsigned int i = 0; i < numObjs; i++)
	{
		std::string key;
		ar >> BOOST_SERIALIZATION_NVP(key);
		physx::PxSerialObjectId id;
		ar >> BOOST_SERIALIZATION_NVP(id);
		m_CollectionObjs.insert(std::make_pair(key, boost::shared_ptr<physx::PxBase>(m_Collection->find(id), PhysxDeleter<physx::PxBase>())));
	}
}

template
void PhysxSceneContext::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void PhysxSceneContext::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void PhysxSceneContext::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void PhysxSceneContext::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void PhysxSceneContext::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void PhysxSceneContext::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void PhysxSceneContext::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void PhysxSceneContext::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void PhysxSceneContext::ClearSerializedObjs(void)
{
	m_CollectionObjs.clear();

	m_Collection.reset();

	m_Registry.reset();

	m_SerializeBuff.reset();
}

void PhysxSceneContext::Shutdown(void)
{
	m_EventPxThreadSubstep.disconnect_all_slots();
	ClearSerializedObjs();
	//_ASSERT(!m_PxScene || 0 == m_PxScene->getNbActors(PxActorTypeSelectionFlags(0xff)));
	m_ControllerMgr.reset();
	m_PxScene.reset();
}

void PhysxSceneContext::TickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	mTriggerPairs.clear();

	m_WaitForResults = Advance(dtime);
}

void PhysxSceneContext::TickPostRender(float dtime)
{
	if(m_WaitForResults)
	{
		m_Sync.Wait(INFINITE);

		const physx::PxActiveTransform* activeTransforms = m_PxScene->getActiveTransforms(mActiveTransformCount, 0);
		mBufferedActiveTransforms.resize(mActiveTransformCount);
		if (!mBufferedActiveTransforms.empty())
		{
			physx::PxMemCopy(&mBufferedActiveTransforms[0], activeTransforms, sizeof(physx::PxActiveTransform) * mActiveTransformCount);
		}

		for (physx::PxU32 i = 0; i < mBufferedActiveTransforms.size(); ++i)
		{
			if (std::find(mDeletedActors.begin(), mDeletedActors.end(), mBufferedActiveTransforms[i].actor) == mDeletedActors.end())
			{
				Actor * actor = (Actor *)mBufferedActiveTransforms[i].userData;
				actor->OnPxTransformChanged(mBufferedActiveTransforms[i].actor2World);
			}
		}
		mDeletedActors.clear();
	}
}

bool PhysxSceneContext::Advance(float dtime)
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

bool PhysxSceneContext::AdvanceSync(float dtime)
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

void PhysxSceneContext::Substep(StepperTask & completionTask)
{
	m_PxScene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysxSceneContext::SubstepDone(StepperTask * ownerTask)
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

void PhysxSceneContext::PushRenderBuffer(my::DrawHelper * drawHelper)
{
	const physx::PxRenderBuffer & debugRenderable = m_PxScene->getRenderBuffer();

	//const physx::PxU32 numPoints = debugRenderable.getNbPoints();
	//if(numPoints)
	//{
	//	const physx::PxDebugPoint* PX_RESTRICT points = debugRenderable.getPoints();
	//	for(physx::PxU32 i=0; i<numPoints; i++)
	//	{
	//		const physx::PxDebugPoint& point = points[i];
	//	}
	//}

	physx::PxU32 numLines = debugRenderable.getNbLines();
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

	if (((physx::Cct::CharacterControllerManager *)m_ControllerMgr.get())->mDebugRenderingFlags)
	{
		physx::PxRenderBuffer& controllerDebugRenderable = m_ControllerMgr->getRenderBuffer();

		numLines = controllerDebugRenderable.getNbLines();
		if (numLines)
		{
			const physx::PxDebugLine* PX_RESTRICT lines = controllerDebugRenderable.getLines();
			for (physx::PxU32 i = 0; i < numLines; i++)
			{
				const physx::PxDebugLine& line = lines[i];
				drawHelper->PushLine((my::Vector3 &)line.pos0, (my::Vector3 &)line.pos1, line.color0);
			}
		}
		controllerDebugRenderable.clear();
	}
}

void PhysxSceneContext::Flush(void)
{
	m_PxScene->flushSimulation(false);
}

physx::PxFilterFlags PhysxSceneContext::filter(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	physx::PxU32 constantBlockSize)
{
	// let triggers through
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}

	if (filterData0.word0 && filterData1.word0 && !(filterData0.word0 & filterData1.word0))
	{
		return physx::PxFilterFlag::eSUPPRESS;
	}

	// generate contacts for all that were not filtered above
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	return physx::PxFilterFlag::eDEFAULT;
}

void PhysxSceneContext::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}

void PhysxSceneContext::onWake(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysxSceneContext::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysxSceneContext::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
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

				//FModContext::getSingleton().OnControlSound("demo2_3/untitled/15");
			}
		}
	}
}

void PhysxSceneContext::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
		{
			continue;
		}

		// fetchResults true will block other px thread
		mTriggerPairs.push_back(pairs[i]);
	}
}

void PhysxSceneContext::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
{

}

void PhysxSceneContext::removeRenderActorsFromPhysicsActor(const physx::PxRigidActor * actor)
{
	// check if the actor is in the active transform list and remove
	if (actor->getType() == physx::PxActorType::eRIGID_DYNAMIC)
	{
		for (physx::PxU32 i = 0; i < mActiveTransformCount; i++)
		{
			if (mBufferedActiveTransforms[i].actor == actor)
			{
				mBufferedActiveTransforms[i] = mBufferedActiveTransforms[mActiveTransformCount - 1];
				mActiveTransformCount--;
				break;
			}
		}
		mDeletedActors.push_back(const_cast<physx::PxRigidActor*>(actor));
	}
}

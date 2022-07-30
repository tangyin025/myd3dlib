#include "PhysxContext.h"
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
#include "GuIntersectionTriangleBox.h"
#include "common/GuBoxConversion.h"
#include "intersection/GuIntersectionRayTriangle.h"
#include "NpSpatialIndex.h"
#include "SqAABBPruner.h"

using namespace my;

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

uint32_t PhysxInputData::getLength() const
{
	return m_istr->GetSize();
}

bool PhysxSdk::Init(void)
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

void PhysxSdk::Shutdown(void)
{
	m_CollectionObjs.clear();

	if(m_sdk)
	{
		PxCloseExtensions();
	}

	m_CpuDispatcher.reset();

	m_Cooking.reset();

	m_sdk.reset();

	m_Foundation.reset();
}

void PhysxSdk::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
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

void PhysxScene::StepperTask::run(void)
{
	m_Scene->SubstepDone(this);
	release();
}

const char * PhysxScene::StepperTask::getName(void) const
{
	return "Stepper Task";
}

bool PhysxScene::Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher, const physx::PxSceneFlags & flags, const my::Vector3 & gravity)
{
	_ASSERT(flags.isSet(physx::PxSceneFlag::eENABLE_PCM));

	physx::PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (physx::PxVec3&)gravity;
	sceneDesc.simulationEventCallback = this;
	sceneDesc.contactModifyCallback = this;
	sceneDesc.cpuDispatcher = dispatcher;
	//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = PhysxScene::filter;
	sceneDesc.flags = flags;
	m_PxScene.reset(sdk->createScene(sceneDesc), PhysxDeleter<physx::PxScene>());
	if (!m_PxScene)
	{
		THROW_CUSEXCEPTION("sdk->createScene failed");
	}
	m_PxScene->userData = this;

	m_ControllerMgr.reset(PxCreateControllerManager(*m_PxScene), PhysxDeleter<physx::PxControllerManager>());
	if (!m_ControllerMgr)
	{
		THROW_CUSEXCEPTION("PxCreateControllerManager failed");
	}

	// ! only enable it if experiencing collision problems
	m_ControllerMgr->setTessellation(false, 1.0f);

	return true;
}

float PhysxScene::GetVisualizationParameter(physx::PxVisualizationParameter::Enum paramEnum) const
{
	return m_PxScene->getVisualizationParameter(paramEnum);
}

void PhysxScene::SetVisualizationParameter(physx::PxVisualizationParameter::Enum param, float value)
{
	m_PxScene->setVisualizationParameter(param, value);
}

void PhysxScene::SetControllerDebugRenderingFlags(physx::PxU32 flags)
{
	m_ControllerMgr->setDebugRenderingFlags(physx::PxControllerDebugRenderFlags(flags));
}

void PhysxScene::SetGravity(const my::Vector3 & vec)
{
	m_PxScene->setGravity((physx::PxVec3&)vec);
}

my::Vector3 PhysxScene::GetGravity(void) const
{
	return (my::Vector3&)m_PxScene->getGravity();
}

void PhysxScene::Shutdown(void)
{
	//m_EventPxThreadSubstep.disconnect_all_slots();
	_ASSERT(m_EventPxThreadSubstep.empty());
	//_ASSERT(!m_PxScene || 0 == m_PxScene->getNbActors(PxActorTypeSelectionFlags(0xff)));
	m_ControllerMgr.reset();
	m_PxScene.reset();
}

void PhysxScene::TickPreRender(float dtime)
{
	m_Sync.ResetEvent();

	mTriggerPairs.clear();

	mContactPairs.clear();

	PhysxSdk::getSingleton().m_RenderTickMuted = true;

	m_WaitForResults = Advance(dtime);
}

void PhysxScene::TickPostRender(float dtime)
{
	if (m_WaitForResults)
	{
		m_Sync.Wait(INFINITE);

		PhysxSdk::getSingleton().m_RenderTickMuted = false;

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
				if (mBufferedActiveTransforms[i].userData)
				{
					Actor* actor = (Actor*)mBufferedActiveTransforms[i].userData;
					if (!actor->m_Base || !actor->GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC)) // ! Actor::Update, m_Base->GetAttachPose
					{
						actor->SetPose((Vector3&)mBufferedActiveTransforms[i].actor2World.p, (Quaternion&)mBufferedActiveTransforms[i].actor2World.q);
					}
				}
			}
		}
		mDeletedActors.clear();

		TriggerPairList::const_iterator trigger_iter = mTriggerPairs.begin();
		for (; trigger_iter != mTriggerPairs.end(); trigger_iter++)
		{
			// ! PxTriggerPair::otherActor may not have userData for Controller objs
			if (trigger_iter->otherShape->userData)
			{
				Component* self_cmp = (Component*)trigger_iter->otherShape->userData;
				if (trigger_iter->triggerShape->userData)
				{
					Component* other_cmp = (Component*)trigger_iter->triggerShape->userData;
					TriggerEventArg arg(self_cmp->m_Actor, self_cmp, other_cmp->m_Actor, other_cmp, trigger_iter->status);
					self_cmp->m_Actor->m_EventOnTrigger(&arg);
				}
			}
		}

		ContactPairList::const_iterator contact_iter = mContactPairs.begin();
		for (; contact_iter != mContactPairs.end(); contact_iter++)
		{
			for (unsigned int i = 0; i < _countof(ContactPair::shapes); i++)
			{
				if (contact_iter->shapes[i]->userData)
				{
					Component* self_cmp = (Component*)contact_iter->shapes[i]->userData;
					unsigned int other_i = (i + 1) % _countof(ContactPair::shapes);
					if (contact_iter->shapes[other_i]->userData)
					{
						Component* other_cmp = (Component*)contact_iter->shapes[other_i]->userData;
						ContactEventArg arg(self_cmp->m_Actor, self_cmp, other_cmp->m_Actor, other_cmp, contact_iter->events);
						arg.position = (Vector3&)contact_iter->position;
						arg.separation = contact_iter->separation;
						arg.normal = (Vector3&)contact_iter->normal;
						arg.impulse = (Vector3&)contact_iter->impulse;
						self_cmp->m_Actor->m_EventOnContact(&arg);
					}
				}
			}
		}
	}
	else
	{
		PhysxSdk::getSingleton().m_RenderTickMuted = false;
	}
}

bool PhysxScene::Advance(float dtime)
{
	m_Timer.m_RemainingTime += my::Min(0.1f, dtime);

	if(!m_Timer.Step())
	{
		return false;
	}

	m_Completion0.setContinuation(*m_PxScene->getTaskManager(), NULL);

	Substep(m_Completion0);

	m_Completion0.removeReference();

	return true;
}

void PhysxScene::AdvanceSync(float dtime)
{
	m_Timer.m_RemainingTime += my::Min(0.1f, dtime);

	for (; m_Timer.Step(); )
	{
		m_PxScene->simulate(m_Timer.m_Interval, NULL, 0, 0, true);

		m_PxScene->fetchResults(true, &m_ErrorState);

		_ASSERT(0 == m_ErrorState);

		m_EventPxThreadSubstep(m_Timer.m_Interval);
	}
}

void PhysxScene::Substep(StepperTask & completionTask)
{
	m_PxScene->simulate(m_Timer.m_Interval, &completionTask, 0, 0, true);
}

void PhysxScene::SubstepDone(StepperTask * ownerTask)
{
	m_PxScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

	// ! be aware of multi thread
	m_EventPxThreadSubstep(m_Timer.m_Interval);

	if(!m_Timer.Step())
	{
		m_Sync.SetEvent();
		return;
	}

	StepperTask & task = (ownerTask == &m_Completion0 ? m_Completion1 : m_Completion0);

	task.setContinuation(*m_PxScene->getTaskManager(), NULL);

	Substep(task);

	task.removeReference();
}

void PhysxScene::PushRenderBuffer(my::DrawHelper * drawHelper)
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
			drawHelper->PushLine((Vector3 &)line.pos0, (Vector3 &)line.pos1, line.color0);
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

	if (static_cast<physx::Cct::CharacterControllerManager*>(m_ControllerMgr.get())->mDebugRenderingFlags)
	{
		physx::PxRenderBuffer& controllerDebugRenderable = m_ControllerMgr->getRenderBuffer();

		numLines = controllerDebugRenderable.getNbLines();
		if (numLines)
		{
			const physx::PxDebugLine* PX_RESTRICT lines = controllerDebugRenderable.getLines();
			for (physx::PxU32 i = 0; i < numLines; i++)
			{
				const physx::PxDebugLine& line = lines[i];
				drawHelper->PushLine((Vector3 &)line.pos0, (Vector3 &)line.pos1, line.color0);
			}
		}
		controllerDebugRenderable.clear();
	}
}

void PhysxScene::Flush(void)
{
	m_PxScene->flushSimulation(false);
}

physx::PxFilterFlags PhysxScene::filter(
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
	pairFlags = physx::PxPairFlag::eSOLVE_CONTACT | /*physx::PxPairFlag::eMODIFY_CONTACTS |*/ physx::PxPairFlag::eDETECT_DISCRETE_CONTACT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

	return physx::PxFilterFlag::eDEFAULT;
}

void PhysxScene::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
	// This is called when a breakable constraint breaks.
}

void PhysxScene::onWake(physx::PxActor** actors, physx::PxU32 count)
{
	// Only called on actors for which the PxActorFlag eSEND_SLEEP_NOTIFIES has been set.
}

void PhysxScene::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
	// Only called on actors for which the PxActorFlag eSEND_SLEEP_NOTIFIES has been set.
}

void PhysxScene::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	// ! set physx::PxPairFlag::eNOTIFY_TOUCH_FOUND for PhysxScene::filter pairFlags
	std::vector<physx::PxContactPairPoint> contactPoints;
	for (physx::PxU32 i = 0; i < nbPairs; i++)
	{
		if (pairs[i].flags & (physx::PxContactPairFlag::eREMOVED_SHAPE_0 | physx::PxContactPairFlag::eREMOVED_SHAPE_1))
		{
			continue;
		}

		physx::PxU32 contactCount = pairs[i].contactCount;
		if (contactCount)
		{
			contactPoints.resize(contactCount);
			pairs[i].extractContacts(&contactPoints[0], contactCount);
			for (physx::PxU32 j = 0; j < contactCount; j++)
			{
				// fetchResults true will block other px thread
				ContactPairList::iterator pair_iter = mContactPairs.insert(mContactPairs.end(), ContactPair());
				pair_iter->position = contactPoints[j].position;
				pair_iter->separation = contactPoints[j].separation;
				pair_iter->normal = contactPoints[j].normal;
				pair_iter->internalFaceIndex0 = contactPoints[j].internalFaceIndex0;
				pair_iter->impulse = contactPoints[j].impulse;
				pair_iter->internalFaceIndex1 = contactPoints[j].internalFaceIndex1;
				pair_iter->shapes[0] = pairs[i].shapes[0];
				pair_iter->shapes[1] = pairs[i].shapes[1];
				pair_iter->flags = pairs[i].flags;
				pair_iter->events = pairs[i].events;

				//FModContext::getSingleton().OnControlSound("demo2_3/untitled/15");
			}
		}
	}
}

void PhysxScene::onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 count)
{
	// ! set physx::PxPairFlag::eMODIFY_CONTACTS for PhysxScene::filter pairFlags
	for (unsigned int i = 0; i < count; i++)
	{
		for (unsigned int j = 0; j < pairs[i].contacts.size(); j++)
		{
			// In addition to modifying contact properties, it is possible to :
			// Set target velocities for each contact
			// Limit the maximum impulse applied at each contact
			// Adjust inverse massand inverse inertia scales separately for each body
			pairs[i].contacts.setTargetVelocity(j, physx::PxVec3(0, 0, 0));
		}
	}
}

void PhysxScene::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
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

void PhysxScene::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
{

}

void PhysxScene::removeRenderActorsFromPhysicsActor(const physx::PxActor * actor)
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
		mDeletedActors.push_back(const_cast<physx::PxActor*>(actor));
	}
}

bool PhysxScene::Overlap(
		const physx::PxGeometry & geometry,
		const my::Vector3 & Position,
		const my::Quaternion & Rotation,
		unsigned int filterWord0,
		const my::EventCallback & callback,
		unsigned int MaxNbTouches) const
{
	//// ref: physx::Cct::Controller::move, userObstacles
	//physx::Cct::CharacterControllerManager* mManager = static_cast<physx::Cct::CharacterControllerManager*>(m_ControllerMgr.get());
	//const physx::PxU32 nbControllers = mManager->getNbControllers();
	//physx::Cct::Controller** controllers = mManager->getControllers();

	physx::PxTransform pose((physx::PxVec3&)Position, (physx::PxQuat&)Rotation);
	int callback_i = 0;
	//for (physx::PxU32 i = 0; i < nbControllers && callback_i < MaxNbTouches; i++)
	//{
	//	physx::Cct::Controller* currentController = controllers[i];
	//	if (currentController->mUserData)
	//	{
	//		try
	//		{
	//			_ASSERT(Component::ComponentTypeController == ((Component*)currentController->mUserData)->GetComponentType());
	//			Controller* other_cmp = static_cast<Controller*>((Component*)currentController->mUserData);
	//			switch (currentController->mType)
	//			{
	//			case physx::PxControllerShapeType::eBOX:
	//			{
	//				physx::Cct::BoxController* BC = static_cast<physx::Cct::BoxController*>(currentController);
	//				physx::PxBoxGeometry box(BC->mHalfHeight, BC->mHalfSideExtent, BC->mHalfForwardExtent);
	//				physx::PxTransform box_pose(physx::toVec3(BC->getPosition()), BC->mUserParams.mQuatFromUp);
	//				if (physx::PxGeometryQuery::overlap(geometry, pose, box, box_pose))
	//				{
	//					OverlapHitArg arg(other_cmp->m_Actor, other_cmp, 0xFFFFffff);
	//					if (!callback(&arg))
	//					{
	//						return false;
	//					}
	//					callback_i++;
	//				}
	//				break;
	//			}
	//			case physx::PxControllerShapeType::eCAPSULE:
	//			{
	//				physx::Cct::CapsuleController* CC = static_cast<physx::Cct::CapsuleController*>(currentController);
	//				physx::PxCapsuleGeometry capsule(CC->mRadius, CC->mHeight * 0.5f);
	//				physx::PxTransform capsule_pose(physx::toVec3(CC->getPosition()), CC->mUserParams.mQuatFromUp);
	//				if (physx::PxGeometryQuery::overlap(geometry, pose, capsule, capsule_pose))
	//				{
	//					OverlapHitArg arg(other_cmp->m_Actor, other_cmp, 0xFFFFffff);
	//					if (!callback(&arg))
	//					{
	//						return false;
	//					}
	//					callback_i++;
	//				}
	//				break;
	//			}
	//			}
	//		}
	//		catch (const luabind::error& e)
	//		{
	//			my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
	//			return false;
	//		}
	//	}
	//}

	std::vector<physx::PxOverlapHit> buff(MaxNbTouches - callback_i);
	physx::PxOverlapBuffer hitbuff(buff.data(), buff.size());
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
		physx::PxFilterData(filterWord0, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC /*| physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eANY_HIT*/);
	if (m_PxScene->overlap(geometry, pose, hitbuff, filterData, NULL))
	{
		//try
		//{
			for (unsigned int i = 0; i < hitbuff.nbTouches; i++)
			{
				const physx::PxOverlapHit& hit = buff[i];
				if (hit.shape->userData)
				{
					Component* other_cmp = (Component*)hit.shape->userData;
					OverlapHitArg arg(other_cmp->m_Actor, other_cmp, hit.faceIndex);
					if (!callback(&arg))
					{
						return false;
					}
				}
			}
		//}
		//catch (const luabind::error& e)
		//{
		//	my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		//	return false;
		//}
	}
	return callback_i + hitbuff.nbTouches > 0;
}

bool PhysxScene::Raycast(
	const my::Vector3 & origin,
	const my::Vector3 & unitDir,
	float distance,
	unsigned int filterWord0,
	const my::EventCallback & callback,
	unsigned int MaxNbTouches) const
{
	std::vector<physx::PxRaycastHit> buff(MaxNbTouches);
	physx::PxRaycastBuffer hitbuff(buff.data(), buff.size());
	hitbuff.block.distance = FLT_MAX;
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
		physx::PxFilterData(filterWord0, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC /*| physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eANY_HIT*/);
	if (m_PxScene->raycast((physx::PxVec3&)origin, (physx::PxVec3&)unitDir, distance, hitbuff, physx::PxHitFlag::eDEFAULT, filterData, NULL, NULL))
	{
		//try
		//{
			for (unsigned int i = 0; i < hitbuff.nbTouches; i++)
			{
				const physx::PxRaycastHit& hit = buff[i];
				if (hit.shape->userData)
				{
					Component* other_cmp = (Component*)hit.shape->userData;
					RaycastHitArg arg(other_cmp->m_Actor, other_cmp, hit.faceIndex, (Vector3&)hit.position, (Vector3&)hit.normal, hit.distance, hit.u, hit.v);
					if (!callback(&arg))
					{
						return false;
					}
				}
			}
		//}
		//catch (const luabind::error& e)
		//{
		//	my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		//	return false;
		//}
	}
	return hitbuff.nbTouches > 0;
}

bool PhysxScene::Sweep(
	const physx::PxGeometry & geometry,
	const my::Vector3 & Position,
	const my::Quaternion & Rotation,
	const my::Vector3 & unitDir,
	float distance,
	unsigned int filterWord0,
	const my::EventCallback & callback,
	unsigned int MaxNbTouches) const
{
	physx::PxTransform pose((physx::PxVec3&)Position, (physx::PxQuat&)Rotation);

	std::vector<physx::PxSweepHit> buff(MaxNbTouches);
	physx::PxSweepBuffer hitbuff(buff.data(), buff.size());
	hitbuff.block.distance = FLT_MAX;
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
		physx::PxFilterData(filterWord0, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC /*| physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eANY_HIT*/);
	if (m_PxScene->sweep(geometry, pose, (physx::PxVec3&)unitDir, distance, hitbuff, physx::PxHitFlag::eDEFAULT, filterData, NULL, NULL, 0.0f))
	{
		//try
		//{
			for (unsigned int i = 0; i < hitbuff.nbTouches; i++)
			{
				const physx::PxSweepHit& hit = buff[i];
				if (hit.shape->userData)
				{
					Component* other_cmp = (Component*)hit.shape->userData;
					SweepHitArg arg(other_cmp->m_Actor, other_cmp, hit.faceIndex, (Vector3&)hit.position, (Vector3&)hit.normal, hit.distance);
					if (!callback(&arg))
					{
						return false;
					}
				}
			}
		//}
		//catch (const luabind::error& e)
		//{
		//	my::D3DContext::getSingleton().m_EventLog(lua_tostring(e.state(), -1));
		//	return false;
		//}
	}
	return hitbuff.nbTouches > 0;
}

PhysxSpatialIndex::PhysxSpatialIndex(void)
	: m_PxSpatialIndex(physx::PxCreateSpatialIndex(), PhysxDeleter<physx::PxSpatialIndex>())
{

}

PhysxSpatialIndex::~PhysxSpatialIndex(void)
{

}

void PhysxSpatialIndex::AddTriangle(const my::Vector3& v0, const my::Vector3& v1, const my::Vector3& v2)
{
	m_TriangleList.push_back(physx::PxTriangle((physx::PxVec3&)v0, (physx::PxVec3&)v1, (physx::PxVec3&)v2));
}

void PhysxSpatialIndex::AddBox(float hx, float hy, float hz, const my::Vector3& Pos, const my::Quaternion& Rot)
{
	AddGeometry(physx::PxBoxGeometry(hx, hy, hz), physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot));
}

void PhysxSpatialIndex::AddGeometry(const physx::PxGeometry& geom, const physx::PxTransform& pose)
{
	GeometryPairPtr geompair(new GeometryPair(geom, pose));
	m_GeometryList.push_back(geompair);
	BOOST_VERIFY(m_GeometryList.size() - 1 == m_PxSpatialIndex->insert(
		(physx::PxSpatialIndexItem&)*geompair, physx::PxGeometryQuery::getWorldBounds(geom, pose)));
}

int PhysxSpatialIndex::GetTriangleNum(void) const
{
	return m_TriangleList.size();
}

int PhysxSpatialIndex::GetGeometryNum(void) const
{
	return m_GeometryList.size();
}

void PhysxSpatialIndex::GetTriangle(int i, my::Vector3& v0, my::Vector3& v1, my::Vector3& v2) const
{
	v0 = (Vector3&)m_TriangleList[i].verts[0];
	v1 = (Vector3&)m_TriangleList[i].verts[1];
	v2 = (Vector3&)m_TriangleList[i].verts[2];
}

void PhysxSpatialIndex::GetBox(int i, float& hx, float& hy, float& hz, my::Vector3& Pos, my::Quaternion& Rot) const
{
	const physx::PxBoxGeometry& box = m_GeometryList[i]->first.box();
	hx = box.halfExtents.x;
	hy = box.halfExtents.y;
	hz = box.halfExtents.z;

	Pos = (Vector3&)m_GeometryList[i]->second.p;
	Rot = (Quaternion&)m_GeometryList[i]->second.q;
}

bool PhysxSpatialIndex::Raycast(const my::Vector3& pos, const my::Vector3& dir, float dist, float& t) const
{
	struct HitCallback : physx::PxSpatialLocationCallback
	{
		float closest;
		const my::Vector3& pos;
		const my::Vector3& dir;
		float dist;
		HitCallback(const my::Vector3& _pos, const my::Vector3& _dir, float _dist)
			: closest(FLT_MAX)
			, pos(_pos)
			, dir(_dir)
			, dist(_dist)
		{
		}
		virtual physx::PxAgain onHit(physx::PxSpatialIndexItem& item, physx::PxReal distance, physx::PxReal& shrunkDistance)
		{
			GeometryPair& geompair = (GeometryPair&)item;
			physx::PxRaycastHit hit;
			if (physx::PxGeometryQuery::raycast((physx::PxVec3&)pos, (physx::PxVec3&)dir, geompair.first.any(), geompair.second, dist, physx::PxHitFlag::eDISTANCE, 1, &hit))
			{
				if (hit.distance < closest)
				{
					closest = hit.distance;
				}
			}
			return true;
		}
	};

	HitCallback cb(pos, dir, dist);
	m_PxSpatialIndex->raycast((physx::PxVec3&)pos, (physx::PxVec3&)dir, dist, cb);

	TriangleList::const_iterator tri_iter = m_TriangleList.begin();
	for (; tri_iter != m_TriangleList.end(); tri_iter++)
	{
		float t, u, v;
		if (physx::Gu::intersectRayTriangleCulling((physx::PxVec3&)pos, (physx::PxVec3&)dir, tri_iter->verts[0], tri_iter->verts[1], tri_iter->verts[2], t, u, v))
		{
			if (t < cb.closest)
			{
				cb.closest = t;
			}
		}
	}

	if (cb.closest < FLT_MAX)
	{
		t = cb.closest;
		return true;
	}
	return false;
}

bool PhysxSpatialIndex::OverlapBox(float hx, float hy, float hz, const my::Vector3& Pos, const my::Quaternion& Rot) const
{
	struct OverlapCallback : physx::PxSpatialOverlapCallback
	{
		physx::PxBoxGeometry box;
		physx::PxTransform pose;
		bool overlap;
		OverlapCallback(float hx, float hy, float hz, const my::Vector3& Pos, const my::Quaternion& Rot)
			: box(hx, hy, hz)
			, pose((physx::PxVec3&)Pos, (physx::PxQuat&)Rot)
			, overlap(false)
		{
		}
		virtual physx::PxAgain onHit(physx::PxSpatialIndexItem& item)
		{
			GeometryPair& geompair = (GeometryPair&)item;
			if (physx::PxGeometryQuery::overlap(box, pose, geompair.first.any(), geompair.second))
			{
				overlap = true;
				return false;
			}
			return true;
		}
	};

	OverlapCallback cb(hx, hy, hz, Pos, Rot);
	m_PxSpatialIndex->overlap(physx::PxGeometryQuery::getWorldBounds(cb.box, cb.pose), cb);

	if (!cb.overlap)
	{
		physx::Gu::BoxPadded box;
		physx::buildFrom(box, cb.pose.p, cb.box.halfExtents, cb.pose.q);
		TriangleList::const_iterator tri_iter = m_TriangleList.begin();
		for (; tri_iter != m_TriangleList.end(); tri_iter++)
		{
			if (physx::Gu::intersectTriangleBox(box, tri_iter->verts[0], tri_iter->verts[1], tri_iter->verts[2]))
			{
				return true;
			}
		}
	}
	return cb.overlap;
}

bool PhysxSpatialIndex::SweepBox(float hx, float hy, float hz, const my::Vector3& Pos, const my::Quaternion& Rot, const my::Vector3& dir, float dist, float& t) const
{
	struct HitCallback : physx::PxSpatialLocationCallback
	{
		float closest;
		const Vector3 & dir;
		float dist;
		physx::PxBoxGeometry box;
		physx::PxTransform pose;
		HitCallback(const Vector3& _dir, float _dist, float hx, float hy, float hz, const my::Vector3& Pos, const my::Quaternion& Rot)
			: closest(FLT_MAX)
			, dir(_dir)
			, dist(_dist)
			, box(hx, hy, hz)
			, pose((physx::PxVec3&)Pos, (physx::PxQuat&)Rot)
		{
		}
		virtual physx::PxAgain onHit(physx::PxSpatialIndexItem& item, physx::PxReal distance, physx::PxReal& shrunkDistance)
		{
			GeometryPair& geompair = (GeometryPair&)item;
			physx::PxSweepHit hit;
			if (physx::PxGeometryQuery::sweep((physx::PxVec3&)dir, dist, box, pose, geompair.first.any(), geompair.second, hit))
			{
				if (hit.distance < closest)
				{
					closest = hit.distance;
				}
			}
			return true;
		}
	};

	HitCallback cb(dir, dist, hx, hy, hz, Pos, Rot);
	m_PxSpatialIndex->sweep(physx::PxGeometryQuery::getWorldBounds(cb.box, cb.pose), (physx::PxVec3&)dir, dist, cb);

	physx::PxSweepHit hit;
	if (physx::PxMeshQuery::sweep((physx::PxVec3&)dir, dist, cb.box, cb.pose, GetTriangleNum(), m_TriangleList.data(), hit))
	{
		if (hit.distance < cb.closest)
		{
			cb.closest = hit.distance;
		}
	}

	if (cb.closest < FLT_MAX)
	{
		t = cb.closest;
		return true;
	}
	return false;
}

// access private member using template trick, https://stackoverflow.com/questions/12993219/access-private-member-using-template-trick
template<typename Tag, typename Tag::type M>
struct Rob {
	friend typename Tag::type get(Tag) {
		return M;
	}
};

struct NpSpatialIndex_f {
	typedef physx::Sq::IncrementalPruner* physx::NpSpatialIndex::* type;
	friend type get(NpSpatialIndex_f);
};

template struct Rob<NpSpatialIndex_f, &physx::NpSpatialIndex::mPruner>;

my::AABB PhysxSpatialIndex::CalculateAABB(void) const
{
	my::AABB ret(FLT_MAX, -FLT_MAX);
	physx::NpSpatialIndex* idx = static_cast<physx::NpSpatialIndex*>(m_PxSpatialIndex.get());
	physx::Sq::AABBPruner* pruner = static_cast<physx::Sq::AABBPruner*>(idx->*get(NpSpatialIndex_f()));
	for (unsigned int i = 0; i < pruner->mPool.mNbObjects; i++)
	{
		ret.unionSelf((AABB&)pruner->mPool.mWorldBoxes[i]);
	}
	return ret;
}

// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "PhysxContext.h"
#include "Component.h"
#include "Terrain.h"
#include "Actor.h"
#include "myDxutApp.h"
#include "libc.h"
#include <fcntl.h>
#include <io.h>
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
#include <boost/scope_exit.hpp>
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

PhysxFileOutputStream::PhysxFileOutputStream(LPCTSTR pFilename)
	: m_fp(0)
{
	errno_t err = _tsopen_s(&m_fp, pFilename, _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
	if (0 != err)
	{
		THROW_CUSEXCEPTION(str_printf("cannot open file archive: %S", pFilename));
	}
}

PhysxFileOutputStream::~PhysxFileOutputStream(void)
{
	_close(m_fp);
}

uint32_t PhysxFileOutputStream::write(const void* src, uint32_t count)
{
	return _write(m_fp, src, count);
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

struct PhysxSceneShaderInfo
{
	PhysxScene* scene;
};

bool PhysxScene::Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher, const physx::PxSceneFlags & flags, const my::Vector3 & gravity)
{
	_ASSERT(flags.isSet(physx::PxSceneFlag::eENABLE_PCM));

	physx::PxSceneDesc sceneDesc(sdk->getTolerancesScale());
	sceneDesc.gravity = (physx::PxVec3&)gravity;
	sceneDesc.simulationEventCallback = this;
	sceneDesc.contactModifyCallback = this;
	sceneDesc.cpuDispatcher = dispatcher;
	//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	PhysxSceneShaderInfo info = { this };
	sceneDesc.filterShaderData = &info;
	sceneDesc.filterShaderDataSize = sizeof(info);
	sceneDesc.filterShader = PhysxScene::FilterShader;
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

	m_ObstacleContext.reset(m_ControllerMgr->createObstacleContext(), PhysxDeleter<physx::PxObstacleContext>());

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
	m_ObstacleContext.reset();
	m_ControllerMgr.reset();
	m_PxScene.reset();
}

void PhysxScene::TickPreRender(float fElapsedTime)
{
	m_Sync.ResetEvent();

	mTriggerPairs.clear();

	mContactPairs.clear();

	InterlockedExchange(&PhysxSdk::getSingleton().m_RenderTickMuted, 1);

	m_WaitForResults = Advance(fElapsedTime);
}

void PhysxScene::TickPostRender(float fElapsedTime)
{
	if (m_WaitForResults)
	{
		m_Sync.Wait(INFINITE);

		InterlockedExchange(&PhysxSdk::getSingleton().m_RenderTickMuted, 0);

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
			if (trigger_iter->triggerShape->userData)
			{
				Component* self_cmp = (Component*)trigger_iter->triggerShape->userData;
				if (trigger_iter->otherShape->userData)
				{
					Component* other_cmp = (Component*)trigger_iter->otherShape->userData;
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
		InterlockedExchange(&PhysxSdk::getSingleton().m_RenderTickMuted, 0);
	}
}

bool PhysxScene::Advance(float fElapsedTime)
{
	m_RemainingTime += fElapsedTime;

	if (Timer::Step(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/))
	{
		m_Completion0.setContinuation(*m_PxScene->getTaskManager(), NULL);

		Substep(m_Completion0);

		m_Completion0.removeReference();

		return true;
	}
	return false;
}

void PhysxScene::AdvanceSync(float fElapsedTime)
{
	InterlockedExchange(&PhysxSdk::getSingleton().m_RenderTickMuted, 1);
	BOOST_SCOPE_EXIT(void)
	{
		InterlockedExchange(&PhysxSdk::getSingleton().m_RenderTickMuted, 0);
	}
	BOOST_SCOPE_EXIT_END

	m_RemainingTime += fElapsedTime;

	for (; Timer::Step(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/); )
	{
		m_PxScene->simulate(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/, NULL, 0, 0, true);

		m_PxScene->fetchResults(true, &m_ErrorState);

		_ASSERT(0 == m_ErrorState);

		m_EventPxThreadSubstep(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/);
	}
}

void PhysxScene::Substep(StepperTask & completionTask)
{
	m_PxScene->simulate(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/, &completionTask, 0, 0, true);
}

void PhysxScene::SubstepDone(StepperTask * ownerTask)
{
	m_PxScene->fetchResults(true, &m_ErrorState);

	_ASSERT(0 == m_ErrorState);

	// ! be aware of multi thread
	m_EventPxThreadSubstep(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/);

	if(Timer::Step(PhysxSdk::getSingleton().m_FrameInterval/* * my::D3DContext::getSingleton().m_fTimeScale*/))
	{
		StepperTask& task = (ownerTask == &m_Completion0 ? m_Completion1 : m_Completion0);

		task.setContinuation(*m_PxScene->getTaskManager(), NULL);

		Substep(task);

		task.removeReference();

		return;
	}

	m_Sync.SetEvent();
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

physx::PxFilterFlags PhysxScene::FilterShader(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	physx::PxU32 constantBlockSize)
{
	_ASSERT(constantBlockSize == sizeof(PhysxSceneShaderInfo));

	PhysxSceneShaderInfo* info = (PhysxSceneShaderInfo*)constantBlock;

	return info->scene->onSimulationFilter(attributes0, filterData0, attributes1, filterData1, pairFlags);
}

physx::PxFilterFlags PhysxScene::onSimulationFilter(
	physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags)
{
	// The custom filter shader to use for collision filtering. If you don't want to define your own filter shader you can
	// use the default shader #PxDefaultSimulationFilterShader which can be found in the PhysX extensions
	// library.
	return physx::PxDefaultSimulationFilterShader(attributes0, filterData0, attributes1, filterData1, pairFlags, NULL, 0);
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
	// ! set physx::PxPairFlag::eNOTIFY_TOUCH_FOUND for PhysxScene::FilterShader pairFlags
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
	// ! set physx::PxPairFlag::eMODIFY_CONTACTS for PhysxScene::FilterShader pairFlags
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

bool PhysxScene::OnControllerFilter(const physx::PxController& a, const physx::PxController& b)
{
	return true;
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

void PhysxSpatialIndex::AddMesh(my::OgreMesh* mesh, int sub_mesh_id, const my::Vector3& Pos, const my::Quaternion& Rot, const my::Vector3& Scale)
{
	std::pair<TriangleMeshMap::iterator, bool> res = m_TriangleMeshMap.insert(std::make_pair(std::make_pair(mesh, sub_mesh_id), boost::shared_ptr<physx::PxTriangleMesh>()));
	if (res.second)
	{
		D3DXATTRIBUTERANGE rang;
		if (sub_mesh_id < 0)
		{
			rang = { 0, 0, mesh->GetNumFaces(), 0, mesh->GetNumVertices() };
		}
		else
		{
			rang = mesh->m_AttribTable[sub_mesh_id];
		}
		physx::PxTriangleMeshDesc desc;
		desc.points.count = rang.VertexStart + rang.VertexCount;
		desc.points.stride = mesh->GetNumBytesPerVertex();
		desc.points.data = &mesh->m_VertexElems.GetPosition(mesh->LockVertexBuffer());
		desc.triangles.count = rang.FaceCount;
		if (mesh->GetOptions() & D3DXMESH_32BIT)
		{
			desc.triangles.stride = 3 * sizeof(DWORD);
		}
		else
		{
			desc.triangles.stride = 3 * sizeof(WORD);
			desc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;
		}
		desc.triangles.data = (unsigned char*)mesh->LockIndexBuffer() + rang.FaceStart * desc.triangles.stride;

		//// mesh should be validated before cooked without the mesh cleaning
		//_ASSERT(PhysxSdk::getSingleton().m_Cooking->validateTriangleMesh(desc));

		physx::PxTriangleMesh* tri_mesh = PhysxSdk::getSingleton().m_Cooking->createTriangleMesh(desc, PhysxSdk::getSingleton().m_sdk->getPhysicsInsertionCallback());
		res.first->second.reset(tri_mesh, PhysxDeleter<physx::PxTriangleMesh>());
		mesh->UnlockIndexBuffer();
		mesh->UnlockVertexBuffer();
	}

	physx::PxMeshScale mesh_scaling((physx::PxVec3&)Scale, physx::PxQuat(physx::PxIdentity));
	AddGeometry(physx::PxTriangleMeshGeometry(res.first->second.get(), mesh_scaling, physx::PxMeshGeometryFlags()), Pos, Rot);
}

void PhysxSpatialIndex::AddGeometry(const physx::PxGeometry& geom, const my::Vector3& Pos, const my::Quaternion& Rot)
{
	GeometryPairPtr geompair(new GeometryPair(geom, physx::PxTransform((physx::PxVec3&)Pos, (physx::PxQuat&)Rot)));
	m_GeometryList.push_back(geompair);
	BOOST_VERIFY(m_GeometryList.size() - 1 == m_PxSpatialIndex->insert(
		(physx::PxSpatialIndexItem&)*geompair, physx::PxGeometryQuery::getWorldBounds(geom, geompair->second)));
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

physx::PxGeometryType::Enum PhysxSpatialIndex::GetGeometryType(int i) const
{
	return m_GeometryList[i]->first.getType();
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

const my::AABB& PhysxSpatialIndex::GetGeometryWorldBox(int i) const
{
	physx::NpSpatialIndex* idx = static_cast<physx::NpSpatialIndex*>(m_PxSpatialIndex.get());
	physx::Sq::AABBPruner* pruner = static_cast<physx::Sq::AABBPruner*>(idx->*get(NpSpatialIndex_f()));
	_ASSERT(i <= pruner->mPool.mNbObjects);
	return (AABB&)pruner->mPool.mWorldBoxes[i];
}

bool PhysxSpatialIndex::Raycast(const my::Vector3& pos, const my::Vector3& dir, float dist, float& t) const
{
	struct HitCallback : physx::PxSpatialLocationCallback
	{
		const my::Vector3& pos;
		const my::Vector3& dir;
		float dist;
		float closest;
		HitCallback(const my::Vector3& _pos, const my::Vector3& _dir, float _dist)
			: pos(_pos)
			, dir(_dir)
			, dist(_dist)
			, closest(FLT_MAX)
		{
		}
		virtual physx::PxAgain onHit(physx::PxSpatialIndexItem& item, physx::PxReal distance, physx::PxReal& shrunkDistance)
		{
			GeometryPair& geompair = (GeometryPair&)item;
			physx::PxRaycastHit hit;
			physx::PxU32 hitCount = physx::PxGeometryQuery::raycast((physx::PxVec3&)pos, (physx::PxVec3&)dir, geompair.first.any(), geompair.second, dist, physx::PxHitFlag::eDISTANCE, 1, &hit);
			if (hitCount > 0)
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

bool PhysxSpatialIndex::Overlap(const physx::PxGeometry& geometry, const my::Vector3& Pos, const my::Quaternion& Rot) const
{
	struct OverlapCallback : physx::PxSpatialOverlapCallback
	{
		const physx::PxGeometry& geom;
		physx::PxTransform pose;
		bool overlap;
		OverlapCallback(const physx::PxGeometry& _geom, const my::Vector3& Pos, const my::Quaternion& Rot)
			: geom(_geom)
			, pose((physx::PxVec3&)Pos, (physx::PxQuat&)Rot)
			, overlap(false)
		{
		}
		virtual physx::PxAgain onHit(physx::PxSpatialIndexItem& item)
		{
			GeometryPair& geompair = (GeometryPair&)item;
			if (physx::PxGeometryQuery::overlap(geom, pose, geompair.first.any(), geompair.second))
			{
				overlap = true;
				return false;
			}
			return true;
		}
	};

	OverlapCallback cb(geometry, Pos, Rot);
	m_PxSpatialIndex->overlap(physx::PxGeometryQuery::getWorldBounds(geometry, cb.pose), cb);

	if (!cb.overlap)
	{
		physx::PxSweepHit hit;
		if (physx::PxMeshQuery::sweep(physx::PxVec3(1, 0, 0), 1, geometry, cb.pose, GetTriangleNum(), m_TriangleList.data(), hit, physx::PxHitFlag::eDEFAULT, NULL, 0.0f, true))
		{
			return true;
		}
	}
	return cb.overlap;
}

bool PhysxSpatialIndex::Sweep(const physx::PxGeometry& geometry, const my::Vector3& Pos, const my::Quaternion& Rot, const my::Vector3& dir, float dist, float& t) const
{
	struct HitCallback : physx::PxSpatialLocationCallback
	{
		float closest;
		const Vector3 & dir;
		float dist;
		const physx::PxGeometry& geom;
		physx::PxTransform pose;
		HitCallback(const Vector3& _dir, float _dist, const physx::PxGeometry& _geom, const my::Vector3& Pos, const my::Quaternion& Rot)
			: closest(FLT_MAX)
			, dir(_dir)
			, dist(_dist)
			, geom(_geom)
			, pose((physx::PxVec3&)Pos, (physx::PxQuat&)Rot)
		{
		}
		virtual physx::PxAgain onHit(physx::PxSpatialIndexItem& item, physx::PxReal distance, physx::PxReal& shrunkDistance)
		{
			GeometryPair& geompair = (GeometryPair&)item;
			physx::PxSweepHit hit;
			if (physx::PxGeometryQuery::sweep((physx::PxVec3&)dir, dist, geom, pose, geompair.first.any(), geompair.second, hit))
			{
				if (hit.distance < closest)
				{
					closest = hit.distance;
				}
			}
			return true;
		}
	};

	HitCallback cb(dir, dist, geometry, Pos, Rot);
	m_PxSpatialIndex->sweep(physx::PxGeometryQuery::getWorldBounds(geometry, cb.pose), (physx::PxVec3&)dir, dist, cb);

	physx::PxSweepHit hit;
	if (physx::PxMeshQuery::sweep((physx::PxVec3&)dir, dist, geometry, cb.pose, GetTriangleNum(), m_TriangleList.data(), hit))
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

my::AABB PhysxSpatialIndex::CalculateAABB(void) const
{
	my::AABB ret = my::AABB::Invalid();
	for (int i = 0; i < GetTriangleNum(); i++)
	{
		my::Vector3 v0, v1, v2;
		GetTriangle(i, v0, v1, v2);
		ret.unionSelf(v0);
		ret.unionSelf(v1);
		ret.unionSelf(v2);
	}
	for (int i = 0; i < GetGeometryNum(); i++)
	{
		ret.unionSelf(GetGeometryWorldBox(i));
	}
	return ret;
}

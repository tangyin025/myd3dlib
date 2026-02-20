// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <PxPhysicsAPI.h>
#include "mySingleton.h"
#include "myMath.h"
#include "myUtility.h"
#include "myThread.h"

namespace my
{
	class OgreMesh;
};

class Actor;

class Component;

class Controller;

class PhysxAllocator : public physx::PxAllocatorCallback
{
public:
	PhysxAllocator(void)
	{
	}

	void * allocate(size_t size, const char * typeName, const char * filename, int line);

	void deallocate(void * ptr);
};

template<class T>
struct PhysxDeleter
{
	typedef void result_type;

	typedef T * argument_type;

	void operator()(T * x) const
	{
		_ASSERT(x); x->release();
	}
};

class PhysxInputData : public physx::PxInputData
{
protected:
	my::IStreamPtr m_istr;

public:
	PhysxInputData(my::IStreamPtr istr)
		: m_istr(istr)
	{
	}

	virtual uint32_t read(void* dest, uint32_t count)
	{
		return m_istr->read(dest, count);
	}

	virtual uint32_t getLength() const;

	virtual void seek(uint32_t offset)
	{
		m_istr->seek(offset, SEEK_SET);
	}

	virtual uint32_t tell() const
	{
		return m_istr->tell();
	}
};

class PhysxFileOutputStream : public physx::PxOutputStream
{
protected:
	int m_fp;

public:
	PhysxFileOutputStream(LPCTSTR pFilename);

	virtual ~PhysxFileOutputStream(void);

	virtual uint32_t write(const void* src, uint32_t count);
};

class PhysxSdk
	: public my::SingletonInstance<PhysxSdk>
	, public physx::PxErrorCallback
{
public:
	PhysxAllocator m_Allocator;

	boost::shared_ptr<physx::PxFoundation> m_Foundation;

	boost::shared_ptr<physx::PxPvd> m_Pvd;

	boost::shared_ptr<physx::PxPhysics> m_sdk;

	boost::shared_ptr<physx::PxCooking> m_Cooking;

	boost::shared_ptr<physx::PxDefaultCpuDispatcher> m_CpuDispatcher;

	typedef std::map<std::string, boost::shared_ptr<physx::PxBase> > CollectionObjMap;

	CollectionObjMap m_CollectionObjs;

	my::CriticalSection m_CollectionObjsSec;

	float m_FrameInterval;

	volatile LONG m_RenderTickMuted;

public:
	PhysxSdk(void)
		: m_FrameInterval(1 / 60.0f)
		, m_RenderTickMuted(0)
	{
	}

	bool Init(void);

	void Shutdown(void);

	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};

struct HitArg : my::EventArg
{
public:
	Actor* actor;
	Component* cmp;

	HitArg(Actor* _actor, Component* _cmp)
		: actor(_actor)
		, cmp(_cmp)
	{
	}
};

struct OverlapHitArg : HitArg
{
public:
	unsigned int faceIndex;

	OverlapHitArg(Actor* _actor, Component* _cmp, unsigned int _faceIndex)
		: HitArg(_actor, _cmp)
		, faceIndex(_faceIndex)
	{
	}
};

struct SweepHitArg : OverlapHitArg
{
public:
	my::Vector3 position;
	my::Vector3 normal;
	float distance;

	SweepHitArg(Actor* _actor, Component* _cmp, unsigned int _faceIndex, const my::Vector3& _position, const my::Vector3& _normal, float _distance)
		: OverlapHitArg(_actor, _cmp, _faceIndex)
		, position(_position)
		, normal(_normal)
		, distance(_distance)
	{
	}
};

struct RaycastHitArg : SweepHitArg
{
public:
	float u, v;

	RaycastHitArg(Actor* _actor, Component* _cmp, unsigned int _faceIndex, const my::Vector3& _position, const my::Vector3& _normal, float _distance, float _u, float _v)
		: SweepHitArg(_actor, _cmp, _faceIndex, _position, _normal, _distance)
		, u(_u)
		, v(_v)
	{
	}
};

class PhysxScene
	: public physx::PxSimulationEventCallback
	, public physx::PxContactModifyCallback
	, public my::Timer
{
public:
	class StepperTask
		: public physx::PxLightCpuTask
	{
	public:
		PhysxScene * m_Scene;

		StepperTask(PhysxScene * Scene)
			: m_Scene(Scene)
		{
		}

		virtual void run(void);

		virtual const char * getName(void) const;
	};

	StepperTask m_Completion0, m_Completion1;

	my::Event m_Sync;

	bool m_WaitForResults;

	physx::PxU32 m_ErrorState;

	boost::shared_ptr<physx::PxScene> m_PxScene;

	typedef boost::signals2::signal<void (float)> SubstepEvent;

	SubstepEvent m_EventPxThreadSubstep;

	boost::shared_ptr<physx::PxControllerManager> m_ControllerMgr;

	boost::shared_ptr<physx::PxObstacleContext> m_ObstacleContext;

	std::vector<physx::PxActiveTransform> mBufferedActiveTransforms;

	std::vector<physx::PxActor *> mDeletedActors;

	physx::PxU32 mActiveTransformCount;

	typedef std::vector<physx::PxTriggerPair> TriggerPairList;
	
	TriggerPairList mTriggerPairs;

	struct ContactPair : physx::PxContactPairPoint
	{
		physx::PxShape* shapes[2];
		physx::PxContactPairFlags flags;
		physx::PxPairFlags events;
	};

	typedef std::vector<ContactPair> ContactPairList;

	ContactPairList mContactPairs;

	struct ControllerFilterCallback : physx::PxControllerFilterCallback
	{
		PhysxScene* scene;

		ControllerFilterCallback(PhysxScene * _scene)
			: scene(_scene)
		{
		}

		virtual bool filter(const physx::PxController& a, const physx::PxController& b)
		{
			return scene->OnControllerFilter(a, b);
		}
	};

	ControllerFilterCallback m_ControllerFilter;

public:
	PhysxScene(void)
		: m_Completion0(this)
		, m_Completion1(this)
		, m_Sync(NULL, FALSE, FALSE, NULL)
		, m_WaitForResults(false)
		, m_ErrorState(0)
		, mActiveTransformCount(0)
		, m_ControllerFilter(this)
	{
	}

	bool Init(physx::PxPhysics * sdk, physx::PxDefaultCpuDispatcher * dispatcher, const physx::PxSceneFlags & flags, const my::Vector3 & gravity);

	float GetVisualizationParameter(physx::PxVisualizationParameter::Enum paramEnum) const;

	void SetVisualizationParameter(physx::PxVisualizationParameter::Enum param, float value);

	void SetControllerDebugRenderingFlags(physx::PxU32 flags);

	void SetGravity(const my::Vector3 & vec);

	my::Vector3 GetGravity(void) const;

	void Shutdown(void);

	void TickPreRender(float fElapsedTime);

	void TickPostRender(float fElapsedTime);

	bool Advance(float fElapsedTime);

	void AdvanceSync(float fElapsedTime);

	void Substep(StepperTask & completionTask);

	void SubstepDone(StepperTask * ownerTask);

	void PushRenderBuffer(my::DrawHelper * drawHelper);

	void Flush(void);

	static physx::PxFilterFlags FilterShader(
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags,
		const void* constantBlock,
		physx::PxU32 constantBlockSize);

	virtual physx::PxFilterFlags onSimulationFilter(
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags);

	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count);

	virtual void onWake(physx::PxActor** actors, physx::PxU32 count);

	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count);

	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);

	virtual void onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 count);

	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);

	virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count);

	virtual bool OnControllerFilter(const physx::PxController& a, const physx::PxController& b);

	void removeRenderActorsFromPhysicsActor(const physx::PxActor * actor);
};

class PhysxSpatialIndex
{
public:
	boost::shared_ptr<physx::PxSpatialIndex> m_PxSpatialIndex;

	typedef std::vector<physx::PxTriangle> TriangleList;

	TriangleList m_TriangleList;

	typedef std::map<std::pair<my::OgreMesh*, unsigned int>, boost::shared_ptr<physx::PxTriangleMesh> > TriangleMeshMap;

	TriangleMeshMap m_TriangleMeshMap;

	typedef std::pair<physx::PxGeometryHolder, physx::PxTransform> GeometryPair;

	typedef boost::shared_ptr<GeometryPair> GeometryPairPtr;

	typedef std::vector<GeometryPairPtr> GeometryTupleList;

	GeometryTupleList m_GeometryList;

public:
	PhysxSpatialIndex(void);

	~PhysxSpatialIndex(void);

	void AddTriangle(const my::Vector3& v0, const my::Vector3& v1, const my::Vector3& v2);

	void AddMesh(my::OgreMesh* mesh, int sub_mesh_id, const my::Vector3& Pos, const my::Quaternion& Rot, const my::Vector3& Scale);

	void AddGeometry(const physx::PxGeometry& geom, const my::Vector3& Pos, const my::Quaternion& Rot);

	int GetTriangleNum(void) const;

	int GetGeometryNum(void) const;

	void GetTriangle(int i, my::Vector3& v0, my::Vector3& v1, my::Vector3& v2) const;

	physx::PxGeometryType::Enum GetGeometryType(int i) const;

	void GetBox(int i, float& hx, float& hy, float& hz, my::Vector3& Pos, my::Quaternion& Rot) const;

	const my::AABB& GetGeometryWorldBox(int i) const;

	bool Raycast(const my::Vector3& pos, const my::Vector3& dir, float dist, float& t) const;

	bool Overlap(const physx::PxGeometry& geometry, const my::Vector3& Pos, const my::Quaternion& Rot) const;

	bool Sweep(const physx::PxGeometry& geometry, const my::Vector3& Pos, const my::Quaternion& Rot, const my::Vector3& dir, float dist, float& t) const;

	my::AABB CalculateAABB(void) const;
};

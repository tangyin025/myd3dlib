#include "Steering.h"
#include "Actor.h"
#include "Controller.h"
#include "NavigationSerialization.h"
#include "DetourCommon.h"

using namespace my;

ObstacleAvoidanceContext::ObstacleAvoidanceContext(void)
	: dtObstacleAvoidanceQuery()
{
	dtObstacleAvoidanceQuery::init(6, 8);
}

ObstacleAvoidanceContext::~ObstacleAvoidanceContext(void)
{
}

Steering::Steering(const Actor* _Actor, float BrakingRate, float MaxSpeed)
	: m_Actor(_Actor)
	, m_Forward(_Actor->m_World.getColumn<2>().xyz.normalize())
	, m_Speed(0.0f)
	, m_BrakingRate(BrakingRate)
	, m_MaxSpeed(MaxSpeed)
	, m_agentPos(FLT_MAX)
	, m_targetPos(FLT_MAX)
	, m_targetRef(0)
	, m_ncorners(0)
{
	m_corridor.init(256);
}

Steering::~Steering(void)
{
}

my::Vector3 Steering::SeekDir(my::Vector3 Force, float dtime)
{
	// https://github.com/meshula/OpenSteer/blob/master/src/SimpleVehicle.cpp
	// SimpleVehicle::applySteeringForce

	float forceLength = Force.magnitude();
	if (forceLength < EPSILON_E3)
	{
		// apply a given braking force (for a given dt) to our momentum.
		float Braking = Min(m_Speed * m_BrakingRate, 10.0f);
		m_Speed -= Braking * dtime;
	}

	// allows a specific vehicle class to redefine this adjustment.
	// default is to disallow backward-facing steering at low speed.
	const float maxAdjustedSpeed = 0.2f * m_MaxSpeed;
	if (forceLength >= EPSILON_E3 && m_Speed < maxAdjustedSpeed)
	{
		const float range = m_Speed / maxAdjustedSpeed;
		// const float cosine = interpolate (pow (range, 6), 1.0f, -1.0f);
		// const float cosine = interpolate (pow (range, 10), 1.0f, -1.0f);
		// const float cosine = interpolate (pow (range, 20), 1.0f, -1.0f);
		// const float cosine = interpolate (pow (range, 100), 1.0f, -1.0f);
		// const float cosine = interpolate (pow (range, 50), 1.0f, -1.0f);
		const float cosineOfConeAngle = Lerp(1.0f, -1.0f, powf(range, 20.0f));

		// measure the angular diviation of "source" from "basis"
		const Vector3 direction = Force / forceLength;
		float cosineOfSourceAngle = direction.dot(m_Forward);

		// Simply return "source" if it already meets the angle criteria.
		// (note: we hope this top "if" gets compiled out since the flag
		// is a constant when the function is inlined into its caller)
		if (cosineOfSourceAngle < cosineOfConeAngle)
		{
			// find the portion of "source" that is perpendicular to "basis"
			const Vector3 perp = Force - m_Forward * cosineOfSourceAngle;

			// normalize that perpendicular
			const Vector3 unitPerp = perp.normalize();

			// construct a new vector whose length equals the source vector,
			// and lies on the intersection of a plane (formed the source and
			// basis vectors) and a cone (whose axis is "basis" and whose
			// angle corresponds to cosineOfConeAngle)
			float perpDist = sqrtf(1.0f - (cosineOfConeAngle * cosineOfConeAngle));
			const Vector3 c0 = m_Forward * cosineOfConeAngle;
			const Vector3 c1 = unitPerp * perpDist;
			Force = (c0 + c1) * forceLength;
		}
	}

	// compute acceleration and velocity
	Vector3 newAcceleration = Force * 5.0f;
	Vector3 newVelocity = m_Forward * m_Speed;

	// Euler integrate (per frame) acceleration into velocity
	newVelocity += newAcceleration * dtime;

	// enforce speed limit
	m_Speed = newVelocity.magnitude();
	if (m_Speed > m_MaxSpeed)
	{
		// update Speed
		newVelocity = newVelocity * m_MaxSpeed / m_Speed;
		m_Speed = m_MaxSpeed;
	}

	// regenerate local space (by default: align vehicle's forward axis with
	// new velocity, but this behavior may be overridden by derived classes.)
	if (m_Speed > EPSILON_E3)
	{
		m_Forward = newVelocity / m_Speed;
		_ASSERT(IS_NORMALIZED(m_Forward));
	}
	return newVelocity;
}

my::Vector3 Steering::SeekTarget(const my::Vector3& Target, float dtime)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		const Actor* self;
		Vector3 pos;
		std::vector<Controller*> neighbors;
		Navigation* navi;
		Callback(const Actor* _self, const my::Vector3& _pos)
			: self(_self)
			, pos(_pos)
			, navi(NULL)
		{
		}

		virtual void OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);

			if (actor != self)
			{
				if (!navi && (navi = actor->GetFirstComponent<Navigation>()) && !actor->m_OctAabb->Intersect2D(pos))
				{
					navi = NULL;
				}

				Controller* neighbor = actor->GetFirstComponent<Controller>();
				if (neighbor)
				{
					neighbors.push_back(neighbor);
				}
			}
		}
	};

	const Controller* controller = m_Actor->GetFirstComponent<Controller>();
	OctNode* Root = m_Actor->m_Node->GetTopNode();
	const Vector3 pos = controller->GetPosition();
	Callback cb(m_Actor, pos);
	Root->QueryEntity(AABB(pos, 5.0f), &cb);

	if (!cb.navi)
	{
		return Vector3(0, 0, 0);
	}

	// https://github.com/recastnavigation/recastnavigation/blob/master/DetourCrowd/Source/DetourCrowd.cpp
	// dtCrowd::update
	dtQueryFilter filter;
	dtPolyRef agentRef = m_corridor.getFirstPoly();
	float m_agentPlacementHalfExtents[3] = { controller->m_Radius, controller->m_Height * 0.5, controller->m_Radius };
	if ((m_agentPos - pos).magnitude2D() > EPSILON_E3 || !cb.navi->m_navQuery->isValidPolyRef(agentRef, &filter))
	{
		// Find nearest position on navmesh and place the agent there.
		dtPolyRef ref = 0;
		dtStatus status = cb.navi->m_navQuery->findNearestPoly(&pos.x, m_agentPlacementHalfExtents, &filter, &ref, &m_agentPos.x);
		if (dtStatusFailed(status))
		{
			return Vector3(0, 0, 0);
		}
		m_corridor.reset(ref, &m_agentPos.x);
	}

	// CrowdToolState::setMoveTarget
	if ((m_targetPos - Target).magnitude2D() > EPSILON_E3)
	{
		// Find nearest point on navmesh and set move request to that location.
		dtStatus status = cb.navi->m_navQuery->findNearestPoly(&Target.x, m_agentPlacementHalfExtents, &filter, &m_targetRef, &m_targetPos.x);
		if (dtStatusFailed(status))
		{
			return Vector3(0, 0, 0);
		}
	}

	// dtCrowd::updateMoveRequest
	const dtPolyRef* path = m_corridor.getPath();
	const int npath = m_corridor.getPathCount();
	static const int MAX_RES = 32;
	float reqPos[3];
	dtPolyRef reqPath[MAX_RES];	// The path to the request location
	int reqPathCount = 0;

	// Quick search towards the goal.
	static const int MAX_ITER = 20;
	dtStatus status = 0;
	status = cb.navi->m_navQuery->initSlicedFindPath(path[0], m_targetRef, &pos.x, &m_targetPos.x, &filter);
	status = cb.navi->m_navQuery->updateSlicedFindPath(MAX_ITER, 0);
	//// Try to use existing steady path during replan if possible.
	//status = cb.navi->m_navQuery->finalizeSlicedFindPathPartial(path, npath, reqPath, &reqPathCount, MAX_RES);
	// Try to move towards target when goal changes.
	status = cb.navi->m_navQuery->finalizeSlicedFindPath(reqPath, &reqPathCount, MAX_RES);
	if (!dtStatusFailed(status) && reqPathCount > 0)
	{
		// In progress or succeed.
		if (reqPath[reqPathCount - 1] != m_targetRef)
		{
			// Partial path, constrain target position inside the last polygon.
			status = cb.navi->m_navQuery->closestPointOnPoly(reqPath[reqPathCount - 1], &m_targetPos.x, reqPos, 0);
			if (dtStatusFailed(status))
				reqPathCount = 0;
		}
		else
		{
			dtVcopy(reqPos, &m_targetPos.x);
		}
	}
	else
	{
		reqPathCount = 0;
	}

	if (!reqPathCount)
	{
		// Could not find path, start the request from current location.
		dtVcopy(reqPos, &pos.x);
		reqPath[0] = path[0];
		reqPathCount = 1;
	}

	m_corridor.setCorridor(reqPos, reqPath, reqPathCount);

	// Find next corner to steer to.
	m_ncorners = m_corridor.findCorners(m_cornerVerts, m_cornerFlags, m_cornerPolys,
		DT_CROWDAGENT_MAX_CORNERS, cb.navi->m_navQuery.get(), &filter);
	if (!m_ncorners)
	{
		return Vector3(0, 0, 0);
	}

	my::Vector3 desiredForce = (*(Vector3*)&m_cornerVerts[0] - pos).normalize();
	return SeekDir(desiredForce, dtime);
}

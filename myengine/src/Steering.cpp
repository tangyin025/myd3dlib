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

Steering::Steering(const char * Name, float BrakingRate, float MaxSpeed)
	: Component(Name)
	, m_Forward(0, 0, 1)
	, m_Speed(0.0f)
	, m_BrakingRate(BrakingRate)
	, m_MaxSpeed(MaxSpeed)
	, m_agentPos(FLT_MAX)
	, m_targetPos(FLT_MAX)
	, m_targetRef(0)
	, m_targetRefPos(FLT_MAX)
	, m_targetReplanTime(0.0f)
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
	// https://github.com/recastnavigation/recastnavigation/blob/master/DetourCrowd/Source/DetourCrowd.cpp
	// dtCrowd::update

	struct Callback : public my::OctNode::QueryCallback
	{
		const Actor* self;
		Vector3 pos;
		std::vector<Steering*> neighbors;
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

			if (actor != self && !actor->m_Base)
			{
				if (!navi && (navi = actor->GetFirstComponent<Navigation>()) && !actor->m_OctAabb->Intersect2D(pos))
				{
					navi = NULL;
				}

				Steering* neighbor = actor->GetFirstComponent<Steering>();
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
	const float collisionQueryRange = controller->m_Radius * 12.0f;
	Callback cb(m_Actor, pos);
	Root->QueryEntity(AABB(pos, collisionQueryRange), &cb);
	if (!cb.navi)
	{
		return Vector3(0, 0, 0);
	}

	// CrowdToolState::setMoveTarget
	bool replan = false;
	dtQueryFilter filter;
	float m_agentPlacementHalfExtents[3] = { controller->m_Radius * 2.0f, controller->m_Height * 1.5f, controller->m_Radius * 2.0f };
	if ((m_targetPos - Target).magnitude2D() > EPSILON_E3 || !cb.navi->m_navQuery->isValidPolyRef(m_targetRef, &filter))
	{
		// Find nearest point on navmesh and set move request to that location.
		dtStatus status = cb.navi->m_navQuery->findNearestPoly(&Target.x, m_agentPlacementHalfExtents, &filter, &m_targetRef, &m_targetRefPos.x);
		if (dtStatusFailed(status) || !m_targetRef)
		{
			return Vector3(0, 0, 0);
		}
		m_targetPos = Target;
		m_corridor.reset(0, &m_agentPos.x);
		m_boundary.reset();
		replan = true;
	}

	dtPolyRef agentRef = m_corridor.getFirstPoly();
	if (!cb.navi->m_navQuery->isValidPolyRef(agentRef, &filter))
	{
		// Find nearest position on navmesh and place the agent there.
		dtStatus status = cb.navi->m_navQuery->findNearestPoly(&pos.x, m_agentPlacementHalfExtents, &filter, &agentRef, &m_agentPos.x);
		if (dtStatusFailed(status) || !agentRef)
		{
			return Vector3(0, 0, 0);
		}
		m_corridor.reset(agentRef, &m_agentPos.x);
		m_boundary.reset();
		replan = true;
	}
	else
	{
		// Move along navmesh.
		if (m_corridor.movePosition(&pos.x, cb.navi->m_navQuery.get(), &filter) && dtVdist2DSqr(m_corridor.getPos(), &pos.x) < EPSILON_E12)
		{
			// Get valid constrained position back.
			dtVcopy(&m_agentPos.x, m_corridor.getPos());
		}
		else
		{
			Vector3 dvel = (*(Vector3*)m_corridor.getPos() - pos).normalize();
			m_corridor.reset(0, &pos.x);
			m_boundary.reset();
			return SeekDir(dvel, dtime);
		}
	}

	// dtCrowd::updateMoveRequest
	static const int CHECK_LOOKAHEAD = 10;
	static const float TARGET_REPLAN_DELAY = 1.0; // seconds
	if (replan)
	{
		const dtPolyRef* path = m_corridor.getPath();
		const int npath = m_corridor.getPathCount();
		static const int MAX_RES = 32;
		float reqPos[3];
		dtPolyRef reqPath[MAX_RES];	// The path to the request location
		int reqPathCount = 0;

		// Quick search towards the goal.
		static const int MAX_ITER = 20;
		dtStatus status = 0;
		status = cb.navi->m_navQuery->initSlicedFindPath(path[0], m_targetRef, &pos.x, &m_targetRefPos.x, &filter);
		status = cb.navi->m_navQuery->updateSlicedFindPath(MAX_ITER, 0);
		// Try to move towards target when goal changes.
		status = cb.navi->m_navQuery->finalizeSlicedFindPath(reqPath, &reqPathCount, MAX_RES);
		if (!dtStatusFailed(status) && reqPathCount > 0)
		{
			// In progress or succeed.
			if (reqPath[reqPathCount - 1] != m_targetRef)
			{
				// Partial path, constrain target position inside the last polygon.
				status = cb.navi->m_navQuery->closestPointOnPoly(reqPath[reqPathCount - 1], &m_targetRefPos.x, reqPos, 0);
				if (dtStatusFailed(status))
					reqPathCount = 0;
			}
			else
			{
				dtVcopy(reqPos, &m_targetRefPos.x);
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
		m_boundary.reset();
		m_targetReplanTime = 0.0f;
	}
	else if (m_targetReplanTime > TARGET_REPLAN_DELAY &&
		m_corridor.getPathCount() < CHECK_LOOKAHEAD &&
		m_corridor.getLastPoly() != m_targetRef)
	{
		//// Try to use existing steady path during replan if possible.
		//status = cb.navi->m_navQuery->finalizeSlicedFindPathPartial(path, npath, reqPath, &reqPathCount, MAX_RES);
	}

	// Find next corner to steer to.
	m_ncorners = m_corridor.findCorners(m_cornerVerts, m_cornerFlags, m_cornerPolys,
		DT_CROWDAGENT_MAX_CORNERS, cb.navi->m_navQuery.get(), &filter);
	if (!m_ncorners)
	{
		return Vector3(0, 0, 0);
	}

	// Calculate steering.
	Vector3 dvel = (*(Vector3*)&m_cornerVerts[0] - pos).normalize();
	Vector3 vel = m_Forward * m_Speed;

	// Update the collision boundary after certain distance has been passed or
	// if it has become invalid.
	const float updateThr = collisionQueryRange * 0.25f;
	if (dtVdist2DSqr(&pos.x, m_boundary.getCenter()) > dtSqr(updateThr) || !m_boundary.isValid(cb.navi->m_navQuery.get(), &filter))
	{
		m_boundary.update(m_corridor.getFirstPoly(), &pos.x, collisionQueryRange, cb.navi->m_navQuery.get(), &filter);
	}

	// Add neighbours as obstacles.
	ObstacleAvoidanceContext::getSingleton().reset();
	for (int i = 0; i < cb.neighbors.size(); ++i)
	{
		const Steering* nei = cb.neighbors[i];
		const Controller* neicontroller = nei->m_Actor->GetFirstComponent<Controller>();
		if (neicontroller)
		{
			Vector3 npos = neicontroller->GetPosition();
			Vector3 vel = nei->m_Forward * nei->m_Speed;
			ObstacleAvoidanceContext::getSingleton().addCircle(&npos.x, neicontroller->m_Radius, &vel.x, &vel.x);
		}
	}

	// Append neighbour segments as obstacles.
	for (int i = 0; i < m_boundary.getSegmentCount(); ++i)
	{
		const float* s = m_boundary.getSegment(i);
		if (dtTriArea2D(&pos.x, s, s + 3) < 0.0f)
			continue;
		ObstacleAvoidanceContext::getSingleton().addSegment(s, s + 3);
	}

	// Sample new safe velocity.
	dtObstacleAvoidanceParams params;
	params.velBias = 0.4f;
	params.weightDesVel = 2.0f;
	params.weightCurVel = 0.75f;
	params.weightSide = 0.75f;
	params.weightToi = 2.5f;
	params.horizTime = 2.5f;
	params.gridSize = 33;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 5;
	Vector3 nvel;
	ObstacleAvoidanceContext::getSingleton().sampleVelocityAdaptive(&pos.x, controller->m_Radius, m_MaxSpeed,
		&vel.x, &dvel.x, &nvel.x, &params, NULL);
	return SeekDir(nvel, dtime);
}

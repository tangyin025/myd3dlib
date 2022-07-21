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

Steering::Steering(const char * Name, float MaxSpeed, float BrakingSpeed, float MaxAdjustedSpeed)
	: Component(Name)
	, m_Forward(0, 0, 1)
	, m_Speed(0.0f)
	, m_MaxSpeed(MaxSpeed)
	, m_BrakingSpeed(BrakingSpeed)
	, m_MaxAdjustedSpeed(MaxAdjustedSpeed)
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

	// apply a given braking force (for a given dt) to our momentum.
	m_Speed = Max(0.0f, m_Speed - m_BrakingSpeed * dtime);

	// allows a specific vehicle class to redefine this adjustment.
	// default is to disallow backward-facing steering at low speed.
	const float forceLength = Force.magnitude();
	if (forceLength >= EPSILON_E3 && m_Speed < m_MaxAdjustedSpeed)
	{
		const float range = m_Speed / m_MaxAdjustedSpeed;
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
			const Vector3 perp = Force - m_Forward * Force.dot(m_Forward);

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
	const Vector3 newAcceleration = Force;
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

my::Vector3 Steering::SeekTarget(const my::Vector3& Target, float forceLength, float dtime)
{
	// https://github.com/recastnavigation/recastnavigation/blob/master/DetourCrowd/Source/DetourCrowd.cpp
	// dtCrowd::update

	struct Callback : public my::OctNode::QueryCallback
	{
		const Actor* self;
		Vector3 pos;
		std::vector<Steering*> neighbors;
		std::vector<Controller*> nei_controllers;
		Navigation* navi;
		Callback(const Actor* _self, const my::Vector3& _pos)
			: self(_self)
			, pos(_pos)
			, navi(NULL)
		{
		}

		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			Actor* actor = dynamic_cast<Actor*>(oct_entity);

			if (actor != self && !actor->m_Base)
			{
				if (!navi && (navi = actor->GetFirstComponent<Navigation>()) && !actor->m_OctAabb->Intersect2D(pos))
				{
					navi = NULL;
				}

				Steering* neighbor = actor->GetFirstComponent<Steering>();
				Controller* nei_controller = actor->GetFirstComponent<Controller>();
				if (neighbor && nei_controller && nei_controller->GetQueryFilterWord0() & ~0x03 && !actor->m_Base)
				{
					neighbors.push_back(neighbor);
					nei_controllers.push_back(nei_controller);
				}
			}
			return true;
		}
	};

	const Controller* controller = m_Actor->GetFirstComponent<Controller>();
	OctNode* Root = m_Actor->m_Node->GetTopNode();
	const Vector3 pos = controller->GetPosition();
	const float collisionQueryRange = controller->GetRadius() * 12.0f;
	Callback cb(m_Actor, pos);
	Root->QueryEntity(AABB(pos, collisionQueryRange), &cb);
	if (!cb.navi)
	{
		return Vector3(0, 0, 0);
	}

	// CrowdToolState::setMoveTarget
	static const int CHECK_LOOKAHEAD = 10;
	static const float TARGET_REPLAN_DELAY = 1.0; // seconds
	bool replan = false;
	dtQueryFilter filter;
	const float m_agentPlacementHalfExtents[3] = { 5,5,5 };// { controller->GetRadius() * 2.0f, controller->GetHeight() * 1.5f, controller->GetRadius() * 2.0f };
	if ((m_targetPos - Target).magnitudeSq() > EPSILON_E6 || !cb.navi->m_navQuery->isValidPolyRef(m_targetRef, &filter))
	{
		// Find nearest point on navmesh and set move request to that location.
		dtStatus status = cb.navi->m_navQuery->findNearestPoly(&Target.x, m_agentPlacementHalfExtents, &filter, &m_targetRef, &m_targetRefPos.x);
		if (dtStatusFailed(status) || !m_targetRef)
		{
			return SeekDir(Vector3(0, 0, 0), dtime);
		}

		m_targetPos = Target;
		m_corridor.reset(0, &m_agentPos.x);
		m_boundary.reset();
		replan = true;
	}
	//else if (m_targetReplanTime > TARGET_REPLAN_DELAY &&
	//	m_corridor.getPathCount() < CHECK_LOOKAHEAD &&
	//	m_corridor.getLastPoly() != m_targetRef)
	//{
	//	m_corridor.reset(0, &m_agentPos.x);
	//	m_boundary.reset();
	//	replan = true;
	//}

	dtPolyRef agentRef = m_corridor.getFirstPoly();
	if (!cb.navi->m_navQuery->isValidPolyRef(agentRef, &filter))
	{
		// Find nearest position on navmesh and place the agent there.
		dtStatus status = cb.navi->m_navQuery->findNearestPoly(&pos.x, m_agentPlacementHalfExtents, &filter, &agentRef, &m_agentPos.x);
		if (dtStatusFailed(status) || !agentRef)
		{
			return SeekDir(Vector3(0, 0, 0), dtime);
		}

		if (dtVdist2DSqr(&m_agentPos.x, &pos.x) < EPSILON_E6)
		{
			m_corridor.reset(agentRef, &m_agentPos.x);
			m_boundary.reset();
			replan = true;
		}
		else
		{
			Vector3 dvel = (m_agentPos - pos).normalize2D();
			m_corridor.reset(0, &pos.x);
			m_boundary.reset();
			return SeekDir(dvel * forceLength, dtime);
		}
	}
	else
	{
		// Move along navmesh.
		if (m_corridor.movePosition(&pos.x, cb.navi->m_navQuery.get(), &filter) && dtVdist2DSqr(m_corridor.getPos(), &pos.x) < EPSILON_E6)
		{
			// Get valid constrained position back.
			dtVcopy(&m_agentPos.x, m_corridor.getPos());
		}
		else
		{
			Vector3 dvel = (*(Vector3*)m_corridor.getPos() - pos).normalize2D();
			m_corridor.reset(0, &pos.x);
			m_boundary.reset();
			return SeekDir(dvel * forceLength, dtime);
		}
	}

	// dtCrowd::updateMoveRequest
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
		status = cb.navi->m_navQuery->initSlicedFindPath(path[0], m_targetRef, &m_agentPos.x, &m_targetRefPos.x, &filter);
		status = cb.navi->m_navQuery->updateSlicedFindPath(MAX_ITER, 0);
		//if (ag->targetReplan) // && npath > 10)
		//{
		//	// Try to use existing steady path during replan if possible.
		//	status = cb.navi->m_navQuery->finalizeSlicedFindPathPartial(path, npath, reqPath, &reqPathCount, MAX_RES);
		//}
		//else
		{
			// Try to move towards target when goal changes.
			status = cb.navi->m_navQuery->finalizeSlicedFindPath(reqPath, &reqPathCount, MAX_RES);
		}
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
			dtVcopy(reqPos, &m_agentPos.x);
			reqPath[0] = path[0];
			reqPathCount = 1;
		}
		m_corridor.setCorridor(reqPos, reqPath, reqPathCount);
		m_boundary.reset();
		m_targetReplanTime = 0.0f;
	}

	// Find next corner to steer to.
	m_ncorners = m_corridor.findCorners(m_cornerVerts, m_cornerFlags, m_cornerPolys,
		DT_CROWDAGENT_MAX_CORNERS, cb.navi->m_navQuery.get(), &filter);
	if (!m_ncorners)
	{
		return Vector3(0, 0, 0);
	}

	// Calculate steering.
	Vector3 dvel = (*(Vector3*)&m_cornerVerts[0] - m_agentPos).normalize() * m_MaxSpeed;

	// Separation
	const float separationDist = collisionQueryRange;
	const float invSeparationDist = 1.0f / separationDist;
	const float separationWeight = 8.0f;

	float w = 0;
	float disp[3] = { 0,0,0 };

	for (int i = 0; i < cb.neighbors.size(); ++i)
	{
		const Steering* nei = cb.neighbors[i];
		const Controller* neicontroller = cb.nei_controllers[i];

		float diff[3];
		Vector3 neipos = neicontroller->GetPosition();
		dtVsub(diff, &m_agentPos.x, &neipos.x);
		diff[1] = 0;

		const float distSqr = dtVlenSqr(diff);
		if (distSqr < 0.00001f)
			continue;
		if (distSqr > dtSqr(separationDist))
			continue;
		const float dist = dtMathSqrtf(distSqr);
		const float weight = separationWeight * (1.0f - dtSqr(dist * invSeparationDist));

		dtVmad(disp, disp, diff, weight / dist);
		w += 1.0f;
	}

	if (w > 0.0001f)
	{
		// Adjust desired velocity.
		dtVmad(&dvel.x, &dvel.x, disp, 1.0f / w);
		// Clamp desired velocity to desired speed.
		const float speedSqr = dtVlenSqr(&dvel.x);
		const float desiredSqr = dtSqr(m_MaxSpeed);
		if (speedSqr > desiredSqr)
			dtVscale(&dvel.x, &dvel.x, desiredSqr / speedSqr);
	}

	// Update the collision boundary after certain distance has been passed or
	// if it has become invalid.
	const float updateThr = collisionQueryRange * 0.25f;
	if (dtVdist2DSqr(&m_agentPos.x, m_boundary.getCenter()) > dtSqr(updateThr) || !m_boundary.isValid(cb.navi->m_navQuery.get(), &filter))
	{
		m_boundary.update(m_corridor.getFirstPoly(), &m_agentPos.x, collisionQueryRange, cb.navi->m_navQuery.get(), &filter);
	}

	// Add neighbours as obstacles.
	ObstacleAvoidanceContext::getSingleton().reset();
	for (int i = 0; i < cb.neighbors.size(); ++i)
	{
		const Steering* nei = cb.neighbors[i];
		const Controller* neicontroller = cb.nei_controllers[i];

		Vector3 npos = neicontroller->GetPosition();
		Vector3 vel = nei->m_Forward * nei->m_Speed;
		ObstacleAvoidanceContext::getSingleton().addCircle(&npos.x, neicontroller->GetRadius(), &vel.x, &vel.x);
	}

	// Append neighbour segments as obstacles.
	for (int i = 0; i < m_boundary.getSegmentCount(); ++i)
	{
		const float* s = m_boundary.getSegment(i);
		if (dtTriArea2D(&m_agentPos.x, s, s + 3) < 0.0f)
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
	Vector3 vel = m_Forward * m_Speed;
	ObstacleAvoidanceContext::getSingleton().sampleVelocityAdaptive(&m_agentPos.x, controller->GetRadius(), m_MaxSpeed,
		&vel.x, &dvel.x, &nvel.x, &params, NULL);
	return SeekDir(nvel.normalize() * forceLength, dtime);
}

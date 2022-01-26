#include "Steering.h"
#include "Actor.h"
#include "Controller.h"
#include "NavigationSerialization.h"

using namespace my;

Steering::Steering(const Actor* _Actor, float BrakingRate, float MaxSpeed)
	: m_Actor(_Actor)
	, m_Forward(_Actor->m_World.getColumn<2>().xyz.normalize())
	, m_Speed(0.0f)
	, m_BrakingRate(BrakingRate)
	, m_MaxSpeed(MaxSpeed)
{
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
	Callback cb(m_Actor, controller->GetPosition());
	Root->QueryEntity(AABB(controller->GetPosition(), 5.0f), &cb);

	if (!cb.navi)
	{
		return Vector3(0, 0, 0);
	}

	return m_Forward * m_Speed;
}

#include "stdafx.h"
#include "myCollision.h"

namespace my
{
	// /////////////////////////////////////////////////////////////////////////////////////
	// BoundingSphere
	// /////////////////////////////////////////////////////////////////////////////////////

	BoundingSphere::BoundingSphere(const Vector3 & _center, float _radius)
		: center(_center)
		, radius(_radius)
	{
	}

	bool BoundingSphere::overlaps(const BoundingSphere & other) const
	{
		return (center - other.center).magnitudeSq() < (radius + other.radius) * (radius + other.radius);
	}

	float BoundingSphere::getGrowth(const BoundingSphere & other) const
	{
		BoundingSphere newSphere = buildBoundingSphere(*this, other);

		return newSphere.getRadius() * newSphere.getRadius() - radius * radius;
	}

	float BoundingSphere::getVolumn(void) const
	{
		return (float)4.0 / (float)3.0 * (float)D3DX_PI * radius * radius * radius;
	}

	BoundingSphere buildBoundingSphere(const BoundingSphere & lhs, const BoundingSphere & rhs)
	{
		Vector3 centerOffset = lhs.getCenter() - rhs.getCenter();
		float distanceSquare = centerOffset.magnitudeSq();
		float radiusDiff = lhs.getRadius() - rhs.getRadius();

		if(radiusDiff * radiusDiff >= distanceSquare)
		{
			if(lhs.getRadius() >= rhs.getRadius())
			{
				return BoundingSphere(lhs.getCenter(), lhs.getRadius());
			}
			else
			{
				return BoundingSphere(rhs.getCenter(), rhs.getRadius());
			}
		}
		else
		{
			float distance = sqrt(distanceSquare);
			float newRadius = (lhs.getRadius() + distance + rhs.getRadius()) / 2;
			Vector3 newCenter = lhs.getCenter() + centerOffset * ((newRadius - lhs.getRadius()) / distance);

			return BoundingSphere(newCenter, newRadius);
		}
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionPrimitive
	// /////////////////////////////////////////////////////////////////////////////////////

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionSphere
	// /////////////////////////////////////////////////////////////////////////////////////

	CollisionSphere::CollisionSphere(
		float _radius,
		const Matrix4 & _offset,
		float _friction,
		float _restitution)
		: CollisionPrimitive(_offset, _friction, _restitution)
		, radius(_radius)
	{
	}

	unsigned CollisionSphere::collide(
		const CollisionPrimitive * rhs,
		Contact * contacts,
		unsigned limits) const
	{
		return rhs->collideSphere(this, contacts, limits);
	}

	unsigned CollisionSphere::collideSphere(
		const CollisionSphere * sphere,
		Contact * contacts,
		unsigned limits) const
	{
		return CollisionDetector::sphereAndSphere(*this, *sphere, contacts, limits);
	}

	unsigned CollisionSphere::collideBox(
		const CollisionBox * box,
		Contact * contacts,
		unsigned limits) const
	{
		return CollisionDetector::boxAndSphere(*box, *this, contacts, limits);
	}

	unsigned CollisionSphere::collideHalfSpace(
		const Plane & plane,
		float planeFriction,
		float planeRestitution,
		Contact * contacts,
		unsigned limits) const
	{
		return CollisionDetector::sphereAndHalfSpace(*this, plane, planeFriction, planeRestitution, contacts, limits);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionBox
	// /////////////////////////////////////////////////////////////////////////////////////

	CollisionBox::CollisionBox(
		const Vector3 & _halfSize,
		const Matrix4 & _offset,
		float _friction,
		float _restitution)
		: CollisionPrimitive(_offset, _friction, _restitution)
		, halfSize(_halfSize)
	{
	}

	unsigned CollisionBox::collide(
		const CollisionPrimitive * rhs,
		Contact * contacts,
		unsigned limits) const
	{
		return rhs->collideBox(this, contacts, limits);
	}

	unsigned CollisionBox::collideSphere(
		const CollisionSphere * sphere,
		Contact * contacts,
		unsigned limits) const
	{
		return CollisionDetector::boxAndSphere(*this, *sphere, contacts, limits);
	}

	unsigned CollisionBox::collideBox(
		const CollisionBox * box,
		Contact * contacts,
		unsigned limits) const
	{
		return CollisionDetector::boxAndBox(*this, *box, contacts, limits);
	}

	unsigned CollisionBox::collideHalfSpace(
		const Plane & plane,
		float planeFriction,
		float planeRestitution,
		Contact * contacts,
		unsigned limits) const
	{
		return CollisionDetector::boxAndHalfSpace(*this, plane, planeFriction, planeRestitution, contacts, limits);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// IntersectionTests
	// /////////////////////////////////////////////////////////////////////////////////////

	bool IntersectionTests::sphereAndHalfSpace(
		const CollisionSphere & sphere,
		const Plane & plane)
	{
		_ASSERT(IS_NORMALIZED(plane.normal));

		return plane.DistanceToPoint(sphere.getTransformAxis(3)) < sphere.radius;
	}

	bool IntersectionTests::sphereAndSphere(
		const CollisionSphere & sphere0,
		const CollisionSphere & sphere1)
	{
		return (sphere0.getTransformAxis(3) - sphere1.getTransformAxis(3)).magnitudeSq() < (sphere0.radius + sphere1.radius) * (sphere0.radius + sphere1.radius);
	}

	float IntersectionTests::calculateBoxAxisHalfProjection(const CollisionBox & box, const Vector3 & axis)
	{
		_ASSERT(IS_NORMALIZED(axis));

		return
			box.halfSize.x * abs(axis.dot(box.getTransformAxis(0))) +
			box.halfSize.y * abs(axis.dot(box.getTransformAxis(1))) +
			box.halfSize.z * abs(axis.dot(box.getTransformAxis(2))); // ***
	}

	bool IntersectionTests::boxAndHalfSpace(
		const CollisionBox & box,
		const Plane & plane)
	{
		_ASSERT(IS_NORMALIZED(plane.normal));

		return plane.DistanceToPoint(box.getTransformAxis(3)) < calculateBoxAxisHalfProjection(box, plane.normal);
	}

	bool IntersectionTests::_overlapOnAxis(
		const CollisionBox & box0,
		const CollisionBox & box1,
		const Vector3 & axis,
		const Vector3 & toCentre)
	{
		return
			abs(toCentre.dot(axis)) < calculateBoxAxisHalfProjection(box0, axis) + calculateBoxAxisHalfProjection(box1, axis); // ***
	}

	bool IntersectionTests::_zeroAxisOrOverlapOnAxis(
		const CollisionBox & box0,
		const CollisionBox & box1,
		const Vector3 & axis,
		const Vector3 & toCentre)
	{
		if(abs(axis.x) < EPSILON_E6 && abs(axis.y) < EPSILON_E6 && abs(axis.z) < EPSILON_E6)
		{
			return true;
		}
		
		return _overlapOnAxis(box0, box1, axis, toCentre);
	}

	bool IntersectionTests::boxAndBox(
		const CollisionBox & box0,
		const CollisionBox & box1)
	{
		Vector3 toCentre = box1.getTransformAxis(3) - box0.getTransformAxis(3);

		return
			_overlapOnAxis(box0, box1, box0.getTransformAxis(0), toCentre) &&
			_overlapOnAxis(box0, box1, box0.getTransformAxis(1), toCentre) &&
			_overlapOnAxis(box0, box1, box0.getTransformAxis(2), toCentre) &&

			_overlapOnAxis(box0, box1, box1.getTransformAxis(0), toCentre) &&
			_overlapOnAxis(box0, box1, box1.getTransformAxis(1), toCentre) &&
			_overlapOnAxis(box0, box1, box1.getTransformAxis(2), toCentre) &&

			/*
			 * NOTE:
			 *	some cross of two box will be invalid if their have the same axis, but if this occure
			 *	only one axis should be checked, and have alread checked, so just continue
			 */

			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(0).cross(box1.getTransformAxis(0)), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(0).cross(box1.getTransformAxis(1)), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(0).cross(box1.getTransformAxis(2)), toCentre) &&

			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(1).cross(box1.getTransformAxis(0)), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(1).cross(box1.getTransformAxis(1)), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(1).cross(box1.getTransformAxis(2)), toCentre) &&

			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(2).cross(box1.getTransformAxis(0)), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(2).cross(box1.getTransformAxis(1)), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, box0.getTransformAxis(2).cross(box1.getTransformAxis(2)), toCentre); // ***
	}

	//static float _caculateInternalAngles(
	//	const Vector3 & v0,
	//	const Vector3 & v1,
	//	const Vector3 & v2,
	//	const Vector3 & intersection)
	//{
	//	Vector3 ldir = t3d::vec3Sub(v0, intersection);
	//	Vector3 rdir = t3d::vec3Sub(v1, intersection);
	//	float llen = t3d::vec3LengthSquare(ldir);
	//	float rlen = t3d::vec3LengthSquare(rdir);
	//	if(IS_ZERO_FLOAT(llen) || IS_ZERO_FLOAT(rlen))
	//	{
	//		return DEG_TO_RAD(360);
	//	}

	//	float angles = acos(t3d::vec3CosTheta(ldir, rdir));

	//	ldir = t3d::vec3Sub(v1, intersection);
	//	rdir = t3d::vec3Sub(v2, intersection);
	//	llen = t3d::vec3LengthSquare(ldir);
	//	rlen = t3d::vec3LengthSquare(rdir);
	//	if(IS_ZERO_FLOAT(llen) || IS_ZERO_FLOAT(rlen))
	//	{
	//		return DEG_TO_RAD(360);
	//	}

	//	angles += acos(t3d::vec3CosTheta(ldir, rdir));

	//	ldir = t3d::vec3Sub(v2, intersection);
	//	rdir = t3d::vec3Sub(v0, intersection);
	//	llen = t3d::vec3LengthSquare(ldir);
	//	rlen = t3d::vec3LengthSquare(rdir);
	//	if(IS_ZERO_FLOAT(llen) || IS_ZERO_FLOAT(rlen))
	//	{
	//		return DEG_TO_RAD(360);
	//	}

	//	return angles += acos(t3d::vec3CosTheta(ldir, rdir));
	//}

	static float _caculateNearestDistance(
		const Vector3 & point,
		const Vector3 & v0,
		const Vector3 & v1)
	{
		Vector3 u = v1 - v0;

		float offset = (point - v0).dot(u) / u.magnitude();

		Vector3 closestPoint;
		if(offset <= 0)
		{
			closestPoint = v0;
		}
		else if(offset >= (v1 - v0).magnitude())
		{
			closestPoint = v1;
		}
		else
		{
			closestPoint = v0 + u.normalize() * offset;
		}

		return (closestPoint - point).magnitude();
	}

	std::pair<Vector3, Vector3> IntersectionTests::CalculateRay(const Matrix4 & InverseViewProj, const Vector3 & pos, const Vector2 & pt, const Vector2 & dim)
	{
		Vector3 ptProj(Lerp(-1.0f, 1.0f, pt.x / dim.x), Lerp(1.0f, -1.0f, pt.y / dim.y), 1.0f);

		return std::make_pair(pos, (ptProj.transformCoord(InverseViewProj) - pos).normalize());
	}

	IntersectionTests::TestResult IntersectionTests::rayAndParallelPlane(const Vector3 & pos, const Vector3 & dir, size_t axis_i, float value)
	{
		if (fabs(dir[axis_i]) < EPSILON_E6)
		{
			return TestResult(false, FLT_MAX);
		}
		return TestResult(true, (value - pos[axis_i]) / dir[axis_i]);
	}

	IntersectionTests::TestResult IntersectionTests::rayAndAABB(const Vector3 & pos, const Vector3 & dir, const AABB & aabb)
	{
		float tNear = FLT_MIN;
		float tFar = FLT_MAX;
		for (int i = 0; i < 3; i++)
		{
			if (fabs(dir[i]) < EPSILON_E6)
			{
				if (pos[i] < aabb.Min[i] || pos[i] > aabb.Max[i])
				{
					return TestResult(false, FLT_MAX);
				}
			}

			float t0 = (aabb.Min[i] - pos[i]) / dir[i];
			float t1 = (aabb.Max[i] - pos[i]) / dir[i];

			tNear = Max(tNear, Min(t0, t1));
			tFar = Min(tFar, Max(t0, t1));

			if (tNear > tFar || tFar < 0)
			{
				return TestResult(false, FLT_MAX);
			}
		}
		return TestResult(true, tNear > 0 ? tNear : tFar);
	}

	IntersectionTests::TestResult IntersectionTests::rayAndHalfSpace(
		const Vector3 & pos,
		const Vector3 & dir,
		const Plane & plane)
	{
		_ASSERT(IS_NORMALIZED(dir));

		float denom = plane.normal.dot(dir);

		if(abs(denom) < EPSILON_E6)
		{
			return TestResult(false, FLT_MAX);
		}

		return TestResult(true, -(plane.normal.dot(pos) + plane.d) / denom);
	}

	IntersectionTests::TestResult IntersectionTests::rayAndCylinder(
		const Vector3 & pos,
		const Vector3 & dir,
		const float cylinderRadius,
		const float cylinderHeight)
	{
		const float a = dir.y * dir.y + dir.z * dir.z;
		const float b = 2 * dir.y * pos.y + 2 * dir.z * pos.z;
		const float c = pos.y * pos.y + pos.z * pos.z - cylinderRadius * cylinderRadius;
		const float discrm = b * b - 4 * a * c;
		float t[4];
		Vector3 k[4];
		bool v[4];
		if(discrm > 0)
		{
			t[0] = (-b + sqrt(discrm)) / (2 * a);
			k[0] = pos + dir * t[0];
			v[0] = (k[0].x >= 0 && k[0].x <= cylinderHeight);
			t[1] = (-b - sqrt(discrm)) / (2 * a);
			k[1] = pos + dir * t[1];
			v[1] = (k[1].x >= 0 && k[1].x <= cylinderHeight);
			t[2] = (0 - pos.x) / dir.x;
			k[2] = pos + dir * t[2];
			v[2] = (k[2].yz.dot(k[2].yz) <= cylinderRadius * cylinderRadius);
			t[3] = (cylinderHeight - pos.x) / dir.x;
			k[3] = pos + dir * t[3];
			v[3] = (k[3].yz.dot(k[3].yz) <= cylinderRadius * cylinderRadius);
			float min_t = FLT_MAX;
			for(int i = 0; i < 4; i++)
			{
				if(v[i] && t[i] < min_t)
				{
					min_t = t[i];
				}
			}
			return TestResult(min_t < FLT_MAX, min_t);
		}
		else if(discrm == 0)
		{
			t[0] = -b / (2 * a);
			k[0] = pos + dir * t[0];
			v[0] = (k[0].x >= 0 && k[0].x <= cylinderHeight);
			return TestResult(v[0], t[0]);
		}
		return TestResult(false, FLT_MAX);
	}

	IntersectionTests::TestResult IntersectionTests::rayAndSphere(
		const Vector3 & pos,
		const Vector3 & dir,
		const Vector3 & sphereCenter,
		const float sphereRadius)
	{
		const Vector3 minusC = pos - sphereCenter;
		const float a = dir.dot(dir);
		const float b = 2 * dir.dot(minusC);
		const float c = minusC.dot(minusC) - sphereRadius * sphereRadius;
		const float discrm = b * b - 4 * a * c;
		float t[2];
		if(discrm > 0)
		{
			t[0] = (-b + sqrt(discrm)) / (2 * a);
			t[1] = (-b - sqrt(discrm)) / (2 * a);
			return TestResult(true, Min(t[0], t[1]));
		}
		else if(discrm == 0)
		{
			return TestResult(true, -b / (2 * a));
		}
		return TestResult(false, FLT_MAX);
	}

	Vector3 IntersectionTests::calculateTriangleDirection(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
	{
		return (v1 - v0).cross(v2 - v0);
	}

	bool IntersectionTests::isValidTriangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
	{
		return abs(calculateTriangleDirection(v0, v1, v2).magnitude()) > EPSILON_E6;
	}

	Vector3 IntersectionTests::calculateTriangleNormal(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
	{
		_ASSERT(isValidTriangle(v0, v1, v2));

		return calculateTriangleDirection(v0, v1, v2).normalize();
	}

	float IntersectionTests::isInsideTriangle(const Vector3 & point, const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
	{
		_ASSERT(isValidTriangle(v0, v1, v2));

		Vector3 planeDir = calculateTriangleDirection(v0, v1, v2);

		return planeDir.dot(calculateTriangleDirection(point, v0, v1)) >= 0
			&& planeDir.dot(calculateTriangleDirection(point, v1, v2)) >= 0
			&& planeDir.dot(calculateTriangleDirection(point, v2, v0)) >= 0;
	}

	IntersectionTests::TestResult IntersectionTests::rayAndTriangle(
		const Vector3 & pos,
		const Vector3 & dir,
		float radius,
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2)
	{
		_ASSERT(IS_NORMALIZED(dir));

		Vector3 normal = calculateTriangleNormal(v0, v1, v2);

		float denom = normal.dot(dir);

		if(denom > -EPSILON_E6)
		{
			return TestResult(false, FLT_MAX);
		}

		float t = -(normal.dot(pos) - normal.dot(v0)) / denom;

		Vector3 intersection = pos + dir * t;

		return TestResult(
			isInsideTriangle(intersection, v0, v1, v2)
			|| abs(_caculateNearestDistance(intersection, v0, v1)) <= radius
			|| abs(_caculateNearestDistance(intersection, v1, v2)) <= radius
			|| abs(_caculateNearestDistance(intersection, v2, v0)) <= radius, t);
	}

	IntersectionTests::IntersectionType IntersectionTests::IntersectAABBAndFrustum(const AABB & aabb, const Frustum & frustum)
	{
		IntersectionType ret = IntersectionTypeInside;
		for (int i = 0; i < 6; i++)
		{
			if (frustum[i].DistanceToPoint(aabb.p(frustum[i].normal)) <= 0)
			{
				return IntersectionTypeOutside;
			}
			else if (frustum[i].DistanceToPoint(aabb.n(frustum[i].normal)) < 0)
			{
				ret = IntersectionTypeIntersect;
			}
		}
		return ret;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// VolumnHelper
	// /////////////////////////////////////////////////////////////////////////////////////

	float VolumnHelper::calculateBoxVolume(float width, float height, float deepth)
	{
		return width * height * deepth;
	}

	float VolumnHelper::calculateBoxMass(float width, float height, float deepth, float density)
	{
		return calculateBoxVolume(width, height, deepth) * density;
	}

	float VolumnHelper::calculateSphereVolume(float radius)
	{
		return (float)4 / (float)3 * (float)D3DX_PI * radius * radius * radius;
	}

	float VolumnHelper::calculateSphereMass(float radius, float density)
	{
		return calculateSphereVolume(radius) * density;
	}

	Matrix4 VolumnHelper::calculateInertiaTensor(float Ixx, float Iyy, float Izz, float Ixy, float Ixz, float Iyz)
	{
		return Matrix4(
			 Ixx,			-Ixy,			-Ixz,			0,
			-Ixy,			 Iyy,			-Iyz,			0,
			-Ixz,			-Iyz,			 Izz,			0,
			0,				0,				0,				1);
	}

	Matrix4 VolumnHelper::calculateBoxInertiaTensor(const Vector3 & halfSizes, float mass)
	{
		Vector3 squares = halfSizes * halfSizes;

		return calculateInertiaTensor(
			0.3f * mass * (squares.y + squares.z),
			0.3f * mass * (squares.x + squares.z),
			0.3f * mass * (squares.x + squares.y));
	}

	Matrix4 VolumnHelper::calculateSphereInertiaTensor(float radius, float mass)
	{
		float coeff = 0.4f * mass * radius * radius;

		return calculateInertiaTensor(coeff, coeff, coeff);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionDetector
	// /////////////////////////////////////////////////////////////////////////////////////

	unsigned CollisionDetector::sphereAndHalfSpace(
		const CollisionSphere & sphere,
		const Plane & plane,
		float planeFriction,
		float planeRestitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		_ASSERT(IS_NORMALIZED(plane.normal));

		Vector3 spherePosition = sphere.getTransformAxis(3);

		float penetration = sphere.radius - plane.DistanceToPoint(spherePosition);

		if(penetration <= 0)
		{
			return 0;
		}

		contacts->contactNormal = plane.normal;
		contacts->penetration = penetration;
		contacts->contactPoint = spherePosition - plane.normal * (sphere.radius - penetration);
		contacts->bodys[0] = sphere.body;
		contacts->bodys[1] = NULL;
		contacts->friction = sphere.friction * planeFriction;
		contacts->restitution = sphere.restitution * planeRestitution;
		return 1;
	}

	unsigned CollisionDetector::sphereAndTruePlane(
		const CollisionSphere & sphere,
		const Plane & plane,
		float planeFriction,
		float planeRestitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		_ASSERT(IS_NORMALIZED(plane.normal));

		Vector3 spherePosition = sphere.getTransformAxis(3);

		float centreDistance = plane.DistanceToPoint(spherePosition);

		if(centreDistance > 0)
		{
			if(centreDistance >= sphere.radius)
			{
				return 0;
			}

			contacts->contactNormal = plane.normal;
			contacts->penetration = sphere.radius - centreDistance;
		}
		else
		{
			if(centreDistance <= -sphere.radius)
			{
				return 0;
			}

			contacts->contactNormal = -plane.normal;
			contacts->penetration = sphere.radius + centreDistance;
		}

		contacts->contactPoint = spherePosition - plane.normal * centreDistance;
		contacts->bodys[0] = sphere.body;
		contacts->bodys[1] = NULL;
		contacts->friction = sphere.friction * planeFriction;
		contacts->restitution = sphere.restitution * planeRestitution;
		return 1;
	}

	unsigned CollisionDetector::sphereAndPoint(
		const CollisionSphere & sphere,
		const Vector3 & point,
		RigidBody * pointBody,
		float pointFriction,
		float pointRestitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 direction = sphere.getTransformAxis(3) - point;

		float distance = direction.magnitude();

		if(distance >= sphere.radius)
		{
			return 0;
		}

		contacts->contactNormal = direction.normalize();
		contacts->penetration = sphere.radius - distance;
		contacts->contactPoint = point;
		contacts->bodys[0] = sphere.body;
		contacts->bodys[1] = pointBody;
		contacts->friction = sphere.friction * pointFriction;
		contacts->restitution = sphere.restitution * pointRestitution;
		return 1;
	}

	unsigned CollisionDetector::sphereAndLine(
		const CollisionSphere & sphere,
		const Vector3 & v0,
		const Vector3 & v1,
		RigidBody * lineBody,
		float lineFriction,
		float lineRestitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 u = v1 - v0;

		float offset = (sphere.getTransformAxis(3) - v0).dot(u) / u.magnitude();

		Vector3 closestPoint;
		if(offset <= 0)
		{
			closestPoint = v0;
		}
		else if(offset >= (v1 - v0).magnitude())
		{
			closestPoint = v1;
		}
		else
		{
			closestPoint = v0 + u.normalize() * offset;
		}

		return sphereAndPoint(sphere, closestPoint, lineBody, lineFriction, lineRestitution, contacts, limits);
	}

	float CollisionDetector::calculatePointPlaneDistance(
		const Vector3 & point,
		const Vector3 & planePoint,
		const Vector3 & planeNormal)
	{
		_ASSERT(IS_NORMALIZED(planeNormal));

		return (point - planePoint).dot(planeNormal);
	}

	unsigned CollisionDetector::sphereAndTriangle(
		const CollisionSphere & sphere,
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2,
		RigidBody * triangleBody,
		float triangleFriction,
		float triangleResitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 direction = calculateTriangleNormal(v0, v1, v2);

		float distance = abs(calculatePointPlaneDistance(sphere.getTransformAxis(3), v0, direction));

		if(distance >= sphere.radius)
		{
			return 0;
		}

		Vector3 intersection = sphere.getTransformAxis(3) - direction * distance;

		if(isInsideTriangle(intersection, v0, v1, v2))
		{
			contacts->contactNormal = direction;
			contacts->penetration = sphere.radius - distance;
			contacts->contactPoint = intersection;

			contacts->bodys[0] = sphere.body;
			contacts->bodys[1] = triangleBody;
			return 1;
		}

		unsigned res = sphereAndLine(sphere, v0, v1, triangleBody, triangleFriction, triangleResitution, contacts, limits);

		if(0 == res)
		{
			res = sphereAndLine(sphere, v1, v2, triangleBody, triangleFriction, triangleResitution, contacts, limits);
		}

		if(0 == res)
		{
			res = sphereAndLine(sphere, v2, v0, triangleBody, triangleFriction, triangleResitution, contacts, limits);
		}

		return res;
	}

	unsigned CollisionDetector::sphereAndSphere(
		const CollisionSphere & sphere0,
		const CollisionSphere & sphere1,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 position0 = sphere0.getTransformAxis(3);

		Vector3 position1 = sphere1.getTransformAxis(3);

		Vector3 midline = position0 - position1;

		float distance = midline.magnitude();

		/*
		 * NOTE:
		 *	two sphere in the same position cannot generate contact normal
		 *  so, just skip and return 0
		 */

		if(abs(distance) < EPSILON_E6 || distance >= sphere0.radius + sphere1.radius) // ***
		{
			return 0;
		}

		contacts->contactNormal = midline / distance;
		contacts->penetration = sphere0.radius + sphere1.radius - distance;
		//contacts->contactPoint = t3d::vec3Add(position0, t3d::vec3Mul(midline, (float)0.5));
		contacts->contactPoint = position0 - midline * 0.5f; // ***
		contacts->bodys[0] = sphere0.body;
		contacts->bodys[1] = sphere1.body;
		contacts->friction = sphere0.friction * sphere1.friction;
		contacts->restitution = sphere0.restitution * sphere1.restitution;
		return 1;
	}

	unsigned CollisionDetector::pointAndHalfSpace(
		const Vector3 & point,
		RigidBody * pointBody,
		float pointFriction,
		float pointRestitution,
		const Plane & plane,
		float planeFriction,
		float planeRestitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		float penetration = -plane.DistanceToPoint(point);

		if(penetration <= 0)
		{
			return 0;
		}

		contacts->contactNormal = plane.normal;
		contacts->penetration = penetration;
		contacts->contactPoint = point + plane.normal * (penetration * 0.5f); // ***
		contacts->bodys[0] = pointBody;
		contacts->bodys[1] = NULL;
		contacts->friction = pointFriction * planeFriction;
		contacts->restitution = pointRestitution * planeRestitution;
		return 1;
	}

	unsigned CollisionDetector::boxAndHalfSpace(
		const CollisionBox & box,
		const Plane & plane,
		float planeFriction,
		float planeRestitution,
		Contact * contacts,
		unsigned limits)
	{
		unsigned res = 0;

		if((res += pointAndHalfSpace(
			Vector3( box.halfSize.x,  box.halfSize.y,  box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			Vector3(-box.halfSize.x,  box.halfSize.y,  box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			Vector3( box.halfSize.x, -box.halfSize.y,  box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			Vector3( box.halfSize.x,  box.halfSize.y, -box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			Vector3(-box.halfSize.x, -box.halfSize.y, -box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			Vector3( box.halfSize.x, -box.halfSize.y, -box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			Vector3(-box.halfSize.x,  box.halfSize.y, -box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		return res += pointAndHalfSpace(
			Vector3(-box.halfSize.x, -box.halfSize.y,  box.halfSize.z).transformCoord(box.getTransform()),
			box.body,
			box.friction,
			box.restitution,
			plane,
			planeFriction,
			planeRestitution,
			&contacts[res],
			limits - res);
	}

	unsigned CollisionDetector::boxAndSphere(
		const CollisionBox & box,
		const CollisionSphere & sphere,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 centre = sphere.getTransformAxis(3);

		Vector3 relCentre = centre.transformCoord(box.getTransform().inverse());

		if(abs(relCentre.x) > box.halfSize.x + sphere.radius
			|| abs(relCentre.y) > box.halfSize.y + sphere.radius
			|| abs(relCentre.z) > box.halfSize.z + sphere.radius)
		{
			return 0;
		}

		Vector3 closestPoint(
			relCentre.x > 0 ? Min(relCentre.x, box.halfSize.x) : Max(relCentre.x, -box.halfSize.x),
			relCentre.y > 0 ? Min(relCentre.y, box.halfSize.y) : Max(relCentre.y, -box.halfSize.y),
			relCentre.z > 0 ? Min(relCentre.z, box.halfSize.z) : Max(relCentre.z, -box.halfSize.z));

		float distanceSquare = (relCentre - closestPoint).magnitudeSq();

		if(/*IS_ZERO_FLOAT(distanceSquare) || */distanceSquare >= sphere.radius * sphere.radius)
		{
			return 0;
		}

		/*
		 * NOTE:
		 *	if this sphere's centre was inside the box, the closest point was the sphere's centre
		 *	and then detector this point with box
		 */

		if(abs(distanceSquare) < EPSILON_E6) // ***
		{
			unsigned used = boxAndPointAways(box, centre, sphere.body, sphere.friction, sphere.restitution, contacts, limits);

			_ASSERT(1 == used && contacts->penetration >= -EPSILON_E3);

			contacts->penetration += sphere.radius;

			return used;
		}

		Vector3 closestPointWorld = closestPoint.transformCoord(box.getTransform());

		contacts->contactNormal = (closestPointWorld - centre).normalize();
		contacts->penetration = sphere.radius - sqrt(distanceSquare);
		contacts->contactPoint = closestPointWorld;
		contacts->bodys[0] = box.body;
		contacts->bodys[1] = sphere.body;
		contacts->friction = box.friction * sphere.friction;
		contacts->restitution = box.restitution * sphere.restitution;
		return 1;
	}

	unsigned CollisionDetector::boxAndPointAways(
		const CollisionBox & box,
		const Vector3 & point,
		RigidBody * pointBody,
		float pointFriction,
		float pointResitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 relPoint = point.transformCoord(box.getTransform().inverse());

		///*
		// * NOTE:
		// *	cannot generate contact normal for the point witch at the box's centre
		// */

		//if(IS_ZERO_FLOAT(relPoint.x) && IS_ZERO_FLOAT(relPoint.y) && IS_ZERO_FLOAT(relPoint.z))
		//{
		//	return 0;
		//}

		float xDepth = box.halfSize.x - abs(relPoint.x);
		//if(xDepth < 0)
		//{
		//	return 0;
		//}

		float yDepth = box.halfSize.y - abs(relPoint.y);
		//if(yDepth < 0)
		//{
		//	return 0;
		//}

		float zDepth = box.halfSize.z - abs(relPoint.z);
		//if(zDepth < 0)
		//{
		//	return 0;
		//}

		/*
		 * NOTE:
		 *	always generate contact, whenever the point is or not inside the box
		 */

		if(xDepth < yDepth)
		{
			if(xDepth < zDepth)
			{
				contacts->contactNormal = relPoint.x < 0 ? -box.getTransformAxis(0) : box.getTransformAxis(0);
				contacts->penetration = xDepth;
			}
			else
			{
				contacts->contactNormal = relPoint.x < 0 ? -box.getTransformAxis(2) : box.getTransformAxis(2);
				contacts->penetration = zDepth;
			}
		}
		else
		{
			if(yDepth < zDepth)
			{
				contacts->contactNormal = relPoint.x < 0 ? -box.getTransformAxis(1) : box.getTransformAxis(1);
				contacts->penetration = yDepth;
			}
			else
			{
				contacts->contactNormal = relPoint.x < 0 ? -box.getTransformAxis(2) : box.getTransformAxis(2);
				contacts->penetration = zDepth;
			}
		}

		contacts->contactPoint = point;
		contacts->bodys[0] = box.body;
		contacts->bodys[1] = pointBody;
		contacts->friction = box.friction * pointFriction;
		contacts->restitution = box.restitution * pointResitution;
		return 1;
	}

	//static float _penetrationOnAxis(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const Vector3 & axis,
	//	const Vector3 & toCentre)
	//{
	//	return _transformToAxis(box0, axis) + _transformToAxis(box1, axis) - abs(t3d::vec3Dot(toCentre, axis)); // ***
	//}

	//static bool _tryAxis(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const Vector3 & axis,
	//	const Vector3 & toCentre,
	//	unsigned index,
	//	float & smallestPenetration,
	//	unsigned & smallestIndex)
	//{
	//	_ASSERT(IS_ZERO_FLOAT(t3d::vec3Length(axis) - 1));

	//	float penetration = _penetrationOnAxis(box0, box1, axis, toCentre);

	//	if(penetration <= 0)
	//	{
	//		return false;
	//	}

	//	if(penetration < smallestPenetration)
	//	{
	//		smallestPenetration = penetration;
	//		smallestIndex = index;
	//	}

	//	return true;
	//}

	//static bool _zeroAxisOrTryAxis(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const Vector3 & axis,
	//	const Vector3 & toCentre,
	//	unsigned index,
	//	float & smallestPenetration,
	//	unsigned & smallestIndex)
	//{
	//	if(IS_ZERO_FLOAT(axis.x) && IS_ZERO_FLOAT(axis.y) && IS_ZERO_FLOAT(axis.z))
	//	{
	//		return true;
	//	}

	//	return _tryAxis(box0, box1, t3d::vec3Normalize(axis), toCentre, index, smallestPenetration, smallestIndex);
	//}

	//static Vector3 _getBoxAxisByIndex(const CollisionBox & box, unsigned index)
	//{
	//	switch(index)
	//	{
	//	case 0:
	//		return box.getTransformAxis(0);

	//	case 1:
	//		return box.getTransformAxis(1);

	//	case 2:
	//		return box.getTransformAxis(2);
	//	}

	//	_ASSERT(false); return Vector3::ZERO;
	//}

	//static unsigned _detectorPointFaceBoxAndBox(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const Vector3 & axis,
	//	const Vector3 & toCentre,
	//	float penetration,
	//	Contact * contacts,
	//	unsigned limits)
	//{
	//	_ASSERT(limits > 0);

	//	_ASSERT(IS_ZERO_FLOAT(t3d::vec3Length(axis) - 1));

	//	Vector3 normal = t3d::vec3Dot(axis, toCentre) > 0 ? t3d::vec3Neg(axis) : axis;

	//	contacts->contactNormal = normal;
	//	contacts->penetration = penetration;
	//	contacts->contactPoint = Vector3(
	//		t3d::vec3Dot(box1.getTransformAxis(0), normal) < 0 ? -box1.halfSize.x : box1.halfSize.x,
	//		t3d::vec3Dot(box1.getTransformAxis(1), normal) < 0 ? -box1.halfSize.y : box1.halfSize.y,
	//		t3d::vec3Dot(box1.getTransformAxis(2), normal) < 0 ? -box1.halfSize.z : box1.halfSize.z) * box1.getTransform();
	//	//contacts->contactPoint = Vector3(
	//	//	t3d::vec3Dot(box1.getTransformAxis(0), toCentre) > 0 ? -box1.halfSize.x : box1.halfSize.x,
	//	//	t3d::vec3Dot(box1.getTransformAxis(1), toCentre) > 0 ? -box1.halfSize.y : box1.halfSize.y,
	//	//	t3d::vec3Dot(box1.getTransformAxis(2), toCentre) > 0 ? -box1.halfSize.z : box1.halfSize.z) * box1.getTransform(); // ***

	//	contacts->bodys[0] = box0.body;
	//	contacts->bodys[1] = box1.body;
	//	return 1;

	//	UNREFERENCED_PARAMETER(box0);
	//}

	static Vector3 _contactPoint(
		const Vector3 & pOne,
		const Vector3 & dOne,
		float oneSize,
		const Vector3 & pTwo,
		const Vector3 & dTwo,
		float twoSize,
		bool useOne)
	{
		/*
		 * NOTE:
		 *	calcuate contact point between edge and edge
		 */

		float smOne = dOne.magnitudeSq();
		float smTwo = dTwo.magnitudeSq();
		float dpOneTwo = dTwo.dot(dOne);

		Vector3 toSt = pOne - pTwo;
		float dpStaOne = dOne.dot(toSt);
		float dpStaTwo = dTwo.dot(toSt);

		float denom = smOne * smTwo - dpOneTwo * dpOneTwo;

		if(abs(denom) < EPSILON_E6)
		{
//REPORT_ERROR(_T("ddd"));
			return useOne ? pOne : pTwo;
		}

		float mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
		float mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

		if(mua > oneSize
			|| mua < -oneSize
			|| mub > twoSize
			|| mub < -twoSize)
		{
//REPORT_ERROR(_T("eee"));
			return useOne ? pOne : pTwo;
		}

//REPORT_ERROR(_T("fff"));
		Vector3 cOne = pOne + dOne * mua;
		Vector3 cTwo = pTwo + dTwo * mub;

		return (cOne + cTwo) * 0.5f;
	}

	float CollisionDetector::calculateBoxAxisAndBoxPenetration(
		const CollisionBox & box0,
		const Vector3 & axis,
		const CollisionBox & box1)
	{
		Vector3 toCentre = box1.getTransformAxis(3) - box0.getTransformAxis(3);

		return calculateBoxAxisHalfProjection(box0, axis)
			+ calculateBoxAxisHalfProjection(box1, axis) - abs(toCentre.dot(axis)); // ***
	}

	//template <typename elem_t>
	//elem_t judgeValueByOffset(float offset, float limit, elem_t value)
	//{
	//	if(offset > limit)
	//		return value;

	//	if(offset < -limit)
	//		return -value;

	//	return 0;
	//}

	Vector3 CollisionDetector::findPointFromBoxByDirection(const CollisionBox & box, const Vector3 & dir)
	{
		return Vector3(
			box.getTransformAxis(0).dot(dir) >= 0 ? box.halfSize.x : -box.halfSize.x,
			box.getTransformAxis(1).dot(dir) >= 0 ? box.halfSize.y : -box.halfSize.y,
			box.getTransformAxis(2).dot(dir) >= 0 ? box.halfSize.z : -box.halfSize.z).transformCoord(box.getTransform());
	}

	bool CollisionDetector::_tryBoxAxisAndBox(
		const CollisionBox & box0,
		const Vector3 & axis,
		const CollisionBox & box1,
		unsigned index,
		float & smallestPenetration,
		unsigned & smallestIndex)
	{
		_ASSERT(IS_NORMALIZED(axis));

		float penetration = calculateBoxAxisAndBoxPenetration(box0, axis, box1);

		if(penetration <= 0)
		{
			return false;
		}

		if(penetration < smallestPenetration)
		{
			smallestPenetration = penetration;
			smallestIndex = index;
		}

		return true;
	}

	bool CollisionDetector::_zeroAxisOrTryBoxAxisAndBox(
		const CollisionBox & box0,
		const Vector3 & axis,
		const CollisionBox & box1,
		unsigned index,
		float & smallestPenetration,
		unsigned & smallestIndex)
	{
		if(axis.magnitude() < EPSILON_E6)
		{
			return true;
		}

		return _tryBoxAxisAndBox(box0, axis.normalize(), box1, index, smallestPenetration, smallestIndex);
	}

	unsigned CollisionDetector::_detectorBoxAxisAndBoxPoint(
		const CollisionBox & box0,
		const Vector3 & axis,
		const CollisionBox & box1,
		float penetration,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		_ASSERT(IS_NORMALIZED(axis));

		Vector3 toCentre = box1.getTransformAxis(3) - box0.getTransformAxis(3);

		contacts->contactNormal = axis.dot(toCentre) > 0 ? -axis : axis;
		contacts->penetration = penetration;
		contacts->contactPoint = findPointFromBoxByDirection(box1, contacts->contactNormal);
		contacts->bodys[0] = box0.body;
		contacts->bodys[1] = box1.body;
		contacts->friction = box0.friction * box1.friction;
		contacts->restitution = box0.restitution * box1.restitution;
		return 1;
	}

	unsigned CollisionDetector::boxAndBox(
		const CollisionBox & box0,
		const CollisionBox & box1,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		Vector3 toCentre = box1.getTransformAxis(3) - box0.getTransformAxis(3);

		float smallestPenetration = FLT_MAX;

		unsigned smallestIndex = UINT_MAX;

		unsigned smallestSingleAxis;

		/*
		 * NOTE:
		 *	intersection test for project from each axis, and determine whether point to face or edge to edge
		 */

		//if(!_tryAxis(box0, box1, box0.getTransformAxis(0), toCentre, 0, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box0.getTransformAxis(1), toCentre, 1, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box0.getTransformAxis(2), toCentre, 2, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box1.getTransformAxis(0), toCentre, 3, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box1.getTransformAxis(1), toCentre, 4, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box1.getTransformAxis(2), toCentre, 5, smallestPenetration, smallestIndex)
		//	|| (smallestSingleAxis = smallestIndex, false)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(0), box1.getTransformAxis(0)), toCentre, 6, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(0), box1.getTransformAxis(1)), toCentre, 7, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(0), box1.getTransformAxis(2)), toCentre, 8, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(1), box1.getTransformAxis(0)), toCentre, 9, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(1), box1.getTransformAxis(1)), toCentre, 10, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(1), box1.getTransformAxis(2)), toCentre, 11, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(2), box1.getTransformAxis(0)), toCentre, 12, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(2), box1.getTransformAxis(1)), toCentre, 13, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis(2), box1.getTransformAxis(2)), toCentre, 14, smallestPenetration, smallestIndex))

		if(!_tryBoxAxisAndBox(box0, box0.getTransformAxis(0), box1, 0, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box0.getTransformAxis(1), box1, 1, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box0.getTransformAxis(2), box1, 2, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box1.getTransformAxis(0), box1, 3, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box1.getTransformAxis(1), box1, 4, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box1.getTransformAxis(2), box1, 5, smallestPenetration, smallestIndex)
			|| (smallestSingleAxis = smallestIndex, false)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(0).cross(box1.getTransformAxis(0)), box1, 6, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(0).cross(box1.getTransformAxis(1)), box1, 7, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(0).cross(box1.getTransformAxis(2)), box1, 8, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(1).cross(box1.getTransformAxis(0)), box1, 9, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(1).cross(box1.getTransformAxis(1)), box1, 10, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(1).cross(box1.getTransformAxis(2)), box1, 11, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(2).cross(box1.getTransformAxis(0)), box1, 12, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(2).cross(box1.getTransformAxis(1)), box1, 13, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, box0.getTransformAxis(2).cross(box1.getTransformAxis(2)), box1, 14, smallestPenetration, smallestIndex))
		{
//REPORT_ERROR(_T("aaa"));
			return 0;
		}

		_ASSERT(UINT_MAX != smallestIndex);

		if(smallestIndex < 3)
		{
//REPORT_ERROR(_T("bbb"));
			//return _detectorPointFaceBoxAndBox(box0, box1, _getBoxAxisByIndex(box0, smallestIndex), toCentre, smallestPenetration, contacts, limits);
			return _detectorBoxAxisAndBoxPoint(box0, box0.getTransformAxis(smallestIndex), box1, smallestPenetration, contacts, limits);
		}
		else if(smallestIndex < 6)
		{
//REPORT_ERROR(_T("ccc - reverse"));
			//return _detectorPointFaceBoxAndBox(box1, box0, _getBoxAxisByIndex(box1, smallestIndex - 3), t3d::vec3Neg(toCentre), smallestPenetration, contacts, limits);
			return _detectorBoxAxisAndBoxPoint(box1, box1.getTransformAxis(smallestIndex - 3), box0, smallestPenetration, contacts, limits);
		}

		smallestIndex -= 6;

		unsigned box0AxisIndex = smallestIndex / 3;
		unsigned box1AxisIndex = smallestIndex % 3;

		Vector3 box0Axis = box0.getTransformAxis(box0AxisIndex);
		Vector3 box1Axis = box1.getTransformAxis(box1AxisIndex);

		Vector3 axis = box0Axis.cross(box1Axis).normalize();

		if(axis.dot(toCentre) > 0)
		{
			axis = -axis;
		}

		Vector3 pointOnBox0Edge = Vector3(
			0 == box0AxisIndex ? 0 : (box0.getTransformAxis(0).dot(axis) > 0 ? -box0.halfSize.x : box0.halfSize.x),
			1 == box0AxisIndex ? 0 : (box0.getTransformAxis(1).dot(axis) > 0 ? -box0.halfSize.y : box0.halfSize.y),
			2 == box0AxisIndex ? 0 : (box0.getTransformAxis(2).dot(axis) > 0 ? -box0.halfSize.z : box0.halfSize.z)).transformCoord(box0.getTransform());

		Vector3 pointOnBox1Edge = Vector3(
			0 == box1AxisIndex ? 0 : (box1.getTransformAxis(0).dot(axis) < 0 ? -box1.halfSize.x : box1.halfSize.x),
			1 == box1AxisIndex ? 0 : (box1.getTransformAxis(1).dot(axis) < 0 ? -box1.halfSize.y : box1.halfSize.y),
			2 == box1AxisIndex ? 0 : (box1.getTransformAxis(2).dot(axis) < 0 ? -box1.halfSize.z : box1.halfSize.z)).transformCoord(box1.getTransform());

		contacts->contactNormal = axis;
		contacts->penetration = smallestPenetration;
		contacts->contactPoint = _contactPoint(
			pointOnBox0Edge,
			box0Axis,
			box0.halfSize[box0AxisIndex],
			pointOnBox1Edge,
			box1Axis,
			box1.halfSize[box1AxisIndex],
			smallestSingleAxis > 2); // ***

		contacts->bodys[0] = box0.body;
		contacts->bodys[1] = box1.body;
		contacts->friction = box0.friction * box1.friction;
		contacts->restitution = box0.restitution * box1.restitution;
		return 1;
	}

	bool CollisionDetector::_tryBoxAxisAndTriangle(
		const CollisionBox & box,
		const Vector3 & axis,
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2,
		unsigned index,
		float & smallestPenetration,
		unsigned & smallestIndex)
	{
		_ASSERT(IS_NORMALIZED(axis));

		float penetration = calculateBoxAxisAndTrianglePenetration(box, axis, v0, v1, v2);

		if(penetration <= 0)
		{
			return false;
		}

		if(penetration < smallestPenetration)
		{
			smallestPenetration = penetration;
			smallestIndex = index;
		}

		return true;
	}

	bool CollisionDetector::_zeroAxisOrTryBoxAxisAndTriangle(
		const CollisionBox & box,
		const Vector3 & axis,
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2,
		unsigned index,
		float & smallestPenetration,
		unsigned & smallestIndex)
	{
		if(axis.magnitude() < EPSILON_E6)
		{
			return true;
		}

		return _tryBoxAxisAndTriangle(box, axis.normalize(), v0, v1, v2, index, smallestPenetration, smallestIndex);
	}

	float CollisionDetector::calculateBoxAxisAndTrianglePenetration(
		const CollisionBox & box,
		const Vector3 & axis,
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2)
	{
		_ASSERT(IS_NORMALIZED(axis));

		Vector3 triCentor = (v0 + v1 + v2) / 3.0f;

		Vector3 toCentre = triCentor - box.getTransformAxis(3);

		float triHalfProjection;
		if(toCentre.dot(axis) > 0)
			triHalfProjection = Min((v0 - triCentor).dot(axis), (v1 - triCentor).dot(axis), (v2 - triCentor).dot(axis));
		else
			triHalfProjection = Max((v0 - triCentor).dot(axis), (v1 - triCentor).dot(axis), (v2 - triCentor).dot(axis));

		return abs(triHalfProjection)
			+ calculateBoxAxisHalfProjection(box, axis) - abs(toCentre.dot(axis));
	}

	Vector3 CollisionDetector::findPointFromTriangleByDirection(
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2,
		const Vector3 & dir)
	{
		float proj0 = v0.dot(dir);
		float proj1 = v1.dot(dir);
		float proj2 = v2.dot(dir);

		if(proj0 < proj1)
		{
			if(proj0 < proj2)
			{
				return v0;
			}
		}
		else
		{
			if(proj1 < proj2)
			{
				return v1;
			}
		}

		return v2;
	}

	unsigned CollisionDetector::boxAndTriangle(
		const CollisionBox & box,
		const Vector3 & v0,
		const Vector3 & v1,
		const Vector3 & v2,
		RigidBody * triangleBody,
		float triangleFriction,
		float triangleRestitution,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		float smallestPenetration = FLT_MAX;

		unsigned smallestIndex = UINT_MAX;

		unsigned smallestSingleAxis;

		if(!_tryBoxAxisAndTriangle(box, calculateTriangleNormal(v0, v1, v2), v0, v1, v2, 0, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndTriangle(box, box.getTransformAxis(0), v0, v1, v2, 1, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndTriangle(box, box.getTransformAxis(1), v0, v1, v2, 2, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndTriangle(box, box.getTransformAxis(2), v0, v1, v2, 3, smallestPenetration, smallestIndex)
			|| (smallestSingleAxis = smallestIndex, false)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(0).cross(v1 - v0), v0, v1, v2, 4, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(0).cross(v2 - v1), v0, v1, v2, 5, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(0).cross(v0 - v2), v0, v1, v2, 6, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(1).cross(v1 - v0), v0, v1, v2, 7, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(1).cross(v2 - v1), v0, v1, v2, 8, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(1).cross(v0 - v2), v0, v1, v2, 9, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(2).cross(v1 - v0), v0, v1, v2, 10, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(2).cross(v2 - v1), v0, v1, v2, 11, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndTriangle(box, box.getTransformAxis(2).cross(v0 - v2), v0, v1, v2, 12, smallestPenetration, smallestIndex))
		{
			return 0;
		}

		if(0 == smallestIndex)
		{
			contacts->contactNormal = -calculateTriangleNormal(v0, v1, v2);
			contacts->penetration = smallestPenetration;
			contacts->contactPoint = findPointFromBoxByDirection(box, contacts->contactNormal);
			contacts->bodys[0] = box.body;
			contacts->bodys[1] = triangleBody;
			return 1;
		}

		if(smallestIndex < 4)
		{
			contacts->contactNormal = -box.getTransformAxis(smallestIndex - 1);
			contacts->penetration = smallestPenetration;
			contacts->contactPoint = findPointFromTriangleByDirection(v0, v1, v2, contacts->contactNormal);
			contacts->bodys[0] = box.body;
			contacts->bodys[1] = triangleBody;
			return 1;
		}

		smallestIndex -= 4;

		unsigned boxAxisIndex = smallestIndex / 3;
		unsigned triAxisIndex = smallestIndex % 3;

		Vector3 boxAxis = box.getTransformAxis(boxAxisIndex);
		Vector3 triEdge;
		Vector3 triVert;
		switch(triAxisIndex)
		{
		case 0:
			triEdge = v1 - v0;
			triVert = v0;
			break;
		case 1:
			triEdge = v2 - v1;
			triVert = v1;
			break;
		case 2:
			triEdge = v0 - v2;
			triVert = v2;
			break;
		default:
			_ASSERT(false); return 0;
		}

		Vector3 axis = boxAxis.cross(triEdge).normalize();

		Vector3 triCentor = (v0 + v1 + v2) / 3.0f;

		Vector3 toCentre = triCentor - box.getTransformAxis(3);

		if(toCentre.dot(axis) > 0)
		{
			axis = -axis;
		}

		Vector3 pointOnBoxEdge = Vector3(
			0 == boxAxisIndex ? 0 : (box.getTransformAxis(0).dot(axis) > 0 ? -box.halfSize.x : box.halfSize.x),
			1 == boxAxisIndex ? 0 : (box.getTransformAxis(1).dot(axis) > 0 ? -box.halfSize.y : box.halfSize.y),
			2 == boxAxisIndex ? 0 : (box.getTransformAxis(2).dot(axis) > 0 ? -box.halfSize.z : box.halfSize.z)).transformCoord(box.getTransform());

		contacts->contactNormal = axis;
		contacts->penetration = smallestPenetration;
		contacts->contactPoint = _contactPoint(
			pointOnBoxEdge,
			boxAxis,
			box.halfSize[boxAxisIndex],
			triVert,
			triEdge.normalize(),
			triEdge.magnitude() * 0.5f,
			smallestSingleAxis > 0);

		contacts->bodys[0] = box.body;
		contacts->bodys[1] = triangleBody;
		contacts->friction = box.friction * triangleFriction;
		contacts->restitution = box.restitution * triangleRestitution;
		return 1;
	}
}

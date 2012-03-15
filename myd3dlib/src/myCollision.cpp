
#include "stdafx.h"
#include "myCollision.h"

namespace my
{
	//// /////////////////////////////////////////////////////////////////////////////////////
	//// BoundingSphere
	//// /////////////////////////////////////////////////////////////////////////////////////

	//const t3d::Vec4<real> & BoundingSphere::getCenter(void) const
	//{
	//	return center;
	//}

	//void BoundingSphere::setCenter(const t3d::Vec4<real> & _center)
	//{
	//	center = _center;
	//}

	//const real BoundingSphere::getRadius(void) const
	//{
	//	return radius;
	//}

	//void BoundingSphere::setRadius(real _radius)
	//{
	//	radius = _radius;
	//}

	//BoundingSphere::BoundingSphere(const t3d::Vec4<real> & _center, real _radius)
	//	: center(_center)
	//	, radius(_radius)
	//{
	//}

	//bool BoundingSphere::overlaps(const BoundingSphere & other) const
	//{
	//	return t3d::vec3LengthSquare(t3d::vec3Sub(center, other.center)) < (radius + other.radius) * (radius + other.radius);
	//}

	//real BoundingSphere::getGrowth(const BoundingSphere & other) const
	//{
	//	BoundingSphere newSphere = buildBoundingSphere(*this, other);

	//	return newSphere.getRadius() * newSphere.getRadius() - radius * radius;
	//}

	//real BoundingSphere::getVolumn(void) const
	//{
	//	return (real)4.0 / (real)3.0 * (real)PI * radius * radius * radius;
	//}

	//BoundingSphere buildBoundingSphere(const BoundingSphere & lhs, const BoundingSphere & rhs)
	//{
	//	t3d::Vec4<real> centerOffset = t3d::vec3Sub(lhs.getCenter(), rhs.getCenter());
	//	real distanceSquare = t3d::vec3LengthSquare(centerOffset);
	//	real radiusDiff = lhs.getRadius() - rhs.getRadius();

	//	if(radiusDiff * radiusDiff >= distanceSquare)
	//	{
	//		if(lhs.getRadius() >= rhs.getRadius())
	//		{
	//			return BoundingSphere(lhs.getCenter(), lhs.getRadius());
	//		}
	//		else
	//		{
	//			return BoundingSphere(rhs.getCenter(), rhs.getRadius());
	//		}
	//	}
	//	else
	//	{
	//		real distance = sqrt(distanceSquare);
	//		real newRadius = (lhs.getRadius() + distance + rhs.getRadius()) / 2;
	//		t3d::Vec4<real> newCenter = t3d::vec3Add(lhs.getCenter(), t3d::vec3Mul(centerOffset, (newRadius - lhs.getRadius()) / distance));

	//		return BoundingSphere(newCenter, newRadius);
	//	}
	//}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionPrimitive
	// /////////////////////////////////////////////////////////////////////////////////////

	CollisionPrimitive::CollisionPrimitive(
		RigidBody * _body,
		const t3d::Mat4<real> & _offset /*= my::Mat4<real>::IDENTITY*/,
		const t3d::Mat4<real> & _rotationOffset /*= my::Mat4<real>::IDENTITY*/)
		: body(_body)
		, offset(_offset)
		, rotationOffset(_rotationOffset)
	{
	}

	CollisionPrimitive::~CollisionPrimitive(void)
	{
	}

	void CollisionPrimitive::calculateInternals(void)
	{
		_ASSERT(NULL != body);

		transform = body->getTransform() * offset;

		rotationTransform = body->getRotationTransform() * rotationOffset;
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionSphere
	// /////////////////////////////////////////////////////////////////////////////////////

	CollisionSphere::CollisionSphere(
		real _radius,
		RigidBody * _body,
		const t3d::Mat4<real> & _offset /*= my::Mat4<real>::IDENTITY*/,
		const t3d::Mat4<real> & _rotationOffset /*= my::Mat4<real>::IDENTITY*/)
		: CollisionPrimitive(_body, _offset, _rotationOffset)
		, radius(_radius)
	{
	}

	CollisionSphere::CollisionSphere(void)
		: CollisionPrimitive(NULL)
	{
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionBox
	// /////////////////////////////////////////////////////////////////////////////////////

	CollisionBox::CollisionBox(
		const t3d::Vec4<real> & _halfSize,
		RigidBody * _body,
		const t3d::Mat4<real> & _offset /*= my::Mat4<real>::IDENTITY*/,
		const t3d::Mat4<real> & _rotationOffset /*= my::Mat4<real>::IDENTITY*/)
		: CollisionPrimitive(_body, _offset, _rotationOffset)
		, halfSize(_halfSize)
	{
	}

	CollisionBox::CollisionBox(void)
		: CollisionPrimitive(NULL)
	{
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionPlane
	// /////////////////////////////////////////////////////////////////////////////////////

	CollisionPlane::CollisionPlane(
		const t3d::Vec4<real> & direction,
		real _distance)
		: normal(t3d::vec3Normalize(direction))
		, distance(_distance)
	{
	}

	CollisionPlane::CollisionPlane(void)
	{
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// IntersectionTests
	// /////////////////////////////////////////////////////////////////////////////////////

	bool IntersectionTests::sphereAndHalfSpace(
		const CollisionSphere & sphere,
		const t3d::Vec4<real> & planeNormal,
		real planeDistance)
	{
		_ASSERT(t3d::vec3IsNormalized(planeNormal));

		return t3d::vec3Dot(sphere.getTransformAxis3(), planeNormal) - sphere.radius < planeDistance;
	}

	bool IntersectionTests::sphereAndSphere(
		const CollisionSphere & sphere0,
		const CollisionSphere & sphere1)
	{
		return t3d::vec3LengthSquare(t3d::vec3Sub(sphere0.getTransformAxis3(), sphere1.getTransformAxis3())) < (sphere0.radius + sphere1.radius) * (sphere0.radius + sphere1.radius);
	}

	real IntersectionTests::calculateBoxAxisHalfProjection(const CollisionBox & box, const t3d::Vec4<real> & axis)
	{
		_ASSERT(t3d::vec3IsNormalized(axis));

		return
			box.halfSize.x * abs(t3d::vec3Dot(axis, box.getTransformAxis0())) +
			box.halfSize.y * abs(t3d::vec3Dot(axis, box.getTransformAxis1())) +
			box.halfSize.z * abs(t3d::vec3Dot(axis, box.getTransformAxis2())); // ***
	}

	bool IntersectionTests::boxAndHalfSpace(
		const CollisionBox & box,
		const t3d::Vec4<real> & planeNormal,
		real planeDistance)
	{
		_ASSERT(t3d::vec3IsNormalized(planeNormal));

		return t3d::vec3Dot(box.getTransformAxis3(), planeNormal) - calculateBoxAxisHalfProjection(box, planeNormal) < planeDistance;
	}

	bool IntersectionTests::_overlapOnAxis(
		const CollisionBox & box0,
		const CollisionBox & box1,
		const t3d::Vec4<real> & axis,
		const t3d::Vec4<real> & toCentre)
	{
		return
			abs(t3d::vec3Dot(toCentre, axis)) < calculateBoxAxisHalfProjection(box0, axis) + calculateBoxAxisHalfProjection(box1, axis); // ***
	}

	bool IntersectionTests::_zeroAxisOrOverlapOnAxis(
		const CollisionBox & box0,
		const CollisionBox & box1,
		const t3d::Vec4<real> & axis,
		const t3d::Vec4<real> & toCentre)
	{
		if(IS_ZERO_FLOAT(axis.x) && IS_ZERO_FLOAT(axis.y) && IS_ZERO_FLOAT(axis.z))
		{
			return true;
		}
		
		return _overlapOnAxis(box0, box1, axis, toCentre);
	}

	bool IntersectionTests::boxAndBox(
		const CollisionBox & box0,
		const CollisionBox & box1)
	{
		t3d::Vec4<real> toCentre = t3d::vec3Sub(box1.getTransformAxis3(), box0.getTransformAxis3());

		return
			_overlapOnAxis(box0, box1, box0.getTransformAxis0(), toCentre) &&
			_overlapOnAxis(box0, box1, box0.getTransformAxis1(), toCentre) &&
			_overlapOnAxis(box0, box1, box0.getTransformAxis2(), toCentre) &&

			_overlapOnAxis(box0, box1, box1.getTransformAxis0(), toCentre) &&
			_overlapOnAxis(box0, box1, box1.getTransformAxis1(), toCentre) &&
			_overlapOnAxis(box0, box1, box1.getTransformAxis2(), toCentre) &&

			/*
			 * NOTE:
			 *	some cross of two box will be invalid if their have the same axis, but if this occure
			 *	only one axis should be checked, and have alread checked, so just continue
			 */

			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis0()), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis1()), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis2()), toCentre) &&

			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis0()), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis1()), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis2()), toCentre) &&

			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis0()), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis1()), toCentre) &&
			_zeroAxisOrOverlapOnAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis2()), toCentre); // ***
	}

	//static real _caculateInternalAngles(
	//	const t3d::Vec4<real> & v0,
	//	const t3d::Vec4<real> & v1,
	//	const t3d::Vec4<real> & v2,
	//	const t3d::Vec4<real> & intersection)
	//{
	//	t3d::Vec4<real> ldir = t3d::vec3Sub(v0, intersection);
	//	t3d::Vec4<real> rdir = t3d::vec3Sub(v1, intersection);
	//	real llen = t3d::vec3LengthSquare(ldir);
	//	real rlen = t3d::vec3LengthSquare(rdir);
	//	if(IS_ZERO_FLOAT(llen) || IS_ZERO_FLOAT(rlen))
	//	{
	//		return DEG_TO_RAD(360);
	//	}

	//	real angles = acos(t3d::vec3CosTheta(ldir, rdir));

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

	static real _caculateNearestDistance(
		const t3d::Vec4<real> & point,
		const t3d::Vec4<real> & v0,
		const t3d::Vec4<real> & v1)
	{
		t3d::Vec4<real> u = t3d::vec3Sub(v1, v0);

		real offset = t3d::vec3Dot(t3d::vec3Sub(point, v0), u) / t3d::vec3Length(u);

		t3d::Vec4<real> closestPoint;
		if(offset <= 0)
		{
			closestPoint = v0;
		}
		else if(offset >= t3d::vec3Length(t3d::vec3Sub(v1, v0)))
		{
			closestPoint = v1;
		}
		else
		{
			closestPoint = t3d::vec3Add(v0, t3d::vec3Mul(t3d::vec3Normalize(u), offset));
		}

		return t3d::vec3Length(t3d::vec3Sub(closestPoint, point));
	}

	std::pair<bool, real> IntersectionTests::rayAndHalfTriangle(
		const t3d::Vec4<real> & pos,
		const t3d::Vec4<real> & dir,
		real radius,
		const t3d::Vec4<real> & v0,
		const t3d::Vec4<real> & v1,
		const t3d::Vec4<real> & v2)
	{
		_ASSERT(IS_ZERO_FLOAT(t3d::vec3Length(dir) - 1));

		t3d::Vec4<real> normal = calculateTriangleNormal(v0, v1, v2);

		real denom = t3d::vec3Dot(normal, dir);

		if(denom > -REAL_ZERO_LIMIT)
		{
			return std::pair<bool, real>(false, REAL_MAX);
		}

		real t = -(t3d::vec3Dot(normal, pos) - t3d::vec3Dot(normal, v0)) / denom;

		t3d::Vec4<real> intersection = t3d::vec3Add(pos, t3d::vec3Mul(dir, t));

		return std::pair<bool, real>(
			isInsideTriangle(intersection, v0, v1, v2)
			|| abs(_caculateNearestDistance(intersection, v0, v1)) <= radius
			|| abs(_caculateNearestDistance(intersection, v1, v2)) <= radius
			|| abs(_caculateNearestDistance(intersection, v2, v0)) <= radius, t);
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionDetector
	// /////////////////////////////////////////////////////////////////////////////////////

	unsigned CollisionDetector::sphereAndHalfSpace(
		const CollisionSphere & sphere,
		const t3d::Vec4<real> & planeNormal,
		real planeDistance,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		_ASSERT(t3d::vec3IsNormalized(planeNormal));

		t3d::Vec4<real> spherePosition = sphere.getTransformAxis3();

		real penetration = sphere.radius + planeDistance - t3d::vec3Dot(spherePosition, planeNormal);

		if(penetration <= 0)
		{
			return 0;
		}

		contacts->contactNormal = planeNormal;
		contacts->penetration = penetration;
		contacts->contactPoint = t3d::vec3Sub(spherePosition, t3d::vec3Mul(planeNormal, sphere.radius - penetration));

		contacts->bodys[0] = sphere.body;
		contacts->bodys[1] = NULL;
		return 1;
	}

	unsigned CollisionDetector::sphereAndTruePlane(
		const CollisionSphere & sphere,
		const t3d::Vec4<real> & planeNormal,
		real planeDistance,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		_ASSERT(t3d::vec3IsNormalized(planeNormal));

		t3d::Vec4<real> spherePosition = sphere.getTransformAxis3();

		real centreDistance = t3d::vec3Dot(spherePosition, planeNormal) - planeDistance;

		if(centreDistance > 0)
		{
			if(centreDistance >= sphere.radius)
			{
				return 0;
			}

			contacts->contactNormal = planeNormal;
			contacts->penetration = sphere.radius - centreDistance;
		}
		else
		{
			if(centreDistance <= -sphere.radius)
			{
				return 0;
			}

			contacts->contactNormal = t3d::vec3Neg(planeNormal);
			contacts->penetration = sphere.radius + centreDistance;
		}
		contacts->contactPoint = t3d::vec3Sub(spherePosition, t3d::vec3Mul(planeNormal, centreDistance));

		contacts->bodys[0] = sphere.body;
		contacts->bodys[1] = NULL;
		return 1;
	}

	unsigned CollisionDetector::sphereAndPoint(
		const CollisionSphere & sphere,
		const t3d::Vec4<real> & point,
		RigidBody * bodyForPoint,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		t3d::Vec4<real> direction = t3d::vec3Sub(sphere.getTransformAxis3(), point);

		real distance = t3d::vec3Length(direction);

		if(distance >= sphere.radius)
		{
			return 0;
		}

		contacts->contactNormal = t3d::vec3Normalize(direction);
		contacts->penetration = sphere.radius - distance;
		contacts->contactPoint = point;

		contacts->bodys[0] = sphere.body;
		contacts->bodys[1] = bodyForPoint;
		return 1;
	}

	unsigned CollisionDetector::sphereAndLine(
		const CollisionSphere & sphere,
		const t3d::Vec4<real> & v0,
		const t3d::Vec4<real> & v1,
		RigidBody * bodyForLine,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		t3d::Vec4<real> u = t3d::vec3Sub(v1, v0);

		real offset = t3d::vec3Dot(t3d::vec3Sub(sphere.getTransformAxis3(), v0), u) / t3d::vec3Length(u);

		t3d::Vec4<real> closestPoint;
		if(offset <= 0)
		{
			closestPoint = v0;
		}
		else if(offset >= t3d::vec3Length(t3d::vec3Sub(v1, v0)))
		{
			closestPoint = v1;
		}
		else
		{
			closestPoint = t3d::vec3Add(v0, t3d::vec3Mul(t3d::vec3Normalize(u), offset));
		}

		return sphereAndPoint(sphere, closestPoint, bodyForLine, contacts, limits);
	}

	unsigned CollisionDetector::sphereAndTriangle(
		const CollisionSphere & sphere,
		const t3d::Vec4<real> & v0,
		const t3d::Vec4<real> & v1,
		const t3d::Vec4<real> & v2,
		RigidBody * bodyForTriangle,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		my::Vec4<real> direction = calculateTriangleNormal(v0, v1, v2);

		real distance = abs(calculatePointPlaneDistance(sphere.getTransformAxis3(), v0, direction));

		if(distance >= sphere.radius)
		{
			return 0;
		}

		my::Vec4<real> intersection = t3d::vec3Sub(sphere.getTransformAxis3(), t3d::vec3Mul(direction, distance));

		if(isInsideTriangle(intersection, v0, v1, v2))
		{
			contacts->contactNormal = direction;
			contacts->penetration = sphere.radius - distance;
			contacts->contactPoint = intersection;

			contacts->bodys[0] = sphere.body;
			contacts->bodys[1] = bodyForTriangle;
			return 1;
		}

		unsigned res = sphereAndLine(sphere, v0, v1, bodyForTriangle, contacts, limits);

		if(0 == res)
		{
			res = sphereAndLine(sphere, v1, v2, bodyForTriangle, contacts, limits);
		}

		if(0 == res)
		{
			res = sphereAndLine(sphere, v2, v0, bodyForTriangle, contacts, limits);
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

		t3d::Vec4<real> position0 = sphere0.getTransformAxis3();

		t3d::Vec4<real> position1 = sphere1.getTransformAxis3();

		t3d::Vec4<real> midline = t3d::vec3Sub(position0, position1);

		real distance = t3d::vec3Length(midline);

		/*
		 * NOTE:
		 *	two sphere in the same position cannot generate contact normal
		 *  so, just skip and return 0
		 */

		if(IS_ZERO_FLOAT(distance) || distance >= sphere0.radius + sphere1.radius) // ***
		{
			return 0;
		}

		contacts->contactNormal = midline / distance;
		contacts->penetration = sphere0.radius + sphere1.radius - distance;
		//contacts->contactPoint = t3d::vec3Add(position0, t3d::vec3Mul(midline, (real)0.5));
		contacts->contactPoint = t3d::vec3Sub(position0, t3d::vec3Mul(midline, (real)0.5)); // ***

		contacts->bodys[0] = sphere0.body;
		contacts->bodys[1] = sphere1.body;
		return 1;
	}

	unsigned CollisionDetector::pointAndHalfSpace(
		const t3d::Vec4<real> & point,
		RigidBody * body,
		const t3d::Vec4<real> & planeNormal,
		real planeDistance,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		real penetration = -calculatePointPlaneDistance(point, planeNormal, planeDistance);

		if(penetration <= 0)
		{
			return 0;
		}

		contacts->contactNormal = planeNormal;
		contacts->penetration = penetration;
		contacts->contactPoint = t3d::vec3Add(point, t3d::vec3Mul(planeNormal, penetration * 0.5f)); // ***

		contacts->bodys[0] = body;
		contacts->bodys[1] = NULL;
		return 1;
	}

	unsigned CollisionDetector::boxAndHalfSpace(
		const CollisionBox & box,
		const t3d::Vec4<real> & planeNormal,
		real planeDistance,
		Contact * contacts,
		unsigned limits)
	{
		unsigned res = 0;

		if((res += pointAndHalfSpace(
			my::Vec4<real>( box.halfSize.x,  box.halfSize.y,  box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			my::Vec4<real>(-box.halfSize.x,  box.halfSize.y,  box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			my::Vec4<real>( box.halfSize.x, -box.halfSize.y,  box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			my::Vec4<real>( box.halfSize.x,  box.halfSize.y, -box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			my::Vec4<real>(-box.halfSize.x, -box.halfSize.y, -box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			my::Vec4<real>( box.halfSize.x, -box.halfSize.y, -box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		if((res += pointAndHalfSpace(
			my::Vec4<real>(-box.halfSize.x,  box.halfSize.y, -box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
			&contacts[res],
			limits - res)) >= limits)
		{
			return res;
		}

		return res += pointAndHalfSpace(
			my::Vec4<real>(-box.halfSize.x, -box.halfSize.y,  box.halfSize.z) * box.getTransform(),
			box.body,
			planeNormal,
			planeDistance,
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

		t3d::Vec4<real> centre = sphere.getTransformAxis3();

		t3d::Vec4<real> relCentre = centre * box.getTransform().inverse();

		if(abs(relCentre.x) > box.halfSize.x + sphere.radius
			|| abs(relCentre.y) > box.halfSize.y + sphere.radius
			|| abs(relCentre.z) > box.halfSize.z + sphere.radius)
		{
			return 0;
		}

		my::Vec4<real> closestPoint(
			relCentre.x > 0 ? std::min(relCentre.x, box.halfSize.x) : std::max(relCentre.x, -box.halfSize.x),
			relCentre.y > 0 ? std::min(relCentre.y, box.halfSize.y) : std::max(relCentre.y, -box.halfSize.y),
			relCentre.z > 0 ? std::min(relCentre.z, box.halfSize.z) : std::max(relCentre.z, -box.halfSize.z));

		real distanceSquare = t3d::vec3LengthSquare(t3d::vec3Sub(relCentre, closestPoint));

		if(/*IS_ZERO_FLOAT(distanceSquare) || */distanceSquare >= sphere.radius * sphere.radius)
		{
			return 0;
		}

		/*
		 * NOTE:
		 *	if this sphere's centre was inside the box, the closest point was the sphere's centre
		 *	and then detector this point with box
		 */

		if(IS_ZERO_FLOAT(distanceSquare)) // ***
		{
			unsigned used = boxAndPointAways(box, centre, sphere.body, contacts, limits);

			_ASSERT(1 == used && contacts->penetration >= -EPSILON_E3);

			contacts->penetration += sphere.radius;

			return used;
		}

		t3d::Vec4<real> closestPointWorld = closestPoint * box.getTransform();

		contacts->contactNormal = t3d::vec3Normalize(t3d::vec3Sub(closestPointWorld, centre));
		contacts->penetration = sphere.radius - sqrt(distanceSquare);
		contacts->contactPoint = closestPointWorld;

		contacts->bodys[0] = box.body;
		contacts->bodys[1] = sphere.body;
		return 1;
	}

	unsigned CollisionDetector::boxAndPointAways(
		const CollisionBox & box,
		const t3d::Vec4<real> & point,
		RigidBody * bodyForPoint,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		t3d::Vec4<real> relPoint = point * box.getTransform().inverse();

		///*
		// * NOTE:
		// *	cannot generate contact normal for the point witch at the box's centre
		// */

		//if(IS_ZERO_FLOAT(relPoint.x) && IS_ZERO_FLOAT(relPoint.y) && IS_ZERO_FLOAT(relPoint.z))
		//{
		//	return 0;
		//}

		real xDepth = box.halfSize.x - abs(relPoint.x);
		//if(xDepth < 0)
		//{
		//	return 0;
		//}

		real yDepth = box.halfSize.y - abs(relPoint.y);
		//if(yDepth < 0)
		//{
		//	return 0;
		//}

		real zDepth = box.halfSize.z - abs(relPoint.z);
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
				contacts->contactNormal = relPoint.x < 0 ? t3d::vec3Neg(box.getTransformAxis0()) : box.getTransformAxis0();
				contacts->penetration = xDepth;
			}
			else
			{
				contacts->contactNormal = relPoint.x < 0 ? t3d::vec3Neg(box.getTransformAxis2()) : box.getTransformAxis2();
				contacts->penetration = zDepth;
			}
		}
		else
		{
			if(yDepth < zDepth)
			{
				contacts->contactNormal = relPoint.x < 0 ? t3d::vec3Neg(box.getTransformAxis1()) : box.getTransformAxis1();
				contacts->penetration = yDepth;
			}
			else
			{
				contacts->contactNormal = relPoint.x < 0 ? t3d::vec3Neg(box.getTransformAxis2()) : box.getTransformAxis2();
				contacts->penetration = zDepth;
			}
		}

		contacts->contactPoint = point;

		contacts->bodys[0] = box.body;
		contacts->bodys[1] = bodyForPoint;
		return 1;
	}

	//static real _penetrationOnAxis(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & toCentre)
	//{
	//	return _transformToAxis(box0, axis) + _transformToAxis(box1, axis) - abs(t3d::vec3Dot(toCentre, axis)); // ***
	//}

	//static bool _tryAxis(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & toCentre,
	//	unsigned index,
	//	real & smallestPenetration,
	//	unsigned & smallestIndex)
	//{
	//	_ASSERT(IS_ZERO_FLOAT(t3d::vec3Length(axis) - 1));

	//	real penetration = _penetrationOnAxis(box0, box1, axis, toCentre);

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
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & toCentre,
	//	unsigned index,
	//	real & smallestPenetration,
	//	unsigned & smallestIndex)
	//{
	//	if(IS_ZERO_FLOAT(axis.x) && IS_ZERO_FLOAT(axis.y) && IS_ZERO_FLOAT(axis.z))
	//	{
	//		return true;
	//	}

	//	return _tryAxis(box0, box1, t3d::vec3Normalize(axis), toCentre, index, smallestPenetration, smallestIndex);
	//}

	//static t3d::Vec4<real> _getBoxAxisByIndex(const CollisionBox & box, unsigned index)
	//{
	//	switch(index)
	//	{
	//	case 0:
	//		return box.getTransformAxis0();

	//	case 1:
	//		return box.getTransformAxis1();

	//	case 2:
	//		return box.getTransformAxis2();
	//	}

	//	_ASSERT(false); return my::Vec4<real>::ZERO;
	//}

	//static unsigned _detectorPointFaceBoxAndBox(
	//	const CollisionBox & box0,
	//	const CollisionBox & box1,
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & toCentre,
	//	real penetration,
	//	Contact * contacts,
	//	unsigned limits)
	//{
	//	_ASSERT(limits > 0);

	//	_ASSERT(IS_ZERO_FLOAT(t3d::vec3Length(axis) - 1));

	//	t3d::Vec4<real> normal = t3d::vec3Dot(axis, toCentre) > 0 ? t3d::vec3Neg(axis) : axis;

	//	contacts->contactNormal = normal;
	//	contacts->penetration = penetration;
	//	contacts->contactPoint = my::Vec4<real>(
	//		t3d::vec3Dot(box1.getTransformAxis0(), normal) < 0 ? -box1.halfSize.x : box1.halfSize.x,
	//		t3d::vec3Dot(box1.getTransformAxis1(), normal) < 0 ? -box1.halfSize.y : box1.halfSize.y,
	//		t3d::vec3Dot(box1.getTransformAxis2(), normal) < 0 ? -box1.halfSize.z : box1.halfSize.z) * box1.getTransform();
	//	//contacts->contactPoint = my::Vec4<real>(
	//	//	t3d::vec3Dot(box1.getTransformAxis0(), toCentre) > 0 ? -box1.halfSize.x : box1.halfSize.x,
	//	//	t3d::vec3Dot(box1.getTransformAxis1(), toCentre) > 0 ? -box1.halfSize.y : box1.halfSize.y,
	//	//	t3d::vec3Dot(box1.getTransformAxis2(), toCentre) > 0 ? -box1.halfSize.z : box1.halfSize.z) * box1.getTransform(); // ***

	//	contacts->bodys[0] = box0.body;
	//	contacts->bodys[1] = box1.body;
	//	return 1;

	//	UNREFERENCED_PARAMETER(box0);
	//}

	static t3d::Vec4<real> _contactPoint(
		const t3d::Vec4<real> & pOne,
		const t3d::Vec4<real> & dOne,
		real oneSize,
		const t3d::Vec4<real> & pTwo,
		const t3d::Vec4<real> & dTwo,
		real twoSize,
		bool useOne)
	{
		/*
		 * NOTE:
		 *	why the algorithm for this funciton could not be understund by me ???
		 */

		real smOne = t3d::vec3LengthSquare(dOne);
		real smTwo = t3d::vec3LengthSquare(dTwo);
		real dpOneTwo = t3d::vec3Dot(dTwo, dOne);

		t3d::Vec4<real> toSt = t3d::vec3Sub(pOne, pTwo);
		real dpStaOne = t3d::vec3Dot(dOne, toSt);
		real dpStaTwo = t3d::vec3Dot(dTwo, toSt);

		real denom = smOne * smTwo - dpOneTwo * dpOneTwo;

		if(IS_ZERO_FLOAT(denom))
		{
//REPORT_ERROR(_T("ddd"));
			return useOne ? pOne : pTwo;
		}

		real mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
		real mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

		if(mua > oneSize
			|| mua < -oneSize
			|| mub > twoSize
			|| mub < -twoSize)
		{
//REPORT_ERROR(_T("eee"));
			return useOne ? pOne : pTwo;
		}

//REPORT_ERROR(_T("fff"));
		t3d::Vec4<real> cOne = t3d::vec3Add(pOne, t3d::vec3Mul(dOne, mua));
		t3d::Vec4<real> cTwo = t3d::vec3Add(pTwo, t3d::vec3Mul(dTwo, mub));

		return t3d::vec3Mul(t3d::vec3Add(cOne, cTwo), (real)0.5);
	}

	real CollisionDetector::calculateBoxAxisAndBoxPenetration(
		const CollisionBox & box0,
		const t3d::Vec4<real> & axis,
		const CollisionBox & box1)
	{
		t3d::Vec4<real> toCentre = t3d::vec3Sub(box1.getTransformAxis3(), box0.getTransformAxis3());

		return IntersectionTests::calculateBoxAxisHalfProjection(box0, axis)
			+ IntersectionTests::calculateBoxAxisHalfProjection(box1, axis) - abs(t3d::vec3Dot(toCentre, axis)); // ***
	}

	//template <typename elem_t>
	//elem_t judgeValueByOffset(real offset, real limit, elem_t value)
	//{
	//	if(offset > limit)
	//		return value;

	//	if(offset < -limit)
	//		return -value;

	//	return 0;
	//}

	t3d::Vec4<real> CollisionDetector::findPointFromBoxByDirection(const CollisionBox & box, const t3d::Vec4<real> & dir)
	{
		return t3d::vec3Build(
			t3d::vec3Dot(box.getTransformAxis0(), dir) >= 0 ? box.halfSize.x : -box.halfSize.x,
			t3d::vec3Dot(box.getTransformAxis1(), dir) >= 0 ? box.halfSize.y : -box.halfSize.y,
			t3d::vec3Dot(box.getTransformAxis2(), dir) >= 0 ? box.halfSize.z : -box.halfSize.z) * box.getTransform();
	}

	bool CollisionDetector::_tryBoxAxisAndBox(
		const CollisionBox & box0,
		const t3d::Vec4<real> & axis,
		const CollisionBox & box1,
		unsigned index,
		real & smallestPenetration,
		unsigned & smallestIndex)
	{
		_ASSERT(t3d::vec3IsNormalized(axis));

		real penetration = calculateBoxAxisAndBoxPenetration(box0, axis, box1);

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
		const t3d::Vec4<real> & axis,
		const CollisionBox & box1,
		unsigned index,
		real & smallestPenetration,
		unsigned & smallestIndex)
	{
		if(t3d::vec3IsZero(axis))
		{
			return true;
		}

		return _tryBoxAxisAndBox(box0, t3d::vec3Normalize(axis), box1, index, smallestPenetration, smallestIndex);
	}

	unsigned CollisionDetector::_detectorBoxAxisAndBoxPoint(
		const CollisionBox & box0,
		const t3d::Vec4<real> & axis,
		const CollisionBox & box1,
		real penetration,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		_ASSERT(t3d::vec3IsNormalized(axis));

		t3d::Vec4<real> toCentre = t3d::vec3Sub(box1.getTransformAxis3(), box0.getTransformAxis3());

		contacts->contactNormal = t3d::vec3Dot(axis, toCentre) > 0 ? t3d::vec3Neg(axis) : axis;
		contacts->penetration = penetration;
		contacts->contactPoint = findPointFromBoxByDirection(box1, contacts->contactNormal);
		contacts->bodys[0] = box0.body;
		contacts->bodys[1] = box1.body;
		return 1;
	}

	unsigned CollisionDetector::boxAndBox(
		const CollisionBox & box0,
		const CollisionBox & box1,
		Contact * contacts,
		unsigned limits)
	{
		_ASSERT(limits > 0);

		t3d::Vec4<real> toCentre = t3d::vec3Sub(box1.getTransformAxis3(), box0.getTransformAxis3());

		real smallestPenetration = REAL_MAX;

		unsigned smallestIndex = UINT_MAX;

		unsigned smallestSingleAxis;

		/*
		 * NOTE:
		 *	why the algorithm for this funciton could not be understund by me ???
		 */

		//if(!_tryAxis(box0, box1, box0.getTransformAxis0(), toCentre, 0, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box0.getTransformAxis1(), toCentre, 1, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box0.getTransformAxis2(), toCentre, 2, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box1.getTransformAxis0(), toCentre, 3, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box1.getTransformAxis1(), toCentre, 4, smallestPenetration, smallestIndex)
		//	|| !_tryAxis(box0, box1, box1.getTransformAxis2(), toCentre, 5, smallestPenetration, smallestIndex)
		//	|| (smallestSingleAxis = smallestIndex, false)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis0()), toCentre, 6, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis1()), toCentre, 7, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis2()), toCentre, 8, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis0()), toCentre, 9, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis1()), toCentre, 10, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis2()), toCentre, 11, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis0()), toCentre, 12, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis1()), toCentre, 13, smallestPenetration, smallestIndex)
		//	|| !_zeroAxisOrTryAxis(box0, box1, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis2()), toCentre, 14, smallestPenetration, smallestIndex))

		if(!_tryBoxAxisAndBox(box0, box0.getTransformAxis0(), box1, 0, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box0.getTransformAxis1(), box1, 1, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box0.getTransformAxis2(), box1, 2, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box1.getTransformAxis0(), box1, 3, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box1.getTransformAxis1(), box1, 4, smallestPenetration, smallestIndex)
			|| !_tryBoxAxisAndBox(box0, box1.getTransformAxis2(), box1, 5, smallestPenetration, smallestIndex)
			|| (smallestSingleAxis = smallestIndex, false)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis0()), box1, 6, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis1()), box1, 7, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis0(), box1.getTransformAxis2()), box1, 8, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis0()), box1, 9, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis1()), box1, 10, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis1(), box1.getTransformAxis2()), box1, 11, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis0()), box1, 12, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis1()), box1, 13, smallestPenetration, smallestIndex)
			|| !_zeroAxisOrTryBoxAxisAndBox(box0, t3d::vec3Cross(box0.getTransformAxis2(), box1.getTransformAxis2()), box1, 14, smallestPenetration, smallestIndex))
		{
//REPORT_ERROR(_T("aaa"));
			return 0;
		}

		_ASSERT(UINT_MAX != smallestIndex);

		if(smallestIndex < 3)
		{
//REPORT_ERROR(_T("bbb"));
			//return _detectorPointFaceBoxAndBox(box0, box1, _getBoxAxisByIndex(box0, smallestIndex), toCentre, smallestPenetration, contacts, limits);
			return _detectorBoxAxisAndBoxPoint(box0, box0.getTransformAxisN(smallestIndex), box1, smallestPenetration, contacts, limits);
		}
		else if(smallestIndex < 6)
		{
//REPORT_ERROR(_T("ccc - reverse"));
			//return _detectorPointFaceBoxAndBox(box1, box0, _getBoxAxisByIndex(box1, smallestIndex - 3), t3d::vec3Neg(toCentre), smallestPenetration, contacts, limits);
			return _detectorBoxAxisAndBoxPoint(box1, box1.getTransformAxisN(smallestIndex - 3), box0, smallestPenetration, contacts, limits);
		}

		smallestIndex -= 6;

		unsigned box0AxisIndex = smallestIndex / 3;
		unsigned box1AxisIndex = smallestIndex % 3;

		t3d::Vec4<real> box0Axis = box0.getTransformAxisN(box0AxisIndex);
		t3d::Vec4<real> box1Axis = box1.getTransformAxisN(box1AxisIndex);

		t3d::Vec4<real> axis = t3d::vec3Normalize(t3d::vec3Cross(box0Axis, box1Axis));

		if(t3d::vec3Dot(axis, toCentre) > 0)
		{
			t3d::vec3NegSelf(axis);
		}

		t3d::Vec4<real> pointOnBox0Edge = my::Vec4<real>(
			0 == box0AxisIndex ? 0 : (t3d::vec3Dot(box0.getTransformAxis0(), axis) > 0 ? -box0.halfSize.x : box0.halfSize.x),
			1 == box0AxisIndex ? 0 : (t3d::vec3Dot(box0.getTransformAxis1(), axis) > 0 ? -box0.halfSize.y : box0.halfSize.y),
			2 == box0AxisIndex ? 0 : (t3d::vec3Dot(box0.getTransformAxis2(), axis) > 0 ? -box0.halfSize.z : box0.halfSize.z)) * box0.getTransform();

		t3d::Vec4<real> pointOnBox1Edge = my::Vec4<real>(
			0 == box1AxisIndex ? 0 : (t3d::vec3Dot(box1.getTransformAxis0(), axis) < 0 ? -box1.halfSize.x : box1.halfSize.x),
			1 == box1AxisIndex ? 0 : (t3d::vec3Dot(box1.getTransformAxis1(), axis) < 0 ? -box1.halfSize.y : box1.halfSize.y),
			2 == box1AxisIndex ? 0 : (t3d::vec3Dot(box1.getTransformAxis2(), axis) < 0 ? -box1.halfSize.z : box1.halfSize.z)) * box1.getTransform();

		contacts->contactNormal = axis;
		contacts->penetration = smallestPenetration;
		contacts->contactPoint = _contactPoint(
			pointOnBox0Edge,
			box0Axis,
			box0.halfSize._M[box0AxisIndex],
			pointOnBox1Edge,
			box1Axis,
			box1.halfSize._M[box1AxisIndex],
			smallestSingleAxis > 2); // ***

		contacts->bodys[0] = box0.body;
		contacts->bodys[1] = box1.body;
		return 1;
	}

	//static real calculateBoxAxisAndTrianglePenetration(
	//	const CollisionBox & box,
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & v0,
	//	const t3d::Vec4<real> & v1,
	//	const t3d::Vec4<real> & v2)
	//{
	//	_ASSERT(t3d::vec3IsNormalized(axis));

	//	t3d::Vec4<real> planeNormal = t3d::vec3Neg(axis);

	//	real smallestDistance = t3d::min(
	//		calculatePointPlaneDistance(box.getTransformAxis3(), v0, planeNormal),
	//		calculatePointPlaneDistance(box.getTransformAxis3(), v1, planeNormal),
	//		calculatePointPlaneDistance(box.getTransformAxis3(), v2, planeNormal));

	//	return calculateBoxAxisHalfProjection(box, axis) - smallestDistance;
	//}

	//static t3d::Vec4<real> findPointFromTriangleByDirection(
	//	const t3d::Vec4<real> & v0,
	//	const t3d::Vec4<real> & v1,
	//	const t3d::Vec4<real> & v2,
	//	const t3d::Vec4<real> & dir)
	//{
	//	real proj0 = t3d::vec3Dot(v0, dir);
	//	real proj1 = t3d::vec3Dot(v1, dir);
	//	real proj2 = t3d::vec3Dot(v2, dir);

	//	if(proj0 > proj1)
	//	{
	//		if(proj0 > proj2)
	//		{
	//			return v0;
	//		}
	//	}
	//	else
	//	{
	//		if(proj1 > proj2)
	//		{
	//			return v1;
	//		}
	//	}

	//	return v2;
	//}

	//static bool _tryBoxAxisAndTriangle(
	//	const CollisionBox & box,
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & v0,
	//	const t3d::Vec4<real> & v1,
	//	const t3d::Vec4<real> & v2,
	//	unsigned index,
	//	real & smallestPenetration,
	//	unsigned & smallestIndex)
	//{
	//	_ASSERT(t3d::vec3IsNormalized(axis));

	//	real penetration = calculateBoxAxisAndTrianglePenetration(box, axis, v0, v1, v2);

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

	//static bool _zeroAxisOrTryBoxAxisAndTriangle(
	//	const CollisionBox & box,
	//	const t3d::Vec4<real> & axis,
	//	const t3d::Vec4<real> & v0,
	//	const t3d::Vec4<real> & v1,
	//	const t3d::Vec4<real> & v2,
	//	unsigned index,
	//	real & smallestPenetration,
	//	unsigned & smallestIndex)
	//{
	//	if(t3d::vec3IsZero(axis))
	//	{
	//		return true;
	//	}

	//	return _tryBoxAxisAndTriangle(box, t3d::vec3Normalize(axis), v0, v1, v2, index, smallestPenetration, smallestIndex);
	//}

	//unsigned CollisionDetector::boxAndTriangle(
	//	const CollisionBox & box,
	//	const t3d::Vec4<real> & v0,
	//	const t3d::Vec4<real> & v1,
	//	const t3d::Vec4<real> & v2,
	//	RigidBody * bodyForTriangle,
	//	Contact * contacts,
	//	unsigned limits)
	//{
	//	_ASSERT(limits > 0);

	//	real smallestPenetration = REAL_MAX;

	//	unsigned smallestIndex = UINT_MAX;

	//	unsigned smallestSingleAxis;

	//	if(!_tryBoxAxisAndTriangle(box, calculateTriangleNormal(v0, v1, v2), v0, v1, v2, 0, smallestPenetration, smallestIndex)
	//		|| !_tryBoxAxisAndTriangle(box, box.getTransformAxis0(), v0, v1, v2, 1, smallestPenetration, smallestIndex)
	//		|| !_tryBoxAxisAndTriangle(box, box.getTransformAxis1(), v0, v1, v2, 2, smallestPenetration, smallestIndex)
	//		|| !_tryBoxAxisAndTriangle(box, box.getTransformAxis2(), v0, v1, v2, 3, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis0(), t3d::vec3Sub(v1, v0)), v0, v1, v2, 4, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis0(), t3d::vec3Sub(v2, v1)), v0, v1, v2, 5, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis0(), t3d::vec3Sub(v0, v2)), v0, v1, v2, 6, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis1(), t3d::vec3Sub(v1, v0)), v0, v1, v2, 7, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis1(), t3d::vec3Sub(v2, v1)), v0, v1, v2, 8, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis1(), t3d::vec3Sub(v0, v2)), v0, v1, v2, 9, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis2(), t3d::vec3Sub(v1, v0)), v0, v1, v2, 10, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis2(), t3d::vec3Sub(v2, v1)), v0, v1, v2, 11, smallestPenetration, smallestIndex)
	//		|| !_zeroAxisOrTryBoxAxisAndTriangle(box, t3d::vec3Cross(box.getTransformAxis2(), t3d::vec3Sub(v0, v2)), v0, v1, v2, 12, smallestPenetration, smallestIndex))
	//	{
	//		return 0;
	//	}

	//	if(0 == smallestIndex)
	//	{
	//		contacts->contactNormal = t3d::vec3Neg(calculateTriangleNormal(v0, v1, v2));
	//		contacts->penetration = smallestPenetration;
	//		contacts->contactPoint = findPointFromBoxByDirection(box, contacts->contactNormal);
	//		contacts->bodys[0] = box.body;
	//		contacts->bodys[1] = bodyForTriangle;
	//		return 1;
	//	}

	//	if(smallestIndex < 4)
	//	{
	//		contacts->contactNormal = t3d::vec3Neg(box.getTransformAxisN(smallestIndex - 1));
	//		contacts->penetration = smallestPenetration;
	//		contacts->contactPoint = findPointFromTriangleByDirection(v0, v1, v2, contacts->contactNormal);
	//		contacts->bodys[0] = box.body;
	//		contacts->bodys[1] = bodyForTriangle;
	//		return 1;
	//	}

	//	_ASSERT(false); return 0;
	//}
}

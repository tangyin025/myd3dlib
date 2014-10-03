
#ifndef __MYCOLLISION_H__
#define __MYCOLLISION_H__

#include "myMath.h"
#include <vector>

namespace my
{
	class Contact;

	class RigidBody;

	// /////////////////////////////////////////////////////////////////////////////////////
	// BoundingSphere
	// /////////////////////////////////////////////////////////////////////////////////////

	class BoundingSphere
	{
	protected:
		Vector3 center;

		float radius;

	public:
		const Vector3 & getCenter(void) const
		{
			return center;
		}

		void setCenter(const Vector3 & _center)
		{
			center = _center;
		}

		const float getRadius(void) const
		{
			return radius;
		}

		void setRadius(float _radius)
		{
			radius = _radius;
		}

	public:
		BoundingSphere(const Vector3 & _center, float _radius);

		bool overlaps(const BoundingSphere & other) const;

		float getGrowth(const BoundingSphere & other) const;

		float getVolumn(void) const;
	};

	BoundingSphere buildBoundingSphere(const BoundingSphere & lhs, const BoundingSphere & rhs);

	// /////////////////////////////////////////////////////////////////////////////////////
	// PotentialContact
	// /////////////////////////////////////////////////////////////////////////////////////

	struct PotentialContact
	{
		RigidBody * body0;

		RigidBody * body1;
	};

	//// /////////////////////////////////////////////////////////////////////////////////////
	//// BVHNode
	//// /////////////////////////////////////////////////////////////////////////////////////

	//template <class BoundingVolumeClass>
	//class BVHNode
	//{
	//protected:
	//	class BoundingVolumeLt
	//	{
	//	protected:
	//		BoundingVolumeClass volume;

	//	public:
	//		BoundingVolumeLt(const BoundingVolumeClass & _volume)
	//			: volume(_volume)
	//		{
	//		}

	//		bool operator () (const BVHNode<BoundingVolumeClass> & lhs, const BVHNode<BoundingVolumeClass> & rhs)
	//		{
	//			return lhs.volume.getGrowth(volume) < rhs.volume.getGrowth(volume);
	//		}
	//	};

	//protected:
	//	typedef std::vector<BVHNode<BoundingVolumeClass> > BVHNodeList;

	//	BVHNodeList childs;

	//	BoundingVolumeClass volume;

	//	RigidBody * body;

	//protected:
	//	bool overlaps(const BVHNode<BoundingVolumeClass> & other) const
	//	{
	//		return volume.overlap(other.volume);
	//	}

	//	unsigned getPotentialContactsWith(const BVHNode<BoundingVolumeClass> & other, PotentialContact * contacts, unsigned limit) const
	//	{
	//		if(limit > 0)
	//		{
	//			if(overlaps(other))
	//			{
	//				if(isLeaf())
	//				{
	//					if(other.isLeaf())
	//					{
	//						contacts[0].body0 = body;
	//						contacts[0].body1 = other.body;
	//						return 1;
	//					}

	//					return other.getPotentialContactsWith(*this, contants, limit);
	//				}

	//				unsigned nret = 0;
	//				BVHNodeList::constant_iterator child_iter = childs.begin();
	//				for(; child_iter != childs.end(); child_iter++)
	//				{
	//					nret += child_iter->getPotentialContactsWith(other, &contacts[nret], limit - nret);
	//				}
	//				return nret;
	//			}
	//		}
	//		return 0;
	//	}

	//	void recalculateBoundingVolume(void)
	//	{
	//		_ASSERT(!childs.empty());

	//		BVHNodeList::const_iterator child_iter = childs.begin();
	//		volume = child_iter->volume;
	//		child_iter++;
	//		for(; child_iter != childs.end(); child_iter++)
	//		{
	//			volume = buildBoundingSphere(volume, child_iter->volume);
	//		}
	//	}

	//public:
	//	BVHNode(const BoundingVolumeClass & _volume, RigidBody * _body = NULL)
	//		: volume(_volume)
	//		, body(_body)
	//	{
	//	}

	//	bool isLeaf(void) const
	//	{
	//		if(NULL == body)
	//		{
	//			_ASSERT(childs.empty());

	//			return true;
	//		}

	//		_ASSERT(!childs.empty());

	//		return false;
	//	}

	//	unsigned getPotentialContacts(PotentialContact * contacts, unsigned limit) const
	//	{
	//		if(limit > 0)
	//		{
	//			if(!isLeaf())
	//			{
	//				unsigned nret = 0;
	//				BVHNodeList::constant_iterator child_iter = childs.begin();
	//				for(; child_iter != childs.end(); child_iter++)
	//				{
	//					BVHNodeList::constant_iterator other_child_iter = child_iter + 1;
	//					for(; other_child_iter != childs.end(); other_child_iter++)
	//					{
	//						nret += child_iter->getPotentialContactsWith(*other_child_iter, &contacts[nret], limit - nret);
	//					}
	//				}
	//				return nret;
	//			}
	//		}
	//		return 0;
	//	}

	//	void insert(RigidBody * _body, const BoundingVolumeClass & _volume)
	//	{
	//		if(isLeaf())
	//		{
	//			childs.push_back(BVHNode<BoundingVolumeClass>(volume, body));

	//			childs.push_back(BVHNode<BoundingVolumeClass>(_volume, _body));

	//			body = NULL;
	//		}
	//		else
	//		{
	//			BVHNodeList::iterator child_iter = std::min_element(childs.begin(), childs.end(), BoundingVolumeLt(_volume));

	//			child_iter->insert(_body, _volume);
	//		}

	//		recalculateBoundingVolume();
	//	}
	//};

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionPrimitive
	// /////////////////////////////////////////////////////////////////////////////////////

	class IntersectionTests;

	class CollisionDetector;

	class CollisionSphere;

	class CollisionBox;

	class CollisionPrimitive
	{
		friend IntersectionTests;

		friend CollisionDetector;

	protected:
		RigidBody * body;

		Matrix4 offset;

		float friction;

		float restitution;

		Matrix4 transform;

	public:
		void setRigidBody(RigidBody * _body)
		{
			body = _body;
		}

		RigidBody * getRigidBody(void) const
		{
			return body;
		}

		void setOffset(const Matrix4 & _offset)
		{
			offset = _offset;
		}

		const Matrix4 & getOffset(void) const
		{
			return offset;
		}

		void setFriction(float _friction)
		{
			friction = _friction;
		}

		float getFriction(void) const
		{
			return friction;
		}

		void setResitution(float _restitution)
		{
			restitution = _restitution;
		}

		float getRestitution(void) const
		{
			return restitution;
		}

		const Matrix4 & getTransform(void) const
		{
			return transform;
		}

		const Vector3 & getTransformAxis(unsigned i) const
		{
			return transform[i].xyz;
		}

		void calculateInternals(void);

	protected:
		CollisionPrimitive(const Matrix4 & _offset, float _friction, float _restitution)
			: body(NULL)
			, offset(_offset)
			, friction(_friction)
			, restitution(_restitution)
		{
		}

	public:
		virtual ~CollisionPrimitive(void)
		{
		}

		enum PrimitiveType
		{
			PrimitiveTypeShere,
			PrimitiveTypeBox,
		};

		virtual PrimitiveType getPrimitiveType(void) const = 0;

		virtual unsigned collide(
			const CollisionPrimitive * rhs,
			Contact * contacts,
			unsigned limits) const = 0;

		virtual unsigned collideSphere(
			const CollisionSphere * sphere,
			Contact * contacts,
			unsigned limits) const = 0;

		virtual unsigned collideBox(
			const CollisionBox * box,
			Contact * contacts,
			unsigned limits) const = 0;

		virtual unsigned collideHalfSpace(
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits) const = 0;
	};

	typedef boost::shared_ptr<CollisionPrimitive> CollisionPrimitivePtr;

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionSphere
	// /////////////////////////////////////////////////////////////////////////////////////

	class CollisionSphere : public CollisionPrimitive
	{
		friend IntersectionTests;

		friend CollisionDetector;

	protected:
		float radius;

	public:
		void setRadius(float _radius)
		{
			radius = _radius;
		}

		float getRadius(void) const
		{
			return radius;
		}

		PrimitiveType getPrimitiveType(void) const
		{
			return PrimitiveTypeShere;
		}

	public:
		CollisionSphere(
			float _radius,
			const Matrix4 & _offset = my::Matrix4::identity,
			float _friction = 0,
			float _restitution = 0);

		virtual unsigned collide(
			const CollisionPrimitive * rhs,
			Contact * contacts,
			unsigned limits) const;

		virtual unsigned collideSphere(
			const CollisionSphere * sphere,
			Contact * contacts,
			unsigned limits) const;

		virtual unsigned collideBox(
			const CollisionBox * box,
			Contact * contacts,
			unsigned limits) const;

		virtual unsigned collideHalfSpace(
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits) const;
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionBox
	// /////////////////////////////////////////////////////////////////////////////////////

	class CollisionBox : public CollisionPrimitive
	{
		friend IntersectionTests;

		friend CollisionDetector;

	protected:
		Vector3 halfSize;

	public:
		void setHalfSize(const Vector3 & _halfSize)
		{
			halfSize = _halfSize;
		}

		const Vector3 & getHalfSize(void) const
		{
			return halfSize;
		}

		PrimitiveType getPrimitiveType(void) const
		{
			return PrimitiveTypeBox;
		}

	public:
		CollisionBox(
			const Vector3 & _halfSize,
			const Matrix4 & _offset = my::Matrix4::identity,
			float _friction = 0,
			float _restitution = 0);

		virtual unsigned collide(
			const CollisionPrimitive * rhs,
			Contact * contacts,
			unsigned limits) const;

		virtual unsigned collideSphere(
			const CollisionSphere * sphere,
			Contact * contacts,
			unsigned limits) const;

		virtual unsigned collideBox(
			const CollisionBox * box,
			Contact * contacts,
			unsigned limits) const;

		virtual unsigned collideHalfSpace(
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits) const;
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// IntersectionTests
	// /////////////////////////////////////////////////////////////////////////////////////

	class IntersectionTests
	{
	public:
		static bool sphereAndHalfSpace(
			const CollisionSphere & sphere,
			const Plane & plane);

		static bool sphereAndSphere(
			const CollisionSphere & sphere0,
			const CollisionSphere & sphere1);

		static float calculateBoxAxisHalfProjection(const CollisionBox & box, const Vector3 & axis);

		static bool boxAndHalfSpace(
			const CollisionBox & box,
			const Plane & plane);

		static bool _overlapOnAxis(
			const CollisionBox & box0,
			const CollisionBox & box1,
			const Vector3 & axis,
			const Vector3 & toCentre);

		static bool _zeroAxisOrOverlapOnAxis(
			const CollisionBox & box0,
			const CollisionBox & box1,
			const Vector3 & axis,
			const Vector3 & toCentre);

		static bool boxAndBox(
			const CollisionBox & box0,
			const CollisionBox & box1);

		static std::pair<Vector3, Vector3> CalculateRay(const Matrix4 & InverseViewProj, const Vector3 & pos, const Vector2 & pt, const Vector2 & dim);

		typedef std::pair<bool, float> TestResult;

		static TestResult rayAndParallelPlane(const Vector3 & pos, const Vector3 & dir, size_t axis_i, float value);

		static TestResult rayAndAABB(const Vector3 & pos, const Vector3 & dir, const AABB & aabb);

		static TestResult rayAndHalfSpace(
			const Vector3 & pos,
			const Vector3 & dir,
			const Plane & plane);

		// ! the returned second matched un-normalized ray
		static TestResult rayAndSphere(
			const Vector3 & pos,
			const Vector3 & dir,
			const Vector3 & sphereCenter,
			const float sphereRadius);

		static TestResult rayAndCylinder(
			const Vector3 & pos,
			const Vector3 & dir,
			const float cylinderRadius,
			const float cylinderHeight); // Note: Z Axis Cylinder

		static Vector3 calculateTriangleDirection(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);

		static bool isValidTriangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);

		static Vector3 calculateTriangleNormal(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);

		static float isInsideTriangle(const Vector3 & point, const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);

		static TestResult rayAndTriangle(
			const Vector3 & pos,
			const Vector3 & dir,
			float radius,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2);

		enum IntersectionType
		{
			IntersectionTypeOutside,
			IntersectionTypeInside,
			IntersectionTypeIntersect
		};

		static IntersectionType IntersectAABBAndFrustum(const AABB & aabb, const Frustum & frustum);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// VolumnHelper
	// /////////////////////////////////////////////////////////////////////////////////////

	class VolumnHelper
	{
	public:
		static float calculateBoxVolume(float width, float height, float deepth);

		static float calculateBoxMass(float width, float height, float deepth, float density);

		static float calculateSphereVolume(float radius);

		static float calculateSphereMass(float radius, float density);

		static Matrix4 calculateInertiaTensor(float Ixx, float Iyy, float Izz, float Ixy = 0, float Ixz = 0, float Iyz = 0);

		static Matrix4 calculateBoxInertiaTensor(const Vector3 & halfSizes, float mass);

		static Matrix4 calculateSphereInertiaTensor(float radius, float mass);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionDetector
	// /////////////////////////////////////////////////////////////////////////////////////

	class Contact;

	typedef std::vector<Contact> ContactList;

	class CollisionDetector : public IntersectionTests
	{
	public:
		static unsigned sphereAndHalfSpace(
			const CollisionSphere & sphere,
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndTruePlane(
			const CollisionSphere & sphere,
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndPoint(
			const CollisionSphere & sphere,
			const Vector3 & point,
			RigidBody * pointBody,
			float pointFriction,
			float pointRestitution,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndLine(
			const CollisionSphere & sphere,
			const Vector3 & v0,
			const Vector3 & v1,
			RigidBody * lineBody,
			float lineFriction,
			float lineRestitution,
			Contact * contacts,
			unsigned limits);

		static float calculatePointPlaneDistance(
			const Vector3 & point,
			const Vector3 & planePoint,
			const Vector3 & planeNormal);

		static unsigned sphereAndTriangle(
			const CollisionSphere & sphere,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			RigidBody * triangleBody,
			float triangleFriction,
			float triangleResitution,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndSphere(
			const CollisionSphere & sphere0,
			const CollisionSphere & sphere1,
			Contact * contacts,
			unsigned limits);

		static unsigned pointAndHalfSpace(
			const Vector3 & point,
			RigidBody * pointBody,
			float pointFriction,
			float pointRestitution,
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits);

		static unsigned boxAndHalfSpace(
			const CollisionBox & box,
			const Plane & plane,
			float planeFriction,
			float planeRestitution,
			Contact * contacts,
			unsigned limits);

		static unsigned boxAndSphere(
			const CollisionBox & box,
			const CollisionSphere & sphere,
			Contact * contacts,
			unsigned limits);

		static unsigned boxAndPointAways(
			const CollisionBox & box,
			const Vector3 & point,
			RigidBody * pointBody,
			float pointFriction,
			float pointResitution,
			Contact * contacts,
			unsigned limits);

		static float calculateBoxAxisAndBoxPenetration(
			const CollisionBox & box0,
			const Vector3 & axis,
			const CollisionBox & box1);

		static Vector3 findPointFromBoxByDirection(const CollisionBox & box, const Vector3 & dir);

		static bool _tryBoxAxisAndBox(
			const CollisionBox & box0,
			const Vector3 & axis,
			const CollisionBox & box1,
			unsigned index,
			float & smallestPenetration,
			unsigned & smallestIndex);

		static bool _zeroAxisOrTryBoxAxisAndBox(
			const CollisionBox & box0,
			const Vector3 & axis,
			const CollisionBox & box1,
			unsigned index,
			float & smallestPenetration,
			unsigned & smallestIndex);

		static unsigned _detectorBoxAxisAndBoxPoint(
			const CollisionBox & box0,
			const Vector3 & axis,
			const CollisionBox & box1,
			float penetration,
			Contact * contacts,
			unsigned limits);

		static unsigned boxAndBox(
			const CollisionBox & box0,
			const CollisionBox & box1,
			Contact * contacts,
			unsigned limits);

		static bool _tryBoxAxisAndTriangle(
			const CollisionBox & box,
			const Vector3 & axis,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			unsigned index,
			float & smallestPenetration,
			unsigned & smallestIndex);

		static bool _zeroAxisOrTryBoxAxisAndTriangle(
			const CollisionBox & box,
			const Vector3 & axis,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			unsigned index,
			float & smallestPenetration,
			unsigned & smallestIndex);

		static float calculateBoxAxisAndTrianglePenetration(
			const CollisionBox & box,
			const Vector3 & axis,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2);

		static Vector3 findPointFromTriangleByDirection(
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			const Vector3 & dir);

		// ! still can't work properly
		static unsigned boxAndTriangle(
			const CollisionBox & box,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			RigidBody * triangleBody,
			float triangleFriction,
			float triangleRestitution,
			Contact * contacts,
			unsigned limits);
	};
}

#endif // __MYCOLLISION_H__

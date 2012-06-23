
#ifndef __MYCOLLISION_H__
#define __MYCOLLISION_H__

#include "myMath.h"
#include <vector>
#include "myPhysics.h"

namespace my
{
	// /////////////////////////////////////////////////////////////////////////////////////
	// BoundingSphere
	// /////////////////////////////////////////////////////////////////////////////////////

	class BoundingSphere
	{
	protected:
		Vector3 center;

		float radius;

	public:
		const Vector3 & getCenter(void) const;

		void setCenter(const Vector3 & _center);

		const float getRadius(void) const;

		void setRadius(float _radius);

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

	class RigidBody;

	struct PotentialContact
	{
		RigidBody * body0;

		RigidBody * body1;
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// BVHNode
	// /////////////////////////////////////////////////////////////////////////////////////

	template <class BoundingVolumeClass>
	class BVHNode
	{
	protected:
		class BoundingVolumeLt
		{
		protected:
			BoundingVolumeClass volume;

		public:
			BoundingVolumeLt(const BoundingVolumeClass & _volume)
				: volume(_volume)
			{
			}

			bool operator () (const BVHNode<BoundingVolumeClass> & lhs, const BVHNode<BoundingVolumeClass> & rhs)
			{
				return lhs.volume.getGrowth(volume) < rhs.volume.getGrowth(volume);
			}
		};

	protected:
		typedef std::vector<BVHNode<BoundingVolumeClass> > BVHNodeList;

		BVHNodeList childs;

		BoundingVolumeClass volume;

		RigidBody * body;

	protected:
		bool overlaps(const BVHNode<BoundingVolumeClass> & other) const;

		unsigned getPotentialContactsWith(const BVHNode<BoundingVolumeClass> & other, PotentialContact * contacts, unsigned limit) const;

		void recalculateBoundingVolume(void);

	public:
		BVHNode(const BoundingVolumeClass & _volume, RigidBody * _body = NULL);

		bool isLeaf(void) const;

		unsigned getPotentialContacts(PotentialContact * contacts, unsigned limit) const;

		void insert(RigidBody * _body, const BoundingVolumeClass & _volume);
	};

	template <class BoundingVolumeClass>
	bool BVHNode<BoundingVolumeClass>::overlaps(const BVHNode<BoundingVolumeClass> & other) const
	{
		return volume.overlap(other.volume);
	}

	template <class BoundingVolumeClass>
	unsigned BVHNode<BoundingVolumeClass>::getPotentialContactsWith(const BVHNode<BoundingVolumeClass> & other, PotentialContact * contacts, unsigned limit) const
	{
		if(limit > 0)
		{
			if(overlaps(other))
			{
				if(isLeaf())
				{
					if(other.isLeaf())
					{
						contacts[0].body0 = body;
						contacts[0].body1 = other.body;
						return 1;
					}

					return other.getPotentialContactsWith(*this, contants, limit);
				}

				unsigned nret = 0;
				BVHNodeList::constant_iterator child_iter = childs.begin();
				for(; child_iter != childs.end(); child_iter++)
				{
					nret += child_iter->getPotentialContactsWith(other, &contacts[nret], limit - nret);
				}
				return nret;
			}
		}
		return 0;
	}

	template <class BoundingVolumeClass>
	void BVHNode<BoundingVolumeClass>::recalculateBoundingVolume(void)
	{
		_ASSERT(!childs.empty());

		BVHNodeList::const_iterator child_iter = childs.begin();
		volume = child_iter->volume;
		child_iter++;
		for(; child_iter != childs.end(); child_iter++)
		{
			volume = buildBoundingSphere(volume, child_iter->volume);
		}
	}

	template <class BoundingVolumeClass>
	BVHNode<BoundingVolumeClass>::BVHNode(const BoundingVolumeClass & _volume, RigidBody * _body = NULL)
		: volume(_volume)
		, body(_body)
	{
	}

	template <class BoundingVolumeClass>
	bool BVHNode<BoundingVolumeClass>::isLeaf(void) const
	{
		if(NULL == body)
		{
			_ASSERT(childs.empty());

			return true;
		}

		_ASSERT(!childs.empty());

		return false;
	}

	template <class BoundingVolumeClass>
	unsigned BVHNode<BoundingVolumeClass>::getPotentialContacts(PotentialContact * contacts, unsigned limit) const
	{
		if(limit > 0)
		{
			if(!isLeaf())
			{
				unsigned nret = 0;
				BVHNodeList::constant_iterator child_iter = childs.begin();
				for(; child_iter != childs.end(); child_iter++)
				{
					BVHNodeList::constant_iterator other_child_iter = child_iter + 1;
					for(; other_child_iter != childs.end(); other_child_iter++)
					{
						nret += child_iter->getPotentialContactsWith(*other_child_iter, &contacts[nret], limit - nret);
					}
				}
				return nret;
			}
		}
		return 0;
	}

	template <class BoundingVolumeClass>
	void BVHNode<BoundingVolumeClass>::insert(RigidBody * _body, const BoundingVolumeClass & _volume)
	{
		if(isLeaf())
		{
			childs.push_back(BVHNode<BoundingVolumeClass>(volume, body));

			childs.push_back(BVHNode<BoundingVolumeClass>(_volume, _body));

			body = NULL;
		}
		else
		{
			BVHNodeList::iterator child_iter = std::min_element(childs.begin(), childs.end(), BoundingVolumeLt(_volume));

			child_iter->insert(_body, _volume);
		}

		recalculateBoundingVolume();
	}

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionPrimitive
	// /////////////////////////////////////////////////////////////////////////////////////

	class IntersectionTests;

	class CollisionDetector;

	class CollisionPrimitive
	{
		friend IntersectionTests;

		friend CollisionDetector;

	protected:
		RigidBody * body;

		Matrix4 offset;

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

		void setTransform(const Matrix4 & _transform)
		{
			transform = _transform;
		}

		const Matrix4 & getTransform(void) const
		{
			return transform;
		}

		const Vector3 & getTransformAxis(unsigned i) const
		{
			return transform[i];
		}

	public:
		CollisionPrimitive(
			RigidBody * _body,
			const Matrix4 & _offset = Matrix4::Identity());

		virtual ~CollisionPrimitive(void);

	public:
		void calculateInternals(void);
	};

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

	public:
		CollisionSphere(
			float _radius,
			RigidBody * _body,
			const Matrix4 & _offset = Matrix4::Identity());

		CollisionSphere(void);
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

	public:
		CollisionBox(
			const Vector3 & _halfSize,
			RigidBody * _body,
			const Matrix4 & _offset = Matrix4::Identity());

		CollisionBox(void);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionPlane
	// /////////////////////////////////////////////////////////////////////////////////////

	class CollisionPlane //: public CollisionPrimitive
	{
	protected:
		Vector3 normal;

		float distance;

	public:
		void setNormal(const Vector3 _normal)
		{
			_ASSERT(abs(_normal.length() - 1) < EPSILON_E6);

			normal = _normal;
		}

		const Vector3 & getNormal(void) const
		{
			return normal;
		}

		void setDistance(float _distance)
		{
			distance = _distance;
		}

		float getDistance(void) const
		{
			return distance;
		}

	public:
		CollisionPlane(
			const Vector3 & direction,
			float _distance);

		CollisionPlane(void);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// IntersectionTests
	// /////////////////////////////////////////////////////////////////////////////////////

	class IntersectionTests
	{
	public:
		static bool sphereAndHalfSpace(
			const CollisionSphere & sphere,
			const Vector3 & planeNormal,
			float planeDistance);

		static bool sphereAndSphere(
			const CollisionSphere & sphere0,
			const CollisionSphere & sphere1);

		static float calculateBoxAxisHalfProjection(const CollisionBox & box, const Vector3 & axis);

		static bool boxAndHalfSpace(
			const CollisionBox & box,
			const Vector3 & planeNormal,
			float planeDistance);

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

		typedef std::pair<bool, float> TestResult;

		static TestResult rayAndHalfSpace(
			const Vector3 & pos,
			const Vector3 & dir,
			const Vector3 & planeNormal,
			float planeDistance);

		static TestResult rayAndTriangle(
			const Vector3 & pos,
			const Vector3 & dir,
			float radius,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2);
	};

	// /////////////////////////////////////////////////////////////////////////////////////
	// CollisionDetector
	// /////////////////////////////////////////////////////////////////////////////////////

	class Contact;

	typedef std::vector<Contact> ContactList;

	class CollisionDetector
	{
	public:
		static unsigned sphereAndHalfSpace(
			const CollisionSphere & sphere,
			const Vector3 & planeNormal,
			float planeDistance,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndTruePlane(
			const CollisionSphere & sphere,
			const Vector3 & planeNormal,
			float planeDistance,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndPoint(
			const CollisionSphere & sphere,
			const Vector3 & point,
			RigidBody * bodyForPoint,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndLine(
			const CollisionSphere & sphere,
			const Vector3 & v0,
			const Vector3 & v1,
			RigidBody * bodyForLine,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndTriangle(
			const CollisionSphere & sphere,
			const Vector3 & v0,
			const Vector3 & v1,
			const Vector3 & v2,
			RigidBody * bodyForTriangle,
			Contact * contacts,
			unsigned limits);

		static unsigned sphereAndSphere(
			const CollisionSphere & sphere0,
			const CollisionSphere & sphere1,
			Contact * contacts,
			unsigned limits);

		static unsigned pointAndHalfSpace(
			const Vector3 & point,
			RigidBody * body,
			const Vector3 & planeNormal,
			float planeDistance,
			Contact * contacts,
			unsigned limits);

		static unsigned boxAndHalfSpace(
			const CollisionBox & box,
			const Vector3 & planeNormal,
			float planeDistance,
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
			RigidBody * bodyForPoint,
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

		//static unsigned boxAndTriangle(
		//	const CollisionBox & box,
		//	const Vector3 & v0,
		//	const Vector3 & v1,
		//	const Vector3 & v2,
		//	RigidBody * bodyForTriangle,
		//	Contact * contacts,
		//	unsigned limits);
	};
}

#endif // __MYCOLLISION_H__

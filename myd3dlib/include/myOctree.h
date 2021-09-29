#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <list>
#include <array>

namespace my
{
	class OctEntity
	{
	public:
		friend class OctNode;

		OctNode * m_Node;

		const AABB * m_OctAabb;

	public:
		OctEntity(void)
			: m_Node(NULL)
			, m_OctAabb(NULL)
		{
		}

		virtual ~OctEntity(void);
	};

	typedef boost::shared_ptr<OctEntity> OctEntityPtr;

	class OctNode : public AABB
	{
	public:
		struct QueryCallback
		{
		public:
			virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType) = 0;
		};

		OctNode * m_Parent;

		Vector3 m_Half;

		typedef std::list<std::pair<OctEntity *, AABB> > OctEntityMap;

		OctEntityMap m_Entities;

		typedef std::array<boost::shared_ptr<OctNode>, QuadrantNum> ChildArray;

		ChildArray m_Childs;

	public:
		OctNode(void)
			: m_Parent(NULL)
			, m_Half(0, 0, 0)
		{
		}

		OctNode(OctNode * Parent, float minv, float maxv)
			: AABB(minv, maxv)
			, m_Parent(Parent)
			, m_Half(Center())
		{
		}

		OctNode(OctNode * Parent, float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: AABB(minx, miny, minz, maxx, maxy, maxz)
			, m_Parent(Parent)
			, m_Half(Center())
		{
		}

		OctNode(OctNode * Parent, const Vector3 & _Min, const Vector3 & _Max)
			: AABB(_Min, _Max)
			, m_Parent(Parent)
			, m_Half(Center())
		{
		}

		OctNode(OctNode * Parent, const Vector3 & center, float radius)
			: AABB(center, radius)
			, m_Parent(Parent)
			, m_Half(Center())
		{
		}

		virtual ~OctNode(void)
		{
		}

		bool HaveNode(const OctNode * node) const;

		const OctNode * GetTopNode(void) const;

		OctNode * GetTopNode(void);

		virtual void AddEntity(OctEntity * entity, const AABB & aabb, float minblock, float threshold);

		void AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctEntity * entity, const AABB & aabb, float minblock, float threshold);

		void QueryEntity(const Ray & ray, QueryCallback * callback) const;

		void QueryEntity(const AABB & aabb, QueryCallback * callback) const;

		void QueryEntity(const Frustum & frustum, QueryCallback * callback) const;

		void QueryEntityAll(QueryCallback * callback) const;

		virtual void RemoveEntity(OctEntity * entity);

		void ClearAllEntity(void);

		void Flush(void);
	};

	class OctRoot : public OctNode
	{
	protected:
		OctRoot(void)
		{
		}

	public:
		OctRoot(float minv, float maxv)
			: OctNode(NULL, minv, maxv)
		{
		}

		OctRoot(float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: OctNode(NULL, minx, miny, minz, maxx, maxy, maxz)
		{
		}

		OctRoot(const Vector3 & _Min, const Vector3 & _Max)
			: OctNode(NULL, _Min, _Max)
		{
		}

		OctRoot(const Vector3 & center, float radius)
			: OctNode(NULL, center, radius)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AABB);
			ar & BOOST_SERIALIZATION_NVP(m_Half);
		}
	};
}

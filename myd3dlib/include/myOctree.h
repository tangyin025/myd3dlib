#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <vector>
#include <array>

namespace my
{
	class OctEntity
	{
	public:
		friend class OctNode;

		OctNode * m_Node;

	public:
		OctEntity(void)
			: m_Node(NULL)
		{
		}

		virtual ~OctEntity(void);

		const AABB & GetOctAABB(void) const;
	};

	typedef boost::shared_ptr<OctEntity> OctEntityPtr;

	class OctNode
	{
	public:
		struct QueryCallback
		{
		public:
			virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType) = 0;
		};

		static const float THRESHOLD;

		static const float MIN_BLOCK;

		OctNode * m_Parent;

		AABB m_aabb;

		Vector3 m_Half;

		typedef std::map<OctEntity *, AABB> OctEntityMap;

		OctEntityMap m_Entities;

		typedef std::array<boost::shared_ptr<OctNode>, AABB::QuadrantNum> ChildArray;

		ChildArray m_Childs;

	public:
		OctNode(void)
			: m_Parent(NULL)
			, m_aabb(AABB::Invalid())
			, m_Half(0, 0, 0)
		{
		}

		OctNode(OctNode * Parent, const AABB & aabb)
			: m_Parent(Parent)
			, m_aabb(aabb)
			, m_Half(aabb.Center())
		{
		}

		virtual ~OctNode(void)
		{
		}

		bool HaveNode(const OctNode * node) const;

		const OctNode * GetTopNode(void) const;

		OctNode * GetTopNode(void);

		void AddEntity(OctEntity * entity, const AABB & aabb);

		virtual void AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctEntity * entity, const AABB & aabb);

		void QueryEntity(const Ray & ray, QueryCallback * callback) const;

		void QueryEntity(const AABB & aabb, QueryCallback * callback) const;

		void QueryEntity(const Frustum & frustum, QueryCallback * callback) const;

		void QueryEntityAll(QueryCallback * callback) const;

		bool RemoveEntity(OctEntity * entity);

		void ClearAllEntityInCurrentNode(void);

		void ClearAllEntity(void);

		void Flush(void);
	};

	class OctRoot : public OctNode
	{
	protected:
		OctRoot(void)
			: OctNode(NULL, AABB::Invalid())
		{
		}

	public:
		OctRoot(const AABB & aabb)
			: OctNode(NULL, aabb)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_aabb);
			ar & BOOST_SERIALIZATION_NVP(m_Half);
		}
	};
}

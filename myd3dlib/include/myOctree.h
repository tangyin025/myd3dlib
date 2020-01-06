#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <vector>
#include <array>

namespace my
{
	class OctActor : public boost::enable_shared_from_this<OctActor>
	{
	public:
		friend class OctNode;

		OctNode * m_Node;

	public:
		OctActor(void)
			: m_Node(NULL)
		{
		}

		virtual ~OctActor(void)
		{
			_ASSERT(!m_Node);
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
		}

		const AABB & GetOctAABB(void) const;
	};

	typedef boost::shared_ptr<OctActor> OctActorPtr;

	class OctNode
	{
	public:
		struct QueryCallback
		{
		public:
			virtual void OnQueryActor(my::OctActor * oct_actor, const my::AABB & aabb, my::IntersectionTests::IntersectionType) = 0;
		};

		static const float THRESHOLD;

		static const float MIN_BLOCK;

		OctNode * m_Parent;

		AABB m_aabb;

		Vector3 m_Half;

		typedef std::map<OctActorPtr, AABB> OctActorMap;

		OctActorMap m_Actors;

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

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		bool HaveNode(const OctNode * node) const;

		const OctNode * GetTopNode(void) const;

		OctNode * GetTopNode(void);

		void AddActor(OctActorPtr actor, const AABB & aabb);

		virtual void AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctActorPtr actor, const AABB & aabb);

		void QueryActor(const Ray & ray, QueryCallback * callback) const;

		void QueryActor(const AABB & aabb, QueryCallback * callback) const;

		void QueryActor(const Frustum & frustum, QueryCallback * callback) const;

		void QueryActorAll(QueryCallback * callback) const;

		bool RemoveActor(OctActorPtr actor);

		void ClearAllActorInCurrentNode(void);

		void ClearAllNode(void);

		void Flush(void);
	};

	typedef boost::shared_ptr<OctNode> OctNodePtr;

	class OctRoot : public OctNode
	{
	public:
		OctRoot(void)
			: OctNode(NULL, AABB::Invalid())
		{
		}

		OctRoot(const AABB & aabb)
			: OctNode(NULL, aabb)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive & ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}
	};
}

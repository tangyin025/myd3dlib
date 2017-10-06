#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace my
{
	class OctActor : public boost::enable_shared_from_this<OctActor>
	{
	public:
		friend class OctNodeBase;

		template <DWORD Offset> friend class OctNode;

		OctNodeBase * m_Node;

	public:
		OctActor(void)
			: m_Node(NULL)
		{
		}

		virtual ~OctActor(void)
		{
			//_ASSERT(!m_Node);
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
		}
	};

	typedef boost::shared_ptr<OctActor> OctActorPtr;

	class OctNodeBase
	{
	public:
		struct QueryCallback
		{
		public:
			virtual void operator() (OctActor * oct_actor, IntersectionTests::IntersectionType) = 0;
		};

		OctNodeBase * m_Parent;

		AABB m_aabb;

		typedef std::map<OctActorPtr, AABB> OctActorMap;

		OctActorMap m_Actors;

		typedef boost::array<boost::shared_ptr<OctNodeBase>, 2> ChildArray;

		ChildArray m_Childs;

	protected:
		OctNodeBase(OctNodeBase * Parent, const AABB & aabb)
			: m_Parent(Parent)
			, m_aabb(aabb)
		{
		}

		OctNodeBase(void)
			: m_Parent(NULL)
			, m_aabb(AABB::Invalid())
		{
		}

	public:
		virtual ~OctNodeBase(void)
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

		bool HaveNode(const OctNodeBase * node) const;

		const OctNodeBase * GetTopNode(void) const;

		OctNodeBase * GetTopNode(void);

		virtual void AddActor(OctActorPtr actor, const AABB & aabb, float threshold = 0.1f) = 0;

		void QueryActor(const Ray & ray, QueryCallback * callback);

		void QueryActor(const AABB & aabb, QueryCallback * callback);

		void QueryActor(const Frustum & frustum, QueryCallback * callback);

		void QueryActorAll(QueryCallback * callback);

		void QueryActorIntersected(const AABB & aabb, QueryCallback * callback);

		void QueryActorIntersected(const Frustum & frustum, QueryCallback * callback);

		bool RemoveActor(OctActorPtr actor);

		void ClearAllActor(void);

		void Flush(void);
	};

	typedef boost::shared_ptr<OctNodeBase> OctNodeBasePtr;

	template <DWORD Offset>
	class OctNode : public OctNodeBase
	{
	public:
		typedef OctNode<(Offset + 1) % 3> ChildOctNode;

		template <DWORD> friend class OctNode;

		float m_Half;

		float m_MinBlock;

	protected:
		OctNode(void)
			: m_Half(0)
			, m_MinBlock(0)
		{
		}

	public:
		OctNode(OctNodeBase * Parent, const AABB & aabb, float MinBlock)
			: OctNodeBase(Parent, aabb)
			, m_Half((aabb.m_min[Offset] + aabb.m_max[Offset]) * 0.5f)
			, m_MinBlock(MinBlock)
		{
		}

		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctNodeBase);
			ar & BOOST_SERIALIZATION_NVP(m_Half);
			ar & BOOST_SERIALIZATION_NVP(m_MinBlock);
		}

		virtual void AddActor(OctActorPtr actor, const AABB & aabb, float threshold = 0.1f)
		{
			_ASSERT(!actor->m_Node);
			if (aabb.m_max[Offset] < m_Half + threshold && m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 _Max = m_aabb.m_max;
					_Max[Offset] = m_Half;
					m_Childs[0].reset(new ChildOctNode(this, AABB(m_aabb.m_min, _Max), m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[0])->AddActor(actor, aabb, threshold);
			}
			else if (aabb.m_min[Offset] > m_Half - threshold &&  m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 _Min = m_aabb.m_min;
					_Min[Offset] = m_Half;
					m_Childs[1].reset(new ChildOctNode(this, AABB(_Min, m_aabb.m_max), m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[1])->AddActor(actor, aabb, threshold);
			}
			else
			{
				m_Actors.insert(std::make_pair(actor, aabb));
				actor->m_Node = this;
			}
		}
	};
}

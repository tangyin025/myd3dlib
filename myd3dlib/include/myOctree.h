#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <vector>

namespace my
{
	class OctComponent : public boost::enable_shared_from_this<OctComponent>
	{
	public:
		friend class OctNodeBase;

		template <DWORD Offset> friend class OctNode;

		AABB m_aabb;

		OctNodeBase * m_OctNode;

	public:
		OctComponent(const my::AABB & aabb)
			: m_aabb(aabb)
			, m_OctNode(NULL)
		{
		}

		OctComponent(void)
			: m_aabb(AABB::Invalid())
			, m_OctNode(NULL)
		{
		}

		virtual ~OctComponent(void)
		{
			_ASSERT(!m_OctNode);
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(m_aabb);
		}
	};

	typedef boost::shared_ptr<OctComponent> OctComponentPtr;

	struct IQueryCallback
	{
	public:
		virtual void operator() (OctComponent * cmp, IntersectionTests::IntersectionType) = 0;
	};

	class OctNodeBase
	{
	public:
		AABB m_aabb;

		typedef std::vector<OctComponentPtr> OctComponentList;

		OctComponentList m_Components;

		typedef boost::array<boost::shared_ptr<OctNodeBase>, 2> ChildArray;

		ChildArray m_Childs;

	public:
		OctNodeBase(float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: m_aabb(minx, miny, minz, maxx, maxy, maxz)
		{
		}

		OctNodeBase(const Vector3 & _Min, const Vector3 & _Max)
			: m_aabb(_Min, _Max)
		{
		}

		OctNodeBase(const AABB & aabb)
			: m_aabb(aabb)
		{
		}

		OctNodeBase(void)
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

		void QueryComponent(const Ray & ray, IQueryCallback * callback);

		void QueryComponent(const AABB & aabb, IQueryCallback * callback);

		void QueryComponent(const Frustum & frustum, IQueryCallback * callback);

		void QueryComponentAll(IQueryCallback * callback);

		void QueryComponentIntersected(const AABB & aabb, IQueryCallback * callback);

		void QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback);

		bool RemoveComponent(OctComponentPtr cmp);

		void ClearAllComponents(void);
	};

	template <DWORD Offset>
	class OctNode : public OctNodeBase
	{
	public:
		typedef OctNode<(Offset + 1) % 3> ChildOctNode;

		float m_Half;

		float m_MinBlock;

	public:
		OctNode(float minx, float miny, float minz, float maxx, float maxy, float maxz, float MinBlock)
			: OctNodeBase(minx, miny, minz, maxx, maxy, maxz)
			, m_Half((Vector3(minx, miny, minz)[Offset] + Vector3(maxx, maxy, maxz)[Offset]) * 0.5f)
			, m_MinBlock(MinBlock)
		{
		}

		OctNode(const Vector3 & _Min, const Vector3 & _Max, float MinBlock)
			: OctNodeBase(_Min, _Max)
			, m_Half((_Min[Offset] + _Max[Offset]) * 0.5f)
			, m_MinBlock(MinBlock)
		{
		}

		OctNode(const AABB & aabb, float MinBlock)
			: OctNodeBase(aabb)
			, m_Half((aabb.m_min[Offset] + aabb.m_max[Offset]) * 0.5f)
			, m_MinBlock(MinBlock)
		{
		}

		OctNode(void)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctNodeBase);
			ar & BOOST_SERIALIZATION_NVP(m_Half);
			ar & BOOST_SERIALIZATION_NVP(m_MinBlock);
		}

		void AddComponent(OctComponentPtr cmp, const AABB & aabb, float threshold = 0.1f)
		{
			_ASSERT(!cmp->m_OctNode);
			if (aabb.m_max[Offset] < m_Half + threshold && m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 _Max = m_aabb.m_max;
					_Max[Offset] = m_Half;
					m_Childs[0].reset(new ChildOctNode(m_aabb.m_min, _Max, m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[0])->AddComponent(cmp, aabb, threshold);
			}
			else if (aabb.m_min[Offset] > m_Half - threshold &&  m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 _Min = m_aabb.m_min;
					_Min[Offset] = m_Half;
					m_Childs[1].reset(new ChildOctNode(_Min, m_aabb.m_max, m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[1])->AddComponent(cmp, aabb, threshold);
			}
			else
			{
				m_Components.push_back(cmp);
				cmp->m_OctNode = this;
			}
		}
	};

	class OctTree : public OctNode<0>
	{
	public:
		OctTree(float minx, float miny, float minz, float maxx, float maxy, float maxz, float MinBlock)
			: OctNode(minx, miny, minz, maxx, maxy, maxz, MinBlock)
		{
		}

		OctTree(const Vector3 & _Min, const Vector3 & _Max, float MinBlock)
			: OctNode(_Min, _Max, MinBlock)
		{
		}

		OctTree(const AABB & aabb, float MinBlock)
			: OctNode(aabb, MinBlock)
		{
		}

		OctTree(void)
		{
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("OctNode0", boost::serialization::base_object< OctNode<0> >(*this));
		}
	};

	typedef boost::shared_ptr<OctTree> OctRootPtr;
}

#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace my
{
	class OctComponent
	{
	protected:
		friend class OctNodeBase;

		template <DWORD Offset> friend class OctNode;

		OctNodeBase * m_OctNode;

	public:
		AABB m_aabb;

	public:
		OctComponent(const AABB & aabb)
			: m_OctNode(NULL)
			, m_aabb(aabb)
		{
		}

		virtual ~OctComponent(void)
		{
			_ASSERT(!m_OctNode);
		}
	};

	struct IQueryCallback
	{
	public:
		virtual void operator() (OctComponent * cmp, IntersectionTests::IntersectionType) = 0;
	};

	class OctNodeBase
	{
	public:
		const AABB m_aabb;

		typedef boost::unordered_set<OctComponent *> OctComponentSet;

		OctComponentSet m_Components;

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

		void QueryComponent(const Ray & ray, IQueryCallback * callback);

		void QueryComponent(const AABB & aabb, IQueryCallback * callback);

		void QueryComponent(const Frustum & frustum, IQueryCallback * callback);

		void QueryComponentAll(IQueryCallback * callback);

		void QueryComponentIntersected(const AABB & aabb, IQueryCallback * callback);

		void QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback);

		bool RemoveComponent(OctComponent * cmp);

		void ClearAllComponents(void);
	};

	template <DWORD Offset>
	class OctNode : public OctNodeBase
	{
	public:
		typedef OctNode<(Offset + 1) % 3> ChildOctNode;

		const float m_Half;

		const float m_MinBlock;

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

		void AddComponent(OctComponent * cmp, float threshold = 0.1f)
		{
			_ASSERT(!cmp->m_OctNode);
			if (cmp->m_aabb.m_max[Offset] < m_Half + threshold && m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 _Max = m_aabb.m_max;
					_Max[Offset] = m_Half;
					m_Childs[0].reset(new ChildOctNode(m_aabb.m_min, _Max, m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[0])->AddComponent(cmp, threshold);
			}
			else if (cmp->m_aabb.m_min[Offset] > m_Half - threshold &&  m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 _Min = m_aabb.m_min;
					_Min[Offset] = m_Half;
					m_Childs[1].reset(new ChildOctNode(_Min, m_aabb.m_max, m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[1])->AddComponent(cmp, threshold);
			}
			else
			{
				m_Components.insert(cmp);
				cmp->m_OctNode = this;
			}
		}
	};

	class OctRoot : public OctNode<0>
	{
	public:
		OctRoot(float minx, float miny, float minz, float maxx, float maxy, float maxz, float MinBlock)
			: OctNode(minx, miny, minz, maxx, maxy, maxz, MinBlock)
		{
		}

		OctRoot(const Vector3 & _Min, const Vector3 & _Max, float MinBlock)
			: OctNode(_Min, _Max, MinBlock)
		{
		}

		OctRoot(const AABB & aabb, float MinBlock)
			: OctNode(aabb, MinBlock)
		{
		}
	};

	typedef boost::shared_ptr<OctRoot> OctRootPtr;
}

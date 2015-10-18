#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace my
{
	class AABBComponent
	{
	public:
		AABBComponent(void)
		{
		}

		virtual ~AABBComponent(void)
		{
		}
	};

	typedef boost::shared_ptr<AABBComponent> AABBComponentPtr;

	typedef std::pair<AABB, AABBComponentPtr> AABBComponentPtrPair;

	struct IQueryCallback
	{
	public:
		virtual void operator() (const AABBComponentPtr & cmp, IntersectionTests::IntersectionType) = 0;
	};

	class OctNodeBase
	{
	public:
		const AABB m_aabb;

		typedef std::vector<AABBComponentPtrPair> AABBComponentPtrPairList;

		AABBComponentPtrPairList m_ComponentList;

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

		void QueryComponent(const Frustum & frustum, IQueryCallback * callback);

		void QueryComponentAll(IQueryCallback * callback);

		void QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback);

		bool HaveChildNodes(void);

		bool RemoveComponent(AABBComponentPtr cmp);
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

		void AddComponent(AABBComponentPtr cmp, const AABB & cmp_aabb, float threshold = 0.1f)
		{
			if (cmp_aabb.m_max[Offset] < m_Half + threshold && m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 _Max = m_aabb.m_max;
					_Max[Offset] = m_Half;
					m_Childs[0].reset(new ChildOctNode(m_aabb.m_min, _Max, m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[0])->AddComponent(cmp, cmp_aabb, threshold);
			}
			else if (cmp_aabb.m_min[Offset] > m_Half - threshold &&  m_aabb.m_max[Offset] - m_aabb.m_min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 _Min = m_aabb.m_min;
					_Min[Offset] = m_Half;
					m_Childs[1].reset(new ChildOctNode(_Min, m_aabb.m_max, m_MinBlock));
				}
				boost::static_pointer_cast<ChildOctNode>(m_Childs[1])->AddComponent(cmp, cmp_aabb, threshold);
			}
			else
			{
				m_ComponentList.push_back(std::make_pair(cmp_aabb, cmp));
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

		void ClearComponents(void);
	};

	typedef boost::shared_ptr<OctRoot> OctRootPtr;
}

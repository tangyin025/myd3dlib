#pragma once

#include "myCollision.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace my
{
	class AABBComponent : public AABB
	{
	public:
		AABBComponent(float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: AABB(minx, miny, minz, maxx, maxy, maxz)
		{
		}

		AABBComponent(const Vector3 & _Min, const Vector3 & _Max)
			: AABB(_Min, _Max)
		{
		}

		AABBComponent(const AABB & aabb)
			: AABB(aabb)
		{
		}

		virtual ~AABBComponent(void)
		{
		}
	};

	typedef boost::function<void (AABBComponent *, IntersectionTests::IntersectionType)> QueryCallback;

	template <class ChildClass>
	class OctNodeBase : public AABBComponent
	{
	public:
		typedef std::vector<AABBComponent *> AABBComponentList;

		AABBComponentList m_ComponentList;

		typedef boost::array<boost::shared_ptr<ChildClass>, 2> ChildArray;

		ChildArray m_Childs;

	public:
		OctNodeBase(float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: AABBComponent(minx, miny, minz, maxx, maxy, maxz)
		{
		}

		OctNodeBase(const Vector3 & _Min, const Vector3 & _Max)
			: AABBComponent(_Min, _Max)
		{
		}

		OctNodeBase(const AABB & aabb)
			: AABBComponent(aabb)
		{
		}

		void QueryComponent(const Frustum & frustum, const QueryCallback & callback)
		{
			switch(IntersectionTests::IntersectAABBAndFrustum(*this, frustum))
			{
			case IntersectionTests::IntersectionTypeInside:
				QueryComponentAll(frustum, callback);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				QueryComponentIntersected(frustum, callback);
				break;
			}
		}

		void QueryComponentAll(const Frustum & frustum, const QueryCallback & callback)
		{
			AABBComponentList::iterator comp_iter = m_ComponentList.begin();
			for(; comp_iter != m_ComponentList.end(); comp_iter++)
			{
				callback(*comp_iter, IntersectionTests::IntersectionTypeInside);
			}

			ChildArray::iterator node_iter = m_Childs.begin();
			for(; node_iter != m_Childs.end(); node_iter++)
			{
				if (*node_iter)
				{
					(*node_iter)->QueryComponentAll(frustum, callback);
				}
			}
		}

		void QueryComponentIntersected(const Frustum & frustum, const QueryCallback & callback)
		{
			AABBComponentList::iterator comp_iter = m_ComponentList.begin();
			for(; comp_iter != m_ComponentList.end(); comp_iter++)
			{
				IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(*(*comp_iter), frustum);
				switch(intersect_type)
				{
				case IntersectionTests::IntersectionTypeInside:
				case IntersectionTests::IntersectionTypeIntersect:
					callback(*comp_iter, intersect_type);
					break;
				}
			}

			ChildArray::iterator node_iter = m_Childs.begin();
			for(; node_iter != m_Childs.end(); node_iter++)
			{
				if (*node_iter)
				{
					(*node_iter)->QueryComponent(frustum, callback);
				}
			}
		}

		bool HaveChildNodes(void)
		{
			for (unsigned int i = 0; i < m_Childs.size(); i++)
			{
				if (m_Childs[i])
				{
					return true;
				}
			}
			return false;
		}

		bool RemoveComponent(AABBComponent * comp)
		{
			AABBComponentList::iterator comp_iter = std::find(m_ComponentList.begin(), m_ComponentList.end(), comp);
			if (comp_iter != m_ComponentList.end())
			{
				m_ComponentList.erase(comp_iter);
				return true;
			}
			else
			{
				for (unsigned int i = 0; i < m_Childs.size(); i++)
				{
					if (m_Childs[i] && m_Childs[i]->RemoveComponent(comp))
					{
						if (!m_Childs[i]->HaveChildNodes() && m_Childs[i]->m_ComponentList.empty())
						{
							m_Childs[i].reset();
						}
						return true;
					}
				}
			}
			return false;
		}
	};

	template <DWORD Offset>
	class OctNode : public OctNodeBase<OctNode<(Offset + 1) % 3> >
	{
	public:
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
			, m_Half((aabb.Min[Offset] + aabb.Max[Offset]) * 0.5f)
			, m_MinBlock(MinBlock)
		{
		}

		void PushComponent(AABBComponent * comp, float threshold = 0.1f)
		{
			if (comp->Max[Offset] < m_Half + threshold && Max[Offset] - Min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 _Max = Max;
					_Max[Offset] = m_Half;
					m_Childs[0].reset(new OctNode<(Offset + 1) % 3>(Min, _Max, m_MinBlock));
				}
				m_Childs[0]->PushComponent(comp, threshold);
			}
			else if (comp->Min[Offset] > m_Half - threshold &&  Max[Offset] - Min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 _Min = Min;
					_Min[Offset] = m_Half;
					m_Childs[1].reset(new OctNode<(Offset + 1) % 3>(_Min, Max, m_MinBlock));
				}
				m_Childs[1]->PushComponent(comp, threshold);
			}
			else
			{
				m_ComponentList.push_back(comp);
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

		void ClearComponent(void);
	};

	typedef boost::shared_ptr<OctRoot> OctRootPtr;
}

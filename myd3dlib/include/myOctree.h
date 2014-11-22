#pragma once

#include "myMath.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace my
{
	class AABBNode
	{
	public:
		const AABB m_aabb;

	public:
		AABBNode(const AABB & aabb)
			: m_aabb(aabb)
		{
		}

		virtual ~AABBNode(void)
		{
		}
	};

	typedef boost::shared_ptr<AABBNode> AABBNodePtr;

	typedef boost::function<void (AABBNode *)> QueryCallback;

	template <class ChildClass>
	class OctNodeBase : public AABBNode
	{
	public:
		typedef std::vector<AABBNodePtr> ComponentPtrList;

		ComponentPtrList m_ComponentList;

		typedef boost::array<boost::shared_ptr<ChildClass>, 2> ChildArray;

		ChildArray m_Childs;

	public:
		OctNodeBase(const AABB & aabb)
			: AABBNode(aabb)
		{
		}

		void QueryComponent(const Frustum & frustum, const QueryCallback & callback)
		{
			switch(IntersectionTests::IntersectAABBAndFrustum(m_aabb, frustum))
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
			ComponentPtrList::iterator comp_iter = m_ComponentList.begin();
			for(; comp_iter != m_ComponentList.end(); comp_iter++)
			{
				callback(comp_iter->get());
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
			ComponentPtrList::iterator comp_iter = m_ComponentList.begin();
			for(; comp_iter != m_ComponentList.end(); comp_iter++)
			{
				switch(IntersectionTests::IntersectAABBAndFrustum((*comp_iter)->m_aabb, frustum))
				{
				case IntersectionTests::IntersectionTypeInside:
				case IntersectionTests::IntersectionTypeIntersect:
					callback(comp_iter->get());
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

		bool RemoveComponent(AABBNodePtr comp)
		{
			ComponentPtrList::iterator comp_iter = std::find(m_ComponentList.begin(), m_ComponentList.end(), comp);
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
		OctNode(const AABB & aabb, float MinBlock)
			: OctNodeBase(aabb)
			, m_Half((aabb.Min[Offset] + aabb.Max[Offset]) * 0.5f)
			, m_MinBlock(MinBlock)
		{
		}

		void PushComponent(AABBNodePtr comp, float threshold = 0.1f)
		{
			if (comp->m_aabb.Max[Offset] < m_Half + threshold && m_aabb.Max[Offset] - m_aabb.Min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 Max = m_aabb.Max;
					Max[Offset] = m_Half;
					m_Childs[0].reset(new OctNode<(Offset + 1) % 3>(AABB(m_aabb.Min, Max), m_MinBlock));
				}
				m_Childs[0]->PushComponent(comp, threshold);
			}
			else if (comp->m_aabb.Min[Offset] > m_Half - threshold &&  m_aabb.Max[Offset] - m_aabb.Min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 Min = m_aabb.Min;
					Min[Offset] = m_Half;
					m_Childs[1].reset(new OctNode<(Offset + 1) % 3>(AABB(Min, m_aabb.Max), m_MinBlock));
				}
				m_Childs[1]->PushComponent(comp, threshold);
			}
			else
			{
				m_ComponentList.push_back(comp);
			}
		}
	};

	typedef OctNode<0> OctRoot;

	typedef boost::shared_ptr<OctRoot> OctRootPtr;
}

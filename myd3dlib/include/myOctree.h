#pragma once

#include "myMath.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/function.hpp>
#include <vector>

namespace my
{
	class Component
	{
	public:
		AABB m_aabb;

	public:
		Component(const AABB & aabb)
			: m_aabb(aabb)
		{
		}

		virtual ~Component(void)
		{
		}
	};

	typedef boost::shared_ptr<Component> ComponentPtr;

	typedef boost::function<void (Component *)> QueryCallback;

	template <class ChildClass>
	class OctreeNodeBase
	{
	public:
		const AABB m_aabb;

		typedef std::vector<ComponentPtr> ComponentPtrList;

		ComponentPtrList m_ComponentList;

		typedef boost::array<boost::shared_ptr<ChildClass>, 2> ChildArray;

		ChildArray m_Childs;

	public:
		OctreeNodeBase(const AABB & aabb)
			: m_aabb(aabb)
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

		bool RemoveComponent(ComponentPtr comp)
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

	class OctreeNodeY;

	class OctreeNodeZ;

	class OctreeNodeX : public OctreeNodeBase<OctreeNodeY>
	{
	public:
		const float m_X;

	public:
		OctreeNodeX(const AABB & aabb)
			: OctreeNodeBase(aabb)
			, m_X((aabb.Min.x + aabb.Max.x) * 0.5f)
		{
		}

		void PushComponent(ComponentPtr comp, float threshold = 1.0f);
	};

	class OctreeNodeY : public OctreeNodeBase<OctreeNodeZ>
	{
	public:
		const float m_Y;

	public:
		OctreeNodeY(const AABB & aabb)
			: OctreeNodeBase(aabb)
			, m_Y((aabb.Min.y + aabb.Max.y) * 0.5f)
		{
		}

		void PushComponent(ComponentPtr comp, float threshold = 1.0f);
	};

	class OctreeNodeZ : public OctreeNodeBase<OctreeNodeX>
	{
	public:
		const float m_Z;

	public:
		OctreeNodeZ(const AABB & aabb)
			: OctreeNodeBase(aabb)
			, m_Z((aabb.Min.z + aabb.Max.z) * 0.5f)
		{
		}

		void PushComponent(ComponentPtr comp, float threshold = 1.0f);
	};

	typedef OctreeNodeX OctreeRoot;

	typedef boost::shared_ptr<OctreeRoot> OctreeRootPtr;
}

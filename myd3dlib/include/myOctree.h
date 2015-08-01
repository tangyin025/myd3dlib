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
		AABB m_aabb;

	public:
		AABBComponent(float minx, float miny, float minz, float maxx, float maxy, float maxz)
			: m_aabb(minx, miny, minz, maxx, maxy, maxz)
		{
		}

		AABBComponent(const Vector3 & _Min, const Vector3 & _Max)
			: m_aabb(_Min, _Max)
		{
		}

		AABBComponent(const AABB & aabb)
			: m_aabb(aabb)
		{
		}

		virtual ~AABBComponent(void)
		{
		}
	};

	typedef boost::shared_ptr<AABBComponent> AABBComponentPtr;

	struct IQueryCallback
	{
	public:
		virtual void operator() (AABBComponent * comp, IntersectionTests::IntersectionType) = 0;
	};

	template <class ChildClass>
	class OctNodeBase : public AABBComponent
	{
	public:
		typedef std::vector<AABBComponentPtr> AABBComponentPtrList;

		AABBComponentPtrList m_ComponentList;

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

		void QueryComponent(const Frustum & frustum, IQueryCallback * callback)
		{
			switch(IntersectionTests::IntersectAABBAndFrustum(m_aabb, frustum))
			{
			case IntersectionTests::IntersectionTypeInside:
				QueryComponentAll(callback);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				QueryComponentIntersected(frustum, callback);
				break;
			}
		}

		void QueryComponentAll(IQueryCallback * callback)
		{
			AABBComponentPtrList::iterator comp_iter = m_ComponentList.begin();
			for(; comp_iter != m_ComponentList.end(); comp_iter++)
			{
				(*callback)(comp_iter->get(), IntersectionTests::IntersectionTypeInside);
			}

			ChildArray::iterator node_iter = m_Childs.begin();
			for(; node_iter != m_Childs.end(); node_iter++)
			{
				if (*node_iter)
				{
					(*node_iter)->QueryComponentAll(callback);
				}
			}
		}

		void QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback)
		{
			AABBComponentPtrList::iterator comp_iter = m_ComponentList.begin();
			for(; comp_iter != m_ComponentList.end(); comp_iter++)
			{
				//IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(*(*comp_iter), frustum);
				//switch(intersect_type)
				//{
				//case IntersectionTests::IntersectionTypeInside:
				//case IntersectionTests::IntersectionTypeIntersect:
				//	(*callback)(comp_iter->get(), intersect_type);
				//	break;
				//}
				(*callback)(comp_iter->get(), IntersectionTests::IntersectionTypeIntersect);
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

		bool RemoveComponent(AABBComponentPtr comp)
		{
			AABBComponentPtrList::iterator comp_iter = std::find(m_ComponentList.begin(), m_ComponentList.end(), comp);
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

		void AddComponent(AABBComponentPtr comp, float threshold = 0.1f)
		{
			if (comp->m_aabb.Max[Offset] < m_Half + threshold && m_aabb.Max[Offset] - m_aabb.Min[Offset] > m_MinBlock)
			{
				if (!m_Childs[0])
				{
					Vector3 _Max = m_aabb.Max;
					_Max[Offset] = m_Half;
					m_Childs[0].reset(new OctNode<(Offset + 1) % 3>(m_aabb.Min, _Max, m_MinBlock));
				}
				m_Childs[0]->AddComponent(comp, threshold);
			}
			else if (comp->m_aabb.Min[Offset] > m_Half - threshold &&  m_aabb.Max[Offset] - m_aabb.Min[Offset] > m_MinBlock)
			{
				if (!m_Childs[1])
				{
					Vector3 _Min = m_aabb.Min;
					_Min[Offset] = m_Half;
					m_Childs[1].reset(new OctNode<(Offset + 1) % 3>(_Min, m_aabb.Max, m_MinBlock));
				}
				m_Childs[1]->AddComponent(comp, threshold);
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

		void ClearComponents(void);
	};

	typedef boost::shared_ptr<OctRoot> OctRootPtr;
}

#pragma once

#include <set>
#include <map>
#include <boost/multi_array.hpp>
#include <atlbase.h>
#include <atltypes.h>

namespace my
{
	template <typename NodeT>
	class AStar
	{
	public:
		std::set<NodeT> open;
		std::set<NodeT> close;
		std::map<NodeT, float> gscore;
		std::map<NodeT, float> fscore;
		std::map<NodeT, NodeT> from;

	public:
		AStar(void)
		{
		}

		void cleanup(void)
		{
			open.clear();
			close.clear();
			gscore.clear();
			fscore.clear();
			from.clear();
		}

		bool find(const NodeT & start, const NodeT & goal)
		{
			cleanup();

			open.insert(start);
			gscore[start] = 0;
			fscore[start] = heuristic_cost_estimate(start, goal);

			while (!open.empty())
			{
				CPoint current = the_node_in_open_having_the_lowest_fScore_value();
				if (current == goal)
				{
					return true;
				}
				open.erase(current);
				close.insert(current);
				std::vector<CPoint> neighbors = get_neighbors(current);
				std::vector<CPoint>::const_iterator neighbor_iter = neighbors.begin();
				for (; neighbor_iter != neighbors.end(); neighbor_iter++)
				{
					if (close.find(*neighbor_iter) != close.end())
					{
						continue;
					}
					if (open.find(*neighbor_iter) == open.end())
					{
						open.insert(*neighbor_iter);
						_ASSERT(gscore.find(*neighbor_iter) == gscore.end());
						gscore[*neighbor_iter] = FLT_MAX;
					}
					float tentative_gscore = gscore[current] + dist_between(current, *neighbor_iter);
					if (tentative_gscore > gscore[*neighbor_iter])
					{
						continue;
					}
					gscore[*neighbor_iter] = tentative_gscore;
					fscore[*neighbor_iter] = gscore[*neighbor_iter] + heuristic_cost_estimate(*neighbor_iter, goal);
					from[*neighbor_iter] = current;
				}
			}
			return false;
		}

		const NodeT & the_node_in_open_having_the_lowest_fScore_value(void)
		{
			float lowest_score = FLT_MAX;
			std::set<NodeT>::const_iterator ret = open.end();
			std::set<NodeT>::const_iterator iter = open.begin();
			for (; iter != open.end(); iter++)
			{
				std::map<NodeT, float>::const_iterator fscore_iter = fscore.find(*iter);
				_ASSERT(fscore_iter != fscore.end());
				if (fscore_iter->second < lowest_score)
				{
					lowest_score = fscore_iter->second;
					ret = iter;
				}
			}
			_ASSERT(ret != open.end());
			return *ret;
		}

		virtual float heuristic_cost_estimate(const NodeT & start, const NodeT & goal) = 0;

		virtual std::vector<NodeT> get_neighbors(const NodeT & pt) = 0;

		virtual float dist_between(const NodeT & start, const NodeT & goal) = 0;
	};

	template <typename T>
	class AStar2D : public AStar<CPoint>
	{
	public:
		boost::multi_array_ref<T, 2> map;
		T obstacle;

	public:
		AStar2D(int height, int pitch, T * pBits, T _obstacle)
			: map(pBits, boost::extents[height][pitch / sizeof(T)])
			, obstacle(_obstacle)
		{
		}

		float heuristic_cost_estimate(const CPoint & start, const CPoint & goal)
		{
			return (float)abs(start.x - goal.x) + abs(start.y - goal.y);
		}

		std::vector<CPoint> get_neighbors(const CPoint & pt)
		{
			std::vector<CPoint> ret;
			int i = std::max<int>(pt.x - 1, 0);
			for (; i <= std::min<int>(pt.x + 1, map.shape()[1] - 1); i++)
			{
				int j = std::max<int>(pt.y - 1, 0);
				for (; j <= std::min<int>(pt.y + 1, map.shape()[0] - 1); j++)
				{
					if (i != pt.x || j != pt.y)
					{
						if (map[j][i] != obstacle)
						{
							ret.push_back(CPoint(i, j));
						}
					}
				}
			}
			return ret;
		}

		float dist_between(const CPoint & start, const CPoint & goal)
		{
			const float Dist[3][3] =
			{
				{ 1.4f, 1.0f, 1.4f },
				{ 1.0f, 1.0f, 1.0f },
				{ 1.4f, 1.0f, 1.4f },
			};
			_ASSERT(abs(goal.x - start.x) < 2);
			_ASSERT(abs(goal.y - start.y) < 2);
			return Dist[goal.y - start.y + 1][goal.x - start.x + 1];
		}
	};
}

inline bool operator < (const CPoint & lhs, const CPoint & rhs)
{
	if (lhs.x < rhs.x)
	{
		return true;
	}

	if (lhs.x == rhs.x)
	{
		return lhs.y < rhs.y;
	}

	return false;
}

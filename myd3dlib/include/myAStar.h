#pragma once

#include <set>
#include <map>
#include <boost/multi_array.hpp>
#include <atlbase.h>
#include <atltypes.h>
#include "myMath.h"
#include <boost/align/align_up.hpp>
#include <boost/function.hpp>

namespace my
{
	template <typename NodeT, typename NodePred = std::less<NodeT> >
	class AStar
	{
	public:
		const NodeT start;
		const NodeT goal;
		const size_t limit;
		std::set<NodeT, NodePred> open;
		std::set<NodeT, NodePred> close;
		std::map<NodeT, float, NodePred> gscore;
		std::map<NodeT, float, NodePred> fscore;
		std::map<NodeT, NodeT, NodePred> from;

	public:
		AStar(const NodeT & _start, const NodeT & _goal, size_t _limit)
			: start(_start)
			, goal(_goal)
			, limit(_limit)
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

		bool solve()
		{
			cleanup();

			open.insert(start);
			gscore[start] = 0;
			fscore[start] = heuristic_cost_estimate(start, goal);

			while (!open.empty())
			{
				const NodeT current = the_node_in_open_having_the_lowest_fScore_value();
				if (current == goal)
				{
					return true;
				}
				else if (close.size() > limit)
				{
					return false;
				}
				open.erase(current);
				close.insert(current);
				std::vector<NodeT> neighbors = get_neighbors(current);
				std::vector<NodeT>::const_iterator neighbor_iter = neighbors.begin();
				for (; neighbor_iter != neighbors.end(); neighbor_iter++)
				{
					if (close.find(*neighbor_iter) != close.end())
					{
						continue;
					}
					std::map<NodeT, float, NodePred>::iterator gscore_nei_iter = gscore.find(*neighbor_iter);
					float tentative_gscore = gscore[current] + dist_between(current, *neighbor_iter);
					if (gscore_nei_iter != gscore.end() && tentative_gscore > gscore_nei_iter->second)
					{
						continue;
					}
					if (open.find(*neighbor_iter) == open.end())
					{
						open.insert(*neighbor_iter);
						_ASSERT(gscore.find(*neighbor_iter) == gscore.end());
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

	struct CPointCompare
	{
		bool operator() (const CPoint & lhs, const CPoint & rhs) const
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
	};

	template <typename T>
	class AStar2D : public AStar<CPoint, CPointCompare>
	{
	public:
		boost::multi_array_ref<T, 2> map;
		T obstacle;

	public:
		AStar2D(int height, int pitch, T * pBits, T _obstacle, const CPoint & start, const CPoint & goal, size_t limit)
			: AStar(start, goal, limit)
			, map(pBits, boost::extents[height][pitch / sizeof(T)])
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
			for (int i = Max<int>(pt.y - 1, 0); i < Min<int>(pt.y + 2, map.shape()[0]); i++)
			{
				for (int j = Max<int>(pt.x - 1, 0); j < Min<int>(pt.x + 2, map.shape()[1]); j++)
				{
					if (j != pt.x || i != pt.y)
					{
						if (map[i][j] != obstacle)
						{
							ret.push_back(CPoint(j, i));
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

	template <typename T>
	class BilinearFiltering
	{
	public:
		boost::multi_array_ref<T, 2> pixel;

		int width;

		BilinearFiltering(T* pBits, int pitch, int _width, int height)
			: pixel(pBits, boost::extents[height][pitch / sizeof(T)])
			, width(_width)
		{
		}

		BilinearFiltering(const D3DLOCKED_RECT& lrc, int _width, int height)
			: BilinearFiltering((T*)lrc.pBits, lrc.Pitch, _width, height)
		{
		}

		void SampleFourNeighbors(float u, float v, T& n0, T& n1, T& n2, T& n3, float& uf, float& vf) const
		{
			const float us = u * width - 0.5f;
			const float vs = v * pixel.shape()[0] - 0.5f;
			const int j = (int)us;
			const int i = (int)vs;
			n0 = pixel[(i + 0) % pixel.shape()[0]][(j + 0) % width];
			n1 = pixel[(i + 0) % pixel.shape()[0]][(j + 1) % width];
			n2 = pixel[(i + 1) % pixel.shape()[0]][(j + 0) % width];
			n3 = pixel[(i + 1) % pixel.shape()[0]][(j + 1) % width];
			uf = us - j;
			vf = vs - i;
		}

		const T& Get(int i, int j) const
		{
			return pixel[i][j];
		}

		void Set(int i, int j, const T& value)
		{
			pixel[i][j] = value;
		}

		T Sample(float u, float v) const
		{
			T n[4];
			float uf, vf;
			SampleFourNeighbors(u, v, n[0], n[1], n[2], n[3], uf, vf);
			return Lerp(Lerp(n[0], n[1], uf), Lerp(n[2], n[3], uf), vf);
		}
	};

	template <>
	D3DCOLOR BilinearFiltering<D3DCOLOR>::Sample(float u, float v) const;

	class IndexedBitmap : protected boost::multi_array<unsigned char, 2>
	{
	public:
		IndexedBitmap(int width, int height)
			: multi_array(boost::extents[height][boost::alignment::align_up(width, 4)])
		{
			std::fill_n(origin(), num_elements(), 0);
		}

		int GetWidth(void) const
		{
			return (int)shape()[1];
		}

		int GetHeight(void) const
		{
			return (int)shape()[0];
		}

		unsigned char GetPixel(int i, int j) const
		{
			return operator[](i)[j];
		}

		void SetPixel(int i, int j, unsigned char pixel)
		{
			operator[](i)[j] = pixel;
		}

		void LoadFromFile(const char* path);

		void SaveIndexedBitmap(const char* path, const boost::function<DWORD(unsigned char)> & get_color);
	};
}

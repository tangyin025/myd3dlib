#pragma once

#include <boost/multi_array.hpp>

namespace my
{

	class Vector2Int
	{
	public:
		int x, y;

	public:
		Vector2Int(void)
			//: x(0)
			//, y(0)
		{
		}

		Vector2Int(int _v)
			: x(_v)
			, y(_v)
		{
		}

		Vector2Int(int _x, int _y)
			: x(_x)
			, y(_y)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
		}

	public:
		bool operator ==(const Vector2Int & rhs) const
		{
			return x == rhs.x
				&& y == rhs.y;
		}

		bool operator !=(const Vector2Int & rhs) const
		{
			return !operator ==(rhs);
		}

		bool operator <(const Vector2Int & rhs) const
		{
			if (x < rhs.x)
			{
				return true;
			}

			if (x == rhs.x)
			{
				return y < rhs.y;
			}

			return false;
		}

	public:
		Vector2Int operator - (void) const
		{
			return Vector2Int(-x, -y);
		}

		Vector2Int operator + (const Vector2Int & rhs) const
		{
			return Vector2Int(x + rhs.x, y + rhs.y);
		}

		Vector2Int operator + (int scaler) const
		{
			return Vector2Int(x + scaler, y + scaler);
		}

		Vector2Int operator - (const Vector2Int & rhs) const
		{
			return Vector2Int(x - rhs.x, y - rhs.y);
		}

		Vector2Int operator - (int scaler) const
		{
			return Vector2Int(x - scaler, y - scaler);
		}

		Vector2Int operator * (const Vector2Int & rhs) const
		{
			return Vector2Int(x * rhs.x, y * rhs.y);
		}

		Vector2Int operator * (int scaler) const
		{
			return Vector2Int(x * scaler, y * scaler);
		}

		Vector2Int operator / (const Vector2Int & rhs) const
		{
			return Vector2Int(x / rhs.x, y / rhs.y);
		}

		Vector2Int operator / (int scaler) const
		{
			int invScaler = 1 / scaler;

			return Vector2Int(x * invScaler, y * invScaler);
		}

		Vector2Int & operator += (const Vector2Int & rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		Vector2Int & operator += (int scaler)
		{
			x += scaler;
			y += scaler;
			return *this;
		}

		Vector2Int & operator -= (const Vector2Int & rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		Vector2Int & operator -= (int scaler)
		{
			x -= scaler;
			y -= scaler;
			return *this;
		}

		Vector2Int & operator *= (const Vector2Int & rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}

		Vector2Int & operator *= (int scaler)
		{
			x *= scaler;
			y *= scaler;
			return *this;
		}

		Vector2Int & operator /= (const Vector2Int & rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}

		Vector2Int & operator /= (int scaler)
		{
			int invScaler = 1 / scaler;
			x *= invScaler;
			y *= invScaler;
			return *this;
		}

	public:
		static const Vector2Int zero;

		static const Vector2Int unitX;

		static const Vector2Int unitY;
	};

	template <typename T>
	class AStar
	{
	public:
		std::set<Vector2Int> open;
		std::set<Vector2Int> close;
		std::map<Vector2Int, float> gscore;
		std::map<Vector2Int, float> fscore;
		std::map<Vector2Int, Vector2Int> from;
		boost::multi_array_ref<T, 2> map;
		T obstacle;

	public:
		AStar(int height, int pitch, T * pBits, T _obstacle)
			: map(pBits, boost::extents[height][pitch / sizeof(T)])
			, obstacle(_obstacle)
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

		bool find(const Vector2Int & start, const Vector2Int & goal)
		{
			cleanup();

			open.insert(start);
			gscore[start] = 0;
			fscore[start] = heuristic_cost_estimate(start, goal);

			while (!open.empty())
			{
				Vector2Int current = the_node_in_open_having_the_lowest_fScore_value();
				if (current == goal)
				{
					return true;
				}
				open.erase(current);
				close.insert(current);
				std::vector<Vector2Int> neighbors = get_neighbors(current);
				std::vector<Vector2Int>::const_iterator neighbor_iter = neighbors.begin();
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

		Vector2Int the_node_in_open_having_the_lowest_fScore_value(void)
		{
			float lowest_score = FLT_MAX;
			std::set<Vector2Int>::const_iterator ret = open.end();
			std::set<Vector2Int>::const_iterator iter = open.begin();
			for (; iter != open.end(); iter++)
			{
				std::map<Vector2Int, float>::const_iterator fscore_iter = fscore.find(*iter);
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

		float heuristic_cost_estimate(const Vector2Int & start, const Vector2Int & goal)
		{
			return (float)abs(start.x - goal.x) + abs(start.y - goal.y);
		}

		std::vector<Vector2Int> get_neighbors(const Vector2Int & pt)
		{
			std::vector<Vector2Int> ret;
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
							ret.push_back(Vector2Int(i, j));
						}
					}
				}
			}
			return ret;
		}

		float dist_between(const Vector2Int & start, const Vector2Int & goal)
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

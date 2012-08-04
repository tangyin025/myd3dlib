#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>

namespace my
{
	class SplineNode
	{
	public:
		float x, y, k0, k;

	public:
		SplineNode(float _x, float _y, float _k0, float _k)
			: x(_x), y(_y), k0(_k0), k(_k)
		{
		}

		float Interpolate(const SplineNode & rhs, float s);
	};

	typedef boost::shared_ptr<SplineNode> SplineNodePtr;

	typedef std::vector<SplineNodePtr> SplineNodeList;

	class Spline
	{
	protected:
		SplineNodeList nodes;

	public:
		Spline(void)
		{
		}

		~Spline(void)
		{
		}

		bool InsertNode(SplineNodePtr node, int begin_i, int end_i);

		void AddNode(SplineNodePtr node);

		void AddNode(float x, float y, float k0, float k);

		float _Interpolate(float s, int begin_i, int end_i);

		float Interpolate(float s);
	};
}

#include "mySpline.h"
#include <boost/bind.hpp>
#include <algorithm>

using namespace my;

float SplineNode::Interpolate(const SplineNode & rhs, float s) const
{
	_ASSERT(s >= x && s < rhs.x);

	float t = (s - x) / (rhs.x - x);

	float a = k * (rhs.x - x) - (rhs.y - y);

	float b = -rhs.k0 * (rhs.x - x) + (rhs.y - y);

	return (1 - t) * y + t * rhs.y + t * (1 - t) * (a * (1 - t) + b * t);
}

void Spline::AddNode(float x, float y, float k0, float k)
{
	iterator iter = std::lower_bound(begin(), end(), x,
		boost::bind(std::less<float>(), boost::bind(&SplineNode::x, boost::placeholders::_1), boost::placeholders::_2));
	if (iter != end() && iter->x == x)
	{
		iter->y = y;
		iter->k0 = k0;
		iter->k = k;
	}
	else
	{
		insert(iter, SplineNode(x, y, k0, k));
	}
}

float Spline::Interpolate(float s, float value) const
{
	const_iterator iter = std::upper_bound(begin(), end(), s,
		boost::bind(std::less<float>(), boost::placeholders::_1, boost::bind(&SplineNode::x, boost::placeholders::_2)));
	if (iter != begin())
	{
		if (iter != end())
		{
			return (iter - 1)->Interpolate(*iter, s);
		}
		return (iter - 1)->y;
	}
	else if (iter != end())
	{
		return iter->y;
	}
	return value;
}

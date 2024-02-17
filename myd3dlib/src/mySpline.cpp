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

float Tween::Step(float fElapsedTime)
{
	time += fElapsedTime;
	return From.Interpolate(To, time);
}

float Tween::Duration(void) const
{
	return To.x;
}

Shake::Shake(float Duration, float Strength, int Vibrato, float StartMagnitude)
	: time(0)
{
	float shakeMagnitude = Strength;
	int totIterations = Max(2, (int)(Vibrato * Duration));
	float decay = shakeMagnitude / (totIterations - 1);
	bool neg = StartMagnitude > 0 ? true : false;
	AddNode(0.0f, StartMagnitude, 0.0f, 0.0f);
	for (int i = 0; i < totIterations; i++)
	{
		if (i < totIterations - 1) {
			float iterationPerc = (i + 1) / (float)totIterations;
			AddNode(iterationPerc * Duration, neg ? -shakeMagnitude : shakeMagnitude, 0.0f, 0.0f);
			shakeMagnitude -= decay;
			neg = !neg;
		}
		else
			AddNode(Duration, 0.0f, 0.0f, 0.0f);
	}
}

float Shake::Step(float fElapsedTime)
{
	time += fElapsedTime;
	return Interpolate(time, 0.0f);
}

float Shake::Duration(void) const
{
	if (!empty())
	{
		return back().x;
	}
	return 0.0f;
}

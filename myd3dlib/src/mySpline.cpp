#include "StdAfx.h"
#include "mySpline.h"

using namespace my;

float SplineNode::Interpolate(const SplineNode & rhs, float s)
{
	_ASSERT(s >= x && s < rhs.x);

	float t = (s - x) / (rhs.x - x);

	float a = k * (rhs.x - x) - (rhs.y - y);

	float b = -rhs.k0 * (rhs.x - x) + (rhs.y - y);

	return (1 - t) * y + t * rhs.y + t * (1 - t) * (a * (1 - t) + b * t);
}

bool Spline::InsertNode(SplineNodePtr node, int begin_i, int end_i)
{
	if(begin_i >= end_i)
	{
		insert(begin() + begin_i, node);
		return true;
	}

	int mid_i = (begin_i + end_i) / 2;

	if(node->x < operator [](mid_i)->x)
	{
		return InsertNode(node, begin_i, mid_i);
	}

	if(node->x == operator [](mid_i)->x)
	{
		operator [](mid_i) = node;
		return true;
	}

	return InsertNode(node, mid_i + 1, end_i);
}

void Spline::InsertNode(SplineNodePtr node)
{
	InsertNode(node, 0, size());
}

void Spline::AddNode(float x, float y, float k0, float k)
{
	InsertNode(SplineNodePtr(new SplineNode(x, y, k0, k)));
}

float Spline::Interpolate(float s, int begin_i, int end_i) const
{
	if(begin_i >= end_i)
	{
		if(begin_i >= (int)size())
			return back()->y;

		if(end_i <= 0)
			return front()->y;

		return operator [](begin_i - 1)->Interpolate(*operator [](begin_i), s);
	}

	int mid_i = (begin_i + end_i) / 2;

	if(s < operator [](mid_i)->x)
	{
		return Interpolate(s, begin_i, mid_i);
	}

	if(s == operator [](mid_i)->x)
	{
		return operator [](mid_i)->y;
	}

	return Interpolate(s, mid_i + 1, end_i);
}

float Spline::Interpolate(float s, float value) const
{
	if(!empty())
	{
		return Interpolate(s, 0, size());
	}

	return value;
}

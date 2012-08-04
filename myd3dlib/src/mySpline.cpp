#include "StdAfx.h"
#include "mySpline.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

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
		nodes.insert(nodes.begin() + begin_i, node);
		return true;
	}

	int mid_i = (begin_i + end_i) / 2;

	if(node->x < nodes[mid_i]->x)
	{
		return InsertNode(node, begin_i, mid_i);
	}

	if(node->x == nodes[mid_i]->x)
	{
		nodes[mid_i] = node;
		return true;
	}

	return InsertNode(node, mid_i + 1, end_i);
}

void Spline::AddNode(SplineNodePtr node)
{
	InsertNode(node, 0, nodes.size());
}

void Spline::AddNode(float x, float y, float k0, float k)
{
	AddNode(SplineNodePtr(new SplineNode(x, y, k0, k)));
}

float Spline::_Interpolate(float s, int begin_i, int end_i)
{
	if(begin_i >= end_i)
	{
		if(begin_i >= (int)nodes.size())
			return nodes.back()->y;

		if(end_i <= 0)
			return nodes.front()->y;

		return nodes[begin_i - 1]->Interpolate(*nodes[begin_i], s);
	}

	int mid_i = (begin_i + end_i) / 2;

	if(s < nodes[mid_i]->x)
	{
		return _Interpolate(s, begin_i, mid_i);
	}

	if(s == nodes[mid_i]->x)
	{
		return nodes[mid_i]->y;
	}

	return _Interpolate(s, mid_i + 1, end_i);
}

float Spline::Interpolate(float s)
{
	_ASSERT(!nodes.empty());

	return _Interpolate(s, 0, nodes.size());
}

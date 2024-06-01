// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <vector>
#include "myMath.h"

namespace my
{
	template <typename T>
	class LinearNodes : public std::vector<std::pair<float, T> >
	{
	public:
		LinearNodes(void)
		{
		}

		void AddNode(float x, const T& v)
		{
			iterator iter = std::lower_bound(begin(), end(), x,
				boost::bind(std::less<float>(), boost::bind(&value_type::first, boost::placeholders::_1), boost::placeholders::_2));
			if (iter != end() && iter->first == x)
			{
				iter->second = v;
			}
			else
			{
				insert(iter, std::make_pair(x, v));
			}
		}

		float GetLength(void) const
		{
			return !empty() ? back().first : 0;
		}

		T Interpolate(float s, const T& value) const
		{
			const_iterator iter = std::upper_bound(begin(), end(), s,
				boost::bind(std::less<float>(), boost::placeholders::_1, boost::bind(&value_type::first, boost::placeholders::_2)));
			if (iter != begin())
			{
				if (iter != end())
				{
					return Lerp(iter - 1, iter, s);
				}
				return (iter - 1)->second;
			}
			else if (iter != end())
			{
				return iter->second;
			}
			return value;
		}

		T Lerp(const_iterator lhs, const_iterator rhs, float s) const;
	};

	class SplineNode
	{
	public:
		float y, k0, k;

	public:
		SplineNode(void)
			//: y(0)
			//, k0(0)
			//, k(0)
		{
		}

		SplineNode(float _y, float _k0, float _k)
			: y(_y), k0(_k0), k(_k)
		{
		}
	};

	class Spline : public LinearNodes<SplineNode>
	{
	public:
		Spline(void)
		{
		}

		template<class Archive>
		void save(Archive& ar, const unsigned int version) const;

		template<class Archive>
		void load(Archive& ar, const unsigned int version);

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			boost::serialization::split_member(ar, *this, version);
		}

		void AddNode(float x, float y, float k0, float k);

		float Interpolate(float s, float value) const;
	};

	class Shake : public Spline
	{
	public:
		float time;

		Shake(float Duration, float Strength, int Vibrato, float StartMagnitude);

		float Step(float fElapsedTime);
	};
}

// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <vector>
#include "myMath.h"

namespace my
{
	template <typename T>
	class LinearNode
	{
	public:
		float x, k0, k;

		T y;

	protected:
		LinearNode(void)
			//: x(0)
			//, v(0)
			//, k0(0)
			//, k(0)
		{
		}

		friend class boost::serialization::access;

	public:
		LinearNode(float _x, const T & _y, float _k0, float _k)
			: x(_x), y(_y), k0(_k0), k(_k)
		{
		}
	};

	template <typename T>
	class LinearNodes : public std::vector<LinearNode<T> >
	{
	public:
		LinearNodes(void)
		{
		}

		void AddNode(float x, const T & y, float k0, float k)
		{
			iterator iter = std::lower_bound(begin(), end(), x,
				boost::bind(std::less<float>(), boost::bind(&LinearNode<T>::x, boost::placeholders::_1), boost::placeholders::_2));
			if (iter != end() && iter->x == x)
			{
				iter->y = y;
				iter->k0 = k0;
				iter->k = k;
			}
			else
			{
				insert(iter, LinearNode<T>(x, y, k0, k));
			}
		}

		float GetLength(void) const
		{
			return !empty() ? back().x : 0;
		}

		T Interpolate(float s, const T & value) const
		{
			const_iterator iter = std::upper_bound(begin(), end(), s,
				boost::bind(std::less<float>(), boost::placeholders::_1, boost::bind(&LinearNode<T>::x, boost::placeholders::_2)));
			if (iter != begin())
			{
				if (iter != end())
				{
					return Lerp(iter - 1, iter, s);
				}
				return (iter - 1)->y;
			}
			else if (iter != end())
			{
				return iter->y;
			}
			return value;
		}

		T Lerp(const_iterator lhs, const_iterator rhs, float s) const;
	};

	class Spline : public LinearNodes<float>
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

		float Interpolate(float s) const;
	};

	class Shake : public Spline
	{
	public:
		float time;

		Shake(float Duration, float Strength, int Vibrato, float StartMagnitude);

		float Step(float fElapsedTime);
	};
}

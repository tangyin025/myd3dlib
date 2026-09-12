// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include <vector>
#include "myMath.h"

namespace my
{
	class SplineNode
	{
	public:
		float x, y, k0, k;

	protected:
		SplineNode(void)
			//: x(0)
			//, v(0)
			//, k0(0)
			//, k(0)
		{
		}

		friend class boost::serialization::access;

	public:
		SplineNode(float _x, float _y, float _k0, float _k)
			: x(_x), y(_y), k0(_k0), k(_k)
		{
		}
	};

	class Spline : public std::vector<SplineNode>
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

		float Interpolate(const_iterator lhs, const_iterator rhs, float s) const;

		float Interpolate(float s) const;

		float GetLength(void) const
		{
			return !empty() ? back().x : 0;
		}
	};

	class Shake : public Spline
	{
	public:
		float time;

		Shake(float Duration, float Strength, int Vibrato, float StartMagnitude);

		float Step(float fElapsedTime);
	};
}

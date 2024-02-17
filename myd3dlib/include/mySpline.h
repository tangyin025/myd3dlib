#pragma once

#include <vector>
#include "myMath.h"

namespace my
{
	class SplineNode
	{
	public:
		float x, y, k0, k;

	public:
		SplineNode(void)
			//: x(0)
			//, y(0)
			//, k0(0)
			//, k(0)
		{
		}

		SplineNode(float _x, float _y, float _k0, float _k)
			: x(_x), y(_y), k0(_k0), k(_k)
		{
		}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(x);
			ar & BOOST_SERIALIZATION_NVP(y);
			ar & BOOST_SERIALIZATION_NVP(k0);
			ar & BOOST_SERIALIZATION_NVP(k);
		}

		float Interpolate(const SplineNode & rhs, float s) const;
	};

	class Spline : public std::vector<SplineNode>
	{
	public:
		Spline(void)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("SplineNodeList", boost::serialization::base_object<std::vector<SplineNode> >(*this));
		}

		void AddNode(float x, float y, float k0, float k);

		float Interpolate(float s, float value) const;
	};

	class Tween
	{
	public:
		const SplineNode From, To;

		float time;

		Tween(float _From, float _To, float Duration)
			: From(0.0f, _From, 0.0f, 0.0f)
			, To(Duration, _To, 0.0f, 0.0f)
			, time(0)
		{
		}

		float Step(float fElapsedTime);

		float Duration(void) const;
	};

	class Shake : public Spline
	{
	public:
		float time;

		Shake(float Duration, float Strength, int Vibrato, float StartMagnitude);

		float Step(float fElapsedTime);

		float Duration(void) const;
	};
}

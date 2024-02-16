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

	template <typename T>
	class Tween
	{
	public:
		const T From, To;

		const float Duration;

		float time;

		Tween(const T& _From, const T& _To, float _Duration)
			: From(_From)
			, To(_To)
			, Duration(_Duration)
			, time(0)
		{
		}

		T Step(float fElapsedTime);
	};

	template <>
	float Tween<float>::Step(float fElapsedTime);

	template <>
	Vector2 Tween<Vector2>::Step(float fElapsedTime);

	template <>
	Vector3 Tween<Vector3>::Step(float fElapsedTime);
}

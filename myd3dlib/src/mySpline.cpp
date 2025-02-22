// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "mySpline.h"
#include <boost/bind.hpp>
#include <algorithm>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Spline)

template <>
float LinearNodes<float>::Lerp(LinearNodes<float>::const_iterator lhs, LinearNodes<float>::const_iterator rhs, float s) const
{
	_ASSERT(s >= lhs->x && s < rhs->x);

	float t = (s - lhs->x) / (rhs->x - lhs->x);

	float a = lhs->k * (rhs->x - lhs->x) - (rhs->y - lhs->y);

	float b = -rhs->k0 * (rhs->x - lhs->x) + (rhs->y - lhs->y);

	return (1 - t) * lhs->y + t * rhs->y + t * (1 - t) * (a * (1 - t) + b * t);
}

namespace boost { 
	namespace serialization {
		template<class Archive>
		inline void serialize(Archive & ar, LinearNode<float> & t, const unsigned int file_version)
		{
			ar & BOOST_SERIALIZATION_NVP(t.x);
			ar & BOOST_SERIALIZATION_NVP(t.y);
			ar & BOOST_SERIALIZATION_NVP(t.k0);
			ar & BOOST_SERIALIZATION_NVP(t.k);
		}
	}
}

template<class Archive>
void Spline::save(Archive& ar, const unsigned int version) const
{
	ar << boost::serialization::make_nvp("SplineNodeList", boost::serialization::base_object<std::vector<LinearNode<float> > >(*this));
}

template<class Archive>
void Spline::load(Archive& ar, const unsigned int version)
{
	ar >> boost::serialization::make_nvp("SplineNodeList", boost::serialization::base_object<std::vector<LinearNode<float> > >(*this));
}

void Spline::AddNode(float x, float y, float k0, float k)
{
	LinearNodes::AddNode(x, y, k0, k);
}

float Spline::Interpolate(float s) const
{
	return LinearNodes::Interpolate(s, 1.0f);
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
	return Interpolate(time);
}

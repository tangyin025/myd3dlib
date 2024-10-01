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

namespace boost { 
	namespace serialization {
		template<class Archive>
		inline void serialize(Archive & ar, std::pair<float, SplineNode> & t, const unsigned int file_version)
		{
			ar & BOOST_SERIALIZATION_NVP(t.first);
			ar & BOOST_SERIALIZATION_NVP(t.second.y);
			ar & BOOST_SERIALIZATION_NVP(t.second.k0);
			ar & BOOST_SERIALIZATION_NVP(t.second.k);
		}
	}
}

template<class Archive>
void Spline::save(Archive& ar, const unsigned int version) const
{
	ar << boost::serialization::make_nvp("SplineNodeList", boost::serialization::base_object<std::vector<std::pair<float, SplineNode> > >(*this));
}

template<class Archive>
void Spline::load(Archive& ar, const unsigned int version)
{
	ar >> boost::serialization::make_nvp("SplineNodeList", boost::serialization::base_object<std::vector<std::pair<float, SplineNode> > >(*this));
}

void Spline::AddNode(float x, float y, float k0, float k)
{
	LinearNodes::AddNode(x, SplineNode(y, k0, k));
}

template <>
SplineNode LinearNodes<SplineNode>::Lerp(LinearNodes<SplineNode>::const_iterator lhs, LinearNodes<SplineNode>::const_iterator rhs, float s) const
{
	_ASSERT(s >= lhs->first && s < rhs->first);

	float t = (s - lhs->first) / (rhs->first - lhs->first);

	float a = lhs->second.k * (rhs->first - lhs->first) - (rhs->second.y - lhs->second.y);

	float b = -rhs->second.k0 * (rhs->first - lhs->first) + (rhs->second.y - lhs->second.y);

	return SplineNode((1 - t) * lhs->second.y + t * rhs->second.y + t * (1 - t) * (a * (1 - t) + b * t), 0, 0);
}

float Spline::Interpolate(float s) const
{
	return LinearNodes::Interpolate(s, SplineNode(0, 0, 0)).y;
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

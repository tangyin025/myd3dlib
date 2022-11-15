#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include <boost/serialization/nvp.hpp>

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

		bool InsertNode(const SplineNode & node, int begin_i, int end_i);

		void AddNode(float x, float y, float k0, float k);

		float Interpolate(float s, int begin_i, int end_i) const;

		float Interpolate(float s, float value) const;
	};
}

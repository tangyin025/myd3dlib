#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>

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
			ar & x;
			ar & y;
			ar & k0;
			ar & k;
		}

		float Interpolate(const SplineNode & rhs, float s);
	};

	typedef boost::shared_ptr<SplineNode> SplineNodePtr;

	class Spline : public std::vector<SplineNodePtr>
	{
	public:
		Spline(void)
		{
		}

		template <class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & boost::serialization::base_object<std::vector<SplineNodePtr> >(*this);
		}

		bool InsertNode(SplineNodePtr node, int begin_i, int end_i);

		void InsertNode(SplineNodePtr node);

		void AddNode(float x, float y, float k0, float k);

		float Interpolate(float s, int begin_i, int end_i) const;

		float Interpolate(float s) const;
	};
}

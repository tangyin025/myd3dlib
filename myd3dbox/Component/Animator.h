#pragma once

#include "ResourceBundle.h"

class Animator
{
public:
	ResourceBundle<my::OgreSkeletonAnimation> m_SkeletonRes;

	my::TransformList m_DualQuats;

public:
	Animator(void)
	{
	}

	virtual ~Animator(void)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_SkeletonRes);
	}

	virtual void RequestResource(void)
	{
		m_SkeletonRes.RequestResource();
	}

	virtual void ReleaseResource(void)
	{
		m_SkeletonRes.ReleaseResource();
	}

	virtual void Update(float fElapsedTime)
	{
	}
};

typedef boost::shared_ptr<Animator> AnimatorPtr;

class SimpleAnimator
	: public Animator
{
public:
	float m_Time;

public:
	SimpleAnimator(void)
		: m_Time(0)
	{
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Animator);
	}

	virtual void Update(float fElapsedTime);
};

typedef boost::shared_ptr<SimpleAnimator> SimpleAnimatorPtr;

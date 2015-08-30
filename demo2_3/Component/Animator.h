#pragma once

class Animator
{
public:
	my::OgreSkeletonAnimationPtr m_Skeleton;

	my::TransformList m_DualQuats;

public:
	Animator(void)
	{
	}

	virtual ~Animator(void)
	{
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

	virtual void Update(float fElapsedTime);
};

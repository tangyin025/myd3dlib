#pragma once

class MeshAnimator
{
public:
	my::OgreSkeletonAnimationPtr m_Animation;

	my::TransformList m_DualQuats;

public:
	MeshAnimator(void)
	{
	}

	virtual ~MeshAnimator(void)
	{
	}

	virtual void Update(float fElapsedTime)
	{
	}

	const my::Matrix4 * GetDualQuats(void) const;

	UINT GetDualQuatsNum(void) const;
};

typedef boost::shared_ptr<MeshAnimator> MeshAnimatorPtr;

class SimpleMeshAnimator
	: public MeshAnimator
{
public:
	float m_Time;

public:
	SimpleMeshAnimator(void)
		: m_Time(0)
	{
	}

	virtual void Update(float fElapsedTime);
};

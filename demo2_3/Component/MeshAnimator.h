#pragma once

class MeshAnimator
{
public:
	my::TransformList m_DualQuats;

public:
	MeshAnimator(void)
	{
	}

	virtual ~MeshAnimator(void)
	{
	}

	const my::Matrix4 * GetDualQuats(void) const;

	UINT GetDualQuatsNum(void) const;
};

typedef boost::shared_ptr<MeshAnimator> MeshAnimatorPtr;

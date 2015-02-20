#include "stdafx.h"
#include "MeshAnimator.h"

using namespace my;

const my::Matrix4 * MeshAnimator::GetDualQuats(void) const
{
	return &m_DualQuats[0];
}

UINT MeshAnimator::GetDualQuatsNum(void) const
{
	return m_DualQuats.size();
}

#include "stdafx.h"
#include "Animator.h"

using namespace my;

const my::Matrix4 * Animator::GetDualQuats(void) const
{
	return &m_DualQuats[0];
}

UINT Animator::GetDualQuatsNum(void) const
{
	return m_DualQuats.size();
}

void SimpleAnimator::Update(float fElapsedTime)
{
	OgreSkeletonAnimation::OgreAnimationNameMap::const_iterator anim_iter = m_Animation->m_animationMap.begin();
	if (anim_iter == m_Animation->m_animationMap.end())
	{
		return;
	}

	m_Time = fmod(m_Time + fElapsedTime, m_Animation->GetAnimation(anim_iter->first).GetTime());;

	BoneList animPose(m_Animation->m_boneBindPose.size());
	BoneList bindPoseHier(m_Animation->m_boneBindPose.size());
	BoneList animPoseHier(m_Animation->m_boneBindPose.size());
	my::BoneIndexSet::const_iterator root_iter = m_Animation->m_boneRootSet.begin();
	for (; root_iter != m_Animation->m_boneRootSet.end(); root_iter++)
	{
		m_Animation->BuildAnimationPose(
			animPose, m_Animation->m_boneHierarchy, *root_iter, anim_iter->first, m_Time);

		m_Animation->m_boneBindPose.BuildHierarchyBoneList(
			bindPoseHier, m_Animation->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));

		animPose.BuildHierarchyBoneList(
			animPoseHier, m_Animation->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));
	}
	m_DualQuats.resize(m_Animation->m_boneBindPose.size());
	bindPoseHier.BuildDualQuaternionList(m_DualQuats, animPoseHier);
}

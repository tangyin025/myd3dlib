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

void SimpleMeshAnimator::Update(float fElapsedTime)
{
	OgreSkeletonAnimation::OgreAnimationNameMap::const_iterator anim_iter = m_Animation->m_animationMap.begin();
	if (anim_iter == m_Animation->m_animationMap.end())
	{
		return;
	}

	m_Time = fmod(m_Time + fElapsedTime, m_Animation->GetAnimation(anim_iter->first).GetTime());;

	my::BoneIndexSet::const_iterator root_iter = m_Animation->m_boneRootSet.begin();
	for (; root_iter != m_Animation->m_boneRootSet.end(); root_iter++)
	{
		BoneList animPose(m_Animation->m_boneBindPose.size());
		m_Animation->BuildAnimationPose(
			animPose, m_Animation->m_boneHierarchy, *root_iter, anim_iter->first, m_Time);

		BoneList bindPoseHier(m_Animation->m_boneBindPose.size());
		m_Animation->m_boneBindPose.BuildHierarchyBoneList(
			bindPoseHier, m_Animation->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));

		BoneList animPoseHier(m_Animation->m_boneBindPose.size());
		animPose.BuildHierarchyBoneList(
			animPoseHier, m_Animation->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));

		m_DualQuats.resize(m_Animation->m_boneBindPose.size());
		bindPoseHier.BuildDualQuaternionList(m_DualQuats, animPoseHier);
	}
}

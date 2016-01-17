#include "stdafx.h"
#include "Animator.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

template<>
void ResourceBundle<my::BaseTexture>::RequestResource(void)
{
	my::ResourceMgr::getSingleton().LoadTextureAsync(m_ResPath, this);
}

template<>
void ResourceBundle<my::Mesh>::RequestResource(void)
{
	my::ResourceMgr::getSingleton().LoadMeshAsync(m_ResPath, this);
}

template<>
void ResourceBundle<my::OgreSkeletonAnimation>::RequestResource(void)
{
	my::ResourceMgr::getSingleton().LoadSkeletonAsync(m_ResPath, this);
}

BOOST_CLASS_EXPORT(Animator)

BOOST_CLASS_EXPORT(SimpleAnimator)

void SimpleAnimator::Update(float fElapsedTime)
{
	if (m_SkeletonRes.m_Res)
	{
		OgreSkeletonAnimation::OgreAnimationNameMap::const_iterator anim_iter = m_SkeletonRes.m_Res->m_animationMap.begin();
		if (anim_iter == m_SkeletonRes.m_Res->m_animationMap.end())
		{
			return;
		}

		m_Time = fmod(m_Time + fElapsedTime, m_SkeletonRes.m_Res->GetAnimation(anim_iter->first).GetTime());;

		BoneList animPose(m_SkeletonRes.m_Res->m_boneBindPose.size());
		BoneList bindPoseHier(m_SkeletonRes.m_Res->m_boneBindPose.size());
		BoneList animPoseHier(m_SkeletonRes.m_Res->m_boneBindPose.size());
		my::BoneIndexSet::const_iterator root_iter = m_SkeletonRes.m_Res->m_boneRootSet.begin();
		for (; root_iter != m_SkeletonRes.m_Res->m_boneRootSet.end(); root_iter++)
		{
			m_SkeletonRes.m_Res->BuildAnimationPose(
				animPose, m_SkeletonRes.m_Res->m_boneHierarchy, *root_iter, anim_iter->first, m_Time);

			m_SkeletonRes.m_Res->m_boneBindPose.BuildHierarchyBoneList(
				bindPoseHier, m_SkeletonRes.m_Res->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));

			animPose.BuildHierarchyBoneList(
				animPoseHier, m_SkeletonRes.m_Res->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));
		}
		m_DualQuats.resize(m_SkeletonRes.m_Res->m_boneBindPose.size());
		bindPoseHier.BuildDualQuaternionList(m_DualQuats, animPoseHier);
	}
}

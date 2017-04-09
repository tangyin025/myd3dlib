#include "stdafx.h"
#include "Animator.h"
#include "Character.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(AnimationNode)

AnimationNode::AnimationNode(void)
	: m_Owner(NULL)
{
}

AnimationNode::~AnimationNode(void)
{
}

void AnimationNode::SetOwner(Animator * Owner)
{
	m_Owner = Owner;
}

void AnimationNode::Advance(float duration)
{
}

my::BoneList & AnimationNode::GetPose(my::BoneList & pose) const
{
	return pose;
}

BOOST_CLASS_EXPORT(Animator)

Animator::Animator(void)
	: m_Cmp(NULL)
{
}

Animator::~Animator(void)
{
}

template<>
void Animator::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_SkeletonRes);
	ar << BOOST_SERIALIZATION_NVP(m_Node);
}

template<>
void Animator::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_SkeletonRes);
	ar >> BOOST_SERIALIZATION_NVP(m_Node);
	m_Node->SetOwner(this);
}

void Animator::RequestResource(void)
{
	m_SkeletonRes.RequestResource();
}

void Animator::ReleaseResource(void)
{
	m_SkeletonRes.ReleaseResource();
}

void Animator::Update(float fElapsedTime)
{
	if (m_SkeletonRes.m_Res)
	{
		m_Node->Advance(fElapsedTime);
		BoneList anim_pose(m_SkeletonRes.m_Res->m_boneBindPose.size(), Bone(Quaternion::Identity(), Vector3::zero));
		m_Node->GetPose(anim_pose);
		BoneList bind_pose_hier(m_SkeletonRes.m_Res->m_boneBindPose.size());
		BoneList anim_pose_hier(m_SkeletonRes.m_Res->m_boneBindPose.size());
		my::BoneIndexSet::const_iterator root_iter = m_SkeletonRes.m_Res->m_boneRootSet.begin();
		for (; root_iter != m_SkeletonRes.m_Res->m_boneRootSet.end(); root_iter++)
		{
			anim_pose.IncrementSelf(
				m_SkeletonRes.m_Res->m_boneBindPose, m_SkeletonRes.m_Res->m_boneHierarchy, *root_iter);

			m_SkeletonRes.m_Res->m_boneBindPose.BuildHierarchyBoneList(
				bind_pose_hier, m_SkeletonRes.m_Res->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));

			anim_pose.BuildHierarchyBoneList(
				anim_pose_hier, m_SkeletonRes.m_Res->m_boneHierarchy, *root_iter, Quaternion(0,0,0,1), Vector3(0,0,0));
		}
		BoneList final_pose(bind_pose_hier.size());
		for (size_t i = 0; i < bind_pose_hier.size(); i++)
		{
			final_pose[i].m_rotation = bind_pose_hier[i].m_rotation.conjugate() * anim_pose_hier[i].m_rotation;
			final_pose[i].m_position = (-bind_pose_hier[i].m_position).transform(final_pose[i].m_rotation) + anim_pose_hier[i].m_position;
		}
		m_DualQuats.resize(m_SkeletonRes.m_Res->m_boneBindPose.size());
		final_pose.BuildDualQuaternionList(m_DualQuats);
	}
}

BOOST_CLASS_EXPORT(AnimationNodeSequence)

AnimationNodeSequence::AnimationNodeSequence(void)
	: m_Time(0)
{
}

AnimationNodeSequence::~AnimationNodeSequence(void)
{
}

void AnimationNodeSequence::Advance(float fElapsedTime)
{
	if (m_Owner->m_SkeletonRes.m_Res)
	{
		const OgreAnimation * anim = m_Owner->m_SkeletonRes.m_Res->GetAnimation(m_Name);
		if (anim)
		{
			m_Time = fmod(m_Time + fElapsedTime, anim->GetTime());
		}
	}
}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose) const
{
	if (m_Owner->m_SkeletonRes.m_Res)
	{
		const OgreAnimation * anim = m_Owner->m_SkeletonRes.m_Res->GetAnimation(m_Name);
		if (anim)
		{
			boost::unordered_map<std::string, int>::const_iterator root_iter = m_Owner->m_SkeletonRes.m_Res->m_boneNameMap.find(m_Root);
			if (root_iter != m_Owner->m_SkeletonRes.m_Res->m_boneNameMap.end())
			{
				anim->GetPose(pose, m_Owner->m_SkeletonRes.m_Res->m_boneHierarchy, root_iter->second, m_Time);
			}
		}
	}
	return pose;
}

AnimationNodeBlend::AnimationNodeBlend(void)
	: m_BlendTime(0)
	, m_ActiveChild(0)
{
}

AnimationNodeBlend::~AnimationNodeBlend(void)
{
}

void AnimationNodeBlend::SetActiveChild(unsigned int ActiveChild, float BlendTime)
{
	_ASSERT(ActiveChild < AnimationNodePtrArray::static_size);
	m_ActiveChild = ActiveChild;
	m_BlendTime = BlendTime;
	m_TargetWeight = (ActiveChild == 0 ? 0.0f : 1.0f);
}

void AnimationNodeBlend::SetOwner(Animator * Owner)
{
	AnimationNode::SetOwner(Owner);

	AnimationNodePtrArray::iterator child_iter = m_Childs.begin();
	for (; child_iter != m_Childs.end(); child_iter++)
	{
		(*child_iter)->SetOwner(Owner);
	}
}

void AnimationNodeBlend::Advance(float fElapsedTime)
{
	AnimationNodePtrArray::iterator child_iter = m_Childs.begin();
	for (; child_iter != m_Childs.end(); child_iter++)
	{
		(*child_iter)->Advance(fElapsedTime);
	}

	if (m_BlendTime < fElapsedTime)
	{
		m_Weight = m_TargetWeight;
		m_BlendTime = 0;
	}
	else if (m_BlendTime > 0)
	{
		const float delta = m_TargetWeight - m_Weight;
		m_Weight += delta * fElapsedTime / m_BlendTime;
		m_BlendTime -= fElapsedTime;
	}
}

my::BoneList & AnimationNodeBlend::GetPose(my::BoneList & pose) const
{
	if (m_Weight <= 0.0f)
	{
		return m_Childs[0]->GetPose(pose);
	}

	if (m_Weight >= 1.0f)
	{
		return m_Childs[1]->GetPose(pose);
	}

	m_Childs[0]->GetPose(pose);
	my::BoneList child_pose(pose.size());
	m_Childs[1]->GetPose(child_pose);
	for (unsigned int i = 0; i < pose.size(); i++)
	{
		pose[i].LerpSelf(child_pose[i], m_Weight);
	}
	return pose;
}

AnimationNodeBlendBySpeed::AnimationNodeBlendBySpeed(void)
	: m_Speed0(0.1f)
{
}

AnimationNodeBlendBySpeed::~AnimationNodeBlendBySpeed(void)
{
}

void AnimationNodeBlendBySpeed::Advance(float fElapsedTime)
{
	//if (m_Owner->m_Cmp->getVelocity().magnitudeSq() < m_Speed0)
	//{
	//	if (m_ActiveChild != 0)
	//	{
	//		SetActiveChild(0, 0.3f);
	//	}
	//}
	//else
	//{
	//	if (m_ActiveChild != 1)
	//	{
	//		SetActiveChild(1, 0.3f);
	//	}
	//}

	AnimationNodeBlend::Advance(fElapsedTime);
}

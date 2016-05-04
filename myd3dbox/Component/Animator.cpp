#include "stdafx.h"
#include "Animator.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
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
	: m_Character(NULL)
{
}

Animator::~Animator(void)
{
}

template<>
void Animator::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_SkeletonRes);
	ar << BOOST_SERIALIZATION_NVP(m_Node);
}

template<>
void Animator::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version)
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
		BoneList pose(m_SkeletonRes.m_Res->m_boneBindPose.size());
		m_Node->GetPose(pose);
		m_DualQuats.resize(m_SkeletonRes.m_Res->m_boneBindPose.size());
		pose.BuildDualQuaternionList(m_DualQuats);
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
		m_Time = fmod(m_Time + fElapsedTime, m_Owner->m_SkeletonRes.m_Res->GetAnimation(m_Name).GetTime());
	}
}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose) const
{
	if (m_Owner->m_SkeletonRes.m_Res)
	{
		boost::unordered_map<std::string, int>::const_iterator root_iter = m_Owner->m_SkeletonRes.m_Res->m_boneNameMap.find(m_Root);
		if (root_iter != m_Owner->m_SkeletonRes.m_Res->m_boneNameMap.end())
		{
			m_Owner->m_SkeletonRes.m_Res->BuildAnimationPose(
				pose, m_Owner->m_SkeletonRes.m_Res->m_boneHierarchy, root_iter->second, m_Name, m_Time);
		}
	}
	return pose;
}

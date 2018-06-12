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

BOOST_CLASS_EXPORT(Animator)

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
	m_Node->m_Owner = this;
	m_Node->OnSetOwner();
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
	if (m_SkeletonRes.m_Res && m_Node)
	{
		m_Node->Tick(fElapsedTime);
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

void Animator::AddToSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	_ASSERT(FindFromSequenceGroup(name, sequence) == m_SeqGroups.end());
	m_SeqGroups.insert(std::make_pair(name, sequence));
}

void Animator::RemoveFromSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	SequenceGroupMap::iterator seq_iter = FindFromSequenceGroup(name, sequence);
	_ASSERT(seq_iter != m_SeqGroups.end());
	m_SeqGroups.erase(seq_iter);
}

Animator::SequenceGroupMap::iterator Animator::FindFromSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	SequenceGroupMap::iterator seq_iter = m_SeqGroups.find(name);
	for (; seq_iter != m_SeqGroups.end(); seq_iter++)
	{
		if (seq_iter->second == sequence)
		{
			return seq_iter;
		}
	}
	return m_SeqGroups.end();
}

void Animator::UpdateGroupTime(const std::string & name, AnimationNodeSequence * sequence)
{
	float alpha = sequence->m_Time / sequence->GetLength();
	SequenceGroupMap::iterator seq_iter = m_SeqGroups.find(name);
	for (; seq_iter != m_SeqGroups.end(); seq_iter++)
	{
		if (seq_iter->second != sequence)
		{
			seq_iter->second->m_Time = Lerp<float>(alpha, 0, seq_iter->second->GetLength());
		}
	}
}

BOOST_CLASS_EXPORT(AnimationNode)

void AnimationNode::Tick(float duration)
{
}

my::BoneList & AnimationNode::GetPose(my::BoneList & pose) const
{
	return pose;
}

BOOST_CLASS_EXPORT(AnimationNodeSequence)

void AnimationNodeSequence::OnSetOwner(void)
{
	if (m_Owner && !m_Group.empty())
	{
		m_Owner->AddToSequenceGroup(m_Group, this);
	}
}

void AnimationNodeSequence::Tick(float fElapsedTime)
{
	if (m_Group.empty())
	{
		Advance(fElapsedTime);
	}
}

void AnimationNodeSequence::Advance(float fElapsedTime)
{
	if (m_Owner->m_SkeletonRes.m_Res)
	{
		m_Time = fmod(m_Time + fElapsedTime, GetLength());
	}
}

float AnimationNodeSequence::GetLength(void) const
{
	const OgreAnimation * anim = m_Owner->m_SkeletonRes.m_Res->GetAnimation(m_Name);
	if (anim)
	{
		return anim->GetTime();
	}
	return 0;
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

BOOST_CLASS_EXPORT(AnimationNodeBlend)

template<>
void AnimationNodeBlend::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
	ar << BOOST_SERIALIZATION_NVP(m_BlendTime);
	ar << BOOST_SERIALIZATION_NVP(m_ActiveChild);
}

template<>
void AnimationNodeBlend::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	ar >> BOOST_SERIALIZATION_NVP(m_BlendTime);
	ar >> BOOST_SERIALIZATION_NVP(m_ActiveChild);
}

void AnimationNodeBlend::OnSetOwner(void)
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		m_Childs[i]->m_Owner = m_Owner;
		m_Childs[i]->OnSetOwner();
	}
}

void AnimationNodeBlend::SetActiveChild(unsigned int ActiveChild, float BlendTime)
{
	_ASSERT(ActiveChild < AnimationNodePtrArray::static_size);
	m_ActiveChild = ActiveChild;
	m_BlendTime = BlendTime;
	m_TargetWeight = (ActiveChild == 0 ? 0.0f : 1.0f);
}

void AnimationNodeBlend::Tick(float fElapsedTime)
{
	AnimationNodePtrArray::iterator child_iter = m_Childs.begin();
	for (; child_iter != m_Childs.end(); child_iter++)
	{
		(*child_iter)->Tick(fElapsedTime);
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

BOOST_CLASS_EXPORT(AnimationNodeBlendBySpeed)

void AnimationNodeBlendBySpeed::Tick(float fElapsedTime)
{
	Character * character = dynamic_cast<Character *>(m_Owner->m_Actor);
	if (character)
	{
		float speed = character->m_Velocity.x * character->m_Velocity.x + character->m_Velocity.z * character->m_Velocity.z;
		if (speed < m_Speed0)
		{
			if (m_ActiveChild != 0)
			{
				SetActiveChild(0, 0.3f);
			}
		}
		else
		{
			if (m_ActiveChild != 1)
			{
				SetActiveChild(1, 0.3f);
			}
		}
	}

	AnimationNodeBlend::Tick(fElapsedTime);
}

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
		m_Node->Tick(fElapsedTime, 1.0f);

		UpdateGroup(fElapsedTime);

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

void Animator::UpdateGroup(float fElapsedTime)
{
	SequenceGroupMap::iterator seq_iter = m_SequenceGroups.begin();
	while (seq_iter != m_SequenceGroups.end())
	{
		SequenceGroupMap::iterator master_seq_iter = seq_iter;
		SequenceGroupMap::iterator next_seq_iter = seq_iter;
		for (next_seq_iter++; next_seq_iter != m_SequenceGroups.end() && next_seq_iter->first == seq_iter->first; next_seq_iter++)
		{
			if (next_seq_iter->second->m_Weight > master_seq_iter->second->m_Weight)
			{
				master_seq_iter = next_seq_iter;
			}
		}

		_ASSERT(master_seq_iter != m_SequenceGroups.end());
		master_seq_iter->second->Advance(fElapsedTime);
		float time_pct = master_seq_iter->second->m_Time / master_seq_iter->second->GetLength();

		SequenceGroupMap::iterator adjust_seq_iter = seq_iter;
		for (; adjust_seq_iter != next_seq_iter; adjust_seq_iter++)
		{
			if (adjust_seq_iter != master_seq_iter)
			{
				adjust_seq_iter->second->m_Time = Lerp(0.0f, adjust_seq_iter->second->GetLength(), time_pct);
			}
		}
		seq_iter = next_seq_iter;
	}
}

void Animator::AddToSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	SequenceGroupMap::_Pairii range = m_SequenceGroups.equal_range(name);
	SequenceGroupMap::iterator seq_iter = std::find(range.first, range.second, SequenceGroupMap::value_type(name, sequence));
	_ASSERT(seq_iter == range.second);
	m_SequenceGroups.insert(std::make_pair(name, sequence));
}

void Animator::RemoveFromSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	SequenceGroupMap::_Pairii range = m_SequenceGroups.equal_range(name);
	SequenceGroupMap::iterator seq_iter = std::find(range.first, range.second, SequenceGroupMap::value_type(name, sequence));
	_ASSERT(seq_iter != range.second);
	m_SequenceGroups.erase(seq_iter);
}

BOOST_CLASS_EXPORT(AnimationNode)

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

void AnimationNodeSequence::UpdateRate(float fRate)
{
	m_Rate = fRate;
}

void AnimationNodeSequence::Tick(float fElapsedTime, float fTotalWeight)
{
	m_Weight = fTotalWeight;

	if (m_Group.empty())
	{
		Advance(fElapsedTime);
	}
}

void AnimationNodeSequence::Advance(float fElapsedTime)
{
	m_Time = fmod(m_Time + fElapsedTime * m_Rate, GetLength());
}

float AnimationNodeSequence::GetLength(void) const
{
	if (m_Owner->m_SkeletonRes.m_Res)
	{
		const OgreAnimation * anim = m_Owner->m_SkeletonRes.m_Res->GetAnimation(m_Name);
		if (anim)
		{
			return anim->GetTime();
		}
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

BOOST_CLASS_EXPORT(AnimationNodeSlot)

void AnimationNodeSlot::OnSetOwner(void)
{
	AnimationNodeSequence::OnSetOwner();

	if (m_Child0)
	{
		m_Child0->OnSetOwner();
	}
}

void AnimationNodeSlot::UpdateRate(float fRate)
{
	AnimationNodeSequence::UpdateRate(fRate);

	if (m_Child0)
	{
		m_Child0->UpdateRate(fRate);
	}
}

void AnimationNodeSlot::Tick(float fElapsedTime, float fTotalWeight)
{
	float TargetWeight = m_Playing ? 1.0f : 0.0f;
	if (m_BlendTime < fElapsedTime)
	{
		m_Weight = TargetWeight;
		m_BlendTime = 0;
	}
	else if (m_BlendTime > 0)
	{
		const float delta = TargetWeight - m_Weight;
		m_Weight += delta * fElapsedTime / m_BlendTime;
		m_BlendTime -= fElapsedTime;
	}

	m_Child0->Tick(fElapsedTime, fTotalWeight * (1 - TargetWeight));

	AnimationNodeSequence::Tick(fElapsedTime, fTotalWeight * TargetWeight);
}

void AnimationNodeSlot::Advance(float fElapsedTime)
{
	if (m_Playing)
	{
		if (m_Time < GetLength())
		{
			m_Time += fElapsedTime * m_Rate;
		}
		else if (m_Loop)
		{
			m_Time = 0;
		}
		else
		{
			m_Playing = false;
			m_BlendTime = 0.3f;
		}
	}
}

void AnimationNodeSlot::Play(const std::string & Name, bool Loop, float Rate)
{
	m_Time = 0;
	m_Rate = Rate;
	m_Name = Name;
	m_Playing = true;
	m_BlendTime = 0.3f;
	m_Loop = Loop;
}

void AnimationNodeSlot::Stop(void)
{
	m_Playing = false;
	m_BlendTime = 0.3f;
}

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose) const
{
	if (m_Weight <= 0.0f)
	{
		return m_Child0->GetPose(pose);
	}

	if (m_Weight >= 1.0f)
	{
		AnimationNodeSequence::GetPose(pose);
	}

	m_Child0->GetPose(pose);
	my::BoneList child_pose(pose.size());
	AnimationNodeSequence::GetPose(child_pose);
	for (unsigned int i = 0; i < pose.size(); i++)
	{
		pose[i].LerpSelf(child_pose[i], m_Weight);
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

void AnimationNodeBlend::UpdateRate(float fRate)
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		m_Childs[i]->UpdateRate(fRate);
	}
}

void AnimationNodeBlend::SetActiveChild(unsigned int ActiveChild, float BlendTime)
{
	_ASSERT(ActiveChild < AnimationNodePtrArray::static_size);
	m_ActiveChild = ActiveChild;
	m_BlendTime = BlendTime;
	m_TargetWeight = (ActiveChild == 0 ? 0.0f : 1.0f);
}

void AnimationNodeBlend::Tick(float fElapsedTime, float fTotalWeight)
{
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

	m_Childs[0]->Tick(fElapsedTime, fTotalWeight * (1 - m_TargetWeight));

	m_Childs[1]->Tick(fElapsedTime, fTotalWeight * m_TargetWeight);
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

void AnimationNodeBlendBySpeed::Tick(float fElapsedTime, float fTotalWeight)
{
	Character * character = dynamic_cast<Character *>(m_Owner->m_Actor);
	if (character)
	{
		float speed_sq = character->m_Velocity.x * character->m_Velocity.x + character->m_Velocity.z * character->m_Velocity.z;
		if (speed_sq < m_Speed0 * m_Speed0)
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

	AnimationNodeBlend::Tick(fElapsedTime, fTotalWeight);
}

void AnimationNodeRateBySpeed::OnSetOwner(void)
{
	m_Child0->OnSetOwner();
}

void AnimationNodeRateBySpeed::UpdateRate(float fRate)
{
	m_Child0->UpdateRate(fRate);
}

void AnimationNodeRateBySpeed::Tick(float fElapsedTime, float fTotalWeight)
{
	Character * character = dynamic_cast<Character *>(m_Owner->m_Actor);
	if (character)
	{
		float speed_sq = character->m_Velocity.x * character->m_Velocity.x + character->m_Velocity.z * character->m_Velocity.z;
		UpdateRate(sqrtf(speed_sq) / m_BaseSpeed);
	}

	m_Child0->Tick(fElapsedTime, fTotalWeight);
}

my::BoneList & AnimationNodeRateBySpeed::GetPose(my::BoneList & pose) const
{
	return m_Child0->GetPose(pose);
}

#include "Animator.h"
#include "Actor.h"
#include "myResource.h"
#include "PhysxContext.h"
#include "myUtility.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/copy.hpp>

using namespace my;

BOOST_CLASS_EXPORT(AnimationNode)

BOOST_CLASS_EXPORT(AnimationNodeSequence)

BOOST_CLASS_EXPORT(AnimationNodeSlot)

BOOST_CLASS_EXPORT(AnimationNodeBlend)
//
//BOOST_CLASS_EXPORT(AnimationNodeBlendBySpeed)

BOOST_CLASS_EXPORT(AnimationNodeRate)

BOOST_CLASS_EXPORT(Animator)

AnimationNode::~AnimationNode(void)
{
	_ASSERT(!m_Parent);

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		RemoveChild(i);
	}
}

template<class Archive>
void AnimationNode::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
}

template<class Archive>
void AnimationNode::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->m_Parent = this;
		}
	}
}

void AnimationNode::RemoveChild(int i)
{
	if (m_Childs[i])
	{
		_ASSERT(m_Childs[i]->m_Parent == this);
		m_Childs[i]->m_Parent = NULL;
		m_Childs[i].reset();
	}
}

const AnimationNode * AnimationNode::GetTopNode(void) const
{
	if (m_Parent)
	{
		return m_Parent->GetTopNode();
	}
	return this;
}

AnimationNode * AnimationNode::GetTopNode(void)
{
	if (m_Parent)
	{
		return m_Parent->GetTopNode();
	}
	return this;
}

AnimationNodeSequence::~AnimationNodeSequence(void)
{
	if (m_GroupOwner)
	{
		m_GroupOwner->RemoveSequenceGroup(m_Group, this);
	}
}

AnimationNodeSequence & AnimationNodeSequence::operator = (const AnimationNodeSequence & rhs)
{
	if (m_GroupOwner)
	{
		m_GroupOwner->RemoveSequenceGroup(m_Group, this);
	}

	m_Time = rhs.m_Time;
	m_Weight = rhs.m_Weight;
	m_LastElapsedTime = rhs.m_LastElapsedTime;
	m_Name = rhs.m_Name;
	m_Rate = rhs.m_Rate;
	m_Loop = rhs.m_Loop;
	m_Group = rhs.m_Group;

	if (rhs.m_GroupOwner)
	{
		Animator * TmpGroupOwner = rhs.m_GroupOwner;

		rhs.m_GroupOwner->RemoveSequenceGroup(rhs.m_Group, const_cast<AnimationNodeSequence *>(&rhs));

		TmpGroupOwner->AddSequenceGroup(m_Group, this);
	}

	return *this;
}

void AnimationNodeSequence::Tick(float fElapsedTime, float fTotalWeight)
{
	m_Weight = fTotalWeight;

	if (!m_Group.empty())
	{
		m_LastElapsedTime = fElapsedTime;
	}
	else
	{
		Advance(fElapsedTime);
	}
}

void AnimationNodeSequence::Advance(float fElapsedTime)
{
	if (m_Loop)
	{
		m_Time = fmod(m_Time + fElapsedTime * m_Rate, GetLength());
	}
	else
	{
		m_Time = my::Min(m_Time + fElapsedTime * m_Rate, GetLength());
	}
}

float AnimationNodeSequence::GetLength(void) const
{
	const Animator * Root = dynamic_cast<const Animator *>(GetTopNode());
	if (Root->m_Skeleton)
	{
		const OgreAnimation * anim = Root->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			return anim->GetLength();
		}
	}
	return 1.0f;
}
//
//void AnimationNodeSequence::SetRootList(std::string RootList)
//{
//	boost::trim_if(RootList, boost::algorithm::is_any_of(" \t,"));
//	if (!RootList.empty())
//	{
//		boost::algorithm::split(m_RootList, RootList, boost::algorithm::is_any_of(" \t,"), boost::algorithm::token_compress_on);
//	}
//	else
//	{
//		m_RootList.clear();
//	}
//}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose) const
{
	const Animator * Root = dynamic_cast<const Animator *>(GetTopNode());
	if (Root->m_Skeleton)
	{
		const OgreAnimation * anim = Root->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			BoneIndexSet::const_iterator root_iter = Root->m_Skeleton->m_boneRootSet.begin();
			for (; root_iter != Root->m_Skeleton->m_boneRootSet.end(); root_iter++)
			{
				anim->GetPose(pose, Root->m_Skeleton->m_boneHierarchy, *root_iter, m_Time);
			}
		}
	}
	return pose;
}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose, int root_i) const
{
	const Animator* Root = dynamic_cast<const Animator*>(GetTopNode());
	if (Root->m_Skeleton)
	{
		const OgreAnimation* anim = Root->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			anim->GetPose(pose, Root->m_Skeleton->m_boneHierarchy, root_i, m_Time);
		}
	}
	return pose;
}

void AnimationNodeSlot::Tick(float fElapsedTime, float fTotalWeight)
{
	Animator * Root = dynamic_cast<Animator *>(GetTopNode());
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end();)
	{
		float Weight = 0;
		if (seq_iter->m_BlendTime < fElapsedTime)
		{
			if (seq_iter->m_TargetWeight <= 0)
			{
				seq_iter = m_SequenceSlot.erase(seq_iter);
				continue;
			}
			Weight = seq_iter->m_TargetWeight;
			seq_iter->m_BlendTime = 0;
		}
		else if (seq_iter->m_BlendTime > 0)
		{
			float WeightDelta = seq_iter->m_TargetWeight - seq_iter->m_Weight;
			Weight = seq_iter->m_Weight + WeightDelta * fElapsedTime / seq_iter->m_BlendTime;
			seq_iter->m_BlendTime -= fElapsedTime;
		}

		seq_iter->Tick(fElapsedTime, Weight);

		fTotalWeight = Max(0.0f, fTotalWeight - Weight);

		if (seq_iter->m_TargetWeight > 0 && !seq_iter->m_Loop && seq_iter->m_Time >= seq_iter->GetLength())
		{
			seq_iter->m_TargetWeight = 0;
			seq_iter->m_BlendTime = seq_iter->m_BlendOutTime;
		}

		seq_iter++;
	}

	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime, fTotalWeight);
	}
}

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose) const
{
	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose);
	}

	SequenceList::const_reverse_iterator seq_iter = m_SequenceSlot.rbegin();
	for (; seq_iter != m_SequenceSlot.rend(); seq_iter++)
	{
		my::BoneList OtherPose(pose.size());
		seq_iter->GetPose(OtherPose);
		pose.LerpSelf(OtherPose, seq_iter->m_Weight);
	}
	return pose;
}

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose, int root_i) const
{
	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose, root_i);
	}

	const Animator* Root = dynamic_cast<const Animator*>(GetTopNode());
	if (Root->m_Skeleton)
	{
		SequenceList::const_reverse_iterator seq_iter = m_SequenceSlot.rbegin();
		for (; seq_iter != m_SequenceSlot.rend(); seq_iter++)
		{
			my::BoneList OtherPose(pose.size());
			seq_iter->GetPose(OtherPose, root_i);
			pose.LerpSelf(OtherPose, Root->m_Skeleton->m_boneHierarchy, root_i, seq_iter->m_Weight);
		}
	}
	return pose;
}

void AnimationNodeSlot::Play(const std::string & Name, std::string RootList, float Rate, float BlendTime, float BlendOutTime, bool Loop, int Priority, float StartTime, const std::string & Group, DWORD_PTR UserData)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		if (seq_iter->m_Priority)
		{
			break;
		}
	}

	SequenceList::iterator res_seq_iter = m_SequenceSlot.rinsert(seq_iter);
	if (res_seq_iter != m_SequenceSlot.end())
	{
		res_seq_iter->m_Time = StartTime;
		res_seq_iter->m_Weight = 0;
		res_seq_iter->m_Name = Name;
		//res_seq_iter->SetRootList(RootList);
		res_seq_iter->m_Rate = Rate;
		res_seq_iter->m_Loop = Loop;
		res_seq_iter->m_Group = Group;
		res_seq_iter->m_Priority = Priority;
		res_seq_iter->m_BlendTime = BlendTime;
		res_seq_iter->m_BlendOutTime = BlendOutTime;
		res_seq_iter->m_TargetWeight = 1.0f;
		res_seq_iter->m_UserData = UserData;
		res_seq_iter->m_Parent = this;

		if (!Group.empty())
		{
			Animator* Root = dynamic_cast<Animator*>(GetTopNode());
			Root->AddSequenceGroup(Group, &m_SequenceSlot.front());
			Root->SyncSequenceGroupTime(Group, res_seq_iter->m_Time / res_seq_iter->GetLength());
		}
	}
}

void AnimationNodeSlot::StopIndex(int i)
{
	_ASSERT(i >= 0 && i < (int)m_SequenceSlot.size());

	m_SequenceSlot[i].m_TargetWeight = 0;
	m_SequenceSlot[i].m_BlendTime = m_SequenceSlot[i].m_BlendOutTime;
}

void AnimationNodeSlot::Stop(DWORD_PTR UserData)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		if (seq_iter->m_UserData == UserData && seq_iter->m_TargetWeight != 0)
		{
			StopIndex(std::distance(m_SequenceSlot.begin(), seq_iter));
		}
	}
}

void AnimationNodeSlot::StopAll(void)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		if (seq_iter->m_TargetWeight != 0)
		{
			StopIndex(std::distance(m_SequenceSlot.begin(), seq_iter));
		}
	}
}

void AnimationNodeBlend::SetActiveChild(int ActiveChild, float BlendTime)
{
	_ASSERT(ActiveChild < m_Childs.size());

	if (ActiveChild <= 0)
	{
		if (m_TargetWeight > 0.0f)
		{
			m_TargetWeight = 0.0f;
			m_BlendTime = BlendTime;
		}
	}
	else
	{
		if (m_TargetWeight < 1.0f)
		{
			m_TargetWeight = 1.0f;
			m_BlendTime = BlendTime;
		}
	}
}

int AnimationNodeBlend::GetActiveChild(void) const
{
	if (m_TargetWeight < 0.5f)
	{
		return 0;
	}
	return 1;
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

	if (m_Childs[0] && (m_Weight < 1.0f || m_TargetRootId >= 0))
	{
		m_Childs[0]->Tick(fElapsedTime, fTotalWeight * (m_TargetRootId >= 0 ? 1.0f : (1.0f - m_TargetWeight)));
	}

	if (m_Childs[1] && m_Weight > 0.0f)
	{
		m_Childs[1]->Tick(fElapsedTime, fTotalWeight * m_TargetWeight);
	}
}

my::BoneList & AnimationNodeBlend::GetPose(my::BoneList & pose) const
{
	if (m_Weight <= 0.0f)
	{
		if (m_Childs[0])
		{
			return m_Childs[0]->GetPose(pose);
		}
		return pose;
	}

	if (m_Weight >= 1.0f && m_TargetRootId < 0)
	{
		if (m_Childs[1])
		{
			return m_Childs[1]->GetPose(pose);
		}
		return pose;
	}

	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose);
	}

	const Animator* Root = dynamic_cast<const Animator*>(GetTopNode());
	if (m_Childs[1])
	{
		my::BoneList OtherPose(pose.size());
		m_Childs[1]->GetPose(OtherPose);
		if (m_TargetRootId < 0)
		{
			pose.LerpSelf(OtherPose, m_Weight);
		}
		else if (Root->m_Skeleton)
		{
			pose.LerpSelf(OtherPose, Root->m_Skeleton->m_boneHierarchy, m_TargetRootId, m_Weight);
		}
	}

	return pose;
}

my::BoneList & AnimationNodeBlend::GetPose(my::BoneList & pose, int root_i) const
{
	if (m_Weight <= 0.0f)
	{
		if (m_Childs[0])
		{
			return m_Childs[0]->GetPose(pose, root_i);
		}
		return pose;
	}

	if (m_Weight >= 1.0f && m_TargetRootId < 0)
	{
		if (m_Childs[1])
		{
			return m_Childs[1]->GetPose(pose, root_i);
		}
		return pose;
	}

	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose, root_i);
	}

	const Animator* Root = dynamic_cast<const Animator*>(GetTopNode());
	if (Root->m_Skeleton && m_Childs[1])
	{
		my::BoneList OtherPose(pose.size());
		m_Childs[1]->GetPose(OtherPose, root_i);
		if (m_TargetRootId < 0)
		{
			pose.LerpSelf(OtherPose, Root->m_Skeleton->m_boneHierarchy, root_i, m_Weight);
		}
		else
		{
			pose.LerpSelf(OtherPose, Root->m_Skeleton->m_boneHierarchy, m_TargetRootId, m_Weight);
		}
	}

	return pose;
}

AnimationNodeBlendList::AnimationNodeBlendList(void)
{

}

AnimationNodeBlendList::~AnimationNodeBlendList(void)
{

}

void AnimationNodeBlendList::SetActiveChild(int ActiveChild, float BlendTime)
{
	if (m_TargetWeight.size() < m_Childs.size())
	{
		m_TargetWeight.resize(m_Childs.size(), 0);

		m_Weight.resize(m_Childs.size(), 0);
	}

	if (ActiveChild >= 0 && ActiveChild < m_Childs.size() && m_TargetWeight[ActiveChild] < 1.0f)
	{
		for (int i = 0; i < m_TargetWeight.size(); i++)
		{
			if (i == ActiveChild)
			{
				m_TargetWeight[i] = 1.0f;
			}
			else
			{
				m_TargetWeight[i] = 0.0f;
			}
		}

		m_BlendTime = BlendTime;
	}
}

int AnimationNodeBlendList::GetActiveChild(void) const
{
	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_TargetWeight[i] > 0.5f)
		{
			return i;
		}
	}
	return 0;
}

void AnimationNodeBlendList::Tick(float fElapsedTime, float fTotalWeight)
{
	if (m_BlendTime < fElapsedTime)
	{
		boost::range::copy(m_Weight, m_TargetWeight.begin());

		m_BlendTime = 0;
	}
	else
	{
		for (int i = 0; i < m_TargetWeight.size(); i++)
		{
			const float delta = m_TargetWeight[i] - m_Weight[i];
			m_Weight[i] += delta * fElapsedTime / m_BlendTime;
		}
		m_BlendTime -= fElapsedTime;
	}

	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_Childs[i] && m_Weight[i] > 0.0f)
		{
			m_Childs[i]->Tick(fElapsedTime, m_TargetWeight[i]);
		}
	}
}

my::BoneList & AnimationNodeBlendList::GetPose(my::BoneList & pose) const
{
	bool init_pose = false;

	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_Childs[i] && m_Weight[i] > 0.0f)
		{
			if (!init_pose)
			{
				m_Childs[i]->GetPose(pose);
				init_pose = true;
			}
			else
			{
				my::BoneList OtherPose(pose.size());
				m_Childs[i]->GetPose(OtherPose);
				pose.LerpSelf(OtherPose, m_Weight[i]);
			}
		}
	}

	return pose;
}

my::BoneList & AnimationNodeBlendList::GetPose(my::BoneList & pose, int root_i) const
{
	bool init_pose = false;

	const Animator* Root = dynamic_cast<const Animator*>(GetTopNode());
	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_Childs[i] && m_Weight[i] > 0.0f)
		{
			if (!init_pose)
			{
				m_Childs[i]->GetPose(pose, root_i);
				init_pose = true;
			}
			else if (Root->m_Skeleton)
			{
				my::BoneList OtherPose(pose.size());
				m_Childs[i]->GetPose(OtherPose, root_i);
				pose.LerpSelf(OtherPose, Root->m_Skeleton->m_boneHierarchy, root_i, m_Weight[i]);
			}
		}
	}

	return pose;
}

void AnimationNodeRate::Tick(float fElapsedTime, float fTotalWeight)
{
	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime * m_Rate, fTotalWeight);
	}
}

my::BoneList & AnimationNodeRate::GetPose(my::BoneList & pose) const
{
	if (m_Childs[0])
	{
		return m_Childs[0]->GetPose(pose);
	}
	return pose;
}

my::BoneList & AnimationNodeRate::GetPose(my::BoneList & pose, int root_i) const
{
	if (m_Childs[0])
	{
		return m_Childs[0]->GetPose(pose, root_i);
	}
	return pose;
}

template<class Archive>
void Animator::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNodeSlot);
	ar << BOOST_SERIALIZATION_NVP(m_SkeletonPath);
}

template<class Archive>
void Animator::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNodeSlot);
	ar >> BOOST_SERIALIZATION_NVP(m_SkeletonPath);
	ReloadSequenceGroup();
}

Animator::~Animator(void)
{
	if (!m_SkeletonPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_SkeletonPath, this);
	}

	SequenceGroupMap::iterator group_iter = m_SequenceGroup.begin();
	for (; group_iter != m_SequenceGroup.end(); group_iter = m_SequenceGroup.begin())
	{
		RemoveSequenceGroup(group_iter->first, group_iter->second);
	}
}

void Animator::OnSkeletonReady(my::DeviceResourceBasePtr res)
{
	m_Skeleton = boost::dynamic_pointer_cast<my::OgreSkeletonAnimation>(res);

	bind_pose_hier.resize(m_Skeleton->m_boneBindPose.size());
	anim_pose_hier.resize(m_Skeleton->m_boneBindPose.size());
	my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
	for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
	{
		m_Skeleton->m_boneBindPose.BuildHierarchyBoneList(
			bind_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion::Identity(), Vector3(0, 0, 0));

		m_Skeleton->m_boneBindPose.BuildHierarchyBoneList(
			anim_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion::Identity(), Vector3(0, 0, 0));
	}
	anim_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Quaternion::Identity(), Vector3(0, 0, 0)));
	final_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Quaternion::Identity(), Vector3(0, 0, 0)));
}

void Animator::RequestResource(void)
{
	Component::RequestResource();

	if (!m_SkeletonPath.empty())
	{
		_ASSERT(!m_Skeleton);

		my::ResourceMgr::getSingleton().LoadSkeletonAsync(m_SkeletonPath.c_str(), boost::bind(&Animator::OnSkeletonReady, this, boost::placeholders::_1));
	}
}

void Animator::ReleaseResource(void)
{
	Component::ReleaseResource();

	if (!m_SkeletonPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_SkeletonPath, boost::bind(&Animator::OnSkeletonReady, this, boost::placeholders::_1));

		m_Skeleton.reset();
	}
}

void Animator::Update(float fElapsedTime)
{
	if (m_Skeleton)
	{
		if (fElapsedTime > 0.0f)
		{
			Tick(fElapsedTime, 1.0f);

			UpdateSequenceGroup();
		}

		GetPose(anim_pose);

		my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
		for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
		{
			anim_pose.BuildHierarchyBoneList(
				anim_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion::Identity(), Vector3(0, 0, 0));
		}

		DynamicBoneContextMap::iterator db_iter = m_DynamicBones.begin();
		for (; db_iter != m_DynamicBones.end(); db_iter++)
		{
			int particle_i = 0;
			UpdateDynamicBone(db_iter->second, anim_pose_hier[db_iter->second.parent_i],
				anim_pose_hier[db_iter->second.parent_i].m_position.transformCoord(m_Actor->m_World), db_iter->first, particle_i, fElapsedTime);
		}

		IKContextMap::iterator ik_iter = m_Iks.begin();
		for (; ik_iter != m_Iks.end(); ik_iter++)
		{
			UpdateIK(ik_iter->second);
		}

		for (size_t i = 0; i < bind_pose_hier.size(); i++)
		{
			final_pose[i].m_rotation = bind_pose_hier[i].m_rotation.conjugate() * anim_pose_hier[i].m_rotation;
			final_pose[i].m_position = final_pose[i].m_rotation * -bind_pose_hier[i].m_position + anim_pose_hier[i].m_position;
		}

		m_DualQuats.resize(m_Skeleton->m_boneBindPose.size());
		final_pose.BuildDualQuaternionList(m_DualQuats);
	}
}

void Animator::AddSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	std::pair<SequenceGroupMap::iterator, SequenceGroupMap::iterator> range = m_SequenceGroup.equal_range(name);
	SequenceGroupMap::iterator seq_iter = std::find(range.first, range.second, SequenceGroupMap::value_type(name, sequence));
	_ASSERT(seq_iter == range.second);
	_ASSERT(sequence->m_GroupOwner == NULL);
	m_SequenceGroup.insert(std::make_pair(name, sequence));
	sequence->m_GroupOwner = this;
}

void Animator::RemoveSequenceGroup(const std::string & name, AnimationNodeSequence * sequence)
{
	std::pair<SequenceGroupMap::iterator, SequenceGroupMap::iterator> range = m_SequenceGroup.equal_range(name);
	SequenceGroupMap::iterator seq_iter = std::find(range.first, range.second, SequenceGroupMap::value_type(name, sequence));
	_ASSERT(seq_iter != range.second);
	_ASSERT(sequence->m_GroupOwner == this);
	m_SequenceGroup.erase(seq_iter);
	sequence->m_GroupOwner = NULL;
}

void Animator::ReloadSequenceGroup(void)
{
	m_SequenceGroup.clear();

	AnimationNodePtrList::iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			ReloadSequenceGroupWalker(node_iter->get());
		}
	}
}

void Animator::ReloadSequenceGroupWalker(AnimationNode * node)
{
	AnimationNodeSequence * sequence = dynamic_cast<AnimationNodeSequence *>(node);
	if (sequence && !sequence->m_Group.empty())
	{
		AddSequenceGroup(sequence->m_Group, sequence);
	}

	AnimationNodePtrList::iterator node_iter = node->m_Childs.begin();
	for (; node_iter != node->m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			ReloadSequenceGroupWalker(node_iter->get());
		}
	}
}

void Animator::UpdateSequenceGroup(void)
{
	SequenceGroupMap::iterator seq_iter = m_SequenceGroup.begin();
	while (seq_iter != m_SequenceGroup.end())
	{
		SequenceGroupMap::iterator master_seq_iter = seq_iter;
		SequenceGroupMap::iterator next_seq_iter = seq_iter;
		for (next_seq_iter++; next_seq_iter != m_SequenceGroup.end() && next_seq_iter->first == seq_iter->first; next_seq_iter++)
		{
			if (next_seq_iter->second->m_Weight > master_seq_iter->second->m_Weight)
			{
				master_seq_iter = next_seq_iter;
			}
		}

		_ASSERT(master_seq_iter != m_SequenceGroup.end());
		if (master_seq_iter->second->m_Weight > EPSILON_E3)
		{
			master_seq_iter->second->Advance(master_seq_iter->second->m_LastElapsedTime);

			float Time = master_seq_iter->second->m_Time;

			float Percent = Time / master_seq_iter->second->GetLength();

			SyncSequenceGroupTime(seq_iter, next_seq_iter, Percent);

			master_seq_iter->second->m_Time = Time;
		}
		seq_iter = next_seq_iter;
	}
}

void Animator::SyncSequenceGroupTime(const std::string & Group, float Percent)
{
	std::pair<SequenceGroupMap::iterator, SequenceGroupMap::iterator> range = m_SequenceGroup.equal_range(Group);
	SyncSequenceGroupTime(range.first, range.second, Percent);
}

void Animator::SyncSequenceGroupTime(SequenceGroupMap::iterator begin, SequenceGroupMap::iterator end, float Percent)
{
	SequenceGroupMap::iterator seq_iter = begin;
	for (; seq_iter != end; seq_iter++)
	{
		seq_iter->second->m_Time = Lerp(0.0f, seq_iter->second->GetLength(), Percent);
	}
}

void Animator::AddDynamicBone(int node_i, const my::BoneHierarchy & boneHierarchy, float mass, float damping, float springConstant)
{
	_ASSERT(mass > 0);

	int parent_i = boneHierarchy.FindParent(node_i);
	if (parent_i < 0)
	{
		return;
	}

	std::pair<DynamicBoneContextMap::iterator, bool> res = m_DynamicBones.insert(std::make_pair(node_i, DynamicBoneContext()));
	if (!res.second)
	{
		return;
	}

	res.first->second.parent_i = parent_i;
	res.first->second.springConstant = springConstant;
	AddDynamicBone(res.first->second, node_i, boneHierarchy, mass, damping);
}

void Animator::AddDynamicBone(DynamicBoneContext & context, int node_i, const my::BoneHierarchy & boneHierarchy, float mass, float damping)
{
	context.m_ParticleList.push_back(Particle(
		Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 1 / mass, damping));
	int child_i = boneHierarchy[node_i].m_child;
	for (; child_i >= 0; child_i = boneHierarchy[child_i].m_sibling)
	{
		AddDynamicBone(context, child_i, boneHierarchy, mass, damping);
	}
}

void Animator::UpdateDynamicBone(DynamicBoneContext & context, const my::Bone & parent, const my::Vector3& parent_world_pos, int node_i, int & particle_i, float fElapsedTime)
{
	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	Bone target(
		m_Skeleton->m_boneBindPose[node_i].m_rotation * parent.m_rotation,
		parent.m_rotation * m_Skeleton->m_boneBindPose[node_i].m_position + parent.m_position);
	Vector3 target_world_pos = target.m_position.transformCoord(m_Actor->m_World);

	Particle & particle = context.m_ParticleList[particle_i];
	particle.clearAccumulator();
	particle.setAcceleration(Vector3::Gravity);
	Vector3 distance = target_world_pos - particle.getPosition();
	float length = distance.magnitude();
	Vector3 direction = distance.normalize();
	Vector3 force = fabs(length) > EPSILON_E6 ? direction * (-context.springConstant * length) : Vector3(0, 0, 0);
	particle.addForce(force);
	particle.integrate(fElapsedTime);
	Vector3 d0 = target_world_pos - parent_world_pos;
	Vector3 d1 = particle.getPosition() - parent_world_pos;
	particle.setPosition(parent_world_pos + d1.normalize() * d0.magnitude());

	anim_pose_hier[node_i].m_rotation =
		target.m_rotation * m_Actor->m_Rotation * Quaternion::RotationFromTo(d0, d1, Vector3::zero) * m_Actor->m_Rotation.conjugate();

	anim_pose_hier[node_i].m_position =
		particle.getPosition().transformCoord(m_Actor->m_World.inverse());

	particle_i++;
	int child_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; child_i >= 0; child_i = m_Skeleton->m_boneHierarchy[child_i].m_sibling)
	{
		UpdateDynamicBone(context, anim_pose_hier[node_i], particle.getPosition(), child_i, particle_i, fElapsedTime);
	}
}

void Animator::AddIK(int node_i, const my::BoneHierarchy & boneHierarchy, float hitRadius, unsigned int filterWord0)
{
	std::pair<IKContextMap::iterator, bool> res = m_Iks.insert(std::make_pair(node_i, IKContext()));
	if (!res.second)
	{
		return;
	}

	int child_i = node_i;
	for (int i = 0; i < _countof(res.first->second.id); i++, child_i = boneHierarchy[child_i].m_child)
	{
		if (child_i < 0)
		{
			m_Iks.erase(node_i);
			return;
		}
		res.first->second.id[i] = child_i;
	}

	res.first->second.hitRadius = hitRadius;

	res.first->second.filterWord0 = filterWord0;
}

void Animator::UpdateIK(IKContext & ik)
{
	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	PhysxScene * scene = dynamic_cast<PhysxScene *>(m_Actor->m_Node->GetTopNode());
	_ASSERT(scene);

	Vector3 pos[3] = {
		anim_pose_hier[ik.id[0]].m_position,
		anim_pose_hier[ik.id[1]].m_position,
		anim_pose_hier[ik.id[2]].m_position };
	Vector3 dir[3] = { pos[1] - pos[0], pos[2] - pos[1], pos[2] - pos[0] };
	float length[3] = { dir[0].magnitude(), dir[1].magnitude(), dir[2].magnitude() };
	Vector3 normal[3] = { dir[0] / length[0], dir[1] / length[1], dir[2] / length[2] };
	float theta[2] = { acos(Vector3::CosTheta(normal[0], normal[2])), acos(Vector3::CosTheta(-normal[0], normal[1])) };

	physx::PxSweepBuffer hit;
	physx::PxSphereGeometry sphere(ik.hitRadius);
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData(
		physx::PxFilterData(ik.filterWord0, 0, 0, 0), physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC);
	bool status = scene->m_PxScene->sweep(sphere, physx::PxTransform((physx::PxVec3&)pos[0].transformCoord(m_Actor->m_World)),
		(physx::PxVec3&)(normal[2].transformNormal(m_Actor->m_World).normalize()), length[2] * m_Actor->m_Scale.x, hit, physx::PxHitFlag::eDEFAULT, filterData);
	if (status && hit.block.distance > 0)
	{
		float local_dist = hit.block.distance / m_Actor->m_Scale.x; // ! m_Actor must be orthogonal Scale
		float new_theta[2] = {
			acos(Vector3::Cosine(length[1], length[0], local_dist)),
			acos(Vector3::Cosine(local_dist, length[1], length[0])) };
		Quaternion rot[2] = {
			Quaternion::RotationAxis(normal[2].cross(normal[0]), new_theta[0] - theta[0]),
			Quaternion::RotationAxis(normal[1].cross(normal[0]), new_theta[1] - theta[1]) };

		TransformHierarchyBoneList(anim_pose_hier, m_Skeleton->m_boneHierarchy,
			ik.id[0], rot[0], anim_pose_hier[ik.id[0]].m_position);

		TransformHierarchyBoneList(anim_pose_hier, m_Skeleton->m_boneHierarchy,
			ik.id[1], rot[1], anim_pose_hier[ik.id[1]].m_position);

		TransformHierarchyBoneList(anim_pose_hier, m_Skeleton->m_boneHierarchy,
			ik.id[2], rot[1].conjugate() * rot[0].conjugate(), anim_pose_hier[ik.id[2]].m_position);
	}
}

void Animator::TransformHierarchyBoneList(
	my::BoneList & boneList,
	const my::BoneHierarchy & boneHierarchy,
	int node_i,
	const my::Quaternion & Rotation,
	const my::Vector3 & Position)
{
	BoneHierarchy::const_reference node = boneHierarchy[node_i];
	BoneList::reference bone = boneList[node_i];
	bone.m_rotation *= Rotation;
	bone.m_position = Rotation * (bone.m_position - Position) + Position;

	int child_i = boneHierarchy[node_i].m_child;
	for (; child_i >= 0; child_i = boneHierarchy[child_i].m_sibling)
	{
		TransformHierarchyBoneList(boneList, boneHierarchy, child_i, Rotation, Position);
	}
}

void Animator::DrawDebugBone(my::DrawHelper * helper, D3DCOLOR color)
{
	_ASSERT(m_Actor);

	if (m_Skeleton)
	{
		my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
		for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
		{
			std::deque<int> stack;
			stack.push_back(*root_iter);
			while (!stack.empty())
			{
				int node_i = stack.back();
				stack.pop_back();
				int child_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
				for (; child_i >= 0; child_i = m_Skeleton->m_boneHierarchy[child_i].m_sibling)
				{
					helper->PushLine(anim_pose_hier[node_i].m_position.transformCoord(m_Actor->m_World),
						anim_pose_hier[child_i].m_position.transformCoord(m_Actor->m_World), color);
					stack.push_back(child_i);
				}
			}
		}
	}
}

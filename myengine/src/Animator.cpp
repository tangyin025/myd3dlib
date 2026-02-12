// Copyright (c) 2011-2024 tangyin025
// License: MIT
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
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/assign/list_of.hpp>

using namespace my;

BOOST_CLASS_EXPORT(AnimationNode)

BOOST_CLASS_EXPORT(AnimationNodeSequence)

BOOST_CLASS_EXPORT(AnimationNodeSlot)

BOOST_CLASS_EXPORT(AnimationNodeSubTree)

BOOST_CLASS_EXPORT(AnimationNodeBlendList)

BOOST_CLASS_EXPORT(Animator)

AnimationNode::~AnimationNode(void)
{
	_ASSERT(!m_Parent);

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		SetChild(i, AnimationNodePtr());
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

void AnimationNode::SetChild(int i, AnimationNodePtr node)
{
	if (i >= 0 && i < m_Childs.size())
	{
		if (m_Childs[i])
		{
			_ASSERT(m_Childs[i]->m_Parent == this);
			m_Childs[i]->m_Parent = NULL;
			m_Childs[i].reset();
		}

		if (node)
		{
			_ASSERT(!node->m_Parent);
			m_Childs[i] = node;
			node->m_Parent = this;
		}
		return;
	}
	THROW_CUSEXCEPTION("index overflow ");
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

const AnimationNode * AnimationNode::FindSubNode(const std::string & Name) const
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			if (m_Childs[i]->m_Name == Name)
			{
				return m_Childs[i].get();
			}

			AnimationNode* node = m_Childs[i]->FindSubNode(Name);
			if (node)
			{
				return node;
			}
		}
	}

	return NULL;
}

AnimationNode * AnimationNode::FindSubNode(const std::string & Name)
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			if (m_Childs[i]->m_Name == Name)
			{
				return m_Childs[i].get();
			}

			AnimationNode* node = m_Childs[i]->FindSubNode(Name);
			if (node)
			{
				return node;
			}
		}
	}

	return NULL;
}

AnimationNodeSequence::~AnimationNodeSequence(void)
{
	if (m_GroupOwner)
	{
		_ASSERT(false); m_GroupOwner->RemoveSequenceGroup(m_Group, this);
	}
}

void AnimationNodeSequence::Tick(float fElapsedTime, float fTotalWeight)
{
	m_TargetWeight = fTotalWeight;

	if (!m_Group.empty())
	{
		Animator* Root = dynamic_cast<Animator*>(GetTopNode());

		Animator::SequenceList::iterator seq_iter = std::lower_bound(Root->m_ActiveSequence.begin(), Root->m_ActiveSequence.end(),
			m_Group, boost::bind(std::less<std::string>(), boost::bind(&AnimationNodeSequence::m_Group, boost::placeholders::_1), boost::placeholders::_2));
		if (seq_iter == Root->m_ActiveSequence.end() || (*seq_iter)->m_Group != m_Group)
		{
			Root->m_ActiveSequence.insert(seq_iter, this);
		}
		else if ((*seq_iter)->m_TargetWeight < m_TargetWeight)
		{
			*seq_iter = this;
		}
	}
	else
	{
		Advance(fElapsedTime);
	}
}

void AnimationNodeSequence::Advance(float fElapsedTime)
{
	const float NewTime = m_Time + fElapsedTime * m_Rate;

	const float Length = GetLength();

	const float OldTime = m_Time;

	if (m_Rate >= 0)
	{
		if (NewTime > Length && m_Loop)
		{
			m_Time = fmodf(NewTime, Length);

			TriggerEvent(OldTime, Length);

			TriggerEvent(0, m_Time);
		}
		else
		{
			m_Time = NewTime;

			TriggerEvent(OldTime, m_Time);
		}
	}
	else
	{
		if (NewTime < 0 && m_Loop)
		{
			m_Time = Length + fmodf(NewTime, Length);

			TriggerEvent(0, OldTime);

			TriggerEvent(m_Time, Length);
		}
		else
		{
			m_Time = NewTime;

			TriggerEvent(m_Time, OldTime);
		}
	}
}

float AnimationNodeSequence::GetLength(void) const
{
	const Animator * Root = dynamic_cast<const Animator *>(GetTopNode());

	const OgreAnimation* anim;

	if (Root->m_Skeleton && (anim = Root->m_Skeleton->GetAnimation(m_Name)))
	{
		return anim->GetLength();
	}
	return FLT_MAX;
}

void AnimationNodeSequence::AddEvent(float time, unsigned int id)
{
	m_Events.insert(std::make_pair(time, id));
}

void AnimationNodeSequence::TriggerEvent(float begin, float end)
{
	EventMap::const_iterator evt_iter = m_Events.lower_bound(begin);

	if (evt_iter != m_Events.end() && evt_iter->first < end)
	{
		Animator* Root = dynamic_cast<Animator*>(GetTopNode());

		AnimationEventArg arg(Root->m_Actor, Root, this, evt_iter->second);
		Root->m_Actor->m_EventAnimation(&arg);
	}
}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	const Animator* Root = dynamic_cast<const Animator*>(GetTopNode());
	_ASSERT(Root->m_Skeleton);

	const OgreAnimation* anim = Root->m_Skeleton->GetAnimation(m_Name);
	if (anim)
	{
		anim->GetPose(pose, boneHierarchy, root_i, m_Time);
	}
	else
	{
		pose.assign(Root->m_Skeleton->m_boneBindPose.begin(), Root->m_Skeleton->m_boneBindPose.end());
	}
	return pose;
}

void AnimationNodeSlot::Tick(float fElapsedTime, float fTotalWeight)
{
	Animator * Root = dynamic_cast<Animator *>(GetTopNode());
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end();)
	{
		if (seq_iter->m_BlendTime <= fElapsedTime)
		{
			if (seq_iter->m_TargetWeight <= 0)
			{
				seq_iter = RemoveSlotIter(seq_iter);
				continue;
			}
			seq_iter->m_Weight = seq_iter->m_TargetWeight;
			seq_iter->m_BlendTime = 0;
		}
		else if (seq_iter->m_BlendTime > 0)
		{
			float WeightDelta = seq_iter->m_TargetWeight - seq_iter->m_Weight;
			seq_iter->m_Weight = seq_iter->m_Weight + WeightDelta * fElapsedTime / seq_iter->m_BlendTime;
			seq_iter->m_BlendTime -= fElapsedTime;
		}

		seq_iter->Tick(fElapsedTime, seq_iter->m_TargetWeight);

		if (seq_iter->m_TargetWeight > 0 /*&& !seq_iter->m_Loop*/ && seq_iter->m_Time >= seq_iter->GetLength())
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

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	int slot_root = m_NodeId >= 0 ? m_NodeId : root_i;

	_ASSERT(root_i == slot_root || boneHierarchy.IsChild(root_i, slot_root));

	bool init_pose = false;

	float fTotalWeight = 0.0f;

	int nPriority = INT_MAX;

	float fPriorityWeight = 1.0f;

	my::BoneList slot_pose(pose.size());
	SequenceList::const_iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		if (seq_iter->m_Weight > 0.0f)
		{
			if (seq_iter->m_Priority < nPriority)
			{
				nPriority = seq_iter->m_Priority;
				fPriorityWeight -= fTotalWeight;
				if (fPriorityWeight <= 0)
				{
					break;
				}
			}
			
			fTotalWeight += seq_iter->m_Weight * fPriorityWeight;

			if (!init_pose)
			{
				seq_iter->GetPose(slot_pose, slot_root, boneHierarchy);
				init_pose = true;
			}
			else
			{
				my::BoneList OtherPose(slot_pose.size());
				seq_iter->GetPose(OtherPose, slot_root, boneHierarchy);
				slot_pose.LerpSelf(OtherPose, boneHierarchy, slot_root, seq_iter->m_Weight * fPriorityWeight / fTotalWeight);
			}
		}
	}

	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose, root_i, boneHierarchy);

		if (init_pose)
		{
			pose.LerpSelf(slot_pose, boneHierarchy, slot_root, Min(1.0f, fTotalWeight));
		}
	}
	return pose;
}

void AnimationNodeSlot::Play(const std::string & Name, float Rate, float Weight, float BlendTime, float BlendOutTime, const std::string & Group, int Priority, DWORD_PTR UserData)
{
	// ! prevent infinite seqs for unactive slot
	if (m_SequenceSlot.size() >= 4)
	{
		SequenceList::reverse_iterator seq_riter = m_SequenceSlot.rbegin();
		for (; seq_riter != m_SequenceSlot.rend(); seq_riter++)
		{
			if (seq_riter->m_TargetWeight <= 0 && seq_riter->m_Weight <= 0)
			{
				RemoveSlotIter((++seq_riter).base());
				goto after_remove_seq;
			}
		}
		RemoveSlotIter((++m_SequenceSlot.rbegin()).base());
	}
after_remove_seq:

	SequenceList::iterator seq_iter = m_SequenceSlot.insert(std::lower_bound(m_SequenceSlot.begin(), m_SequenceSlot.end(), Priority,
		boost::bind(std::greater<float>(), boost::bind(&Sequence::m_Priority, boost::placeholders::_1), boost::placeholders::_2)), Sequence());
	if (seq_iter != m_SequenceSlot.end())
	{
		Animator* Root = dynamic_cast<Animator*>(GetTopNode());

		const OgreAnimation* anim;

		seq_iter->m_Time = Rate < 0 && Root->m_Skeleton && (anim = Root->m_Skeleton->GetAnimation(Name)) ? anim->GetLength() : 0;
		seq_iter->m_TargetWeight = Weight;
		seq_iter->m_Name = Name;
		seq_iter->m_Rate = Rate;
		seq_iter->m_Loop = false;
		seq_iter->m_Group = Group;
		seq_iter->m_Priority = Priority;
		seq_iter->m_BlendTime = BlendTime;
		seq_iter->m_BlendOutTime = BlendOutTime;
		seq_iter->m_Weight = 0;
		seq_iter->m_UserData = UserData;
		seq_iter->m_Parent = this;

		if (!seq_iter->m_Group.empty())
		{
			Root->AddSequenceGroup(seq_iter->m_Group, &*seq_iter);
		}
	}
}

AnimationNodeSlot::SequenceList::iterator AnimationNodeSlot::RemoveSlotIter(SequenceList::iterator iter)
{
	if (iter->m_GroupOwner)
	{
		iter->m_GroupOwner->RemoveSequenceGroup(iter->m_Group, &*iter);
	}
	return m_SequenceSlot.erase(iter);
}

void AnimationNodeSlot::StopSlotIter(SequenceList::iterator iter, float BlendOutTime)
{
	if (iter->m_TargetWeight > 0 || iter->m_BlendTime > BlendOutTime)
	{
		iter->m_TargetWeight = 0;
		iter->m_BlendTime = my::Min(BlendOutTime, iter->m_BlendOutTime);
	}
}

void AnimationNodeSlot::StopSlotByUserData(DWORD_PTR UserData, float BlendOutTime)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		if (seq_iter->m_UserData == UserData)
		{
			StopSlotIter(seq_iter, BlendOutTime);
		}
	}
}

bool AnimationNodeSlot::IsPlaying(void) const
{
	return !m_SequenceSlot.empty() && m_SequenceSlot.back().m_TargetWeight != 0;
}

void AnimationNodeSlot::StopAllSlot(float BlendOutTime)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		StopSlotIter(seq_iter, BlendOutTime);
	}
}

void AnimationNodeSubTree::Tick(float fElapsedTime, float fTotalWeight)
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->Tick(fElapsedTime, fTotalWeight);
		}
	}
}

my::BoneList & AnimationNodeSubTree::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	if (m_Childs[0])
	{
		if (m_NodeId >= 0)
		{
			if (m_boneHierarchy.empty())
			{
				const_cast<my::BoneHierarchy&>(m_boneHierarchy).resize(boneHierarchy.size());
				boneHierarchy.BuildLeafedHierarchy(const_cast<my::BoneHierarchy&>(m_boneHierarchy), root_i, boost::assign::list_of(m_NodeId));
			}

			m_Childs[0]->GetPose(pose, root_i, m_boneHierarchy);
		}
		else
		{
			m_Childs[0]->GetPose(pose, root_i, boneHierarchy);
		}
	}

	if (m_Childs[1] && m_NodeId >= 0)
	{
		m_Childs[1]->GetPose(pose, m_NodeId, boneHierarchy);
	}
	return pose;
}

AnimationNodeBlendList::AnimationNodeBlendList(const char * Name, unsigned int ChildNum)
	: AnimationNode(Name, ChildNum)
	, m_BlendTime(0)
	, m_Weight(ChildNum, 0.0f)
	, m_TargetWeight(boost::assign::list_of(1.0f).repeat(ChildNum - 1, 0.0f))
{
}

void AnimationNodeBlendList::SetTargetWeight(int Child, float Weight)
{
	if (Child >= 0 && Child < m_Childs.size())
	{
		m_TargetWeight[Child] = Weight;
	}
}

void AnimationNodeBlendList::SetTargetWeight(int Child, float Weight, bool Exclusive)
{
	SetTargetWeight(Child, Weight);

	if (Exclusive)
	{
		float ExclusiveWeight = my::Max(0.0f, 1 - Weight);
		for (int i = 0; i < m_Childs.size(); i++)
		{
			if (i != Child)
			{
				m_TargetWeight[i] *= ExclusiveWeight;
			}
		}
	}
}

float AnimationNodeBlendList::GetTargetWeight(int Child)
{
	if (Child >= 0 && Child < m_Childs.size())
	{
		return m_TargetWeight[Child];
	}
	return 0;
}

void AnimationNodeBlendList::SetActiveChild(int ActiveChild, float BlendTime)
{
	if (ActiveChild >= 0 && ActiveChild < m_Childs.size() && m_TargetWeight[ActiveChild] < 1.0f)
	{
		SetTargetWeight(ActiveChild, 1.0f, true);

		m_BlendTime = BlendTime;
	}
}

int AnimationNodeBlendList::GetActiveChild(void) const
{
	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_TargetWeight[i] > 0.99f)
		{
			return i;
		}
	}
	return 0;
}

void AnimationNodeBlendList::Tick(float fElapsedTime, float fTotalWeight)
{
	if (m_BlendTime < fElapsedTime + EPSILON_E12)
	{
		boost::range::copy(m_TargetWeight, m_Weight.begin());

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
			m_Childs[i]->Tick(fElapsedTime, m_TargetWeight[i] * fTotalWeight);
		}
	}
}

my::BoneList & AnimationNodeBlendList::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	bool init_pose = false;

	float fTotalWeight = 0.0f;

	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_Childs[i] && m_Weight[i] > 0.0f)
		{
			fTotalWeight += m_Weight[i];

			if (!init_pose)
			{
				m_Childs[i]->GetPose(pose, root_i, boneHierarchy);
				init_pose = true;
			}
			else
			{
				my::BoneList OtherPose(pose.size());
				m_Childs[i]->GetPose(OtherPose, root_i, boneHierarchy);
				pose.LerpSelf(OtherPose, boneHierarchy, root_i, m_Weight[i] / fTotalWeight);
			}
		}
	}
	_ASSERT(init_pose); // ! not all of m_Weight and m_Rate are zero
	return pose;
}

template<class Archive>
void Animator::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
	ar << BOOST_SERIALIZATION_NVP(m_SkeletonPath);
	ar << BOOST_SERIALIZATION_NVP(m_RootBone);
	ar << BOOST_SERIALIZATION_NVP(m_DynamicBones);
}

template<class Archive>
void Animator::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
	ar >> BOOST_SERIALIZATION_NVP(m_SkeletonPath);
	ar >> BOOST_SERIALIZATION_NVP(m_RootBone);
	ar >> BOOST_SERIALIZATION_NVP(m_DynamicBones);
	ReloadSequenceGroup();
}

Animator::~Animator(void)
{
	if (IsRequested())
	{
		_ASSERT(false); ReleaseResource();
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

	bind_pose.resize(m_Skeleton->m_boneBindPose.size());
	my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
	for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
	{
		m_Skeleton->m_boneBindPose.FlatHierarchyBoneList(
			bind_pose, m_Skeleton->m_boneHierarchy, *root_iter, Bone(Vector3(0, 0, 0)));
	}
	anim_pose_hier.resize(m_Skeleton->m_boneBindPose.size(), Bone(Vector3(0, 0, 0)));
	anim_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Vector3(0, 0, 0)));
	final_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Vector3(0, 0, 0)));
}

void Animator::RequestResource(void)
{
	Component::RequestResource();

	if (!m_SkeletonPath.empty())
	{
		_ASSERT(!m_Skeleton);

		my::ResourceMgr::getSingleton().LoadSkeletonAsync(m_SkeletonPath.c_str(), boost::bind(&Animator::OnSkeletonReady, this, boost::placeholders::_1), (m_LodMask & LOD0) ? ResPriorityLod0 : (m_LodMask & LOD1) ? ResPriorityLod1 : ResPriorityLod2);
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
	Tick(fElapsedTime, 1.0f);
}

void Animator::Tick(float fElapsedTime, float fTotalWeight)
{
	if (m_Childs[0])
	{
		m_ActiveSequence.clear();

		m_Childs[0]->Tick(fElapsedTime, fTotalWeight);

		UpdateSequenceGroup(fElapsedTime);

		if (m_Skeleton)
		{
			my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
			for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
			{
				GetPose(anim_pose_hier, *root_iter, m_Skeleton->m_boneHierarchy);

				UpdateHierarchyBoneList(*root_iter, m_RootBone);
			}

			DynamicBoneContextMap::iterator db_iter = m_DynamicBones.begin();
			for (; db_iter != m_DynamicBones.end(); db_iter++)
			{
				if (db_iter->second.parent_i < 0)
				{
					db_iter->second.parent_i = m_Skeleton->m_boneHierarchy.FindParent(db_iter->first);
				}

				int particle_i = 0;
				UpdateDynamicBone(db_iter->second, anim_pose[db_iter->second.parent_i],
					anim_pose[db_iter->second.parent_i].m_position.transformCoord(m_Actor->m_World), db_iter->first, particle_i, fElapsedTime);
			}

			IKContextMap::iterator ik_iter = m_Iks.begin();
			for (; ik_iter != m_Iks.end(); ik_iter++)
			{
				UpdateIK(ik_iter->second);
			}

			for (size_t i = 0; i < bind_pose.size(); i++)
			{
				final_pose[i].m_rotation = bind_pose[i].m_rotation.conjugate() * anim_pose[i].m_rotation;
				final_pose[i].m_position = final_pose[i].m_rotation * -bind_pose[i].m_position + anim_pose[i].m_position;
			}

			m_DualQuats.resize(m_Skeleton->m_boneBindPose.size());
			final_pose.BuildDualQuaternionList(m_DualQuats);
		}
	}
}

my::BoneList & Animator::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose, root_i, boneHierarchy);
	}
	return pose;
}

void Animator::UpdateHierarchyBoneList(const int node_i, const my::Bone& parent)
{
	const Bone& src = anim_pose_hier[node_i];
	Bone& dst = anim_pose[node_i];
	Actor::AttachList::iterator act_iter = boost::find_if(m_Actor->m_Attaches, boost::bind(std::equal_to<int>(), node_i, boost::bind(&Actor::m_BaseBoneId, boost::placeholders::_1)));
	if (act_iter != m_Actor->m_Attaches.end() && (*act_iter)->m_PxActor && !(*act_iter)->GetRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC))
	{
		physx::PxRigidBody* body = (*act_iter)->m_PxActor->is<physx::PxRigidBody>();
		_ASSERT(body);
		physx::PxTransform pose = body->getGlobalPose();
		dst.m_rotation = (my::Quaternion&)pose.q * m_Actor->m_Rotation.conjugate();
		dst.m_position = ((my::Vector3&)pose.p).transformCoord(m_Actor->m_World.inverse());
	}
	else
	{
		dst = src.Transform(parent);
	}

	int child_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; child_i >= 0; child_i = m_Skeleton->m_boneHierarchy[child_i].m_sibling)
	{
		UpdateHierarchyBoneList(child_i, dst);
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

void Animator::UpdateSequenceGroup(float fElapsedTime)
{
	Animator* const BaseAnimator = m_Actor->m_Base ? m_Actor->m_Base->GetFirstComponent<Animator>() : NULL;

	SequenceGroupMap::iterator seq_iter = m_SequenceGroup.begin();
	while (seq_iter != m_SequenceGroup.end())
	{
		SequenceGroupMap::iterator next_seq_iter = m_SequenceGroup.upper_bound(seq_iter->first);

		SequenceList::iterator master_seq_iter;
		if (BaseAnimator && (master_seq_iter = boost::find_if(BaseAnimator->m_ActiveSequence, boost::bind(
			std::equal_to<std::string>(), boost::bind(&AnimationNodeSequence::m_Group, boost::placeholders::_1), seq_iter->second->m_Group))) != BaseAnimator->m_ActiveSequence.end())
		{
			SyncSequenceGroupTime(seq_iter, next_seq_iter, *master_seq_iter);
		}
		else if ((master_seq_iter = boost::find_if(m_ActiveSequence, boost::bind(
			std::equal_to<std::string>(), boost::bind(&AnimationNodeSequence::m_Group, boost::placeholders::_1), seq_iter->second->m_Group))) != m_ActiveSequence.end())
		{
			(*master_seq_iter)->Advance(fElapsedTime);

			SyncSequenceGroupTime(seq_iter, next_seq_iter, *master_seq_iter);
		}

		seq_iter = next_seq_iter;
	}
}

void Animator::SyncSequenceGroupTime(SequenceGroupMap::iterator begin, SequenceGroupMap::iterator end, const AnimationNodeSequence * master)
{
	SequenceGroupMap::iterator seq_iter = begin;
	for (; seq_iter != end; seq_iter++)
	{
		//_ASSERT(seq_iter->second != master);

		seq_iter->second->m_Time = master->m_Time;
	}
}

void Animator::AddDynamicBone(int node_i, float mass, float damping, float springConstant)
{
	_ASSERT(mass > 0);

	std::pair<DynamicBoneContextMap::iterator, bool> res = m_DynamicBones.insert(std::make_pair(node_i, DynamicBoneContext()));
	if (res.second)
	{
		res.first->second.parent_i = -1;
		res.first->second.springConstant = springConstant;
	}

	_ASSERT(res.first->second.parent_i == -1);
	_ASSERT(res.first->second.springConstant == springConstant);
	res.first->second.m_ParticleList.push_back(Particle(
		Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 1 / mass, damping));
}

void Animator::UpdateDynamicBone(DynamicBoneContext & context, const my::Bone & parent, const my::Vector3& parent_world_pos, int node_i, int & particle_i, float fElapsedTime)
{
	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	Bone target(
		parent.m_rotation * m_Skeleton->m_boneBindPose[node_i].m_position + parent.m_position,
		m_Skeleton->m_boneBindPose[node_i].m_rotation * parent.m_rotation);
	Vector3 target_world_pos = target.m_position.transformCoord(m_Actor->m_World);

	if (context.m_ParticleList.size() <= particle_i)
	{
		context.m_ParticleList.resize(particle_i + 1, context.m_ParticleList.back());
	}

	Particle & particle = context.m_ParticleList[particle_i];
	particle.clearAccumulator();
	particle.setAcceleration(Vector3::Gravity);
	Vector3 distance = target_world_pos - particle.getPosition();
	float length = distance.magnitude();
	Vector3 force = fabs(length) > EPSILON_E6 ? distance.normalize() * (-context.springConstant * length) : Vector3(0, 0, 0);
	particle.addForce(force);
	particle.integrate(fElapsedTime);
	Vector3 d0 = target_world_pos - parent_world_pos;
	Vector3 d1 = particle.getPosition() - parent_world_pos;
	particle.setPosition(parent_world_pos + d1.normalize() * d0.magnitude());

	anim_pose[node_i].m_rotation =
		target.m_rotation * m_Actor->m_Rotation * Quaternion::RotationFromToSafe(d0, d1) * m_Actor->m_Rotation.conjugate();

	anim_pose[node_i].m_position =
		particle.getPosition().transformCoord(m_Actor->m_World.inverse());

	particle_i++;
	int child_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; child_i >= 0; child_i = m_Skeleton->m_boneHierarchy[child_i].m_sibling)
	{
		UpdateDynamicBone(context, anim_pose[node_i], particle.getPosition(), child_i, particle_i, fElapsedTime);
	}
}

void Animator::AddIK(int node_i, float hitRadius, unsigned int filterWord0)
{
	std::pair<IKContextMap::iterator, bool> res = m_Iks.insert(std::make_pair(node_i, IKContext()));
	if (!res.second)
	{
		return;
	}

	res.first->second.id = node_i;
	res.first->second.hitRadius = hitRadius;
	res.first->second.filterWord0 = filterWord0;
}

void Animator::UpdateIK(IKContext & ik)
{
	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	PhysxScene * scene = dynamic_cast<PhysxScene *>(m_Actor->m_Node->GetTopNode());
	_ASSERT(scene);

	const int ids[3] = {
		ik.id,
		m_Skeleton->m_boneHierarchy[ik.id].m_child,
		m_Skeleton->m_boneHierarchy[m_Skeleton->m_boneHierarchy[ik.id].m_child].m_child };

	const Vector3 pos[3] = {
		anim_pose[ids[0]].m_position,
		anim_pose[ids[1]].m_position,
		anim_pose[ids[2]].m_position };
	const Vector3 dir[3] = { pos[1] - pos[0], pos[2] - pos[1], pos[2] - pos[0] };
	const float length[3] = { dir[0].magnitude(), dir[1].magnitude(), dir[2].magnitude() };
	if (length[0] < EPSILON_E12 || length[1] < EPSILON_E12 || length[2] < EPSILON_E12)
	{
		return;
	}

	const Vector3 normal[3] = { dir[0] / length[0], dir[1] / length[1], dir[2] / length[2] };
	const float theta[2] = { normal[0].angle(normal[2]), (-normal[0]).angle(normal[1]) };
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
			acos(my::Clamp(Vector3::Cosine(length[1], length[0], local_dist), -1.0f, 1.0f)),
			acos(my::Clamp(Vector3::Cosine(local_dist, length[1], length[0]), -1.0f, 1.0f)) };
		Quaternion rot[2] = {
			Quaternion::RotationAxis(normal[2].cross(normal[0]), new_theta[0] - theta[0]),
			Quaternion::RotationAxis(normal[1].cross(normal[0]), new_theta[1] - theta[1]) };

		TransformHierarchyBoneList(anim_pose, m_Skeleton->m_boneHierarchy,
			ids[0], rot[0], anim_pose[ids[0]].m_position);

		TransformHierarchyBoneList(anim_pose, m_Skeleton->m_boneHierarchy,
			ids[1], rot[1], anim_pose[ids[1]].m_position);

		TransformHierarchyBoneList(anim_pose, m_Skeleton->m_boneHierarchy,
			ids[2], rot[1].conjugate() * rot[0].conjugate(), anim_pose[ids[2]].m_position);
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
					helper->PushLine(anim_pose[node_i].m_position.transformCoord(m_Actor->m_World),
						anim_pose[child_i].m_position.transformCoord(m_Actor->m_World), color);
					stack.push_back(child_i);
				}
			}
		}
	}
}

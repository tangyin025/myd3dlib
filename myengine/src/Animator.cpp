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
#include <boost/assign/list_of.hpp>

using namespace my;

BOOST_CLASS_EXPORT(AnimationNode)

BOOST_CLASS_EXPORT(AnimationNodeSequence)

BOOST_CLASS_EXPORT(AnimationNodeSlot)

BOOST_CLASS_EXPORT(AnimationNodeSubTree)

BOOST_CLASS_EXPORT(AnimationNodeBlendList)

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

AnimationNodePtr AnimationNode::GetChild(int i) const
{
	if (i >= 0 && i < m_Childs.size())
	{
		return m_Childs[i];
	}
	return AnimationNodePtr();
}

void AnimationNode::SetChild(int i, AnimationNodePtr node)
{
	if (i >= 0 && i < m_Childs.size())
	{
		if (m_Childs[i])
		{
			RemoveChild(i);
		}

		m_Childs[i] = node;

		node->m_Parent = this;
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
	_ASSERT(Root->m_Skeleton);

	const OgreAnimation* anim = Root->m_Skeleton->GetAnimation(m_Name);
	if (anim)
	{
		return anim->GetLength();
	}
	return 1.0f;
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

		if (seq_iter->m_RootId < 0)
		{
			fTotalWeight = Max(0.0f, fTotalWeight - Weight);
		}

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

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose, root_i, boneHierarchy);
	}

	SequenceList::const_iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		my::BoneList OtherPose(pose.size());
		seq_iter->GetPose(OtherPose, root_i, boneHierarchy);
		if (seq_iter->m_RootId < 0)
		{
			pose.LerpSelf(OtherPose, boneHierarchy, root_i, seq_iter->m_Weight);
		}
		else
		{
			pose.LerpSelf(OtherPose, boneHierarchy, seq_iter->m_RootId, seq_iter->m_Weight);
		}
	}
	return pose;
}

void AnimationNodeSlot::Play(const std::string & Name, float Rate, float Weight, float BlendTime, float BlendOutTime, bool Loop, int Priority, const std::string & Group, int RootId, DWORD_PTR UserData)
{
	SequenceList::iterator seq_iter = std::upper_bound(m_SequenceSlot.begin(), m_SequenceSlot.end(), Priority,
		boost::bind(std::less<float>(), boost::placeholders::_1, boost::bind(&Sequence::m_Priority, boost::placeholders::_2)));

	if (seq_iter != m_SequenceSlot.begin() or !m_SequenceSlot.full())
	{
		SequenceList::iterator res_seq_iter = m_SequenceSlot.insert(seq_iter);
		res_seq_iter->m_Time = 0;
		res_seq_iter->m_Weight = 0;
		res_seq_iter->m_Name = Name;
		res_seq_iter->m_Rate = Rate;
		res_seq_iter->m_Loop = Loop;
		res_seq_iter->m_Group = Group;
		res_seq_iter->m_Priority = Priority;
		res_seq_iter->m_BlendTime = BlendTime;
		res_seq_iter->m_BlendOutTime = BlendOutTime;
		res_seq_iter->m_TargetWeight = Weight;
		res_seq_iter->m_RootId = RootId;
		res_seq_iter->m_UserData = UserData;
		res_seq_iter->m_Parent = this;

		if (!Group.empty())
		{
			Animator* Root = dynamic_cast<Animator*>(GetTopNode());
			Root->AddSequenceGroup(Group, &*res_seq_iter);
			Root->SyncSequenceGroupTime(Group, res_seq_iter->m_Time / res_seq_iter->GetLength());
		}
	}
}

void AnimationNodeSlot::StopIndex(int i)
{
	_ASSERT(i >= 0 && i < (int)m_SequenceSlot.size());

	m_SequenceSlot[i].m_TargetWeight = 0;
	m_SequenceSlot[i].m_BlendTime = m_SequenceSlot[i].m_BlendOutTime;

	if (!m_SequenceSlot[i].m_Group.empty())
	{
		Animator* Root = dynamic_cast<Animator*>(GetTopNode());
		Root->RemoveSequenceGroup(m_SequenceSlot[i].m_Group, &m_SequenceSlot[i]);
	}
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

AnimationNodeBlendList::AnimationNodeBlendList(unsigned int ChildNum)
	: AnimationNode(ChildNum)
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
		if (m_Childs[i])
		{
			m_Childs[i]->Tick(fElapsedTime, m_TargetWeight[i] * fTotalWeight);
		}
	}
}

my::BoneList & AnimationNodeBlendList::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	bool init_pose = false;

	for (int i = 0; i < m_TargetWeight.size(); i++)
	{
		if (m_Childs[i] && m_Weight[i] > 0.0f)
		{
			if (!init_pose)
			{
				m_Childs[i]->GetPose(pose, root_i, boneHierarchy);
				init_pose = true;
			}
			else
			{
				my::BoneList OtherPose(pose.size());
				m_Childs[i]->GetPose(OtherPose, root_i, boneHierarchy);
				pose.LerpSelf(OtherPose, boneHierarchy, root_i, m_Weight[i]);
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

my::BoneList & AnimationNodeRate::GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const
{
	if (m_Childs[0])
	{
		return m_Childs[0]->GetPose(pose, root_i, boneHierarchy);
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
	anim_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Vector3(0, 0, 0)));
	final_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Vector3(0, 0, 0)));
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
		my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
		for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
		{
			GetPose(anim_pose, *root_iter, m_Skeleton->m_boneHierarchy);

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

void Animator::Tick(float fElapsedTime, float fTotalWeight)
{
	if (m_Skeleton)
	{
		AnimationNodeSlot::Tick(fElapsedTime, fTotalWeight);

		UpdateSequenceGroup();
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
		parent.m_rotation * m_Skeleton->m_boneBindPose[node_i].m_position + parent.m_position,
		m_Skeleton->m_boneBindPose[node_i].m_rotation * parent.m_rotation);
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

	const Vector3 pos[3] = {
		anim_pose_hier[ik.id[0]].m_position,
		anim_pose_hier[ik.id[1]].m_position,
		anim_pose_hier[ik.id[2]].m_position };
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

#include "Animation.h"
#include "Actor.h"
#include "myResource.h"
#include "PhysxContext.h"
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

void AnimationNode::RemoveChild(unsigned int i)
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
	m_RootList = rhs.m_RootList;
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

void AnimationNodeSequence::SetRootList(std::string RootList)
{
	boost::trim_if(RootList, boost::algorithm::is_any_of(" \t,"));
	if (!RootList.empty())
	{
		boost::algorithm::split(m_RootList, RootList, boost::algorithm::is_any_of(" \t,"), boost::algorithm::token_compress_on);
	}
	else
	{
		m_RootList.clear();
	}
}

std::string AnimationNodeSequence::GetRootList(void) const
{
	return std::string();
}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose) const
{
	const Animator * Root = dynamic_cast<const Animator *>(GetTopNode());
	if (Root->m_Skeleton)
	{
		const OgreAnimation * anim = Root->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			if (m_RootList.empty())
			{
				BoneIndexSet::const_iterator root_iter = Root->m_Skeleton->m_boneRootSet.begin();
				for (; root_iter != Root->m_Skeleton->m_boneRootSet.end(); root_iter++)
				{
					anim->GetPose(pose, Root->m_Skeleton->m_boneHierarchy, *root_iter, m_Time);
				}
			}
			else
			{
				std::vector<std::string>::const_iterator root_name_iter = m_RootList.begin();
				for (; root_name_iter != m_RootList.end(); root_name_iter++)
				{
					OgreSkeleton::BoneNameMap::const_iterator root_iter = Root->m_Skeleton->m_boneNameMap.find(*root_name_iter);
					if (root_iter != Root->m_Skeleton->m_boneNameMap.end())
					{
						anim->GetPose(pose, Root->m_Skeleton->m_boneHierarchy, root_iter->second, m_Time);
					}
				}
			}
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

				if (m_SequenceSlot.empty())
				{
					m_Priority = INT_MIN;
				}

				continue;
			}
			Weight = seq_iter->m_TargetWeight;
			seq_iter->m_BlendTime = 0;
		}
		else
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

void AnimationNodeSlot::Play(const std::string & Name, std::string RootList, float Rate, float BlendTime, float BlendOutTime, bool Loop, int Prority, float StartTime, const std::string & Group, DWORD_PTR UserData)
{
	if (Prority >= m_Priority)
	{
		m_Priority = Prority;
		Sequence seq;
		seq.m_Time = StartTime;
		seq.m_Weight = 0;
		seq.m_Name = Name;
		seq.SetRootList(RootList);
		seq.m_Rate = Rate;
		seq.m_Loop = Loop;
		seq.m_Group = Group;
		seq.m_BlendTime = BlendTime;
		seq.m_BlendOutTime = BlendOutTime;
		seq.m_TargetWeight = 1.0f;
		seq.m_UserData = UserData;
		seq.m_Parent = this;
		m_SequenceSlot.push_front(seq);

		if (!Group.empty())
		{
			Animator * Root = dynamic_cast<Animator *>(GetTopNode());
			Root->AddSequenceGroup(Group, &m_SequenceSlot.front());
			Root->SyncSequenceGroupTime(Group, seq.m_Time / seq.GetLength());
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

void AnimationNodeBlend::SetActiveChild(unsigned int ActiveChild, float BlendTime)
{
	_ASSERT(ActiveChild < m_Childs.size());
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

	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime, fTotalWeight * (1 - m_TargetWeight));
	}

	if (m_Childs[1])
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

	if (m_Weight >= 1.0f)
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

	my::BoneList OtherPose(pose.size());
	if (m_Childs[1])
	{
		m_Childs[1]->GetPose(OtherPose);
	}

	for (unsigned int i = 0; i < pose.size(); i++)
	{
		pose[i].LerpSelf(OtherPose[i], m_Weight);
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
	if (!m_SkeletonPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_SkeletonPath, boost::bind(&Animator::OnSkeletonReady, this, boost::placeholders::_1));

		m_Skeleton.reset();
	}

	Component::ReleaseResource();
}

void Animator::Update(float fElapsedTime)
{
	if (m_Skeleton)
	{
		Tick(fElapsedTime, 1.0f);

		UpdateSequenceGroup();

		GetPose(anim_pose);

		my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
		for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
		{
			anim_pose.BuildHierarchyBoneList(
				anim_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion::Identity(), Vector3(0, 0, 0));
		}

		JiggleBoneContextMap::iterator jb_iter = m_JiggleBones.begin();
		for (; jb_iter != m_JiggleBones.end(); jb_iter++)
		{
			int particle_i = 0;
			UpdateJiggleBone(jb_iter->second, anim_pose_hier[jb_iter->second.root_i],
				anim_pose_hier[jb_iter->second.root_i].m_position.transformCoord(m_Actor->m_World), jb_iter->first, particle_i, fElapsedTime);
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

void Animator::AddJiggleBone(int node_i, const my::BoneHierarchy & boneHierarchy, float mass, float damping, float springConstant)
{
	_ASSERT(mass > 0);

	int root_i = boneHierarchy.FindParent(node_i);
	if (-1 == root_i)
	{
		return;
	}

	std::pair<JiggleBoneContextMap::iterator, bool> res = m_JiggleBones.insert(std::make_pair(node_i, JiggleBoneContext()));
	if (!res.second)
	{
		return;
	}

	res.first->second.root_i = root_i;
	res.first->second.springConstant = springConstant;
	AddJiggleBone(res.first->second, node_i, boneHierarchy, mass, damping);
}

void Animator::AddJiggleBone(JiggleBoneContext & context, int node_i, const my::BoneHierarchy & boneHierarchy, float mass, float damping)
{
	context.m_ParticleList.push_back(Particle(
		Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 1 / mass, damping));
	node_i = boneHierarchy[node_i].m_child;
	for (; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		AddJiggleBone(context, node_i, boneHierarchy, mass, damping);
	}
}

void Animator::UpdateJiggleBone(JiggleBoneContext & context, const my::Bone & parent, const my::Vector3& parent_world_pos, int node_i, int & particle_i, float fElapsedTime)
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
	int sub_node_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; sub_node_i >= 0; sub_node_i = m_Skeleton->m_boneHierarchy[sub_node_i].m_sibling)
	{
		UpdateJiggleBone(context, anim_pose_hier[node_i], particle.getPosition(), sub_node_i, particle_i, fElapsedTime);
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
	int root_i,
	const my::Quaternion & Rotation,
	const my::Vector3 & Position)
{
	BoneHierarchy::const_reference node = boneHierarchy[root_i];
	BoneList::reference bone = boneList[root_i];
	bone.m_rotation *= Rotation;
	bone.m_position = Rotation * (bone.m_position - Position) + Position;

	int node_i = boneHierarchy[root_i].m_child;
	for (; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		TransformHierarchyBoneList(boneList, boneHierarchy, node_i, Rotation, Position);
	}
}

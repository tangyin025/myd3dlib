#include "Animator.h"
#include "Character.h"
#include "myResource.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Animator)

template<>
void Animator::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_SkeletonPath);
	ar << BOOST_SERIALIZATION_NVP(m_Node);
}

template<>
void Animator::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_SkeletonPath);
	ar >> BOOST_SERIALIZATION_NVP(m_Node);
	m_Node->m_Owner = this;
	m_Node->OnSetOwner();
}

void Animator::OnReady(my::DeviceResourceBasePtr res)
{
	m_Skeleton = boost::dynamic_pointer_cast<my::OgreSkeletonAnimation>(res);

	if (m_SkeletonEventReady)
	{
		m_SkeletonEventReady(&my::ControlEventArgs(NULL));
	}

	bind_pose_hier.resize(m_Skeleton->m_boneBindPose.size());
	anim_pose_hier.resize(m_Skeleton->m_boneBindPose.size());
	my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
	for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
	{
		m_Skeleton->m_boneBindPose.BuildHierarchyBoneList(
			bind_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion(0, 0, 0, 1), Vector3(0, 0, 0));

		m_Skeleton->m_boneBindPose.BuildHierarchyBoneList(
			anim_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, Quaternion(0, 0, 0, 1), Vector3(0, 0, 0));
	}
	anim_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Quaternion::Identity(), Vector3(0, 0, 0)));
	final_pose.resize(m_Skeleton->m_boneBindPose.size(), Bone(Quaternion::Identity(), Vector3(0, 0, 0)));
}

void Animator::RequestResource(void)
{
	if (!m_SkeletonPath.empty())
	{
		_ASSERT(!m_Skeleton);

		my::ResourceMgr::getSingleton().LoadSkeletonAsync(m_SkeletonPath.c_str(), this);
	}
}

void Animator::ReleaseResource(void)
{
	if (!m_SkeletonPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_SkeletonPath, this);

		m_Skeleton.reset();
	}
}

void Animator::Update(float fElapsedTime)
{
	if (m_Skeleton && m_Node)
	{
		m_Node->Tick(fElapsedTime, 1.0f);

		UpdateGroup(fElapsedTime);

		m_Node->GetPose(anim_pose);
		my::BoneIndexSet::const_iterator root_iter = m_Skeleton->m_boneRootSet.begin();
		for (; root_iter != m_Skeleton->m_boneRootSet.end(); root_iter++)
		{
			anim_pose.IncrementSelf(
				m_Skeleton->m_boneBindPose, m_Skeleton->m_boneHierarchy, *root_iter);

			anim_pose.BuildHierarchyBoneList(
				anim_pose_hier, m_Skeleton->m_boneHierarchy, *root_iter, m_Actor->m_Rotation, m_Actor->m_Position);
		}

		JiggleBoneContextMap::iterator context_iter = m_JiggleBones.begin();
		for (; context_iter != m_JiggleBones.end(); context_iter++)
		{
			int particle_i = 0;
			UpdateJiggleBone(context_iter->second, context_iter->second.root_i, context_iter->first, particle_i, fElapsedTime);
		}

		for (size_t i = 0; i < bind_pose_hier.size(); i++)
		{
			final_pose[i].m_rotation = bind_pose_hier[i].m_rotation.conjugate() * anim_pose_hier[i].m_rotation;
			final_pose[i].m_position = (-bind_pose_hier[i].m_position).transform(final_pose[i].m_rotation) + anim_pose_hier[i].m_position;
		}

		m_DualQuats.resize(m_Skeleton->m_boneBindPose.size());
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
		if (master_seq_iter->second->m_Weight > EPSILON_E3)
		{
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

int Animator::AddJiggleBone(const std::string & bone_name, float mass, float damping, float springConstant)
{
	_ASSERT(mass > 0);

	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	OgreSkeleton::BoneNameMap::const_iterator bone_name_iter = m_Skeleton->m_boneNameMap.find(bone_name);
	if (bone_name_iter == m_Skeleton->m_boneNameMap.end())
	{
		return -1;
	}

	int root_i = m_Skeleton->FindParent(bone_name_iter->second);
	if (-1 == root_i)
	{
		return -1;
	}

	JiggleBoneContext & context = m_JiggleBones[bone_name_iter->second];
	context.root_i = root_i;
	context.springConstant = springConstant;
	AddJiggleBone(context, bone_name_iter->second, mass, damping);
	return root_i;
}

void Animator::AddJiggleBone(JiggleBoneContext & context, int node_i, float mass, float damping)
{
	context.m_ParticleList.push_back(Particle(
		Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 1 / mass, damping));
	node_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; node_i >= 0; node_i = m_Skeleton->m_boneHierarchy[node_i].m_sibling)
	{
		AddJiggleBone(context, node_i, mass, damping);
	}
}

void Animator::UpdateJiggleBone(JiggleBoneContext & context, int parent_i, int node_i, int & particle_i, float fElapsedTime)
{
	const Bone & parent = anim_pose_hier[parent_i];
	Bone target(
		m_Skeleton->m_boneBindPose[node_i].m_rotation * parent.GetRotation(),
		m_Skeleton->m_boneBindPose[node_i].m_position.transform(parent.GetRotation()) + parent.GetPosition());
	Particle & particle = context.m_ParticleList[particle_i];
	particle.clearAccumulator();
	particle.setAcceleration(Vector3::Gravity);
	Vector3 distance = target.GetPosition() - particle.getPosition();
	float length = distance.magnitude();
	Vector3 direction = distance.normalize();
	Vector3 force = fabs(length) > EPSILON_E6 ? direction * (-context.springConstant * length) : Vector3(0, 0, 0);
	particle.addForce(force);
	particle.integrate(fElapsedTime);
	Vector3 d0 = target.GetPosition() - parent.GetPosition();
	Vector3 d1 = particle.getPosition() - parent.GetPosition();
	particle.setPosition(parent.GetPosition() + d1.normalize() * d0.magnitude());
	anim_pose_hier[node_i].SetPosition(particle.getPosition());
	anim_pose_hier[node_i].SetRotation(target.GetRotation() * Quaternion::RotationFromTo(d0, d1));

	particle_i++;
	parent_i = node_i;
	node_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; node_i >= 0; node_i = m_Skeleton->m_boneHierarchy[node_i].m_sibling)
	{
		UpdateJiggleBone(context, parent_i, node_i, particle_i, fElapsedTime);
	}
}

BOOST_CLASS_EXPORT(AnimationNode)

void AnimationNode::OnSetOwner(void)
{
	AnimationNodePtrList::iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->m_Owner = m_Owner;
			(*node_iter)->OnSetOwner();
		}
	}
}

void AnimationNode::UpdateRate(float fRate)
{
	AnimationNodePtrList::iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->UpdateRate(fRate);
		}
	}
}

void AnimationNode::Tick(float fElapsedTime, float fTotalWeight)
{
}

my::BoneList & AnimationNode::GetPose(my::BoneList & pose) const
{
	for (unsigned int i = 0; i < m_Owner->m_Skeleton->m_boneBindPose.size(); i++)
	{
		pose[i].SetPosition(Vector3(0, 0, 0));
		pose[i].SetRotation(Quaternion::Identity());
	}
	return pose;
}

BOOST_CLASS_EXPORT(AnimationNodeSequence)

void AnimationNodeSequence::OnSetOwner(void)
{
	AnimationNode::OnSetOwner();

	if (m_Owner && !m_Group.empty())
	{
		m_Owner->AddToSequenceGroup(m_Group, this);
	}
}

void AnimationNodeSequence::UpdateRate(float fRate)
{
	AnimationNode::UpdateRate(fRate);

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
	if (m_Owner->m_Skeleton)
	{
		const OgreAnimation * anim = m_Owner->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			return anim->GetTime();
		}
	}
	return 0;
}

my::BoneList & AnimationNodeSequence::GetPose(my::BoneList & pose) const
{
	if (m_Owner->m_Skeleton)
	{
		const OgreAnimation * anim = m_Owner->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			boost::unordered_map<std::string, int>::const_iterator root_iter = m_Owner->m_Skeleton->m_boneNameMap.find(m_Root);
			if (root_iter != m_Owner->m_Skeleton->m_boneNameMap.end())
			{
				anim->GetPose(pose, m_Owner->m_Skeleton->m_boneHierarchy, root_iter->second, m_Time);
			}
		}
	}
	return pose;
}

BOOST_CLASS_EXPORT(AnimationNodeSlot)

void AnimationNodeSlot::Tick(float fElapsedTime, float fTotalWeight)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		if (seq_iter->m_BlendTime < fElapsedTime)
		{
			seq_iter->m_Weight = seq_iter->m_TargetWeight;
			seq_iter->m_BlendTime = 0;
		}
		else
		{
			const float delta = seq_iter->m_TargetWeight - seq_iter->m_Weight;
			seq_iter->m_Weight += delta * fElapsedTime / seq_iter->m_BlendTime;
			seq_iter->m_BlendTime -= fElapsedTime;
		}

		fTotalWeight *= 1 - seq_iter->m_TargetWeight;
	}

	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime, fTotalWeight);
	}

	Advance(fElapsedTime);
}

void AnimationNodeSlot::Advance(float fElapsedTime)
{
	if (m_Owner->m_Skeleton)
	{
		SequenceList::iterator seq_iter = m_SequenceSlot.begin();
		while (seq_iter != m_SequenceSlot.end())
		{
			const OgreAnimation * anim = m_Owner->m_Skeleton->GetAnimation(seq_iter->m_Name);

			if (seq_iter->m_TargetWeight <= 0 && seq_iter->m_BlendTime < fElapsedTime)
			{
				seq_iter = m_SequenceSlot.erase(seq_iter);
				continue;
			}

			float Length = anim ? anim->GetTime() : 0;

			seq_iter->m_Time += fElapsedTime * seq_iter->m_Rate;

			if (seq_iter->m_Time > Length)
			{
				if (seq_iter->m_Loop)
				{
					seq_iter->m_Time -= Length;
				}
				else if (seq_iter->m_TargetWeight > 0)
				{
					seq_iter->m_TargetWeight = 0;
					seq_iter->m_BlendTime = m_BlendOutTime;
				}
			}
			seq_iter++;
		}
	}
}

void AnimationNodeSlot::Play(const std::string & Name, const std::string & Root, int Priority, bool Loop /*= false*/, bool StopBehind /*= true*/, float Rate /*= 1.0f*/, float Weight /*= 1.0f*/)
{
	Sequence seq;
	seq.m_Time = 0;
	seq.m_Rate = Rate;
	seq.m_Weight = 0;
	seq.m_Name = Name;
	seq.m_Root = Root;
	seq.m_Priority = Priority;
	seq.m_Loop = Loop;
	seq.m_BlendTime = m_BlendInTime;
	seq.m_TargetWeight = Weight;
	SequenceList::iterator seq_iter = std::lower_bound(m_SequenceSlot.begin(), m_SequenceSlot.end(), seq,
		boost::bind(std::greater<int>(), boost::bind(&Sequence::m_Priority, _1), seq.m_Priority));
	seq_iter = m_SequenceSlot.insert(seq_iter, seq);

	if (StopBehind)
	{
		StopFrom(seq_iter + 1);
	}
}

void AnimationNodeSlot::StopFrom(SequenceList::iterator seq_iter)
{
	while (seq_iter != m_SequenceSlot.end())
	{
		if (seq_iter->m_Loop || seq_iter->m_TargetWeight > 0)
		{
			seq_iter->m_Loop = false;
			seq_iter->m_TargetWeight = 0;
			seq_iter->m_BlendTime = m_BlendOutTime;
		}
		seq_iter++;
	}
}

void AnimationNodeSlot::Stop(void)
{
	StopFrom(m_SequenceSlot.begin());
}

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose) const
{
	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose);
	}

	if (m_Owner->m_Skeleton)
	{
		SequenceList::const_reverse_iterator seq_iter = m_SequenceSlot.rbegin();
		for (; seq_iter != m_SequenceSlot.rend(); seq_iter++)
		{
			const OgreAnimation * anim = m_Owner->m_Skeleton->GetAnimation(seq_iter->m_Name);
			if (anim)
			{
				boost::unordered_map<std::string, int>::const_iterator root_iter = m_Owner->m_Skeleton->m_boneNameMap.find(seq_iter->m_Root);
				if (root_iter != m_Owner->m_Skeleton->m_boneNameMap.end())
				{
					my::BoneList other(pose.size());
					anim->GetPose(other, m_Owner->m_Skeleton->m_boneHierarchy, root_iter->second, seq_iter->m_Time);
					pose.LerpSelf(other, m_Owner->m_Skeleton->m_boneHierarchy, root_iter->second, seq_iter->m_Weight);
				}
			}
		}
	}
	return pose;
}

BOOST_CLASS_EXPORT(AnimationNodeBlend)

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
				SetActiveChild(0, m_BlendInTime);
			}
		}
		else
		{
			if (m_ActiveChild != 1)
			{
				SetActiveChild(1, m_BlendInTime);
			}
		}
	}

	AnimationNodeBlend::Tick(fElapsedTime, fTotalWeight);
}

BOOST_CLASS_EXPORT(AnimationNodeRateBySpeed)

void AnimationNodeRateBySpeed::Tick(float fElapsedTime, float fTotalWeight)
{
	Character * character = dynamic_cast<Character *>(m_Owner->m_Actor);
	if (character)
	{
		float speed_sq = character->m_Velocity.x * character->m_Velocity.x + character->m_Velocity.z * character->m_Velocity.z;
		UpdateRate(sqrtf(speed_sq) / m_BaseSpeed);
	}

	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime, fTotalWeight);
	}
}

my::BoneList & AnimationNodeRateBySpeed::GetPose(my::BoneList & pose) const
{
	if (m_Childs[0])
	{
		return m_Childs[0]->GetPose(pose);
	}
	return pose;
}

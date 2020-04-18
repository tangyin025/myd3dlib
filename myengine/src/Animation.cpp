#include "Animation.h"
#include "Character.h"
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

BOOST_CLASS_EXPORT(AnimationNodeBlendBySpeed)

BOOST_CLASS_EXPORT(AnimationNodeRateBySpeed)

BOOST_CLASS_EXPORT(AnimationRoot)

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

template
void AnimationNode::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void AnimationNode::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void AnimationNode::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void AnimationNode::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void AnimationNode::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void AnimationNode::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void AnimationNode::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void AnimationNode::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

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
		m_Time = fmod(m_Time + fElapsedTime, GetLength());
	}
	else
	{
		m_Time = my::Min(m_Time + fElapsedTime, GetLength());
	}
}

float AnimationNodeSequence::GetLength(void) const
{
	const AnimationRoot * Root = dynamic_cast<const AnimationRoot *>(GetTopNode());
	if (Root->m_Skeleton)
	{
		const OgreAnimation * anim = Root->m_Skeleton->GetAnimation(m_Name);
		if (anim)
		{
			return anim->GetLength();
		}
	}
	return 0;
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
	const AnimationRoot * Root = dynamic_cast<const AnimationRoot *>(GetTopNode());
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
	}

	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime, fTotalWeight);
	}

	Advance(fElapsedTime);
}

void AnimationNodeSlot::Advance(float fElapsedTime)
{
	const AnimationRoot * Root = dynamic_cast<const AnimationRoot *>(GetTopNode());
	if (Root->m_Skeleton)
	{
		SequenceList::iterator seq_iter = m_SequenceSlot.begin();
		while (seq_iter != m_SequenceSlot.end())
		{
			const OgreAnimation * anim = Root->m_Skeleton->GetAnimation(seq_iter->m_Name);

			if (seq_iter->m_TargetWeight <= 0 && seq_iter->m_BlendTime < fElapsedTime)
			{
				seq_iter = m_SequenceSlot.erase(seq_iter);
				continue;
			}

			float Length = anim ? anim->GetLength() : 0;

			seq_iter->m_Time += fElapsedTime;

			if (seq_iter->m_Time > Length)
			{
				seq_iter->m_TargetWeight = 0;
				seq_iter->m_BlendTime = seq_iter->m_BlendOutTime;
			}
			seq_iter++;
		}
	}
}

my::BoneList & AnimationNodeSlot::GetPose(my::BoneList & pose) const
{
	if (m_Childs[0])
	{
		m_Childs[0]->GetPose(pose);
	}

	const AnimationRoot * Root = dynamic_cast<const AnimationRoot *>(GetTopNode());
	if (Root->m_Skeleton)
	{
		SequenceList::const_reverse_iterator seq_iter = m_SequenceSlot.rbegin();
		for (; seq_iter != m_SequenceSlot.rend(); seq_iter++)
		{
			my::BoneList OtherPose(pose.size());
			seq_iter->GetPose(OtherPose);
			pose.LerpSelf(OtherPose, seq_iter->m_Weight);
		}
	}
	return pose;
}

void AnimationNodeSlot::Play(const std::string & Name, std::string RootList, float BlendTime, float BlendOutTime, float Weight /*= 1.0f*/)
{
	Sequence seq;
	seq.m_Time = 0;
	seq.m_Weight = 0;
	seq.m_Name = Name;
	seq.SetRootList(RootList);
	seq.m_BlendTime = BlendTime;
	seq.m_BlendOutTime = BlendOutTime;
	seq.m_TargetWeight = Weight;
	seq.m_Parent = this;
	m_SequenceSlot.push_front(seq);
}

void AnimationNodeSlot::Stop(void)
{
	SequenceList::iterator seq_iter = m_SequenceSlot.begin();
	for (; seq_iter != m_SequenceSlot.end(); seq_iter++)
	{
		seq_iter->m_TargetWeight = 0;
		seq_iter->m_BlendTime = seq_iter->m_BlendOutTime;
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

void AnimationNodeBlendBySpeed::Tick(float fElapsedTime, float fTotalWeight)
{
	AnimationRoot * Root = dynamic_cast<AnimationRoot *>(GetTopNode());
	Character * character = dynamic_cast<Character *>(Root->m_Actor);
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

void AnimationNodeRateBySpeed::Tick(float fElapsedTime, float fTotalWeight)
{
	AnimationRoot * Root = dynamic_cast<AnimationRoot *>(GetTopNode());
	Character * character = dynamic_cast<Character *>(Root->m_Actor);
	float fRate = 1.0f;
	if (character)
	{
		float speed_sq = character->m_Velocity.x * character->m_Velocity.x + character->m_Velocity.z * character->m_Velocity.z;
		fRate = sqrtf(speed_sq) / m_Speed0;
	}

	if (m_Childs[0])
	{
		m_Childs[0]->Tick(fElapsedTime * fRate, fTotalWeight);
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

template<class Archive>
void AnimationRoot::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNodeSlot);
	ar << BOOST_SERIALIZATION_NVP(m_SkeletonPath);
}

template<class Archive>
void AnimationRoot::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNodeSlot);
	ar >> BOOST_SERIALIZATION_NVP(m_SkeletonPath);
	ReloadSequenceGroup();
}

template
void AnimationRoot::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void AnimationRoot::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void AnimationRoot::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void AnimationRoot::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void AnimationRoot::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void AnimationRoot::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void AnimationRoot::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void AnimationRoot::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

void AnimationRoot::OnReady(my::IORequest * request)
{
	m_Skeleton = boost::dynamic_pointer_cast<my::OgreSkeletonAnimation>(request->m_res);

	if (m_SkeletonEventReady)
	{
		my::EventArg arg;
		m_SkeletonEventReady(&arg);
	}

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

void AnimationRoot::RequestResource(void)
{
	if (!m_SkeletonPath.empty())
	{
		_ASSERT(!m_Skeleton);

		my::ResourceMgr::getSingleton().LoadSkeletonAsync(m_SkeletonPath.c_str(), this);
	}
}

void AnimationRoot::ReleaseResource(void)
{
	if (!m_SkeletonPath.empty())
	{
		my::ResourceMgr::getSingleton().RemoveIORequestCallback(m_SkeletonPath, this);

		m_Skeleton.reset();
	}
}

void AnimationRoot::Update(float fElapsedTime)
{
	if (m_Skeleton && m_Childs[0])
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
			Bone parent(
				anim_pose_hier[jb_iter->second.root_i].m_rotation * m_Actor->m_Rotation,
				anim_pose_hier[jb_iter->second.root_i].m_position.transform(m_Actor->m_Rotation) + m_Actor->m_Position);
			UpdateJiggleBone(jb_iter->second, parent, jb_iter->first, particle_i, fElapsedTime);
		}

		IKContextMap::iterator ik_iter = m_Iks.begin();
		for (; ik_iter != m_Iks.end(); ik_iter++)
		{
			UpdateIK(ik_iter->second);
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

void AnimationRoot::ReloadSequenceGroup(void)
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

void AnimationRoot::ReloadSequenceGroupWalker(AnimationNode * node)
{
	AnimationNodeSequence * sequence = dynamic_cast<AnimationNodeSequence *>(node);
	if (sequence && !sequence->m_Group.empty())
	{
		m_SequenceGroup.insert(std::make_pair(sequence->m_Group, sequence));
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

void AnimationRoot::UpdateSequenceGroup(void)
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
			float time_pct = master_seq_iter->second->m_Time / master_seq_iter->second->GetLength();

			SequenceGroupMap::iterator slave_seq_iter = seq_iter;
			for (; slave_seq_iter != next_seq_iter; slave_seq_iter++)
			{
				if (slave_seq_iter != master_seq_iter)
				{
					slave_seq_iter->second->m_Time = Lerp(0.0f, slave_seq_iter->second->GetLength(), time_pct);
				}
			}
		}
		seq_iter = next_seq_iter;
	}
}

void AnimationRoot::AddJiggleBone(const std::string & bone_name, float mass, float damping, float springConstant)
{
	_ASSERT(mass > 0);

	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	OgreSkeleton::BoneNameMap::const_iterator bone_name_iter = m_Skeleton->m_boneNameMap.find(bone_name);
	if (bone_name_iter == m_Skeleton->m_boneNameMap.end())
	{
		return;
	}

	int root_i = m_Skeleton->FindParent(bone_name_iter->second);
	if (-1 == root_i)
	{
		return;
	}

	JiggleBoneContext & context = m_JiggleBones[bone_name_iter->second];
	context.root_i = root_i;
	context.springConstant = springConstant;
	AddJiggleBone(context, bone_name_iter->second, mass, damping);
}

void AnimationRoot::AddJiggleBone(JiggleBoneContext & context, int node_i, float mass, float damping)
{
	context.m_ParticleList.push_back(Particle(
		Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 1 / mass, damping));
	node_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; node_i >= 0; node_i = m_Skeleton->m_boneHierarchy[node_i].m_sibling)
	{
		AddJiggleBone(context, node_i, mass, damping);
	}
}

void AnimationRoot::UpdateJiggleBone(JiggleBoneContext & context, const my::Bone & parent, int node_i, int & particle_i, float fElapsedTime)
{
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
	Bone final(
		target.GetRotation() * Quaternion::RotationFromTo(d0, d1),
		parent.GetPosition() + d1.normalize() * d0.magnitude());
	particle.setPosition(final.GetPosition());
	anim_pose_hier[node_i].SetRotation(final.GetRotation() * m_Actor->m_Rotation.conjugate());
	anim_pose_hier[node_i].SetPosition((final.GetPosition() - m_Actor->m_Position).transform(m_Actor->m_Rotation.conjugate()));

	particle_i++;
	node_i = m_Skeleton->m_boneHierarchy[node_i].m_child;
	for (; node_i >= 0; node_i = m_Skeleton->m_boneHierarchy[node_i].m_sibling)
	{
		UpdateJiggleBone(context, final, node_i, particle_i, fElapsedTime);
	}
}

void AnimationRoot::AddIK(const std::string & bone_name, float hitRadius, unsigned int filterWord0)
{
	_ASSERT(m_Actor);

	_ASSERT(m_Skeleton);

	OgreSkeleton::BoneNameMap::const_iterator bone_name_iter = m_Skeleton->m_boneNameMap.find(bone_name);
	if (bone_name_iter == m_Skeleton->m_boneNameMap.end())
	{
		return;
	}

	IKContext & ik = m_Iks[bone_name_iter->second];
	int node_i = bone_name_iter->second;
	for (int i = 0; i < _countof(ik.id); i++, node_i = m_Skeleton->m_boneHierarchy[node_i].m_child)
	{
		if (node_i < 0)
		{
			m_Iks.erase(bone_name_iter->second);
			return;
		}
		ik.id[i] = node_i;
	}

	ik.hitRadius = hitRadius;

	ik.filterWord0 = filterWord0;
}

void AnimationRoot::UpdateIK(IKContext & ik)
{
	PhysxSceneContext * scene = dynamic_cast<PhysxSceneContext *>(m_Actor->m_Node->GetTopNode());
	_ASSERT(scene);

	Vector3 pos[3] = {
		anim_pose_hier[ik.id[0]].m_position,
		anim_pose_hier[ik.id[1]].m_position,
		anim_pose_hier[ik.id[2]].m_position };

	Vector3 dir[3] = { pos[1] - pos[0], pos[2] - pos[1], pos[2] - pos[0] };
	float length[3] = { dir[0].magnitude(), dir[1].magnitude(), dir[2].magnitude() };
	Vector3 normal[3] = { dir[0] / length[0], dir[1] / length[1], dir[2] / length[2] };
	float theta[2] = {
		acos(Vector3::CosTheta(normal[0], normal[2])),
		acos(Vector3::CosTheta(-normal[0], normal[1])) };

	physx::PxSweepBuffer hit;
	physx::PxSphereGeometry sphere(ik.hitRadius);
	physx::PxQueryFilterData filterData = physx::PxQueryFilterData();
	filterData.data.word0 = ik.filterWord0;
	bool status = scene->m_PxScene->sweep(sphere,
		physx::PxTransform((physx::PxVec3&)(pos[0].transform(m_Actor->m_Rotation) + m_Actor->m_Position)),
		(physx::PxVec3&)normal[2].transform(m_Actor->m_Rotation), length[2], hit, physx::PxHitFlag::eDEFAULT, filterData);
	if (status && hit.block.distance > 0)
	{
		float new_theta[2] = {
			acos(Vector3::Cosine(length[1], length[0], hit.block.distance)),
			acos(Vector3::Cosine(hit.block.distance, length[1], length[0])) };
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

void AnimationRoot::TransformHierarchyBoneList(
	my::BoneList & boneList,
	const my::BoneHierarchy & boneHierarchy,
	int root_i,
	const my::Quaternion & Rotation,
	const my::Vector3 & Position)
{
	BoneHierarchy::const_reference node = boneHierarchy[root_i];
	BoneList::reference bone = boneList[root_i];
	bone.m_rotation *= Rotation;
	bone.m_position = (bone.m_position - Position).transform(Rotation) + Position;

	int node_i = boneHierarchy[root_i].m_child;
	for (; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		TransformHierarchyBoneList(boneList, boneHierarchy, node_i, Rotation, Position);
	}
}

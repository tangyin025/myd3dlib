// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "mySkeleton.h"
#include "Component.h"
#include "myPhysics.h"

class AnimationNode;

typedef boost::shared_ptr<AnimationNode> AnimationNodePtr;

class AnimationNode
{
public:
	AnimationNode * m_Parent;

	std::string m_Name;

	typedef std::vector<AnimationNodePtr> AnimationNodePtrList;

	AnimationNodePtrList m_Childs;

protected:
	AnimationNode(void)
		: m_Parent(NULL)
	{
	}

public:
	AnimationNode(const char * Name, unsigned int ChildNum)
		: m_Parent(NULL)
		, m_Name(Name)
		, m_Childs(ChildNum)
	{
	}

	virtual ~AnimationNode(void);

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	void SetChild(int i, AnimationNodePtr node);

	const AnimationNode * GetTopNode(void) const;

	AnimationNode * GetTopNode(void);

	const AnimationNode * FindSubNode(const std::string & Name) const;

	AnimationNode * FindSubNode(const std::string & Name);

	virtual void Tick(float fElapsedTime, float fTotalWeight) = 0;

	virtual my::BoneList & GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const = 0;
};

class AnimationNodeSequence : public AnimationNode
{
public:
	float m_Time;

	float m_TargetWeight;

	float m_Rate;

	bool m_Loop;

	std::string m_Group;

	typedef std::map<float, unsigned int> EventMap;

	EventMap m_Events;

	Animator * m_GroupOwner;

protected:
	AnimationNodeSequence(void)
		: m_Time(0)
		, m_TargetWeight(1.0f)
		, m_Rate(1.0f)
		, m_Loop(true)
		, m_GroupOwner(NULL)
	{
	}

public:
	AnimationNodeSequence(const char * Name, float Rate = 1.0f, bool Loop = true, const char * Group = "")
		: AnimationNode(Name, 0)
		, m_Time(0)
		, m_TargetWeight(1.0f)
		, m_Rate(Rate)
		, m_Loop(Loop)
		, m_Group(Group)
		, m_GroupOwner(NULL)
	{
	}

	virtual ~AnimationNodeSequence(void);

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_Name);
		ar & BOOST_SERIALIZATION_NVP(m_Rate);
		ar & BOOST_SERIALIZATION_NVP(m_Loop);
		ar & BOOST_SERIALIZATION_NVP(m_Group);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	void Advance(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const;

	float GetLength(void) const;

	void AddEvent(float time, unsigned int id);

	void TriggerEvent(float begin, float end);
};

typedef boost::shared_ptr<AnimationNodeSequence> AnimationNodeSequencePtr;

class AnimationNodeSlot : public AnimationNode
{
public:
	class Sequence : public AnimationNodeSequence
	{
	public:
		int m_Priority;

		float m_BlendTime;

		float m_BlendOutTime;

		float m_Weight;

		DWORD_PTR m_UserData;

		Sequence(void)
			: m_Priority(INT_MIN)
			, m_BlendTime(0)
			, m_BlendOutTime(0)
			, m_Weight(0)
			, m_UserData(0)
		{
		}

		~Sequence(void)
		{
			_ASSERT(!m_Parent || dynamic_cast<AnimationNodeSlot *>(m_Parent));
			m_Parent = NULL;
		}
	};

	typedef std::list<Sequence> SequenceList;

	SequenceList m_SequenceSlot;

	int m_NodeId;

protected:
	AnimationNodeSlot(void)
		: m_NodeId(-1)
	{
	}

public:
	AnimationNodeSlot(const char * Name)
		: AnimationNode(Name, 1)
		, m_NodeId(-1)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_NodeId);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const;

	void Play(const std::string & Name, float Rate, float Weight, float BlendTime, float BlendOutTime, const std::string & Group, int Priority, DWORD_PTR UserData);

	SequenceList::iterator RemoveSlotIter(SequenceList::iterator iter);

	void StopSlotIter(SequenceList::iterator iter, float BlendOutTime);

	void StopSlotByUserData(DWORD_PTR UserData, float BlendOutTime);

	bool IsPlaying(void) const;

	void StopAllSlot(float BlendOutTime);
};

typedef boost::shared_ptr<AnimationNodeSlot> AnimationNodeSlotPtr;

class AnimationNodeSubTree : public AnimationNode
{
public:
	int m_NodeId;

	my::BoneHierarchy m_boneHierarchy;

protected:
	AnimationNodeSubTree(void)
		: m_NodeId(-1)
	{
	}

public:
	AnimationNodeSubTree(const char * Name)
		: AnimationNode(Name, 2)
		, m_NodeId(-1)
	{
	}

	virtual ~AnimationNodeSubTree(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_NodeId);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const;
};

typedef boost::shared_ptr<AnimationNodeSubTree> AnimationNodeSubTreePtr;

class AnimationNodeBlendList : public AnimationNode
{
public:
	float m_BlendTime;

	std::vector<float> m_Weight;

	std::vector<float> m_TargetWeight;

protected:
	AnimationNodeBlendList(void)
		: m_BlendTime(0)
	{
	}

public:
	AnimationNodeBlendList(const char * Name, unsigned int ChildNum);

	virtual ~AnimationNodeBlendList(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_TargetWeight);
	}

	void SetTargetWeight(int Child, float Weight);

	void SetTargetWeight(int Child, float Weight, bool Exclusive);

	float GetTargetWeight(int Child);

	void SetActiveChild(int ActiveChild, float BlendTime);

	int GetActiveChild(void) const;

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const;
};

typedef boost::shared_ptr<AnimationNodeBlendList> AnimationNodeBlendListPtr;

class Actor;

class Animator;

namespace my
{
	class DrawHelper;
}

class Animator
	: public Component
	, public AnimationNode
{
public:
	enum { TypeID = ComponentTypeAnimator };

	std::string m_SkeletonPath;

	my::OgreSkeletonAnimationPtr m_Skeleton;

	my::BoneList anim_pose_hier;

	my::BoneList bind_pose;

	my::BoneList anim_pose;

	my::BoneList final_pose;

	my::TransformList m_DualQuats;

	typedef std::multimap<std::string, AnimationNodeSequence *> SequenceGroupMap;

	SequenceGroupMap m_SequenceGroup;

	typedef std::vector<AnimationNodeSequence *> SequenceList;

	SequenceList m_ActiveSequence;

	typedef std::vector<my::Particle> ParticleList;

	class DynamicBoneContext
	{
	public:
		int parent_i;

		float springConstant;

		ParticleList m_ParticleList;

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_NVP(parent_i);
			ar & BOOST_SERIALIZATION_NVP(springConstant);
			ar & BOOST_SERIALIZATION_NVP(m_ParticleList);
		}
	};

	typedef std::map<int, DynamicBoneContext> DynamicBoneContextMap;

	DynamicBoneContextMap m_DynamicBones;

	class IKContext
	{
	public:
		int id;

		float hitRadius;

		unsigned int filterWord0;
	};

	typedef std::map<int, IKContext> IKContextMap;

	IKContextMap m_Iks;

protected:
	Animator(void)
	{
	}

public:
	Animator(const char * Name)
		: Component(Name)
		, AnimationNode(Name ? Name : "unknown", 1)
	{
	}

	virtual ~Animator(void);

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	void OnSkeletonReady(my::DeviceResourceBasePtr res);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose, int root_i, const my::BoneHierarchy & boneHierarchy) const;

	void UpdateHierarchyBoneList(const int node_i, const my::Bone& parent);

	void AddSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	void RemoveSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	void ReloadSequenceGroup(void);

	void ReloadSequenceGroupWalker(AnimationNode * node);

	void UpdateSequenceGroup(float fElapsedTime);

	void SyncSequenceGroupTime(SequenceGroupMap::iterator begin, SequenceGroupMap::iterator end, const AnimationNodeSequence * master);

	void AddDynamicBone(int node_i, float mass, float damping, float springConstant);

	void UpdateDynamicBone(DynamicBoneContext & context, const my::Bone & parent, const my::Vector3 & parent_world_pos, int node_i, int & particle_i, float fElapsedTime);

	void AddIK(int node_i, float hitRadius, unsigned int filterWord0);

	void UpdateIK(IKContext & ik);

	static void TransformHierarchyBoneList(
		my::BoneList & boneList,
		const my::BoneHierarchy & boneHierarchy,
		int node_i,
		const my::Quaternion & Rotation,
		const my::Vector3 & Position);

	void DrawDebugBone(my::DrawHelper * helper, D3DCOLOR color);
};

typedef boost::shared_ptr<Animator> AnimatorPtr;

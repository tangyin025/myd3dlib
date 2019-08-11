#pragma once

#include "mySkeleton.h"
#include "myPhysics.h"

class AnimationNode;

typedef boost::shared_ptr<AnimationNode> AnimationNodePtr;

class AnimationNodeSequence;

class Actor;

class Animator : public my::IResourceCallback
{
public:
	Actor * m_Actor;

	std::string m_SkeletonPath;

	my::OgreSkeletonAnimationPtr m_Skeleton;

	my::ControlEvent m_SkeletonEventReady;

	AnimationNodePtr m_Node;

	my::BoneList anim_pose;

	my::BoneList bind_pose_hier;

	my::BoneList anim_pose_hier;

	my::BoneList final_pose;

	my::TransformList m_DualQuats;

	typedef std::multimap<std::string, AnimationNodeSequence *> SequenceGroupMap;

	SequenceGroupMap m_SequenceGroups;

	typedef std::vector<my::Particle> ParticleList;

	class JiggleBoneContext
	{
	public:
		int root_i;

		float springConstant;

		ParticleList m_ParticleList;
	};

	typedef std::map<int, JiggleBoneContext> JiggleBoneContextMap;

	JiggleBoneContextMap m_JiggleBones;

	class IKContext
	{
	public:
		int id[3];

		float hitRadius;

		unsigned int filterWord0;
	};

	typedef std::map<int, IKContext> IKContextMap;

	IKContextMap m_Iks;

protected:
	Animator(void)
		: m_Actor(NULL)
	{
	}

public:
	Animator(Actor * actor)
		: m_Actor(actor)
	{
	}

	virtual ~Animator(void)
	{
	}

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

	virtual void OnReady(my::IORequest * request);

	void RequestResource(void);

	void ReleaseResource(void);

	void Update(float fElapsedTime);

	void UpdateGroup(float fElapsedTime);

	void AddToSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	void RemoveFromSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	void AddJiggleBone(const std::string & bone_name, float mass, float damping, float springConstant);

	void AddJiggleBone(JiggleBoneContext & context, int node_i, float mass, float damping);

	void UpdateJiggleBone(JiggleBoneContext & context, const my::Bone & parent, int node_i, int & particle_i, float fElapsedTime);

	void AddIK(const std::string & bone_name, float hitRadius, unsigned int filterWord0);

	void UpdateIK(IKContext & ik);

	static void TransformHierarchyBoneList(
		my::BoneList & boneList,
		const my::BoneHierarchy & boneHierarchy,
		int root_i,
		const my::Quaternion & Rotation,
		const my::Vector3 & Position);
};

typedef boost::shared_ptr<Animator> AnimatorPtr;

class AnimationNode
{
public:
	Animator * m_Owner;

	typedef std::vector<AnimationNodePtr> AnimationNodePtrList;

	AnimationNodePtrList m_Childs;

protected:
	AnimationNode(void)
		: m_Owner(NULL)
	{
	}

public:
	AnimationNode(Animator * Owner, unsigned int ChildNum)
		: m_Owner(Owner)
		, m_Childs(ChildNum)
	{
	}

	virtual ~AnimationNode(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Childs);
	}

	template <unsigned int i>
	AnimationNodePtr GetChild(void) const
	{
		return m_Childs[i];
	}

	template <unsigned int i>
	void SetChild(AnimationNodePtr node)
	{
		m_Childs[i] = node;
	}

	virtual void OnSetOwner(void);

	virtual void UpdateRate(float fRate);

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

class AnimationNodeSequence : public AnimationNode
{
public:
	float m_Time;

	float m_Rate;

	float m_Weight;

	std::string m_Name;

	std::string m_Root;

	std::string m_Group;

protected:
	AnimationNodeSequence(void)
		: m_Time(0)
		, m_Rate(1)
		, m_Weight(0)
	{
	}

public:
	AnimationNodeSequence(Animator * Owner)
		: AnimationNode(Owner, 0)
		, m_Time(0)
		, m_Rate(1)
		, m_Weight(0)
	{
	}

	~AnimationNodeSequence(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_Name);
		ar & BOOST_SERIALIZATION_NVP(m_Root);
		ar & BOOST_SERIALIZATION_NVP(m_Group);
	}

	virtual void OnSetOwner(void);

	virtual void UpdateRate(float fRate);

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual void Advance(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;

	float GetLength(void) const;
};

typedef boost::shared_ptr<AnimationNodeSequence> AnimationNodeSequencePtr;

class AnimationNodeSlot : public AnimationNode
{
public:
	struct Sequence
	{
	public:
		float m_Time;

		float m_Rate;

		float m_Weight;

		std::string m_Name;

		std::string m_Root;

		int m_Priority;

		bool m_Loop;

		float m_BlendTime;

		float m_TargetWeight;
	};

	typedef std::vector<Sequence> SequenceList;

	SequenceList m_SequenceSlot;

	float m_BlendInTime;

	float m_BlendOutTime;

protected:
	AnimationNodeSlot(void)
		: m_BlendInTime(0.3f)
		, m_BlendOutTime(0.3f)
	{
	}

public:
	AnimationNodeSlot(Animator * Owner)
		: AnimationNode(Owner, 1)
		, m_BlendInTime(0.3f)
		, m_BlendOutTime(0.3f)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_BlendInTime);
		ar & BOOST_SERIALIZATION_NVP(m_BlendOutTime);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual void Advance(float fElapsedTime);

	void Play(const std::string & Name, const std::string & Root, int Priority, bool Loop = false, bool StopBehind = true, float Rate = 1.0f, float Weight = 1.0f);

	void StopFrom(SequenceList::iterator seq_iter);

	void Stop(void);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNodeSlot> AnimationNodeSlotPtr;

class AnimationNodeBlend : public AnimationNode
{
public:
	float m_BlendTime;

	unsigned int m_ActiveChild;

	float m_Weight;

	float m_TargetWeight;

protected:
	AnimationNodeBlend(void)
		: m_BlendTime(0)
		, m_ActiveChild(0)
		, m_Weight(0)
		, m_TargetWeight(0)
	{
	}

public:
	AnimationNodeBlend(Animator * Owner)
		: AnimationNode(Owner, 2)
		, m_BlendTime(0)
		, m_ActiveChild(0)
		, m_Weight(0)
		, m_TargetWeight(0)
	{
	}

	~AnimationNodeBlend(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_BlendTime);
		ar & BOOST_SERIALIZATION_NVP(m_ActiveChild);
	}

	void SetActiveChild(unsigned int ActiveChild, float BlendTime);

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNodeBlend> AnimationNodeBlendPtr;

class AnimationNodeBlendBySpeed : public AnimationNodeBlend
{
public:
	float m_Speed0;

	float m_BlendInTime;

protected:
	AnimationNodeBlendBySpeed(void)
		: m_Speed0(0.1f)
		, m_BlendInTime(0.3f)
	{
	}

public:
	AnimationNodeBlendBySpeed(Animator * Owner)
		: AnimationNodeBlend(Owner)
		, m_Speed0(0.1f)
		, m_BlendInTime(0.3f)
	{
	}

	~AnimationNodeBlendBySpeed(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNodeBlend);
		ar & BOOST_SERIALIZATION_NVP(m_Speed0);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);
};

typedef boost::shared_ptr<AnimationNodeBlendBySpeed> AnimationNodeBlendBySpeedPtr;

class AnimationNodeRateBySpeed : public AnimationNode
{
public:
	float m_BaseSpeed;

protected:
	AnimationNodeRateBySpeed(void)
		: m_BaseSpeed(1.0f)
	{
	}

public:
	AnimationNodeRateBySpeed(Animator * Owner)
		: AnimationNode(Owner, 1)
		, m_BaseSpeed(1.0f)
	{
	}

	~AnimationNodeRateBySpeed(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_BaseSpeed);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNodeRateBySpeed> AnimationNodeRateBySpeedPtr;

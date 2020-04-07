#pragma once

#include "mySkeleton.h"
#include "myPhysics.h"
#include <boost/circular_buffer.hpp>

class AnimationNode;

typedef boost::shared_ptr<AnimationNode> AnimationNodePtr;

class AnimationNode
{
public:
	AnimationNode * m_Parent;

	typedef std::vector<AnimationNodePtr> AnimationNodePtrList;

	AnimationNodePtrList m_Childs;

protected:
	AnimationNode(void)
		: m_Parent(NULL)
	{
	}

public:
	AnimationNode(unsigned int ChildNum)
		: m_Parent(NULL)
		, m_Childs(ChildNum)
	{
	}

	virtual ~AnimationNode(void)
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

	template <unsigned int i>
	AnimationNodePtr GetChild(void) const
	{
		return m_Childs[i];
	}

	template <unsigned int i>
	void SetChild(AnimationNodePtr node)
	{
		m_Childs[i] = node;
		node->m_Parent = this;
	}

	const AnimationNode * GetTopNode(void) const;

	AnimationNode * GetTopNode(void);

	virtual void UpdateRate(float fRate);

	virtual void Tick(float fElapsedTime, float fTotalWeight) = 0;

	virtual my::BoneList & GetPose(my::BoneList & pose) const = 0;
};

class AnimationNodeSequence : public AnimationNode
{
public:
	float m_Time;

	float m_Rate;

	float m_Weight;

	std::string m_Name;

	std::string m_Group;

public:
	AnimationNodeSequence(void)
		: AnimationNode(0)
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
		ar & BOOST_SERIALIZATION_NVP(m_Group);
	}

	virtual void UpdateRate(float fRate);

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	void Advance(float fElapsedTime);

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

		std::vector<std::string> m_RootList;

		float m_BlendTime;

		float m_BlendOutTime;

		float m_TargetWeight;
	};

	typedef boost::circular_buffer<Sequence> SequenceList;

	SequenceList m_SequenceSlot;

public:
	AnimationNodeSlot(void)
		: AnimationNode(1)
		, m_SequenceSlot(2)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
	}

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	void Advance(float fElapsedTime);

	void Play(const std::string & Name, std::string RootList, float BlendTime, float BlendOutTime, float Rate = 1.0f, float Weight = 1.0f);

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

public:
	AnimationNodeBlend(void)
		: AnimationNode(2)
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

public:
	AnimationNodeBlendBySpeed(void)
		: m_Speed0(0.1f)
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

public:
	AnimationNodeRateBySpeed(void)
		: AnimationNode(1)
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

class Actor;

class AnimationRoot : public AnimationNode, public my::IResourceCallback
{
public:
	Actor * m_Actor;

	std::string m_SkeletonPath;

	my::OgreSkeletonAnimationPtr m_Skeleton;

	my::ControlEvent m_SkeletonEventReady;

	my::BoneList anim_pose;

	my::BoneList bind_pose_hier;

	my::BoneList anim_pose_hier;

	my::BoneList final_pose;

	my::TransformList m_DualQuats;

	typedef std::multimap<std::string, AnimationNodeSequence *> SequenceGroupMap;

	SequenceGroupMap m_SequenceGroup;

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
	AnimationRoot(void)
		: m_Actor(NULL)
	{
	}

public:
	AnimationRoot(Actor * actor)
		: AnimationNode(1)
		, m_Actor(actor)
	{
	}

	virtual ~AnimationRoot(void)
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

	virtual void Tick(float fElapsedTime, float fTotalWeight);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;

	void RequestResource(void);

	void ReleaseResource(void);

	void Update(float fElapsedTime);

	void ReloadSequenceGroup(void);

	void ReloadSequenceGroupWalker(AnimationNode * node);

	void UpdateSequenceGroup(float fElapsedTime);

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

typedef boost::shared_ptr<AnimationRoot> AnimationRootPtr;

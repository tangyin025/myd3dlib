#pragma once

#include "ResourceBundle.h"

class AnimationNode;

typedef boost::shared_ptr<AnimationNode> AnimationNodePtr;

class AnimationNodeSequence;

class Actor;

class Animator
{
public:
	Actor * m_Actor;

	ResourceBundle<my::OgreSkeletonAnimation> m_SkeletonRes;

	AnimationNodePtr m_Node;

	my::TransformList m_DualQuats;

	typedef std::multimap<std::string, AnimationNodeSequence *> SequenceGroupMap;

	SequenceGroupMap m_SeqGroups;

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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	void AddToSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	void RemoveFromSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	SequenceGroupMap::iterator FindFromSequenceGroup(const std::string & name, AnimationNodeSequence * sequence);

	void UpdateGroupTime(const std::string & name, AnimationNodeSequence * sequence);
};

typedef boost::shared_ptr<Animator> AnimatorPtr;

class AnimationNode
{
public:
	Animator * m_Owner;

protected:
	AnimationNode(void)
		: m_Owner(NULL)
	{
	}

public:
	AnimationNode(Animator * Owner)
		: m_Owner(Owner)
	{
	}

	virtual ~AnimationNode(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	}

	virtual void OnSetOwner(void)
	{
	}

	virtual void Tick(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

class AnimationNodeSequence : public AnimationNode
{
public:
	float m_Time;

	std::string m_Name;

	std::string m_Root;

	std::string m_Group;

protected:
	AnimationNodeSequence(void)
		: m_Time(0)
	{
	}

public:
	AnimationNodeSequence(Animator * Owner)
		: AnimationNode(Owner)
		, m_Time(0)
	{
		OnSetOwner();
	}

	~AnimationNodeSequence(void)
	{
		if (m_Owner && !m_Group.empty())
		{
			m_Owner->RemoveFromSequenceGroup(m_Group, this);
		}
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

	virtual void Tick(float fElapsedTime);

	void Advance(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;

	float GetLength(void) const;
};

typedef boost::shared_ptr<AnimationNodeSequence> AnimationNodeSequencePtr;

class AnimationNodeBlend : public AnimationNode
{
public:
	typedef boost::array<AnimationNodePtr, 2> AnimationNodePtrArray;

	AnimationNodePtrArray m_Childs;

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
		: AnimationNode(Owner)
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
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual void OnSetOwner(void);

	void SetActiveChild(unsigned int ActiveChild, float BlendTime);

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

	virtual void Tick(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNodeBlend> AnimationNodeBlendPtr;

class AnimationNodeBlendBySpeed : public AnimationNodeBlend
{
public:
	float m_Speed0;

protected:
	AnimationNodeBlendBySpeed(void)
		: m_Speed0(0.1f)
	{
	}

public:
	AnimationNodeBlendBySpeed(Animator * Owner)
		: AnimationNodeBlend(Owner)
		, m_Speed0(0.1f)
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

	virtual void Tick(float fElapsedTime);
};

typedef boost::shared_ptr<AnimationNodeBlendBySpeed> AnimationNodeBlendBySpeedPtr;

#pragma once

#include "ResourceBundle.h"

class Animator;

class AnimationNode
{
public:
	Animator * m_Owner;

public:
	AnimationNode(void);

	virtual ~AnimationNode(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
	}

	virtual void SetOwner(Animator * Owner);

	virtual void Advance(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNode> AnimationNodePtr;

class Character;

class Animator
{
public:
	Character * m_Character;

	ResourceBundle<my::OgreSkeletonAnimation> m_SkeletonRes;

	AnimationNodePtr m_Node;

	my::TransformList m_DualQuats;

public:
	Animator(void);

	virtual ~Animator(void);

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
};

typedef boost::shared_ptr<Animator> AnimatorPtr;

class AnimationNodeSequence : public AnimationNode
{
public:
	float m_Time;

	std::string m_Name;

	std::string m_Root;

public:
	AnimationNodeSequence(void);

	~AnimationNodeSequence(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_Name);
		ar & BOOST_SERIALIZATION_NVP(m_Root);
	}

	virtual void Advance(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNodeSequence> AnimationNodeSequencePtr;

class AnimationNodeBlend : public AnimationNode
{
public:
	typedef boost::array<AnimationNodePtr, 2> AnimationNodePtrArray;

	AnimationNodePtrArray m_Childs;

	float m_BlendTime;

	float m_Weight;

	float m_TargetWeight;

	unsigned int m_ActiveChild;

public:
	AnimationNodeBlend(void);

	~AnimationNodeBlend(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNode);
		ar & BOOST_SERIALIZATION_NVP(m_Childs);
		ar & BOOST_SERIALIZATION_NVP(m_BlendTime);
		ar & BOOST_SERIALIZATION_NVP(m_Weight);
		ar & BOOST_SERIALIZATION_NVP(m_TargetWeight);
		ar & BOOST_SERIALIZATION_NVP(m_ActiveChild);
	}

	void SetActiveChild(unsigned int ActiveChild, float BlendTime);

	virtual void SetOwner(Animator * Owner);

	virtual void Advance(float fElapsedTime);

	virtual my::BoneList & GetPose(my::BoneList & pose) const;
};

typedef boost::shared_ptr<AnimationNodeBlend> AnimationNodeBlendPtr;

class AnimationNodeBlendBySpeed : public AnimationNodeBlend
{
public:
	float m_Speed0;

public:
	AnimationNodeBlendBySpeed(void);

	~AnimationNodeBlendBySpeed(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AnimationNodeBlend);
		ar & BOOST_SERIALIZATION_NVP(m_Speed0);
	}

	virtual void Advance(float fElapsedTime);
};

typedef boost::shared_ptr<AnimationNodeBlendBySpeed> AnimationNodeBlendBySpeedPtr;

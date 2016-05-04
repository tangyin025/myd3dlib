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

	virtual void Advance(float duration);

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

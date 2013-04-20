#pragma once

class Character
{
public:
	my::OgreMeshPtr m_Mesh;

	my::OgreSkeletonAnimationPtr m_Skeleton;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	enum State
	{
		StateIdle,
	};

	State m_State;

	float m_StateTime;

	my::BoneIndexSet m_leafNodeSet;

	my::BoneHierarchy m_leafedBoneHierarchy;

	my::BoneList m_animPose;

	my::BoneList m_incrementedPose;

	my::BoneList m_hierarchyBoneList;

	my::BoneList m_hierarchyBoneList2;

	my::TransformList m_dualQuaternionList;

public:
	Character(void)
		: m_Position(0,0,0)
		, m_Rotation(0,0,0,1)
		, m_Scale(1,1,1)
		, m_State(StateIdle)
		, m_StateTime(0)
	{
	}

	~Character(void)
	{
	}

	void OnFrameMove(double fTime, float fElapsedTime);
};

typedef boost::shared_ptr<Character> CharacterPtr;

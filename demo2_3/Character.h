#pragma once

#include "EffectMesh.h"
#include <mySkeleton.h>

class Character
{
public:
	typedef std::vector<EffectMeshPtr> EffectMeshPtrList;

	EffectMeshPtrList m_meshLOD;

	typedef std::vector<my::OgreSkeletonAnimationPtr> OgreSkeletonAnimationPtrList;

	OgreSkeletonAnimationPtrList m_skeletonLOD;

	int m_LODLevel;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	enum State
	{
		StateIdle,
	};

	State m_state;

	float m_stateTime;

	my::BoneList m_animPose;

	my::BoneList m_incrementedPose;

	my::BoneList m_hierarchyBoneList;

	my::BoneList m_hierarchyBoneList2;

	my::TransformList m_dualQuaternionList;

public:
	Character(void)
		: m_LODLevel(0)
		, m_Position(0,0,0)
		, m_Rotation(0,0,0,1)
		, m_state(StateIdle)
		, m_stateTime(0)
	{
	}

	~Character(void)
	{
	}

	void InsertMeshLOD(EffectMeshPtr mesh)
	{
		m_meshLOD.push_back(mesh);
	}

	void InsertSkeletonLOD(my::OgreSkeletonAnimationPtr skeleton)
	{
		m_skeletonLOD.push_back(skeleton);
	}

	void Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime);
};

typedef boost::shared_ptr<Character> CharacterPtr;

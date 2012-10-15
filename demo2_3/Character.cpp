#include "StdAfx.h"
#include "Character.h"
#include "GameState.h"

void Character::OnFrameMove(double fTime, float fElapsedTime)
{
	const float totalTime = m_skeletonLOD[m_LODLevel]->GetAnimation("walk").GetTime();

	m_StateTime = fmod(m_StateTime + fElapsedTime, totalTime);

	// 获取叶子结点
	m_leafedBoneHierarchy.clear();
	m_leafedBoneHierarchy.resize(m_skeletonLOD[m_LODLevel]->m_boneHierarchy.size());
	m_skeletonLOD[m_LODLevel]->BuildLeafedHierarchy(
		m_leafedBoneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"),
		my::BoneIndexSet());

	// 获取当前动画
	m_animPose.clear();
	m_animPose.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size(), my::Bone(my::Quaternion::Identity(), my::Vector3(0,0,0)));
	m_skeletonLOD[m_LODLevel]->BuildAnimationPose(
		m_animPose,
		m_leafedBoneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"),
		"walk", m_StateTime);

	// 固定根节点的z轴移动
	m_animPose[m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01")].m_position.z = 0;

	// 将当前动画和绑定动作叠加
	m_incrementedPose.clear();
	m_incrementedPose.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_animPose.Increment(
		m_incrementedPose,
		m_skeletonLOD[m_LODLevel]->m_boneBindPose,
		m_skeletonLOD[m_LODLevel]->m_boneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"));

	// 为绑定动作生成层次化的骨骼列表（列表中子骨骼的数据将包含父骨骼的变换）
	m_hierarchyBoneList.clear();
	m_hierarchyBoneList.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_skeletonLOD[m_LODLevel]->m_boneBindPose.BuildHierarchyBoneList(
		m_hierarchyBoneList,
		m_skeletonLOD[m_LODLevel]->m_boneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"));

	// 为目标动作生成层次化的骨骼列表
	m_hierarchyBoneList2.clear();
	m_hierarchyBoneList2.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_incrementedPose.BuildHierarchyBoneList(
		m_hierarchyBoneList2,
		m_skeletonLOD[m_LODLevel]->m_boneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"));

	// 将绑定动作及目标动作的骨骼列表生成双四元式（绑定动作不再需要逆变换，双四元式会进行处理）
	m_dualQuaternionList.clear();
	m_dualQuaternionList.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_hierarchyBoneList.BuildDualQuaternionList(m_dualQuaternionList, m_hierarchyBoneList2);
}

void Character::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	m_meshLOD[m_LODLevel]->Draw(pd3dDevice, fElapsedTime);
}

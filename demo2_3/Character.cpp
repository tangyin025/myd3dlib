#include "StdAfx.h"
#include "Character.h"
#include "GameState.h"

void Character::OnFrameMove(double fTime, float fElapsedTime)
{
	const float totalTime = m_skeletonLOD[m_LODLevel]->GetAnimation("walk").GetTime();

	m_StateTime = fmod(m_StateTime + fElapsedTime, totalTime);

	// ��ȡҶ�ӽ��
	m_leafedBoneHierarchy.clear();
	m_leafedBoneHierarchy.resize(m_skeletonLOD[m_LODLevel]->m_boneHierarchy.size());
	m_skeletonLOD[m_LODLevel]->BuildLeafedHierarchy(
		m_leafedBoneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"),
		my::BoneIndexSet());

	// ��ȡ��ǰ����
	m_animPose.clear();
	m_animPose.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size(), my::Bone(my::Quaternion::Identity(), my::Vector3(0,0,0)));
	m_skeletonLOD[m_LODLevel]->BuildAnimationPose(
		m_animPose,
		m_leafedBoneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"),
		"walk", m_StateTime);

	// �̶����ڵ��z���ƶ�
	m_animPose[m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01")].m_position.z = 0;

	// ����ǰ�����Ͱ󶨶�������
	m_incrementedPose.clear();
	m_incrementedPose.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_animPose.Increment(
		m_incrementedPose,
		m_skeletonLOD[m_LODLevel]->m_boneBindPose,
		m_skeletonLOD[m_LODLevel]->m_boneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"));

	// Ϊ�󶨶������ɲ�λ��Ĺ����б��б����ӹ��������ݽ������������ı任��
	m_hierarchyBoneList.clear();
	m_hierarchyBoneList.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_skeletonLOD[m_LODLevel]->m_boneBindPose.BuildHierarchyBoneList(
		m_hierarchyBoneList,
		m_skeletonLOD[m_LODLevel]->m_boneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"));

	// ΪĿ�궯�����ɲ�λ��Ĺ����б�
	m_hierarchyBoneList2.clear();
	m_hierarchyBoneList2.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_incrementedPose.BuildHierarchyBoneList(
		m_hierarchyBoneList2,
		m_skeletonLOD[m_LODLevel]->m_boneHierarchy,
		m_skeletonLOD[m_LODLevel]->GetBoneIndex("Bip01"));

	// ���󶨶�����Ŀ�궯���Ĺ����б�����˫��Ԫʽ���󶨶���������Ҫ��任��˫��Ԫʽ����д���
	m_dualQuaternionList.clear();
	m_dualQuaternionList.resize(m_skeletonLOD[m_LODLevel]->m_boneBindPose.size());
	m_hierarchyBoneList.BuildDualQuaternionList(m_dualQuaternionList, m_hierarchyBoneList2);
}

void Character::Draw(IDirect3DDevice9 * pd3dDevice, float fElapsedTime)
{
	m_meshLOD[m_LODLevel]->Draw(pd3dDevice, fElapsedTime);
}

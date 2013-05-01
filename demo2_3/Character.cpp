#include "StdAfx.h"
#include "Character.h"
#include "GameState.h"

void Character::OnFrameMove(double fTime, float fElapsedTime)
{
	const float totalTime = m_Skeleton->GetAnimation("walk").GetTime();

	m_StateTime = fmod(m_StateTime + fElapsedTime, totalTime);

	// ��ȡҶ�ӽ��
	m_leafedBoneHierarchy.clear();
	m_leafedBoneHierarchy.resize(m_Skeleton->m_boneHierarchy.size());
	m_Skeleton->BuildLeafedHierarchy(
		m_leafedBoneHierarchy,
		m_Skeleton->GetBoneIndex("Bip01"));

	// ��ȡ��ǰ����
	m_animPose.clear();
	m_animPose.resize(m_Skeleton->m_boneBindPose.size(), my::Bone(my::Quaternion::Identity(), my::Vector3(0,0,0)));
	m_Skeleton->BuildAnimationPose(
		m_animPose,
		m_leafedBoneHierarchy,
		m_Skeleton->GetBoneIndex("Bip01"),
		"walk", m_StateTime);

	// �̶����ڵ��z���ƶ�
	m_animPose[m_Skeleton->GetBoneIndex("Bip01")].m_position.z = 0;

	// ����ǰ�����Ͱ󶨶�������
	m_incrementedPose.clear();
	m_incrementedPose.resize(m_Skeleton->m_boneBindPose.size());
	m_animPose.Increment(
		m_incrementedPose,
		m_Skeleton->m_boneBindPose,
		m_Skeleton->m_boneHierarchy,
		m_Skeleton->GetBoneIndex("Bip01"));

	// Ϊ�󶨶������ɲ�λ��Ĺ����б��б����ӹ��������ݽ������������ı任��
	m_hierarchyBoneList.clear();
	m_hierarchyBoneList.resize(m_Skeleton->m_boneBindPose.size());
	m_Skeleton->m_boneBindPose.BuildHierarchyBoneList(
		m_hierarchyBoneList,
		m_Skeleton->m_boneHierarchy,
		m_Skeleton->GetBoneIndex("Bip01"));

	// ΪĿ�궯�����ɲ�λ��Ĺ����б�
	m_hierarchyBoneList2.clear();
	m_hierarchyBoneList2.resize(m_Skeleton->m_boneBindPose.size());
	m_incrementedPose.BuildHierarchyBoneList(
		m_hierarchyBoneList2,
		m_Skeleton->m_boneHierarchy,
		m_Skeleton->GetBoneIndex("Bip01"));

	// ���󶨶�����Ŀ�궯���Ĺ����б�����˫��Ԫʽ���󶨶���������Ҫ��任��˫��Ԫʽ����д���
	m_dualQuaternionList.clear();
	m_dualQuaternionList.resize(m_Skeleton->m_boneBindPose.size());
	m_hierarchyBoneList.BuildDualQuaternionList(m_dualQuaternionList, m_hierarchyBoneList2);
}


#include "StdAfx.h"
#include "myD3dlib.h"

namespace my
{
	void BoneHierarchy::InsertSibling(int root_i, int sibling_i)
	{
		_ASSERT(root_i >= 0 && root_i < (int)size());
		_ASSERT(sibling_i >= 0 && sibling_i < (int)size());

		reference node = operator[](root_i);
		if(node.m_sibling >= 0)
		{
			InsertSibling(node.m_sibling, sibling_i);
		}
		else
		{
			node.m_sibling = sibling_i;
		}
	}

	void BoneHierarchy::InsertChild(int root_i, int child_i)
	{
		_ASSERT(root_i >= 0 && root_i < (int)size());
		_ASSERT(child_i >= 0 && child_i < (int)size());

		reference node = operator[](root_i);
		if(node.m_child >= 0)
		{
			InsertSibling(node.m_child, child_i);
		}
		else
		{
			node.m_child = child_i;
		}
	}

	BoneList & BoneList::Lerp(
		BoneList & boneList,
		const BoneList & rhs,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		float t) const
	{
		_ASSERT(boneList.size() == size());
		_ASSERT(boneList.size() == rhs.size());

		boneList[root_i] = operator[](root_i).Lerp(rhs[root_i], t);

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			Lerp(boneList, rhs, boneHierarchy, node.m_sibling, t);
		}

		if(node.m_child >= 0)
		{
			Lerp(boneList, rhs, boneHierarchy, node.m_child, t);
		}

		return boneList;
	}

	BoneTransformList & BuildBoneTransformList(
		BoneTransformList & boneTransformList,
		const BoneList & boneList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		const Matrix4 & rootTransform /*= Matrix4::identity*/)
	{
		_ASSERT(boneList.size() == boneTransformList.size());
		_ASSERT(boneList.size() == boneHierarchy.size());

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		boneTransformList[root_i] = Matrix4::Translation(boneList[root_i].m_position) * Matrix4::RotationQuaternion(boneList[root_i].m_rotation) * rootTransform;

		if(node.m_sibling >= 0)
		{
			BuildBoneTransformList(boneTransformList, boneList, boneHierarchy, node.m_sibling, rootTransform);
		}

		if(node.m_child >= 0)
		{
			BuildBoneTransformList(boneTransformList, boneList, boneHierarchy, node.m_child, boneTransformList[root_i]);
		}

		return boneTransformList;
	}

	BoneTransformList & BuildInverseBoneTransformList(
		BoneTransformList & boneTransformList,
		const BoneList & boneList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		const Matrix4 & inverseRootTransform /*= Matrix4::identity*/)
	{
		_ASSERT(boneList.size() == boneTransformList.size());
		_ASSERT(boneList.size() == boneHierarchy.size());

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		boneTransformList[root_i] = inverseRootTransform * Matrix4::Translation(-boneList[root_i].m_position) * Matrix4::RotationQuaternion(boneList[root_i].m_rotation.inverse());

		if(node.m_sibling >= 0)
		{
			BuildInverseBoneTransformList(boneTransformList, boneList, boneHierarchy, node.m_sibling, inverseRootTransform);
		}

		if(node.m_child >= 0)
		{
			BuildInverseBoneTransformList(boneTransformList, boneList, boneHierarchy, node.m_child, boneTransformList[root_i]);
		}

		return boneTransformList;
	}

	Bone BoneTrack::GetPoseBone(float time)
	{
		_ASSERT(!empty());

		const_iterator key_iter = begin();
		if(time < key_iter->m_time)
		{
			return *key_iter;
		}

		key_iter++;
		for(; key_iter != end(); key_iter++)
		{
			if(time < key_iter->m_time)
			{
				const_iterator prev_key_iter = key_iter - 1;
				return prev_key_iter->Lerp(*key_iter, (time - prev_key_iter->m_time) / (key_iter->m_time - prev_key_iter->m_time));
			}
		}

		return *key_iter;
	}

	BoneList & BoneAnimation::GetPose(
		BoneList & boneList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		float time)
	{
		_ASSERT(boneList.size() == size());
		_ASSERT(boneList.size() == boneHierarchy.size());

		boneList[root_i] = operator[](root_i).GetPoseBone(time);

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			GetPose(boneList, boneHierarchy, node.m_sibling, time);
		}

		if(node.m_child >= 0)
		{
			GetPose(boneList, boneHierarchy, node.m_child, time);
		}

		return boneList;
	}
}

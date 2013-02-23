#pragma once

#include <vector>
#include "myMath.h"
#include <set>
#include <hash_map>
#include <boost/shared_ptr.hpp>
#include "mySingleton.h"

namespace my
{
	class BoneHierarchyNode
	{
	public:
		int m_sibling;

		int m_child;

	public:
		BoneHierarchyNode(void)
			: m_sibling(-1)
			, m_child(-1)
		{
		}
	};

	typedef std::set<int> BoneIndexSet;

	class BoneHierarchy
		: public std::vector<BoneHierarchyNode>
	{
	public:
		void InsertSibling(int root_i, int sibling_i);

		void InsertChild(int root_i, int child_i);

		BoneHierarchy & BuildLeafedHierarchy(
			BoneHierarchy & leafedBoneHierarchy,
			int root_i,
			const BoneIndexSet & leafNodeIndices);
	};

	class Bone
	{
	public:
		Quaternion m_rotation;

		Vector3 m_position;

	public:
		Bone(void)
		{
		}

		Bone(const Quaternion & rotation, const Vector3 & position)
			: m_rotation(rotation)
			, m_position(position)
		{
		}

		void SetRotation(const Quaternion & rotation)
		{
			m_rotation = rotation;
		}

		const Quaternion & GetRotation(void) const
		{
			return m_rotation;
		}

		void SetPosition(const Vector3 & position)
		{
			m_position = position;
		}

		const Vector3 & GetPosition(void) const
		{
			return m_position;
		}

		Bone Increment(const Bone & rhs) const
		{
			return Bone(
				m_rotation * rhs.m_rotation,
				m_position + rhs.m_position);
		}

		Bone IncrementSelf(const Bone & rhs)
		{
			m_rotation *= rhs.m_rotation;
			m_position += rhs.m_position;
			return *this;
		}

		Bone Lerp(const Bone & rhs, float t) const
		{
			return Bone(
				m_rotation.slerp(rhs.m_rotation, t),
				m_position.lerp(rhs.m_position, t));
		}

		Bone LerpSelf(const Bone & rhs, float t)
		{
			m_rotation.slerpSelf(rhs.m_rotation, t);
			m_position.lerpSelf(rhs.m_position, t);
			return *this;
		}

		Matrix4 BuildTransform(void) const
		{
			return Matrix4::RotationQuaternion(m_rotation) * Matrix4::Translation(m_position);
		}

		Matrix4 BuildInverseTransform(void) const
		{
			return Matrix4::Translation(-m_position) * Matrix4::RotationQuaternion(m_rotation.conjugate());
		}
	};

	class TransformList
		: public std::vector<Matrix4>
	{
	public:
		TransformList & Transform(
			TransformList & hierarchyTransformList,
			const TransformList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i) const;

		TransformList & TransformSelf(
			const TransformList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i);
	};

	class BoneList
		: public std::vector<Bone>
	{
	public:
		BoneList & Increment(
			BoneList & boneList,
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i) const;

		BoneList & IncrementSelf(
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i);

		BoneList & Lerp(
			BoneList & boneList,
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float t) const;

		BoneList & LerpSelf(
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float t);

		BoneList & BuildHierarchyBoneList(
			BoneList & hierarchyBoneList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			const Quaternion & rootRotation = Quaternion(0, 0, 0, 1),
			const Vector3 & rootPosition = Vector3::zero);

		BoneList & BuildInverseHierarchyBoneList(
			BoneList & inverseHierarchyBoneList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			const Quaternion & inverseRootRotation = Quaternion(0, 0, 0, 1),
			const Vector3 & inverseRootPosition = Vector3::zero);

		TransformList & BuildTransformList(
			TransformList & transformList) const;

		TransformList & BuildTransformListTF(
			TransformList & inverseTransformList) const;

		TransformList & BuildInverseTransformList(
			TransformList & inverseTransformList) const;

		TransformList & BuildDualQuaternionList(
			TransformList & dualQuaternionList,
			const BoneList & rhs) const;

		TransformList & BuildHierarchyTransformList(
			TransformList & hierarchyTransformList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			const Matrix4 & rootTransform = Matrix4::identity);

		TransformList & BuildInverseHierarchyTransformList(
			TransformList & inverseHierarchyTransformList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			const Matrix4 & inverseRootTransform = Matrix4::identity);
	};

	class BoneKeyframe
		: public Bone
	{
	public:
		float m_time;

	public:
		BoneKeyframe(void)
			: m_time(0)
		{
		}

		BoneKeyframe(const Quaternion & rotation, const Vector3 & position, float time)
			: Bone(rotation, position)
			, m_time(time)
		{
		}

		void SetTime(float time)
		{
			m_time = time;
		}

		float GetTime(void) const
		{
			return m_time;
		}
	};

	class BoneTrack
		: public std::vector<BoneKeyframe>
	{
	public:
		Bone GetPoseBone(float time) const;
	};

	class BoneTrackList
		: public std::vector<BoneTrack>
	{
	public:
		BoneList & GetPose(
			BoneList & boneList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float time) const;
	};

	class OgreAnimation
		: public BoneTrackList
	{
	public:
		float m_time;

	public:
		OgreAnimation(void)
			: m_time(0)
		{
		}

		void SetTime(float time)
		{
			m_time = time;
		}

		float GetTime(void) const
		{
			return m_time;
		}
	};

	class OgreSkeleton
	{
	public:
		stdext::hash_map<std::string, int> m_boneNameMap;

		BoneList m_boneBindPose;

		BoneHierarchy m_boneHierarchy;

	public:
		OgreSkeleton(void)
		{
		}

		void Clear(void)
		{
			m_boneNameMap.clear();

			m_boneBindPose.clear();

			m_boneHierarchy.clear();
		}

		int GetBoneIndex(const std::string & bone_name) const
		{
			_ASSERT(m_boneNameMap.end() != m_boneNameMap.find(bone_name));

			return m_boneNameMap.find(bone_name)->second;
		}

		BoneHierarchy & BuildLeafedHierarchy(BoneHierarchy & leafedBoneHierarchy, int root_i, const BoneIndexSet & leafNodeIndices)
		{
			_ASSERT(leafedBoneHierarchy.size() >= m_boneHierarchy.size());

			return m_boneHierarchy.BuildLeafedHierarchy(leafedBoneHierarchy, root_i, leafNodeIndices);
		}
	};

	class OgreSkeletonAnimation
		: public OgreSkeleton
		, public DeviceRelatedObjectBase
	{
	public:
		stdext::hash_map<std::string, OgreAnimation> m_animationMap;

	public:
		OgreSkeletonAnimation(void)
		{
		}

		void CreateOgreSkeletonAnimation(
			LPCSTR pSrcData,
			UINT srcDataLen);

		void CreateOgreSkeletonAnimationFromFile(
			LPCTSTR pFilename);

		void OnResetDevice(void) {}

		void OnLostDevice(void) {}

		void OnDestroyDevice(void) {}

		void Clear(void)
		{
			OgreSkeleton::Clear();

			m_animationMap.clear();
		}

		const OgreAnimation & GetAnimation(const std::string & anim_name) const
		{
			_ASSERT(m_animationMap.end() != m_animationMap.find(anim_name));

			return m_animationMap.find(anim_name)->second;
		}

		BoneList & BuildAnimationPose(BoneList & pose, const BoneHierarchy & boneHierarchy, int root_i, const std::string & anim_name, float time) const
		{
			_ASSERT(pose.size() >= m_boneBindPose.size());

			return GetAnimation(anim_name).GetPose(pose, boneHierarchy, root_i, time);
		}
	};

	typedef boost::shared_ptr<OgreSkeletonAnimation> OgreSkeletonAnimationPtr;
}

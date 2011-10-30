
#pragma once

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

	class BoneHierarchy : public std::vector<BoneHierarchyNode>
	{
	public:
		void InsertSibling(int root_i, int sibling_i);

		void InsertChild(int root_i, int child_i);
	};

	class Bone
	{
	public:
		Vector3 m_position;

		Quaternion m_rotation;

	public:
		Bone(void)
		{
		}

		Bone(const Vector3 & position, const Quaternion & rotation)
			: m_position(position)
			, m_rotation(rotation)
		{
		}

	public:
		void SetPosition(const Vector3 & position)
		{
			m_position = position;
		}

		const Vector3 & GetPosition(void) const
		{
			return m_position;
		}

		void SetRotation(const Quaternion & rotation)
		{
			m_rotation = rotation;
		}

		const Quaternion & GetRotation(void) const
		{
			return m_rotation;
		}

		Bone Lerp(const Bone & rhs, float t) const
		{
			return Bone(
				m_position.lerp(rhs.m_position, t),
				m_rotation.slerp(rhs.m_rotation, t));
		}
	};

	class BoneTransformList : public std::vector<Matrix4>
	{
	};

	class BoneList : public std::vector<Bone>
	{
	public:
		BoneList & Lerp(
			BoneList & boneList,
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float t) const;

		BoneTransformList & BuildBoneTransformList(
			BoneTransformList & boneTransformList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			const Matrix4 & rootTransform = Matrix4::identity);

		BoneTransformList & BuildInverseBoneTransformList(
			BoneTransformList & boneTransformList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			const Matrix4 & inverseRootTransform = Matrix4::identity);
	};

	class BoneKeyframe : public Bone
	{
	public:
		float m_time;

	public:
		BoneKeyframe(void)
			: m_time(0)
		{
		}

		BoneKeyframe(const Vector3 & position, const Quaternion & rotation, float time)
			: Bone(position, rotation)
			, m_time(time)
		{
		}
	};

	class BoneTrack : public std::vector<BoneKeyframe>
	{
	public:
		Bone GetPoseBone(float time);
	};

	class BoneTrackList : public std::vector<BoneTrack>
	{
	public:
		BoneList & GetPose(
			BoneList & boneList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float time);
	};

	class OgreAnimation : public BoneTrackList
	{
	public:
		float m_time;
	};

	class OgreSkeleton
	{
	protected:
		std::map<std::string, int> m_boneNameMap;

		BoneList m_boneBindPose;

		BoneHierarchy m_boneHierarcy;

	public:
		OgreSkeleton(void)
		{
		}
	};

	class OgreSkeletonAnimation;

	typedef boost::shared_ptr<OgreSkeletonAnimation> OgreSkeletonAnimationPtr;

	class OgreSkeletonAnimation : public OgreSkeleton
	{
	protected:
		std::map<std::string, OgreAnimation> m_animationMap;

		OgreSkeletonAnimation(void)
		{
		}

	public:
		static OgreSkeletonAnimationPtr CreateOgreSkeletonAnimation(
			LPCSTR pSrcData,
			UINT srcDataLen);

		static OgreSkeletonAnimationPtr CreateOgreSkeletonAnimationFromFile(
			LPCTSTR pFilename);
	};
}


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

		Bone Increment(const Bone & rhs) const
		{
			return Bone(
				m_position + rhs.m_position,
				m_rotation * rhs.m_rotation);
		}

		Bone IncrementSelf(const Bone & rhs)
		{
			m_position += rhs.m_position;
			m_rotation *= rhs.m_rotation;
			return *this;
		}

		Bone Lerp(const Bone & rhs, float t) const
		{
			return Bone(
				m_position.lerp(rhs.m_position, t),
				m_rotation.slerp(rhs.m_rotation, t));
		}

		Bone LerpSelf(const Bone & rhs, float t)
		{
			m_position.lerpSelf(rhs.m_position, t);
			m_rotation.slerpSelf(rhs.m_rotation, t);
			return *this;
		}
	};

	class BoneTransformList : public std::vector<Matrix4>
	{
	public:
		BoneTransformList & Transform(
			BoneTransformList & boneTransformList,
			const BoneTransformList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i) const;

		BoneTransformList & TransformSelf(
			const BoneTransformList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i);
	};

	class BoneList : public std::vector<Bone>
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
		Bone GetPoseBone(float time) const;
	};

	class BoneTrackList : public std::vector<BoneTrack>
	{
	public:
		BoneList & GetPose(
			BoneList & boneList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float time) const;
	};

	class OgreAnimation : public BoneTrackList
	{
	public:
		float m_time;
	};

	class OgreSkeleton
	{
	public:
		std::map<std::string, int> m_boneNameMap;

		BoneList m_boneBindPose;

		BoneHierarchy m_boneHierarchy;

		int GetBoneIndex(const std::string & bone_name) const;

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
		OgreSkeletonAnimation(void)
		{
		}

	public:
		std::map<std::string, OgreAnimation> m_animationMap;

		const OgreAnimation & GetAnimation(const std::string & anim_name) const;

		BoneList & BuildAnimationPose(BoneList & pose, const std::string & bone_name, const std::string & anim_name, float time) const;

	public:
		static OgreSkeletonAnimationPtr CreateOgreSkeletonAnimation(
			LPCSTR pSrcData,
			UINT srcDataLen);

		static OgreSkeletonAnimationPtr CreateOgreSkeletonAnimationFromFile(
			LPCTSTR pFilename);
	};
}

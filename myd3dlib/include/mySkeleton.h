#pragma once

#include <vector>
#include <map>
#include <set>
#include "myMath.h"
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include "mySingleton.h"
#include "rapidxml.hpp"

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

		bool HaveSibling(int root_i, int sibling_i) const;

		bool IsChild(int root_i, int child_i) const;

		BoneHierarchy & BuildLeafedHierarchy(
			BoneHierarchy & leafedBoneHierarchy,
			int root_i,
			const BoneIndexSet & leafNodeIndices = BoneIndexSet());

		TransformList & Transform(
			TransformList & hierarchyTransformList,
			const TransformList & lhs,
			const TransformList & rhs,
			int root_i) const;
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

		Bone Increment(const Bone & rhs) const;

		Bone & IncrementSelf(const Bone & rhs);

		Bone Lerp(const Bone & rhs, float t) const;

		Bone & LerpSelf(const Bone & rhs, float t);

		Matrix4 BuildTransform(void) const;

		Matrix4 BuildInverseTransform(void) const;
	};

	class BoneList
		: public std::vector<Bone>
	{
	public:
		BoneList(void)
		{
		}

		explicit BoneList(size_type _Count)
			: vector(_Count)
		{
		}

		BoneList(size_t _Count, const Bone & _Val)
			: vector(_Count, _Val)
		{
		}

		BoneList & CopyTo(
			BoneList & boneList,
			const BoneHierarchy & boneHierarchy,
			int root_i) const;

		BoneList & Increment(
			BoneList & boneList,
			const BoneList & rhs) const;

		BoneList & Increment(
			BoneList & boneList,
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i) const;

		BoneList & IncrementSelf(
			const BoneList & rhs);

		BoneList & IncrementSelf(
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i);

		BoneList & Lerp(
			BoneList & boneList,
			const BoneList & rhs,
			float t) const;

		BoneList & Lerp(
			BoneList & boneList,
			const BoneList & rhs,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float t) const;

		BoneList & LerpSelf(
			const BoneList & rhs,
			float t);

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

		TransformList & BuildTransformList(
			TransformList & transformList) const;

		TransformList & BuildTransformListTF(
			TransformList & inverseTransformList) const;

		TransformList & BuildInverseTransformList(
			TransformList & inverseTransformList) const;

		TransformList & BuildDualQuaternionList(
			TransformList & dualQuaternionList) const;

		//TransformList & BuildHierarchyTransformList(
		//	TransformList & hierarchyTransformList,
		//	const BoneHierarchy & boneHierarchy,
		//	int root_i,
		//	const Matrix4 & rootTransform = Matrix4::identity);

		//TransformList & BuildInverseHierarchyTransformList(
		//	TransformList & inverseHierarchyTransformList,
		//	const BoneHierarchy & boneHierarchy,
		//	int root_i,
		//	const Matrix4 & inverseRootTransform = Matrix4::identity);
	};

	class OgreAnimation
		: public std::map<float, BoneList>
	{
	public:
		OgreAnimation(void)
		{
		}

		float GetLength(void) const
		{
			_ASSERT(!empty());
			return rbegin()->first;
		}

		BoneList & GetPose(
			BoneList & boneList,
			const BoneHierarchy & boneHierarchy,
			int root_i,
			float time) const;
	};

	class OgreSkeleton
		: public DeviceResourceBase
	{
	public:
		typedef boost::unordered_map<std::string, int> BoneNameMap;
		
		BoneNameMap m_boneNameMap;

		BoneList m_boneBindPose;

		BoneHierarchy m_boneHierarchy;

		BoneIndexSet m_boneRootSet;

	public:
		OgreSkeleton(void)
		{
		}

		void Clear(void);

		int GetBoneIndex(const std::string & bone_name) const;

		const char * FindBoneName(int node_i) const;

		int FindParent(int target_i) const;

		int FindParent(int parent_i, int current_i, int target_i) const;

		BoneHierarchy & BuildLeafedHierarchy(
			BoneHierarchy & leafedBoneHierarchy,
			int root_i,
			const BoneIndexSet & leafNodeIndices = BoneIndexSet());
	};

	class OgreSkeletonAnimation
		: public OgreSkeleton
	{
	public:
		typedef boost::unordered_map<std::string, OgreAnimation> OgreAnimationMap;

		OgreAnimationMap m_animationMap;

	public:
		OgreSkeletonAnimation(void)
		{
		}

		void CreateOgreSkeletonAnimation(
			const rapidxml::xml_node<char> * node_root);

		void CreateOgreSkeletonAnimationFromMemory(
			LPSTR pSrcData,
			UINT srcDataLen);

		void CreateOgreSkeletonAnimationFromFile(
			LPCTSTR pFilename);

		static void ParseBasePoseAndNameMap(
			BoneList & base_pose,
			BoneNameMap & name_map,
			const rapidxml::xml_node<char> * node_bones);

		void AddOgreSkeletonAnimation(
			const rapidxml::xml_node<char> * node_animation,
			const BoneList & rhs_base_pose,
			const BoneNameMap & rhs_name_map);

		void AddOgreSkeletonAnimationFromMemory(
			LPSTR pSrcData,
			UINT srcDataLen);

		void AddOgreSkeletonAnimationFromFile(const char * path);

		void SaveOgreSkeletonAnimation(const char * path);

		void Transform(const my::Matrix4 & trans);

		void Clear(void);

		const OgreAnimation * GetAnimation(const std::string & anim_name) const;
	};

	typedef boost::shared_ptr<OgreSkeletonAnimation> OgreSkeletonAnimationPtr;
}

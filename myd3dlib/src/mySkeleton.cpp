
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

	TransformList & TransformList::Transform(
		TransformList & hierarchyTransformList,
		const TransformList & rhs,
		const BoneHierarchy & boneHierarchy,
		int root_i) const
	{
		_ASSERT(hierarchyTransformList.size() == size());
		_ASSERT(hierarchyTransformList.size() == rhs.size());

		hierarchyTransformList[root_i] = operator[](root_i) * rhs[root_i];

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			Transform(hierarchyTransformList, rhs, boneHierarchy, node.m_sibling);
		}

		if(node.m_child >= 0)
		{
			Transform(hierarchyTransformList, rhs, boneHierarchy, node.m_child);
		}

		return hierarchyTransformList;
	}

	TransformList & TransformList::TransformSelf(
		const TransformList & rhs,
		const BoneHierarchy & boneHierarchy,
		int root_i)
	{
		_ASSERT(size() == rhs.size());

		operator[](root_i) *= rhs[root_i];

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			TransformSelf(rhs, boneHierarchy, node.m_sibling);
		}

		if(node.m_child >= 0)
		{
			TransformSelf(rhs, boneHierarchy, node.m_child);
		}

		return *this;
	}

	BoneList & BoneList::Increment(
		BoneList & boneList,
		const BoneList & rhs,
		const BoneHierarchy & boneHierarchy,
		int root_i) const
	{
		_ASSERT(boneList.size() == size());
		_ASSERT(boneList.size() == rhs.size());

		boneList[root_i] = operator[](root_i).Increment(rhs[root_i]);

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			Increment(boneList, rhs, boneHierarchy, node.m_sibling);
		}

		if(node.m_child >= 0)
		{
			Increment(boneList, rhs, boneHierarchy, node.m_child);
		}

		return boneList;
	}

	BoneList & BoneList::IncrementSelf(
		const BoneList & rhs,
		const BoneHierarchy & boneHierarchy,
		int root_i)
	{
		_ASSERT(size() == rhs.size());

		operator[](root_i).IncrementSelf(rhs[root_i]);

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			IncrementSelf(rhs, boneHierarchy, node.m_sibling);
		}

		if(node.m_child >= 0)
		{
			IncrementSelf(rhs, boneHierarchy, node.m_child);
		}

		return *this;
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

	BoneList & BoneList::LerpSelf(
		const BoneList & rhs,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		float t)
	{
		_ASSERT(size() == rhs.size());

		operator[](root_i).LerpSelf(rhs[root_i], t);

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		if(node.m_sibling >= 0)
		{
			LerpSelf(rhs, boneHierarchy, node.m_sibling, t);
		}

		if(node.m_child >= 0)
		{
			LerpSelf(rhs, boneHierarchy, node.m_child, t);
		}

		return *this;
	}

	TransformList & HierarchyBoneList::BuildTransformList(TransformList & transformList) const
	{
		_ASSERT(transformList.size() == size());

		for(size_t i = 0; i < size(); i++)
		{
			const_reference bone = operator[](i);
			transformList[i] = Matrix4::RotationQuaternion(bone.m_rotation) * Matrix4::Translation(bone.m_position);
		}

		return transformList;
	}

	TransformList & HierarchyBoneList::BuildInverseTransformList(TransformList & inverseTransformList) const
	{
		_ASSERT(inverseTransformList.size() == size());

		for(size_t i = 0; i < size(); i++)
		{
			const_reference bone = operator[](i);
			inverseTransformList[i] = Matrix4::Translation(-bone.m_position) * Matrix4::RotationQuaternion(bone.m_rotation.conjugate());
		}

		return inverseTransformList;
	}

	HierarchyBoneList & BoneList::BuildHierarchyBoneList(
		HierarchyBoneList & hierarchyBoneList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		const Quaternion & rootRotation /*= Quaternion(0, 0, 0, 1)*/,
		const Vector3 & rootPosition /*= Vector3::zero)*/)
	{
		_ASSERT(hierarchyBoneList.size() == size());
		_ASSERT(hierarchyBoneList.size() == boneHierarchy.size());

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		const_reference bone = operator[](root_i);
		HierarchyBoneList::reference hier_bone = hierarchyBoneList[root_i];
		hier_bone.m_rotation = bone.m_rotation * rootRotation;
		hier_bone.m_position = bone.m_position.transform(rootRotation) + rootPosition;

		if(node.m_sibling >= 0)
		{
			BuildHierarchyBoneList(hierarchyBoneList, boneHierarchy, node.m_sibling, rootRotation, rootPosition);
		}

		if(node.m_child >= 0)
		{
			BuildHierarchyBoneList(hierarchyBoneList, boneHierarchy, node.m_child, hier_bone.m_rotation, hier_bone.m_position);
		}

		return hierarchyBoneList;
	}

	TransformList & BoneList::BuildHierarchyTransformList(
		TransformList & hierarchyTransformList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		const Matrix4 & rootTransform /*= Matrix4::identity*/)
	{
		_ASSERT(hierarchyTransformList.size() == size());
		_ASSERT(hierarchyTransformList.size() == boneHierarchy.size());

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		const_reference bone = operator[](root_i);
		hierarchyTransformList[root_i] = Matrix4::RotationQuaternion(bone.m_rotation) * Matrix4::Translation(bone.m_position) * rootTransform;

		if(node.m_sibling >= 0)
		{
			BuildHierarchyTransformList(hierarchyTransformList, boneHierarchy, node.m_sibling, rootTransform);
		}

		if(node.m_child >= 0)
		{
			BuildHierarchyTransformList(hierarchyTransformList, boneHierarchy, node.m_child, hierarchyTransformList[root_i]);
		}

		return hierarchyTransformList;
	}

	TransformList & BoneList::BuildInverseHierarchyTransformList(
		TransformList & inverseHierarchyTransformList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		const Matrix4 & inverseRootTransform /*= Matrix4::identity*/)
	{
		_ASSERT(inverseHierarchyTransformList.size() == size());
		_ASSERT(inverseHierarchyTransformList.size() == boneHierarchy.size());

		BoneHierarchy::const_reference node = boneHierarchy[root_i];
		const_reference bone = operator[](root_i);
		inverseHierarchyTransformList[root_i] = inverseRootTransform * Matrix4::Translation(-bone.m_position) * Matrix4::RotationQuaternion(bone.m_rotation.conjugate());

		if(node.m_sibling >= 0)
		{
			BuildInverseHierarchyTransformList(inverseHierarchyTransformList, boneHierarchy, node.m_sibling, inverseRootTransform);
		}

		if(node.m_child >= 0)
		{
			BuildInverseHierarchyTransformList(inverseHierarchyTransformList, boneHierarchy, node.m_child, inverseHierarchyTransformList[root_i]);
		}

		return inverseHierarchyTransformList;
	}

	Bone BoneTrack::GetPoseBone(float time) const
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

		return back();
	}

	BoneList & BoneTrackList::GetPose(
		BoneList & boneList,
		const BoneHierarchy & boneHierarchy,
		int root_i,
		float time) const
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

	OgreSkeletonAnimationPtr OgreSkeletonAnimation::CreateOgreSkeletonAnimation(
		LPCSTR pSrcData,
		UINT srcDataLen)
	{
		std::string xmlStr(pSrcData, srcDataLen);

		rapidxml::xml_document<char> doc;
		try
		{
			doc.parse<0>(&xmlStr[0]);
		}
		catch(rapidxml::parse_error & e)
		{
			THROW_CUSEXCEPTION(e.what());
		}

		rapidxml::xml_node<char> * node_root = &doc;
		DEFINE_XML_NODE_SIMPLE(skeleton, root);
		DEFINE_XML_NODE_SIMPLE(bones, skeleton);
		DEFINE_XML_NODE_SIMPLE(bone, bones);

		int bone_i = 0;
		OgreSkeletonAnimationPtr ogre_skel_anim(new OgreSkeletonAnimation());
		for(; node_bone != NULL; node_bone = node_bone->next_sibling(), bone_i++)
		{
			DEFINE_XML_ATTRIBUTE_INT_SIMPLE(id, bone);
			if(id != bone_i)
			{
				THROW_CUSEXCEPTION(str_printf("invalid bone id: %d", id));
			}

			DEFINE_XML_ATTRIBUTE_SIMPLE(name, bone);
			if(ogre_skel_anim->m_boneNameMap.end() != ogre_skel_anim->m_boneNameMap.find(attr_name->value()))
			{
				THROW_CUSEXCEPTION(str_printf("bone name \"%s\"have already existed", attr_name->value()));
			}

			ogre_skel_anim->m_boneNameMap.insert(std::make_pair(attr_name->value(), id));

			DEFINE_XML_NODE_SIMPLE(position, bone);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(x, position);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(y, position);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(z, position);

			DEFINE_XML_NODE_SIMPLE(rotation, bone);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(angle, rotation);
			DEFINE_XML_NODE_SIMPLE(axis, rotation);
			float axis_x, axis_y, axis_z;
			rapidxml::xml_attribute<char> * attr_axis_x, * attr_axis_y, * attr_axis_z;
			DEFINE_XML_ATTRIBUTE_FLOAT(axis_x, attr_axis_x, node_axis, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(axis_y, attr_axis_y, node_axis, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(axis_z, attr_axis_z, node_axis, z);

			ogre_skel_anim->m_boneBindPose.push_back(
				Bone(Quaternion::RotationAxis(Vector3(axis_x, axis_y, -axis_z), -angle), Vector3(x, y, -z)));

			_ASSERT(id == ogre_skel_anim->m_boneBindPose.size() - 1);
		}

		DEFINE_XML_NODE_SIMPLE(bonehierarchy, skeleton);
		DEFINE_XML_NODE_SIMPLE(boneparent, bonehierarchy);

		ogre_skel_anim->m_boneHierarchy.resize(ogre_skel_anim->m_boneBindPose.size());
		for(; node_boneparent != NULL; node_boneparent = node_boneparent->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_SIMPLE(bone, boneparent);
			if(ogre_skel_anim->m_boneNameMap.end() == ogre_skel_anim->m_boneNameMap.find(attr_bone->value()))
			{
				THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
			}

			DEFINE_XML_ATTRIBUTE_SIMPLE(parent, boneparent);
			if(ogre_skel_anim->m_boneNameMap.end() == ogre_skel_anim->m_boneNameMap.find(attr_parent->value()))
			{
				THROW_CUSEXCEPTION(str_printf("invalid bone parent name: %s", attr_parent->value()));
			}

			ogre_skel_anim->m_boneHierarchy.InsertChild(
				ogre_skel_anim->m_boneNameMap[attr_parent->value()], ogre_skel_anim->m_boneNameMap[attr_bone->value()]);
		}

		DEFINE_XML_NODE_SIMPLE(animations, skeleton);
		DEFINE_XML_NODE_SIMPLE(animation, animations);
		DEFINE_XML_NODE_SIMPLE(tracks, animation);

		for(; node_animation != NULL; node_animation = node_animation->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_SIMPLE(name, animation);
			if(ogre_skel_anim->m_animationMap.end() != ogre_skel_anim->m_animationMap.find(attr_name->value()))
			{
				THROW_CUSEXCEPTION(str_printf("animation \"%s\" have already existed", attr_name->value()));
			}

			ogre_skel_anim->m_animationMap.insert(std::make_pair(attr_name->value(), OgreAnimation()));
			OgreAnimation & anim = ogre_skel_anim->m_animationMap[attr_name->value()];

			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(length, animation);
			anim.m_time = length;
			anim.resize(ogre_skel_anim->m_boneBindPose.size());

			DEFINE_XML_NODE_SIMPLE(track, tracks);
			for(; node_track != NULL; node_track = node_track->next_sibling())
			{
				DEFINE_XML_ATTRIBUTE_SIMPLE(bone, track);
				if(ogre_skel_anim->m_boneNameMap.end() == ogre_skel_anim->m_boneNameMap.find(attr_bone->value()))
				{
					THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
				}

				BoneTrack & bone_track = anim[ogre_skel_anim->m_boneNameMap[attr_bone->value()]];

				DEFINE_XML_NODE_SIMPLE(keyframes, track);
				DEFINE_XML_NODE_SIMPLE(keyframe, keyframes);
				for(; node_keyframe != NULL; node_keyframe = node_keyframe->next_sibling())
				{
					DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(time, keyframe);

					rapidxml::xml_attribute<char> * attr_translate_x, * attr_translate_y, * attr_translate_z;
					float translate_x, translate_y, translate_z;
					DEFINE_XML_NODE_SIMPLE(translate, keyframe);
					DEFINE_XML_ATTRIBUTE_FLOAT(translate_x, attr_translate_x, node_translate, x);
					DEFINE_XML_ATTRIBUTE_FLOAT(translate_y, attr_translate_y, node_translate, y);
					DEFINE_XML_ATTRIBUTE_FLOAT(translate_z, attr_translate_z, node_translate, z);

					DEFINE_XML_NODE_SIMPLE(rotate, keyframe);
					DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(angle, rotate);

					rapidxml::xml_attribute<char> * attr_axis_x, * attr_axis_y, * attr_axis_z;
					float axis_x, axis_y, axis_z;
					DEFINE_XML_NODE_SIMPLE(axis, rotate);
					DEFINE_XML_ATTRIBUTE_FLOAT(axis_x, attr_axis_x, node_axis, x);
					DEFINE_XML_ATTRIBUTE_FLOAT(axis_y, attr_axis_y, node_axis, y);
					DEFINE_XML_ATTRIBUTE_FLOAT(axis_z, attr_axis_z, node_axis, z);

					bone_track.push_back(
						BoneKeyframe(Quaternion::RotationAxis(Vector3(axis_x, axis_y, -axis_z), -angle), Vector3(translate_x, translate_y, -translate_z), time));
				}
			}
		}

		return ogre_skel_anim;
	}

	OgreSkeletonAnimationPtr OgreSkeletonAnimation::CreateOgreSkeletonAnimationFromFile(
		LPCTSTR pFilename)
	{
		FILE * fp;
		if(0 != _tfopen_s(&fp, pFilename, _T("rb")))
		{
			THROW_CUSEXCEPTION(tstringToMString(str_printf(_T("cannot open file archive: %s"), pFilename)));
		}
		
		ArchiveStreamPtr stream(new FileArchiveStream(fp));

		CachePtr cache = ReadWholeCacheFromStream(stream);

		return CreateOgreSkeletonAnimation((LPCSTR)&(*cache)[0], cache->size());
	}
}

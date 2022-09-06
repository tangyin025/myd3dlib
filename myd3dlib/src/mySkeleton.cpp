#include "mySkeleton.h"
#include "libc.h"
#include "myResource.h"
#include <fstream>

using namespace my;

Matrix4 TransformList::BuildSkinnedDualQuaternion(const Matrix4 * DualQuaternions, DWORD DualQuaternionSize, DWORD indices, const Vector4 & weights)
{
	_ASSERT(((unsigned char*)&indices)[0] < DualQuaternionSize);
	Matrix4 m = DualQuaternions[((unsigned char*)&indices)[0]];
	Vector4 dq0 = m[0];
	Matrix4 dual = m * weights.x;

	_ASSERT(((unsigned char*)&indices)[1] < DualQuaternionSize);
	m = DualQuaternions[((unsigned char*)&indices)[1]];
	Vector4 dq = m[0];
	if (dq0.dot(dq) < 0)
		dual -= m * weights.y;
	else
		dual += m * weights.y;

	_ASSERT(((unsigned char*)&indices)[2] < DualQuaternionSize);
	m = DualQuaternions[((unsigned char*)&indices)[2]];
	dq = m[0];
	if (dq0.dot(dq) < 0)
		dual -= m * weights.z;
	else
		dual += m * weights.z;

	_ASSERT(((unsigned char*)&indices)[3] < DualQuaternionSize);
	m = DualQuaternions[((unsigned char*)&indices)[3]];
	dq = m[0];
	if (dq0.dot(dq) < 0)
		dual -= m * weights.w;
	else
		dual += m * weights.w;

	float length = dual[0].magnitude();
	dual = dual / length;
	return dual;
}

Vector3 TransformList::TransformVertexWithDualQuaternion(const Vector3 & position, const Matrix4 & dual)
{
	Vector3 outPosition = position + dual[0].xyz.cross(dual[0].xyz.cross(position) + position * dual[0].w) * 2;
	Vector3 translation = (dual[1].xyz * dual[0].w - dual[0].xyz * dual[1].w + dual[0].xyz.cross(dual[1].xyz)) * 2;
	outPosition += translation;
	return outPosition;
}

Vector3 TransformList::TransformVertexWithDualQuaternionList(const Vector3 & position, DWORD indices, const Vector4 & weights) const
{
	return TransformVertexWithDualQuaternion(position, BuildSkinnedDualQuaternion(data(), size(), indices, weights));
}

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

bool BoneHierarchy::HaveSibling(int root_i, int sibling_i) const
{
	if (root_i >= 0 && root_i < (int)size())
	{
		const_reference node = operator[](root_i);
		if (node.m_sibling == sibling_i)
		{
			return true;
		}

		return HaveSibling(node.m_sibling, sibling_i);
	}
	return false;
}

bool BoneHierarchy::IsChild(int root_i, int child_i) const
{
	if (root_i >= 0 && root_i < (int)size())
	{
		const_reference node = operator[](root_i);
		if (node.m_child == child_i)
		{
			return true;
		}

		return HaveSibling(node.m_child, child_i);
	}
	return false;
}

int BoneHierarchy::FindParent(int child_i) const
{
	int node_i = 0;
	for (; node_i < size(); node_i++)
	{
		if (operator[](node_i).m_child == child_i)
		{
			return node_i;
		}

		if (operator[](node_i).m_sibling == child_i)
		{
			return FindParent(node_i);
		}
	}

	return -1;
}

BoneHierarchy & BoneHierarchy::BuildLeafedHierarchy(
	BoneHierarchy & leafedBoneHierarchy,
	int root_i,
	const BoneIndexSet & leafNodeIndices) const
{
	_ASSERT(leafedBoneHierarchy.size() == size());

	reference node = (leafedBoneHierarchy[root_i] = operator[](root_i));

	if(leafNodeIndices.end() == leafNodeIndices.find(root_i))
	{
		int node_i = node.m_child;
		for(; node_i >= 0; node_i = operator[](node_i).m_sibling)
		{
			BuildLeafedHierarchy(leafedBoneHierarchy, node_i, leafNodeIndices);
		}
	}
	else
		leafedBoneHierarchy[root_i].m_child = -1;

	return leafedBoneHierarchy;
}

TransformList & BoneHierarchy::Transform(
	TransformList & hierarchyTransformList,
	const TransformList & lhs,
	const TransformList & rhs,
	int root_i) const
{
	_ASSERT(hierarchyTransformList.size() == lhs.size());
	_ASSERT(hierarchyTransformList.size() == rhs.size());

	hierarchyTransformList[root_i] = lhs[root_i] * rhs[root_i];

	int node_i = operator[](root_i).m_child;
	for(; node_i >= 0; node_i = operator[](node_i).m_sibling)
	{
		Transform(hierarchyTransformList, lhs, rhs, node_i);
	}

	return hierarchyTransformList;
}

BoneList & BoneList::CopyTo(
	BoneList & boneList,
	const BoneHierarchy & boneHierarchy,
	int root_i) const
{
	_ASSERT(boneList.size() == size());

	boneList[root_i] = operator[](root_i);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		CopyTo(boneList, boneHierarchy, node_i);
	}

	return boneList;
}

BoneList & BoneList::Increment(
	BoneList & boneList,
	const BoneList & rhs) const
{
	_ASSERT(boneList.size() == size());
	_ASSERT(boneList.size() == rhs.size());

	unsigned int node_i = 0;
	for (; node_i < size(); node_i++)
	{
		boneList[node_i] = operator[](node_i).Increment(rhs[node_i]);
	}

	return boneList;
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

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		Increment(boneList, rhs, boneHierarchy, node_i);
	}

	return boneList;
}

BoneList & BoneList::IncrementSelf(
	const BoneList & rhs)
{
	_ASSERT(size() == rhs.size());

	unsigned int node_i = 0;
	for (; node_i < size(); node_i++)
	{
		operator[](node_i).IncrementSelf(rhs[node_i]);
	}

	return *this;
}

BoneList & BoneList::IncrementSelf(
	const BoneList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i)
{
	_ASSERT(size() == rhs.size());

	operator[](root_i).IncrementSelf(rhs[root_i]);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		IncrementSelf(rhs, boneHierarchy, node_i);
	}

	return *this;
}

BoneList & BoneList::Lerp(
	BoneList & boneList,
	const BoneList & rhs,
	float t) const
{
	_ASSERT(boneList.size() == size());
	_ASSERT(boneList.size() == rhs.size());

	unsigned int node_i = 0;
	for (; node_i < size(); node_i++)
	{
		boneList[node_i] = operator[](node_i).Lerp(rhs[node_i], t);
	}

	return boneList;
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

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		Lerp(boneList, rhs, boneHierarchy, node_i, t);
	}

	return boneList;
}

BoneList & BoneList::LerpSelf(
	const BoneList & rhs,
	float t)
{
	_ASSERT(size() == rhs.size());

	unsigned int node_i = 0;
	for (; node_i < size(); node_i++)
	{
		operator[](node_i).LerpSelf(rhs[node_i], t);
	}

	return *this;
}

BoneList & BoneList::LerpSelf(
	const BoneList & rhs,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	float t)
{
	_ASSERT(size() == rhs.size());

	operator[](root_i).LerpSelf(rhs[root_i], t);

	int node_i = boneHierarchy[root_i].m_child;
	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		LerpSelf(rhs, boneHierarchy, node_i, t);
	}

	return *this;
}

BoneList & BoneList::BuildHierarchyBoneList(
	BoneList & hierarchyBoneList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	const Bone & parent)
{
	_ASSERT(hierarchyBoneList.size() == size());
	_ASSERT(hierarchyBoneList.size() == boneHierarchy.size());

	const_reference bone = operator[](root_i);
	reference hier_bone = hierarchyBoneList[root_i];
	hier_bone = bone.Transform(parent);

	int node_i = boneHierarchy[root_i].m_child;
	for (; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
	{
		BuildHierarchyBoneList(hierarchyBoneList, boneHierarchy, node_i, hier_bone);
	}

	return hierarchyBoneList;
}

TransformList & BoneList::BuildTransformList(
	TransformList & transformList) const
{
	_ASSERT(transformList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		transformList[i] = operator[](i).BuildTransform();
	}

	return transformList;
}

TransformList & BoneList::BuildTransformListTF(
	TransformList & inverseTransformList) const
{
	_ASSERT(inverseTransformList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		const_reference bone = operator[](i);
		inverseTransformList[i] = Matrix4::Translation(bone.m_position) * Matrix4::RotationQuaternion(bone.m_rotation);
	}

	return inverseTransformList;
}

TransformList & BoneList::BuildInverseTransformList(
	TransformList & inverseTransformList) const
{
	_ASSERT(inverseTransformList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		inverseTransformList[i] = operator[](i).BuildInverseTransform();
	}

	return inverseTransformList;
}

TransformList & BoneList::BuildDualQuaternionList(
	TransformList & dualQuaternionList) const
{
	_ASSERT(dualQuaternionList.size() == size());

	for(size_t i = 0; i < size(); i++)
	{
		const_reference bone = operator [](i);
		TransformList::reference dual = dualQuaternionList[i];

		dual._11 = bone.m_rotation.x ; 
		dual._12 = bone.m_rotation.y ; 
		dual._13 = bone.m_rotation.z ; 
		dual._14 = bone.m_rotation.w ; 
		dual._21 = 0.5f * ( bone.m_position.x * bone.m_rotation.w + bone.m_position.y * bone.m_rotation.z - bone.m_position.z * bone.m_rotation.y ) ; 
		dual._22 = 0.5f * (-bone.m_position.x * bone.m_rotation.z + bone.m_position.y * bone.m_rotation.w + bone.m_position.z * bone.m_rotation.x ) ; 
		dual._23 = 0.5f * ( bone.m_position.x * bone.m_rotation.y - bone.m_position.y * bone.m_rotation.x + bone.m_position.z * bone.m_rotation.w ) ; 
		dual._24 = -0.5f * (bone.m_position.x * bone.m_rotation.x + bone.m_position.y * bone.m_rotation.y + bone.m_position.z * bone.m_rotation.z ) ; 
	}

	return dualQuaternionList;
}
//
//TransformList & BoneList::BuildHierarchyTransformList(
//	TransformList & hierarchyTransformList,
//	const BoneHierarchy & boneHierarchy,
//	int root_i,
//	const Matrix4 & rootTransform)
//{
//	_ASSERT(hierarchyTransformList.size() == size());
//	_ASSERT(hierarchyTransformList.size() == boneHierarchy.size());
//
//	const_reference bone = operator[](root_i);
//	hierarchyTransformList[root_i] = Matrix4::RotationQuaternion(bone.m_rotation) * Matrix4::Translation(bone.m_position) * rootTransform;
//
//	int node_i = boneHierarchy[root_i].m_child;
//	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
//	{
//		BuildHierarchyTransformList(hierarchyTransformList, boneHierarchy, node_i, hierarchyTransformList[root_i]);
//	}
//
//	return hierarchyTransformList;
//}
//
//TransformList & BoneList::BuildInverseHierarchyTransformList(
//	TransformList & inverseHierarchyTransformList,
//	const BoneHierarchy & boneHierarchy,
//	int root_i,
//	const Matrix4 & inverseRootTransform)
//{
//	_ASSERT(inverseHierarchyTransformList.size() == size());
//	_ASSERT(inverseHierarchyTransformList.size() == boneHierarchy.size());
//
//	const_reference bone = operator[](root_i);
//	inverseHierarchyTransformList[root_i] = inverseRootTransform * Matrix4::Translation(-bone.m_position) * Matrix4::RotationQuaternion(bone.m_rotation.conjugate());
//
//	int node_i = boneHierarchy[root_i].m_child;
//	for(; node_i >= 0; node_i = boneHierarchy[node_i].m_sibling)
//	{
//		BuildInverseHierarchyTransformList(inverseHierarchyTransformList, boneHierarchy, node_i, inverseHierarchyTransformList[root_i]);
//	}
//
//	return inverseHierarchyTransformList;
//}

BoneList & OgreAnimation::GetPose(
	BoneList & boneList,
	const BoneHierarchy & boneHierarchy,
	int root_i,
	float time) const
{
	_ASSERT(!empty());

	const_iterator iter = lower_bound(time);
	if (iter != begin())
	{
		if (iter != end())
		{
			const_reverse_iterator prev_iter(iter);
			prev_iter->second.Lerp(boneList, iter->second, boneHierarchy, root_i, (time - prev_iter->first) / (iter->first - prev_iter->first));
			return boneList;
		}

		rbegin()->second.CopyTo(boneList, boneHierarchy, root_i);
		return boneList;
	}

	iter->second.CopyTo(boneList, boneHierarchy, root_i);
	return boneList;
}

void OgreSkeleton::Clear(void)
{
	m_boneNameMap.clear();

	m_boneBindPose.clear();

	m_boneHierarchy.clear();
}

int OgreSkeleton::GetBoneIndex(const char * bone_name) const
{
	BoneNameMap::const_iterator name_iter = m_boneNameMap.find(bone_name);
	if (name_iter != m_boneNameMap.end())
	{
		return name_iter->second;
	}
	return -1;
}

const char * OgreSkeleton::FindBoneName(int node_i) const
{
	BoneNameMap::const_iterator name_iter = m_boneNameMap.begin();
	for (; name_iter != m_boneNameMap.end(); name_iter++)
	{
		if (name_iter->second == node_i)
		{
			return name_iter->first.c_str();
		}
	}
	return "";
}

BoneHierarchy & OgreSkeleton::BuildLeafedHierarchy(
	BoneHierarchy & leafedBoneHierarchy,
	int root_i,
	const BoneIndexSet & leafNodeIndices)
{
	_ASSERT(leafedBoneHierarchy.size() >= m_boneHierarchy.size());

	return m_boneHierarchy.BuildLeafedHierarchy(leafedBoneHierarchy, root_i, leafNodeIndices);
}

void OgreSkeletonAnimation::CreateOgreSkeletonAnimation(
	const rapidxml::xml_node<char> * node_root)
{
	Clear();

	DEFINE_XML_NODE_SIMPLE(skeleton, root);
	DEFINE_XML_NODE_SIMPLE(bones, skeleton);

	ParseBasePoseAndNameMap(m_boneBindPose, m_boneNameMap, node_bones);

	DEFINE_XML_NODE_SIMPLE(bonehierarchy, skeleton);
	DEFINE_XML_NODE_SIMPLE(boneparent, bonehierarchy);

	m_boneHierarchy.resize(m_boneBindPose.size());
	for(; node_boneparent != NULL; node_boneparent = node_boneparent->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(bone, boneparent);
		if(m_boneNameMap.end() == m_boneNameMap.find(attr_bone->value()))
		{
			THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
		}

		DEFINE_XML_ATTRIBUTE_SIMPLE(parent, boneparent);
		if(m_boneNameMap.end() == m_boneNameMap.find(attr_parent->value()))
		{
			continue;
		}

		m_boneHierarchy.InsertChild(
			m_boneNameMap[attr_parent->value()], m_boneNameMap[attr_bone->value()]);
	}

	rapidxml::xml_node<char> * node_animations = node_skeleton->first_node("animations");
	if (node_animations != NULL)
	{
		DEFINE_XML_NODE_SIMPLE(animation, animations);
		for (; node_animation != NULL; node_animation = node_animation->next_sibling())
		{
			AddOgreSkeletonAnimation(node_animation, m_boneBindPose, m_boneNameMap);
		}
	}

	// calculate root set
	m_boneRootSet.clear();
	std::vector<int> boneRootRef(m_boneHierarchy.size(), 0);
	for (unsigned int i = 0; i < m_boneHierarchy.size(); i++)
	{
		int child_i = m_boneHierarchy[i].m_child;
		if (-1 != child_i)
		{
			boneRootRef[child_i]++;
		}
		int sibling_i = m_boneHierarchy[i].m_sibling;
		if (-1 != sibling_i)
		{
			boneRootRef[sibling_i]++;
		}
	}
	for (unsigned int i = 0; i < m_boneHierarchy.size(); i++)
	{
		if (0 == boneRootRef[i])
		{
			m_boneRootSet.insert(i);
		}
	}
}

void OgreSkeletonAnimation::CreateOgreSkeletonAnimationFromMemory(
	LPSTR pSrcData,
	UINT srcDataLen)
{
	_ASSERT(0 == pSrcData[srcDataLen-1]);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(pSrcData);
	}
	catch(rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(e.what());
	}

	CreateOgreSkeletonAnimation(&doc);
}

void OgreSkeletonAnimation::CreateOgreSkeletonAnimationFromFile(
	LPCTSTR pFilename)
{
	CachePtr cache = FileIStream::Open(pFilename)->GetWholeCache();
	cache->push_back(0);
	CreateOgreSkeletonAnimationFromMemory((char *)&(*cache)[0], cache->size());
}

void OgreSkeletonAnimation::ParseBasePoseAndNameMap(
	BoneList & base_pose,
	BoneNameMap & name_map,
	const rapidxml::xml_node<char> * node_bones)
{
	base_pose.clear();
	name_map.clear();
	DEFINE_XML_NODE_SIMPLE(bone, bones);
	for (; node_bone != NULL; node_bone = node_bone->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_INT_SIMPLE(id, bone);
		DEFINE_XML_ATTRIBUTE_SIMPLE(name, bone);
		if (name_map.end() != name_map.find(attr_name->value()))
		{
			THROW_CUSEXCEPTION(str_printf("bone name \"%s\"have already existed", attr_name->value()));
		}
		name_map.insert(std::make_pair(attr_name->value(), id));

		DEFINE_XML_NODE_SIMPLE(position, bone);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(x, position);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(y, position);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(z, position);

		DEFINE_XML_NODE_SIMPLE(rotation, bone);
		DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(angle, rotation);
		DEFINE_XML_NODE_SIMPLE(axis, rotation);
		float axis_x, axis_y, axis_z;
		rapidxml::xml_attribute<char> * attr_axis_x, *attr_axis_y, *attr_axis_z;
		DEFINE_XML_ATTRIBUTE_FLOAT(axis_x, attr_axis_x, node_axis, x);
		DEFINE_XML_ATTRIBUTE_FLOAT(axis_y, attr_axis_y, node_axis, y);
		DEFINE_XML_ATTRIBUTE_FLOAT(axis_z, attr_axis_z, node_axis, z);

		if (id >= (int)base_pose.size())
		{
			base_pose.resize(id + 1, Bone(Vector3(0, 0, 0)));
		}
		base_pose[id] = Bone(Vector3(x, y, z), Quaternion::RotationAxis(Vector3(axis_x, axis_y, axis_z), angle));
	}
}

void OgreSkeletonAnimation::AddOgreSkeletonAnimation(
	const rapidxml::xml_node<char> * node_animation,
	const BoneList & rhs_base_pose,
	const BoneNameMap & rhs_name_map)
{
	DEFINE_XML_ATTRIBUTE_SIMPLE(name, animation);
	OgreAnimationMap::iterator anim_iter = m_animationMap.find(attr_name->value());
	if (anim_iter != m_animationMap.end())
	{
		m_animationMap.erase(anim_iter);
	}

	anim_iter = m_animationMap.insert(m_animationMap.begin(), std::make_pair(attr_name->value(), OgreAnimation()));

	DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(length, animation);

	DEFINE_XML_NODE_SIMPLE(tracks, animation);
	DEFINE_XML_NODE_SIMPLE(track, tracks);
	for (; node_track != NULL; node_track = node_track->next_sibling())
	{
		DEFINE_XML_ATTRIBUTE_SIMPLE(bone, track);

		BoneNameMap::const_iterator name_iter = m_boneNameMap.find(attr_bone->value());
		if (name_iter == m_boneNameMap.end())
		{
			THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
		}

		BoneNameMap::const_iterator rhs_name_iter = rhs_name_map.find(attr_bone->value());
		if (rhs_name_iter == rhs_name_map.end())
		{
			THROW_CUSEXCEPTION(str_printf("invalid bone name: %s", attr_bone->value()));
		}

		DEFINE_XML_NODE_SIMPLE(keyframes, track);
		DEFINE_XML_NODE_SIMPLE(keyframe, keyframes);
		for (; node_keyframe != NULL; node_keyframe = node_keyframe->next_sibling())
		{
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(time, keyframe);
			BoneList & pose = anim_iter->second[time];
			pose.resize(m_boneHierarchy.size(), Bone(Vector3(0, 0, 0)));
			Bone & bone = pose[name_iter->second];

			rapidxml::xml_attribute<char> * attr_translate_x, *attr_translate_y, *attr_translate_z;
			float translate_x, translate_y, translate_z;
			DEFINE_XML_NODE_SIMPLE(translate, keyframe);
			DEFINE_XML_ATTRIBUTE_FLOAT(translate_x, attr_translate_x, node_translate, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(translate_y, attr_translate_y, node_translate, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(translate_z, attr_translate_z, node_translate, z);

			DEFINE_XML_NODE_SIMPLE(rotate, keyframe);
			DEFINE_XML_ATTRIBUTE_FLOAT_SIMPLE(angle, rotate);

			rapidxml::xml_attribute<char> * attr_axis_x, *attr_axis_y, *attr_axis_z;
			float axis_x, axis_y, axis_z;
			DEFINE_XML_NODE_SIMPLE(axis, rotate);
			DEFINE_XML_ATTRIBUTE_FLOAT(axis_x, attr_axis_x, node_axis, x);
			DEFINE_XML_ATTRIBUTE_FLOAT(axis_y, attr_axis_y, node_axis, y);
			DEFINE_XML_ATTRIBUTE_FLOAT(axis_z, attr_axis_z, node_axis, z);

			bone.m_rotation = Quaternion::RotationAxis(Vector3(axis_x, axis_y, axis_z), angle);
			bone.m_position = Vector3(translate_x, translate_y, translate_z);
			bone.IncrementSelf(rhs_base_pose[rhs_name_iter->second]);
		}
	}
}

void OgreSkeletonAnimation::AddOgreSkeletonAnimationFromMemory(
	LPSTR pSrcData,
	UINT srcDataLen)
{
	_ASSERT(0 == pSrcData[srcDataLen - 1]);

	rapidxml::xml_document<char> doc;
	try
	{
		doc.parse<0>(pSrcData);
	}
	catch (rapidxml::parse_error & e)
	{
		THROW_CUSEXCEPTION(e.what());
	}

	rapidxml::xml_node<char> * node_root = &doc;
	DEFINE_XML_NODE_SIMPLE(skeleton, root);
	DEFINE_XML_NODE_SIMPLE(bones, skeleton);

	BoneList base_pose;
	BoneNameMap name_map;
	ParseBasePoseAndNameMap(base_pose, name_map, node_bones);

	rapidxml::xml_node<char> * node_animations = node_skeleton->first_node("animations");
	if (node_animations != NULL)
	{
		DEFINE_XML_NODE_SIMPLE(animation, animations);
		for (; node_animation != NULL; node_animation = node_animation->next_sibling())
		{
			AddOgreSkeletonAnimation(node_animation, base_pose, name_map);
		}
	}
}

void OgreSkeletonAnimation::AddOgreSkeletonAnimationFromFile(const char * path)
{
	CachePtr cache = my::ResourceMgr::getSingleton().OpenIStream(path)->GetWholeCache();
	cache->push_back(0);
	AddOgreSkeletonAnimationFromMemory((char *)&(*cache)[0], cache->size());
}

void OgreSkeletonAnimation::SaveOgreSkeletonAnimation(const char * path)
{
	std::ofstream ofs(path);
	ofs << "<skeleton>\n";
	ofs << "\t<bones>\n";
	for (unsigned int i = 0; i < m_boneBindPose.size(); i++)
	{
		ofs << "\t\t<bone id=\"" << i << "\" name=\"" << FindBoneName(i) << "\">\n";
		const Bone & bone = m_boneBindPose[i];
		ofs << "\t\t\t<position x=\"" << bone.m_position.x << "\" y=\"" << bone.m_position.y << "\" z=\"" << bone.m_position.z << "\"/>\n";
		Vector3 axis; float angle;
		bone.m_rotation.toAxisAngle(axis, angle);
		ofs << "\t\t\t<rotation angle=\"" << angle << "\">\n";
		ofs << "\t\t\t\t<axis x=\"" << axis.x << "\" y=\"" << axis.y << "\" z=\"" << axis.z << "\"/>\n";
		ofs << "\t\t\t</rotation>\n";
		ofs << "\t\t</bone>\n";
	}
	ofs << "\t</bones>\n";
	ofs << "\t<bonehierarchy>\n";
	BoneNameMap::const_iterator name_iter = m_boneNameMap.begin();
	for (; name_iter != m_boneNameMap.end(); name_iter++)
	{
		for (unsigned int i = 0; i < m_boneHierarchy.size(); i++)
		{
			if (m_boneHierarchy.IsChild(i, name_iter->second))
			{
				ofs << "\t\t<boneparent bone=\"" << name_iter->first << "\" parent=\"" << FindBoneName(i) << "\"/>\n";
			}
		}
	}
	ofs << "\t</bonehierarchy>\n";
	ofs << "\t<animations>\n";
	OgreAnimationMap::const_iterator anim_iter = m_animationMap.begin();
	for (; anim_iter != m_animationMap.end(); anim_iter++)
	{
		ofs << "\t\t<animation name=\"" << anim_iter->first << "\" length=\"" << anim_iter->second.GetLength() << "\">\n";
		ofs << "\t\t\t<tracks>\n";
		BoneNameMap::const_iterator name_iter = m_boneNameMap.begin();
		for (; name_iter != m_boneNameMap.end(); name_iter++)
		{
			ofs << "\t\t\t\t<track bone=\"" << name_iter->first << "\">\n";
			ofs << "\t\t\t\t\t<keyframes>\n";
			OgreAnimation::const_iterator frame_iter = anim_iter->second.begin();
			for (; frame_iter != anim_iter->second.end(); frame_iter++)
			{
				ofs << "\t\t\t\t\t\t<keyframe time=\"" << frame_iter->first << "\">\n";
				const Bone & bone = frame_iter->second[name_iter->second];
				const Bone & base_bone = m_boneBindPose[name_iter->second];
				Bone anim_bone(bone.m_position - base_bone.m_position, bone.m_rotation * base_bone.m_rotation.conjugate());
				ofs << "\t\t\t\t\t\t\t<translate x=\"" << anim_bone.m_position.x << "\" y=\"" << anim_bone.m_position.y << "\" z=\"" << anim_bone.m_position.x << "\"/>\n";
				Vector3 axis; float angle;
				anim_bone.m_rotation.toAxisAngle(axis, angle);
				ofs << "\t\t\t\t\t\t\t<rotate angle=\"" << angle << "\">\n";
				ofs << "\t\t\t\t\t\t\t\t<axis x=\"" << axis.x << "\" y=\"" << axis.y << "\" z=\"" << axis.z << "\"/>\n";
				ofs << "\t\t\t\t\t\t\t</rotate>\n";
				ofs << "\t\t\t\t\t\t\t<scale x=\"1\" y=\"1\" z=\"1\"/>\n";
				ofs << "\t\t\t\t\t\t</keyframe>\n";
			}
			ofs << "\t\t\t\t\t</keyframes>\n";
			ofs << "\t\t\t\t</track>\n";
		}
		ofs << "\t\t\t</tracks>\n";
		ofs << "\t\t</animation>\n";
	}
	ofs << "\t</animations>\n";
	ofs << "</skeleton>\n";
}

void OgreSkeletonAnimation::AdjustAnimationRoot(const Bone & pose)
{
	OgreAnimationMap::iterator anim_iter = m_animationMap.begin();
	for (; anim_iter != m_animationMap.end(); anim_iter++)
	{
		OgreAnimation::iterator pose_iter = anim_iter->second.begin();
		for (; pose_iter != anim_iter->second.end(); pose_iter++)
		{
			BoneIndexSet::const_iterator id_iter = m_boneRootSet.begin();
			for (; id_iter != m_boneRootSet.end(); id_iter++)
			{
				Bone& bone = pose_iter->second[*id_iter];
				bone.TransformSelf(pose);
			}
		}
	}
}

void OgreSkeletonAnimation::Clear(void)
{
	OgreSkeleton::Clear();

	m_animationMap.clear();
}

const OgreAnimation * OgreSkeletonAnimation::GetAnimation(const std::string & anim_name) const
{
	OgreAnimationMap::const_iterator anim_iter = m_animationMap.find(anim_name);
	if (anim_iter != m_animationMap.end())
	{
		return &anim_iter->second;
	}
	return NULL;
}

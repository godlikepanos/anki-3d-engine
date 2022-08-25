// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup resource
/// @{

const U32 MAX_CHILDREN_PER_BONE = 12;

/// Skeleton bone
class Bone
{
	friend class SkeletonResource; ///< For loading

public:
	Bone() = default;

	~Bone() = default;

	const String& getName() const
	{
		return m_name;
	}

	const Mat4& getTransform() const
	{
		return m_transform;
	}

	const Mat4& getVertexTransform() const
	{
		return m_vertTrf;
	}

	U32 getIndex() const
	{
		return m_idx;
	}

	ConstWeakArray<Bone*> getChildren() const
	{
		return ConstWeakArray<Bone*>((m_childrenCount) ? &m_children[0] : nullptr, m_childrenCount);
	}

	const Bone* getParent() const
	{
		return m_parent;
	}

private:
	String m_name; ///< The name of the bone

	Mat4 m_transform; ///< See the class notes.
	Mat4 m_vertTrf;

	U32 m_idx;

	Bone* m_parent = nullptr;
	Array<Bone*, MAX_CHILDREN_PER_BONE> m_children = {};
	U8 m_childrenCount = 0;

	void destroy(ResourceAllocator<U8> alloc)
	{
		m_name.destroy(alloc);
	}
};

/// It contains the bones with their position and hierarchy
///
/// XML file format:
///
/// @code
/// <skeleton>
/// 	<bones>
/// 		<bone>
/// 			<name>X</name>
/// 			<transform>16 floats</transform>
/// 			<boneTransform>16 floats</boneTransform>
/// 			[<parent>bone_name</parent>]
/// 		<bone>
///         ...
/// 	</bones>
/// </skeleton>
/// @endcode
class SkeletonResource : public ResourceObject
{
public:
	SkeletonResource(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~SkeletonResource();

	/// Load file
	Error load(const ResourceFilename& filename, Bool async);

	const DynamicArray<Bone>& getBones() const
	{
		return m_bones;
	}

	const Bone* tryFindBone(CString name) const
	{
		// TODO Optimize
		for(const Bone& b : m_bones)
		{
			if(b.m_name == name)
			{
				return &b;
			}
		}

		return nullptr;
	}

	const Bone& getRootBoneAt(const U32 idx) const
	{
		return m_bones[m_rootBonesIdxs[idx]];
	}

	const U32 getRootBoneCount() const
	{
		return m_rootBonesIdxs.getSize();
	}

private:
	DynamicArray<Bone> m_bones;
	DynamicArray<U32> m_rootBonesIdxs;
};
/// @}

} // end namespace anki

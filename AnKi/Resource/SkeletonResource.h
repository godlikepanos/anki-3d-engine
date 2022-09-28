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

constexpr U32 kMaxChildrenPerBone = 8;

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
	Array<Bone*, kMaxChildrenPerBone> m_children = {};
	U8 m_childrenCount = 0;

	void destroy(HeapMemoryPool& pool)
	{
		m_name.destroy(pool);
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

	const Bone& getRootBone() const
	{
		return m_bones[m_rootBoneIdx];
	}

private:
	DynamicArray<Bone> m_bones;
	U32 m_rootBoneIdx = kMaxU32;
};
/// @}

} // end namespace anki

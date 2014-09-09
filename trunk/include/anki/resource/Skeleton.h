// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_SKELETON_H
#define ANKI_RESOURCE_SKELETON_H

#include "anki/resource/Common.h"
#include "anki/Math.h"

namespace anki {

/// Skeleton bone
struct Bone
{
	friend class Skeleton; ///< For loading

public:
	Bone(ResourceAllocator<U8>& alloc)
	:	m_name(alloc)
	{}

	const ResourceString& getName() const
	{
		return m_name;
	}

	const Mat4& getTransform() const
	{
		return m_transform;
	}

private:
	ResourceString m_name; ///< The name of the bone
	static const U32 MAX_CHILDS_PER_BONE = 4; ///< Please dont change this

	// see the class notes
	Mat4 m_transform;
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
/// 			<transform></transform>
/// 		<bone>
///         ...
/// 	</bones>
/// </skeleton>
/// @endcode
class Skeleton
{
public:
	/// Load file
	void load(const CString& filename, ResourceInitializer& init);

	/// @name Accessors
	/// @{
	const ResourceVector<Bone>& getBones() const
	{
		return m_bones;
	}
	/// @}

private:
	ResourceVector<Bone> m_bones;
};

} // end namespace


#endif

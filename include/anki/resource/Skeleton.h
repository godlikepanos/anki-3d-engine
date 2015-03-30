// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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

	/// @privatesection
	/// @{
	void _destroy(ResourceAllocator<U8> alloc)
	{
		m_name.destroy(alloc);
	}
	/// @}

private:
	String m_name; ///< The name of the bone
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
	Skeleton(ResourceAllocator<U8>& alloc);

	~Skeleton();

	/// Load file
	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceInitializer& init);

	/// @name Accessors
	/// @{
	const DArray<Bone>& getBones() const
	{
		return m_bones;
	}
	/// @}

private:
	ResourceAllocator<U8> m_alloc;
	DArray<Bone> m_bones;
};

} // end namespace anki


#endif

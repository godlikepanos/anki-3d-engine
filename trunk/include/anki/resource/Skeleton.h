#ifndef ANKI_RESOURCE_SKELETON_H
#define ANKI_RESOURCE_SKELETON_H

#include "anki/Math.h"
#include "anki/util/Vector.h"

namespace anki {

/// Skeleton bone
struct Bone
{
	friend class Skeleton; ///< For loading

public:
	/// @name Accessors
	/// @{
	const std::string& getName() const
	{
		return name;
	}

	const Mat4& getTransform() const
	{
		return transform;
	}
	/// @}

private:
	std::string name; ///< The name of the bone
	static const U32 MAX_CHILDS_PER_BONE = 4; ///< Please dont change this

	// see the class notes
	Mat4 transform;
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
	void load(const char* filename);

	/// @name Accessors
	/// @{
	const Vector<Bone>& getBones() const
	{
		return bones;
	}
	/// @}

private:
	Vector<Bone> bones;
};


} // end namespace


#endif

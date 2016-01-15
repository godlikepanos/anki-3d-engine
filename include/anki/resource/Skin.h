// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#if 0
#include <anki/resource/ResourceManager.h>
#include <anki/resource/Model.h>

namespace anki {

/// Skin resource
///
/// XML file format:
/// @code<skin>
/// 	<model>path/to/model.mdl</model>
/// 	<skeleton>path/to/skeleton.skel</skeleton>
/// 	<skelAnims>
/// 		<skelAnim>path/to/anim0.sanim</skelAnim>
/// 		...
/// 		<skelAnim>...</skelAnim>
/// 	</skelAnims>
/// </skin>@endcode
class Skin
{
public:
	typedef Vector<SkelAnimResourcePointer> SkeletonAnimationsContainer;

	Skin();
	~Skin();

	/// Implements Resource::load
	void load(const char*);

	/// @name Accessors
	/// @{
	const Model& getModel() const
	{
		return *model;
	}

	const Skeleton& getSkeleton() const
	{
		return *skeleton;
	}

	const SkeletonAnimationsContainer& getSkeletonAnimations() const
	{
		return skelAnims;
	}
	/// @}

private:
	/// @name The resources
	/// @{
	ModelResourcePtr model;
	SkeletonResourcePtr skeleton; ///< The skeleton
	/// The standard skeleton animations
	SkeletonAnimationsContainer skelAnims;
	/// @}
};

} // end namespace anki
#endif

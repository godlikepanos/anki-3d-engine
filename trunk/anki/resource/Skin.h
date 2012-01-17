#ifndef ANKI_RESOURCE_SKIN_H
#define ANKI_RESOURCE_SKIN_H

#include "anki/resource/Resource.h"
#include "anki/resource/Model.h"


namespace anki {


class Skeleton;
class SkelAnim;


/// XML file format:
/// @code
/// <skin>
/// 	<model>path/to/model.mdl</model>
/// 	<skeleton>path/to/skeleton.skel</skeleton>
/// 	<skelAnims>
/// 		<skelAnim>path/to/anim0.sanim</skelAnim>
/// 		...
/// 		<skelAnim>...</skelAnim>
/// 	</skelAnims>
/// </skin>
/// @endcode
class Skin
{
public:
	typedef std::vector<SkelAnimResourcePointer> SkeletonAnimationsContainer;

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
	ModelResourcePointer model;
	SkeletonResourcePointer skeleton; ///< The skeleton
	/// The standard skeleton animations
	SkeletonAnimationsContainer skelAnims;
	/// @}
};


} // end namespace


#endif

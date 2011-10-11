#ifndef ANKI_RESOURCE_SKIN_H
#define ANKI_RESOURCE_SKIN_H

#include "anki/resource/RsrcPtr.h"
#include "anki/resource/Model.h"


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
		Skin();
		~Skin();

		/// Implements Resource::load
		void load(const char*);

		/// @name Accessors
		/// @{
		const Model& getModel() const;
		const boost::ptr_vector<ModelPatch>& getModelPatches() const;
		const Skeleton& getSkeleton() const;
		const std::vector<RsrcPtr<SkelAnim> >& getSkelAnims() const;
		/// @}

	private:
		/// @name The resources
		/// @{
		RsrcPtr<Model> model;
		RsrcPtr<Skeleton> skeleton; ///< The skeleton
		/// The standard skeleton animations
		std::vector<RsrcPtr<SkelAnim> > skelAnims;
		/// @}
};


inline const Model& Skin::getModel() const
{
	return *model;
}


inline const boost::ptr_vector<ModelPatch>& Skin::getModelPatches() const
{
	return model->getModelPatches();
}


inline const Skeleton& Skin::getSkeleton() const
{
	return *skeleton;
}


inline const std::vector<RsrcPtr<SkelAnim> >& Skin::getSkelAnims() const
{
	return skelAnims;
}


#endif

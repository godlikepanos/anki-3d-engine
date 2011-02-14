#ifndef SKIN_H
#define SKIN_H

#include "RsrcPtr.h"
#include "Model.h"


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
		/// Implements Resource::load
		void load(const char*);

		/// @name Accessors
		/// @{
		const Model& getModel() const {return *model;}
		const boost::ptr_vector<ModelPatch>& getModelPatches() const {return model->getModelPatches();}
		const Skeleton& getSkeleton() const {return *skeleton;}
		const Vec<RsrcPtr<SkelAnim> >& getSkelAnims() const {return skelAnims;}
		/// @}

	private:
		/// @name The resources
		/// @{
		RsrcPtr<Model> model;
		RsrcPtr<Skeleton> skeleton; ///< The skeleton
		Vec<RsrcPtr<SkelAnim> > skelAnims; ///< The standard skeleton animations
		/// @}
};


#endif

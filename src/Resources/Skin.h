#ifndef SKIN_H
#define SKIN_H

#include "Resource.h"
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
class Skin: public Resource
{
	public:
		/// Nothing special
		Skin(): Resource(RT_SKIN) {}

		/// Implements Resource::load
		void load(const char*);

		/// @name Accessors
		/// @{
		const boost::ptr_vector<ModelPatch>& getModelPatches() const {return model->getModelPatches();}
		const Skeleton& getSkeleton() const;
		const Vec<RsrcPtr<SkelAnim> >& getSkelAnims() const {return skelAnims;}
		/// @}

	private:
		RsrcPtr<Model> model;
		RsrcPtr<Skeleton> skeleton; ///< The skeleton
		Vec<RsrcPtr<SkelAnim> > skelAnims; ///< The standard skeleton animations
};


#endif

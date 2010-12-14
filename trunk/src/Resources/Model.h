#ifndef MODEL_H
#define MODEL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "Resource.h"
#include "RsrcPtr.h"
#include "Object.h"
#include "Vao.h"
#include "ModelPatch.h"


class Mesh;
class Material;
class Skeleton;
class SkelAnim;
class Scanner;


/// Model is an entity that acts as a container for other resources. Models are all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<modelPatches>
/// 		<modelPatch>
/// 			<mesh>path/to/mesh.mesh</mesh>
/// 			<material>path/to/material.mtl</material>
/// 			<dpMaterial>path/to/dp.mtl</dpMaterial>
/// 		</modelPatch>
/// 		...
/// 		<modelPatch>...</modelPatch>
/// 	</modelPatches>
/// 	<skeleton>path/to/skeleton.skel</skeleton>
/// 	<skelAnims>
/// 		<skelAnim>path/to/anim0.sanim</skelAnim>
/// 		...
/// 		<skelAnim>...</skelAnim>
/// 	</skelAnims>
/// </model>
/// @endcode
///
/// Requirements:
/// - If the materials need texture coords or/and vertex weights then mesh should have them
/// - The skeleton and skelAnims are optional
/// - Its an error to have skelAnims without skeleton
class Model: public Resource
{
	public:
		Model(): Resource(RT_MODEL) {}

		void load(const char* filename);

		/// @name Accessors
		/// @{
		const boost::ptr_vector<ModelPatch>& getModelPatches() const {return modelPatches;}
		const Skeleton& getSkeleton() const;
		const Vec<RsrcPtr<SkelAnim> >& getSkelAnims() const {return skelAnims;}
		/// @}

		bool hasSkeleton() const {return skeleton.get() != NULL;}

	private:
		boost::ptr_vector<ModelPatch> modelPatches; ///< The vector of SubModel
		RsrcPtr<Skeleton> skeleton; ///< The skeleton. It can be empty
		Vec<RsrcPtr<SkelAnim> > skelAnims; ///< The standard skeleton animations
};


inline const Skeleton& Model::getSkeleton() const
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return *skeleton;
}


#endif

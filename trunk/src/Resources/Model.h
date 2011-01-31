#ifndef MODEL_H
#define MODEL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "RsrcPtr.h"
#include "Vao.h"
#include "ModelPatch.h"
#include "Sphere.h"


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
/// </model>
/// @endcode
///
/// Requirements:
/// - If the materials need texture coords then mesh should have them
/// - The skeleton and skelAnims are optional
/// - Its an error to have skelAnims without skeleton
class Model
{
	public:
		Model() {}

		void load(const char* filename);

		/// @name Accessors
		/// @{
		const boost::ptr_vector<ModelPatch>& getModelPatches() const {return modelPatches;}
		const Sphere& getBoundingShape() const {return boundingShape;}
		/// @}

	private:
		boost::ptr_vector<ModelPatch> modelPatches; ///< The vector of ModelPatch
		Sphere boundingShape;
};


#endif

#ifndef ANKI_RESOURCE_MODEL_MODEL_H
#define ANKI_RESOURCE_MODEL_MODEL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "anki/resource/RsrcPtr.h"
#include "anki/gl/Vao.h"
#include "anki/resource/ModelPatch.h"
#include "anki/collision/Obb.h"


/// Model is an entity that acts as a container for other resources. Models are
/// all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<modelPatches>
/// 		<modelPatch>
/// 			<mesh>path/to/mesh.mesh</mesh>
/// 			<material>path/to/material.mtl</material>
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
		void load(const char* filename);

		/// @name Accessors
		/// @{
		const boost::ptr_vector<ModelPatch>& getModelPatches() const
		{
			return modelPatches;
		}

		const Obb& getVisibilityShape() const
		{
			return visibilityShape;
		}
		/// @}

	private:
		/// The vector of ModelPatch
		boost::ptr_vector<ModelPatch> modelPatches;
		Obb visibilityShape;
};


#endif

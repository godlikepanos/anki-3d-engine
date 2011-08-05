#ifndef MODEL_PATCH_H
#define MODEL_PATCH_H

#include "RsrcPtr.h"


class Mesh;
class Material;


/// Its one part of the many used in the Model class. Its basically a container
class ModelPatch
{
	public:
		ModelPatch();
		~ModelPatch();

		/// Load the resources
		void load(const char* meshFName, const char* mtlFName);

		/// @name Accessors
		/// @{
		const Mesh& getMesh() const {return *mesh;}
		const Material& getMaterial() const {return *mtl;}
		/// @}

	private:
		RsrcPtr<Mesh> mesh; ///< The geometry
		RsrcPtr<Material> mtl; ///< Material for MS and BS
};


#endif

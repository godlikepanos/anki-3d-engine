#ifndef MODEL_PATCH_H
#define MODEL_PATCH_H

#include "RsrcPtr.h"


class Mesh;
class Material;


/// Its one part of the many used in the Model class. Its basically a container
class ModelPatch
{
	public:
		/// Load the resources
		void load(const char* meshFName, const char* mtlFName, const char* dpMtlFName);

		/// @name Accessors
		/// @{
		const Mesh& getMesh() const {return *mesh;}
		const Material& getMaterial() const {return *material;}
		const Material& getDpMaterial() const {return *dpMaterial;}
		/// @}

		bool supportsHardwareSkinning() const;

	private:
		RsrcPtr<Mesh> mesh; ///< The geometry
		RsrcPtr<Material> material; ///< Material for MS and BS
		RsrcPtr<Material> dpMaterial; ///< Material for depth passes
};


#endif

#ifndef MODEL_PATCH_H
#define MODEL_PATCH_H

#include "anki/resource/RsrcPtr.h"


class Mesh;
class Material;


/// Its one part of the many used in the Model class. Its basically a container
class ModelPatch
{
	public:
		ModelPatch(const char* meshFName, const char* mtlFName);
		~ModelPatch();

		/// @name Accessors
		/// @{
		const Mesh& getMesh() const
		{
			return *mesh;
		}

		const Material& getMaterial() const
		{
			return *mtl;
		}
		/// @}

		bool supportsHwSkinning() const;

	private:
		RsrcPtr<Mesh> mesh; ///< The geometry
		RsrcPtr<Material> mtl; ///< Material for MS and BS

		/// Load the resources
		void load(const char* meshFName, const char* mtlFName);
};


#endif

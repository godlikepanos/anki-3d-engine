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
		void load(const char* meshFName, const char* mtlFName,
			const char* dpMtlFName);

		/// @name Accessors
		/// @{
		const Mesh& getMesh() const {return *mesh;}
		const Material& getCpMtl() const {return *cpMtl;}
		const Material& getDpMtl() const {return *dpMtl;}
		/// @}

		/// This only checks the mesh for vertex weights
		bool supportsHwSkinning() const;

		/// This checks if any of the materials need normals
		bool supportsNormals() const;

		/// This checks if any of the materials need tangents
		bool supportsTangents() const;

	private:
		RsrcPtr<Mesh> mesh; ///< The geometry
		RsrcPtr<Material> cpMtl; ///< Material for MS and BS
		RsrcPtr<Material> dpMtl; ///< Material for depth passes

		/// Checks if a mesh and a material are compatible
		static void doMeshAndMtlSanityChecks(const Mesh& mesh,
			const Material& mtl);
};


#endif

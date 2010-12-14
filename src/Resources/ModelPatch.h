#ifndef MODEL_PATCH_H
#define MODEL_PATCH_H

#include ""


/// A part of the Model
class ModelPatch
{
	public:
		/// Load the resources
		void load(const char* meshFName, const char* mtlFName, const char* dpMtlFName);

		/// Creates a VAO for an individual ModelPatch
		/// @param[in] material Needed for the shader program uniform variables
		/// @param[in] mesh For providing the VBOs
		/// @param[in,out] subModel For setting a parent to the vao
		/// @param[out] vao The output
		static void createVao(const Material& material, const Mesh& mesh, Vao& vao);

		/// @name Accessors
		/// @{
		const Mesh& getMesh() const {return *mesh;}
		const Material& getMaterial() const {return *material;}
		const Material& getDpMaterial() const {return *dpMaterial;}
		const Vao& getVao() const {return vao;}
		const Vao& getDpVao() const {return dpVao;}
		/// @}

		bool hasHwSkinning() const;

	private:
		RsrcPtr<Mesh> mesh; ///< The geometry
		RsrcPtr<Material> material; ///< Material for MS and BS
		RsrcPtr<Material> dpMaterial; ///< Material for depth passes
		Vao vao; ///< Normal VAO for MS and BS
		Vao dpVao; ///< Depth pass VAO for SM and EarlyZ
};

#endif

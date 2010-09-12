#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/variant.hpp>
#include "Common.h"
#include "Math.h"
#include "Resource.h"
#include "ShaderProg.h"
#include "Texture.h"
#include "RsrcPtr.h"


/**
 * Mesh material @ref Resource
 *
 * Every material keeps info of how to render a @ref MeshNode. Among this info it keeps the locations of attribute and
 * uniform variables. The variables can be standard or user defined. The standard variables have standard names inside
 * the shader program and we dont have to mention them in the .mtl files. The material init func scoops the shader
 * program for standard variables and keeps a pointer to the variable. The standard variables are like the GL build-in
 * variables (that we cannot longer use on GL >3) with a few additions. The user defined variables are defined and
 * values inside the .mtl file. The attribute variables cannot be user defined, the uniform on the other hand can.
 */
class Material: public Resource
{
	friend class Renderer; ///< For the setupMaterial
	friend class Ez;
	friend class Sm;
	friend class Ms;
	friend class Bs;
	friend class Mesh;
	friend class MeshNode;

	private:
		/**
		 * Standard attribute variables that are acceptable inside the @ref ShaderProg
		 */
		enum StdAttribVars
		{
			SAV_POSITION,
			SAV_TANGENT,
			SAV_NORMAL,
			SAV_TEX_COORDS,
			SAV_VERT_WEIGHT_BONES_NUM,
			SAV_VERT_WEIGHT_BONE_IDS,
			SAV_VERT_WEIGHT_WEIGHTS,
			SAV_NUM
		};

		/**
		 * Standard uniform variables
		 *
		 * After changing the enum update also:
		 * - Some statics in Material.cpp
		 * - Renderer::setupMaterial
		 * - The generic material shader (maybe)
		 */
		enum StdUniVars
		{
			// Skinning
			SUV_SKINNING_ROTATIONS,
			SUV_SKINNING_TRANSLATIONS,
			// Matrices
			SUV_MODEL_MAT,
			SUV_VIEW_MAT,
			SUV_PROJECTION_MAT,
			SUV_MODELVIEW_MAT,
			SUV_VIEWPROJECTION_MAT,
			SUV_NORMAL_MAT,
			SUV_MODELVIEWPROJECTION_MAT,
			// FAIs (for materials in blending stage)
			SUV_MS_NORMAL_FAI,
			SUV_MS_DIFFUSE_FAI,
			SUV_MS_SPECULAR_FAI,
			SUV_MS_DEPTH_FAI,
			SUV_IS_FAI,
			SUV_PPS_PRE_PASS_FAI,
			SUV_PPS_POST_PASS_FAI,
			// Other
			SUV_RENDERER_SIZE,
			SUV_SCENE_AMBIENT_COLOR,
			// num
			SUV_NUM ///< The number of standard uniform variables
		};

		/**
		 * Information for the standard shader program variables
		 */
		struct StdVarInfo
		{
			const char* varName;
			GLenum dataType; ///< aka GL data type
		};

		/**
		 * Class for user defined material variables that will be passes in to the shader
		 */
		struct UserDefinedUniVar
		{
			/**
			 * Unfortunately we cannot use union because of complex classes (Vec2, Vec3 etc)
			 */
			struct Value
			{
				RsrcPtr<Texture> texture;
				float float_;
				Vec2 vec2;
				Vec3 vec3;
				Vec4 vec4;
			};

			Value value;
			const ShaderProg::UniVar* sProgVar;
		}; // end UserDefinedVar


		static StdVarInfo stdAttribVarInfos[SAV_NUM];
		static StdVarInfo stdUniVarInfos[SUV_NUM];
		const ShaderProg::AttribVar* stdAttribVars[SAV_NUM];
		const ShaderProg::UniVar* stdUniVars[SUV_NUM];
		RsrcPtr<ShaderProg> shaderProg; ///< The most important aspect of materials
		RsrcPtr<Material> dpMtl; ///< The material for depth passes. To be removed when skinning is done using transform feedback
		ptr_vector<UserDefinedUniVar> userDefinedVars;
		bool blends; ///< The entities with blending are being rendered in blending stage and those without in material stage
		int blendingSfactor;
		int blendingDfactor;
		bool depthTesting;
		bool wireframe;
		bool castsShadow; ///< Used in shadowmapping passes but not in Ez

		/**
		 * The func sweeps all the variables of the shader program to find standard shader program variables. It updates the
		 * stdAttribVars and stdUniVars arrays.
		 * @return True on success
		 */
		bool initStdShaderVars();

		bool hasHWSkinning() const;
		bool hasAlphaTesting() const;

	public:
		Material();
		bool load(const char* filename);
};


inline bool Material::hasHWSkinning() const
{
	return stdAttribVars[SAV_VERT_WEIGHT_BONES_NUM] != NULL;
}


inline bool Material::hasAlphaTesting() const
{
	return dpMtl.get()!=NULL && dpMtl->stdAttribVars[SAV_TEX_COORDS] != NULL;
}


#endif

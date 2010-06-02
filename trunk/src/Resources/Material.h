#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "Common.h"
#include "Math.h"
#include "Resource.h"
#include "ShaderProg.h"


/**
 * Mesh material @ref Resource
 *
 * Every material keeps among other things the locations of the attribute and uniform variables. The attributes come from a selection
 * of standard vertex attributes. We dont have to write these attribs in the .mtl file. The uniforms on the other hand are in two
 * categories. The standard uniforms that we dont have to write in the file and the user defined.
 */
class Material: public Resource
{
	friend class Renderer;
	friend class MeshNode;

	protected:
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
		 */
		enum StdUniVars
		{
			// Skinning
			SUV_SKINNING_ROTATIONS,
			SUV_SKINNING_TRANSLATIONS,
			// Matrices
			SUV_MODELVIEW_MAT,
			SUV_PROJECTION_MAT,
			SUV_MODELVIEWPROJECTION_MAT,
			SUV_NORMAL_MAT,
			// FAIs
			SUV_MS_NORMAL_FAI,
			SUV_MS_DIFFUSE_FAI,
			SUV_MS_SPECULAR_FAI,
			SUV_MS_DEPTH_FAI,
			SUV_IS_FAI,
			SUV_PPS_FAI,
			// Other
			SUV_RENDERER_SIZE,
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
			struct Value  // unfortunately we cannot use union because of Vec3 and Vec4
			{
				Texture* texture;
				float float_;
				Vec2 vec2;
				Vec3 vec3;
				Vec4 vec4;

				Value(): texture(NULL) {}
			};

			Value value;
			const ShaderProg::UniVar* sProgVar;
		}; // end UserDefinedVar


		static StdVarInfo stdAttribVarInfos[ SAV_NUM ];
		static StdVarInfo stdUniVarInfos[ SUV_NUM ];
		const ShaderProg::AttribVar* stdAttribVars[ SAV_NUM ];
		const ShaderProg::UniVar* stdUniVars[ SUV_NUM ];
		ShaderProg* shaderProg; ///< The most important aspect of materials
		Material* dpMtl; ///< The material for depth passes. To be removed when skinning is done using tranform feedback
		Vec<UserDefinedUniVar> userDefinedVars;
		bool blends; ///< The entities with blending are being rendered in blending stage and those without in material stage
		int blendingSfactor;
		int blendingDfactor;
		bool refracts;
		bool depthTesting;
		bool wireframe;
		bool castsShadow; ///< Used in shadowmapping passes but not in EarlyZ


		/**
		 * The func sweeps all the variables of the shader program to find standard shader program variables. It updates the stdAttribVars
		 * and stdUniVars arrays.
		 * @return True on success
		 */
		bool initStdShaderVars();

	public:
		Material();
		void setup();
		bool load( const char* filename );
		void unload();

		bool hasHWSkinning() const { return stdAttribVars[ SAV_VERT_WEIGHT_BONES_NUM ] != NULL; }
		bool hasAlphaTesting() const { return dpMtl!=NULL && dpMtl->stdAttribVars[ SAV_TEX_COORDS ] != NULL; }
};


#endif

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "Common.h"
#include "Math.h"
#include "Resource.h"
#include "ShaderProg.h"


/**
 * Mesh material @ref Resource resource
 */
class Material: public Resource
{
	friend class Renderer;
	friend class MeshNode;

	protected:
		/**
		 *
		 */
		enum SpecialVar
		{
			SV_NONE,           ///< SV_NONE
			// Texture
			SV_MS_NORMAL_FAI,  ///< SV_MS_NORMAL_FAI
			SV_MS_DIFFUSE_FAI, ///< SV_MS_DIFFUSE_FAI
			SV_MS_SPECULAR_FAI,///< SV_MS_SPECULAR_FAI
			SV_MS_DEPTH_FAI,   ///< SV_MS_DEPTH_FAI
			SV_IS_FAI,         ///< SV_IS_FAI
			SV_PPS_FAI,        ///< SV_PPS_FAI
			// Vec2
			SV_RENDERER_SIZE, ///< Active renderer's width and height
			// Mat3
			SV_NORMAL_MAT,
			// Mat4
			SV_MODELVIEW_MAT,
			SV_PROJECTION_MAT,
			SV_MODELVIEWPROJECTION_MAT
		};

		/**
		 * Class for user defined material variables that will be passes in to the shader
		 */
		struct UserDefinedVar
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
			SpecialVar specialValue;
			const ShaderProg::UniVar* sProgVar;

			UserDefinedVar(): specialValue( SV_NONE ) {}
		}; // end UserDefinedVar

		static map<string, SpecialVar> keywordToSpecial;

		ShaderProg* shaderProg; ///< The most important aspect of materials

		bool blends; ///< The entities with blending are being rendered in blending stage and those without in material stage
		int  blendingSfactor;
		int  blendingDfactor;
		bool refracts;
		bool depthTesting;
		bool wireframe;
		bool castsShadow; ///< Used in shadowmapping passes but not in EarlyZ
		Vec<UserDefinedVar> userDefinedVars;

		// vertex attributes
		struct
		{
			int position;
			int tanget;
			int normal;
			int texCoords;

			// for hw skinning
			int vertWeightBonesNum;
			int vertWeightBoneIds;
			int vertWeightWeights;
		} attribLocs;

		// uniforms
		struct
		{
			int skinningRotations;
			int skinningTranslations;
		} uniLocs;

		Material* dpMtl;

		void setToDefault();
		bool additionalInit(); ///< The func is for not polluting load with extra code
		
	public:
		Material() { setToDefault(); }
		void setup();
		bool load( const char* filename );
		void unload();

		bool hasHWSkinning() const { return attribLocs.vertWeightBonesNum != -1; }
		bool hasAlphaTesting() const { return dpMtl!=NULL && dpMtl->attribLocs.texCoords!=-1; }
};


#endif

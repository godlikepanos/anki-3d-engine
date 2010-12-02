#ifndef MATERIAL_H
#define MATERIAL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include "Math.h"
#include "Resource.h"
#include "ShaderProg.h"
#include "Texture.h"
#include "RsrcPtr.h"


/// Mesh material Resource
///
/// Every material keeps info of how to render a MeshNode. Among this info it keeps the locations of attribute and
/// uniform variables. The variables can be standard or user defined. The standard variables have standard names inside
/// the shader program and we dont have to mention them in the .mtl files. The material init func scoops the shader
/// program for standard variables and keeps a pointer to the variable. The standard variables are like the GL build-in
/// variables (that we cannot longer use on GL >3) with a few additions. The user defined variables are defined and
/// values inside the .mtl file. The attribute variables cannot be user defined, the uniform on the other hand can.
///
/// XML file format:
/// @code
/// <material>
/// 	<shaderProg>
/// 		<file>path/file.glsl</file> |
/// 		<customMsSProg>
/// 			[<defines>
/// 				<define>DEFINE_SOMETHING</define>
/// 				...
/// 				<define>DEFINE_SOMETHING_ELSE</define>
/// 			</defines>]
/// 		</customMsSProg> |
/// 		<customDpSProg>...</customDpSProg>
/// 	</shaderProg>
///
/// 	[<blendingStage>true|false</blendingStage>]
///
/// 	[<blendFuncs> *
/// 		<sFactor>GL_SOMETHING</sFactor>
/// 		<dFactor>GL_SOMETHING</dFactor>
/// 	</blendFuncs>]
///
/// 	[<depthTesting>true|false</depthTesting>]
///
/// 	[<wireframe>true|false</wireframe>]
///
/// 	[<castsShadow>true|false</castsShadow>]
///
/// 	[<userDefinedVars>
/// 		<userDefinedVar>
/// 			<name>varNameInShader</name>
/// 			<value> **
/// 				<texture>path/tex.png</texture> |
/// 				<float>0.0</float> |
/// 				<vec2><x>0.0</x><y>0.0</y></vec2> |
/// 				<vec3><x>0.0</x><y>0.0</y><z>0.0</z></vec3> |
/// 				<vec4><x>0.0</x><y>0.0</y><z>0.0</z><w>0.0</w></vec4>
/// 			</value>
/// 		</userDefinedVar>
/// 		...
/// 		<userDefinedVar>...</userDefinedVar>
/// 	</userDefinedVars>]
/// </material>
///
/// *: Has nothing to do with the blendingStage. blendFuncs can be in material stage as well
/// **: Depends on the type of the var
/// @endcode
class Material: public Resource
{
	//====================================================================================================================
	// Nested                                                                                                            =
	//====================================================================================================================
	public:
		/// Standard attribute variables that are acceptable inside the material stage @ref ShaderProg
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

		/// Standard uniform variables. The Renderer sees what are applicable and sets them
		/// After changing the enum update also:
		/// - Some statics in Material.cpp
		/// - Renderer::setupMaterial
		/// - The generic material GLSL shader (maybe)
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

		/// Class for user defined material variables that will be passes in to the shader
		class UserDefinedUniVar
		{
			PROPERTY_R(float, float_, getFloat)
			PROPERTY_R(Vec2, vec2, getVec2)
			PROPERTY_R(Vec3, vec3, getVec3)
			PROPERTY_R(Vec4, vec4, getVec4)

			public:
				UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const char* texFilename);
				UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, float f);
				UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const Vec2& v);
				UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const Vec3& v);
				UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const Vec4& v);

				void set(uint& textureUnit) const;

			private:
				const ShaderProg::UniVar& sProgVar;
				RsrcPtr<Texture> texture;
		}; // end UserDefinedVar

	//====================================================================================================================
	// Properties                                                                                                        =
	//====================================================================================================================
	/// Used in depth passes of shadowmapping and not in other depth passes like EarlyZ
	PROPERTY_R(bool, castsShadow, isShadowCaster)

	/// The entities with blending are being rendered in blending stage and those without in material stage
	PROPERTY_R(bool, blendingStage, renderInBlendingStage)

	PROPERTY_R(int, blendingSfactor, getBlendingSfactor) ///< Default GL_ONE
	PROPERTY_R(int, blendingDfactor, getBlendingDfactor) ///< Default GL_ZERO
	PROPERTY_R(bool, depthTesting, isDepthTestingEnabled)
	PROPERTY_R(bool, wireframe, isWireframeEnabled)

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/// Initialize with default values
		Material();

		/// @see Resource::load
		void load(const char* filename);

		/// @name Accessors
		/// @{
		const ShaderProg::AttribVar* getStdAttribVar(StdAttribVars id) const {return stdAttribVars[id];}
		const ShaderProg::UniVar* getStdUniVar(StdUniVars id) const {return stdUniVars[id];}
		const ShaderProg& getShaderProg() const {return *shaderProg.get();}
		const boost::ptr_vector<UserDefinedUniVar>& getUserDefinedVars() const {return userDefinedVars;}
		/// @}

		/// @return Return true if the shader has references to hardware skinning
		bool hasHwSkinning() const {return stdAttribVars[SAV_VERT_WEIGHT_BONES_NUM] != NULL;}

		/// @return Return true if the shader has references to texture coordinates
		bool hasTexCoords() const {return stdAttribVars[SAV_TEX_COORDS] != NULL;}

		bool isBlendingEnabled() const {return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;}

	//====================================================================================================================
	// Private                                                                                                           =
	//====================================================================================================================
	private:
		/// A simple pair-like structure
		struct PreprocDefines
		{
			const char* switchName;
			const char prefix;
		};

		/// Information for the standard shader program variables
		struct StdVarNameAndGlDataTypePair
		{
			const char* varName;
			GLenum dataType; ///< aka GL data type
		};

		static PreprocDefines msGenericDefines[]; ///< Material stage defines accepted in MsGeneric.glsl
		static PreprocDefines dpGenericDefines[]; ///< Depth pass defines accepted in DpGeneric.glsl
		static StdVarNameAndGlDataTypePair stdAttribVarInfos[SAV_NUM];
		static StdVarNameAndGlDataTypePair stdUniVarInfos[SUV_NUM];
		const ShaderProg::AttribVar* stdAttribVars[SAV_NUM];
		const ShaderProg::UniVar* stdUniVars[SUV_NUM];
		RsrcPtr<ShaderProg> shaderProg; ///< The most important aspect of materials
		boost::ptr_vector<UserDefinedUniVar> userDefinedVars;

		/// The func sweeps all the variables of the shader program to find standard shader program variables. It updates
		/// the stdAttribVars and stdUniVars arrays.
		/// @exception Exception
		void initStdShaderVars();

		/// Parse for a custom shader
		/// @param[in] defines The acceptable defines array
		/// @param[in] pt The property tree. Its not the root tree
		/// @param[out] source The source to feed to ShaderProg::createSrcCodeToCache
		/// @param[out] prefix The prefix of file to feed to ShaderProg::createSrcCodeToCache
		static void parseCustomShader(const PreprocDefines defines[], const boost::property_tree::ptree& pt,
		                              std::string& source, std::string& prefix);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline Material::UserDefinedUniVar::UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, float f):
	float_ (f),
	sProgVar(sProgVar)
{}


inline Material::UserDefinedUniVar::UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const Vec2& v):
	vec2(v),
	sProgVar(sProgVar)
{}


inline Material::UserDefinedUniVar::UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const Vec3& v):
	vec3(v),
	sProgVar(sProgVar)
{}


inline Material::UserDefinedUniVar::UserDefinedUniVar(const ShaderProg::UniVar& sProgVar, const Vec4& v):
	vec4(v),
	sProgVar(sProgVar)
{}


#endif

#ifndef MATERIAL_H
#define MATERIAL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/array.hpp>
#include "Math.h"
#include "ShaderProg.h"
#include "RsrcPtr.h"
#include "MtlUserDefinedVar.h"


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
/// 				<fai>msDepthFai | isFai | ppsPrePassFai | ppsPostPassFai</fai> |
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
class Material
{
	//==================================================================================================================
	// Public                                                                                                          =
	//==================================================================================================================
	public:
		/// Standard attribute variables that are acceptable inside the @ref ShaderProg
		enum StdAttribVars
		{
			SAV_POSITION,
			SAV_TANGENT,
			SAV_NORMAL,
			SAV_TEX_COORDS,
			SAV_NUM
		};

		/// Standard uniform variables. The Renderer sees what are applicable and sets them
		/// After changing the enum update also:
		/// - Some statics in Material.cpp
		/// - Renderer::setupShaderProg
		/// - The generic material GLSL shader (maybe)
		enum StdUniVars
		{
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


		/// Initialize with default values
		Material();

		/// @see Resource::load
		void load(const char* filename);

		/// @name Accessors
		/// @{
		const SProgAttribVar* getStdAttribVar(StdAttribVars id) const {return stdAttribVars[id];}
		const SProgUniVar* getStdUniVar(StdUniVars id) const {return stdUniVars[id];}
		const ShaderProg& getShaderProg() const {return *shaderProg.get();}
		GETTER_R(bool, castsShadow, isShadowCaster)
		GETTER_R(bool, blendingStage, renderInBlendingStage)
		GETTER_R(int, blendingSfactor, getBlendingSfactor)
		GETTER_R(int, blendingDfactor, getBlendingDfactor)
		GETTER_R(bool, depthTesting, isDepthTestingEnabled)
		GETTER_R(bool, wireframe, isWireframeEnabled)
		GETTER_R(boost::ptr_vector<MtlUserDefinedVar>, userDefinedVars, getUserDefinedVars)
		/// @}

		/// @return Return true if the shader has references to texture coordinates
		bool hasTexCoords() const {return stdAttribVars[SAV_TEX_COORDS] != NULL;}

		bool isBlendingEnabled() const {return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;}

	//==================================================================================================================
	// Private                                                                                                         =
	//==================================================================================================================
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

		bool castsShadow; ///< Used in depth passes of shadowmapping and not in other depth passes like EarlyZ
		/// The entities with blending are being rendered in blending stage and those without in material stage
		bool blendingStage;
		int blendingSfactor; ///< Default GL_ONE
		int blendingDfactor; ///< Default GL_ZERO
		bool depthTesting;
		bool wireframe;
		boost::ptr_vector<MtlUserDefinedVar> userDefinedVars;

		static PreprocDefines msGenericDefines[]; ///< Material stage defines accepted in MsGeneric.glsl
		static PreprocDefines dpGenericDefines[]; ///< Depth pass defines accepted in DpGeneric.glsl
		static boost::array<StdVarNameAndGlDataTypePair, SAV_NUM> stdAttribVarInfos;
		static boost::array<StdVarNameAndGlDataTypePair, SUV_NUM> stdUniVarInfos;
		boost::array<const SProgAttribVar*, SAV_NUM> stdAttribVars;
		boost::array<const SProgUniVar*, SUV_NUM> stdUniVars;
		RsrcPtr<ShaderProg> shaderProg; ///< The most important aspect of materials

		/// The func sweeps all the variables of the shader program to find standard shader program variables. It
		/// updates the stdAttribVars and stdUniVars arrays.
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


#endif
